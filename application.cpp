#include "application.h"

#include <experimental/filesystem>

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
		_window = std::unique_ptr<GLFWwindow, decltype (deleter)>(glfwCreateWindow(1520, 855, "Test", NULL, NULL), deleter);
		if(!_window)
		{
			Logger::error() << "GLFW window creation failed";
			throw std::runtime_error("GLFW window creation failed");
		}
		glfwMakeContextCurrent(_window.get());
		glfwSwapInterval(1);	// VSYNC

		//DBG
		Logger::debug() << "GLFW initialised.";

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
		Logger::debug() << "GLEW initialised.";
	}

	void Application::execute()
	{
		glfwSetInputMode(_window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		// Set up input manager
		auto input = InputManager{};
		glfwSetWindowUserPointer(_window.get(), &input);

		auto key_callback = [] (GLFWwindow* window, int keycode, int /*scancode*/, int action, int /*mods*/)
		{
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


//		// Crude CLI-UI
//		// Select simulation step
//		int step_index_input = 0;
//		std::cout << "\nChoose a time step [0," << _ensemble.num_steps() << ")\n";
//		std::cin >> step_index_input;
//		int aggregation_count = 0;
//		std::cout << "\nChoose how many steps you want to aggregate (merge) [0, n)\n";
//		std::cin >> aggregation_count;
//		int aggregation_stride = 0;
//		std::cout << "\nChoose the aggregation stride [0, n)\n";
//		std::cin >> aggregation_stride;
//		_ensemble.read_headers(step_index_input, aggregation_count, aggregation_stride);

//		// Select field
//		std::cout << "\nThe simulations contain " << _ensemble.headers().size() << " fields.\n";
//		{
//			int i = 0;
//			for(const auto& field : _ensemble.headers())
//				std::cout << i++ << " " << field.layout_to_string() << '\n';
//		}
//		std::cout << "Choose one [0," << _ensemble.headers().size() << ")\n";
//		int field_index_input = 2;	// Magic number as default.
//		std::cin >> field_index_input;

//		// Select analysis
//		std::cout << "\nAnalyze field using:\n0 Maximum likelihood Normal distribution\n1 Maximum likelihood GMM\n";
//		int analysis_input = 0;
//		std::cin >> analysis_input;
//		_ensemble.analyse_field(field_index_input, Ensemble::Analysis(analysis_input));


//		// Select renderer
//		auto vis = std::unique_ptr<Visualisation>{};
//		std::cout << "\nRender result using:\n0 Heightfield renderer\n1 Glyph renderer\n";
//		int renderer_input = 0;
//		std::cin >> renderer_input;
//		switch (renderer_input)
//		{
//		case 0:
//			if(static_cast<Ensemble::Analysis>(analysis_input) == Ensemble::Analysis::GAUSSIAN_SINGLE)
//				vis = std::make_unique<Heightfield>(input, _ensemble.fields());
//			else
//				vis = std::make_unique<HeightfieldGMM>(input, _ensemble.fields());
//			break;
//		case 1:
//			if(static_cast<Ensemble::Analysis>(analysis_input) == Ensemble::Analysis::GAUSSIAN_SINGLE)
//				vis = std::make_unique<Glyph>(input, _ensemble.fields());
//			else
//				vis = std::make_unique<GlyphGMM>(input, _ensemble.fields());
//			break;
//		default:
//			// error
//			break;
//		}
//		vis->setup();

		auto test = 0;
		auto question = 0;

		auto vis = std::unique_ptr<Visualisation>{};
		std::cout << "\nTest Nr.: ";
		std::cin >> test;

		auto timestep = study_timestep(test, question);
		auto analysis = study_analysis(question);
		auto visualisation = study_visualisation(test, question);
		auto highlight = study_data(test, question);
		bool init = false;

		// OpenGL & window state
		glClearColor(.1f, .1f, .1f, 1.f);

		auto time = glfwGetTime();
		_delta = 0.0;

		GLenum polygon_mode = GL_FILL;
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

			auto enter_in = input.release_get_key(GLFW_KEY_ENTER);
			auto backspace_in = input.release_get_key(GLFW_KEY_BACKSPACE);
			if(!init || enter_in || backspace_in)
			{
				if(enter_in)
					question = (question + 1) % 24;
				if(backspace_in)
					question = (question + 23) % 24;

				auto new_timestep = study_timestep(test, question);
				auto new_analysis = study_analysis(question);
				auto new_visualisation = study_visualisation(test, question);

				if(!init || new_timestep != timestep)
					_ensemble.read_headers(new_timestep, 1, 1);
				if(!init || new_timestep != timestep || new_analysis != analysis)
					_ensemble.analyse_field(2, new_analysis);
				if(!init || new_visualisation != visualisation)
					switch(new_visualisation)
					{
					case 0:
						vis = std::make_unique<Heightfield>(input, _ensemble.fields());
						break;
					case 1:
						vis = std::make_unique<Glyph>(input, _ensemble.fields());
						break;
					case 2:
						vis = std::make_unique<HeightfieldGMM>(input, _ensemble.fields());
						break;
					case 3:
						vis = std::make_unique<GlyphGMM>(input, _ensemble.fields());
						break;
					case 4:
						vis = std::make_unique<GlyphGMM>(input, _ensemble.fields(), true);
						break;
					}
				if(!init || new_timestep != timestep || new_analysis != analysis || new_visualisation != visualisation)
					vis->setup();
				timestep = new_timestep;
				analysis = new_analysis;
				visualisation = new_visualisation;
				highlight = study_data(test, question);

				init = true;
			}

			if(input.release_get_key(GLFW_KEY_L))
			{
				if(polygon_mode == GL_FILL)
					polygon_mode = GL_LINE;
				else
					polygon_mode = GL_FILL;
				glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
			}


			if(static_cast<int>(time*3) % 4 <= 2)
				vis->set_highlight_area(std::get<0>(highlight));
			else if(static_cast<int>(time*3) % 4 > 2)
				vis->set_highlight_area(std::get<1>(highlight));
			else vis->set_highlight_area(glm::ivec4{-10});

			vis->update(_delta, static_cast<float>(time));
			vis->draw();

			statusline.set_viewport(input.get_framebuffer_size());
			statusline.set_lines({/*std::to_string(1.f/_delta) + */" Cursor (" + std::to_string(vis->point_under_cursor().x) + ", " + std::to_string(vis->point_under_cursor().y) + ") Highlight (" + std::to_string(std::get<0>(highlight).x) + ", " + std::to_string(std::get<0>(highlight).y)
								  + ") Question " + std::to_string(question)
								 + " mean " + std::to_string(_ensemble.fields().at(0).get_value(0, vis->point_under_cursor().x, vis->point_under_cursor().y, 0)) +
								 " dev " + std::to_string(_ensemble.fields().at(1).get_value(0, vis->point_under_cursor().x, vis->point_under_cursor().y, 0))});
			statusline.set_positions({glm::vec2{-1.f, 1.f - statusline.relative_sizes().front().y}});
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

	int Application::study_timestep(int test, int question)
	{
		int steps[] = {512, 1440};
		if(question < 12)
			return steps[(question/3 + test) % 2];
		else
			return steps[(question/4 + test) % 2];
	}
	std::tuple<glm::ivec4, glm::ivec4> Application::study_data(int test, int question)
	{
		if(question < 12)
		{
			std::tuple<glm::ivec4, glm::ivec4> data[] = {
				{{160,  64, 180,  80}, {160,  64, 180,  80}},	// Region
				{{175,  67, 175,  67}, {175,  67, 175,  67}},	// Single point
				{{175,  61, 175,  61}, {173,  60, 173,  60}},	// Point pair

				{{ 76,  30, 105,  51}, { 76,  30, 105,  51}},	// Region
				{{103,  36, 103,  36}, {103,  36, 103,  36}},	// Single point
				{{ 81,  45,  81,  45}, { 84,  47,  84,  47}},	// Point pair

				{{  0,  35,  45,  55}, {  0,  35,  45,  55}},	// Region
				{{  5,  53,   5,  53}, {  5,  53,   5,  53}},	// Single point
				{{ 34,  43,  34,  43}, { 34,  50,  34,  50}},	// Point pair

				{{ 56,  68,  85,  83}, { 56,  68,  85,  83}},	// Region
				{{ 65,  80,  65,  80}, { 65,  80,  65,  80}},	// Single point
				{{ 67,  70,  67,  70}, { 70,  71,  70,  71}}};	// Point pair
			return data[(question + test/2*3) % 12];
		}
		else
		{
			std::tuple<glm::ivec4, glm::ivec4> data[] = {
				{{ 76,  30, 105,  51}, { 76,  30, 105,  51}},	// Region
				{{ 98,  51,  98,  51}, { 98,  51,  98,  51}},	// Single point
				{{ 82,  34,  82,  34}, { 82,  34,  82,  34}},	// Single point
				{{ 90,  46,  90,  46}, { 91,  47,  91,  47}},	// Point pair

				{{  0,  35,  45,  55}, {  0,  35,  45,  55}},	// Region
				{{  9,  50,   9,  50}, {  9,  50,   9,  50}},	// Single point
				{{  2,  49,   2,  49}, {  2,  49,   2,  49}},	// Single point
				{{  7,  49,   7,  49}, {  8,  49,   8,  49}},	// Point pair

				{{ 56,  68,  85,  83}, { 56,  68,  85,  83}},	// Region
				{{ 65,  80,  65,  80}, { 65,  80,  65,  80}},	// Single point
				{{ 75,  80,  75,  80}, { 75,  80,  75,  80}},	// Single point
				{{ 67,  70,  67,  70}, { 70,  71,  70,  71}}};	// Point pair
			return data[(question + test/2*4) % 12];
		}
	}
	Ensemble::Analysis Application::study_analysis(int question)
	{
		if(question < 12)
			return Ensemble::Analysis::GAUSSIAN_SINGLE;
		else
			return Ensemble::Analysis::GAUSSIAN_MIXTURE;
	}
	int Application::study_visualisation(int test, int question)
	{
		if(question < 12)
			return (question/6 + test/2) % 2;
		else
		{
			int vis[][3] = {{0, 1, 2},
							{0, 2, 1},
							{1, 0, 2},
							{1, 2, 0},
							{2, 0, 1},
							{2, 1, 0}};
			return 2 + vis[test/2][question/4];
		}
	}
}
