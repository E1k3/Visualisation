#include "timestep.h"

#include <sstream>
#include <algorithm>
#include <cmath>
#include <random>

#include "logger.h"
#include "math_util.h"

namespace vis
{
	bool Timestep::ScalarField::same_dimensions(const Timestep::ScalarField& other) const
	{
		return other._width == _width
				&& other._height == _height
				&& other._depth == _depth;
	}

	float Timestep::ScalarField::minimum() const
	{
		return *std::min_element(_data.begin(), _data.end());
	}

	float Timestep::ScalarField::maximum() const
	{
		return *std::max_element(_data.begin(), _data.end());
	}

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

		// Create fields of identical format twice, one for average, one for variance.
		newstep._fields.reserve(front.num_fields()*2);
		for(const auto& field : front._fields)
		{
			newstep._fields.push_back(ScalarField(field._width, field._height, field._depth));
			newstep._fields.back()._name = field._name + "_average"s;

		}
		for(const auto& field : front._fields)
		{
			newstep._fields.push_back(ScalarField(field._width, field._height, field._depth));
			newstep._fields.back()._name = field._name + "_variance"s;
		}

		for(unsigned fi = 0; fi < front.num_fields(); ++fi)
		{
			// Index of the variance field
			unsigned fi_var = front.num_fields() + fi;
			for(unsigned i = 0; i < front._fields[fi].volume(); ++i)
			{
				for(const auto& curstep : ensemble)
					newstep._fields[fi]._data[i] += curstep._fields[fi]._data[i];	// Sum
				newstep._fields[fi]._data[i] /= ensemble.size();	// Average

				for(const auto& curstep : ensemble)
					newstep._fields[fi_var]._data[i] += std::pow(newstep._fields[fi]._data[i] - curstep._fields[fi]._data[i], 2);	// Sum of squared difference from the average
				newstep._fields[fi_var]._data[i] /= ensemble.size();	// Variance
			}
		}

		return newstep;
	}

	Timestep Timestep::gaussianMixtureAnalysis(const std::vector<Timestep>& ensemble, unsigned max_components)
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

		// Create fields of identical format three times, for average, variance and mixture weight.
		newstep._fields.reserve(front.num_fields()*3);
		for(const auto& field : front._fields)
		{
			newstep._fields.push_back(ScalarField(field._width, field._height, max_components));
			newstep._fields.back()._name = field._name + "_average";
		}
		for(const auto& field : front._fields)
		{
			newstep._fields.push_back(ScalarField(field._width, field._height, max_components));
			newstep._fields.back()._name = field._name + "_variance";
		}
		for(const auto& field : front._fields)
		{
			newstep._fields.push_back(ScalarField(field._width, field._height, max_components));
			newstep._fields.back()._name = field._name + "_weight";
		}

//		auto re = std::default_random_engine{std::random_device{}()};
//		auto random = std::uniform_real_distribution<float>{0, 1};
		for(unsigned f = 0; f < front.num_fields(); ++f)
		{
			for(unsigned i = 0; i < newstep._fields[f].area(); ++i)
			{
				// Collect samples
				auto samples = std::vector<float>{};
				samples.reserve(ensemble.size());
				for(const auto& sample : ensemble)
					samples.push_back(sample.fields()[f]._data[i]);

				// Determine the mode count
				auto mode_count = math_util::count_modes(samples);

				// If modecount <= 1 -> gaussian approximation


				// If modecount > 1 -> GMM approximation (using EM)
			}
		}

//		for(unsigned fi = 0; fi < front.num_fields(); ++fi)
//		{
//			// unsigned fi_var = front.num_fields() + fi;
//			// unsigned fi_wei = 2 * front.num_fields() + fi;

//			for(unsigned i = 0; i < front._fields[fi]._width*front._fields[fi]._height; ++i)
//			{
//				// EM
//				// Random init
//				newstep._fields[fi]._data[i] = random(re) * (newstep._fields[fi].maximum() - newstep._fields[fi].minimum()) + newstep._fields[fi].minimum();
//				// TODO:calc GMM using EM
//				// store subsequent averages and variances in points with higher depth.
//			}
//		}

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
			linestream >> field._name;

		// Skip vector fields header
		instream.ignore(1024, '\n');	// This line should not be longer than 1024 characters. Magic numbers ftw!

		// Read field data
		for(auto& field : _fields)
		{
			for(unsigned i = 0; i < field.volume(); ++i)
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
		return std::equal(_fields.begin(), _fields.end(), other.fields().begin(), other.fields().end(),
						  [] (const auto& a, const auto& b) {return a.same_dimensions(b);});
	}

	unsigned Timestep::num_fields() const
	{
		return static_cast<unsigned>(_fields.size());
	}

	bool Timestep::empty() const
	{
		return std::all_of(_fields.begin(), _fields.end(),
						   [] (const auto& field) { return field.volume() == 0; });
	}
}
