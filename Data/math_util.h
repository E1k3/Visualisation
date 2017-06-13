#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#include <vector>

namespace vis
{
	namespace math_util
	{
		struct GMMComponent
		{
			float _weight;
			float _mean;
			float _variance;
		};

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
		float gmm_density(float x, const std::vector<GMMComponent>& components);

		/**
		 * @brief em_step Executes one step of the "Expectation Maximization" algorithm on a GMM using sample data.
		 * @param samples The sample data.
		 * @param components The GMMs components.
		 */
		void em_step(const std::vector<float>& samples, std::vector<GMMComponent>& components);
	}
}

#endif // MATH_UTIL_H
