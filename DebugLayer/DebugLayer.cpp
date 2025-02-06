#include "DebugLayer.h"

bool DXDebugLayer::Init()
{
#ifdef _DEBUG

	if (SUCCEEDED(D3D12GetDebugInterface(__uuidof(ID3D12Debug6), reinterpret_cast<void**>(m_D3D12Debug.GetAddressOf()))))
	{
		m_D3D12Debug->EnableDebugLayer();
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(m_DXGIDebug.GetAddressOf()))))
		{
			m_DXGIDebug->EnableLeakTrackingForThread();
			return true;
		}
	}
#endif // _DEBUG
	return false;
}

void DXDebugLayer::ShutDown()
{
#ifdef _DEBUG

	if (m_DXGIDebug)
	{
		OutputDebugStringW(L"Living object: \n");
		m_DXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	}
	m_DXGIDebug.Reset();
	m_D3D12Debug.Reset();

#endif // DEBUG
}