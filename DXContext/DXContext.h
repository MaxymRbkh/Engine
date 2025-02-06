#pragma once
#include <Support/WinInclude.h>

class DXContext
{
public:
	bool Init();
	void Shutdown();
	void SignalAndWait();
	void ExecuteCommandList();
	ID3D12GraphicsCommandList7* InitCommandList();

	inline ComPtr<IDXGIFactory7> &GetFactory()
	{
		return m_factory;
	}
	inline ComPtr<ID3D12Device10> GetDevice()
	{
		return m_device;
	}
	inline ComPtr<ID3D12CommandQueue>& GetCommandQueue()
	{
		return m_cmdQueue;
	}
	inline void Flush(size_t FrameCount)
	{
		for (size_t i = 0; i < FrameCount; i++)
		{
			SignalAndWait();
		}
	}

private:
	ComPtr<IDXGIFactory7> m_factory;
	ComPtr<ID3D12Device10> m_device;
	ComPtr<ID3D12CommandAllocator> m_cmdAlloc;
	ComPtr<ID3D12CommandQueue> m_cmdQueue;
	ComPtr<ID3D12GraphicsCommandList7> m_cmdList;
	ComPtr<ID3D12Fence1> m_fence;
	HANDLE m_fenceEvent = nullptr;
	UINT64 m_fenceValue = 0;

public:
	DXContext(const DXContext&)           = delete;
	DXContext operator=(const DXContext&) = delete;
	inline static DXContext& Get()
	{
		static DXContext Instance;
		return Instance;	
	}

private:
	DXContext() = default;
};

