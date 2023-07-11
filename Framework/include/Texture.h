#pragma once
#include <d3d12.h>
#include "ComPtr.h"
#include <ResourceUploadBatch.h>

// �O���錾
namespace D3D
{
	class DescriptorHandle;
	class DescriptorPool;
}

namespace Resource
{
	class Texture
	{
	public:

		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		Texture();

		/// <summary>
		/// �f�X�g���N�^
		/// </summary>
		~Texture();

		/// <summary>
		/// ����������
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="pPool">�f�B�X�N���v�^�v�[��</param>
		/// <param name="filename">�t�@�C����</param>
		/// <param name="batch">�X�V�o�b�`. �e�N�X�`���̍X�V�ɕK�v�ȃf�[�^���i�[</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		bool Init(ID3D12Device*                 pDevice, 
			      D3D::DescriptorPool*          pPool, 
			      const wchar_t*                filename, 
			      DirectX::ResourceUploadBatch& batch);

		/// <summary>
		/// ����������(SRGB�t�H�[�}�b�g���g�p)
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="pPool">�f�B�X�N���v�^�v�[��</param>
		/// <param name="filename">�t�@�C����</param>
		/// <param name="isSRGB">SRGB�t�H�[�}�b�g���g�p���邩</param>
		/// <param name="batch">�X�V�o�b�`</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		bool Init(ID3D12Device*                 pDevice,
			      D3D::DescriptorPool*          pPool,
			      const wchar_t*                filename,
			      bool                          isSRGB,
			      DirectX::ResourceUploadBatch& batch);

		/// <summary>
		/// ����������(�L���[�u�}�b�v���g�p)
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="pPool">�f�B�X�N���v�^�v�[��</param>
		/// <param name="pDesc">�\���ݒ�</param>
		/// <param name="isCube">�L���[�u�}�b�v��</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		bool Init(ID3D12Device*              pDevice, 
			      D3D::DescriptorPool*       pPool, 
			      const D3D12_RESOURCE_DESC* pDesc, 
			      D3D12_RESOURCE_STATES      initState,
			      bool                       isCube);

		/// <summary>
		/// ����������(SRGB�t�H�[�}�b�g�E�L���[�u�}�b�v���g�p)
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="pPool">�f�B�X�N���v�^�v�[��</param>
		/// <param name="pDesc">�\���ݒ�</param>
		/// <param name="initState"></param>
		/// <param name="isCube">�L���[�u�}�b�v���g�p���邩</param>
		/// <param name="isSRGB">SRGB�t�H�[�}�b�g���g�p���邩</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		bool Init(ID3D12Device*              pDevice,
			      D3D::DescriptorPool*       pPool,
			      const D3D12_RESOURCE_DESC* pDesc,
			      D3D12_RESOURCE_STATES      initState,
			      bool                       isCube,
			      bool                       isSRGB);

		/// <summary>
		/// �I������
		/// </summary>
		void Term();

	// �Q�b�^�[ //

		/// <summary>
		/// CPU�f�B�X�N���v�^�n���h���̎擾
		/// </summary>
		/// <returns>CPU�f�B�X�N���v�^�n���h��</returns>
		D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPU() const;

		/// <summary>
		/// GPU�f�B�X�N���v�^�n���h���̎擾
		/// </summary>
		/// <returns>GPU�f�B�X�N���v�^�n���h��</returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU() const;

	private:

		/// <summary>
		/// �V�F�[�_�[���\�[�X�r���[�̐ݒ�̎擾
		/// </summary>
		/// <param name="isCube">�L���[�u�}�b�v��</param>
		/// <returns>�V�F�[�_�[���\�[�X�r���[�̐ݒ�</returns>
		D3D12_SHADER_RESOURCE_VIEW_DESC GetViewDesc(bool isCube);

		ComPtr<ID3D12Resource> m_pTex;    // �e�N�X�`��
		D3D::DescriptorHandle* m_pHandle; // �f�B�X�N���v�^�n���h��
		D3D::DescriptorPool*   m_pPool;   // �f�B�X�N���v�^�v�[��

		Texture         (const Texture&) = delete; // �A�N�Z�X�֎~
		void operator = (const Texture&) = delete; // �A�N�Z�X�֎~
	};
}