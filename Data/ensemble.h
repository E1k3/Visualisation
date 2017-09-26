#ifndef ENSEMBLE_H
#define ENSEMBLE_H

#include <experimental/filesystem>
#include <vector>
#include <istream>

#include "field.h"

namespace vis
{
	namespace fs = std::experimental::filesystem;
	/**
	 * @brief The Ensemble class manages and analyzes ensembles of fields.
	 * Ensemble data can be read from text files.
	 */
	class Ensemble
	{
	public:
		enum class Analysis
		{
			GAUSSIAN_SINGLE = 0,
			GAUSSIAN_MIXTURE
		};

		/**
		 * @brief Ensemble Creates an ensemble from files stored at the root directory.
		 */
		explicit Ensemble(const fs::path& root);

		/**
		 * @brief num_steps Returns the number of time steps that are available.
		 */
		int num_steps() const;
		/**
		 * @brief num_simulations Returns the number of simulation data that is available for each time step.
		 * @return
		 */
		int num_simulations() const;

		/**
		 * @brief fields Returns a reference to the collection of analysis results.
		 */
		const std::vector<Field>& fields() const;
		/**
		 * @brief headers Returns a reference to the collection of layout data of the loaded ensemble.
		 */
		const std::vector<Field>& headers() const;

		/**
		 * @brief read_headers Reads layout data from the files present in the data root directory.
		 * @param step_index Selects which time step the data will be read from.
		 * @param count Selects the number of time steps that will be aggregated into one.
		 * @param stride Selects the offset between each aggregated time step.
		 * To aggregate time steps 0, 4, 8, 12 and 16, step index has to be 0, aggregation count has to be 5 and stride 4.
		 */
		void read_headers(int step_index, int count = 0, int stride = 0);
		/**
		 * @brief analyse_field Analyzes the time steps specified in the last read_headers call of the selected fields.
		 * @param field_index The index of the field that is to be analyzed.
		 * @param analysis The method by which the field will be analyzed.
		 * The data returned by fields afterwards depends on the selected analysis.
		 * GAUSSIAN_SINGLE -> fields()[0] = means, fields()[1] = standard deviations
		 * GAUSSIAN_MIXTRUE -> fields()[0] = means, fields()[1] = standard deviations, fields()[2] = mixture weights
		 */
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

		std::vector<Field> _headers{};
		std::vector<Field> _fields{};

		std::vector<fs::path> _project_files{};
	};
}
#endif // ENSEMBLE_H
