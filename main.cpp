#include <string>

#include "application.h"

using namespace vis;

int main(int /*argc*/, char */*argv*/[])
{
	// Data root directory
	auto app = Application{"/home/eike/CurrentStuff/bachelor/weatherdata"};
	app.execute();
}
