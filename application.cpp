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
		glfwMakeContextCurrent(_window.get());

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
		glfwSetWindowUserPointer(_window.get(), &input);

		auto key_callback = [] (GLFWwindow* window, int keycode, int /*scancode*/, int action, int /*mods*/)
		{
			if(glfwGetWindowUserPointer(window))
				switch(action)
				{
				case GLFW_PRESS:
					static_cast<InputManager*>(glfwGetWindowUserPointer(window))->press_key(keycode);
					break;
				case GLFW_RELEASE:
					static_cast<InputManager*>(glfwGetWindowUserPointer(window))->release_key(keycode);
					break;
				}
		};
		glfwSetKeyCallback(_window.get(), key_callback);

		glfwSetInputMode(_window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		auto cursor_callback = [] (GLFWwindow* window, double x, double y)
		{
			if(glfwGetWindowUserPointer(window))
				static_cast<InputManager*>(glfwGetWindowUserPointer(window))->set_cursor(static_cast<float>(x), static_cast<float>(y));
		};
		glfwSetCursorPosCallback(_window.get(), cursor_callback);
		auto scroll_callback = [] (GLFWwindow* window, double x, double y)
		{
			if(glfwGetWindowUserPointer(window))
				static_cast<InputManager*>(glfwGetWindowUserPointer(window))->add_scroll_offset(static_cast<int>(x), static_cast<int>(y));
		};
		glfwSetScrollCallback(_window.get(), scroll_callback);
		auto framebuffer_callback = [] (GLFWwindow* window, int x, int y)
		{
			glViewport(0, 0, x, y);
			if(glfwGetWindowUserPointer(window))
				static_cast<InputManager*>(glfwGetWindowUserPointer(window))->resize_framebuffer(x, y);
		};
		glfwSetFramebufferSizeCallback(_window.get(), framebuffer_callback);
		auto focus_callback = [] (GLFWwindow* window, int focused)
		{
			if(glfwGetWindowUserPointer(window))
				static_cast<InputManager*>(glfwGetWindowUserPointer(window))->set_window_focused(focused == GLFW_TRUE);
		};
		glfwSetWindowFocusCallback(_window.get(), focus_callback);

		if(true)	// true->STUDYMODE
		{
			constexpr int step_index_input = 1300;
			constexpr int aggregation_count = 1;
			constexpr int aggregation_stride = 4;
			constexpr int field_index_input = 2;
			constexpr Ensemble::Analysis analysis_input = Ensemble::Analysis::GAUSSIAN_SINGLE;
			_ensemble.read_headers(step_index_input, aggregation_count, aggregation_stride);
			_ensemble.analyse_field(field_index_input, Ensemble::Analysis(analysis_input));
		}
		else
		{
			// UI
			// Select simulation step
			int step_index_input = 0;
			std::cout << "\nChoose a time step [0," << _ensemble.num_steps() << ")\n";
			std::cin >> step_index_input;
			int aggregation_count = 0;
			std::cout << "\nChoose how many steps you want to aggregate (merge) [0, n)\n";
			std::cin >> aggregation_count;
			int aggregation_stride = 0;
			std::cout << "\nChoose the aggregation stride [0, n)\n";
			std::cin >> aggregation_stride;
			_ensemble.read_headers(step_index_input, aggregation_count, aggregation_stride);

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
		}

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

		// OpenGL & window state
		glClearColor(.2f, .2f, .2f, 1.f);

		auto time = glfwGetTime();
		_delta = 0.0;

		glEnable(GL_DEPTH_TEST);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glfwShowWindow(_window.get());

		// Event loop
		while(!glfwWindowShouldClose(_window.get()))
		{
			auto new_time = glfwGetTime();
			_delta = static_cast<float>(new_time - time);
			time = new_time;

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderer->draw(_delta, static_cast<float>(time));

			glfwSwapBuffers(_window.get());
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
