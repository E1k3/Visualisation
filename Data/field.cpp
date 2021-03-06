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
			Logger::error() << "Field created with negative dimensions\n"
							<< "dimension: " << point_dimension
							<< " width: " << width
							<< " height: " << height
							<< " depth: " << depth;
			throw std::length_error("Negative dimensions for field creation");
		}
		if(size() == 0)
			Logger::warning() << "Field created with size 0.";

		if(init)
			initialize();
	}

	Field::Field(const Field& layout, bool init)
		: _dimension{layout._dimension},
		  _width{layout._width},
		  _height{layout._height},
		  _depth{layout._depth},
		  _name{layout.name()},
		  _data{}
	{
		if(init)
			initialize();
	}

	Field Field::operator+(Field other) const
	{
		if(!equal_layout(other))
		{
			Logger::error() << "Fields with different layouts cannot be added.";
			throw std::runtime_error("Addition of Fields with different layout");
		}
		for(int d = 0; d < _dimension; ++d)
			for(int i = 0; i < volume(); ++i)
				other.set_value(d, i, get_value(d, i) + other.get_value(d, i));
		return other;
	}

	Field Field::operator-(Field other) const
	{
		if(!equal_layout(other))
		{
			Logger::error() << "Fields with different layouts cannot be added.";
			throw std::runtime_error("Addition of Fields with different layout");
		}
		for(int d = 0; d < _dimension; ++d)
			for(int i = 0; i < volume(); ++i)
				other.set_value(d, i, get_value(d, i) - other.get_value(d, i));
		return other;
	}

	bool Field::initialized() const               { return _initialized; }

	void Field::initialize()
	{
		if(!_initialized)
			_data.resize(static_cast<size_t>(size()));	// Class invariant, has to be >= 0
		_initialized = true;
	}

	int Field::size() const                       { return volume()*_dimension; }

	int Field::volume() const                     { return area()*_depth; }

	int Field::area() const                       { return _width*_height; }

	int Field::point_dimension() const            { return _dimension; }

	int Field::width() const                      { return _width; }

	int Field::height() const                     { return _height; }

	int Field::depth() const                      { return _depth; }

	float Field::aspect_ratio() const             { return static_cast<float>(_width) / _height; }

	const std::string& Field::name() const        { return _name; }

	void Field::set_name(const std::string& name) { _name = name; }

	float Field::minimum(std::function<bool(float, float)> comp) const
	{
		if(!_initialized)
		{
			Logger::error() << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// ERROR handling. Field not initialized.
		}
		return *std::min_element(_data.begin(), _data.end(), comp);
	}

	float Field::maximum(std::function<bool(float, float)> comp) const
	{
		if(!_initialized)
		{
			Logger::error() << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// ERROR handling. Field not initialized.
		}
		return *std::max_element(_data.begin(), _data.end(), comp);
	}

	std::vector<float> Field::minima(std::function<bool(float, float)> comp) const
	{
		auto minima = std::vector<float>(static_cast<size_t>(_dimension), std::numeric_limits<float>::infinity());
		for(int d = 0; d < _dimension; ++d)
			for(int i = 0; i < volume(); ++i)
				minima[static_cast<size_t>(d)] = std::min(minima[static_cast<size_t>(d)], get_value(d, i), comp);
		return minima;
	}

	std::vector<float> Field::maxima(std::function<bool(float, float)> comp) const
	{
		auto maxima = std::vector<float>(static_cast<size_t>(_dimension), -std::numeric_limits<float>::infinity());
		for(int d = 0; d < _dimension; ++d)
			for(int i = 0; i < volume(); ++i)
				maxima[static_cast<size_t>(d)] = std::max(maxima[static_cast<size_t>(d)], get_value(d, i), comp);
		return maxima;
	}

	float Field::partial_minimum(int x1, int y1, int z1, int x2, int y2, int z2, std::function<bool(float, float)> comp) const
	{
		if(x1 > x2 || y1 > y2 || z1 > z2)
		{
			Logger::error() << "Volume boundaries have to be LL to UR.";
			throw std::runtime_error("Volume boundaries in reverse order");
		}
		float minimum = std::numeric_limits<float>::infinity();
		for(int d = 0; d < _dimension; ++d)
			for(int x = x1; x <= x2; ++x)
				for(int y = y1; y <= y2; ++y)
					for(int z = z1; z <= z2; ++z)
						minimum = std::min(minimum, get_value(d, x, y, z), comp);
		return minimum;
	}

	float Field::partial_maximum(int x1, int y1, int z1, int x2, int y2, int z2, std::function<bool(float, float)> comp) const
	{
		if(x1 > x2 || y1 > y2 || z1 > z2)
		{
			Logger::error() << "Volume boundaries have to be LL to UR.";
			throw std::runtime_error("Volume boundaries in reverse order");
		}
		float maximum = -std::numeric_limits<float>::infinity();
		for(int d = 0; d < _dimension; ++d)
			for(int x = x1; x <= x2; ++x)
				for(int y = y1; y <= y2; ++y)
					for(int z = z1; z <= z2; ++z)
						maximum = std::max(maximum, get_value(d, x, y, z), comp);
		return maximum;
	}

	std::vector<float> Field::partial_minima(int x1, int y1, int z1, int x2, int y2, int z2, std::function<bool(float, float)> comp) const
	{
		if(x1 > x2 || y1 > y2 || z1 > z2)
		{
			Logger::error() << "Volume boundaries have to be LL to UR.";
			throw std::runtime_error("Volume boundaries in reverse order");
		}
		auto minima = std::vector<float>(static_cast<size_t>(_dimension), std::numeric_limits<float>::infinity());
		for(int d = 0; d < _dimension; ++d)
			for(int x = x1; x <= x2; ++x)
				for(int y = y1; y <= y2; ++y)
					for(int z = z1; z <= z2; ++z)
						minima[static_cast<size_t>(d)] = std::min(minima[static_cast<size_t>(d)], get_value(d, x, y, z), comp);
		return minima;
	}

	std::vector<float> Field::partial_maxima(int x1, int y1, int z1, int x2, int y2, int z2, std::function<bool(float, float)> comp) const
	{
		if(x1 > x2 || y1 > y2 || z1 > z2)
		{
			Logger::error() << "Volume boundaries have to be LL to UR.";
			throw std::runtime_error("Volume boundaries in reverse order");
		}
		auto maxima = std::vector<float>(static_cast<size_t>(_dimension), -std::numeric_limits<float>::infinity());
		for(int d = 0; d < _dimension; ++d)
			for(int x = x1; x <= x2; ++x)
				for(int y = y1; y <= y2; ++y)
					for(int z = z1; z <= z2; ++z)
						maxima[static_cast<size_t>(d)] = std::max(maxima[static_cast<size_t>(d)], get_value(d, x, y, z), comp);
		return maxima;
	}

	bool Field::equal_layout(const Field& other) const
	{
		return _dimension == other._dimension
				&& _width == other._width
				&& _height == other._height
				&& _depth == other._depth;
	}

	std::string Field::layout_to_string() const
	{
		return "Name: " + _name
				+ " point dimension: " + std::to_string(_dimension)
				+ " width: " + std::to_string(_width)
				+ " height: " + std::to_string(_height)
				+ " depth: " + std::to_string(_depth);
	}

	const std::vector<float>& Field::data() const
	{
		return _data;
	}

	std::vector<float> Field::get_point(int i) const
	{
		if(!_initialized)
		{
			Logger::error() << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// ERROR handling. Field not initialized.
		}

		auto point = std::vector<float>(static_cast<size_t>(_dimension));
		std::copy_n(_data.begin() + validate_index(i), _dimension, point.begin());
		return point;
	}

	std::vector<float> Field::get_point(int x, int y, int z) const
	{
		if(!_initialized)
		{
			Logger::error() << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// ERROR handling. Field not initialized.
		}

		auto point = std::vector<float>(static_cast<size_t>(_dimension));
		std::copy_n(_data.begin() + validate_index(x, y, z), _dimension, point.begin());
		return point;
	}

	float Field::get_value(int d, int i) const
	{
		if(!_initialized)
		{
			Logger::error() << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// ERROR handling. Field not initialized.
		}

		return _data[static_cast<size_t>(validate_index(d, i))];
	}

	float Field::get_value(int d, int x, int y, int z) const
	{
		if(!_initialized)
		{
			Logger::error() << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// ERROR handling. Field not initialized.
		}

		return _data[static_cast<size_t>(validate_index(d, x, y, z))];
	}

	void Field::set_point(int i, std::vector<float> point)
	{
		if(!_initialized)
		{
			Logger::error() << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// ERROR handling. Field not initialized.
		}

		point.resize(static_cast<size_t>(_dimension));
		std::copy(point.begin(), point.end(), _data.begin() + validate_index(i));
	}

	void Field::set_point(int x, int y, int z, std::vector<float> point)
	{
		if(!_initialized)
		{
			Logger::error() << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// ERROR handling. Field not initialized.
		}

		point.resize(static_cast<size_t>(_dimension));
		std::copy(point.begin(), point.end(), _data.begin() + validate_index(x, y, z));
	}

	void Field::set_value(int d, int i, float value)
	{
		if(!_initialized)
		{
			Logger::error() << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// ERROR handling. Field not initialized.
		}

		_data[static_cast<size_t>(validate_index(d, i))] = value;
	}

	void Field::set_value(int d, int x, int y, int z, float value)
	{
		if(!_initialized)
		{
			Logger::error() << "Data access on uninitialized field.";
			throw std::runtime_error("Field data accessed before initializing");	// ERROR handling. Field not initialized.
		}

		_data[static_cast<size_t>(validate_index(d, x, y, z))] = value;
	}

	int Field::validate_index(int i) const
	{
		if(i < 0 || i > volume())
		{
			Logger::error() << "Data of field " << _name << " was accessed at index:\n"
							<< "i: " << i
							<< "\nField volume:\n"
							<< "volume: " << volume();
			throw std::length_error("Field data access out of range.");	// ERROR handling. Negative index.
		}
		return i*_dimension;
	}

	int Field::validate_index(int d, int i) const
	{
		if(d < 0 || d >= _dimension)
		{
			Logger::error() << "Data of field " << _name << " was accessed at point dimension:\n"
							<< "d: " << d
							<< "\nField point dimension:\n"
							<< "dimension: " << _dimension ;
			throw std::length_error("Field data access out of range.");	// ERROR handling. Negative index.
		}
		return validate_index(i) + d;
	}

	int Field::validate_index(int x, int y, int z) const
	{
		if(x < 0 || x >= _width || y < 0 || y >= _height || z < 0 || z >= _depth)
		{
			Logger::error() << "Data of field " << _name << " was accessed at indices:\n"
							<< "x: " << x << " y: " << y << " z: " << z
							<< "\nField dimensions:\n"
							<< "width: " << _width << " height: " << _height << " depth: " << _depth;
			throw std::length_error("Field data access out of range.");	// ERROR handling. Negative index.
		}
		return (z*area() + y*_width + x) * _dimension;
	}

	int Field::validate_index(int d, int x, int y, int z) const
	{
		if(d < 0 || d >= _dimension)
		{
			Logger::error() << "Data of field " << _name << " was accessed at point dimension:\n"
							<< "d: " << d
							<< "\nField point dimension:\n"
							<< "dimension: " << _dimension ;
			throw std::length_error("Field data access out of range.");	// ERROR handling. Negative index.
		}
		return validate_index(x, y, z) + d;
	}
}
