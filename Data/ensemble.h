#ifndef ENSEMBLE_H
#define ENSEMBLE_H

#include <string>
#include <experimental/filesystem>

#include "timestep.h"

namespace vis
{
	namespace fs = std::experimental::filesystem;
	class Ensemble
	{
	public:
		explicit Ensemble(fs::path& path);
		explicit Ensemble(std::string& path);

	private:

	};
}

#endif // ENSEMBLE_H
