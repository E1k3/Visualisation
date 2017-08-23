#define GSL_THROW_ON_CONTRACT_VIOLATION

#include "application.h"
#include "logger.h"


using namespace vis;

int main(int /*argc*/, char */*argv*/[])
{
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
