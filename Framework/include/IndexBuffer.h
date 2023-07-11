#pragma once
#include <d3d12.h>
#include "ComPtr.h"
#include <cstdint>

namespace Resource
{
	class IndexBuffer
	{
	public:

		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		IndexBuffer();

		/// <summary>
		/// �f�X�g���N�^
		/// </summary>
		~IndexBuffer();

		/// <summary>
		/// ������
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="size">�C���f�b�N�X�o�b�t�@�T�C�Y</param>
		/// <param name="pInitData">�������f�[�^</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		bool Init(ID3D12Device* pDevice, size_t size, const uint32_t* pInitData = nullptr);

		/// <summary>
		/// �I������
		/// </summary>
		void Term();

		/// <summary>
		/// �������}�b�s���O
		/// </summary>
		/// <returns></returns>
		uint32_t* Map();

		/// <summary>
		/// �������}�b�s���O�̉���
		/// </summary>
		void Unmap();

	// �Q�b�^�[ // 

		/// <summary>
		/// �C���f�b�N�X�o�b�t�@�r���[�̎擾
		/// </summary>
		/// <returns>�C���f�b�N�X�o�b�t�@�r���[</returns>
		D3D12_INDEX_BUFFER_VIEW GetView() const { return m_View; }

	private:

		ComPtr<ID3D12Resource>  m_pIB;  // �C���f�b�N�X�o�b�t�@
		D3D12_INDEX_BUFFER_VIEW m_View; // �C���f�b�N�X�o�b�t�@�r���[

		IndexBuffer     (const IndexBuffer&) = delete; // �A�N�Z�X�֎~
		void operator = (const IndexBuffer&) = delete; // �A�N�Z�X�֎~
	};
}