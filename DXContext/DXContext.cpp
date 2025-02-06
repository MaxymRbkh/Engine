#include "DXContext.h"

bool DXContext::Init()
{
	if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(m_factory.GetAddressOf()))))
	{
		return false;
	}
	if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(m_device.GetAddressOf()))))
	{
		return false;
	}
	if (FAILED(m_device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(m_cmdList.GetAddressOf()))))
	{
		return false;
	}
	if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_cmdAlloc.GetAddressOf()))))
	{
		return false;
	}
	if (FAILED(m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()))))
	{
		return false;
	}
	m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);

	D3D12_COMMAND_QUEUE_DESC cqd;
	cqd.NodeMask = 0;
	cqd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	cqd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (FAILED(m_device->CreateCommandQueue(&cqd, IID_PPV_ARGS(m_cmdQueue.GetAddressOf()))))
	{
		return false;
	}
	return true;
}

void DXContext::Shutdown()
{
	m_fence.Reset();
	m_device.Reset();
	m_cmdQueue.Reset();
	m_cmdAlloc.Reset();
	m_cmdList.Reset();
	m_factory.Reset();
	if (m_fenceEvent)
	{
		CloseHandle(m_fenceEvent);
	}
}

void DXContext::SignalAndWait()
{
	m_cmdQueue->Signal(m_fence.Get(), ++m_fenceValue);
	if (SUCCEEDED(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent)))
	{
		if (WaitForSingleObject(m_fenceEvent, 20000) != WAIT_OBJECT_0)
		{
			std::exit(0);
		}
	}
	else
	{
		std::exit(0);
	}
}

void DXContext::ExecuteCommandList()
{
	if (SUCCEEDED(m_cmdList->Close()))
	{
		ID3D12CommandList* List[] = { m_cmdList.Get() };
		m_cmdQueue->ExecuteCommandLists(1, List);
		SignalAndWait();
	}
}

ID3D12GraphicsCommandList7* DXContext::InitCommandList()
{
	m_cmdAlloc->Reset();
	m_cmdList->Reset(m_cmdAlloc.Get(), nullptr);
	return m_cmdList.Get();
}