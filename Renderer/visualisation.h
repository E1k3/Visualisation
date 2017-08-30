#ifndef VISUALISATION_H
#define VISUALISATION_H

#include <vector>
#include <glm/glm.hpp>

#include "globject2.h"
#include "Data/field.h"
#include "inputmanager.h"

namespace vis
{
	class Visualisation
	{
	public:
		explicit Visualisation(InputManager& input, const std::vector<Field>& fields);
		virtual ~Visualisation() = default;

		/**
		 * @brief setup_data Creates buffer(s), uploads data and configures attribute arrays.
		 */
		virtual void setup_data() = 0;
		/**
		 * @brief setup_shaders Creates, compiles shaders, links program(s) and configures uniform(s).
		 */
		virtual void setup_shaders() = 0;
		/**
		 * @brief update Updates visualisation state.
		 * Uses input to alter uniforms, data, whetever is needed.
		 * @param delta_time The time since the last frame/update.
		 * @param total_time The time since application start.
		 */
		virtual void update(float delta_time, float total_time) = 0;
		/**
		 * @brief draw Renders the visualisation at its current state.
		 */
		virtual void draw() const = 0;

	protected:
		// Input manager that is used to access HID data
		InputManager& _input;
		// Collection of data fields (visualisation input)
		const std::vector<Field>& _fields;

		// OpenGL vertex array object
		VertexArray _vao;
		// Collection of OpenGL buffers
		std::vector<Buffer> _vbos;
		// OpenGL shader program
		Program _program;

		// Number of vertices to draw
		int _vertex_count{0};
		// Model-View-Projection matrix
		glm::mat4 _mvp;

		// Point in [(-1,-1), (1,1)] that is currently selected by the cursor
		glm::vec2 _selection_cursor{0.f};
	};
}
#endif // VISUALISATION_H
