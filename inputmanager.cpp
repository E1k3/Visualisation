#include "inputmanager.h"

#include <iostream>

namespace vis
{
	void InputManager::reset()
	{
		for(auto& kv : _keys_pressed)
			kv.second = false;
		for(auto& kv : _keys_released)
			kv.second = true;

		_scroll_offset = glm::ivec2{0};
		reset_cursor_offsets();
	}

	void InputManager::reset_cursor_offsets()
	{
		_cursor_offset = glm::vec2{0.f};
		ignore_cursor_input = true;
	}

	void InputManager::press_key(int keycode)
	{
		_keys_pressed[keycode] = true;
		_keys_released[keycode] = false;
	}

	void InputManager::release_key(int keycode)
	{
		_keys_released[keycode] = true;
	}

	bool InputManager::get_key(int keycode)
	{
		if(_keys_pressed[keycode])
		{
			_keys_pressed[keycode] = !_keys_released[keycode];
			return true;
		}
		return false;
	}

	bool InputManager::release_get_key(int keycode)
	{
		_keys_released[keycode] = true;
		return get_key(keycode);
	}

	void InputManager::press_button(int buttoncode)
	{
		_buttons_pressed[buttoncode] = true;
		_buttons_released[buttoncode] = false;
	}

	void InputManager::release_button(int buttoncode)
	{
		_buttons_released[buttoncode] = true;
	}

	bool InputManager::get_button(int buttoncode)
	{
		if(_buttons_pressed[buttoncode])
		{
			_buttons_pressed[buttoncode] = !_buttons_released[buttoncode];
			return true;
		}
		return false;
	}

	bool InputManager::release_get_button(int buttoncode)
	{
		_buttons_released[buttoncode] = true;
		return get_button(buttoncode);
	}

	void InputManager::set_cursor(glm::vec2	position)
	{
		if(!ignore_cursor_input)
			_cursor_offset += position - _cursor_pos;
		_cursor_pos = position;

		ignore_cursor_input = false;
	}

	void InputManager::set_cursor(float x_pos, float y_pos)
	{
		set_cursor(glm::vec2{x_pos, y_pos});
	}

	glm::vec2 InputManager::get_cursor_position() const
	{
		return _cursor_pos;
	}

	float InputManager::get_cursor_position_x() const
	{
		return _cursor_pos.x;
	}

	float InputManager::get_cursor_position_y() const
	{
		return _cursor_pos.y;
	}

	glm::vec2 InputManager::get_cursor_offset()
	{
		auto offset = _cursor_offset;
		_cursor_offset = glm::vec2{};
		return offset;
	}

	float InputManager::get_cursor_offset_x()
	{
		auto x_offset = _cursor_offset.x;
		_cursor_offset.x = 0.f;
		return x_offset;
	}

	float InputManager::get_cursor_offset_y()
	{
		auto y_offset = _cursor_offset.y;
		_cursor_offset.y = 0.f;
		return y_offset;
	}

	void InputManager::add_scroll_offset(glm::ivec2 offset)
	{
		_scroll_offset += offset;
	}

	void InputManager::add_scroll_offset(int x_offset, int y_offset)
	{
		add_scroll_offset(glm::ivec2{x_offset, y_offset});
	}

	glm::ivec2 InputManager::get_scroll_offset()
	{
		auto offset = _scroll_offset;
		_scroll_offset = glm::ivec2{0};
		return offset;
	}

	int InputManager::get_scroll_offset_x()
	{
		auto x_offset = _scroll_offset.x;
		_scroll_offset.x = 0;
		return x_offset;
	}

	int InputManager::get_scroll_offset_y()
	{
		auto y_offset = _scroll_offset.y;
		_scroll_offset.y = 0;
		return y_offset;
	}

	void InputManager::resize_framebuffer(int x, int y)
	{
		resize_framebuffer(glm::ivec2(x, y));
	}

	void InputManager::resize_framebuffer(glm::ivec2 size)
	{
		_framebuffer_size = size;
	}

	glm::ivec2 InputManager::get_framebuffer_size() const
	{
		return _framebuffer_size;
	}

	int InputManager::get_framebuffer_size_x() const
	{
		return _framebuffer_size.x;
	}

	int InputManager::get_framebuffer_size_y() const
	{
		return _framebuffer_size.y;
	}

	float InputManager::get_framebuffer_aspect_ratio() const
	{
		if(_framebuffer_size.y == 0)
			return 1;
		return static_cast<float>(_framebuffer_size.x) / _framebuffer_size.y;
	}
}
