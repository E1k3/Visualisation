#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <experimental/filesystem>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Data/ensemblemanager.h"
#include "logger.h"
#include "renderer.h"
#include "heightfieldrenderer.h"

using namespace std::literals::string_literals;
using namespace vis;

[[noreturn]] void glfw_error_callback(int error, const char* description)
{
	Logger::instance() << Logger::Severity::ERROR << "GLFW ERROR: " << error << " " << description << std::endl;

	// TODO:ERROR handling
	throw std::runtime_error("GLFW ERROR");
}

void glfw_framebuffsize_callback(GLFWwindow* window, int width, int height)
{
	(void)window; // UNUSED
	glViewport(0, 0, width, height);
}

void glfw_curserpos_callback(GLFWwindow* window, double x, double y)
{
	(void)window;	// UNUSED

}

int main(int argc, char *argv[])
{
	(void)argc;	// UNUSED
	(void)argv;	// UNUSED

	// Data root directory
	auto path = fs::path{"/home/eike/CurrentStuff/bachelor/weatherdata"};

	auto ensemblemngr = EnsembleManager(path);
	ensemblemngr.processSingleStep(256);
	Timestep& step = ensemblemngr.currentStep();
	step.normaliseAll();

	// OpenGL context and window creation
	glfwSetErrorCallback(glfw_error_callback);

	if(!glfwInit())
	{
		Logger::instance() << Logger::Severity::ERROR << "GLFW init failed" << std::endl;
		// TODO:ERROR handling
		throw std::runtime_error("GLFW init failed");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1000, 1000, "Test", NULL, NULL);

	if(!window)
	{
		Logger::instance() << Logger::Severity::ERROR << "GLFW window creation failed" << std::endl;
		// TODO:ERROR handling
		throw std::runtime_error("GLFW window creation failed");
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, glfw_framebuffsize_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, glfw_curserpos_callback);

	glewExperimental = GL_TRUE;
	GLenum status = glewInit();
	if(status != GLEW_OK)
	{
		Logger::instance() << Logger::Severity::ERROR << "GLEW init failed: "
						   << reinterpret_cast<const char*>(glewGetErrorString(status)) << std::endl;
		// TODO:ERROR handling
		throw std::runtime_error("GLEW init failed");
	}


	HeightfieldRenderer renderer{ensemblemngr};
	glClearColor(0.f, 0.f, 0.f, 1.f);

	while(!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		renderer.draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}
