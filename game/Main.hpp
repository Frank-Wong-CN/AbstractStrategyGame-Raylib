#include "GameContents.hpp"

static GameController *game;

void Start(int argc, char *argv[])
{
	conf::SetGameTitle("Abstract Strategy Game");
	conf::SetScreenSize(800, 800);
	game = new GameController();
	game->SetCommandlineArguments(argc, argv);
	gm::AddInstance(game);
}

void End()
{
	gm::RemoveInstance(game->id);
	delete game;
}