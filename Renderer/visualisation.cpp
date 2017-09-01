#include "visualisation.h"

namespace vis
{
	Visualisation::Visualisation(InputManager& input, const std::vector<Field>& fields)
		: _input{input},
		  _fields{fields}
	{

	}

	void Visualisation::update_selection_cursor(glm::vec2 mouse_offset, glm::mat4 mv)
	{
		constexpr auto cursor_speed = 0.001f;
		auto mv_inv = glm::inverse(mv);
		auto zero_mapped = mv_inv * glm::vec4{0.f, 0.f, 0.f, 1.f};
		auto x_mapped = mv_inv * glm::vec4{1.f, 0.f, 0.f, 1.f};
		auto y_mapped = mv_inv * glm::vec4{0.f, 1.f, 0.f, 1.f};
		zero_mapped /= zero_mapped.w;
		x_mapped = x_mapped / x_mapped.w - zero_mapped;
		y_mapped = y_mapped / y_mapped.w - zero_mapped;

		// cursor += direction * mouse distance * speed
		_selection_cursor += glm::vec2{x_mapped} * mouse_offset.x * cursor_speed;
		_selection_cursor += glm::vec2{y_mapped} * mouse_offset.y * cursor_speed;
		_selection_cursor = glm::clamp(_selection_cursor, glm::vec2(0.f), glm::vec2(1.f));
	}
}
