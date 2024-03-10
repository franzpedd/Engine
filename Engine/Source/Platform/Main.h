#pragma once

#include "Core/Application.h"
#include "Debug/Profiler.h"

// export the create application function
extern Cosmos::Application* Cosmos::CreateApplication();

int main(int argc, char* argv[])
{
	PROFILER_BEGIN("Startup", "Startup.json");
	Cosmos::Application* app = Cosmos::CreateApplication();
	PROFILER_END();
	
	PROFILER_BEGIN("Runtime", "Runtime.json");
	app->Run();
	PROFILER_END();
	
	PROFILER_BEGIN("End", "End.json");
	delete app;
	PROFILER_END();

	return 0;
}