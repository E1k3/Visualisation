#include "application.h"

#include <experimental/filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "logger.h"
#include "heightfieldrenderer.h"

namespace vis
{
	namespace fs = std::experimental::filesystem;
	Application::Application(std::string path)
		: _ensemble{fs::path{path}}
	{
		// GLFW init
		glfwSetErrorCallback(error_callback);

		if(!glfwInit())
		{
			Logger::instance() << Logger::Severity::ERROR << "GLFW init failed" << std::endl;
			// TODO:ERROR handling
			throw std::runtime_error("GLFW init failed");
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		auto deleter = [](GLFWwindow* window) {glfwDestroyWindow(window); glfwTerminate();};
		_window = std::unique_ptr<GLFWwindow, decltype (deleter)>(glfwCreateWindow(1520, 855, "Test", NULL, NULL), deleter);
		if(!_window)
		{
			Logger::instance() << Logger::Severity::ERROR << "GLFW window creation failed" << std::endl;
			// TODO:ERROR handling
			throw std::runtime_error("GLFW window creation failed");
		}

		glfwMakeContextCurrent(&*_window);
		glfwSetFramebufferSizeCallback(&*_window, framebuffer_callback);

		glfwSetInputMode(&*_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// GLEW init
		glewExperimental = GL_TRUE;
		GLenum status = glewInit();
		if(status != GLEW_OK)
		{
			Logger::instance() << Logger::Severity::ERROR << "GLEW init failed: "
							   << reinterpret_cast<const char*>(glewGetErrorString(status)) << std::endl;
			// TODO:ERROR handling
			throw std::runtime_error("GLEW init failed");
		}

		// Load data
		_ensemble.processSingleStep(256);
		auto& step = _ensemble.currentStep();
		step.normaliseAll();
	}

	void Application::run()
	{
		HeightfieldRenderer renderer{_ensemble};

		glClearColor(1.f, 1.f, 1.f, 1.f);

		auto time = glfwGetTime();
		auto delta = 0.0;

		while(!glfwWindowShouldClose(&*_window))
		{
			delta = glfwGetTime() - time;
			time = glfwGetTime();

			using glm::vec3;
			using glm::mat4;

			auto model = glm::scale(mat4{}, vec3{1.f, 1.f, 1.f});
			auto view = glm::lookAt(vec3{1.8f}, vec3{0.f}, vec3{0.f, 0.f, 1.f});
			auto proj = glm::perspective(glm::radians(45.0f), 16.f / 9.f, 1.0f, 10.0f);
			auto mvp = proj * view * model;
			renderer.setMVP(mvp);

			glClear(GL_COLOR_BUFFER_BIT);
			renderer.draw();

			glfwSwapBuffers(&*_window);
			glfwPollEvents();
		}
	}

	void Application::error_callback(int error, const char* description)
	{
		Logger::instance() << Logger::Severity::ERROR << "GLFW ERROR: " << error << " " << description << std::endl;

		// TODO:ERROR handling
		throw std::runtime_error("GLFW ERROR");
	}

	void Application::framebuffer_callback(GLFWwindow* /*window*/, int width, int height)
	{
		glViewport(0, 0, width, height);
	}
}
