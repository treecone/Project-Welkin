#include <stdlib.h>
#include <crtdbg.h>

#include "Game.h"
#include "Helper.h"
#include <cstdlib>
#include<iostream>
#include <stdexcept>

#define _CRTDBG_MAP_ALLOC

int main()
{
	Game game{};
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	try
	{
		Helper::Cout("Update", true);
		game.Update();
	}
	catch(const std::exception &e)
	{
		std::cerr << e.what() << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}