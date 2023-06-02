#include "Runtime.h"

namespace Cosmos
{
	Runtime::Runtime()
	{
		LOG_TRACE("This is the tracing log");
		LOG_INFO("This is the info log");
		LOG_DEBUG("This is the debug log");
		LOG_WARNING("This is the warning log");
		LOG_ERROR("This is the error log");

		LOG_ASSERT(true, "Assertion was triggered");
	}

	Runtime::~Runtime()
	{

	}

	void Runtime::OnInitialize()
	{
	}

	void Runtime::OnUpdate(f32 timestep)
	{
	}

	void Runtime::OnRender(f32 timestep)
	{
	}

	void Runtime::OnTerminate()
	{
	}
}