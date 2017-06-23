#include <string>

#include "application.h"
#include "Data/math_util.h"

using namespace vis;

int main(int /*argc*/, char */*argv*/[])
{
	//math_util::test_em();
	// Data root directory
	auto app = Application{"/home/eike/CurrentStuff/bachelor/weatherdata"};
	app.execute();
}
