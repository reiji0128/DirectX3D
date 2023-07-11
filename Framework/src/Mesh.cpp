#include "Mesh.h"

Render::Mesh::Mesh()
    :m_MaterialID(UINT32_MAX)
    ,m_IndexCount(0)
{
}

Render::Mesh::~Mesh()
{
    Term();
}

bool Render::Mesh::Init(ID3D12Device* pDevice, const Resource::ResMesh& resource)
{
    if (pDevice == nullptr)
    {
        return false;
    }

    if (!m_VB.Init<Resource::MeshVertex>(pDevice, resource.Vertices.size(), resource.Vertices.data()))
    {
        return false;
    }

    if (!m_IB.Init(pDevice, resource.Indices.size(), resource.Indices.data()))
    {
        return false;
    }

    m_MaterialID = resource.MaterialID;
    m_IndexCount = uint32_t(resource.Indices.size());

    return true;
}

void Render::Mesh::Term()
{
    m_VB.Term();
    m_IB.Term();
    m_MaterialID = UINT32_MAX;
    m_IndexCount = 0;
}

void Render::Mesh::Draw(ID3D12GraphicsCommandList* pCmdList)
{
    auto VBV = m_VB.GetView();
    auto IBV = m_IB.GetView();
    pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCmdList->IASetVertexBuffers(0, 1, &VBV);
    pCmdList->IASetIndexBuffer(&IBV);
    pCmdList->DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);
}