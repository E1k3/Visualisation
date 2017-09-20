#define GSL_THROW_ON_CONTRACT_VIOLATION

#include "application.h"
#include "logger.h"
#include "Data/math_util.h"
#include <fstream>
#include <algorithm>


using namespace vis;

int main(int /*argc*/, char */*argv*/[])
{
//	auto ifs = std::ifstream{"/home/eike/Documents/Uni/Bachelor/python/data/sample.dat"};
//	auto sample = std::vector<float>{};
//	auto line = std::string{};
//	while(std::getline(ifs, line))
//	{
//		sample.push_back(std::stof(line));
//		std::cout << sample.back() << '\n';
//	}
//	std::cout << '\n';
//	std::sort(sample.begin(), sample.end());
//	auto gmms = math_util::fit_gmm(sample, 4);

//	auto ofs = std::ofstream{"/home/eike/Documents/Uni/Bachelor/python/data/ensemble.dat"};
//	for(const auto& gmm : gmms)
//		if(gmm._weight != 0.f)
//			ofs << gmm._mean << ' ' << std::sqrt(gmm._variance) << ' ' << gmm._weight << '\n';
//	return 0;

	try
	{
		// Data root directory
		auto app = Application{"/home/eike/CurrentStuff/bachelor/weatherdata"};
		app.execute();
	}
	catch(std::exception& e)
	{
		Logger::error() << "Terminating program due to exception: " << e.what();
		return -1;
	}
	return 0;
}
