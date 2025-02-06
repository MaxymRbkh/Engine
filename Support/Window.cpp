#include "Window.h"

bool Window::Init()
{
	WNDCLASSEXW wcExW{};
	wcExW.cbSize = sizeof(wcExW);
	wcExW.hInstance = GetModuleHandleW(nullptr);
	wcExW.lpszClassName = _T("Pidarok");
	wcExW.cbClsExtra = 0;
	wcExW.cbWndExtra = 0;
	wcExW.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcExW.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcExW.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wcExW.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	wcExW.lpfnWndProc = &Window::WndProc;
	wcExW.lpszMenuName = NULL;
	wcExW.style = CS_OWNDC;
	w_wndClass = RegisterClassExW(&wcExW);
	if (w_wndClass == 0)
	{
		std::cerr << "BBBBBBBBBBBB";
		return 0;
	}

	POINT pt;
	GetCursorPos(&pt);
	HMONITOR monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorInfo{};
	monitorInfo.cbSize = sizeof(monitorInfo);
	GetMonitorInfoW(monitor, &monitorInfo);

	w_window = CreateWindowExW(
		WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW,
		(LPCWSTR)wcExW.lpszClassName,
		_T("Arthas"),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		monitorInfo.rcMonitor.left + 0,
		monitorInfo.rcMonitor.top + 0,
		w_width,
		w_height,
		nullptr,
		nullptr,
		wcExW.hInstance,
		nullptr);

	if (w_window == 0)
	{
		return false;
	}
	
	DXGI_SWAP_CHAIN_DESC1 scd;
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC scf;
	scd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	scd.BufferCount = getFrameCount();
	scd.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.Width = w_width;
	scd.Height = w_height;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.Scaling = DXGI_SCALING_STRETCH;
	scd.Stereo = false;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scf.Windowed = true;

	auto& factory = DXContext::Get().GetFactory();
	ComPtr<IDXGISwapChain1> sc1;
	factory->CreateSwapChainForHwnd(DXContext::Get().GetCommandQueue().Get(), w_window, &scd, &scf, nullptr, &sc1);
	if (FAILED(sc1->QueryInterface(IID_PPV_ARGS(w_swapChain.GetAddressOf()))))
	{
		return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC dhd;
	dhd.NodeMask = 0;
	dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dhd.NumDescriptors = FrameCount;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	DXContext::Get().GetDevice()->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&w_descHeap));
	auto firstDesc = w_descHeap->GetCPUDescriptorHandleForHeapStart();
	auto increment = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (size_t i = 0; i < FrameCount; i++)
	{
		w_cpuHandle[i] = firstDesc;
		w_cpuHandle[i].ptr += increment * i;
	}

	if (!GetBuffers())
	{
		return false;
	}
	return true;
}

void Window::Preset()
{
	w_swapChain->Present(1, 0);
}

void Window::Update()
{
	MSG msg{};
	while (PeekMessageW(&msg, w_window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

void Window::Shutdown()
{
	ReleaseBuffers();
	w_swapChain.Reset();
	w_descHeap.Reset();
	if (w_window)
	{
		DestroyWindow(w_window);
	}
	if (w_wndClass)
	{
		UnregisterClassW((LPCWSTR)w_wndClass, GetModuleHandleW(nullptr));
	}
}

void Window::Resize()
{
	ReleaseBuffers();
	RECT rc;
	if (GetClientRect(w_window, &rc))
	{
		w_width = rc.right - rc.left;
		w_height = rc.bottom - rc.top;

		w_swapChain->ResizeBuffers(FrameCount, w_width, w_height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
		w_shouldResize = false;
	}
	GetBuffers();
}

void Window::ReleaseBuffers()
{
	for (size_t i = 0; i < FrameCount; i++)
	{
		w_buffers[i].Reset();
	}
}


void Window::SetFullscreen(bool enabled)
{
	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	DWORD exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;
	if (enabled)
	{
		style = WS_POPUP | WS_VISIBLE;
		exStyle = WS_EX_APPWINDOW;
	}
	SetWindowLongW(w_window, GWL_STYLE, style);
	SetWindowLongW(w_window, GWL_EXSTYLE, exStyle);

	if (enabled)
	{
		HMONITOR monitor = MonitorFromWindow(w_window, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo{};
		monitorInfo.cbSize = sizeof(monitorInfo);
		if (GetMonitorInfoW(monitor, &monitorInfo))
		{
			SetWindowPos(
				w_window,
				nullptr,
				monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.top,
				monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
				SWP_NOZORDER
			);
		}
	}
	else
	{
		ShowWindow(w_window, SW_MAXIMIZE);
	}
	w_isFullscreen = enabled;
}

bool Window::GetBuffers()
{
	for (int i = 0; i < FrameCount; i++)
	{
		if (FAILED(w_swapChain->GetBuffer(i, IID_PPV_ARGS(&w_buffers[i]))))
		{
			return false;
		}

		D3D12_RENDER_TARGET_VIEW_DESC rtvd;
		rtvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvd.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvd.Texture2D.MipSlice = 0;
		rtvd.Texture2D.PlaneSlice = 0;
		DXContext::Get().GetDevice()->CreateRenderTargetView(w_buffers[i].Get(), &rtvd, w_cpuHandle[i]);
	}
	return true;
}

void Window::BeginFrame(ID3D12GraphicsCommandList7* cmd_list)
{
	w_currentBufferIndex = w_swapChain->GetCurrentBackBufferIndex();
	D3D12_RESOURCE_BARRIER barr;
	barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barr.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barr.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barr.Transition.pResource = w_buffers[w_currentBufferIndex].Get();
	barr.Transition.Subresource = 0;

	cmd_list->ResourceBarrier(1, &barr);
	float color[4] = { 0.4f, 0.4f, 0.4f, 1.0f };
	cmd_list->ClearRenderTargetView(w_cpuHandle[w_currentBufferIndex], color, 0, nullptr);
	cmd_list->OMSetRenderTargets(1, &w_cpuHandle[w_currentBufferIndex], false, nullptr);
}

void Window::EndFrame(ID3D12GraphicsCommandList7* cmd_list)
{
	D3D12_RESOURCE_BARRIER barr;
	barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barr.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barr.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barr.Transition.pResource = w_buffers[w_currentBufferIndex].Get();
	barr.Transition.Subresource = 0;

	cmd_list->ResourceBarrier(1, &barr);
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wP, LPARAM lP)
{
	switch (msg)
	{
	case WM_SIZE:
		if (lP && (HIWORD(lP) != Get().w_height || LOWORD(lP) != Get().w_width))
		{
			Get().w_shouldResize = true;
		}
		break;

	case WM_CLOSE:
		Get().w_shouldClose = true;
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		if (wP == VK_F11)
		{
			Get().SetFullscreen(!Get().isFullscreen());
		}
		break;
	}
		return DefWindowProcW(hwnd, msg, wP, lP);

		//PIDARAS SVASTIKA 卐 )) 
}