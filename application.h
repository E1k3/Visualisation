#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Data/ensemblemanager.h"

namespace vis
{
	class Application
	{
	public:
		/**
		 * @brief Application Constructor
		 * @param path Path to the root directory of the ensemble files.
		 */
		explicit Application(std::string path);

		/**
		 * @brief run
		 */
		void run();

	private:
		/**
		 * @brief error_callback Callback function to be used by GLFW to indicate errors.
		 * @param error The error id.
		 * @param description A human readable description of the error.
		 */
		[[noreturn]] static void error_callback(int error, const char* description);

		/**
		 * @brief framebuffer_callback Callback function to be used by GLFW to indicate changes in framebuffer size.
		 * @param window The window that holds the changed framebuffer.
		 * @param width The new framebuffer width.
		 * @param height The new framebuffer height.
		 */
		static void framebuffer_callback(GLFWwindow* window, int width, int height);

		/**
		 * @brief cursor_callback Callback function to be used by GLFW to indicate mouse movements.
		 * @param window The window that registered the event.
		 * @param x The current cursor x position.
		 * @param y The current cursor y position.
		 */
		static void cursor_callback(GLFWwindow* window, double x, double y);

		/**
		 * @brief key_callback Callback function to be used by GLFW to indicate key actions.
		 * @param window The window that registered the event.
		 * @param key The GLFW keycode of the actuated key.
		 * @param scancode The platform specific code of the actuated key.
		 * @param action The action that was performed with the key.
		 * @param mods Bitfield of the active modifiers during the event.
		 */
		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

		std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>> _window;
		EnsembleManager _ensemble;


		glm::vec3 _cam_direction{};
		glm::vec3 _cam_position{};
		float _delta{};

		bool _keyspressed[3] = {false, false, false};
	};
}
#endif // APPLICATION_H
