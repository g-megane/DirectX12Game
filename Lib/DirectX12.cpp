//////////////////////////////////////////////////
// 作成日:2017/02/26
// 更新日:2017/02/27
// 制作者:got
//////////////////////////////////////////////////
#include <stdexcept>
#include "DirectX12.h"

namespace got {
    DirectX12::DirectX12()
    {
    }

    DirectX12::~DirectX12()
    {
        CloseHandle(m_FenceEveneHandle);
    }

    HRESULT got::DirectX12::init(const int _width, const int _height, std::shared_ptr<Window> _window)
    {
        m_BufferWidth      = _width;
        m_BufferHeight     = _height;
        m_spWindow         = _window;
        m_FrameCount       = 0;
        m_FenceEveneHandle = 0;

#if _DEBUG
        auto hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(m_DxgiFactory.ReleaseAndGetAddressOf()));
        if (FAILED(hr)) {
            throw std::runtime_error("CreateDXGIFactory2() is failed value");
        }
#else
        auto hr = CreateDXGIFactory2(0, IID_PPV_ARGS(m_DxgiFactory.ReleaseAndGetAddressOf());
        if (FAILED()) {
            throw std::runtime_error("CreateDXGIFactory2() is failed value");
        }
#endif

#if _DEBUG
        //std::shared_ptr<ID3D12Debug> debug = nullptr;
        ID3D12Debug *debug = nullptr;
        D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
        if (debug)
        {
            debug->EnableDebugLayer();
            debug->Release();
            debug = nullptr;
        }
#endif
        ID3D12Device *device;
        hr = D3D12CreateDevice(
            nullptr, 
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&device)
        );
        if (FAILED(hr)) {
            throw std::runtime_error("D3D12CreateDevice() is failed value");
        }        
        m_spDevice = std::shared_ptr<ID3D12Device>(device, [](ID3D12Device *&ptr)
        {
            if (!ptr) { return; }
            ptr->Release();
            ptr = nullptr;
        });

        hr = m_spDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(m_CmdAlloc.ReleaseAndGetAddressOf())
        );
        if (FAILED(hr)) {
            throw std::runtime_error("ID3D12Device->CreateCommandAllocator() is failed value");
        }

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        hr = m_spDevice->CreateCommandQueue(
            &queueDesc,
            IID_PPV_ARGS(m_CmdQueue.ReleaseAndGetAddressOf())
        );
        if (FAILED(hr)) {
            throw std::runtime_error("ID3D12Device->CreateCommandQueue() is failed value");
        }

        // スワップチェインの作成
        DXGI_SWAP_CHAIN_DESC1 scDesc = {};
        scDesc.Width            = _width; //TODO:定数もしくはWindowクラスに持たせる
        scDesc.Height           = _height;  //TODO:定数もしくはWindowクラスに持たせる
        scDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
        scDesc.SampleDesc.Count = 1;
        scDesc.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.BufferCount      = 2;
        scDesc.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        hr = m_DxgiFactory->CreateSwapChainForHwnd(
            m_CmdQueue.Get(),
            m_spWindow->getHWND(),
            &scDesc, 
            nullptr, 
            nullptr, 
            m_SwapChain.ReleaseAndGetAddressOf()
        );
        if (FAILED(hr)) {
            throw std::runtime_error("IDXGIFactory2->CreateSwapChainForHwnd() is failed value");
        }

        hr = m_spDevice->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_CmdAlloc.Get(),
            nullptr,
            IID_PPV_ARGS(m_CmdList.ReleaseAndGetAddressOf())
        );
        if (FAILED(hr)) {
            throw std::runtime_error("ID3D12Device->CreateCommandList() is failed value");
        }

        hr = m_spDevice->CreateFence(
            0,
            D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(m_Fence.ReleaseAndGetAddressOf())
        );
        if (FAILED(hr)) {
            throw std::runtime_error("ID3D12Device->CreateFence() is failed value");
        }
        m_FenceEveneHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        for (int i = 0; i < 2/*TODO:BUFFER_COUNT(定数に)*/; ++i) {
            hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_D3DBuffer[i].ReleaseAndGetAddressOf()));
            if (FAILED(hr)) {
                throw std::runtime_error("IDXGISwapChain1->GetBuffer() is failed value");
            }
            m_D3DBuffer[i]->SetName(L"SwapChain_Buffer");
        }

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = 10;
        //desc.Flags = D3D12_DESCRIPTOR_HEAP_SHADER_VISIBLE;
        desc.NodeMask = 0;
        hr = m_spDevice->CreateDescriptorHeap(
            &desc,
            IID_PPV_ARGS(m_DescHeapRtv.ReleaseAndGetAddressOf())
        );
        if (FAILED(hr)) {
            throw std::runtime_error("ID3D12Device->CreateDescriptorHeap() is failed value");
        }

        auto rtvStep = m_spDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for (auto i = 0u; i < 2/*TODO:BUFFER_COUNT(定数に)*/; ++i) {
            auto d = m_DescHeapRtv->GetCPUDescriptorHandleForHeapStart();
            d.ptr += i * rtvStep;
            m_spDevice->CreateRenderTargetView(m_D3DBuffer[i].Get(), nullptr, d);
        }

        return S_OK;
    }

    void DirectX12::draw()
    {
        ++m_FrameCount;

        // Get current RTV descriptor
        auto descHandleRtvStep = m_spDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE descHandleRtv = m_DescHeapRtv->GetCPUDescriptorHandleForHeapStart();
        descHandleRtv.ptr += ((m_FrameCount - 1) % 2/*TODO:BUFFER_COUNT(定数に)*/) * descHandleRtvStep;
        // Get current swap chain
        ID3D12Resource *d3dBuffer = m_D3DBuffer[(m_FrameCount - 1) % 2/*TODO:BUFFER_COUNT(定数に)*/].Get();

        // Barrier Present -> RenderTarget
        setResourceBarrier(m_CmdList.Get(), d3dBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        // Viewport
        D3D12_VIEWPORT viewport = {};
        viewport.Width  = static_cast<float>(m_BufferWidth);
        viewport.Height = static_cast<float>(m_BufferHeight);
        m_CmdList->RSSetViewports(1, &viewport);

        // Clear
        auto saturate = [](float a) { return a < 0 ? 0 : a > 1 ? 1 : a; };
        float clearColor[4];
        static float h = 0.0f;
        h += 0.01f;
        if (h >= 1) h = 0.0f;
        clearColor[0] = saturate(std::abs(h * 6.0f - 3.0f) - 1.0f);
        clearColor[1] = saturate(2.0f - std::abs(h * 6.0f - 2.0f));
        clearColor[2] = saturate(2.0f - std::abs(h * 6.0f - 4.0f));
        m_CmdList->ClearRenderTargetView(descHandleRtv, clearColor, 0, nullptr);

        // Barrier Present -> Present
        setResourceBarrier(m_CmdList.Get(), d3dBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        // Exec
        auto hr = m_CmdList->Close();
        if (FAILED(hr)) {
            throw std::runtime_error("ID3D12GraphicsCommandList->Close() is failed value");
        }
        ID3D12CommandList * const cmdList = m_CmdList.Get();
        m_CmdQueue->ExecuteCommandLists(1, &cmdList);

        // Present
        hr = m_SwapChain->Present(1, 0);
        if (FAILED(hr)) {
            throw std::runtime_error("IDXGISeapChain1->Present() is failed value");
        }

        // Set queue flushed event
        hr = m_Fence->SetEventOnCompletion(m_FrameCount, m_FenceEveneHandle);
        if (FAILED(hr)) {
            throw std::runtime_error("CreateDescriptorHeap() is failed value");
        }

        // Wait for queue flushed
        // This code would CPU stall!
#if 1
        hr = m_CmdQueue->Signal(m_Fence.Get(), m_FrameCount);
        DWORD wait = WaitForSingleObject(m_FenceEveneHandle, 10000);
        if (wait != WAIT_OBJECT_0) {
            throw std::runtime_error("Failed WaitForSingleObject().");
        }
#else
        // Equivalent code
        auto prev = m_Fence->GetCompletedValue();
        hr = m_CmdQueue->Signal(m_Fence.Get(), m_FrameCount);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed WaitForSingleObject().");
        }
        while (prev == m_Fence->GetCompletedValue())
        {
        }
#endif
        if (FAILED(m_CmdAlloc->Reset())) {
            throw std::runtime_error("");
        }
        if (FAILED(m_CmdList->Reset(m_CmdAlloc.Get(), nullptr))) {
            throw std::runtime_error("");
        }

    }

    std::shared_ptr<ID3D12Device> DirectX12::getDevice() const
    {
        return m_spDevice;
    }

    void DirectX12::setResourceBarrier(ID3D12GraphicsCommandList * commandList, ID3D12Resource * res, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
    {
        D3D12_RESOURCE_BARRIER desc = {};
        desc.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        desc.Transition.pResource   = res;
        desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        desc.Transition.StateBefore = before;
        desc.Transition.StateAfter  = after;
        desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

        commandList->ResourceBarrier(1, &desc);
    }
}
