#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#include <vector>

namespace vis
{
	namespace math_util
	{
		struct GMMComponent
		{
			float _mean;
			float _variance;
			float _weight;
		};

		inline float square(float x) { return x*x; }

		void test_em();
		std::vector<float> add_interp_data(std::vector<float> samples, unsigned factor);

		/**
		 * @brief normal_density Calculates the probability density of a normal distribution at one point.
		 * @param x The point.
		 * @param mean The distributions mean.
		 * @param variance The distributions variance.
		 * @return The normal density at x.
		 */
		float normal_density(float x, float mean = 0.f, float variance = 1.f);

		/**
		 * @brief gmm_density Calculates the probability density of a gaussian mixture model at one point.
		 * @param x The point.
		 * @param components The GMMs components
		 * @return The GMM density at x.
		 */
		float gmm_density(float x, const std::vector<GMMComponent>& gmm);

		float mean(const std::vector<float>& samples);

		float variance(const std::vector<float>& samples, float mean);

		/**
		 * @brief em_step Executes one step of the "Expectation Maximization" algorithm on a GMM using sample data.
		 * @param samples The sample data.
		 * @param components The GMMs components.
		 */
		void em_step(const std::vector<float>& samples, std::vector<GMMComponent>& gmm);

		std::vector<GMMComponent> fit_gmm(const std::vector<float>& samples, unsigned num_components, float epsilon, unsigned max_iterations);

		float gmm_log_likelyhood(const std::vector<float>& samples, const std::vector<GMMComponent>& gmm);

		/**
		 * @brief find_peaks Tries to approximate the number of modes (peaks) in the samples distribution.
		 * @param samples The samples that are analyzed.
		 * @return A collection of the midpoints of the peaks. TODO:change this line
		 */
		unsigned count_peaks(const std::vector<float>& samples, unsigned num_bins);

		std::vector<float> pick_randomly(std::vector<float> samples, unsigned num_picks);
	}
}

#endif // MATH_UTIL_H
