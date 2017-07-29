#ifndef HEIGHTFIELDRENDERER_H
#define HEIGHTFIELDRENDERER_H

#include <algorithm>
#include <string>
#include <chrono>

#include <glm/glm.hpp>

#include "renderer.h"
#include "gradientrenderer.h"
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

		virtual void draw(float delta_time, float total_time) override;

	private:
		void init_gaussian(const std::vector<Field>& fields);
		void init_gmm(const std::vector<Field>&);

		InputManager& _input;

		GradientRenderer _palette;


		GLuint _vao{0};
		GLuint _program{0};

		int _num_vertices{};
		GLint _mvp_uniform{};
		GLint _bounds_uniform{};
		GLint _time_uniform{};

		glm::vec4 _bounds{};
		glm::vec3 _cam_position{-1.8f, -1.8f, 1.8f};
	};
}

#endif // HEIGHTFIELDRENDERER_H
