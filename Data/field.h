#ifndef FIELD_H
#define FIELD_H

#include <vector>
#include <string>

namespace vis
{
	/**
	 * @brief The Field class TODO:this
	 */
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

		/// @brief Returns true if the fields data is initialized
		bool initialized() const;
		/// @brief Initializes the fields data with zero.
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
		/// @brief Returns the fields name.
		const std::string& name() const;
		/// @brief Sets the fields name.
		void set_name(const std::string& name);

		float minimum() const;
		float maximum() const;
		std::vector<float> minima() const;
		std::vector<float> maxima() const;

		/// @brief Returns true if other field has the same layout (dimension, width, ...).
		/// Name and content are ignored.
		bool equal_layout(const Field& other) const;
		/// @brief Returns a human readable string describing the layout and name of this field.
		std::string layout_to_string() const;

//		/// @brief Gives direct non const access to data without range checking.
//		float& operator[](int index);
//		/// @brief Gives direct const access to data without range checking.
//		const float& operator[](int index) const;
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
		/// If point.size() is smaller than point_dimension(), the missing values will be zero.
		void set_point(int i, std::vector<float> point);
		/// @brief set_point Sets all components of the point at (x, y, z) <-(width, height, depth). Only possible if initialized().
		/// If point.size() is greater than point_dimension(), the excess values will be discarded.
		/// If point.size() is smaller than point_dimension(), the missing values will be zero.
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
