// ArcadeEmulator.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>

#include "src/spaceinvaders.h"
#include "src/ui/ui.h"


int main(int argc, char** argv)
{
    // UI creation
    if (!ae::ui::init())
        return 1;

    // Machine selection
    ae::spaceinvaders si;
    si.init();
    
    si.run();

    ae::ui::destroy();
    return 0;
}

