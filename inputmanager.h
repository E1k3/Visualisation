#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <map>

#include <glm/glm.hpp>

namespace vis
{
	class InputManager
	{
	public:
		explicit InputManager() = default;

		/**
		 * @brief press_key Saves the pressed key until it is released and requested.
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
		 * Resets the pressed status if it was released befor this function was called.
		 * @param keycode The keycode of the requested key.
		 * @return The status of the requested key.
		 */
		bool get_key(int keycode);

		/**
		 * @brief move_cursor Saves the mouse movement until it is requested.
		 * @param x_pos The current x position of the cursor.
		 * @param y_pos The current y position of the cursor.
		 */
		void set_cursor(float x_pos, float y_pos);
		/**
		 * @brief move_cursor Saves the mouse movement until it is requested.
		 * @param position The current position of the cursor.
		 */
		void set_cursor(glm::vec2 position);

		/**
		 * @brief get_cursor_position Returns the current cursor position.
		 * @return The cursor position as 2D vector.
		 */
		glm::vec2 get_cursor_position();
		/**
		 * @brief get_cursor_position_x Returns the current cursor x position.
		 * @return The cursor position in x direction.
		 */
		float get_cursor_position_x();
		/**
		 * @brief get_cursor_position_y Returns the current cursor y position.
		 * @return The cursor position in y direction.
		 */
		float get_cursor_position_y();

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
		 * @param x_offset The occuring offset in x direction.
		 * @param y_offset The occuring offset in y direction.
		 */
		void add_scroll_offset(int x_offset, int y_offset);
		/**
		 * @brief add_scroll_offset Adds the offset to the stored value.
		 * The value is stored until it is requested.
		 * @param offset The occuring offset.
		 */
		void add_scroll_offset(glm::ivec2 offset);
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
		 * @brief get_scroll_offset Returns the total scroll offset since the last request and resets the stored value.
		 * @return The scroll offset.
		 */
		glm::ivec2 get_scroll_offset();

	private:
		std::map<int, bool> _pressed;
		std::map<int, bool> _released;

		glm::vec2 _cursor_pos;
		glm::vec2 _cursor_offset;

		glm::ivec2 _scroll_offset;
	};
}

#endif // INPUTMANAGER_H
