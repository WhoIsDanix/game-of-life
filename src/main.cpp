#include "Game.h"

int main(int argc, char** argv) {
    Game game;

    if (!game.Initialize(1024, 600)) {
        SDL_Log("Something went wrong when initializing the game");
        return -1;
    }

    game.Start();
    return 0;
}