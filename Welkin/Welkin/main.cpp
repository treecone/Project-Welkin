#include "Game.h"

#include <cstdlib>
#include<iostream>
#include <stdexcept>

int main()
{
	Game game{};

	try
	{
		game.Update();
	}
	catch(const std::exception &e)
	{
		std::cerr << e.what() << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}