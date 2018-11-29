#include <iostream>
#include "simulation.h"

int main()
{
	try
	{
		Simulation simulation;
		simulation.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
