#ifndef HEIGHTFIELDRENDERER_H
#define HEIGHTFIELDRENDERER_H

#include <algorithm>
#include <string>
#include <chrono>

#include <glm/glm.hpp>

#include "renderer.h"
#include "Data/field.h"
#include "inputmanager.h"

namespace vis
{
	class HeightfieldRenderer : public Renderer
	{
	public:
		/// @brief HeightfieldRenderer Constructor.
		explicit HeightfieldRenderer(const std::vector<Field>& fields, InputManager& input);
		virtual ~HeightfieldRenderer() = default;

		virtual void draw(float delta_time) override;

	private:
		InputManager& _input;

		unsigned _num_vertices;
		GLint _mvp_uniform;
		GLint _bounds_uniform;

		glm::vec3 _cam_position{-1.8f, -1.8f, 1.8f};
		glm::vec3 _cam_direction{-_cam_position};
	};
}

#endif // HEIGHTFIELDRENDERER_H
