#ifndef ENSEMBLEMANAGER_H
#define ENSEMBLEMANAGER_H

#include <string>
#include <experimental/filesystem>
#include <vector>

#include "timestep.h"

namespace vis
{
	namespace fs = std::experimental::filesystem;
	class EnsembleManager
	{		
	public:
		explicit EnsembleManager(fs::path path);

		/**
		 * @brief processSingleStep Loads one timestep from all simulations,
		 * analyses the data and stores the result in a buffer.
		 * The buffer can be obtained by calling currentStep().
		 * @param stepIndex Specifies which timestep is being loaded.
		 */
		void processSingleStep(unsigned index);

		/**
		 * @brief currentStep Getter for the current timestep.
		 * @return Reference to the currently buffered step.
		 */
		Timestep& currentStep();

	private:
		/**
		 * @brief countDirs Counts number of directories present in the path.
		 * @param path The path that will be searched in.
		 */
		static unsigned countDirs(fs::path path);

		/**
		 * @brief countDirs Counts number of files present in the path.
		 * @param path The path that will be searched in.
		 */
		static unsigned countFiles(fs::path path);

		/// Number of simulations that are part of this ensemble.
		unsigned _numSims{0};

		/// Contains paths to all files that are part of the ensemble.
		/// Sorted with time as primary and simulation as secondary key.
		std::vector<fs::path> _files{};

		/// Buffer to store the analyzed timestep.
		/// TODO:maybe add size management/change to circular buffer?
		Timestep _buffer{};
	};
}

#endif // ENSEMBLEMANAGER_H
