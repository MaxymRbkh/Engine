#pragma once
#include "WinInclude.h"
#include <DXContext/DXContext.h>
#include <tchar.h>
#include <iostream>

class Window
{
public:
	bool Init();
	void Update();
	void Shutdown();
	void Preset();
	void Resize();
	void SetFullscreen(bool enabled);
	void BeginFrame(ID3D12GraphicsCommandList7* cmd_list);
	void EndFrame(ID3D12GraphicsCommandList7* cmd_list);

	inline bool ShouldClose() const
	{
		return w_shouldClose;
	}
	inline bool ShouldResize() const
	{
		return w_shouldResize;
	}
	inline bool isFullscreen() const
	{
		return w_isFullscreen;
	}
	inline UINT GetWidth() const
	{
		return w_width;
	}
	inline UINT GetHeight() const
	{
		return w_height;
	}
	
	static constexpr size_t FrameCount = 2;
	static constexpr size_t getFrameCount()
	{
		return FrameCount;
	}

private:
	ATOM w_wndClass;
	HWND w_window;
	UINT64 w_width = 1920;
	UINT64 w_height = 1080;
	bool w_shouldResize = false;
	bool w_isFullscreen = false;
	bool w_shouldClose = false;
	
	ComPtr<IDXGISwapChain3> w_swapChain;
	ComPtr<ID3D12Resource2> w_buffers[FrameCount];	
	size_t w_currentBufferIndex = 0;
	ComPtr<ID3D12DescriptorHeap> w_descHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE w_cpuHandle[FrameCount];

private:
	bool GetBuffers();
	void ReleaseBuffers();
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wP, LPARAM lP);

public:
	Window(const Window&)           = delete;
	Window operator=(const Window&) = delete;
	inline static Window& Get()
	{
		static Window Instance;
		return Instance;
	}

private:
	Window() = default;
};