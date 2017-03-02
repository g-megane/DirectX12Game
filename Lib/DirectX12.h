//////////////////////////////////////////////////
// 作成日:2017/02/26
// 更新日:2017/03/02
// 制作者:got
//
// クラス詳細:DirectX12に関するクラス
//////////////////////////////////////////////////
#pragma once
#ifndef DIRECTX12_H
#define DIRECTX12_H
#include <wrl/client.h>
#include <dxgi1_3.h>
#include <d3d12.h>
#include <d3dcompiler.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Singleton.h"
#include "Window.h"

namespace got
{
    class DirectX12 : public Singleton<DirectX12>
    {
    public:
        ~DirectX12();

        HRESULT init(std::shared_ptr<Window> _window);
        void draw();
        void begineDraw();
        void endDraw();
        std::shared_ptr<ID3D12Device> getDevice() const;

    private:
        friend class Singleton<DirectX12>;
        DirectX12();

        bool createDevice();
        bool createCommandAllocator();
        bool createCommandQueue();
        bool createSwapChain();
        bool createCommandList();
        bool createRenderTarget();
        bool compileShader();

        void setResourceBarrier(
            ID3D12GraphicsCommandList *commandList,
            ID3D12Resource *res,
            D3D12_RESOURCE_STATES before,
            D3D12_RESOURCE_STATES after
        );

        std::shared_ptr<Window> m_spWindow;

        Microsoft::WRL::ComPtr<IDXGIFactory2>   m_DxgiFactory;
        Microsoft::WRL::ComPtr<IDXGISwapChain1> m_SwapChain;
        Microsoft::WRL::ComPtr<ID3D12Resource>  m_D3DBuffer[2];
        const int m_BufferWidth;
        const int m_BufferHeight;
        UINT64 m_FrameCount;
        
        std::shared_ptr<ID3D12Device> m_spDevice;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CmdAlloc;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CmdQueue;
        
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CmdList;
        Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
        HANDLE m_FenceEveneHandle;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescHeapRtv;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescHeapCbvSrvUav;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_Pso;

    };
}
#endif
