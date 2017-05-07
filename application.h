#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <memory>

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

		std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>> _window;
		EnsembleManager _ensemble;
	};
}
#endif // APPLICATION_H
