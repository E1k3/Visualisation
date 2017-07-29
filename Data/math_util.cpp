#include "math_util.h"

#include <cmath>
#include <random>
#include <algorithm>
#include <limits>
#include <random>

#include <iostream>

namespace vis
{
	std::vector<float> math_util::linear_interp(std::vector<float> samples, unsigned factor)
	{
		auto interped = samples;
		interped.reserve(samples.size()*factor - factor);
		std::sort(samples.begin(), samples.end());
		for(unsigned s = 0; s < samples.size()-1; ++s)
		{
			interped.push_back(samples[s]);
			for(unsigned i = 1; i < factor; ++i)
				interped.push_back(samples[s] + 1.f/i * (samples[s+1] - samples[s]));
		}
		std::sort(interped.begin(), interped.end());
		return interped;
	}

	float math_util::normal_density(float x, float mean, float variance)
	{
		if(variance <= 0.f)
			return 0.f;
		constexpr float pi = static_cast<float>(M_PI);
		return (1 / (std::sqrt( 2 * pi * variance))) * std::exp(- square(x - mean) / (2 * variance));
	}

	float math_util::gmm_density(float x, const std::vector<math_util::GMMComponent>& gmm)
	{
		auto sum = 0.f;
		for(auto& c : gmm)
			sum += c._weight * normal_density(x, c._mean, c._variance);
		return sum;
	}

	float math_util::mean(const std::vector<float>& samples)
	{
		float sum = 0.f;
		for(const auto& sample : samples)
			sum += sample;
		return sum / samples.size();
	}

	float math_util::variance(const std::vector<float>& samples, float mean)
	{
		float sum = 0.f;
		for(const auto& sample : samples)
			sum += std::pow(mean - sample, 2);
		return sum / samples.size();
	}

	void math_util::em_step(const std::vector<float>& samples, std::vector<math_util::GMMComponent>& gmm)
	{
		auto sample_weights = std::vector<float>{};
		sample_weights.resize(samples.size() * gmm.size());

		// E-step
		{	// Scope i
			unsigned i = 0;
			for(const auto& samp : samples)
			{
				auto sample_gmm_density = gmm_density(samp, gmm);	// TODO:make sure this is > 0
				for(const auto& comp : gmm)
					sample_weights[i++] = normal_density(samp, comp._mean, comp._variance) * comp._weight / sample_gmm_density;
			}
		}

		// M-step
		for(unsigned c = 0; c < gmm.size(); ++c)
		{
			// Sum of the membership weights of the current component
			auto weight_sum = 0.f;

			// Weight
			for(unsigned s = 0; s < samples.size(); ++s)
				weight_sum += sample_weights[s * gmm.size() + c];
			gmm[c]._weight = weight_sum / samples.size();

			// Mean
			gmm[c]._mean = 0.f;
			for(unsigned s = 0; s < samples.size(); ++s)
				gmm[c]._mean += sample_weights[s * gmm.size() + c] * samples[s];
			gmm[c]._mean /= weight_sum;

			// Variance
			gmm[c]._variance = 0.f;
			for(unsigned s = 0; s < samples.size(); ++s)
				gmm[c]._variance += sample_weights[s * gmm.size() + c] * square(samples[s] - gmm[c]._mean);
			gmm[c]._variance /= weight_sum;

			// Avoid singularity (variance == 0 -> mean == NaN, weight == NaN, etc)
			if(gmm[c]._variance <= std::numeric_limits<float>::min())
			{
				// TODO:test different resets
				gmm[c]._mean = pick_randomly(samples);
				gmm[c]._variance = variance(samples, gmm[c]._mean);
			}
		}
	}


	std::vector<math_util::GMMComponent> math_util::fit_gmm(const std::vector<float>& samples, unsigned max_components)
	{
		if(max_components == 0)
			return {};

		// Initialize result with single gauss MLE
		auto result = std::vector<GMMComponent>{{}};
		result.push_back({});
		result.front()._mean = mean(samples);
		result.front()._variance = variance(samples, result.front()._mean);
		result.front()._weight = 1.f;

		auto min_bic = gmm_bic(samples, result, 0.5f);

		// Try GMMs with [2, max_components] components
		// k = current number of components
		for(unsigned k = 2; k <= max_components; ++k)
		{
			auto gmm = std::vector<GMMComponent>{};

			// Try initializing randomly, choose best result
			{
				float max_likelihood = -std::numeric_limits<float>::infinity();
				for(int t = 0; t < fit_gmm_random_init_tries; ++t)
				{
					auto init = std::vector<GMMComponent>{};
					init.reserve(k);
					for(auto& s : pick_randomly(samples, k))
						init.push_back({s, variance(samples, s), 1.f/k});

					auto cur_likelihood = gmm_likelihood(samples, init);
					if(cur_likelihood > max_likelihood)
					{
						max_likelihood = cur_likelihood;
						gmm = init;
					}
				}
			}

			// Iterate until difference in log-likelihood <= epsilon
			auto confidence = gmm_log_likelihood(samples, gmm);
			for(unsigned j = 0; j < fit_gmm_max_iterations; ++j)
			{
				em_step(samples, gmm);
				auto new_confidence = gmm_log_likelihood(samples, gmm);
				if(std::abs(confidence - new_confidence) < fit_gmm_log_likelihood_epsilon)
					break;
				confidence = new_confidence;
			}
			std::sort(gmm.begin(), gmm.end(), [] (const auto& a, const auto& b) { return a._mean < b._mean && a._weight != 0.f; });

			// If current model has lowest BIC (Bayesian Information criterion), keep iterating
			auto cur_bic = gmm_bic(samples, gmm, 0.5f);
			if(cur_bic < min_bic)
			{
				min_bic = cur_bic;
				result = gmm;
			}
			// If not, the previous model is assumed best
			else
				break;
		}

		result.resize(max_components);
		return result;
	}

	float math_util::gmm_log_likelihood(const std::vector<float>& samples, const std::vector<GMMComponent>& gmm)
	{
		auto sum = 0.f;
		for(const auto& s : samples)
			sum += std::log(gmm_density(s, gmm));
		return sum;
	}

	float math_util::gmm_likelihood(const std::vector<float>& samples, const std::vector<GMMComponent>& gmm)
	{
		auto sum = 0.f;
		for(const auto& s: samples)
			sum += gmm_density(s, gmm);
		return sum;
	}

	float math_util::gmm_bic(const std::vector<float>& samples, const std::vector<GMMComponent>& gmm, float k_bias)
	{
		return std::log(static_cast<float>(samples.size()))     // ln( #observations )
				* k_bias * (gmm.size() * 3 - 1)                          // * #free parameters   | 1D -> n free means, n free variances, n-1 free weights (n sum to 1) = 3n - 1
				- 2 * std::log(gmm_likelihood(samples, gmm));   // - 2 * ln( likelihood )
	}

	float math_util::gmm_aic(const std::vector<float>& samples, const std::vector<GMMComponent>& gmm, float k_bias)
	{
		return k_bias * 2 * (gmm.size() * 3 - 1)                          // 2 * #free parameters   | 1D -> n free means, n free variances, n-1 free weights (n sum to 1) = 3n - 1
				- 2 * std::log(gmm_likelihood(samples, gmm));   // - 2 * ln( likelihood )
	}

	float math_util::gmm_aic_c(const std::vector<float>& samples, const std::vector<GMMComponent>& gmm, float k_bias)
	{
		auto k = gmm.size() * 3 - 1; // #free parameters   | 1D -> n free means, n free variances, n-1 free weights (n sum to 1) = 3n - 1
		return gmm_aic(samples, gmm, k_bias)
				+ (2 * k * (k+1) / (samples.size() - k - 1));   // + 2k(k+1) / (n-k-1)
	}

	unsigned math_util::count_peaks(const std::vector<float>& samples, unsigned num_bins)
	{
		auto bins = std::vector<unsigned>(num_bins);
		if(bins.size() <= 2)
			return {};

		const float min = *std::min_element(samples.begin(), samples.end());
		const float max = *std::max_element(samples.begin(), samples.end());
		float width = (max - min) / (bins.size()-1);

		if(width <= std::numeric_limits<float>::min())
			return {};

		// Fill bins
		for(const auto& sample : samples)
		{
			float ceiling = min + .5f*width;
			for(auto& bin : bins)
			{
				if(sample <= ceiling)
				{
					++bin;
					break;
				}
				ceiling += width;
			}
		}

		// Count peaks
		unsigned num_peaks = 0;
		bool rising = true;
		for(unsigned i = 1; i < bins.size(); ++i)
		{
			//			rising = bins[i] > bins[i-1] || rising;
			if(bins[i] > bins[i-1])
				rising = true;
			else if(bins[i] < bins[i-1])
			{
				if(rising)
					++num_peaks;
				rising = false;
			}
		}
		if(rising)
			++num_peaks;

		return num_peaks;
	}

	float math_util::find_max(const std::vector<float>& samples, const GMMComponent& comp)
	{
		float min = comp._mean-comp._variance;
		float max = comp._mean+comp._variance;
		auto samples_in_range = std::vector<float>{};
		std::copy_if(samples.begin(), samples.end(), std::back_inserter(samples_in_range),
					 [&min, &max] (const auto& s) { return s >= min && s < max; });

		auto bins = std::vector<long>(static_cast<size_t>(std::sqrt(samples_in_range.size()))); // TODO:best way to determine #bins?

		if(bins.size() < 2)
			return comp._mean; // TODO error

		auto bin_width = (max - min) / (bins.size()-1);
		for(int i = 0; i < static_cast<int>(bins.size()); ++i)
			bins[static_cast<size_t>(i)] = std::count_if(samples_in_range.begin(), samples_in_range.end(),
																		  [&i, &bin_width, &min] (const auto& s) { return s >= min + i * bin_width && s < min + i * (bin_width+1); });

		int max_i = 0;
		for(int i = 1; i < static_cast<int>(bins.size()); ++i)
			if(bins[static_cast<size_t>(i)] > bins[static_cast<size_t>(max_i)])
				max_i = i;

		if(static_cast<size_t>(max_i) == bins.size()/2 || bins[static_cast<size_t>(max_i)] == bins[bins.size()/2])
			return comp._mean;
		else
			return min + max_i * (bin_width + .5f);
	}

	float math_util::find_median(const std::vector<float>& samples, const GMMComponent& comp)
	{
		auto samples_in_range = std::vector<float>{};
		std::copy_if(samples.begin(), samples.end(), std::back_inserter(samples_in_range),
					 [&comp] (const auto& s) { return s >= comp._mean-comp._variance && s < comp._mean+comp._variance; });
		std::sort(samples_in_range.begin(), samples_in_range.end());

		return (!samples_in_range.empty()) ? samples_in_range[(samples_in_range.size() - 1) / 2] : comp._mean;
	}

	float math_util::pick_randomly(const std::vector<float>& samples)
	{
		auto re = std::default_random_engine{std::random_device{}()};
		return samples[std::uniform_int_distribution<size_t>(0, samples.size()-1)(re)];
	}

	std::vector<float> math_util::pick_randomly(std::vector<float> samples, unsigned num_picks)
	{
		if(num_picks >= samples.size())
			return {};

		auto re = std::default_random_engine{std::random_device{}()};
		for(unsigned i = 0; i < num_picks; ++i)
		{
			auto dist = std::uniform_int_distribution<size_t>{i, samples.size()-1};
			std::swap(samples[i], samples[dist(re)]);
		}
		samples.resize(num_picks);
		return samples;
	}
}
