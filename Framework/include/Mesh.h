#pragma once
#include "ResMesh.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace Render
{
	class Mesh
	{
	public:

		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		Mesh();

		/// <summary>
		/// �f�X�g���N�^
		/// </summary>
		virtual ~Mesh();

		/// <summary>
		/// ����������
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="resource">���\�[�X���b�V��</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		bool Init(ID3D12Device* pDevice, const Resource::ResMesh& resource);

		/// <summary>
		/// �I������
		/// </summary>
		void Term();

		/// <summary>
		/// �`�揈��
		/// </summary>
		/// <param name="pCmdList">�R�}���h���X�g</param>
		void Draw(ID3D12GraphicsCommandList* pCmdList);

	// �Q�b�^�[ //

		/// <summary>
		/// �}�e���A��ID�̎擾
		/// </summary>
		/// <returns>�}�e���A��ID</returns>
		uint32_t GetMaterialID() const { return m_MaterialID; }

	private:

		Resource::VertexBuffer m_VB; // ���_�o�b�t�@
		Resource::IndexBuffer  m_IB; // �C���f�b�N�X�o�b�t�@
		uint32_t       m_MaterialID; // �}�e���A��ID
		uint32_t       m_IndexCount; // �C���f�b�N�X��

		Mesh            (const Mesh&) = delete; // �A�N�Z�X�֎~
		void operator = (const Mesh&) = delete; // �A�N�Z�X�֎~
	};
}