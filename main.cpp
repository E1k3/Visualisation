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

using namespace std::literals::string_literals;
using namespace vis;

void glfw_error_callback(int error, const char* description)
{
	Logger::instance() << Logger::Severity::ERROR << "GLFW ERROR: "s << error << " " << description << std::endl;

	// TODO:ERROR handling
	std::runtime_error("GLFW ERROR"s);
}

int main(int argc, char *argv[])
{
	// Data root directory
	auto path = fs::path{"/home/eike/CurrentStuff/bachelor/weatherdata"s};

	auto ensemblemngr = EnsembleManager(path);
	ensemblemngr.processSingleStep(0);
	ensemblemngr.processSingleStep(2);
	ensemblemngr.processSingleStep(4);


	// OpenGL context and window creation
	glfwSetErrorCallback(glfw_error_callback);

	if(!glfwInit())
	{
		Logger::instance() << Logger::Severity::ERROR << "GLFW init failed" << std::endl;
		// TODO:ERROR handling
		std::runtime_error("GLFW init failed");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1000, 1000, "Test", NULL, NULL);

	if(!window)
	{
		Logger::instance() << Logger::Severity::ERROR << "GLFW window creation failed" << std::endl;
		// TODO:ERROR handling
		std::runtime_error("GLFW window creation failed");
	}
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	GLenum status = glewInit();
	if(status != GLEW_OK)
	{
		Logger::instance() << Logger::Severity::ERROR << "GLEW init failed: "
						   << reinterpret_cast<const char*>(glewGetErrorString(status)) << std::endl;
		// TODO:ERROR handling
		std::runtime_error("GLEW init failed");
	}


	// Upload to GPU


	// Draw stuff


	// DESTRUCTION!!
	glfwDestroyWindow(window);
	glfwTerminate();
}
