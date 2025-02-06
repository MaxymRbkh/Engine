#include <iostream>
#include <DebugLayer/DebugLayer.h>
#include <DXContext/DXContext.h>
#include <support/Window.h>
#include <support/WinInclude.h>

int main()
{
	if (DXDebugLayer::Get().Init())
	{
		std::cout << "PIDATAAS";
	}
	if (DXContext::Get().Init() && Window::Get().Init())
	{
		while (!Window::Get().ShouldClose())
		{
			Window::Get().Update();
			if (Window::Get().ShouldResize())
			{
				DXContext::Get().Flush(Window::Get().getFrameCount());
				Window::Get().Resize();
			}
			auto* cmdList = DXContext::Get().InitCommandList();
			Window::Get().BeginFrame(cmdList);
			Window::Get().EndFrame(cmdList);
			DXContext::Get().ExecuteCommandList();
			Window::Get().Preset();
		}
		DXContext::Get().Flush(Window::Get().getFrameCount());
		Window::Get().Shutdown();
		DXContext::Get().Shutdown();
	}
}

