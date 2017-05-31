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
		struct ScalarField
		{
			explicit ScalarField(unsigned width = 0, unsigned height = 0, unsigned depth = 0)
				: _width{width},
				  _height{height},
				  _depth{depth},
				  _data(width*height*depth)
			{

			}

			unsigned num_scalars() const
			{
				return _width*_height*_depth;
			}

			bool same_dimensions(const ScalarField& other) const
			{
				return other._width == _width
						&& other._height == _height
						&& other._depth == _depth;
			}

			std::string _name{};
			unsigned _width{0};
			unsigned _height{0};
			unsigned _depth{0};
			std::vector<float> _data{};
			float _minimum{std::numeric_limits<float>::infinity()};
			float _maximum{-std::numeric_limits<float>::infinity()};
		};

		/**
		 * @brief gaussianAnalysis Constructs a new timestep from the results of analysing an ensemble of other timesteps.
		 * The new timestep will contain scalar fields for average and empirical deviation of the analysed timesteps.
		 * @param ensemble The ensemble of timesteps that are analysed. All steps are expected to have the same format.
		 * @return The newly constructed Timestep.
		 */
		static Timestep gaussianAnalysis(const std::vector<Timestep>& ensemble);

		/**
		 * @brief gaussianMixtureAnalysis Constructs a new timestep from the results of analysing an ensemble of other timesteps.
		 * The new timestep will contain scalar fields for average and empirical deviation of the analysed timesteps.
		 * The fields will contain the average and deviation of all components of the fit GMM using the depth.
		 * @param ensemble The ensemble that is analysed. All steps are expected to have the same format. Data with depth>1 will be ignored.
		 * @param num_components The number of components the gaussian mixture model will have.
		 * @return The newly constructed timestep.
		 */
		static Timestep gaussianMixtureAnalysis(const std::vector<Timestep>& ensemble, unsigned num_components = 2);

		/**
		 * @brief Timestep Empty constructor. Leaves everything as default.
		 */
		explicit Timestep() = default;

		/**
		 * @brief Timestep from text. Parses input from @param instream.
		 * TODO:describe text format
		 */
		explicit Timestep(std::istream& instream);

		const std::vector<ScalarField>& fields() const;

		unsigned num_fields() const;

		/**
		 * @brief same_format Compares format to another timestep
		 * @param other Timestep to compare to.
		 * @return Bool that is true, if the format matches.
		 */
		bool same_format(const Timestep& other) const;

		/**
		 * @brief empty Checks if all dimensions and # of fields are 0.
		 * Does not check if any data is present.
		 */
		bool empty() const;

	private:
		std::vector<ScalarField> _fields;
	};
}
#endif // TIMESTEP_H
