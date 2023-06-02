#include "Core/Runtime.h"

#include <iostream>

int main(int argc, char* argv[])
{
	Cosmos::Runtime* app = new Cosmos::Runtime();
	app->Run();
	delete app;

	return 0;
}