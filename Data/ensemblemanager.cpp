#include "ensemblemanager.h"

#include <algorithm>
#include <fstream>

#include "logger.h"

namespace vis
{
	EnsembleManager::EnsembleManager(fs::path path)
	{
		auto comparator = [] (const fs::directory_entry& a, const fs::directory_entry& b) -> bool {return countFiles(a.path()) != countFiles(b.path());};
		if(!fs::is_directory(path)
				|| (_numSims = countDirs(path)) == 0
				|| std::adjacent_find(fs::directory_iterator{path}, fs::directory_iterator{}, comparator) != fs::directory_iterator{})
		{
			//DBG
			Logger::instance() << Logger::Severity::DEBUG
							   << "Ensemble root is not a directory or has the wrong filestructure. "
							   << "Path: " << path
							   << std::endl;

			throw std::runtime_error("Ensemble Manager Error");
			// TODO:ERROR handling. Path is not a directory, has no subdirectories or the subdirectories have a different number of files.
		}

		// Copy all directories
		std::copy_if(fs::recursive_directory_iterator{path}, fs::recursive_directory_iterator{}, std::back_inserter(_files),
					 [] (const fs::path& p) { return fs::is_regular_file(p); });
		// Sort by path (simulation), then stably by name (timestep)
		std::sort(_files.begin(), _files.end());
		std::stable_sort(_files.begin(), _files.end(),
						 [] (const fs::path& a, const fs::path& b) { return a.filename() < b.filename(); });
	}

	void EnsembleManager::processSingleStep(unsigned index)
	{
		auto stepsPerSim = static_cast<unsigned>(_files.size() / _numSims);

		if(index >= stepsPerSim)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Index of step to process is out of range: "
							   << "#steps:" << stepsPerSim << " index:" << index << std::endl;
			throw std::runtime_error("Processing Step Error");
			// TODO:ERROR handling. Index out of range.
		}

		auto buffer = std::vector<Timestep>{};
		buffer.reserve(_numSims);

		for(unsigned i = index*_numSims; i < (index+1)*_numSims; ++i)
		{
			//DBG
			Logger::instance() << Logger::Severity::DEBUG
							   << "Loading file. Index: " << i
							   << " Simulation: " << i/_numSims
							   << " Path: " << _files.at(i) << std::endl;

			auto ifs = std::ifstream{_files[i]};
			buffer.push_back(Timestep{ifs});
		}
		_buffer = Timestep::gaussianAnalysis(buffer);
	}

	Timestep& EnsembleManager::currentStep()
	{
		return _buffer;
	}


	unsigned EnsembleManager::countDirs(fs::path path)
	{
		if(!fs::is_directory(path))
		{
			throw std::runtime_error("Count Dirs Error");
			//TODO:ERROR handling. Path is not a directory.
		}
		return static_cast<unsigned>(std::count_if(fs::directory_iterator(path), fs::directory_iterator{},
												   [] (const fs::path& path) {return fs::is_directory(path);} ));
	}

	unsigned EnsembleManager::countFiles(fs::path path)
	{
		if(!fs::is_directory(path))
		{
			throw std::runtime_error("Count Files Error");
			//TODO:ERROR handling. Path is not a directory.
		}
		return static_cast<unsigned>(std::count_if(fs::directory_iterator(path), fs::directory_iterator{},
												   [] (const fs::path& path) {return fs::is_regular_file(path);} ));
	}
}
