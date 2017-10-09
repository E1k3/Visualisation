#include "application.h"

#include <experimental/filesystem>
#include <fstream>
#include <algorithm>

#include "logger.h"
#include "inputmanager.h"

#include "Renderer/glyph.h"
#include "Renderer/glyphgmm.h"
#include "Renderer/heightfield.h"
#include "Renderer/heightfieldgmm.h"

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
			Logger::error() << "GLFW init failed";
			throw std::runtime_error("GLFW init failed");
		}

		// Window hints
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	// Opengl version 3.
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);	// 3
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	// Core profile
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);	// Hide window
		glfwWindowHint(GLFW_SAMPLES, 16);	// 16xMSAA

		auto deleter = [] (GLFWwindow* window) { glfwDestroyWindow(window); glfwTerminate(); };	// Destroy window and terminate GLFW, when _window gets out of scope.
		_window = std::unique_ptr<GLFWwindow, decltype (deleter)>(glfwCreateWindow(1520, 855, "Test", nullptr, nullptr), deleter);
		if(!_window)
		{
			Logger::error() << "GLFW window creation failed";
			throw std::runtime_error("GLFW window creation failed");
		}
		glfwMakeContextCurrent(_window.get());
		glfwSwapInterval(1);	// VSYNC

		//DBG
		Logger::debug() << "GLFW initialized.";

		// GLEW init
		glewExperimental = GL_TRUE;
		GLenum status = glewInit();
		if(status != GLEW_OK)
		{
			Logger::error() << "GLEW init failed: "
							<< reinterpret_cast<const char*>(glewGetErrorString(status));
			throw std::runtime_error("GLEW init failed");
		}

		//DBG
		Logger::debug() << "GLEW initialized.";
	}

	void Application::execute()
	{
		glfwSetInputMode(_window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		// Set up input manager
		auto input = InputManager{};
		glfwSetWindowUserPointer(_window.get(), &input);

		auto key_callback = [] (GLFWwindow* window, int keycode, int /*scancode*/, int action, int /*mods*/)
		{
			if(keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			if(glfwGetWindowUserPointer(window))
			{
				if(action == GLFW_PRESS)
					static_cast<InputManager*>(glfwGetWindowUserPointer(window))->press_key(keycode);
				else if(action == GLFW_RELEASE)
					static_cast<InputManager*>(glfwGetWindowUserPointer(window))->release_key(keycode);
			}
		};
		glfwSetKeyCallback(_window.get(), key_callback);
		auto button_callback = [] (GLFWwindow* window, int buttoncode, int action, int /*modifier*/)
		{
			if(buttoncode == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			if(glfwGetWindowUserPointer(window))
			{
				if(action == GLFW_PRESS)
					static_cast<InputManager*>(glfwGetWindowUserPointer(window))->press_button(buttoncode);
				else if(action == GLFW_RELEASE)
					static_cast<InputManager*>(glfwGetWindowUserPointer(window))->release_button(buttoncode);
			}
		};
		glfwSetMouseButtonCallback(_window.get(), button_callback);
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
			{
				static_cast<InputManager*>(glfwGetWindowUserPointer(window))->resize_framebuffer(x, y);
				static_cast<InputManager*>(glfwGetWindowUserPointer(window))->reset();
			}
		};
		glfwSetFramebufferSizeCallback(_window.get(), framebuffer_callback);
		auto focus_callback = [] (GLFWwindow* window, int /*focused*/)
		{
			if(glfwGetWindowUserPointer(window))
				static_cast<InputManager*>(glfwGetWindowUserPointer(window))->reset();
		};
		glfwSetWindowFocusCallback(_window.get(), focus_callback);


		// Crude CLI
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
		std::cout << "\nThe simulations contain " << _ensemble.headers().size() << " fields.\n";
		{
			int i = 0;
			for(const auto& field : _ensemble.headers())
				std::cout << i++ << " " << field.layout_to_string() << '\n';
		}
		std::cout << "Choose one [0," << _ensemble.headers().size() << ")\n";
		int field_index_input = 2;	// Magic number as default.
		std::cin >> field_index_input;

		// Select analysis
		std::cout << "\nAnalyze field using:\n0 MLE for Normal distribution\n1 MLE for GMM\n";
		int analysis_input = 0;
		std::cin >> analysis_input;
		_ensemble.analyse_field(field_index_input, Ensemble::Analysis(analysis_input));


		// Select renderer
		auto vis = std::unique_ptr<Visualization>{nullptr};
		if(Ensemble::Analysis(analysis_input) == Ensemble::Analysis::GAUSSIAN_SINGLE)
			std::cout << "\nRender result using:\n0 Heightfield renderer\n1 Glyph renderer\n";
		else
			std::cout << "\nRender result using:\n0 Heightfield renderer\n1 Circular Sector Glyph renderer\n2 Multi-ring Glyph renderer\n";
		int renderer_input = 0;
		std::cin >> renderer_input;
		bool renderer_initialized = false;

		// OpenGL & window state
		glClearColor(.1f, .1f, .1f, 1.f);

		auto time = glfwGetTime();
		_delta = 0.0;

		glEnable(GL_DEPTH_TEST);
		glfwShowWindow(_window.get());

		Text statusline;

		// Event loop
		while(!glfwWindowShouldClose(_window.get()))
		{
			auto new_time = glfwGetTime();
			_delta = static_cast<float>(new_time - time);
			time = new_time;

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Quick-switch renderers
			if(input.release_get_key(GLFW_KEY_ENTER))
			{
				if(Ensemble::Analysis(analysis_input) == Ensemble::Analysis::GAUSSIAN_SINGLE)
				{
					if(renderer_initialized)
						renderer_input = (renderer_input + 1) % 2;
				}
				else
				{
					if(renderer_initialized)
						renderer_input = (renderer_input + 1) % 3;
				}
				renderer_initialized = false;
			}
			if(input.release_get_key(GLFW_KEY_BACKSPACE))
			{
				if(Ensemble::Analysis(analysis_input) == Ensemble::Analysis::GAUSSIAN_SINGLE)
				{
					if(renderer_initialized)
						renderer_input = (renderer_input + 1) % 2;
				}
				else
				{
					if(renderer_initialized)
						renderer_input = (renderer_input + 2) % 3;
				}
				renderer_initialized = false;
			}
			if(!renderer_initialized || input.release_get_key(GLFW_KEY_ENTER))
			{
				vis.reset(nullptr);
				switch (renderer_input)
				{
				case 0:
					if(static_cast<Ensemble::Analysis>(analysis_input) == Ensemble::Analysis::GAUSSIAN_SINGLE)
						vis = std::make_unique<Heightfield>(input, _ensemble.fields());
					else
						vis = std::make_unique<HeightfieldGMM>(input, _ensemble.fields());
					break;
				case 1:
					if(static_cast<Ensemble::Analysis>(analysis_input) == Ensemble::Analysis::GAUSSIAN_SINGLE)
						vis = std::make_unique<Glyph>(input, _ensemble.fields());
					else
						vis = std::make_unique<GlyphGMM>(input, _ensemble.fields(), true);
					break;
				case 2:
					if(static_cast<Ensemble::Analysis>(analysis_input) == Ensemble::Analysis::GAUSSIAN_MIXTURE)
						vis = std::make_unique<GlyphGMM>(input, _ensemble.fields());
					else
					{
						Logger::error() << "The renderer selection is invalid.";
						throw std::runtime_error{"Invalid renderer selection"};
					}
					break;
				default:
					Logger::error() << "The renderer selection is invalid.";
					throw std::runtime_error{"Invalid renderer selection"};
				}
				vis->setup();
				renderer_initialized = true;
			}

			// Update and draw visualizations
			vis->update(_delta, static_cast<float>(time));
			vis->draw();

			statusline.set_viewport(input.get_framebuffer_size());
			auto statusline_text = std::string{" Cursor (" + std::to_string(vis->point_under_cursor().x) + ", " + std::to_string(vis->point_under_cursor().y) + ") "};
			if(Ensemble::Analysis(analysis_input) == Ensemble::Analysis::GAUSSIAN_SINGLE)
				statusline_text += "mean = " + std::to_string(_ensemble.fields().at(0).get_value(0, vis->point_under_cursor().x, vis->point_under_cursor().y, 0))
								   + " deviation = " + std::to_string(_ensemble.fields().at(1).get_value(0, vis->point_under_cursor().x, vis->point_under_cursor().y, 0));

			statusline.set_lines({statusline_text});
			statusline.set_positions({glm::vec2{-1.f, 1.f - statusline.relative_sizes().front().y}});
			// TODO STUDY MODE CHANGE /\--/\--/\--/\--/\--/|

			glDisable(GL_DEPTH_TEST);
			statusline.draw();

			glfwSwapBuffers(_window.get());
			glfwPollEvents();
		}

		Logger::debug() << "Application execution finished successfully\n";
	}

	void Application::error_callback(int error, const char* description)
	{
		Logger::error() << "GLFW ERROR: " << error << " " << description;
		throw std::runtime_error("GLFW ERROR");
	}
}
