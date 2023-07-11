#pragma once
#include <d3d12.h>
#include "ComPtr.h"

namespace Resource
{
	class VertexBuffer
	{
	public:

		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		VertexBuffer();

		/// <summary>
		/// �f�X�g���N�^
		/// </summary>
		~VertexBuffer();

		/// <summary>
		/// ����������
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="size">���_�o�b�t�@�̃T�C�Y</param>
		/// <param name="stride">1���_������̃T�C�Y</param>
		/// <param name="pInitData">�������f�[�^</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		bool Init(ID3D12Device* pDevice, size_t size, size_t stride, const void* pInitData = nullptr);

		/// <summary>
		/// ����������
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="size">���_��</param>
		/// <param name="pInitData">�������f�[�^</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		template<typename T>
		bool Init(ID3D12Device* pDevice, size_t count, const T* pInitData = nullptr)
		{
			return Init(pDevice, sizeof(T) * count,sizeof(T), pInitData);
		}

		/// <summary>
		/// �I������
		/// </summary>
		void Term();

		/// <summary>
		/// �������}�b�s���O
		/// </summary>
		/// <returns></returns>
		void* Map() const;

		/// <summary>
		/// �������}�b�s���O
		/// </summary>
		/// <returns></returns>
		template<typename T>
		T* Map() const { return reinterpret_cast<T*>(Map()); }

		/// <summary>
		/// �������}�b�s���O�̉���
		/// </summary>
		void Unmap();

	// �Q�b�^�[ //

		/// <summary>
		/// ���_�o�b�t�@�r���[�̎擾
		/// </summary>
		/// <returns></returns>
		D3D12_VERTEX_BUFFER_VIEW GetView() const { return m_View; }

		const D3D12_VERTEX_BUFFER_VIEW* GetViewP() const { return &m_View; }

	private:

		ComPtr<ID3D12Resource>   m_pVB;  // ���_�o�b�t�@
		D3D12_VERTEX_BUFFER_VIEW m_View; // ���_�o�b�t�@�r���[

		VertexBuffer    (const VertexBuffer&) = delete; // �A�N�Z�X�֎~
		void operator = (const VertexBuffer&) = delete; // �A�N�Z�X�֎~

	};
	
}