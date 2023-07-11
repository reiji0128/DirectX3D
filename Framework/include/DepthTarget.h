#pragma once
#include <d3d12.h>
#include "ComPtr.h"
#include <cstdint>

// �O���錾
namespace D3D
{
	class DescriptorHandle;
	class DescriptorPool;
}

namespace D3D
{
	class DepthTarget
	{
	public:

		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		DepthTarget();

		/// <summary>
		/// �f�X�g���N�^
		/// </summary>
		~DepthTarget();

		/// <summary>
		/// ����������
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="pPoolDSV">�f�B�X�N���v�^�v�[��</param>
		/// <param name="width">��</param>
		/// <param name="height">����</param>
		/// <param name="format">�s�N�Z���t�H�[�}�b�g</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		bool Init(ID3D12Device*   pDevice,
			      DescriptorPool* pPoolDSV,
			      DescriptorPool* pPoolSRV,
			      uint32_t        width,
			      uint32_t        height,
			      DXGI_FORMAT     format,
			      float           clearDepth,
			      uint8_t         clearStencil);

		/// <summary>
		/// �I������
		/// </summary>
		void Term();

		void ClearView(ID3D12GraphicsCommandList* pCmdList);

	// �Q�b�^�[ //

		/// <summary>
		/// �f�B�X�N���v�^�n���h��(DSV)�̎擾
		/// </summary>
		/// <returns>�f�B�X�N���v�^�n���h��(DSV)</returns>
		DescriptorHandle* GetHandleDSV() const { return m_pHandleDSV; }

		/// <summary>
		/// �f�B�X�N���v�^�n���h��(SRV)�̎擾
		/// </summary>
		/// <returns>�f�B�X�N���v�^�n���h��(SRV)</returns>
		DescriptorHandle* GetHandleSRV() const { return m_pHandleSRV; }

		/// <summary>
		/// ���\�[�X�̎擾
		/// </summary>
		/// <returns>���\�[�X</returns>
		ID3D12Resource* GetResource() const { return m_pTarget.Get(); }

		/// <summary>
		/// ���\�[�X�ݒ�̎擾
		/// </summary>
		/// <returns>���\�[�X�ݒ�</returns>
		D3D12_RESOURCE_DESC GetDesc() const;

		/// <summary>
		/// �[�x�X�e���V���r���[�̐ݒ�̎擾
		/// </summary>
		/// <returns>�[�x�X�e���V���r���[�̐ݒ�</returns>
		D3D12_DEPTH_STENCIL_VIEW_DESC GetDSVDesc() const { return m_DSVDesc; }

		/// <summary>
		/// �V�F�[�_���\�[�X�r���[�̐ݒ�̎擾
		/// </summary>
		/// <returns></returns>
		D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const { return m_SRVDesc; }

	private:

		ComPtr<ID3D12Resource>          m_pTarget;      // ���\�[�X
		DescriptorHandle*               m_pHandleDSV;   // �f�B�X�N���v�^�[�n���h��(DSV)
		DescriptorHandle*               m_pHandleSRV;   // �f�B�X�N���v�^�[�n���h��(SRV)
		DescriptorPool*                 m_pPoolDSV;     // �f�B�X�N���v�^�v�[��(DSV)
		DescriptorPool*                 m_pPoolSRV;     // �f�B�X�N���v�^�v�[��(SRV)
		D3D12_DEPTH_STENCIL_VIEW_DESC   m_DSVDesc;      // �[�x�X�e���V���r���[�̐ݒ�
		D3D12_SHADER_RESOURCE_VIEW_DESC m_SRVDesc;      // �V�F�[�_���\�[�X�r���[�̐ݒ�
		float                           m_ClearDepth;	// �N���A�[�x
		uint8_t                         m_ClearStencil; // �N���A�X�e���V��

		DepthTarget     (const DepthTarget&) = delete; // �A�N�Z�X�֎~
		void operator = (const DepthTarget&) = delete; // �A�N�Z�X�֎~
	};
}