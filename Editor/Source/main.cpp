#include "Core/Editor.h"

int main(int argc, char* argv[])
{
	PROFILER_BEGIN("Startup", "Startup.json");
	Cosmos::Editor* editor = new Cosmos::Editor();
	PROFILER_END();

	// testing
	editor->Initialize();

	PROFILER_BEGIN("Runtime", "Runtime.json");
	editor->Run();
	PROFILER_END();

	PROFILER_BEGIN("End", "End.json");
	delete editor;
	PROFILER_END();

	return 0;
}