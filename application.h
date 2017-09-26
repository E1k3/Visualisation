#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <memory>
#include <functional>
#include <glm/glm.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Data/ensemble.h"

namespace vis
{
	/**
	 * @brief The Application class manages the window and general control.
	 */
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
		void execute();

	private:

		int study_timestep(int test, int question);
		std::tuple<glm::ivec4, glm::ivec4> study_data(int test, int question);
		Ensemble::Analysis study_analysis(int question);
		int study_visualization(int test, int question);

		/**
		 * @brief error_callback Callback function to be used by GLFW to indicate errors.
		 * @param error The error id.
		 * @param description A human readable description of the error.
		 */
		[[noreturn]] static void error_callback(int error, const char* description);

		std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>> _window;
		Ensemble _ensemble;


		float _delta{0.f};
		glm::ivec2 _framebuffer_size{0};
	};
}
#endif // APPLICATION_H
