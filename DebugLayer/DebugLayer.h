#pragma once
#include <Support/WinInclude.h>

class DXDebugLayer
{
public:
	bool Init();
	void ShutDown();

private:
#ifdef _DEBUG
	ComPtr<ID3D12Debug6> m_D3D12Debug;
	ComPtr<IDXGIDebug1> m_DXGIDebug;
#endif // _DEBUG

public:
	DXDebugLayer(const DXDebugLayer&)            = delete;
	DXDebugLayer& operator=(const DXDebugLayer&) = delete;

	inline static DXDebugLayer& Get()
	{
		static DXDebugLayer Instance;
		return Instance;
	}

private:
	DXDebugLayer() = default;
};