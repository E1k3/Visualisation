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

	private:
		std::map<int, bool> _pressed{};
		std::map<int, bool> _released{};

		glm::vec2 _cursor_pos{};
		glm::vec2 _cursor_offset{};
	};
}

#endif // INPUTMANAGER_H
