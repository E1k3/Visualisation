#ifndef TIMESTEPANALYZER_H
#define TIMESTEPANALYZER_H

#include <memory>

#include "analyzerinterface.h"

namespace vis
{
	class GaussianAnalyzer : public AnalyzerInterface
	{
	public:
		/**
		 * @brief analyze Analyzes timesteps in @param testeps and stores result in a
		 * @param tsteps
		 * @return
		 */
		virtual std::unique_ptr<Timestep> analyze(std::vector<Timestep>& tsteps);

		virtual ~GaussianAnalyzer();
	private:
		explicit GaussianAnalyzer();
	};
}

#endif // TIMESTEPANALYZER_H
