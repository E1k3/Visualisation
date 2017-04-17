#include "timestep.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace vis
{
	Timestep::Timestep()
	{

	}

	Timestep::Timestep(std::istream& instream)
	{
		using namespace std::literals::string_literals;

		auto line = ""s;
		auto linestream = std::stringstream{};

		//Read dimensions header
		std::getline(instream, line, '\n');
		linestream.str(line);
		linestream >> _xSize >> _ySize >> _zSize;

		//Read scalar field header
		std::getline(instream, line, '\n');
		linestream.str(line);
		linestream >> _numScalarFields;

		_scalarFieldNames.resize(_numScalarFields);

		for(unsigned i = 0; i < _numScalarFields; ++i)
		{
			linestream >> _scalarFieldNames[i];
		}

		//Skip vector fields header
		instream.ignore(256, '\n'); //This line should not be longer than 256 characters.

		_scalarFieldMin.resize(_numScalarFields, std::numeric_limits<float>::infinity());
		_scalarFieldMax.resize(_numScalarFields, -std::numeric_limits<float>::infinity());
		_data.resize(scalarsPerField()*_numScalarFields);

		//Read data
		for(unsigned i = 0; i < scalarsPerField()*_numScalarFields; ++i)
		{
			//Using instream >> _data[i] takes about twice as long.
			std::getline(instream, line, ' ');
			_data[i] = std::stof(line);

			unsigned field = i/scalarsPerField();

			if(_data[i] < _scalarFieldMin[field])
			{
				_scalarFieldMin[field] = _data[i];
			}
			if(_data[i] > _scalarFieldMax[field])
			{
				_scalarFieldMax[field] = _data[i];
			}
		}
	}


	Timestep::Timestep(std::vector<Timestep>& tsteps)
	{
		using namespace std::literals::string_literals;

		//Validate tsteps to have elements of the same format
		if(tsteps.empty() ||
				std::adjacent_find(tsteps.begin(), tsteps.end(),
								   [] (Timestep& a, Timestep& b) {return !a.matchingFormat(b);} )
				!= tsteps.end())
		{
			return;
			//TODO:ERROR no timesteps or they do not have the same dimensions
		}

		//Set format to the same as the timesteps in tsteps
		_xSize = tsteps.front().xSize();
		_ySize = tsteps.front().xSize();
		_zSize = tsteps.front().xSize();
		//Double the number of scalar fields to store average and variance
		_numScalarFields = 2 * tsteps.front().numScalarFields();

		//Copy scalar field names twice (for average and variance)
		_scalarFieldNames.reserve(_numScalarFields);
		for(auto& name : tsteps.front().scalarFieldNames())
			_scalarFieldNames.push_back(name + "_avg"s);
		for(auto& name : tsteps.front().scalarFieldNames())
			_scalarFieldNames.push_back(name + "_var"s);


		_data.resize(totalPoints(), 0.0f);
		_scalarFieldMin.reserve(_numScalarFields);
		_scalarFieldMax.reserve(_numScalarFields);


		for(unsigned i = 0; i < totalPoints(); ++i)
		{
			//Calculate sum
			if(i < totalPoints()/2)
			{
				for(auto& step : tsteps)
					_data[i] += step.data()[i];
			}
			//Calculate sum of distance
			else
			{
				for(auto& step : tsteps)
					_data[i] += std::fabs(_data[i - totalPoints()/2] -
								step.data()[i - totalPoints()/2]);
			}

			//Average
			_data[i] /= tsteps.size();

			//Calculate min and max per scalar field for normalization
			unsigned field = i/scalarsPerField();
			if(_data[i] < _scalarFieldMin[field])
			{
				_scalarFieldMin[field] = _data[i];
			}
			if(_data[i] > _scalarFieldMax[field])
			{
				_scalarFieldMax[field] = _data[i];
			}
		}
	}



	bool Timestep::matchingFormat(Timestep& other)
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

	unsigned Timestep::xSize()
	{
		return _xSize;
	}

	unsigned Timestep::ySize()
	{
		return _ySize;
	}

	unsigned Timestep::zSize()
	{
		return _zSize;
	}

	unsigned Timestep::scalarsPerField()
	{
		return _xSize*_ySize*_zSize;
	}

	unsigned Timestep::numScalarFields()
	{
		return _numScalarFields;
	}

	unsigned Timestep::totalPoints()
	{
		return _numScalarFields * scalarsPerField();
	}

	const std::vector<float>& Timestep::data()
	{
		return _data;
	}

	std::vector<std::string> Timestep::scalarFieldNames()
	{
		return _scalarFieldNames;
	}
}
