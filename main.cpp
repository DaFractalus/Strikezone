#include "GameEngine.hpp"

int main()
{
	std::cout << "main() is on! Quickly gonna start-up the game...\n";
	std::cout << "		| STRIKEZONE |\n\n";

	int exitCode = -1;
	{
		auto game = Engine::GameEngine(true);
		exitCode = game.Run();
	}

	std::cout << "\n ~ Farewell ...\n";
	return exitCode;
}