#include "inputmanager.h"

namespace vis
{
	void InputManager::press_key(int keycode)
	{
		_pressed[keycode] = true;
		_released[keycode] = false;
	}

	void InputManager::release_key(int keycode)
	{
		_released[keycode] = true;
	}

	bool InputManager::get_key(int keycode)
	{
		if(_pressed[keycode])
		{
			_pressed[keycode] = !_released[keycode];
			return true;
		}
		return false;
	}

	void InputManager::set_cursor(float x_pos, float y_pos)
	{
		set_cursor(glm::vec2{x_pos, y_pos});
	}

	void InputManager::set_cursor(glm::vec2	position)
	{
		if(_cursor_pos != glm::vec2{})
			_cursor_offset += _cursor_pos - position;
		_cursor_pos = position;
	}

	glm::vec2 InputManager::get_cursor_position()
	{
		return _cursor_pos;
	}

	float InputManager::get_cursor_position_x()
	{
		return _cursor_pos.x;
	}

	float InputManager::get_cursor_position_y()
	{
		return _cursor_pos.y;
	}

	glm::vec2 InputManager::get_cursor_offset()
	{
		glm::vec2 offset = _cursor_offset;
		_cursor_offset = glm::vec2{};
		return offset;
	}

	float InputManager::get_cursor_offset_x()
	{
		float x_offset = _cursor_offset.x;
		_cursor_offset.x = 0.f;
		return x_offset;
	}

	float InputManager::get_cursor_offset_y()
	{
		float y_offset = _cursor_offset.y;
		_cursor_offset.y = 0.f;
		return y_offset;
	}
}
