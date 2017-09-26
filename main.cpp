#define GSL_THROW_ON_CONTRACT_VIOLATION

#include "application.h"
#include "logger.h"
#include "Data/math_util.h"


using namespace vis;

int main(int argc, char* argv[])
{
	auto project_path = "/home/eike/CurrentStuff/bachelor/weatherdata"; // "data";

	if(argc == 2)
	{
		if(std::string{argv[1]} == std::string{"-h"})
		{
			std::cout << "Usage: " << argv[0] << " [DATA LOCATION]\n"
					  << "If no directory is specified, ./data will be assumed"
					  << std::endl;
			return 0;
		}
		else
			project_path = argv[1];
	}
	else if(argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " [DATA LOCATION]\n"
				  << "If no directory is specified, ./data will be assumed"
				  << std::endl;
		return -1;
	}


	try
	{
		// Data root directory
		auto app = Application{project_path};
		app.execute();
	}
	catch(std::exception& e)
	{
		Logger::error() << "Terminating program due to exception: " << e.what();
		return -1;
	}
	return 0;
}
