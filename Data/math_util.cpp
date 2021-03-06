#include "math_util.h"

#include <random>
#include <algorithm>
#include <limits>
#include <random>
#include <iostream>

#include <gsl/gsl_assert>

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
		if(variance == 0.f)
			return 0.f;
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
			size_t i = 0;
			for(const auto& samp : samples)
			{
				auto sample_gmm_density = gmm_density(samp, gmm);
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
			for(size_t s = 0; s < samples.size(); ++s)
				weight_sum += sample_weights[s * gmm.size() + c];
			gmm[c]._weight = weight_sum / samples.size();

			// Mean
			gmm[c]._mean = 0.f;
			for(size_t s = 0; s < samples.size(); ++s)
				gmm[c]._mean += sample_weights[s * gmm.size() + c] * samples[s];
			gmm[c]._mean /= weight_sum;

			// Variance
			gmm[c]._variance = 0.f;
			for(size_t s = 0; s < samples.size(); ++s)
				gmm[c]._variance += sample_weights[s * gmm.size() + c] * square(samples[s] - gmm[c]._mean);
			gmm[c]._variance /= weight_sum;

			// Avoid singularity (variance == 0 -> mean == NaN, weight == NaN, etc)
			if(gmm[c]._variance <= std::numeric_limits<float>::min())
			{
				// Reset mean to a random sample and variance to the squared average deviation
				if(fit_gmm_random_init)
					gmm[c]._mean = pick_randomly(samples);
				else // In case randomness is turned off, use the first sample to be constistent between runs
					gmm[c]._mean = samples.front();
				gmm[c]._variance = variance(samples, gmm[c]._mean);
			}
		}
	}


	std::vector<math_util::GMMComponent> math_util::fit_gmm(const std::vector<float>& samples, unsigned max_components)
	{
		Expects(max_components != 0);

		// Initialize with single gauss MLE
		auto result = std::vector<GMMComponent>{{}};
		result.push_back({});
		result.front()._mean = mean(samples);
		result.front()._variance = variance(samples, result.front()._mean);
		result.front()._weight = 1.f;

		auto min_aic = gmm_aic(samples, result, fit_gmm_component_penalty_factor);

		// Try MLE GMMs with [2, max_components] components
		// k = current number of components
		for(unsigned k = 2; k <= max_components; ++k)
		{
			auto gmm = std::vector<GMMComponent>{};
			gmm.reserve(k);

			// Try initializing randomly, choose best result
			if(fit_gmm_random_init)
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
			else // Initialize using evenly spaced samples
			{
				for(unsigned s = 0; s < k; ++s)
					gmm.push_back({samples[static_cast<size_t>(samples.size() / k * (s + .5f))], variance(samples, samples[static_cast<size_t>(samples.size() / k * (s + .5f))]), 1.f/k});
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

			// If current model has lowest AIC (Akaike Information criterion), keep iterating
			auto cur_aic = gmm_aic(samples, gmm, fit_gmm_component_penalty_factor);
			if(cur_aic < min_aic)
			{
				min_aic = cur_aic;
				result = gmm;
			}
			// If not, the previous model is assumed best
			else
				break;
		}

		std::sort(result.begin(), result.end(), [] (const auto& a, const auto& b) { return a._mean < b._mean && a._weight != 0.f; });
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
		Expects(num_bins > 1);
		auto bins = std::vector<unsigned>(num_bins);

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

		auto bins = std::vector<long>(static_cast<size_t>(std::sqrt(samples_in_range.size())));

		if(bins.size() < 2)
			return comp._mean;

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
		std::random_shuffle(samples.begin(), samples.end());
		samples.resize(num_picks);
		return samples;
	}

	std::tuple<float, float> math_util::round_interval(float lower_bound, float upper_bound)
	{
		Expects(lower_bound <= upper_bound);

		if(upper_bound - lower_bound <= std::numeric_limits<float>::min())
			return std::make_tuple(lower_bound, upper_bound);

		constexpr auto max_rounding_error = .1f;
		constexpr auto accuracy_factor = 10;

		auto interval_width = upper_bound - lower_bound;
		auto lower_rounded = std::floor(lower_bound);
		auto upper_rounded = std::ceil(upper_bound);

		int factor = accuracy_factor;
		while(lower_bound - lower_rounded > interval_width * max_rounding_error)
		{
			lower_rounded = std::floor(lower_bound * factor) / factor;
			factor *= accuracy_factor;
		}

		factor = accuracy_factor;
		while(upper_rounded - upper_bound > interval_width * max_rounding_error)
		{
			upper_rounded = std::ceil(upper_bound * factor) / factor;
			factor *= accuracy_factor;
		}

		return std::make_tuple(lower_rounded, upper_rounded);
	}

	std::vector<float> math_util::reasonable_divisions(float lower_bound, float upper_bound, int division_count)
	{
		Expects(lower_bound <= upper_bound);
		Expects(division_count > 0);

		constexpr float readable_sizes[] = {.0001f, .0005f, .001f, .005f, .01f, .05f, .1f, .25, .5, 1.f, 2.f, 5.f, 10.f, 20.f, 50.f, 100.f};

		float best_size = 0.f;
		int last_count = std::numeric_limits<int>::max();
		for(const auto& size : readable_sizes)
		{
			// The effective number of divisions when chosing size as division width
			auto count = static_cast<int>(std::floor(upper_bound / size) - std::ceil(lower_bound / size));


			if(count < 0)
				return {};

			// If the distance to the target count is rising, the last size was the best
			if(std::abs(last_count - division_count) < std::abs(count - division_count))
				break;

			last_count = count;
			best_size = size;
		}

		float lower_rounded = std::ceil(lower_bound / best_size) * best_size;
//		float upper_rounded = std::floor(upper_bound / best_size) * best_size; Not needed

		auto divs = std::vector<float>(static_cast<size_t>(last_count) + 1);
		for(auto& div : divs)
		{
			div = lower_rounded;
			lower_rounded += best_size;
		}

		return divs;
	}

	std::vector<float> math_util::combined_minima(const Field& mean_field, const Field& dev_field)
	{
		Expects(mean_field.equal_layout(dev_field));

		auto minima = std::vector<float>(static_cast<size_t>(mean_field.point_dimension()), std::numeric_limits<float>::infinity());
		for(int d = 0; d < mean_field.point_dimension(); ++d)
			for(int i = 0; i < mean_field.volume(); ++i)
				minima[static_cast<size_t>(d)] = std::min(minima[static_cast<size_t>(d)], mean_field.get_value(d, i) - std::abs(dev_field.get_value(d, i)));
		return minima;
	}

	float math_util::combined_minimum(const Field& mean_field, const Field& dev_field)
	{
		auto minima = combined_minima(mean_field, dev_field);
		return *std::min_element(minima.begin(), minima.end());
	}

	std::vector<float> math_util::combined_maxima(const Field& mean_field, const Field& dev_field)
	{
		Expects(mean_field.equal_layout(dev_field));

		auto maxima = std::vector<float>(static_cast<size_t>(mean_field.point_dimension()), -std::numeric_limits<float>::infinity());
		for(int d = 0; d < mean_field.point_dimension(); ++d)
			for(int i = 0; i < mean_field.volume(); ++i)
				maxima[static_cast<size_t>(d)] = std::max(maxima[static_cast<size_t>(d)], mean_field.get_value(d, i) + std::abs(dev_field.get_value(d, i)));
		return maxima;
	}

	float math_util::combined_maximum(const Field& mean_field, const Field& dev_field)
	{
		auto maxima = combined_maxima(mean_field, dev_field);
		return *std::max_element(maxima.begin(), maxima.end());
	}
}
