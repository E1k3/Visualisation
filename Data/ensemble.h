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

		void read_headers(int step_index, int count = 0, int stride = 0);

		void analyse_field(int field_index, Analysis analysis);

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

		int _selected_step;
		int _cluster_stride;
		int _cluster_size;

		std::vector<Field> _fields{};

		std::vector<fs::path> _project_files{};
	};
}
#endif // ENSEMBLE_H
