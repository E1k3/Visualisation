#include "math_util.h"

#include <cmath>
#include <algorithm>
#include <limits>

namespace vis
{
	float math_util::normal_density(float x, float mean, float variance)
	{
		const float pi = std::acos(-1.f);
		return 1 / (std::sqrt( 2 * pi * variance)) * std::exp(-std::pow(x - mean, 2.f) / (2 * variance));
	}

	float math_util::gmm_density(float x, const std::vector<math_util::GMMComponent>& components)
	{
		auto sum = 0.f;
		for(auto& c : components)
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

	void math_util::em_step(const std::vector<float>& samples, std::vector<math_util::GMMComponent>& components)
	{
		auto sample_weights = std::vector<float>{};
		sample_weights.resize(samples.size() * components.size());

		// E-step
		{	// Scope i
			unsigned i = 0;
			for(const auto& samp : samples)
			{
				auto sample_prob_density = gmm_density(samp, components);
				for(const auto& comp : components)
					sample_weights[i++] = normal_density(samp, comp._mean, comp._variance) * comp._weight / sample_prob_density;
			}
		}

		// M-step
		for(unsigned c = 0; c < components.size(); ++c)
		{
			// Sum of the membership weights of the current component
			auto weight_sum = 0.f;

			// Weight
			for(unsigned s = 0; s < samples.size(); ++s)
				weight_sum += sample_weights[s * components.size() + c];
			components[c]._weight = weight_sum / samples.size();

			// Mean
			components[c]._mean = 0.f;
			for(unsigned s = 0; s < samples.size(); ++s)
				components[c]._mean += sample_weights[s * components.size() + c] * samples[s];
			components[c]._mean /= weight_sum;

			// Variance
			components[c]._variance = 0.f;
			for(unsigned s = 0; s < samples.size(); ++s)
				components[c]._variance += sample_weights[s * components.size() + c] * static_cast<float>(std::pow(samples[s] - components[c]._mean, 2));
			components[c]._variance /= weight_sum;
		}
	}

	unsigned math_util::count_modes(const std::vector<float>& samples)
	{
		// Method: Binning (histogram) using equally spaced (sized) bins between sample_min and sample_max
		// TODO:use a better way to count modes
		auto bins = std::vector<unsigned>(samples.size()/2);
		if(bins.size() < 2)
			return 0;

		const float min = *std::min_element(samples.begin(), samples.end());
		const float max = *std::max_element(samples.begin(), samples.end());
		float width = (max - min) / (bins.size()-1);

		if(width <= std::numeric_limits<float>::epsilon()*3) // TODO:changes this bullshit
			return 0;

		// Fill bins
		for(const auto& sample : samples)
		{
			float lower = min - 0.5f*width;
			for(auto& bin : bins)	// Inefficient search (TODO:use std function or custom binary search)
			{
				if(sample >= lower && sample < lower + width)
					++bin;
				lower += width;
			}
		}

		// Count peaks
		unsigned num_peaks = 0;
		bool rising = true;
		for(unsigned i = 1; i < bins.size(); ++i)
		{
			//			rising = bins[i] > bins[i-1] || rising;
			if(bins[i] > bins[i-1])
			{
				rising = true;
			}
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
}
