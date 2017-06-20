#include "math_util.h"

#include <cmath>
#include <random>
#include <algorithm>
#include <limits>

#include <iostream>

namespace vis
{
	void math_util::test_em()
	{
		auto components = std::vector<GMMComponent>{{0.f, 1.f, .5f},{3.f, 1.f, .5f}};
		auto samples = std::vector<float>(1000);

		auto re = std::default_random_engine{std::random_device{}()};
		for(unsigned i = 0; i < samples.size(); ++i)
		{
			unsigned c = (i < samples.size()/2 ? 0 : 1);
			samples[i] = std::normal_distribution<float>{components[c]._mean, components[c]._variance}(re);
		}

		for(int i = 0; i < 2; ++i)
		{
			std::cout << "\n------------------------------------\nmin:" << *std::min_element(samples.begin(), samples.end()) << " max:" << *std::max_element(samples.begin(), samples.end());
			std::cout << "\n------------------------------------\nbins:\n";
			auto modes = count_modes(samples, 5);
			std::cout << "\nnum_modes: " << modes << "\n";

			std::cout << "real values:\n";
			for(auto& c : components)
				std::cout << "mean:" << c._mean << " variance:" << c._variance << " weight:" << c._weight << "\n";
			components = std::vector<GMMComponent>{{-1.f, 2.f, .1f},{6.f, 1.f, .7f}};
			std::cout << "EM start values:\n";
			for(auto& c : components)
				std::cout << "mean:" << c._mean << " variance:" << c._variance << " weight:" << c._weight << "\n";
			std::cout << "EM start:\n";
			for(unsigned i = 0; i < 50; ++i)
			{
				em_step(samples, components);
				for(auto& c : components)
					std::cout << "mean:" << c._mean << " \tvariance:" << c._variance << " \tweight:" << c._weight << "\n";
				std::cout << "-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-\n";
			}
			std::cout << "\n----------------INTERPOLATION--------------------\n\n";
			components = std::vector<GMMComponent>{{0.f, 1.f, .5f},{3.f, 1.f, .5f}};
			samples = add_interp_data(samples, 2);
		}
	}

	std::vector<float> math_util::add_interp_data(std::vector<float> samples, unsigned factor)
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

	unsigned math_util::count_modes(const std::vector<float>& samples, unsigned num_bins)
	{
		// Method: Binning (histogram) using equally spaced (sized) bins between sample_min and sample_max
		// TODO:use a better way to count modes
		auto bins = std::vector<unsigned>(num_bins);
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

//		std::cout << "min:" << min - 0.5f*width << " max:" << min - 0.5f*width + width*bins.size() << "\n---\n";
//		unsigned sum = 0;
//		for(auto& bin : bins)
//		{
//			sum += bin;
//			std::cout << bin << "\t";
//			for(unsigned i = 0; i < bin; ++i)
//				std::cout << "#";
//			std::cout << "\n---\n";
//		}
//		if(sum < samples.size())
//			std::cout << "ERROR";

		return num_peaks;
	}
}
