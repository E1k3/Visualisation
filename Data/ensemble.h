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

		int num_steps() const;
		int num_simulations() const;

		const std::vector<Field>& fields() const;

		void read_headers(int step_index);

	private:
		/// @brief Returns number (>=0) of files in dir. Throws exception if dir is not a directory.
		static int count_files(const fs::path& dir);
		/// @brief Returns number (>=0) of directories in dir. Throws exception if dir is not a directory.
		static int count_directories(const fs::path& dir);

		int _num_simulations;
		int _num_steps;

		std::vector<fs::path> _files{};

		std::vector<Field> _fields{};
	};
}
#endif // ENSEMBLE_H
