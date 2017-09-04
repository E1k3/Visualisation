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
	class Application
	{
	public:
		static constexpr int step_index_input = 1300;
		static constexpr int aggregation_count = 1;
		static constexpr int aggregation_stride = 4;
		static constexpr int field_index_input = 2;
		static constexpr Ensemble::Analysis analysis_input = Ensemble::Analysis::GAUSSIAN_MIXTURE;
		static const float study_highlights[][4];
		static const int study_steps[];
		static unsigned study_select;


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
