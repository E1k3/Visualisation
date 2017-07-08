#include "ensemble.h"

#include <exception>
#include <algorithm>

#include "logger.h"

namespace vis
{
	Ensemble::Ensemble(const fs::path& root)
	{
		_num_simulations = count_directories(root);
		auto not_equal = [] (const auto& a, const auto& b) { return count_files(a) != count_files(b); };
		if(_num_simulations <= 0
				|| std::adjacent_find(fs::directory_iterator{root}, fs::directory_iterator{}, not_equal)
				!= fs::directory_iterator{})	// Search for a subdirectory of root with different amount of files
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Ensemble root directory does not contain subdirectories of the same size.\n"
							   << "Path: " << root;

			throw std::invalid_argument("Path does not stick to ensemble directory structure");
		}
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
