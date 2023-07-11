#pragma once
#include <d3d12.h>
#include "ComPtr.h"
#include <vector>

namespace D3D
{
	class DescriptorHandle;
	class DescriptorPool;
}

namespace Resource
{
	class ConstantBuffer
	{
	public:

		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		ConstantBuffer();

		/// <summary>
		/// �f�X�g���N�^
		/// </summary>
		~ConstantBuffer();

		/// <summary>
		/// ����������
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="pPool">�f�B�X�N���v�^�v�[��</param>
		/// <param name="size">�v�[���̃T�C�Y</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		bool Init(ID3D12Device* pDevice, D3D::DescriptorPool* pPool, size_t size);

		/// <summary>
		/// �I������
		/// </summary>
		void Term();

	// �Q�b�^�[ //

		/// <summary>
		/// GPU���z�A�h���X�̎擾
		/// </summary>
		/// <returns>GPU���z�A�h���X</returns>
		D3D12_GPU_VIRTUAL_ADDRESS GetAddress() const;

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

		/// <summary>
		/// �������}�b�s���O�ς݃|�C���^�̎擾
		/// </summary>
		/// <returns>�������}�b�s���O�ς݃|�C���^</returns>
		void* GetPtr() const;

		/// <summary>
		/// �������}�b�s���O�ς݃|�C���^�̎擾
		/// </summary>
		/// <returns>�������}�b�s���O�ς݃|�C���^</returns>
		template<typename T>
		T* GetPtr() { return reinterpret_cast<T*>(GetPtr()); }

	private:

		ComPtr<ID3D12Resource>          m_pCB;        // �萔�o�b�t�@
		D3D::DescriptorHandle*          m_pHandle;    // �f�B�X�N���v�^�n���h��
		D3D::DescriptorPool*            m_pPool;      // �f�B�X�N���v�^�v�[��
		D3D12_CONSTANT_BUFFER_VIEW_DESC m_Desc;       // �萔�o�b�t�@�r���[�̍\���ݒ�
		void*                           m_pMappedPtr; // �}�b�v�ς݃|�C���^

		ConstantBuffer  (const ConstantBuffer&) = delete; // �A�N�Z�X�֎~
		void operator = (const ConstantBuffer&) = delete; // �A�N�Z�X�֎~

	};
}