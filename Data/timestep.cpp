#include "timestep.h"

#include <sstream>
#include <algorithm>
#include <cmath>
#include <random>

#include "logger.h"

namespace vis
{
	Timestep Timestep::gaussianAnalysis(const std::vector<Timestep>& ensemble)
	{
		using namespace std::literals::string_literals;

		// Validate ensemble to only have elements of the same format
		if(ensemble.empty() ||
				std::adjacent_find(ensemble.begin(), ensemble.end(),
								   [] (const Timestep& a, const Timestep& b) {return !a.same_format(b);} )
				!= ensemble.end())
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Ensemble has elements with differing format" << std::endl;
			throw std::runtime_error("Gaussian Analysis Error");
			// TODO:ERROR handling no timesteps or they do not have the same dimensions
		}

		auto newstep = Timestep();
		const auto& front = ensemble.front();

		// Create fields of identical format twice, one for average, one for deviation.
		newstep._fields.reserve(front.num_fields()*2);
		for(const auto& field : front._fields)
		{
			newstep._fields.push_back(ScalarField(field._width, field._height, field._depth));
			newstep._fields.back()._name = field._name + "_avg"s;

		}
		for(const auto& field : front._fields)
		{
			newstep._fields.push_back(ScalarField(field._width, field._height, field._depth));
			newstep._fields.back()._name = field._name + "_dev"s;
		}

		for(unsigned fi = 0; fi < front.num_fields(); ++fi)
		{
			// Index of the deviation field
			unsigned fi_dev = front.num_fields() + fi;
			for(unsigned i = 0; i < front._fields[fi].num_scalars(); ++i)
			{
				for(const auto& curstep : ensemble)
					newstep._fields[fi]._data[i] += curstep._fields[fi]._data[i];	// Sum
				newstep._fields[fi]._data[i] /= ensemble.size();	// Average

				newstep._fields[fi]._minimum = std::min(newstep._fields[fi]._data[i], newstep._fields[fi]._minimum);
				newstep._fields[fi]._maximum = std::max(newstep._fields[fi]._data[i], newstep._fields[fi]._maximum);

				for(const auto& curstep : ensemble)
					newstep._fields[fi_dev]._data[i] += std::fabs(newstep._fields[fi]._data[i] - curstep._fields[fi]._data[i]);	// Sum of absolute difference from the average
				newstep._fields[fi_dev]._data[i] /= ensemble.size();	// Deviation

				newstep._fields[fi_dev]._minimum = std::min(newstep._fields[fi_dev]._data[i], newstep._fields[fi_dev]._minimum);
				newstep._fields[fi_dev]._maximum = std::max(newstep._fields[fi_dev]._data[i], newstep._fields[fi_dev]._maximum);
			}
		}

		return newstep;
	}

	Timestep Timestep::gaussianMixtureAnalysis(const std::vector<Timestep>& ensemble, unsigned num_components)
	{
		// Validate ensemble to only have elements of the same format
		if(ensemble.empty() ||
				std::adjacent_find(ensemble.begin(), ensemble.end(),
								   [] (const Timestep& a, const Timestep& b) {return !a.same_format(b);} )
				!= ensemble.end())
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Ensemble has elements with differing format" << std::endl;
			throw std::runtime_error("GMM Analysis Error");
			// TODO:ERROR handling no timesteps or they do not have the same dimensions
		}

		auto newstep = Timestep();
		const auto& front = ensemble.front();

		// Create fields of identical format three times, for average, deviation and mixture weight.
		newstep._fields.reserve(front.num_fields()*3);
		for(const auto& field : front._fields)
		{
			newstep._fields.push_back(ScalarField(field._width, field._height, num_components));
			newstep._fields.back()._name = field._name + "_average";
		}
		for(const auto& field : front._fields)
		{
			newstep._fields.push_back(ScalarField(field._width, field._height, num_components));
			newstep._fields.back()._name = field._name + "_deviation";
		}
		for(const auto& field : front._fields)
		{
			newstep._fields.push_back(ScalarField(field._width, field._height, num_components));
			newstep._fields.back()._name = field._name + "_weight";
		}

		auto re = std::default_random_engine{std::random_device{}()};
		auto random = std::uniform_real_distribution<float>{0, 1};

		for(unsigned fi = 0; fi < front.num_fields(); ++fi)
		{
//			unsigned fi_dev = front.num_fields() + fi;
//			unsigned fi_wei = 2 * front.num_fields() + fi;

			for(unsigned i = 0; i < front._fields[fi]._width*front._fields[fi]._height; ++i)
			{
				// EM
				// Random init
				newstep._fields[fi]._data[i] = random(re) * (newstep._fields[fi]._maximum - newstep._fields[fi]._minimum) + newstep._fields[fi]._minimum;
						// TODO:calc GMM using EM
						// store subsequent averages and deviations in points with higher depth.

						//				newstep._fields[fi]._minimum = std::min(newstep._fields[fi]._data[i], newstep._fields[fi]._minimum);
						//				newstep._fields[fi]._maximum = std::max(newstep._fields[fi]._data[i], newstep._fields[fi]._maximum);
			}
		}

		return newstep;
	}

	Timestep::Timestep(std::istream& instream)
	{
		using namespace std::literals::string_literals;

		auto line = ""s;
		auto linestream = std::stringstream{};

		// Read dimensions header
		std::getline(instream, line, '\n');
		linestream.str(line);
		unsigned width, height, depth;
		linestream >> width >> height >> depth;

		// Read scalar field header
		std::getline(instream, line, '\n');
		linestream.str(line);
		unsigned num_fields;
		linestream >> num_fields;
		_fields.resize(num_fields, ScalarField(width, height, depth));

		for(auto& field : _fields)
		{
			linestream >> field._name;
		}

		// Skip vector fields header
		instream.ignore(1024, '\n'); // This line should not be longer than 1024 characters.

		// Read field data
		for(auto& field : _fields)
		{
			for(unsigned i = 0; i < field.num_scalars(); ++i)
			{
				// Using instream >> _data[i] takes about twice as long.
				std::getline(instream, line, ' ');
				field._data[i] = std::stof(line);
			}
		}
	}

	const std::vector<Timestep::ScalarField>& Timestep::fields() const
	{
		return _fields;
	}

	bool Timestep::same_format(const Timestep& other) const
	{
		if(other.num_fields() != num_fields())
			return false;

		for(unsigned i = 0; i < num_fields(); ++i)
		{
			if(!_fields[i].same_dimensions(other.fields()[i]))
				return false;
		}

		return true;
	}

	unsigned Timestep::num_fields() const
	{
		return static_cast<unsigned>(_fields.size());
	}

	bool Timestep::empty() const
	{
		unsigned num_points = 0;
		for(auto& field : _fields)
			num_points += field.num_scalars();
		return num_points == 0;
	}

	float Timestep::normal_density(float x, float mean, float variance)
	{
		const float pi = std::acos(-1.f);
		return 1 / (std::sqrt( 2 * pi * variance)) * std::exp(-std::pow(x - mean, 2.f) / (2 * variance));
	}

	void Timestep::expectation_maximization(const std::vector<float>& samples, std::vector<Timestep::GMMComponent>& components)
	{
		auto sample_weights = std::vector<float>{};
		sample_weights.reserve(samples.size() * components.size());

		// E-step
		sample_weights.clear();
		for(const auto& samp : samples)
		{
			auto i = sample_weights.size();
			float total_density = 0.f;
			// Fill sample_weights with the density of the current GMM at samp
			for(auto& comp : components)
				total_density += normal_density(samp, comp._mean, comp._variance) * comp._weight;
			sample_weights.resize(sample_weights.size() + components.size(), total_density);

			for(const auto& comp : components)
			{
				// sample_weight_i = COMPONENTdensity(sample)*comp.weight / GMMdensity(sample)
				sample_weights[i] = normal_density(samp, comp._mean, comp._variance) * comp._weight / sample_weights[i];
				++i;
			}
		}

		// M-step
		for(unsigned i = 0; i < components.size(); ++i)
		{

		}
	}
}
