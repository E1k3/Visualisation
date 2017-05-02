#ifndef TIMESTEP_H
#define TIMESTEP_H

#include <vector>
#include <string>
#include <istream>
#include <limits>

namespace vis
{
	class Timestep
	{
	public:
		/**
		 * @brief fromGaussianAnalysis Constructs a new timestep from the results of analysing an ensemble of other timesteps.
		 * The new timestep will contain scalar fields for average and empirical variance of the analysed timesteps.
		 * @param ensemble The ensemble of timesteps that are analysed.
		 * @return The newly constructed Timestep.
		 */
		static Timestep buildFromGaussianAnalysis(const std::vector<Timestep>& ensemble);

		/**
		 * @brief Timestep Empty constructor. Leaves everything as default.
		 */
		explicit Timestep();

		/**
		 * @brief Timestep from text. Parses input from @param instream.
		 * TODO:describe text format
		 */
		explicit Timestep(std::istream& instream);

		/**
		 * @brief normalizeAll Normalizes all scalar fields from [min, max] to [0.0, 1.0].
		 */
		void normaliseAll();

		/**
		 * @brief normalize Normalizes a scalar field relative to the bounds of a (different) field.
		 * From [field.min, field.max] to [0.0, 1.0] <- relative to [boundsField.min, boundsField.max].
		 * @param field The field that is being normalized.
		 * @param boundsField The bounds of this field will be used to normalize.
		 */
		void normalise(unsigned field, unsigned boundsField);

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
		bool matchingFormat(const Timestep& other) const;

		/**
		 * @brief empty Checks if all dimensions and # of fields are 0.
		 * Does not check if any data is present.
		 */
		bool empty() const;

		/**
		 * @brief data Getter for a const reference to the raw float data.
		 * The data is expected to follow the size and format defined by the dimensions and fields.
		 * TODO:describe format
		 */
		const std::vector<float>& data() const;

		/**
		 * @brief data Returns a reference to the raw float data of this step.
		 * @sa @link Timestep::data
		 */
		std::vector<float>& data();

		/**
		 * @brief scalarFieldStart Gets start of a scalar field inside the raw data.
		 * Only checks bounds of index, assumes the data to follow the dimensions.
		 * @param index The index of the scalar field.
		 * @return Pointer to the first float that is part of the scalar field.
		 */
		const float* scalarFieldStart(unsigned index) const;

		/**
		 * @brief scalarFieldNames Contains names of the scalar fields of this step.
		 */
		const std::vector<std::string>& scalarFieldNames() const;

	private:
		/**
		 * @brief calcBounds Calculates the bounds of each scalar field.
		 * Erases everything that is currently stored inside the mins, maxs vectors.
		 */
		void calcBounds(std::vector<float>& mins, std::vector<float>& maxs);

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

		/// Raw data in floats. All scalarfields after each other.
		std::vector<float> _data{};
	};
}
#endif // TIMESTEP_H
