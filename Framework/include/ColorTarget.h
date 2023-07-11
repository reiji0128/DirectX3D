#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include "ComPtr.h"
#include <cstdint>

namespace D3D
{
	class DescriptorHandle;
	class DescriptorPool;
}

namespace D3D
{
	class ColorTarget
	{
	public:

		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		ColorTarget();

		/// <summary>
		/// �f�X�g���N�^
		/// </summary>
		~ColorTarget();

		/// <summary>
		/// ����������
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="pPoolRTV">�f�B�X�N���v�^�v�[��</param>
		/// <param name="width">��</param>
		/// <param name="height">����</param>
		/// <param name="format">�s�N�Z���t�H�[�}�b�g</param>
		/// <param name="useSRGB">SRGB���g�p���邩</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		bool Init(ID3D12Device*   pDevice,
			      DescriptorPool* pPoolRTV, 
			      DescriptorPool* pPoolSRV,
			      uint32_t        width, 
			      uint32_t        height,
				  DXGI_FORMAT     format,
		          float           clearColor[4]);

		/// <summary>
		/// �o�b�N�o�b�t�@�[���珉�����������s��
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="pPoolRTV">�f�B�X�N���v�^�v�[��</param>
		/// <param name="useSRGB">SRGB���g�p���邩</param>
		/// <param name="index">�o�b�N�o�b�t�@�ԍ�</param>
		/// <param name="pSwapChain">�X���b�v�`�F�C��</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		bool InitFromBackBuffer(ID3D12Device*   pDevice,
			                    DescriptorPool* pPoolRTV,
								bool            useSRGB,
			                    uint32_t        index,
			                    IDXGISwapChain* pSwapChain);

		/// <summary>
		/// �I������
		/// </summary>
		void Term();

		/// <summary>
		/// �r���[�̃N���A
		/// </summary>
		/// <param name="pCmdList">�R�}���h���X�g</param>
		void ClearView(ID3D12GraphicsCommandList* pCmdList);

	// �Q�b�^�[ //

		/// <summary>
		/// �f�B�X�N���v�^�n���h��(RTV)�̎擾
		/// </summary>
		/// <returns>�f�B�X�N���v�^�n���h��(RTV)</returns>
		DescriptorHandle* GetHandleRTV() const { return m_pHandleRTV; }

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
		/// �����_�[�^�[�Q�b�g�r���[�̐ݒ�̎擾
		/// </summary>
		/// <returns>�����_�[�^�[�Q�b�g�r���[�̐ݒ�</returns>
		D3D12_RENDER_TARGET_VIEW_DESC GetRTVDesc() const { return m_RTVDesc; }

		/// <summary>
		/// �V�F�[�_���\�[�X�r���[�̐ݒ�̎擾
		/// </summary>
		/// <returns>�V�F�[�_���\�[�X�r���[�̐ݒ�</returns>
		D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const { return m_SRVDesc; }

	private:

		ComPtr<ID3D12Resource>          m_pTarget;       // ���\�[�X
		DescriptorHandle*               m_pHandleRTV;    // �f�B�X�N���v�^�n���h��(RTV)
		DescriptorHandle*               m_pHandleSRV;    // �f�B�X�N���v�^�n���h��(SRV)
		DescriptorPool*                 m_pPoolRTV;      // �f�B�X�N���v�^�v�[��(RTV)
		DescriptorPool*                 m_pPoolSRV;      // �f�B�X�N���v�^�v�[��(SRV)
		D3D12_RENDER_TARGET_VIEW_DESC   m_RTVDesc;       // �����_�[�^�[�Q�b�g�r���[�̍\��
		D3D12_SHADER_RESOURCE_VIEW_DESC m_SRVDesc;       // �V�F�[�_���\�[�X�r���[�̍\��
		float                           m_ClearColor[4]; // �N���A�J���[

		ColorTarget     (const ColorTarget&) = delete; // �A�N�Z�X�֎~
		void operator = (const ColorTarget&) = delete; // �A�N�Z�X�֎~
	};
}