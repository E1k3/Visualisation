#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <unordered_map>

#include <glm/glm.hpp>

namespace vis
{
	/**
	 * @brief The InputManager class collects inputs from GLFW callbacks and provides through a relatively simple interface.
	 */
	class InputManager
	{
	public:
		explicit InputManager() = default;

		/**
		 * @brief reset Resets all offsets and key states.
		 */
		void reset();

		/**
		 * @brief reset_offsets Resets the cursor offsets.
		 */
		void reset_cursor_offsets();

		/**
		 * @brief press_key Saves the pressed key until it is released and has been requested at least once.
		 * @param keycode The keycode of the pressed key.
		 */
		void press_key(int keycode);
		/**
		 * @brief release_key Marks the released key until it is requested, then resets the keys pressed status.
		 * @param keycode The keycode of the released key.
		 */
		void release_key(int keycode);

		/**
		 * @brief get_key Returns the status of a key.
		 * Resets the pressed status if it was released (and not pressed again) before this function was called.
		 * @param keycode The keycode of the requested key.
		 * @return The status of the requested key.
		 */
		bool get_key(int keycode);

		/**
		 * @brief get_release_key Acts identical to get_key, but resets the pressed state even when the physical key is still pressed.
		 */
		bool release_get_key(int keycode);

		/**
		 * @brief press_button Saves the pressed button until it is released and has been requested at least once.
		 * @param buttoncode The buttoncode of the pressed button.
		 */
		void press_button(int buttoncode);

		/**
		 * @brief release_button Marks the released button until it is requested, then resets the buttons pressed status.
		 * @param buttoncode The buttoncode of the released button.
		 */
		void release_button(int buttoncode);

		/**
		 * @brief get_button Returns the status of a key.
		 * Resets the pressed status if it was released (and not pressed again) before this function was called.
		 * @param buttoncode The buttoncode of the requested button.
		 * @return The status of the requested button.
		 */
		bool get_button(int buttoncode);

		/**
		 * @brief get_release_button Acts identical to get_button, but resets the pressed state even when the physical button is still pressed.
		 */
		bool release_get_button(int buttoncode);

		/**
		 * @brief move_cursor Saves the mouse movement until it is requested.
		 * @param position The current position of the cursor.
		 */
		void set_cursor(glm::vec2 position);
		/**
		 * @brief move_cursor Saves the mouse movement until it is requested.
		 * @param x_pos The current x position of the cursor.
		 * @param y_pos The current y position of the cursor.
		 */
		void set_cursor(float x_pos, float y_pos);

		/**
		 * @brief get_cursor_position Returns the current cursor position.
		 * @return The cursor position as 2D vector.
		 */
		glm::vec2 get_cursor_position() const;
		/**
		 * @brief get_cursor_position_x Returns the current cursor x position.
		 * @return The cursor position in x direction.
		 */
		float get_cursor_position_x() const;
		/**
		 * @brief get_cursor_position_y Returns the current cursor y position.
		 * @return The cursor position in y direction.
		 */
		float get_cursor_position_y() const;

		/**
		 * @brief get_cursor_offset Returns the saved cursor offset and resets the stored offset.
		 * @return The offset as 2D vector.
		 */
		glm::vec2 get_cursor_offset();
		/**
		 * @brief get_cursor_offset_x Returns the saved cursor offset in x direction and resets the stored offset.
		 * @return The x offset.
		 */
		float get_cursor_offset_x();
		/**
		 * @brief get_cursor_offset_y Returns the saved cursor offset in y direction and resets the stored offset.
		 * @return The y offset.
		 */
		float get_cursor_offset_y();

		/**
		 * @brief add_scroll_offset Adds the offset to the stored value.
		 * The value is stored until it is requested.
		 * @param offset The occuring offset.
		 */
		void add_scroll_offset(glm::ivec2 offset);
		/**
		 * @brief add_scroll_offset Adds the offset to the stored value.
		 * The value is stored until it is requested.
		 * @param x_offset The occuring offset in x direction.
		 * @param y_offset The occuring offset in y direction.
		 */
		void add_scroll_offset(int x_offset, int y_offset);
		/**
		 * @brief get_scroll_offset Returns the total scroll offset since the last request and resets the stored value.
		 * @return The scroll offset.
		 */
		glm::ivec2 get_scroll_offset();
		/**
		 * @brief get_scroll_offset_x Returns the total scroll offset in x direction since the last request and resets the stored x value.
		 * @return The x offset.
		 */
		int get_scroll_offset_x();
		/**
		 * @brief get_scroll_offset_y Returns the total scroll offset in x direction since the last request and resets the stored y value.
		 * @return The y offset.
		 */
		int get_scroll_offset_y();

		/**
		 * @brief resize_framebuffer Informs the input manager of the current framebuffer size.
		 * @param size The size in pixels.
		 */
		void resize_framebuffer(glm::ivec2 size);
		/**
		 * @brief resize_framebuffer Informs the input manager of the current framebuffer size.
		 * @param x The width in pixels.
		 * @param y The height in pixels.
		 */
		void resize_framebuffer(int x, int y);

		/**
		 * @brief get_framebuffer_size Returns the latest framebuffer size.
		 * @return The framebuffer size in pixels.
		 */
		glm::ivec2 get_framebuffer_size() const;
		/**
		 * @brief get_framebuffer_size_x Returns the latest framebuffer width.
		 * @return The width in pixels.
		 */
		int get_framebuffer_size_x() const;
		/**
		 * @brief get_framebuffer_size_y Returns the latest framebuffer height.
		 * @return The height in pixels.
		 */
		int get_framebuffer_size_y() const;
		/**
		 * @brief get_framebuffer_aspect_ratio Returns the latest framebuffer aspect ratio.
		 * If height == 0, returns 1.
		 * @return The pixel ratio.
		 */
		float get_framebuffer_aspect_ratio() const;

	private:
		bool ignore_cursor_input{true};

		std::unordered_map<int, bool> _keys_pressed;
		std::unordered_map<int, bool> _keys_released;

		std::unordered_map<int, bool> _buttons_pressed;
		std::unordered_map<int, bool> _buttons_released;

		glm::vec2 _cursor_pos;
		glm::vec2 _cursor_offset;

		glm::ivec2 _scroll_offset;

		glm::ivec2 _framebuffer_size;
	};
}

#endif // INPUTMANAGER_H
