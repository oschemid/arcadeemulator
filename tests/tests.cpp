// tests.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include "cpu/i8080_tests.h"

int main()
{
	tests_ae::cpu::i8080_tests test;
	test.run();
}
