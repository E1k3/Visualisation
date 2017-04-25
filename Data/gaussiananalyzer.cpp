#include "gaussiananalyzer.h"

#include <algorithm>
#include <cmath>

#include "timestep.h"

namespace vis
{
	GaussianAnalyzer::GaussianAnalyzer()
	{

	}

	GaussianAnalyzer::~GaussianAnalyzer()
	{

	}

	std::unique_ptr<Timestep> GaussianAnalyzer::analyze(std::vector<Timestep>& tsteps)
	{
		using namespace std::literals::string_literals;

		//Validate tsteps to have elements of the same format
		if(tsteps.empty() ||
				std::adjacent_find(tsteps.begin(), tsteps.end(),
								   [] (Timestep& a, Timestep& b) {return !a.matchingFormat(b);} )
				!= tsteps.end())
		{
			return std::unique_ptr<Timestep>{nullptr};
			//TODO:ERROR handling no timesteps or they do not have the same dimensions
		}

		auto step = std::make_unique<Timestep>();

		//Set format to the same as the timesteps in tsteps
		step->_xSize = tsteps.front().xSize();
		step->_ySize = tsteps.front().ySize();
		step->_zSize = tsteps.front().zSize();
		//Double the number of scalar fields to store average and variance
		step->_numScalarFields = 2 * tsteps.front().numScalarFields();

		//Copy scalar field names twice (for average and variance)
		step->_scalarFieldNames.reserve(step->_numScalarFields);
		for(auto& name : tsteps.front().scalarFieldNames())
			step->_scalarFieldNames.push_back(name + "_avg"s);
		for(auto& name : tsteps.front().scalarFieldNames())
			step->_scalarFieldNames.push_back(name + "_var"s);


		step->_data.resize(step->totalPoints(), 0.0f);
		step->_scalarFieldMin.reserve(step->_numScalarFields);
		step->_scalarFieldMax.reserve(step->_numScalarFields);

		for(unsigned i = 0; i < step->totalPoints(); ++i)
		{
			//Calculate sum
			if(i < step->totalPoints()/2)
			{
				for(auto& curstep : tsteps)
					step->_data[i] += curstep.data()[i];
			}
			//Calculate sum of distance
			else
			{
				for(auto& curstep : tsteps)
					step->_data[i] += std::fabs(step->_data[i - step->totalPoints()/2] - curstep.data()[i - step->totalPoints()/2]);
			}

			//Average
			step->_data[i] /= tsteps.size();

			//Calculate min and max per scalar field for normalization
			unsigned field = i/step->scalarsPerField();
			if(step->_data[i] < step->_scalarFieldMin[field])
			{
				step->_scalarFieldMin[field] = step->_data[i];
			}
			if(step->_data[i] > step->_scalarFieldMax[field])
			{
				step->_scalarFieldMax[field] = step->_data[i];
			}
		}

		return std::move(step);
	}
}
