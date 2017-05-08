#include "application.h"

#include <experimental/filesystem>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

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
		glfwSetWindowUserPointer(&*_window, this);
		glfwSetInputMode(&*_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetFramebufferSizeCallback(&*_window, framebuffer_callback);
		glfwSetKeyCallback(&*_window, key_callback);

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
		_delta = 0.0;

		using glm::vec3;
		using glm::mat4;

		_cam_position = vec3{1.5f};
		_cam_direction = -_cam_position;

		glEnable(GL_DEPTH_TEST);

		while(!glfwWindowShouldClose(&*_window))
		{
			_delta = 2.f * static_cast<float>(glfwGetTime() - time);
			time = glfwGetTime();


			auto model = glm::scale(mat4{}, vec3{1.f, 1.f, 1.f});
			model = glm::rotate(static_cast<float>(glfwGetTime()*0.2), vec3{0.f, 0.f, 1.f});
			auto view = glm::lookAt(_cam_position, _cam_position+_cam_direction, vec3{0.f, 0.f, 1.f});
			auto proj = glm::perspective(glm::radians(45.f), 16.f / 9.f, 1.f, 10.f);
			auto mvp = proj * view * model;
			renderer.setMVP(mvp);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderer.draw();

			glfwSwapBuffers(&*_window);
			glfwPollEvents();
		}
	}

	void Application::cursor_callback(GLFWwindow* window, double x, double y)
	{
		auto& app = *static_cast<Application*>(glfwGetWindowUserPointer(window));
	}

	void Application::key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int mods)
	{
		auto& app = *static_cast<Application*>(glfwGetWindowUserPointer(window));
		switch(key)
		{
		case GLFW_KEY_UP:
			//[[fallthrough]];
		case GLFW_KEY_W:
			app._cam_position += app._delta * glm::normalize(app._cam_direction);
			break;

		case GLFW_KEY_DOWN:
			// [[fallthrough]]
		case GLFW_KEY_S:
			app._cam_position -= app._delta * glm::normalize(app._cam_direction);
			break;

		case GLFW_KEY_LEFT:
			// [[fallthrough]]
		case GLFW_KEY_A:
			app._cam_position += app._delta * glm::cross(glm::normalize(app._cam_direction), glm::vec3{0.f, 0.f, -1.f});
			break;

		case GLFW_KEY_RIGHT:
			// [[fallthrough]]
		case GLFW_KEY_D:
			app._cam_position -= app._delta * glm::cross(glm::normalize(app._cam_direction), glm::vec3{0.f, 0.f, -1.f});
			break;
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
