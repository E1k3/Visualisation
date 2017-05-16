#include "application.h"

#include <experimental/filesystem>

#include "logger.h"
#include "glyphrenderer.h"
#include "heightfieldrenderer.h"
#include "inputmanager.h"

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

		//DBG
		Logger::instance() << Logger::Severity::DEBUG
						   << "GLFW initialised."
						   << std::endl;

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

		//DBG
		Logger::instance() << Logger::Severity::DEBUG
						   << "GLEW initialised."
						   << std::endl;

		// Load data
		_ensemble.processSingleStep(10);
		auto& step = _ensemble.currentStep();
		step.normalise(8, 8);
		step.normalise(2, 2);
	}

	void Application::execute()
	{
		// Set up input manager
		auto input = InputManager{};
		glfwSetWindowUserPointer(&*_window, &input);
		glfwSetFramebufferSizeCallback(&*_window, framebuffer_callback);

		auto key_callback = [] (GLFWwindow* window, int keycode, int /*scancode*/, int action, int /*mods*/)
		{
			auto& input = *static_cast<InputManager*>(glfwGetWindowUserPointer(window));
			if(action == GLFW_PRESS)
				input.press_key(keycode);
			if(action == GLFW_RELEASE)
				input.release_key(keycode);
		};
		glfwSetKeyCallback(&*_window, key_callback);

		glfwSetInputMode(&*_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		auto cursor_callback = [] (GLFWwindow* window, double x, double y)
		{
			auto& input = *static_cast<InputManager*>(glfwGetWindowUserPointer(window));
			input.set_cursor(static_cast<float>(x), static_cast<float>(y));
		};
		glfwSetCursorPosCallback(&*_window, cursor_callback);

		GlyphRenderer renderer{&_ensemble, &input};

		glClearColor(.2f, .2f, .2f, 1.f);

		auto time = glfwGetTime();
		_delta = 0.0;

		glEnable(GL_DEPTH_TEST);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		while(!glfwWindowShouldClose(&*_window))
		{
			_delta = 2.f * static_cast<float>(glfwGetTime() - time);
			time = glfwGetTime();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderer.draw(_delta);

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
