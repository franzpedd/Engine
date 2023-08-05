#include "Core/Editor.h"

int main(int argc, char* argv[])
{
	Cosmos::Editor* editor = new Cosmos::Editor();
	editor->Run();
	delete editor;
	return 0;
}