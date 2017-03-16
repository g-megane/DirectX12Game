//////////////////////////////////////////////////
// 作成日:2017/02/26
// 更新日:2017/03/11
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
#include <vector>
#include <tuple>

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
        bool createCommandQueue();
        bool createCommandAllocator();
        bool createCommandList();
        bool createSwapChain();
        bool createRenderTarget();
        bool createFence();
        bool compileShader();
        bool loadTexture();
        bool loadMesh();

        void setResourceBarrier(
            ID3D12GraphicsCommandList *commandList,
            ID3D12Resource *res,
            D3D12_RESOURCE_STATES before,
            D3D12_RESOURCE_STATES after
        );

        BOOL m_IsFullScreen;

        std::shared_ptr<Window> m_spWindow;

        Microsoft::WRL::ComPtr<IDXGIFactory2>   m_DxgiFactory;
        Microsoft::WRL::ComPtr<IDXGISwapChain1> m_SwapChain;
        Microsoft::WRL::ComPtr<ID3D12Resource>  m_D3DBuffer[2];
        int m_BufferWidth;
        int m_BufferHeight;
        UINT64 m_FrameCount;
        
        std::shared_ptr<ID3D12Device> m_spDevice;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CmdAlloc;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CmdQueue;
        
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CmdList;
        Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
        HANDLE m_FenceEveneHandle;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescHeapRtv;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescHeapCbvSrvUav;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescHeapDsv;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescHeapSampler;
        std::shared_ptr<void> m_CBUploadPtr;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_Pso;
        Microsoft::WRL::ComPtr<ID3D12Resource>      m_TexUpload;
        Microsoft::WRL::ComPtr<ID3D12Resource>      m_TexDefault;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_VB;
        D3D12_VERTEX_BUFFER_VIEW m_VBView = {};
        D3D12_INDEX_BUFFER_VIEW  m_IBView = {};
        UINT m_IndexCount;
        UINT m_VBIndexOffset;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_DB;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_CB;
        std::vector<std::tuple<unsigned int, unsigned int, unsigned int>> m_TexMipSize; // <0>width, <1>height, <2>uploadHeapOffset

        unsigned int texAlign(unsigned int s) {
            return (s + D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1);
        }
        unsigned int pitchAlign(unsigned int w) {
            return (w + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
        }

    };
}
#endif
