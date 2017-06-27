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

		/**
		 * @brief square Squares a float.
		 */
		inline float square(float x) { return x*x; }

		/**
		 * @brief add_interp_data Adds factor-1 data points between all points in samples.
		 * The new points are linearly interpolated between their neighbours.
		 * @param samples The collection of data.
		 * @param factor The factor by which to grow the data.
		 * The data actually grows to oldsize*factor - oldsize
		 * @return The new collection of data including the interpolated points.
		 */
		std::vector<float> linear_interp(std::vector<float> samples, unsigned factor);

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

		/**
		 * @brief mean Calculates the MLE for gaussian mean (average) over the collection of data.
		 * @param samples The collection of sample data.
		 * @return The average.
		 */
		float mean(const std::vector<float>& samples);

		/**
		 * @brief variance Calculates the MLE for gaussian variance (average of squared deviation from mean) over the collection of data.
		 * @param samples The collection of sample data.
		 * @param mean The gaussian mean.
		 * @return The MLE for gaussian variance.
		 */
		float variance(const std::vector<float>& samples, float mean);

		/**
		 * @brief em_step Executes one step of the "Expectation Maximization" algorithm on a GMM using sample data.
		 * @param samples The sample data.
		 * @param components The GMMs components.
		 */
		void em_step(const std::vector<float>& samples, std::vector<GMMComponent>& gmm);

		/**
		 * @brief fit_gmm Attempts to fit a gaussian mixture model with n components to the sample data by using the EM algorithm.
		 * Stops after max_iterations or when the loglikelihood does not change by more than epsilon between iterations.
		 * @param samples The sample data.
		 * @param num_components The number of components the GMM will have.
		 * @param epsilon The epsilon used for the finishing condition (new_loglikelyhood - old_loglikelihood < epsilon)
		 * @param max_iterations Second finishing condition.
		 * @return Collection of components representing the GMM.
		 */
		std::vector<GMMComponent> fit_gmm(const std::vector<float>& samples, unsigned num_components, float epsilon, unsigned max_iterations);

		/**
		 * @brief gmm_log_likelyhood Calculates the log-likelihood of a given GMM at generating given samples.
		 * @param samples The sample data.
		 * @param gmm The GMMs components.
		 * @return The log-likelihood of the GMM generating the samples.
		 */
		float gmm_log_likelihood(const std::vector<float>& samples, const std::vector<GMMComponent>& gmm);

		/**
		 * @brief find_peaks Tries to approximate the number of modes (peaks) in the samples distribution.
		 * @param samples The samples that are analyzed.
		 * @return A collection of the midpoints of the peaks. TODO:change this line
		 */
		unsigned count_peaks(const std::vector<float>& samples, unsigned num_bins);

		/**
		 * @brief pick_randomly Picks a random float from a collection of possible values.
		 * @param samples Collection of possible values.
		 * @return The random float.
		 */
		float pick_randomly(const std::vector<float>& samples);

		/**
		 * @brief pick_randomly Picks a collection of floats from a collection of possible values.
		 * No multiple picks of the same indices.
		 * @param samples Collection of possible values.
		 * @param num_picks Number of picks.
		 * @return The collection that contains all picks.
		 */
		std::vector<float> pick_randomly(std::vector<float> samples, unsigned num_picks);
	}
}

#endif // MATH_UTIL_H
