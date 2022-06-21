// ArcadeEmulator.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include "src/spaceinvaders.h"
#include "src/cpu/i8080.h"
#include "src/memory.h"

#include <SDL2/SDL.h>

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not be initialized: " <<
            SDL_GetError();
    }

    ae::spaceinvaders si;
    si.init();
    si.run();
    return 0;
}

