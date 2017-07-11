#ifndef ENSEMBLE_H
#define ENSEMBLE_H

#include <experimental/filesystem>
#include <vector>
#include <istream>

#include "field.h"

namespace vis
{
	namespace fs = std::experimental::filesystem;
	class Ensemble
	{
	public:
		enum class Analysis
		{
			GAUSSIAN_SINGLE = 0,
			GAUSSIAN_MIXTURE
		};

		explicit Ensemble(const fs::path& root);

		int num_steps() const;
		int num_simulations() const;

		const std::vector<Field>& fields() const;

		void read_headers(int step_index);

		void analyse_field(int step_index, int field_index, Analysis analysis);

	private:
		static void ignore_many(std::istream& stream, int count, char delimiter);

		static std::vector<Field> gaussian_analysis(const std::vector<Field>& fields);
		static std::vector<Field> gaussian_mixture_analysis(const std::vector<Field>& fields);

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
