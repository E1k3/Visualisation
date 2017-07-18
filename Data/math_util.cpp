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
				// TODO:find out which way to avoid singularities is the best
				gmm[c]._mean = pick_randomly(samples);
				gmm[c]._variance = variance(samples, gmm[c]._mean);
			}
		}
	}


	std::vector<math_util::GMMComponent> math_util::fit_gmm(const std::vector<float>& samples, unsigned num_components, float epsilon, unsigned max_iterations)
	{
		if(num_components == 0)
			return {};

		// Initialize result with single gauss mle
		auto result = std::vector<GMMComponent>{{}};
		result.push_back({});
		result.front()._mean = mean(samples);
		result.front()._variance = variance(samples, result.front()._mean);
		result.front()._weight = 1.f;

		auto max_score = gmm_likelihood(samples, result);

		for(unsigned i = 2; i <= num_components; ++i)
		{
			auto gmm = std::vector<GMMComponent>{};

			// Initialize GMM
			gmm.reserve(i);
			for(auto& s : pick_randomly(samples, i))
				gmm.push_back({s, variance(samples, s), 1.f/i});

			// Calculate log likelyhood of current gmm
			auto confidence = gmm_log_likelihood(samples, gmm);
			for(unsigned j = 0; j < max_iterations; ++j)
			{
				em_step(samples, gmm);
				auto new_confidence = gmm_log_likelihood(samples, gmm);
				if(std::abs(confidence - new_confidence) < epsilon)
					break;
				confidence = new_confidence;
			}
			std::sort(gmm.begin(), gmm.end(), [] (const auto& a, const auto& b) { return a._mean > b._mean && a._weight != 0.f; });

			auto current_score = gmm_likelihood(samples, gmm);// / (1 + i*0.1f);
			if(current_score > max_score) //TODO:find a good way to score gmm likelihood vs number of gmm components
			{
				max_score = current_score;
				result = gmm;
			}
			else
				break;
		}
		result.resize(num_components);
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

	float math_util::pick_randomly(const std::vector<float>& samples)
	{
		auto re = std::default_random_engine{std::random_device{}()};
		return samples[std::uniform_int_distribution<size_t>{0, samples.size()-1}(re)];
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
