#pragma once
#include<d3d12.h>
#include<atomic>
#include"ComPtr.h"
#include"Pool.h"

// �O���錾
namespace System
{
	template<typename T>
	class Pool;
}

namespace D3D
{

	class DescriptorHandle
	{
	public:

		D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU; // CPU�f�B�X�N���v�^�n���h��
		D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU; // GPU�f�B�X�N���v�^�n���h��

		bool HasCPU() const { return HandleCPU.ptr != 0; }

		bool HasGPU() const { return HandleGPU.ptr != 0; }
	};

	class DescriptorPool
	{
	public:

		/// <summary>
		/// ��������
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="pDesc">�f�B�X�N���v�^�q�[�v�̍\���ݒ�</param>
		/// <param name="ppPool">�f�B�X�N���v�^�v�[���̊i�[��</param>
		/// <returns>
		/// true  : �����ɐ���
		/// false : �����Ɏ��s
		/// </returns>
		static bool Create(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc, DescriptorPool** ppPool);

		/// <summary>
		/// �Q�ƃJ�E���g�𑝂₷
		/// </summary>
		void AddRef();

		/// <summary>
		/// �������
		/// </summary>
		void Release();

		/// <summary>
		/// �f�B�X�N���v�^�n���h���̊��蓖��
		/// </summary>
		/// <returns>���蓖�Ă�ꂽ�f�B�X�N���v�^�n���h��</returns>
		DescriptorHandle* AllocHandle();

		/// <summary>
		/// �f�B�X�N���v�^�n���h���̉��
		/// </summary>
		/// <param name="pHandle"></param>
		void FreeHandle(DescriptorHandle*& pHandle);

		// �Q�b�^�[ //

		/// <summary>
		/// �Q�ƃJ�E���g�̎擾
		/// </summary>
		/// <returns></returns>
		uint32_t GetCount() const;

		/// <summary>
		/// ���p�\�ȃn���h�����̎擾
		/// </summary>
		/// <returns>���p�\�ȃn���h����</returns>
		uint32_t GetAvailableHandleCount() const;

		/// <summary>
		/// ���蓖�čς݂̃n���h�����̎擾
		/// </summary>
		/// <returns>���蓖�čς݂̃n���h����</returns>
		uint32_t GetAllocatedHandleCount() const;

		/// <summary>
		/// �n���h�������̎擾
		/// </summary>
		/// <returns>�n���h������</returns>
		uint32_t GetHandleCount() const;

		/// <summary>
		/// �f�B�X�N���v�^�q�[�v�̎擾
		/// </summary>
		/// <returns>�f�B�X�N���v�^�q�[�v</returns>
		ID3D12DescriptorHeap* const GetHeap() const;

	private:

		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		DescriptorPool();

		/// <summary>
		/// �f�X�g���N�^
		/// </summary>
		~DescriptorPool();

		std::atomic<uint32_t>          m_RefCount;       // �Q�ƃJ�E���g
		System::Pool<DescriptorHandle> m_Pool;           // �f�B�X�N���v�^�n���h���v�[��
		ComPtr<ID3D12DescriptorHeap>   m_pHeap;          // �f�B�X�N���v�^�q�[�v
		uint32_t                       m_DescriptorSize; // �f�B�X�N���v�^�T�C�Y

		DescriptorPool  (const DescriptorPool&) = delete; // �A�N�Z�X�֎~
		void operator = (const DescriptorPool&) = delete; // �A�N�Z�X�֎~
	};
}