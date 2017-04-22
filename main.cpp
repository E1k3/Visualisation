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
unsigned numDirs(fs::path p)
{
	return unsigned(std::count_if(fs::directory_iterator(p), fs::directory_iterator{},
								  [] (const fs::path& path) {return fs::is_directory(path);} ));
}

int main(int argc, char *argv[])
{
	using namespace vis;
	using namespace std::literals::string_literals;

	//Data root directory
	auto path = fs::path{"/home/eike/CurrentStuff/bachelor/weatherdata"s};

	if(fs::is_directory(path))
	{
		auto dataFiles = std::vector<fs::path>{};
		//Copy all directories
		std::copy_if(fs::recursive_directory_iterator(path), fs::recursive_directory_iterator(), std::back_inserter(dataFiles),
					 [] (const fs::path& p) { return fs::is_regular_file(p); });
		//Sort by path, then stably by name
		std::sort(dataFiles.begin(), dataFiles.end());
		std::stable_sort(dataFiles.begin(), dataFiles.end(),
						 [] (const fs::path& a, const fs::path& b) { return a.filename() < b.filename(); });

		auto tstepBuffer = std::vector<Timestep>(numDirs(path));
		unsigned numStepsPerSim = unsigned(dataFiles.size()/numDirs(path));
		auto analyzedSteps = std::vector<std::unique_ptr<Timestep>>();
		analyzedSteps.reserve(numStepsPerSim);
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
}
