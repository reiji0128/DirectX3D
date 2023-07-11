#include "ColorTarget.h"
#include "DescriptorPool.h"

namespace 
{
    /// <summary>
    /// SRGB�t�H�[�}�b�g�ɕϊ�
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    DXGI_FORMAT ConvertToSRGB(DXGI_FORMAT format)
    {
        DXGI_FORMAT result = format;

        switch (format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            {
                result = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            }
            break;

        case DXGI_FORMAT_BC1_UNORM:
            {
                result = DXGI_FORMAT_BC1_UNORM_SRGB;
            }
            break;

        case DXGI_FORMAT_BC2_UNORM:
            {
                result = DXGI_FORMAT_BC2_UNORM_SRGB;
            }
            break;

        case DXGI_FORMAT_BC3_UNORM:
            {
                result = DXGI_FORMAT_BC3_UNORM_SRGB;
            }
            break;

        case DXGI_FORMAT_B8G8R8A8_UNORM:
            {
                result = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            }
            break;

        case DXGI_FORMAT_B8G8R8X8_UNORM:
            {
                result = DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
            }
            break;

        case DXGI_FORMAT_BC7_UNORM:
            {
                result = DXGI_FORMAT_BC7_UNORM_SRGB;
            }
            break;

        default:
            break;
        }

        return result;
    }
}

D3D::ColorTarget::ColorTarget()
    :m_pTarget   (nullptr)
    ,m_pHandleRTV(nullptr)
    ,m_pHandleSRV(nullptr)
    ,m_pPoolRTV  (nullptr)
    ,m_pPoolSRV  (nullptr)
    ,m_RTVDesc   ()
    ,m_SRVDesc   ()
{
    m_ClearColor[0] = 0.0f;
    m_ClearColor[1] = 0.0f;
    m_ClearColor[2] = 0.0f;
    m_ClearColor[3] = 1.0f;
}

D3D::ColorTarget::~ColorTarget()
{
    Term();
}

bool D3D::ColorTarget::Init(ID3D12Device*   pDevice, 
                            DescriptorPool* pPoolRTV,
                            DescriptorPool* pPoolSRV,
                            uint32_t        width, 
                            uint32_t        height, 
                            DXGI_FORMAT     format,
                            float           clearColor[4])
{
    if (pDevice == nullptr || pPoolRTV == nullptr || width == 0 || height == 0)
    {
        return false;
    }

    assert(m_pHandleRTV == nullptr);
    assert(m_pPoolRTV   == nullptr);

    m_pPoolRTV = pPoolRTV;
    m_pPoolRTV->AddRef();

    m_pHandleRTV = m_pPoolRTV->AllocHandle();

    if (m_pHandleRTV == nullptr)
    {
        return false;
    }

    if (pPoolSRV != nullptr)
    {
        m_pPoolSRV = pPoolSRV;
        m_pPoolSRV->AddRef();

        m_pHandleSRV = m_pPoolSRV->AllocHandle();

        if (m_pHandleSRV == nullptr)
        {
            return false;
        }
    }

    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    prop.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    prop.CreationNodeMask     = 1;
    prop.VisibleNodeMask      = 1;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment          = 0;
    desc.Width              = UINT64(width);
    desc.Height             = height;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = format;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    m_ClearColor[0] = clearColor[0];
    m_ClearColor[1] = clearColor[1];
    m_ClearColor[2] = clearColor[2];
    m_ClearColor[3] = clearColor[3];

    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format   = format;
    clearValue.Color[0] = clearColor[0];
    clearValue.Color[1] = clearColor[1];
    clearValue.Color[2] = clearColor[2];
    clearValue.Color[3] = clearColor[3];

    auto hr = pDevice->CreateCommittedResource(&prop,
                                               D3D12_HEAP_FLAG_NONE,
                                               &desc,
                                               D3D12_RESOURCE_STATE_RENDER_TARGET,
                                               &clearValue,
                                               IID_PPV_ARGS(m_pTarget.GetAddressOf()));

    if (FAILED(hr))
    {
        return false;
    }

    m_RTVDesc.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;
    m_RTVDesc.Format               = format;
    m_RTVDesc.Texture2D.MipSlice   = 0;
    m_RTVDesc.Texture2D.PlaneSlice = 0;

    pDevice->CreateRenderTargetView(m_pTarget.Get(),
                                    &m_RTVDesc,
                                    m_pHandleRTV->HandleCPU);

    if (pPoolSRV != nullptr)
    {
        m_SRVDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;
        m_SRVDesc.Format                        = format;
        m_SRVDesc.Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        m_SRVDesc.Texture2D.MostDetailedMip     = 0;
        m_SRVDesc.Texture2D.MipLevels           = 1;
        m_SRVDesc.Texture2D.PlaneSlice          = 0;
        m_SRVDesc.Texture2D.ResourceMinLODClamp = 0;

        pDevice->CreateShaderResourceView(m_pTarget.Get(), &m_SRVDesc, m_pHandleSRV->HandleCPU);
    }

    return true;
}

bool D3D::ColorTarget::InitFromBackBuffer(ID3D12Device* pDevice, DescriptorPool* pPoolRTV, bool useSRGB, uint32_t index, IDXGISwapChain* pSwapChain)
{
    if (pDevice == nullptr || pPoolRTV == nullptr || pSwapChain == nullptr)
    {
        return false;
    }

    assert(m_pHandleRTV == nullptr);
    assert(m_pPoolRTV   == nullptr);

    m_pPoolRTV = pPoolRTV;
    m_pPoolRTV->AddRef();

    m_pHandleRTV = m_pPoolRTV->AllocHandle();

    if (m_pHandleRTV == nullptr)
    {
        return false;
    }

    auto hr = pSwapChain->GetBuffer(index, IID_PPV_ARGS(m_pTarget.GetAddressOf()));

    if (FAILED(hr))
    {
        return false;
    }

    DXGI_SWAP_CHAIN_DESC desc;
    pSwapChain->GetDesc(&desc);

    DXGI_FORMAT format = desc.BufferDesc.Format;

    // sRGB�t�H�[�}�b�g���g�p����ꍇ�́AsRGB�t�H�[�}�b�g��I��
    if (useSRGB)
    {
        format = ConvertToSRGB(format);
    }

    m_RTVDesc.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;
    m_RTVDesc.Format               = format;
    m_RTVDesc.Texture2D.MipSlice   = 0;
    m_RTVDesc.Texture2D.PlaneSlice = 0;

    pDevice->CreateRenderTargetView(m_pTarget.Get(), &m_RTVDesc, m_pHandleRTV->HandleCPU);

    return true;
}

void D3D::ColorTarget::Term()
{
    m_pTarget.Reset();

    if (m_pPoolRTV != nullptr && m_pHandleRTV != nullptr)
    {
        m_pPoolRTV->FreeHandle(m_pHandleRTV);
        m_pHandleRTV = nullptr;
    }

    if (m_pPoolRTV != nullptr)
    {
        m_pPoolRTV->Release();
        m_pPoolRTV = nullptr;
    }

    if (m_pPoolSRV != nullptr && m_pHandleSRV != nullptr)
    {
        m_pPoolSRV->FreeHandle(m_pHandleSRV);
        m_pHandleSRV = nullptr;
    }

    if (m_pPoolSRV != nullptr)
    {
        m_pPoolSRV->Release();
        m_pPoolSRV = nullptr;
    }
}

void D3D::ColorTarget::ClearView(ID3D12GraphicsCommandList* pCmdList)
{
    pCmdList->ClearRenderTargetView(m_pHandleRTV->HandleCPU, m_ClearColor, 0, nullptr);
}

D3D12_RESOURCE_DESC D3D::ColorTarget::GetDesc() const
{
    if (m_pTarget == nullptr)
    {
        return D3D12_RESOURCE_DESC();
    }
    return m_pTarget->GetDesc();
}
