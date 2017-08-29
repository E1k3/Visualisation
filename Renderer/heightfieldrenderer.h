#ifndef HEIGHTFIELDRENDERER_H
#define HEIGHTFIELDRENDERER_H

#include <algorithm>
#include <string>
#include <chrono>

#include <glm/glm.hpp>

#include "renderer.h"
#include "textrenderer.h"
#include "colormaprenderer.h"
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
		void init_gaussian();
		void init_gmm();

		void init_scale_planes(int divisions);
		void draw_scale_planes(const glm::mat4& mvp, int count);

		std::vector<Field> _fields;

		InputManager& _input;

		ColormapRenderer _palette;

		TextRenderer _scale_plane_text;
		GLuint _vao{0};
		GLuint _program{0};
		GLuint _scale_plane_vao{0};
		GLuint _scale_plane_program{0};
		int _scale_plane_num_vertices{0};
		GLint _scale_plane_mvp_uniform{0};
		GLint _scale_plane_count_uniform{0};
		GLint _scale_plane_opacity_uniform{0};


		int _num_vertices{0};
		GLint _mvp_uniform{0};
		GLint _bounds_uniform{0};
		GLint _time_uniform{0};

		GLint _highlight_uniform{0};

		glm::vec4 _bounds;
		glm::vec3 _cam_position{-1.8f, -1.8f, 1.8f};
		float _model_scale{1.f};
		bool _ortho_projection{true};

		glm::vec2 _selection_cursor{0.f};
	};
}

#endif // HEIGHTFIELDRENDERER_H
