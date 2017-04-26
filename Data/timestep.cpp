#include "timestep.h"

#include <sstream>
#include <algorithm>
#include <cmath>

namespace vis
{
	std::unique_ptr<Timestep> Timestep::buildFromGaussianAnalysis(std::vector<Timestep>& ensemble)
	{
		using namespace std::literals::string_literals;

		// Validate ensemble to only have elements of the same format
		if(ensemble.empty() ||
				std::adjacent_find(ensemble.begin(), ensemble.end(),
								   [] (Timestep& a, Timestep& b) {return !a.matchingFormat(b);} )
				!= ensemble.end())
		{
			return std::unique_ptr<Timestep>{nullptr};
			// TODO:ERROR handling no timesteps or they do not have the same dimensions
		}

		auto step = std::make_unique<Timestep>();

		// Set format to the same as the timesteps in tsteps
		step->_xSize = ensemble.front().xSize();
		step->_ySize = ensemble.front().ySize();
		step->_zSize = ensemble.front().zSize();
		step->_numScalarFields = ensemble.front().numScalarFields() * 2; // *2 to store average and variance for every field

		// Copy scalar field names twice (for average and variance)
		step->_scalarFieldNames.reserve(step->_numScalarFields);
		for(auto& name : ensemble.front().scalarFieldNames())
			step->_scalarFieldNames.push_back(name + "_avg"s);
		for(auto& name : ensemble.front().scalarFieldNames())
			step->_scalarFieldNames.push_back(name + "_var"s);


		step->_data.resize(step->totalPoints(), 0.0f);
		step->_scalarFieldMin.reserve(step->_numScalarFields);
		step->_scalarFieldMax.reserve(step->_numScalarFields);

		for(unsigned i = 0; i < step->totalPoints(); ++i)
		{
			if(i < step->totalPoints()/2)	// Calculate sum
			{
				for(auto& curstep : ensemble)
					step->_data[i] += curstep.data()[i];
			}
			else							// Calculate sum of distance
			{
				for(auto& curstep : ensemble)
					step->_data[i] += std::fabs(step->_data[i - step->totalPoints()/2] - curstep.data()[i - step->totalPoints()/2]);
			}

			// Average
			step->_data[i] /= ensemble.size();

			// Calculate min and max per scalar field for normalization
			unsigned field = i/step->scalarsPerField();
			if(step->_data[i] < step->_scalarFieldMin[field])
				step->_scalarFieldMin[field] = step->_data[i];
			if(step->_data[i] > step->_scalarFieldMax[field])
				step->_scalarFieldMax[field] = step->_data[i];
		}
		return step;
	}

	Timestep::Timestep()
	{

	}

	Timestep::Timestep(std::istream& instream)
	{
		using namespace std::literals::string_literals;

		auto line = ""s;
		auto linestream = std::stringstream{};

		// Read dimensions header
		std::getline(instream, line, '\n');
		linestream.str(line);
		linestream >> _xSize >> _ySize >> _zSize;

		// Read scalar field header
		std::getline(instream, line, '\n');
		linestream.str(line);
		linestream >> _numScalarFields;

		_scalarFieldNames.resize(_numScalarFields);

		for(unsigned i = 0; i < _numScalarFields; ++i)
		{
			linestream >> _scalarFieldNames[i];
		}

		// Skip vector fields header
		instream.ignore(256, '\n'); // This line should not be longer than 256 characters.

		_scalarFieldMin.resize(_numScalarFields, std::numeric_limits<float>::infinity());
		_scalarFieldMax.resize(_numScalarFields, -std::numeric_limits<float>::infinity());
		_data.resize(scalarsPerField()*_numScalarFields);

		// Read data
		for(unsigned i = 0; i < scalarsPerField()*_numScalarFields; ++i)
		{
			// Using instream >> _data[i] takes about twice as long.
			std::getline(instream, line, ' ');
			_data[i] = std::stof(line);

			// Calculate min and max per scalr field for normalization
			unsigned field = i/scalarsPerField();
			if(_data[i] < _scalarFieldMin[field])
				_scalarFieldMin[field] = _data[i];
			if(_data[i] > _scalarFieldMax[field])
				_scalarFieldMax[field] = _data[i];
		}
	}

	bool Timestep::matchingFormat(Timestep& other) const
	{
		return other.xSize() == _xSize &&
				other.ySize() == _ySize &&
				other.zSize() == _zSize &&
				other.numScalarFields() == _numScalarFields &&
				other.scalarFieldNames() == _scalarFieldNames;
	}

	bool Timestep::empty()
	{
		return _xSize == 0 && _ySize == 0 && _zSize == 0 && _numScalarFields == 0;
	}

	unsigned Timestep::xSize() const
	{
		return _xSize;
	}

	unsigned Timestep::ySize() const
	{
		return _ySize;
	}

	unsigned Timestep::zSize() const
	{
		return _zSize;
	}

	unsigned Timestep::scalarsPerField() const
	{
		return _xSize*_ySize*_zSize;
	}

	unsigned Timestep::numScalarFields() const
	{
		return _numScalarFields;
	}

	unsigned Timestep::totalPoints() const
	{
		return _numScalarFields * scalarsPerField();
	}

	const std::vector<float>& Timestep::const_data() const
	{
		return _data;
	}

	std::vector<float>& Timestep::data()
	{
		return _data;
	}

	const std::vector<std::string>& Timestep::scalarFieldNames() const
	{
		return _scalarFieldNames;
	}
}
