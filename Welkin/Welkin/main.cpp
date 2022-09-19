#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "Game.h"
#include "Helper.h"
#include <cstdlib>
#include<iostream>
#include <stdexcept>

int main()
{
	Game game{};

	try
	{
		Helper::Cout("Update", true);
		game.Update();
	}
	catch(const std::exception &e)
	{
		std::cerr << e.what() << endl;
		_CrtDumpMemoryLeaks();
		return EXIT_FAILURE;
	}

	_CrtDumpMemoryLeaks();
	return EXIT_SUCCESS;
}