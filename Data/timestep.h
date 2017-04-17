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
		explicit Timestep();
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
		explicit Timestep(std::vector<Timestep>& tsteps);

		//Dimensions TODO:doc
		unsigned xSize();
		unsigned ySize();
		unsigned zSize();
		unsigned scalarsPerField();
		unsigned numScalarFields();
		unsigned totalPoints();

		/**
		 * @brief sameDimensions Returns true if @param other has the same dimensions and fields.
		 */
		bool matchingFormat(Timestep& other);

		/**
		 * @brief empty Returns true if all dimensions and # of fields are 0.
		 */
		bool empty();

		/**
		 * @brief data Returns a reference to the raw float data of this step.
		 * The data is expected to follow the size and format defined by the dimensions and fields.
		 * TODO:describe format
		 */
		const std::vector<float>& data();

		/**
		 * @brief scalarFieldNames Contains names of the scalar fields of this step.
		 */
		std::vector<std::string> scalarFieldNames();

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
		std::vector<std::string> _scalarFieldNames{};
		///Smallest value per scalar field.
		std::vector<float> _scalarFieldMin{};
		///Largest value per scalar field.
		std::vector<float> _scalarFieldMax{};

		///Raw data in floats. All scalarfields after each other.
		std::vector<float> _data{};
	};
}
#endif // TIMESTEP_H
