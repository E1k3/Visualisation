#include "visualisation.h"

namespace vis
{
	Visualisation::Visualisation(InputManager& input, const std::vector<Field>& fields)
		: _input{input},
		  _fields{fields}
	{
		setup();
	}

	void Visualisation::setup()
	{
		setup_data();
		setup_shaders();
	}

	void Visualisation::update_selection_cursor(glm::vec2 mouse_offset)
	{
		constexpr auto cursor_speed = 0.001f;
		auto mvp_inv = glm::inverse(_mvp);
		auto x_mapped = mvp_inv * glm::vec4{1.f, 0.f, 0.f, 1.f};
		auto y_mapped = mvp_inv * glm::vec4{0.f, 1.f, 0.f, 1.f};
		x_mapped /= x_mapped.w;
		y_mapped /= y_mapped.w;

		// cursor += direction * mouse distance * speed
		_selection_cursor += glm::normalize(glm::vec2{x_mapped}) * mouse_offset.x * cursor_speed;
		_selection_cursor += glm::normalize(glm::vec2{y_mapped}) * mouse_offset.y * cursor_speed;
	}
}
