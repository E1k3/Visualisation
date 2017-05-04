#include "timestep.h"

#include <sstream>
#include <algorithm>
#include <cmath>

#include "logger.h"

namespace vis
{
	Timestep Timestep::buildFromGaussianAnalysis(const std::vector<Timestep>& ensemble)
	{
		using namespace std::literals::string_literals;

		// Validate ensemble to only have elements of the same format
		if(ensemble.empty() ||
				std::adjacent_find(ensemble.begin(), ensemble.end(),
								   [] (const Timestep& a, const Timestep& b) {return !a.matchingFormat(b);} )
				!= ensemble.end())
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Ensemble has elements with varying format" << std::endl;
			throw std::runtime_error("Gaussian Analysis Error");
			// TODO:ERROR handling no timesteps or they do not have the same dimensions
		}

		auto step = Timestep();

		// Set format to the same as the timesteps in tsteps
		step._xSize = ensemble.front().xSize();
		step._ySize = ensemble.front().ySize();
		step._zSize = ensemble.front().zSize();
		step._numScalarFields = ensemble.front().numScalarFields() * 2; // *2 to store average and variance for every field

		// Copy scalar field names twice (for average and variance)
		step._scalarFieldNames.reserve(step._numScalarFields);
		for(auto& name : ensemble.front().scalarFieldNames())
			step._scalarFieldNames.push_back(name + "_avg"s);
		for(auto& name : ensemble.front().scalarFieldNames())
			step._scalarFieldNames.push_back(name + "_var"s);

		step._data.resize(step.totalPoints(), 0.0f);
		for(unsigned i = 0; i < step.totalPoints(); ++i)
		{
			if(i < step.totalPoints()/2)	// Calculate sum
			{
				for(auto& curstep : ensemble)
					step._data[i] += curstep.data()[i];
			}
			else							// Calculate sum of distance
			{
				for(auto& curstep : ensemble)
				{
					step._data[i] += std::fabs(step._data[i - (step.totalPoints()/2)] - curstep.data()[i - (step.totalPoints()/2)]);
				}
			}

			// Average
			step._data[i] /= ensemble.size();
		}

		return step;
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

		_data.resize(scalarsPerField()*_numScalarFields);
		// Read data
		for(unsigned i = 0; i < scalarsPerField()*_numScalarFields; ++i)
		{
			// Using instream >> _data[i] takes about twice as long.
			std::getline(instream, line, ' ');
			_data[i] = std::stof(line);
		}
	}

	void Timestep::normaliseAll()
	{
		auto mins = std::vector<float>{};
		auto maxs = std::vector<float>{};
		calcBounds(mins, maxs);	// Calculate min and max per scalar field and store in min, max

		for(unsigned field = 0; field < _numScalarFields; ++field)
		{
			for(unsigned scalar = 0; scalar < scalarsPerField(); ++scalar)
			{
				unsigned i = (field*scalarsPerField()) + scalar;
				_data[i] = (_data[i] - mins[field]) / (maxs[field] - mins[field]);
			}
		}
	}

	void Timestep::normalise(unsigned field, unsigned boundsField)
	{
		auto mins = std::vector<float>{};
		auto maxs = std::vector<float>{};
		calcBounds(mins, maxs);	// Calculate min and max per scalar field and store in min, max

		if(field >= _numScalarFields || boundsField >= _numScalarFields)
		{
			throw std::runtime_error("Normalize Error");
			//TODO:ERROR handling. Fields are out of bounds.
		}

		for(unsigned i = field * scalarsPerField(); i < (field+1) * scalarsPerField(); ++i)
		{
			_data[i] = (_data[i] - mins[boundsField]) / (maxs[boundsField] - mins[boundsField]);
		}
	}

	void Timestep::calcBounds(std::vector<float>& mins, std::vector<float>& maxs)
	{
		mins.clear();
		maxs.clear();
		mins.resize(_numScalarFields, std::numeric_limits<float>::infinity());
		maxs.resize(_numScalarFields, -std::numeric_limits<float>::infinity());

		for(unsigned field = 0; field < _numScalarFields; ++field)
		{
			for(unsigned scalar = 0; scalar < scalarsPerField(); ++scalar)
			{
				unsigned i = (field*scalarsPerField()) + scalar;
				if(_data[i] < mins[field])
					mins[field] = _data[i];
				if(_data[i] > maxs[field])
					maxs[field] = _data[i];
			}
		}
	}

	bool Timestep::matchingFormat(const Timestep& other) const
	{
		return other.xSize() == _xSize &&
				other.ySize() == _ySize &&
				other.zSize() == _zSize &&
				other.numScalarFields() == _numScalarFields &&
				other.scalarFieldNames() == _scalarFieldNames;
	}

	bool Timestep::empty() const
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

	const std::vector<float>& Timestep::data() const
	{
		return _data;
	}

	std::vector<float>& Timestep::data()
	{
		return _data;
	}

	const float* Timestep::scalarFieldStart(unsigned index) const
	{
		if(index >= _numScalarFields)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Index out of range. #Fields:"
							   << _numScalarFields << " Index:" << index << std::endl;
			throw std::runtime_error("Scalar Field Start Error");
			// TODO:ERROR handling. Index is out of bounds.
		}
		return &_data[index * scalarsPerField()];
	}

	const std::vector<std::string>& Timestep::scalarFieldNames() const
	{
		return _scalarFieldNames;
	}
}
