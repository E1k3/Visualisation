#include "ensemble.h"

#include <exception>
#include <algorithm>

#include "logger.h"

namespace vis
{
	Ensemble::Ensemble(const fs::path& root)
	{

	}

	int Ensemble::count_files(const fs::path& dir)
	{
		if(!fs::is_directory(dir))
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Counting files failed.\n"
							   << dir.string() << " does not point to a directory.";
			throw std::invalid_argument("Path does not point to directory");	// TODO:ERROR handling. Dir is not a directory.
		}
		return static_cast<int>(std::count_if(fs::directory_iterator(dir), fs::directory_iterator{},
											  [] (const auto& path) { return fs::is_regular_file(path); }));
	}

	int Ensemble::count_directories(const fs::path& dir)
	{
		if(!fs::is_directory(dir))
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Counting directories failed.\n"
							   << dir.string() << " does not point to a directory.";
			throw std::invalid_argument("Path does not point to directory");	// TODO:ERROR handling. Dir is not a directory.
		}
		return static_cast<int>(std::count_if(fs::directory_iterator(dir), fs::directory_iterator{},
											  [] (const auto& path) { return fs::is_directory(path); }));
	}
}
