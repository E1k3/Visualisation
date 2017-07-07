#ifndef ENSEMBLE_H
#define ENSEMBLE_H

#include <experimental/filesystem>
#include <vector>

#include "field.h"

namespace vis
{
	namespace fs = std::experimental::filesystem;
	class Ensemble
	{
	public:
		explicit Ensemble(const fs::path& root);

		void read_headers(int step_index);

	private:
		static int count_files(const fs::path& dir);
		static int count_directories(const fs::path& dir);

		std::vector<fs::path> _files;

		std::vector<Field> _fields;
	};
}
#endif // ENSEMBLE_H
