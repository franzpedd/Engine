#include "Dockspace.h"

namespace Cosmos
{
	Dockspace::Dockspace()
	{
		Logger() << "Creating Dockspace";
	}

	Dockspace::~Dockspace()
	{

	}

	void Dockspace::OnUpdate()
	{
		ui::DockspaceOverEverything();
	}

	void Dockspace::OnResize()
	{
	}
}