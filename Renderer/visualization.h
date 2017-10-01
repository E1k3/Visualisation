#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <vector>
#include <glm/glm.hpp>

#include "globject.h"
#include "Data/field.h"
#include "inputmanager.h"
#include "colormap.h"
#include "primitives.h"

namespace vis
{
	/**
	 * @brief The Visualization class is a pure virtual base class for Visualizations.
	 */
	class Visualization
	{
	public:
		explicit Visualization(InputManager& input, const std::vector<Field>& fields);
		virtual ~Visualization() = default;

		virtual void setup();

		/**
		 * @brief setup_data Creates buffer(s), uploads data and configures attribute arrays.
		 */
		virtual void setup_data() = 0;
		/**
		 * @brief setup_shaders Creates and compiles shaders, links program(s) and configures uniform(s).
		 */
		virtual void setup_shaders() = 0;

		/**
		 * @brief update Updates visualization state.
		 * Uses input to alter uniforms, data, whetever is needed.
		 * @param delta_time The time since the last frame/update.
		 * @param total_time The time since application start.
		 */
		virtual void update(float delta_time, float total_time) = 0;
		/**
		 * @brief draw Renders the visualization at its current state.
		 */
		virtual void draw() const = 0;

		virtual glm::vec2 get_cursor() const;

		virtual glm::ivec2 point_under_cursor() const = 0;

		virtual void set_highlight_area(const glm::ivec4& area);

	protected:
		/// @brief update_selection_cursor Updates the cursor position.
		/// Uses the model-view matrix to calculate view direction.
		void update_selection_cursor(glm::vec2 mouse_offset, glm::mat4 modelview, float aspect_ratio, float scale);

		// Input manager that is used to access HID data
		InputManager& _input;
		// Collection of data fields (visualization input)
		const std::vector<Field>& _fields;
		// Renders the color map with divisions
		Colormap _palette;

		// OpenGL vertex array object
		VertexArray _vao;
		// Collection of OpenGL buffers
		std::vector<Buffer> _buffers;
		// Texture
		Texture _texture;
		// OpenGL shader program
		Program _program;

		// Number of vertices to draw
		int _vertex_count{0};

		glm::ivec4 _highlight_area{10};
		// Point in [(-1,-1), (1,1)] that is currently selected by the cursor
		glm::vec2 _cursor_position{0.f};
		// Location of a vec4 uniform containing the LL and UR coordinates of a highlighted rectangle
		// The vec4 looks like this (x1, y1, x2, y1) with (x1, y1) being LL and (x2, y2) being UR
		GLint _highlight_loc{-1};
	};
}
#endif // VISUALIZATION_H
