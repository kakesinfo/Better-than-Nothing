#include "CRuntime.hpp"

#include <BetterThanNothing.hpp>

namespace BetterThanNothing
{
	void CRuntime::OnEnable(CApplicationContext* context)
	{
		context->GetRenderer()->AddPipeline([&](VulkanPipelineBuilder& builder) {
			builder.SetName("main");
			builder.AddVertexShader("main/main.vert");
			builder.AddFragmentShader("main/main.frag");
		});
	}

	void CRuntime::OnUpdate(CApplicationContext* context)
	{
		(void) context;
	}

	void CRuntime::OnRender(CApplicationContext* context)
	{
		(void) context;
	}

	void CRuntime::OnDisable(CApplicationContext* context)
	{
		(void) context;
	}
};

BetterThanNothing::CApplication* CreateApplication()
{
	return new BetterThanNothing::CRuntime();
}
