#ifndef ANALYZERINTERFACE_H
#define ANALYZERINTERFACE_H

#include <memory>
#include <vector>


namespace vis
{
	class Timestep;
	class AnalyzerInterface
	{
	public:
		virtual std::unique_ptr<Timestep> analyze(std::vector<Timestep>& tsteps) = 0;
	};
}

#endif // ANALYZERINTERFACE_H
