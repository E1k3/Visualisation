#ifndef TIMESTEP_H
#define TIMESTEP_H

#include <vector>
#include <string>
#include <istream>
#include <limits>
#include <memory>

namespace vis
{
	class Timestep
	{
	public:
		/**
		 * @brief fromGaussianAnalysis Constructs a new timestep from the results of analysing an ensemble of other timesteps.
		 * The new timestep will contain scalar fields for average and empirical variance of the analysed timesteps.
		 * @param ensemble The ensemble of timesteps that are analysed.
		 * @return Smartpointer to the newly constructed Timestep.
		 */
		static std::unique_ptr<Timestep> buildFromGaussianAnalysis(std::vector<Timestep>& ensemble);

		/**
		 * @brief Timestep from text. Parses input from @param instream.
		 * TODO:describe text format
		 */
		explicit Timestep(std::istream& instream);

		/**
		 * @brief Timestep from analysis of other steps.
		 * Computes average and empirical variance of @param tsteps
		 * and stores the results in adequately named scalar fields.
		 */
		explicit Timestep(std::vector<Timestep>& ensemble);

		// Dimensions TODO:doc
		unsigned xSize() const;
		unsigned ySize() const;
		unsigned zSize() const;
		unsigned scalarsPerField() const;
		unsigned numScalarFields() const;
		unsigned totalPoints() const;

		/**
		 * @brief sameDimensions Compares format to another timestep
		 * @param other Timestep to compare to.
		 * @return Bool that is true, if the format matches.
		 */
		bool matchingFormat(Timestep& other) const;

		/**
		 * @brief empty Checks if all dimensions and # of fields are 0.
		 * Does not check if any data is present.
		 */
		bool empty();

		/**
		 * @brief data Getter for a const reference to the raw float data.
		 * The data is expected to follow the size and format defined by the dimensions and fields.
		 * TODO:describe format
		 */
		const std::vector<float>& const_data() const;

		/**
		 * @brief data Returns a reference to the raw float data of this step.
		 * @sa @link Timestep::const_data
		 */
		std::vector<float>& data();

		/**
		 * @brief scalarFieldNames Contains names of the scalar fields of this step.
		 */
		const std::vector<std::string> scalarFieldNames() const;

	private:
		///Number of data points in x direction.
		unsigned _xSize{0};
		///Number of data points in y direction.
		unsigned _ySize{0};
		///Number of data points in z direction.
		unsigned _zSize{0};

		///Number of scalar fields.
		unsigned _numScalarFields{0};
		///Names of scalar fields.

		/// Size should always equal _numScalarFields
		std::vector<std::string> _scalarFieldNames{};

		/// Smallest value of each scalar field.
		/// Size should always equal _numScalarFields
		std::vector<float> _scalarFieldMin{};
		/// Largest value of each scalar field.
		/// Size should always equal _numScalarFields
		std::vector<float> _scalarFieldMax{};

		/// Raw data in floats. All scalarfields after each other.
		std::vector<float> _data{};
	};
}
#endif // TIMESTEP_H
