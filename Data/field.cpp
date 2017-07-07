#include "field.h"

#include <algorithm>

#include "logger.h"

namespace vis
{
	Field::Field(int point_dimension, int width, int height, int depth, bool init)
		: _dimension{point_dimension},
		  _width{width},
		  _height{height},
		  _depth{depth},
		  _data{}
	{
		if(_dimension < 0 || _width < 0 || _height < 0 || _depth < 0)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Field initialized with negative dimensions\n"
							   << "dimension: " << point_dimension
							   << " width: " << width
							   << " height: " << height
							   << " depth: " << depth;
			// TODO:ERROR handling. Dimensions invalid.
			throw std::invalid_argument("Negative dimensions for field creation");
		}

		if(init)
			initialize();
	}

	bool Field::initialized() const               { return _initialized; }

	int Field::size() const                       { return volume()*_dimension; }

	int Field::volume() const                     { return area()*_depth; }

	int Field::area() const                       { return _width*_height; }

	int Field::point_dimension() const            { return _dimension; }

	int Field::width() const                      { return _width; }

	int Field::height() const                     { return _height; }

	int Field::depth() const                      { return _depth; }

	const std::string& Field::name() const        { return _name; }

	void Field::set_name(const std::string& name) { _name = name; }

	std::vector<float> Field::get_point(int i) const
	{
		if(!_initialized)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// TODO:ERROR handling. Field not initialized.
		}

		auto point = std::vector<float>(static_cast<size_t>(_dimension));
		std::copy_n(_data.begin() + validate_index(i), _dimension, point.begin());
		return point;
	}

	std::vector<float> Field::get_point(int x, int y, int z) const
	{
		if(!_initialized)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// TODO:ERROR handling. Field not initialized.
		}

		auto point = std::vector<float>(static_cast<size_t>(_dimension));
		std::copy_n(_data.begin() + validate_index(x, y, z), _dimension, point.begin());
		return point;
	}

	float Field::get_value(int d, int i) const
	{
		if(!_initialized)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// TODO:ERROR handling. Field not initialized.
		}

		return _data[static_cast<size_t>(validate_index(d, i))];
	}

	float Field::get_value(int d, int x, int y, int z) const
	{
		if(!_initialized)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// TODO:ERROR handling. Field not initialized.
		}

		return _data[static_cast<size_t>(validate_index(d, x, y, z))];
	}

	void Field::set_point(int i, std::vector<float> point)
	{
		if(!_initialized)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// TODO:ERROR handling. Field not initialized.
		}

		point.resize(static_cast<size_t>(_dimension));
		std::copy(point.begin(), point.end(), _data.begin() + validate_index(i));
	}

	void Field::set_point(int x, int y, int z, std::vector<float> point)
	{
		if(!_initialized)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// TODO:ERROR handling. Field not initialized.
		}

		point.resize(static_cast<size_t>(_dimension));
		std::copy(point.begin(), point.end(), _data.begin() + validate_index(x, y, z));
	}

	void Field::set_value(int d, int i, float value)
	{
		if(!_initialized)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// TODO:ERROR handling. Field not initialized.
		}

		_data[static_cast<size_t>(validate_index(d, i))] = value;
	}

	void Field::set_value(int d, int x, int y, int z, float value)
	{
		if(!_initialized)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// TODO:ERROR handling. Field not initialized.
		}

		_data[static_cast<size_t>(validate_index(d, x, y, z))] = value;
	}

	void Field::initialize()
	{
		if(!_initialized)
			_data.resize(static_cast<size_t>(size()));
		_initialized = true;
	}

	int Field::validate_index(int i) const
	{
		if(i < 0 || i > volume())
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Data of field " << _name << " was accessed at index:\n"
							   << "i: " << i
							   << "\nField volume:\n"
							   << "volume: " << volume();
			throw std::out_of_range("Field data access out of range.");	// TODO:ERROR handling. Negative index.
		}
		return i*_dimension;
	}

	int Field::validate_index(int d, int i) const
	{
		if(d < 0 || d >= _dimension)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Data of field " << _name << " was accessed at point dimension:\n"
							   << "d: " << d
							   << "\nField point dimension:\n"
							   << "dimension: " << _dimension ;
			throw std::out_of_range("Field data access out of range.");	// TODO:ERROR handling. Negative index.
		}
		return validate_index(i) + d;
	}

	int Field::validate_index(int x, int y, int z) const
	{
		if(x < 0 || x >= _width || y < 0 || y >= _height || z < 0 || z >= _depth)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Data of field " << _name << " was accessed at indices:\n"
							   << "x: " << x << " y: " << y << " z: " << z
							   << "\nField dimensions:\n"
							   << "width: " << _width << " height: " << _height << " depth: " << _depth;
			throw std::out_of_range("Field data access out of range.");	// TODO:ERROR handling. Negative index.
		}
		return (z*area() + y*_width + x) * _dimension;
	}

	int Field::validate_index(int d, int x, int y, int z) const
	{
		return validate_index(d, validate_index(x, y, z));
	}
}
