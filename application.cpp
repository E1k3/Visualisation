#include "application.h"

#include <experimental/filesystem>

#include "logger.h"
#include "Renderer/glyphrenderer.h"
#include "Renderer/heightfieldrenderer.h"
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
			Logger::instance() << Logger::Severity::ERROR << "GLFW init failed";
			// TODO:ERROR handling
			throw std::runtime_error("GLFW init failed");
		}

		// Window hints
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	// Opengl version 3.
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);	// 3
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	// Core profile
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);	// Hide window
		glfwWindowHint(GLFW_SAMPLES, 16);	// 16xMSAA

		auto deleter = [] (GLFWwindow* window) { glfwDestroyWindow(window); glfwTerminate(); };	// Destroy window and terminate GLFW, when _window gets out of scope.
		_window = std::unique_ptr<GLFWwindow, decltype (deleter)>(glfwCreateWindow(1520, 855, "Test", NULL, NULL), deleter);
		if(!_window)
		{
			Logger::instance() << Logger::Severity::ERROR << "GLFW window creation failed";
			// TODO:ERROR handling
			throw std::runtime_error("GLFW window creation failed");
		}
		glfwMakeContextCurrent(&*_window);

		//DBG
		Logger::instance() << Logger::Severity::DEBUG
						   << "GLFW initialised.";

		// GLEW init
		glewExperimental = GL_TRUE;
		GLenum status = glewInit();
		if(status != GLEW_OK)
		{
			Logger::instance() << Logger::Severity::ERROR << "GLEW init failed: "
							   << reinterpret_cast<const char*>(glewGetErrorString(status));
			// TODO:ERROR handling
			throw std::runtime_error("GLEW init failed");
		}

		//DBG
		Logger::instance() << Logger::Severity::DEBUG
						   << "GLEW initialised.";
	}

	void Application::execute()
	{
		// Set up input manager
		auto input = InputManager{};
		glfwSetWindowUserPointer(&*_window, &input);

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
		auto scroll_callback = [] (GLFWwindow* window, double x, double y)
		{
			auto& input = *static_cast<InputManager*>(glfwGetWindowUserPointer(window));
			input.add_scroll_offset(static_cast<int>(x), static_cast<int>(y));
		};
		glfwSetScrollCallback(&*_window, scroll_callback);
		auto framebuffer_callback = [] (GLFWwindow* window, int x, int y)
		{
			glViewport(0, 0, x, y);
			auto& input = *static_cast<InputManager*>(glfwGetWindowUserPointer(window));
			input.resize_framebuffer(x, y);
		};
		glfwSetFramebufferSizeCallback(&*_window, framebuffer_callback);


		// Select simulation step
		int step_index_input = 0;
		std::cout << "\nChoose a time step [0," << _ensemble.num_steps() << ")\n";
		std::cin >> step_index_input;
		_ensemble.read_headers(step_index_input);

		// Select field
		std::cout << "\nThe simulations contain " << _ensemble.fields().size() << " fields.\n";

		{
			int i = 0;
			for(const auto& field : _ensemble.fields())
				std::cout << i++ << " " << field.layout_to_string() << '\n';
		}
		std::cout << "Choose one [0," << _ensemble.fields().size() << ")\n";
		int field_index_input = 2;	// Magic number as default.
		std::cin >> field_index_input;

		// Select analysis
		std::cout << "\nAnalyze field using:\n0 Maximum likelihood Normal distribution\n1 Maximum likelihood GMM\n";
		int analysis_input = 0;
		std::cin >> analysis_input;
		_ensemble.analyse_field(field_index_input, Ensemble::Analysis(analysis_input));

		// Select renderer
		auto renderer = std::unique_ptr<Renderer>{};
		std::cout << "\nRender result using:\n0 Heightfield renderer\n1 Glyph renderer\n";
		int renderer_input = 0;
		std::cin >> renderer_input;
		switch (renderer_input)
		{
		case 0:
			renderer = std::make_unique<HeightfieldRenderer>(_ensemble.fields(), input);
			break;
		case 1:
			renderer = std::make_unique<GlyphRenderer>(_ensemble.fields(), input);
			break;
		default:
			//TODO error
			break;
		}

		glClearColor(.2f, .2f, .2f, 1.f);

		auto time = glfwGetTime();
		_delta = 0.0;

		glEnable(GL_DEPTH_TEST);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glfwShowWindow(&*_window);
		// Event loop
		while(!glfwWindowShouldClose(&*_window))
		{
			_delta = 2.f * static_cast<float>(glfwGetTime() - time);
			time = glfwGetTime();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderer->draw(_delta, static_cast<float>(time));

			glfwSwapBuffers(&*_window);
			glfwPollEvents();
		}

		Logger::instance() << Logger::Severity::DEBUG << "Application execution finished successfully\n";
	}

	void Application::error_callback(int error, const char* description)
	{
		Logger::instance() << Logger::Severity::ERROR << "GLFW ERROR: " << error << " " << description;

		// TODO:ERROR handling
		throw std::runtime_error("GLFW ERROR");
	}
}
