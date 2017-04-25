#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <experimental/filesystem>
#include "Data/timestep.h"
#include "logger.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace fs = std::experimental::filesystem;
using namespace std::literals::string_literals;
using namespace vis;

unsigned numDirs(fs::path p)
{
	return unsigned(std::count_if(fs::directory_iterator(p), fs::directory_iterator{},
								  [] (const fs::path& path) {return fs::is_directory(path);} ));
}

void glfw_error_callback(int error, const char* description)
{
	Logger::instance() << Logger::Severity::ERROR << "GLFW ERROR: "s << error << " " << description << std::endl;

	//TODO:ERROR handling
	std::runtime_error("GLFW ERROR"s);
}

int main(int argc, char *argv[])
{
	//Data root directory
	auto path = fs::path{"/home/eike/CurrentStuff/bachelor/weatherdata"s};

	if(fs::is_directory(path))
	{
		auto dataFiles = std::vector<fs::path>{};
		//Copy all directories
		std::copy_if(fs::recursive_directory_iterator{path}, fs::recursive_directory_iterator{}, std::back_inserter(dataFiles),
					 [] (const fs::path& p) { return fs::is_regular_file(p); });
		//Sort by path, then stably by name
		std::sort(dataFiles.begin(), dataFiles.end());
		std::stable_sort(dataFiles.begin(), dataFiles.end(),
						 [] (const fs::path& a, const fs::path& b) { return a.filename() < b.filename(); });

		auto tstepBuffer = std::vector<Timestep>(numDirs(path));
		unsigned numStepsPerSim = unsigned(dataFiles.size()/numDirs(path));
		auto analyzedSteps = std::vector<std::unique_ptr<Timestep>>();
		//analyzedSteps.reserve(numStepsPerSim);
		for(unsigned file = 0; file < numStepsPerSim; ++file)
		{
			for(unsigned dir = 0; dir < numDirs(path); ++dir)
			{
				auto ifs = std::ifstream{dataFiles[file*numDirs(path) + dir]};
				tstepBuffer[dir] = Timestep{ifs};
				Logger::instance() << Logger::Severity::DEBUG
								   << dataFiles[file*numDirs(path) + dir]
								   << "\n";
			}
			analyzedSteps.push_back(std::make_unique<Timestep>(tstepBuffer));
		}
	}


	//OpenGL context and window creation
	glfwSetErrorCallback(glfw_error_callback);

	if(!glfwInit())
	{
		Logger::instance() << Logger::Severity::ERROR << "GLFW init failed" << std::endl;
		//TODO:ERROR handling
		std::runtime_error("GLFW init failed");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1000, 1000, "Test", NULL, NULL);

	if(!window)
	{
		Logger::instance() << Logger::Severity::ERROR << "GLFW window creation failed" << std::endl;
		//TODO:ERROR handling
		std::runtime_error("GLFW window creation failed");
	}
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	GLenum status = glewInit();
	if(status != GLEW_OK)
	{
		Logger::instance() << Logger::Severity::ERROR << "GLEW init failed: "
						   << reinterpret_cast<const char*>(glewGetErrorString(status)) << std::endl;
		//TODO:ERROR handling
		std::runtime_error("GLEW init failed");
	}


	//Upload to GPU


	//Draw stuff


	//DESTRUCTION!!
	glfwDestroyWindow(window);
	glfwTerminate();
}
