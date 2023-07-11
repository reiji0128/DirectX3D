#include "DescriptorPool.h"

D3D::DescriptorPool::DescriptorPool()
    :m_RefCount(1)
    ,m_Pool()
    ,m_pHeap()
    ,m_DescriptorSize(0)
{
}


D3D::DescriptorPool::~DescriptorPool()
{
    m_Pool.Term();
    m_pHeap.Reset();
    m_DescriptorSize = 0;
}


bool D3D::DescriptorPool::Create(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc, DescriptorPool** ppPool)
{
    // �����`�F�b�N
    if (pDevice == nullptr || pDesc == nullptr || ppPool == nullptr)
    {
        return false;
    }

    // �C���X�^���X�𐶐�
    auto instance = new (std::nothrow) DescriptorPool();

    if (instance == nullptr)
    {
        return false;
    }

    // �f�B�X�N���v�^�q�[�v�𐶐�
    auto hr = pDevice->CreateDescriptorHeap(pDesc,
                                            IID_PPV_ARGS(instance->m_pHeap.GetAddressOf()));

    // ���s���������������s���ďI��
    if (FAILED(hr))
    {
        instance->Release();
        return false;
    }

    // �v�[����������
    if (!instance->m_Pool.Init(pDesc->NumDescriptors))
    {
        instance->Release();
        return false;
    }

    // �f�B�X�N���v�^�̉��Z�T�C�Y���擾
    instance->m_DescriptorSize = pDevice->GetDescriptorHandleIncrementSize(pDesc->Type);

    // �C���X�^���X��ݒ�
    *ppPool = instance;

    return true;
}

void D3D::DescriptorPool::AddRef()
{
    m_RefCount++;
}

void D3D::DescriptorPool::Release()
{
    m_RefCount--;

    if (m_RefCount == 0)
    {
        delete this;
    }
}

D3D::DescriptorHandle* D3D::DescriptorPool::AllocHandle()
{
    // �������֐�
    auto func = [&](uint32_t index, DescriptorHandle* pHandle)
    {
        auto handleCPU = m_pHeap->GetCPUDescriptorHandleForHeapStart();
        handleCPU.ptr += m_DescriptorSize * index;

        auto handleGPU = m_pHeap->GetGPUDescriptorHandleForHeapStart();
        handleGPU.ptr += m_DescriptorSize * index;

        pHandle->HandleCPU = handleCPU;
        pHandle->HandleGPU = handleGPU;
    };

    // �������֐������s���Ă���n���h����ԋp
    return m_Pool.Alloc(func);
}

void D3D::DescriptorPool::FreeHandle(DescriptorHandle*& pHandle)
{
    if (pHandle != nullptr)
    {
        // �n���h�����v�[���ɖ߂�
        m_Pool.Free(pHandle);

        pHandle = nullptr;
    }
}

uint32_t D3D::DescriptorPool::GetCount() const
{
    return m_RefCount;
}

uint32_t D3D::DescriptorPool::GetAvailableHandleCount() const
{
    return m_Pool.GetAvailableCount();
}

uint32_t D3D::DescriptorPool::GetAllocatedHandleCount() const
{
    return m_Pool.GetUsedCount();
}

uint32_t D3D::DescriptorPool::GetHandleCount() const
{
    return m_Pool.GetSize();
}

ID3D12DescriptorHeap* const D3D::DescriptorPool::GetHeap() const
{
    return m_pHeap.Get();
}