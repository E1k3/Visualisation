#ifndef FIELD_H
#define FIELD_H

#include <vector>
#include <string>

namespace vis
{
	class Field
	{
	public:
		/**
		 * @brief Field Constructs a field if size width*height*depth with point_dimension values for each point.
		 * @param point_dimension The number of values each point contains.
		 * @param width The width of the field.
		 * @param height The height of the field.
		 * @param depth The depth of the field.
		 * @param init If true, initializes the fields data with zero. If false, does not allocate any memory for its data.
		 */
		explicit Field(int point_dimension, int width, int height, int depth = 1, bool init = false);

		/**
		 * @brief Field Creates a field with the same layout and name as another one.
		 * @param layout The field that provides the layout.
		 * @param init If true, initializes the fields data with zero. If false, does not allocate any memory for its data.
		 */
		explicit Field(const Field& layout, bool init);

		/// @brief Returns a field of the same layout containing the sum of this and others values.
		/// Fails if this and other don't have a matching layout.
		Field operator+(Field other) const;
		/// @brief Returns a field of the same layout containing the subtraction of others values from this'.
		/// Fails if this and other don't have a matching layout.
		Field operator-(Field other) const;

		/// @brief Returns true if the fields data is initialized.
		bool initialized() const;
		/// @brief Allocates memory and initializes every data point with 0.0f.
		/// Does nothing if it is already initialized.
		void initialize();

		/// @brief Returns the number of floats this field holds.
		int size() const;
		/// @brief Returns the number of points this field holds.
		int volume() const;
		/// @brief Returns the number of points this field holds in one of its layers.
		int area() const;
		/// @brief Returns the number of values each point holds.
		int point_dimension() const;
		/// @brief Returns the number of columns this field holds in one of its layers.
		int width() const;
		/// @brief Returns the number of rows this field holds in one of its layers.
		int height() const;
		/// @brief Returns Returns the number of layers this field holds.
		int depth() const;
		/// @brief Returns the fields width/height ratio.
		float aspect_ratio() const;
		/// @brief Returns the fields name.
		const std::string& name() const;
		/// @brief Sets the fields name.
		void set_name(const std::string& name);

		/// @brief Returns the smallest value of this field.
		float minimum() const;
		/// @brief Returns the largest value of this field.
		float maximum() const;
		/// @brief Returns a collection containing the smallest value for each dimension of this field.
		std::vector<float> minima() const;
		/// @brief Returns a collection containing the largest value for each dimension of this field.
		std::vector<float> maxima() const;
		/// @brief Returns the smallest value inside the volume that spans between (x1,y2,z1) and (x2,y2,z2).
		float partial_minimum(int x1, int y1, int z1, int x2, int y2, int z2) const;
		/// @brief Returns the largest value inside the volume that spans between (x1,y2,z1) and (x2,y2,z2).
		float partial_maximum(int x1, int y1, int z1, int x2, int y2, int z2) const;
		/// @brief Returns a collection containing the smallest value for each dimension inside the volume that spans between (x1,y2,z1) and (x2,y2,z2).
		std::vector<float> partial_minima(int x1, int y1, int z1, int x2, int y2, int z2) const;
		/// @brief Returns a collection containing the largest value for each dimension inside the volume that spans between (x1,y2,z1) and (x2,y2,z2).
		std::vector<float> partial_maxima(int x1, int y1, int z1, int x2, int y2, int z2) const;

		/// @brief Returns true if other field has the same layout (dimension, width, ...).
		/// Name and content are ignored.
		bool equal_layout(const Field& other) const;
		/// @brief Returns a human readable string describing the layout and name of this field.
		std::string layout_to_string() const;

		const std::vector<float>& data() const;
		/// @brief get_point Gets all components of the i-th point of the field. Only possible if initialized().
		/// @return A vector containing point_dimension() floats.
		std::vector<float> get_point(int i) const;
		/// @brief get_point Gets all components of the point at (x, y, z) <-(width, height, depth). Only possible if initialized().
		/// @return A vector containing point_dimension() floats.
		std::vector<float> get_point(int x, int y, int z) const;
		/// @brief get_value Gets the d-th component of the i-th point of the field. Only possible if initialized().
		float get_value(int d, int i) const;
		/// @brief get_value Gets the d-th component of the point at (x, y, z) <-(width, height, depth). Only possible if initialized().
		float get_value(int d, int x, int y, int z) const;

		/// @brief set_point Sets all components of the i-th point of the field. Only possible if initialized().
		/// If point.size() is greater than point_dimension(), the excess values will be discarded.
		/// If point.size() is smaller than point_dimension(), the missing values will be 0.f.
		void set_point(int i, std::vector<float> point);
		/// @brief set_point Sets all components of the point at (x, y, z) <-(width, height, depth). Only possible if initialized().
		/// If point.size() is greater than point_dimension(), the excess values will be discarded.
		/// If point.size() is smaller than point_dimension(), the missing values will be 0.f.
		void set_point(int x, int y, int z, std::vector<float> point);
		/// @brief set_value Sets the d-th value of the i-th point of the field. Only possible if initialized().
		void set_value(int d, int i, float value);
		/// @brief set_value Sets the d-th value of the point at (x, y, z) <-(width, height, depth). Only possible if initialized().
		void set_value(int d, int x, int y, int z, float value);

	private:
		int validate_index(int i) const;
		int validate_index(int d, int i) const;
		int validate_index(int x, int y, int z) const;
		int validate_index(int d, int x, int y, int z) const;

		int _dimension{};
		int _width{};
		int _height{};
		int _depth{};

		bool _initialized{false};

		std::string _name{};
		std::vector<float> _data;
	};
}

#endif // FIELD_H
