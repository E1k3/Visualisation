#include "visualisation.h"

namespace vis
{
	Visualisation::Visualisation(InputManager& input, const std::vector<Field>& fields)
		: _input{input},
		  _fields{fields}
	{	}

	void Visualisation::setup()
	{
		setup_data();
		setup_shaders();
	}

	void Visualisation::update_selection_cursor(glm::vec2 mouse_offset, glm::mat4 modelview, float aspect_ratio, float scale)
	{
		constexpr auto cursor_speed = 0.0005f;

		if(mouse_offset == glm::vec2{0})
			return;

		auto mv_inv = glm::inverse(modelview);
		auto zero_mapped = mv_inv * glm::vec4{0.f, 0.f, 0.f, 1.f};
		auto x_mapped = mv_inv * glm::vec4{1.f, 0.f, 0.f, 1.f};
		auto y_mapped = mv_inv * glm::vec4{0.f, 1.f, 0.f, 1.f};
		zero_mapped /= zero_mapped.w;
		x_mapped = x_mapped / x_mapped.w - zero_mapped;
		y_mapped = y_mapped / y_mapped.w - zero_mapped;

		_cursor_position += (1.f/aspect_ratio * glm::normalize(glm::vec2{x_mapped}) * mouse_offset.x + glm::normalize(glm::vec2{y_mapped}) * mouse_offset.y) * cursor_speed * (1.f/scale);
		_cursor_position = glm::clamp(_cursor_position, glm::vec2(0.f), glm::vec2(1.f));
	}

	glm::vec2 Visualisation::get_cursor() const
	{
		return _cursor_position;
	}

	void Visualisation::set_highlight_area(const glm::ivec4& area)
	{
		_highlight_area = area;
	}
}
