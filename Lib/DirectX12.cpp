//////////////////////////////////////////////////
// �쐬��:2017/02/26
// �X�V��:2017/03/16
// �����:got
//////////////////////////////////////////////////
#include <stdexcept>
#include <sstream>
#include <fstream>
#include "DirectX12.h"
#include "GlobalValue.h"
#include "..//Common//d3dx12.h"

#include <DirectXMath.h>
using DirectX::XMFLOAT3;
#include "WaveFrontReader.h"

#define USE_BC1_TEXTURE 1


void checkHresult(HRESULT _hr, std::string _message) {
    if (FAILED(_hr)) {
        throw std::runtime_error(_message);
    }
}

namespace got {
    // �R���X�g���N�^
    DirectX12::DirectX12()
        : m_BufferWidth(WINDOW_WIDTH), m_BufferHeight(WINDOW_HEIGHT)
    {
    }
    // �f�X�g���N�^
    DirectX12::~DirectX12()
    {
        m_CB->Unmap(0, nullptr);
        CloseHandle(m_FenceEveneHandle);
    }
    // ������
    HRESULT got::DirectX12::init(std::shared_ptr<Window> _window)
    {
        m_spWindow         = _window;
        m_FrameCount       = 0;
        m_FenceEveneHandle = 0;

        m_CBUploadPtr      = nullptr;
        m_IndexCount       = 0;
        m_VBIndexOffset    = 0;

        if (!createDevice())           { return false; }
        if (!createCommandQueue())     { return false; }
        if (!createCommandAllocator()) { return false; }
        if (!createCommandList())      { return false; }
        if (!createSwapChain())        { return false; }
        if (!createRenderTarget())     { return false; }
        if (!createFence())            { return false; }
        if (!compileShader())          { return false; }
        //if (!loadTexture())            { return false; }
        if (!loadMesh())               { return false; }
        
        return S_OK;
    }
    // �f�o�C�X�̍쐬
    bool DirectX12::createDevice()
    {
#if _DEBUG
        checkHresult(
            CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(m_DxgiFactory.ReleaseAndGetAddressOf())),
            "CreateDXGIFactory2() is failed value"
        );
#else
        checkHresult(
            CreateDXGIFactory2(0, IID_PPV_ARGS(m_DxgiFactory.ReleaseAndGetAddressOf()),
            "CreateDXGIFactory2() is failed value"
        );
#endif

#if _DEBUG
        // �f�o�b�O�ł̓f�o�b�O���C���[��L��������
        ID3D12Debug *debug = nullptr;
        D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
        if (debug)
        {
            debug->EnableDebugLayer();
            debug->Release();
            debug = nullptr;
        }
#endif
        // �f�o�C�X�̍쐬
        ID3D12Device *device;
        checkHresult(
            D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)), 
            "D3D12CreateDevice() is failed value"
        );
        m_spDevice = std::shared_ptr<ID3D12Device>(device, [](ID3D12Device *&ptr)
        {
            if (!ptr) { return; }
            ptr->Release();
            ptr = nullptr;
        });

        return true;
    }
    // CommandAllocator�̍쐬
    bool DirectX12::createCommandAllocator()
    {
        checkHresult(
            // �������̃^�C�v�̓R�}���h���X�g�������^�C�v�ƍ��킹��K�v������
            m_spDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CmdAlloc.ReleaseAndGetAddressOf())),
            "m_spDevice->CreateCommandAllocator() is failed value"
        );

        return true;
    }
    // CommandQueue�̍쐬
    bool DirectX12::createCommandQueue()
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;  // GPU�^�C���A�E�g���L��
        queueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT; // ���ڃR�}���h�L���[
        checkHresult(
            m_spDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CmdQueue.ReleaseAndGetAddressOf())),
            "m_spDevice->CreateCommandQueue() is failed value"
        );

        return true;
    }
    // CommandList�̍쐬
    //      ���쐬�ς݂̃R�}���h�A���P�[�^���K�v�ɂȂ�
    bool DirectX12::createCommandList()
    {
        checkHresult(
            m_spDevice->CreateCommandList(
                0,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                m_CmdAlloc.Get(),
                nullptr,
                IID_PPV_ARGS(m_CmdList.ReleaseAndGetAddressOf())
            ),
            "m_spDevice->CreateCommandList() is failed value"
        );

        return true;
    }
    // �X���b�v�`�F�C���̍쐬(DX11�ƕς��Ȃ�)
    bool DirectX12::createSwapChain()
    {
        DXGI_SWAP_CHAIN_DESC1 scDesc = {};
        scDesc.Width            = WINDOW_WIDTH;                     // �𑜓x�̕���\���l
        scDesc.Height           = WINDOW_HEIGHT;                    // �𑜓x�̍�����\���l
        scDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;       // �\���`�����L�q����DXGI_FORMAT�\����
        scDesc.SampleDesc.Count = 1;                                // �}���`�T���v�����O�p�����[�^���L�q����DXGI_SAMPLE_DESC�\����
        scDesc.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // �o�b�N�o�b�t�@�̃T�[�t�F�[�X�g�p�󋵂�CPU�A�N�Z�X�I�v�V�������L�qDXGI_USAGE�^�̒l
        scDesc.BufferCount      = BUFFER_COUNT;                     // �X���b�v�`�F�[�����̃o�b�t�@����\���l(�ʏ�A���̒l�Ƀt�����g�o�b�t�@���܂߂܂�)
        scDesc.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // �X���b�v�`�F�[���ɂ���Ďg�p�����v���[���e�[�V�������f�����L�q����DXGI_SWAP_EFFECT�^�̒l�A����уT�[�t�F�[�X��񎦂�����Ƀv���[���e�[�V�����o�b�t�@�̓��e����������I�v�V����
        checkHresult(
            m_DxgiFactory->CreateSwapChainForHwnd(
                m_CmdQueue.Get(),      // �R�}���h�L���[�ւ̃|�C���^(11����т���ȑO��Direct3D�ւ̃|�C���^)
                m_spWindow->getHWND(), // CreateSwapChainForHwnd���쐬����X���b�v�`�F�[���Ɋ֘A�t�����Ă���HWND�n���h��(NULL�ɂ��邱�Ƃ͂ł��Ȃ�)
                &scDesc,               // DXGI_SWAP_CHAIN_DESC1�\���̂ւ̃|�C���^
                nullptr,               // DXGI_SWAP_CHAIN_FULLSCREEN_DESC�\����
                nullptr,               // IDXGIOutput�C���^�[�t�F�[�X�ւ̃|�C���^�B
                m_SwapChain.ReleaseAndGetAddressOf()
            ),
            "m_DxgiFactory->CreateSwapChainForHwnd() is failed value"
        );

        for (int i = 0; i < BUFFER_COUNT; ++i) {
            checkHresult(
                // 1.�[������n�܂�o�b�t�@�C���f�b�N�X  2.�o�b�t�@�[�̑���Ɏg�p����C���^�[�t�F�[�X�̎��
                m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_D3DBuffer[i].ReleaseAndGetAddressOf())),
                "m_SwapChain->GetBuffer() is failed value"
            );
            m_D3DBuffer[i]->SetName(L"SwapChain_Buffer");
        }

        return true;
    }
    // �X���b�v�`�F�C����RenderTarget�Ƃ��Ďg�p���邽�߂�DescriptorHeap���쐬
    bool DirectX12::createRenderTarget()
    {
        // DescriptorHeap(DX11�ł���View�̂悤�ȑ���)
        //      Descriptor�Ƃ����A���̃o�b�t�@�������������̂Ȃ̂����L�q�����f�[�^�����݂��Ă��܂��B
        //      DX12�ł�Descriptor�𒼐ڃR�}���h�ɐςނ̂ł͂Ȃ��ADescriptorHeap�̂�����Descriptor
        //      �����݂��邩�炱����g���A�Ƃ����`�Ŏw�肷��K�v������܂��B
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        // �����_�[�^�[�Q�b�g�r���[�p��DescriptorHeap
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = BUFFER_COUNT;
        desc.NodeMask       =  0;
        checkHresult(
            m_spDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DescHeapRtv.ReleaseAndGetAddressOf())),
            "m_spDevice->CreateDescriptorHeap() is failed value"
        );
        // �[�x�X�e���V���r���[�p��DescriptorHeap
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        desc.NumDescriptors = 10;
        desc.NodeMask       = 0;
        checkHresult(
            m_spDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DescHeapDsv.ReleaseAndGetAddressOf())),
            "m_spDevice->CreateDescriptorHeap() is failed value"
        );
        // constant-buffer�Ashader-resource�Aunordered-access�r���[�̑g�ݍ��킹�ɑ΂���DescriptorHeap
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 100;
        desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask       = 0;
        checkHresult(
            m_spDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DescHeapCbvSrvUav.ReleaseAndGetAddressOf())),
            "m_spDevice->CreateDescriptorHeap() is failed value"
        );

        // �X���b�v�`�F�C���̃o�b�t�@���ɍ쐬����DescriptorHeap�ɓo�^����
        auto rtvStep = m_spDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for (auto i = 0u; i < BUFFER_COUNT; ++i) {
            auto d = m_DescHeapRtv->GetCPUDescriptorHandleForHeapStart(); // DescriptorHeap�̊J�n�ʒu�̃n���h�����擾
            d.ptr += i * rtvStep;
            m_spDevice->CreateRenderTargetView(m_D3DBuffer[i].Get(), nullptr, d);
        }

        return true;
    }
    // �������Ƃ邽�߂̃t�F���X���쐬����
    //      �t�F���X�FCommandQueue�ɃT�u�~�b�g����CommandList�̊��������m���邽�߂Ɏg��
    bool DirectX12::createFence()
    {
        checkHresult(
            m_spDevice->CreateFence(
                0,                                             // �t�F���X�������Ă���l�̏����l��ݒ�(�`�揈���������������ǂ����͂��̃t�F���X�l���C���N�������g���ꂽ���ǂ����Ŕ��f����)
                D3D12_FENCE_FLAG_NONE,                         // 
                IID_PPV_ARGS(m_Fence.ReleaseAndGetAddressOf()) // 
            ),
            "m_spDevice->CreateFence() is failed value"
            );
        m_FenceEveneHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        
        return true;
    }
    // �V�F�[�_�[�̓ǂݍ���
    bool DirectX12::compileShader()
    {
        // Descriptor�͈̔͂��w��
        CD3DX12_DESCRIPTOR_RANGE descRange1[1];
        descRange1[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        // RootSignature�o�[�W����1.0�̃X���b�g�ɂ��Ďw��
        CD3DX12_ROOT_PARAMETER rootParam[1];
        rootParam[0].InitAsDescriptorTable(ARRAYSIZE(descRange1), descRange1);
        
        ID3D10Blob *sig, *info;
        // RootSignature : DescriptorTable���܂Ƃߏグ�鑶��
        auto rootSigDesc  = D3D12_ROOT_SIGNATURE_DESC();
        rootSigDesc.NumParameters     = _countof(rootParam);//1;                                      // ���[�g�����̃X���b�g��
        rootSigDesc.NumStaticSamplers = 0;                                                            // �ÓI�T���v���[�̐����w�肷��
        rootSigDesc.pParameters       = rootParam;                                                    // ���[�g�V�O�l�`�����̃X���b�g��D3D12_ROOT_PARAMETER�\���̂̔z��
        rootSigDesc.pStaticSamplers   = nullptr;                                                      // 1�ȏ��D3D12_STATIC_SAMPLER_DESC�\���̂ւ̃|�C���^
        rootSigDesc.Flags             = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // �r�b�g�P�ʂ�OR���Z���g�p���Č������ꂽD3D12_STATIC_SAMPLER_DESC�\���̂ւ̃|�C���^
        // RootSignature���쐬����̂ɕK�v�ȃo�b�t�@���m�ۂ��ATable�����V���A���C�Y����
        checkHresult(
            D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &info),
            "D3D12SerializeRootSignature() is failed value"
        );
        // RootSignature���쐬
        m_spDevice->CreateRootSignature(
            0,
            sig->GetBufferPointer(),
            sig->GetBufferSize(),
            IID_PPV_ARGS(m_RootSignature.ReleaseAndGetAddressOf())
        );
        sig->Release();

        ID3D10Blob *vs, *ps;
        // ID3D10Blob *info;
        UINT flag = 0;
#if _DEBUG
        flag |= D3DCOMPILE_DEBUG;
#endif
        // VS�̓ǂݍ���
        checkHresult(
            D3DCompileFromFile(L"Lib\\Shader\\ShaderSample.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", flag, 0, &vs, &info),
            "VS D3DCompileFromFile() compile is failed"
        );
        // PS�̓ǂݍ���
        checkHresult(
             D3DCompileFromFile(L"Lib\\Shader\\ShaderSample.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", flag, 0, &ps, &info),
            "PS D3DCompileFromFile() compile is failed"
        );
        // InputLayout�̐ݒ�
        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL"   , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD" , 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // PSO(Pipeline State Object)�̍쐬
        //      Direct3D12�ɂ́A���_�V�F�[�_�[��s�N�Z���V�F�[�_�[�̒P�Ɛݒ��A
        //      InputLayout��Rasterizer/Blend/DepthStencilState�̐ݒ��API�͂Ȃ�
        //      ���ׂāAPSO�̍�蒼���ɂȂ�I�I
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.PrimitiveTopologyType            = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;     // 
        // �C���v�b�g���C�A�E�g�̐ݒ�
        psoDesc.InputLayout.NumElements          = 3;                                          // 
        psoDesc.InputLayout.pInputElementDescs   = inputLayout;
        //
        psoDesc.IBStripCutValue                  = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        // ���[�g�V�O�l�`��
        psoDesc.pRootSignature                   = m_RootSignature.Get();                      // ID3D12RootSignature�I�u�W�F�N�g�ւ̃|�C���^�[
        // ���_�V�F�[�_�[���L�q����D3D12_SHADER_BYTECODE�\����                                   
        psoDesc.VS.pShaderBytecode               = vs->GetBufferPointer();                     // D3D12_SHADER_BYTECODE�\���̂ւ̃|�C���^�[
        psoDesc.VS.BytecodeLength                = vs->GetBufferSize();                        // D3D12_SHADER_BYTECODE�\���̂̃T�C�Y
        // �s�N�Z���V�F�[�_�[���L�q����D3D12_SHADER_BYTECODE�\����                               
        psoDesc.PS.pShaderBytecode               = ps->GetBufferPointer();                     // D3D12_SHADER_BYTECODE�\���̂ւ̃|�C���^�[
        psoDesc.PS.BytecodeLength                = ps->GetBufferSize();                        // D3D12_SHADER_BYTECODE�\���̂̃T�C�Y
        // ���X�^���C�Y�X�e�[�g�̐ݒ�
        psoDesc.RasterizerState                  = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT()); // ���X�^���C�U�X�e�[�g���L�q����D3D12_RASTERIZER_DESC�\����
        // �u�����h�X�e�[�g�̐ݒ�
        psoDesc.BlendState                       = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT());      // �u�����h�X�e�[�g���L�q����D3D12_STREAM_OUTPUT_DESC�\���� 
        // �f�v�X�X�e���V���X�e�[�g�̐ݒ�
        psoDesc.DepthStencilState.DepthEnable    = TRUE;                                      // 
        psoDesc.DepthStencilState.DepthFunc      = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        psoDesc.DepthStencilState.StencilEnable  = FALSE;                                      // 
        // �����_�[�^�[�Q�b�g�̐ݒ�
        psoDesc.NumRenderTargets                 = 1;                                          // 
        psoDesc.RTVFormats[0]                    = DXGI_FORMAT_R8G8B8A8_UNORM;                 // 
        // �T���v���n�̐ݒ�
        psoDesc.SampleDesc.Count                 = 1;                                          // 
        psoDesc.SampleMask                       = UINT_MAX;                                   // 
        // �[�x�X�e���V��
        psoDesc.DSVFormat                        = DXGI_FORMAT_D32_FLOAT;
        // PSO�̍쐬
        checkHresult(
            m_spDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_Pso.ReleaseAndGetAddressOf())),
            "m_spDevice->CreateGraphicsPipelineState() is failed value"
        );
        vs->Release();
        ps->Release();

        //struct float3 
        //{
        //    float f[3];
        //};

        //float3 vbData[4] =
        //{
        //    { -0.7f,  0.7f,  0.0f },
        //    {  0.7f,  0.7f,  0.0f },
        //    { -0.7f, -0.7f,  0.0f },
        //    {  0.7f, -0.7f,  0.0f }
        //};
        //unsigned short idData[6] = { 0, 1, 2, 2, 1, 3 };
        //// 
        //D3D12_HEAP_PROPERTIES heapProps;
        //heapProps.Type                 = D3D12_HEAP_TYPE_UPLOAD;          // �q�[�v�^�C�v���w�肷��D3D12_CPU_PROPERTY�^�̒l
        //heapProps.CreationNodeMask     = 1;                               // �q�[�v��CPU�y�[�W�v���p�e�B���w�肷��D3D12_CPU_PAGE_PROPERTY�^�̒l
        //heapProps.VisibleNodeMask      = 1;                               // �q�[�v��CPU�v���p�e�B
        //heapProps.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // 
        //heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;       // 
        //D3D12_RESOURCE_DESC   descResourceTex;
        //descResourceTex.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
        //descResourceTex.Alignment          = 0;
        //descResourceTex.Width              = sizeof(idData) + sizeof(vbData);
        //descResourceTex.Height             = 1;
        //descResourceTex.DepthOrArraySize   = 1;
        //descResourceTex.MipLevels          = 1;
        //descResourceTex.Format             = DXGI_FORMAT_UNKNOWN;
        //descResourceTex.SampleDesc.Count   = 1;
        //descResourceTex.SampleDesc.Quality = 0;
        //descResourceTex.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        //descResourceTex.Flags              = D3D12_RESOURCE_FLAG_NONE;

        //// �q�[�v�����\�[�X�S�̂��i�[����̂ɏ\���ȑ傫���Ń��\�[�X���q�[�v�Ƀ}�b�v�����悤�ɁA
        //// ���\�[�X�ƈÖق̃q�[�v�������쐬����B
        //hr = m_spDevice->CreateCommittedResource
        //(
        //    &heapProps,
        //    D3D12_HEAP_FLAG_NONE,
        //    &descResourceTex,
        //    D3D12_RESOURCE_STATE_GENERIC_READ,
        //    nullptr,
        //    IID_PPV_ARGS(m_VB.ReleaseAndGetAddressOf())
        //);
        //if (FAILED(hr)) {
        //    throw std::runtime_error("ID3D12Device->CreateCommittedResource() is failed value");
        //}
        //m_VB->SetName(L"VertexBuffer");
        //char* vbUploadPtr = nullptr;
        //hr = m_VB->Map(0, nullptr, reinterpret_cast<void**>(&vbUploadPtr));
        //memcpy_s(vbUploadPtr, sizeof(vbData), vbData, sizeof(vbData));
        //memcpy_s(vbUploadPtr + sizeof(vbData), sizeof(idData), idData, sizeof(idData));
        //m_VB->Unmap(0, nullptr);

        //m_VBView.BufferLocation = m_VB->GetGPUVirtualAddress();
        //m_VBView.StrideInBytes  = sizeof(float3);
        //m_VBView.SizeInBytes    = sizeof(vbData);
        //m_IBView.BufferLocation = m_VB->GetGPUVirtualAddress() + sizeof(vbData);
        //m_IBView.Format         = DXGI_FORMAT_R16_UINT;
        //m_IBView.SizeInBytes    = sizeof(idData);
      
        return true;
    }
    bool DirectX12::loadTexture()
    {
        // Calcurate Mip
//        unsigned int initialWidth  = 256;
//        unsigned int initialHeight = 256;
//        unsigned int bytePerPixel  = 4;
//#if USE_BC1_TEXTURE
//        unsigned int bcByte = 8;
//#endif
//        auto  minSize = min(initialHeight, initialWidth);
//        DWORD mipCount;
//        unsigned int copyDestOffset = 0;
//        unsigned int copyDestTotalSize = 0;
//        if (minSize == 1) {
//            m_TexMipSize.resize(1);
//            m_TexMipSize[0] = std::make_tuple(1, 1, copyDestOffset);
//        }
//        else {
//            _BitScanReverse(&mipCount, minSize - 1);
//            m_TexMipSize.resize(mipCount + 2);
//            int w = initialWidth;
//            int h = initialHeight;
//            m_TexMipSize[0] = std::make_tuple(w, h, copyDestOffset);
//            for (auto i = 1u; i < m_TexMipSize.size(); ++i) {
//                int cw = w, ch = h;
//#if USE_BC1_TEXTURE
//                cw = (w + 3) / 4;
//                ch = (h + 3) / 4;
//#endif
//                copyDestOffset += texAlign(pitchAlign(bytePerPixel * cw) * ch);
//                m_TexMipSize[i] = std::make_tuple(w /= 2, h /= 2, copyDestOffset);
//            }
//        }
//        auto &lastSlice = m_TexMipSize[m_TexMipSize.size() - 1];
//        copyDestTotalSize = std::get<2>(lastSlice) + texAlign(pitchAlign(bytePerPixel * std::get<0>(lastSlice)) * std::get<1>(lastSlice));
//
//        // Read DDS File
//        int totalTexSize = 0;
//#if USE_BC1_TEXTURE
//        std::for_each(m_TexMipSize.cbegin(), m_TexMipSize.cend(), [&](auto m) {
//            totalTexSize += ((std::get<0>(m) + 3) / 4) * ((std::get<1>(m) + 3) / 4) * bcByte;
//        });
//#else
//        std::for_each(m_TexMipSize.cbegine(), m_TexMipSize.cend(), [&](auto m) {
//            totalTexSize += std::get<0>(m) * std::get<1>(m) * bytePerPixel;
//        });
//
//#endif
//        std::vector<char> texData(totalTexSize);
//
//#if USE_BC1_TEXTURE
//        std::ifstream ifs("d3d12_bc1.dds", std::ios::binary);
//#else
//        std::ifstream ifs("d3d12.dds", std::ios::binary);
//#endif
//        if (!ifs) {
//            throw std::runtime_error("Texture not found.");
//        }
//        ifs.seekg(128, std::ios::beg); // Skip DDS header
//        ifs.read(texData.data(), texData.size());
//        auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(copyDestTotalSize, D3D12_RESOURCE_FLAG_NONE, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
//        auto hr = m_spDevice->CreateCommittedResource(
//            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
//            D3D12_HEAP_FLAG_NONE,
//            &resourceDesc,
//            D3D12_RESOURCE_STATE_GENERIC_READ,
//            nullptr,
//            IID_PPV_ARGS(m_TexUpload.ReleaseAndGetAddressOf())
//        );
//        if (FAILED(hr)) {
//            throw std::runtime_error("m_spDevice->CreateCommittedResource() is failed value");
//        }
//        m_TexUpload->SetName(L"Texture");
//
//        int copySrcOffset = 0;
//        char *dest;
//        hr = m_TexUpload->Map(0, nullptr, reinterpret_cast<void**>(&dest));
//        if (FAILED(hr)) {
//            throw std::runtime_error("m_TexUpload->Map() is failed value");
//        }
//        for (auto i = 0u; i < m_TexMipSize.size(); ++i) {
//            auto curSlice = m_TexMipSize[i];
//            auto cw       = std::get<0>(curSlice), ch = std::get<1>(curSlice);
//#if USE_BC1_TEXTURE
//            cw = (cw + 3) / 4;
//            ch = (ch + 3) / 4;
//#endif
//            for (auto h = 0u; h < ch; ++h) {
//                auto r = std::get<2>(curSlice) + pitchAlign(bytePerPixel * cw) * h;
//#if USE_BC1_TEXTURE
//                auto s = bcByte * cw;
//#else
//                auto s = bytePerPixel * cw;
//#endif
//                memcpy_s(dest + r, s, texData.data() + copySrcOffset, s); // To maximize WC memory writing performance, size should be aligned by 16 byte on x86.
//                copySrcOffset += s;
//            }
//        }
//        m_TexUpload->Unmap(0, nullptr);
//
//        DXGI_FORMAT texFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
//#if USE_BC1_TEXTURE
//        texFormat = DXGI_FORMAT_BC1_UNORM;
//#endif
//        resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
//            texFormat, 
//            std::get<0>(m_TexMipSize[0]),
//            std::get<1>(m_TexMipSize[0]),
//            1,
//            static_cast<UINT16>(m_TexMipSize.size()),
//            1,
//            0,
//            D3D12_RESOURCE_FLAG_NONE,
//            D3D12_TEXTURE_LAYOUT_UNKNOWN,
//            0
//            );
//        hr = m_spDevice->CreateCommittedResource(
//            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
//            D3D12_HEAP_FLAG_NONE,
//            &resourceDesc,
//            D3D12_RESOURCE_STATE_COPY_DEST,
//            nullptr,
//            IID_PPV_ARGS(m_TexDefault.ReleaseAndGetAddressOf())
//        );
//        if (FAILED(hr)) {
//            throw std::runtime_error("m_spDevice->CreateCommittedResource is failed value");
//        }
//
//        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
//        srvDesc.Format                        = texFormat;
//        srvDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;
//        srvDesc.Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//        srvDesc.Texture2D.MipLevels           = m_TexMipSize.size();
//        srvDesc.Texture2D.MostDetailedMip     = 0;
//        srvDesc.Texture2D.PlaneSlice          = 0;
//        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
//        m_spDevice->CreateShaderResourceView(m_TexDefault.Get(), &srvDesc, m_DescHeapCbvSrvUav->GetCPUDescriptorHandleForHeapStart());
//
//        D3D12_SAMPLER_DESC samplerDesc = {};
//        samplerDesc.Filter         =  D3D12_FILTER_MIN_MAG_MIP_LINEAR;
//        samplerDesc.AddressU       =  D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
//        samplerDesc.AddressV       =  D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
//        samplerDesc.AddressW       =  D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
//        samplerDesc.MinLOD         = -FLT_MAX;
//        samplerDesc.MaxLOD         =  FLT_MAX;
//        samplerDesc.MipLODBias     =  0;
//        samplerDesc.MaxAnisotropy  =  0;
//        samplerDesc.ComparisonFunc =  D3D12_COMPARISON_FUNC_NEVER;
//        m_spDevice->CreateSampler(&samplerDesc, m_DescHeapSampler->GetCPUDescriptorHandleForHeapStart());
//
        return true;
    }
    bool DirectX12::loadMesh()
    {
        WaveFrontReader<uint16_t> mesh;
        checkHresult(mesh.Load(L"teapot.obj"), "mesh.Load() is failed value");

        m_IndexCount    = static_cast<UINT>(mesh.indices.size());
        m_VBIndexOffset = static_cast<UINT>(sizeof(mesh.vertices[0]) * mesh.vertices.size());
        UINT IBSize     = static_cast<UINT>(sizeof(mesh.indices[0] ) * m_IndexCount);

        void *vbData = mesh.vertices.data();
        void *ibData = mesh.indices.data();
        checkHresult(
            m_spDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(m_VBIndexOffset + IBSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(m_VB.ReleaseAndGetAddressOf())
            ),
            "m_spDevice->CreateCommittedResource() is failed value"
        );
        m_VB->SetName(L"VertexBuffer");
        char *vbUploadPtr = nullptr;
        checkHresult(
            m_VB->Map(0, nullptr, reinterpret_cast<void**>(&vbUploadPtr)),
            "m_VB->Map() is failed value"
        );
        memcpy_s(vbUploadPtr, m_VBIndexOffset, vbData, m_VBIndexOffset);
        memcpy_s(vbUploadPtr + m_VBIndexOffset, IBSize, ibData, IBSize);
        m_VB->Unmap(0, nullptr);

        m_VBView.BufferLocation  = m_VB->GetGPUVirtualAddress();
        m_VBView.StrideInBytes   = sizeof(mesh.vertices[0]);
        m_VBView.SizeInBytes     = m_VBIndexOffset;
        m_IBView.BufferLocation  = m_VB->GetGPUVirtualAddress() + m_VBIndexOffset;
        m_IBView.Format          = DXGI_FORMAT_R16_UINT;
        m_IBView.SizeInBytes     = IBSize;

        auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_R32_TYPELESS,
            m_BufferWidth,
            m_BufferHeight,
            1,
            1,
            1,
            0,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            0
        );
        D3D12_CLEAR_VALUE dsvClearValue;
        dsvClearValue.Format               = DXGI_FORMAT_D32_FLOAT;
        dsvClearValue.DepthStencil.Depth   = 1.0f;
        dsvClearValue.DepthStencil.Stencil =    0;
        checkHresult(
            m_spDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &dsvClearValue,
                IID_PPV_ARGS(m_DB.ReleaseAndGetAddressOf())
            ),
            "m_spDevice->CreateCommittedResource() is failed value"
        );
        m_DB->SetName(L"DepthTexture");

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Format             = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.Texture2D.MipSlice = 0;
        dsvDesc.Flags              = D3D12_DSV_FLAG_NONE;
        m_spDevice->CreateDepthStencilView(m_DB.Get(), &dsvDesc, m_DescHeapDsv->GetCPUDescriptorHandleForHeapStart());

        checkHresult(
            m_spDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(m_CB.ReleaseAndGetAddressOf())
            ),
            "m_spDevice->CreateCommittedResource() is failed value"
        );
        m_CB->SetName(L"ConstantBuffer");
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_CB->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes    = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

        m_spDevice->CreateConstantBufferView(
            &cbvDesc, m_DescHeapCbvSrvUav->GetCPUDescriptorHandleForHeapStart()
        );
        checkHresult(
            m_CB->Map(0, nullptr, reinterpret_cast<void**>(&m_CBUploadPtr)),
            "m_CB->Map() is failed value"
        );

        return true;
    }
    // �`�� TODO:�Ƃ肠����
    void DirectX12::draw()
    {
        ++m_FrameCount;

        // Upload constant buffer
        static float rot = 0.0f;
        rot += 1.0f;
        if (rot >= 360.0f) {
            rot = 0.0f;
        }

        DirectX::XMMATRIX worldMat, viewMat, projMat;
        worldMat = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(rot));
        viewMat  = DirectX::XMMatrixLookAtLH({ 0, 1, -1.5f }, { 0, 0.5f, 0 }, { 0, 1, 0 });
        projMat  = DirectX::XMMatrixPerspectiveFovLH(45, static_cast<float>(m_BufferWidth / m_BufferHeight), 0.01f, 50.0f);
        auto mvpMat = DirectX::XMMatrixTranspose(worldMat * viewMat * projMat);

        auto worldTransMat = DirectX::XMMatrixTranspose(worldMat);

        // mCBUploadPtr is Write-Combine memory
        memcpy_s(m_CBUploadPtr.get(), 64, &mvpMat, 64);
        memcpy_s(reinterpret_cast<char*>(m_CBUploadPtr.get()) + 64, 64, &worldTransMat, 64);

        // Get current RTV descriptor
        auto descHandleRtvStep = m_spDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE descHandleRtv = m_DescHeapRtv->GetCPUDescriptorHandleForHeapStart();
        descHandleRtv.ptr += ((m_FrameCount - 1) % BUFFER_COUNT) * descHandleRtvStep;
        // Get current swap chain
        ID3D12Resource *d3dBuffer = m_D3DBuffer[(m_FrameCount - 1) % BUFFER_COUNT].Get();
        // Get DSV
        auto descHandleDsv = m_DescHeapDsv->GetCPUDescriptorHandleForHeapStart();

        // Copy texture from upload heap to default heap
//        if (m_FrameCount == 1) {
//            for (auto i = 0u; i < m_TexMipSize.size(); ++i) {
//                D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
//                srcLoc.pResource              = m_TexUpload.Get();
//                srcLoc.Type                   = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
//                srcLoc.PlacedFootprint.Offset = std::get<2>(m_TexMipSize[i]);
//#if USE_BC1_TEXTURE
//                srcLoc.PlacedFootprint.Footprint.Format = DXGI_FORMAT_BC1_UNORM;
//                srcLoc.PlacedFootprint.Footprint.Width  = max(4, std::get<0>(m_TexMipSize[i]));
//                srcLoc.PlacedFootprint.Footprint.Height = max(4, std::get<1>(m_TexMipSize[i]));
//                auto cw = (std::get<0>(m_TexMipSize[i]) + 3) / 4;
//                srcLoc.PlacedFootprint.Footprint.RowPitch = pitchAlign(cw * 8);
//#else 
//                srcLoc.PlacedFootprint.Footprint.Format   = DXGI_FORMAT_B8G8R8A8_UNORM;
//                srcLoc.PlacedFootprint.Footprint.Width    = std::get<0>(m_TexMipSize[i]);
//                srcLoc.PlacedFootprint.Footprint.Height   = std::get<1>(m_TexMipSize[i]);
//                srcLoc.PlacedFootprint.Footprint.RowPitch = pitchAlign(std::get<0>(m_TexMipSize[i]) * 4);
//#endif
//                srcLoc.PlacedFootprint.Footprint.Depth = 1;
//                D3D12_TEXTURE_COPY_LOCATION destLoc = {};
//                destLoc.pResource        = m_TexDefault.Get();
//                destLoc.SubresourceIndex = i;
//                destLoc.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
//                D3D12_BOX box = {};
//#if USE_BC1_TEXTURE
//                box.right  = max(4, std::get<0>(m_TexMipSize[i]));
//                box.bottom = max(4, std::get<1>(m_TexMipSize[i]));
//#else
//                box.right  = std::get<0>(m_TexMipSize[i]);
//                box.bottom = std::get<1>(m_TexMipSize[i]);
//#endif
//                box.back = 1;
//                m_CmdList->CopyTextureRegion(&destLoc, 0, 0, 0, &srcLoc, &box);
//            }
//
//            setResourceBarrier(m_CmdList.Get(), m_TexDefault.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
//        }

        // Barrier Present -> RenderTarget
        // Attention: Texture deleting must do while it is NOT referred by command list, or you will get segfalut or device removing.
        //if (m_FrameCount == 2) {
        //    m_TexUpload.Reset();
        //}

        // Barrier Presenr -> RenderTarget
        // Viewport
        // �r���[�|�[�g�̐ݒ�
        D3D12_VIEWPORT viewport = {};
        viewport.Width    = static_cast<float>(m_BufferWidth);
        viewport.Height   = static_cast<float>(m_BufferHeight);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        m_CmdList->RSSetViewports(1, &viewport);
        // �V�U�[��`
        D3D12_RECT scissor = {};
        scissor.right  = static_cast<LONG>(m_BufferWidth);
        scissor.bottom = static_cast<LONG>(m_BufferHeight);
        m_CmdList->RSSetScissorRects(1, &scissor);

        setResourceBarrier(m_CmdList.Get(), d3dBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        // Clear DepthTexture
        m_CmdList->ClearDepthStencilView(descHandleDsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        // �o�b�N�o�b�t�@�̃N���A
        float clearColor[4] = {0.1f, 0.2f, 0.3f, 1.0f};
        m_CmdList->ClearRenderTargetView(descHandleRtv, clearColor, 0, nullptr);
        //{
        //    auto saturate = [](float a) { return a < 0 ? 0 : a > 1 ? 1 : a; };
        //    float clearColor[4];
        //    static float h = 0.0f;
        //    h += 0.02f;
        //    if (h >= 1) h = 0.0f;
        //    clearColor[0] = saturate(std::abs(h * 6.0f - 3.0f) - 1.0f);
        //    clearColor[1] = saturate(2.0f - std::abs(h * 6.0f - 2.0f));
        //    clearColor[2] = saturate(2.0f - std::abs(h * 6.0f - 4.0f));
        //    m_CmdList->ClearRenderTargetView(descHandleRtv, clearColor, 0, nullptr);
        //}
        m_CmdList->OMSetRenderTargets(1, &descHandleRtv, true, &descHandleDsv);

        // Draw
        // ���[�g�V�O�l�`����ݒ�(�O���t�B�b�N�X�p�C�v���C����RootSignature��ݒ肷��)
        m_CmdList->SetGraphicsRootSignature(m_RootSignature.Get());
        ID3D12DescriptorHeap *descHeaps[] = {m_DescHeapCbvSrvUav.Get() };
        // DescriptorHeap��ݒ�(���\�b�h�ŕ`��Ɏg�p���邷�ׂĂ�DescriptorHeap��ݒ肷��)
        m_CmdList->SetDescriptorHeaps(ARRAYSIZE(descHeaps), descHeaps);
        //                     (GPU�����猩���A�h���X���������n���h����n��)
        m_CmdList->SetGraphicsRootDescriptorTable(0, m_DescHeapCbvSrvUav->GetGPUDescriptorHandleForHeapStart());
        m_CmdList->SetPipelineState(m_Pso.Get());
        m_CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_CmdList->IASetVertexBuffers(0, 1, &m_VBView);
        m_CmdList->IASetIndexBuffer(&m_IBView);
        m_CmdList->DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);

        // Barrier Present -> Present
        setResourceBarrier(m_CmdList.Get(), d3dBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        // Exec
        // �R�}���h���X�g�ւ̋L�^���I�����A�R�}���h���s
        checkHresult(m_CmdList->Close(), "m_CmdList->Close() is failed value");
        ID3D12CommandList * const cmdList = m_CmdList.Get();
        m_CmdQueue->ExecuteCommandLists(1, &cmdList);

        // Present
        checkHresult(m_SwapChain->Present(1, 0), "m_SwapChain->Present() is failed value");

        // Set queue flushed event
        // ����Fence�ɂ����āAm_FrameCount�̒l�ɂȂ�����C�x���g�𔭉΂�����
        checkHresult(
            m_Fence->SetEventOnCompletion(m_FrameCount, m_FenceEveneHandle),
            "m_Fence->SetEventOnVompletion() is failed value"
        );

        // Wait for queue flushed
        // This code would CPU stall!
        //      ID3D12CommandQueue->Signal() 
        //        : �ȑO�ɃT�u�~�b�g���ꂽ�R�}���h��GPU��ł̎��s���������Ă���A���邢�͊������I�����
        //          �Ƃ��A�����I�ɑ�1�����Ŏw�肵��Fence�̒l���Q�����̒l�ɍX�V����B
        checkHresult(m_CmdQueue->Signal(m_Fence.Get(), m_FrameCount), "m_CmdQueue->Signal() is failed value");
        DWORD wait = WaitForSingleObject(m_FenceEveneHandle, INFINITE/*10000*/);
        if (wait != WAIT_OBJECT_0) {
            throw std::runtime_error("Failed WaitForSingleObject().");
        }
        // �R�}���h���X�g�ƃR�}���h�A���P�[�^�����Z�b�g
        //      ��CommandAllocator��Reset()�́A���܂Ńo�C���h���ꂽ���ׂĂ�
        //        CommandList��GPU�ł̎��s���I����܂ŁAReset()���Ăяo���Ă͂����Ȃ��B
        checkHresult(m_CmdAlloc->Reset(), "m_CmdAlloc->Reset() is failed value");
        checkHresult(m_CmdList->Reset(m_CmdAlloc.Get(), nullptr), "m_CmdList->Reset() is failed value");
        

    }
    void DirectX12::begineDraw()
    {
    }
    void DirectX12::endDraw()
    {
    }
    // �f�o�C�X�̎擾
    std::shared_ptr<ID3D12Device> DirectX12::getDevice() const
    {
        return m_spDevice;
    }
    // �`��̊J�n�E�I�����Ɏg���o���A
    //      ���\�[�X�̏�ԑJ�ڂ��\�ɂȂ�܂Ńo���A��ݒu����
    void DirectX12::setResourceBarrier(ID3D12GraphicsCommandList * commandList, ID3D12Resource * res, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
    {
        D3D12_RESOURCE_BARRIER desc = {};
        desc.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        desc.Transition.pResource   = res;
        desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        desc.Transition.StateBefore = before;
        desc.Transition.StateAfter  = after;
        desc.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;

        commandList->ResourceBarrier(1, &desc);
    }
}
