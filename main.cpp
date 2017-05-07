#include <string>

#include "application.h"

using namespace vis;

int main(int argc, char *argv[])
{
	(void)argc;	// UNUSED
	(void)argv;	// UNUSED

	// Data root directory
	auto app = Application{"/home/eike/CurrentStuff/bachelor/weatherdata"};
	app.run();
}
