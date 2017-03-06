//////////////////////////////////////////////////
// �쐬��:2017/02/26
// �X�V��:2017/03/06
// �����:got
//////////////////////////////////////////////////
#include <stdexcept>
#include <sstream>
#include "DirectX12.h"
#include "GlobalValue.h"

namespace got {
    // �R���X�g���N�^
    DirectX12::DirectX12()
        : m_BufferWidth(WINDOW_WIDTH), m_BufferHeight(WINDOW_HEIGHT)
    {
    }
    // �f�X�g���N�^
    DirectX12::~DirectX12()
    {
        CloseHandle(m_FenceEveneHandle);
    }
    // ������
    HRESULT got::DirectX12::init(std::shared_ptr<Window> _window)
    {
        m_spWindow         = _window;
        m_FrameCount       = 0;
        m_FenceEveneHandle = 0;

        if (!createDevice())           { return false; }
        if (!createCommandQueue())     { return false; }
        if (!createSwapChain())        { return false; }
        if (!createCommandAllocator()) { return false; }
        if (!createRenderTarget())     { return false; }
        if (!createCommandList())      { return false; }
        if (!compileShader())          { return false; }

        return S_OK;
    }
    // �f�o�C�X�̍쐬
    bool DirectX12::createDevice()
    {
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

        return true;
    }
    // CommandAllocator�̍쐬
    bool DirectX12::createCommandAllocator()
    {
        auto hr = m_spDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(m_CmdAlloc.ReleaseAndGetAddressOf())
        );
        if (FAILED(hr)) {
            throw std::runtime_error("ID3D12Device->CreateCommandAllocator() is failed value");
        }

        return true;
    }
    // CommandQueue�̍쐬
    bool DirectX12::createCommandQueue()
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;  // GPU�^�C���A�E�g���L��
        queueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT; // ���ڃR�}���h�L���[
        auto hr = m_spDevice->CreateCommandQueue(
            &queueDesc,
            IID_PPV_ARGS(m_CmdQueue.ReleaseAndGetAddressOf())
        );
        if (FAILED(hr)) {
            throw std::runtime_error("ID3D12Device->CreateCommandQueue() is failed value");
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

        return true;
    }
    // �X���b�v�`�F�C���̍쐬(DX11�ƕς��Ȃ�)
    bool DirectX12::createSwapChain()
    {
        DXGI_SWAP_CHAIN_DESC1 scDesc = {};
        scDesc.Width            = WINDOW_WIDTH;   //TODO:�萔��������Window�N���X�Ɏ�������
        scDesc.Height           = WINDOW_HEIGHT;  //TODO:�萔��������Window�N���X�Ɏ�������
        scDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
        scDesc.SampleDesc.Count = 1;
        scDesc.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.BufferCount      = BUFFER_COUNT;
        scDesc.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        auto hr = m_DxgiFactory->CreateSwapChainForHwnd(
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

        return true;
    }
    // CommandList�̍쐬
    bool DirectX12::createCommandList()
    {
        auto hr = m_spDevice->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_CmdAlloc.Get(),
            nullptr,
            IID_PPV_ARGS(m_CmdList.ReleaseAndGetAddressOf())
        );
        if (FAILED(hr)) {
            throw std::runtime_error("ID3D12Device->CreateCommandList() is failed value");
        }

        return true;
    }
    // �X���b�v�`�F�C����RenderTarget�Ƃ��Ďg�p���邽�߂�DescriptorHeap���쐬
    bool DirectX12::createRenderTarget()
    {
        for (int i = 0; i < BUFFER_COUNT; ++i) {
            auto hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_D3DBuffer[i].ReleaseAndGetAddressOf()));
            if (FAILED(hr)) {
                throw std::runtime_error("IDXGISwapChain1->GetBuffer() is failed value");
            }
            m_D3DBuffer[i]->SetName(L"SwapChain_Buffer");
        }

        // DescriptorHeap(DX11�ł���View�̂悤�ȑ���)
        //      Descriptor�Ƃ����A���̃o�b�t�@�������������̂Ȃ̂����L�q�����f�[�^�����݂��Ă��܂��B
        //      DX12�ł�Descriptor�𒼐ڃR�}���h�ɐςނ̂ł͂Ȃ��ADescriptorHeap�̂�����Descriptor
        //      �����݂��邩�炱����g���A�Ƃ����`�Ŏw�肷��K�v������܂��B
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;  // RenderTargetView
        desc.NumDescriptors = BUFFER_COUNT;                    // �t���[���o�b�t�@�ƃo�b�N�o�b�t�@�̐�
        desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // �V�F�[�_�[����A�N�Z�X���Ȃ��̂�NONE��OK
        desc.NodeMask       = 0;
        auto hr = m_spDevice->CreateDescriptorHeap(
            &desc,
            IID_PPV_ARGS(m_DescHeapRtv.ReleaseAndGetAddressOf())
        );
        if (FAILED(hr)) {
            throw std::runtime_error("ID3D12Device->CreateDescriptorHeap() is failed value");
        }

        // �X���b�v�`�F�C���̃o�b�t�@���ɍ쐬����DescriptorHeap�ɓo�^����
        auto rtvStep = m_spDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for (auto i = 0u; i < BUFFER_COUNT; ++i) {
            auto d = m_DescHeapRtv->GetCPUDescriptorHandleForHeapStart();
            d.ptr += i * rtvStep;
            m_spDevice->CreateRenderTargetView(m_D3DBuffer[i].Get(), nullptr, d);
        }

        return true;
    }
    // �V�F�[�_�[�̓ǂݍ���
    bool DirectX12::compileShader()
    {

        ID3D10Blob *sig, *info;
        auto rootSigDesc  = D3D12_ROOT_SIGNATURE_DESC();
        rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        auto hr           = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &info);
        if (FAILED(hr)) {
            throw std::runtime_error("D3D12SerializeRootSignature() is failed value");
        }
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
        hr = D3DCompileFromFile(L"Lib\\Shader\\ShaderSample.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", flag, 0, &vs, &info);
        if (FAILED(hr)) {
            throw std::runtime_error("VS D3DCompileFromFile() compile is failed");
        }
        // PS�̓ǂݍ���
        hr = D3DCompileFromFile(L"Lib\\Shader\\ShaderSample.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", flag, 0, &ps, &info);
        if (FAILED(hr)) {
            throw std::runtime_error("PS D3DCompileFromFile() compile is failed");
        }
        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        // ���X�^���C�U�X�e�[�g
        D3D12_RASTERIZER_DESC rasterDesc = {};
        rasterDesc.FillMode              = D3D12_FILL_MODE_SOLID;                     // �h��Ԃ��̃��[�h
        rasterDesc.CullMode              = D3D12_CULL_MODE_BACK;                      // �w�肳�ꂽ�����ɖʂ����O�p�`���`�悳�ꂠ�Ȃ����Ƃ��w�肷��
        rasterDesc.FrontCounterClockwise = FALSE;                                     // �O�p�`���\�܂��͗��������Ă��邩�ǂ��������肷��B(TRUE�̏ꍇ�F���_�������_�����O�^�[�Q�b�g��Ŕ����v���ł���A���v���̏ꍇ�������ƌ��Ȃ����BFALSE�͂��̋t)
        rasterDesc.DepthBias             = D3D12_DEFAULT_DEPTH_BIAS;                  // �w�肳�ꂽ�s�N�Z���ɂ������ꂽ���s���l
        rasterDesc.DepthBiasClamp        = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;            // �s�N�Z���̍ő�[�x�o�C�A�X
        rasterDesc.SlopeScaledDepthBias  = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;     // �w�肳�ꂽ�s�N�Z���̎Ζʏ�̃X�J���[
        rasterDesc.DepthClipEnable       = TRUE;                                      // �����Ɋ�Â��ăN���b�s���O��L���ɂ��邩�ǂ���(TRUE�̏ꍇ�F�n�[�h�E�F�A��Z�l���N���b�v����)
        rasterDesc.MultisampleEnable     = FALSE;                                     // �}���`�T���v�����O���邩�ǂ���
        rasterDesc.AntialiasedLineEnable = FALSE;                                     // �s�̃A���`�G�C���A�X��L���ɂ��邩�ǂ���(MultisampleEnable��FALSE�̏ꍇ�̂ݓK�p�����)
        rasterDesc.ForcedSampleCount     = 0;                                         // UAV�����_�����O�܂��̓��X�^���C�Y���ɋ��������T���v����(�L���Ȓl�F0,1,2,4,8�A����уI�v�V�����ł�16�B0�̓T���v��������������Ȃ�)
        rasterDesc.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF; // Conservative Rasterization���I�����I�t��������
        // �u�����h�X�e�[�g
        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.AlphaToCoverageEnable                 = FALSE;                        // �s�N�Z���������_�[ �^�[�Q�b�g�ɐݒ肷��Ƃ��ɁA�}���`�T���v�����O �e�N�j�b�N�Ƃ��ăA���t�@�g�D�J�o���b�W���g�p���邩�ǂ������w�肵�܂��B
        blendDesc.IndependentBlendEnable                = FALSE;                        // ���������̃����_�[ �^�[�Q�b�g�œƗ������u�����f�B���O��L���ɂ���ɂ́ATRUE �ɐݒ肵�܂��BFALSE �ɐݒ肷��ƁARenderTarget[0] �̃����o�[�݂̂��g�p����܂��BRenderTarget[1..7] �͖�������܂��B
        // �ȉ��̓f�t�H���g�̒l��ݒ肵�Ă���
        blendDesc.RenderTarget[0].BlendEnable           = FALSE;                        // �u�����h��L���ɂ��邩�ǂ���
        blendDesc.RenderTarget[0].LogicOpEnable         = FALSE;                        // �_�����Z��L���ɂ��邩�ǂ���
        blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_ONE;              // �s�N�Z���V�F�[�_�[�̏o�͂���RGB�l�ɑ΂��Ď��s���鑀����w�肷��D3D12_BLEND�^�̒l
        blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_ZERO;             // �����_�[�^�[�Q�b�g�̌��݂�RGB�l�ɑ΂��Ď��s���鑀����w�肷��D3D12_BLEND�^�̒l
        blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;           // SrcBlend��DestBlend������������@���`����D3D12_BLEND_OP�^�̒l
        blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;              // �s�N�Z���V�F�[�_�[���o�͂���A���t�@�l�ɑ΂��Ď��s���鑀����w�肷��D3D12_BLEND�^�̒l
        blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;             // �����_�[�^�[�Q�b�g�̌��݂̃A���t�@�l�ɑ΂��Ď��s���鑀����w�肷��D3D12_BLEND�^�̒l
        blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;           // SrcBlendAlpha��DestBlendAlpha������������@���`����D3D12_BLEND_OP�^�̒l
        blendDesc.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;          // �����_�[�^�[�Q�b�g���\������_�����Z���w�肷��D3D12_LOGIC_OP�^�̒l
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // �r�b�g�P�ʂ�OR���Z���g�p���Č������ꂽD3D12_COLOR_WRITE_ENABLE�^�̒l�̑g�ݍ��킹
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
            blendDesc.RenderTarget[i] = D3D12_RENDER_TARGET_BLEND_DESC {
                FALSE,
                FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP,
                D3D12_COLOR_WRITE_ENABLE_ALL
            };
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.PrimitiveTopologyType           = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // 
        psoDesc.InputLayout.NumElements         = 1;                                      // 
        psoDesc.InputLayout.pInputElementDescs  = inputLayout;
        psoDesc.pRootSignature                  = m_RootSignature.Get();                  // ID3D12RootSignature�I�u�W�F�N�g�ւ̃|�C���^�[
        psoDesc.IBStripCutValue                 = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        psoDesc.pRootSignature                  = m_RootSignature.Get();
        // ���_�V�F�[�_�[���L�q����D3D12_SHADER_BYTECODE�\����
        psoDesc.VS.pShaderBytecode              = vs->GetBufferPointer();                 // D3D12_SHADER_BYTECODE�\���̂ւ̃|�C���^�[
        psoDesc.VS.BytecodeLength               = vs->GetBufferSize();                    // D3D12_SHADER_BYTECODE�\���̂̃T�C�Y
        // �s�N�Z���V�F�[�_�[���L�q����D3D12_SHADER_BYTECODE�\����
        psoDesc.PS.pShaderBytecode              = ps->GetBufferPointer();                 // D3D12_SHADER_BYTECODE�\���̂ւ̃|�C���^�[
        psoDesc.PS.BytecodeLength               = ps->GetBufferSize();                    // D3D12_SHADER_BYTECODE�\���̂̃T�C�Y

        psoDesc.RasterizerState                 = rasterDesc;                             // ���X�^���C�U�̏�Ԃ��L�q����D3D12_RASTERIZER_DESC�\����
        psoDesc.BlendState                      = blendDesc;                              // �u�����h��Ԃ��L�q����D3D12_STREAM_OUTPUT_DESC�\���� 
        psoDesc.DepthStencilState.DepthEnable   = FALSE;                                  // 
        psoDesc.DepthStencilState.StencilEnable = FALSE;                                  // 
        psoDesc.SampleMask                      = UINT_MAX;                               // 
        psoDesc.NumRenderTargets                = 1;                                      // 
        psoDesc.RTVFormats[0]                   = DXGI_FORMAT_R8G8B8A8_UNORM;             // 
        psoDesc.SampleDesc.Count                = 1;                                      // 
        hr = m_spDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_Pso.ReleaseAndGetAddressOf()));
        if (FAILED(hr)) {
            throw std::runtime_error("ID3D12Device->CreateGraphicsPipelineState() is failed value");
        }
        vs->Release();
        ps->Release();

        struct float3 
        {
            float f[3];
        };

        float3 vbData[4] =
        {
            { -0.7f,  0.7f,  0.0f },
            {  0.7f,  0.7f,  0.0f },
            { -0.7f, -0.7f,  0.0f },
            {  0.7f, -0.7f,  0.0f }
        };
        unsigned short idData[6] = { 0, 1, 2, 2, 1, 3 };
        // 
        D3D12_HEAP_PROPERTIES heapProps;
        heapProps.Type                 = D3D12_HEAP_TYPE_UPLOAD;          // �q�[�v�^�C�v���w�肷��D3D12_CPU_PROPERTY�^�̒l
        heapProps.CreationNodeMask     = 1;                               // �q�[�v��CPU�y�[�W�v���p�e�B���w�肷��D3D12_CPU_PAGE_PROPERTY�^�̒l
        heapProps.VisibleNodeMask      = 1;                               // �q�[�v��CPU�v���p�e�B
        heapProps.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // 
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;       // 
        D3D12_RESOURCE_DESC   descResourceTex;
        descResourceTex.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
        descResourceTex.Alignment          = 0;
        descResourceTex.Width              = sizeof(idData) + sizeof(vbData);
        descResourceTex.Height             = 1;
        descResourceTex.DepthOrArraySize   = 1;
        descResourceTex.MipLevels          = 1;
        descResourceTex.Format             = DXGI_FORMAT_UNKNOWN;
        descResourceTex.SampleDesc.Count   = 1;
        descResourceTex.SampleDesc.Quality = 0;
        descResourceTex.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        descResourceTex.Flags              = D3D12_RESOURCE_FLAG_NONE;

        // �q�[�v�����\�[�X�S�̂��i�[����̂ɏ\���ȑ傫���Ń��\�[�X���q�[�v�Ƀ}�b�v�����悤�ɁA
        // ���\�[�X�ƈÖق̃q�[�v�������쐬����B
        hr = m_spDevice->CreateCommittedResource
        (
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &descResourceTex,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_VB.ReleaseAndGetAddressOf())
        );
        if (FAILED(hr)) {
            throw std::runtime_error("ID3D12Device->CreateCommittedResource() is failed value");
        }
        m_VB->SetName(L"VertexBuffer");
        char* vbUploadPtr = nullptr;
        hr = m_VB->Map(0, nullptr, reinterpret_cast<void**>(&vbUploadPtr));
        memcpy_s(vbUploadPtr, sizeof(vbData), vbData, sizeof(vbData));
        memcpy_s(vbUploadPtr + sizeof(vbData), sizeof(idData), idData, sizeof(idData));
        m_VB->Unmap(0, nullptr);

        m_VBView.BufferLocation = m_VB->GetGPUVirtualAddress();
        m_VBView.StrideInBytes  = sizeof(float3);
        m_VBView.SizeInBytes    = sizeof(vbData);
        m_IBView.BufferLocation = m_VB->GetGPUVirtualAddress() + sizeof(vbData);
        m_IBView.Format         = DXGI_FORMAT_R16_UINT;
        m_IBView.SizeInBytes    = sizeof(idData);
        //TODO: FullScreen
        //// test
        //// FIXME: Works fine only odd frame
        ////if (m_FrameCount == 9)
        ////{
        //    hr = m_SwapChain->SetFullscreenState(TRUE, nullptr);
        //    if (FAILED(hr)) {
        //        throw std::runtime_error("ID3D12->CreateGraphicsPipelineState() is failed value");
        //    }
        ////}
        ////if (m_FrameCount == 119)
        ////{
        ////    auto hr = m_SwapChain->SetFullscreenState(FALSE, nullptr);
        ////    if (FAILED(hr)) {
        ////        throw std::runtime_error("ID3D12->CreateGraphicsPipelineState() is failed value");
        ////    }
        ////}

        //BOOL isFullScreen;
        // hr = m_SwapChain->GetFullscreenState(&isFullScreen, nullptr);
        //if (FAILED(hr)) {
        //    throw std::runtime_error("IDXGISwapChain1->GetFullscreenState() is failed value");
        //}
        //if (m_IsFullScreen != isFullScreen) {
        //    m_IsFullScreen = isFullScreen;
        //    for (auto& d : m_D3DBuffer) {
        //        d.Reset();
        //    }

        //    RECT rect;
        //    GetClientRect(GetActiveWindow(), &rect);
        //    DXGI_MODE_DESC mode = {};
        //    mode.Format = DXGI_FORMAT_UNKNOWN;
        //    mode.Width = rect.right;
        //    mode.Height = rect.bottom;
        //    mode.RefreshRate.Denominator = 1;
        //    mode.RefreshRate.Numerator = 60; // FIXME: Frame rate is variant
        //    hr = m_SwapChain->ResizeTarget(&mode);
        //    if (FAILED(hr)) {
        //        throw std::runtime_error("IDXGISwapChain1->GetFullscreenState() is failed value");
        //    }
        //    hr = m_SwapChain->ResizeBuffers(BUFFER_COUNT, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
        //    if (FAILED(hr)) {
        //        throw std::runtime_error("IDXGISwapChain1->GetFullscreenState() is failed value");
        //    }
        //    auto rtvStep = m_spDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        //    for (auto i = 0u; i < BUFFER_COUNT; i++)
        //    {
        //        hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_D3DBuffer[i].ReleaseAndGetAddressOf()));
        //        if (FAILED(hr)) {
        //            throw std::runtime_error("IDXGISwapChain1->GetFullscreenState() is failed value");
        //        }
        //        m_D3DBuffer[i]->SetName(L"SwapChain_Buffer");

        //        auto desc = m_D3DBuffer[i]->GetDesc();
        //        m_BufferWidth = static_cast<int>(desc.Width);
        //        m_BufferHeight = static_cast<int>(desc.Height);
        //        std::stringstream ss;
        //        ss << "#SwapChain size changed (" << m_BufferWidth << "," << m_BufferHeight << ")" << std::endl;
        //        OutputDebugStringA(ss.str().c_str());

        //        auto d = m_DescHeapRtv->GetCPUDescriptorHandleForHeapStart();
        //        d.ptr += i * rtvStep;
        //        m_spDevice->CreateRenderTargetView(m_D3DBuffer[i].Get(), nullptr, d);
        //    }
        //}

        return true;
    }
    // �`�� TODO:�Ƃ肠����
    void DirectX12::draw()
    {
        ++m_FrameCount;
        // Get current RTV descriptor
        auto descHandleRtvStep = m_spDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE descHandleRtv = m_DescHeapRtv->GetCPUDescriptorHandleForHeapStart();
        descHandleRtv.ptr += ((m_FrameCount - 1) % BUFFER_COUNT) * descHandleRtvStep;
        // Get current swap chain
        ID3D12Resource *d3dBuffer = m_D3DBuffer[(m_FrameCount - 1) % BUFFER_COUNT].Get();

        // Barrier Present -> RenderTarget
        setResourceBarrier(m_CmdList.Get(), d3dBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        // Viewport
        D3D12_VIEWPORT viewport = {};
        viewport.Width  = static_cast<float>(m_BufferWidth);
        viewport.Height = static_cast<float>(m_BufferHeight);
        m_CmdList->RSSetViewports(1, &viewport);
        D3D12_RECT scissor = {};
        scissor.right  = static_cast<LONG>(m_BufferWidth);
        scissor.bottom = static_cast<LONG>(m_BufferHeight);
        m_CmdList->RSSetScissorRects(1, &scissor);

        // Clear
        //float clearColor[4] = {0.1f, 0.2f, 0.3f, 1.0f};
        //m_CmdList->ClearRenderTargetView(descHandleRtv, clearColor, 0, nullptr);
        {
            auto saturate = [](float a) { return a < 0 ? 0 : a > 1 ? 1 : a; };
            float clearColor[4];
            static float h = 0.0f;
            h += 0.02f;
            if (h >= 1) h = 0.0f;
            clearColor[0] = saturate(std::abs(h * 6.0f - 3.0f) - 1.0f);
            clearColor[1] = saturate(2.0f - std::abs(h * 6.0f - 2.0f));
            clearColor[2] = saturate(2.0f - std::abs(h * 6.0f - 4.0f));
            m_CmdList->ClearRenderTargetView(descHandleRtv, clearColor, 0, nullptr);
        }

        m_CmdList->OMSetRenderTargets(1, &descHandleRtv, true, nullptr);

        // Draw
        m_CmdList->SetGraphicsRootSignature(m_RootSignature.Get());
        m_CmdList->SetPipelineState(m_Pso.Get());
        m_CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_CmdList->IASetVertexBuffers(0, 1, &m_VBView);
        m_CmdList->IASetIndexBuffer(&m_IBView);
        m_CmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

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
            throw std::runtime_error("ID3D12Fence->SetEventOnVompletion() is failed value");
        }

        // Wait for queue flushed
        // This code would CPU stall!
//#if 1
        hr = m_CmdQueue->Signal(m_Fence.Get(), m_FrameCount);
        DWORD wait = WaitForSingleObject(m_FenceEveneHandle, 10000);
        if (wait != WAIT_OBJECT_0) {
            throw std::runtime_error("Failed WaitForSingleObject().");
        }
//#else
//        // Equivalent code
//        auto prev = m_Fence->GetCompletedValue();
//        hr = m_CmdQueue->Signal(m_Fence.Get(), m_FrameCount);
//        if (FAILED(hr)) {
//            throw std::runtime_error("Failed WaitForSingleObject().");
//        }
//        while (prev == m_Fence->GetCompletedValue())
//        {
//        }
//#endif
        if (FAILED(m_CmdAlloc->Reset())) {
            throw std::runtime_error("ID3D12CommandAllocator->Reset() is failed value");
        }
        if (FAILED(m_CmdList->Reset(m_CmdAlloc.Get(), nullptr))) {
            throw std::runtime_error("ID3D12GraphicsCommandList->Reset() is failed value");
        }

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
    // ���\�[�X�n�U�[�h
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
