// tests.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include "cpu/i8080_tests.h"
#include "cpu/z80_tests.h"

int main()
{
	tests_ae::cpu::z80_tests test;
	test.run();
}
