#pragma once
#include <d3d12.h>
#include "ComPtr.h"
#include <cstdint>
#include <vector>

namespace D3D
{
	class CommandList
	{
	public:

		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		CommandList();

		/// <summary>
		/// �f�X�g���N�^
		/// </summary>
		~CommandList();

		/// <summary>
		/// ����������
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="type">�R�}���h���X�g�^�C�v</param>
		/// <param name="count">�A���P�[�^�̐�. �_�u���o�b�t�@������ꍇ��2�ɐݒ�</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		bool Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, uint32_t count);

		/// <summary>
		/// �I������
		/// </summary>
		void Term();

		/// <summary>
		/// ���Z�b�g�������s�����R�}���h���X�g�̎擾
		/// </summary>
		/// <returns>���Z�b�g�������s�����R�}���h���X�g</returns>
		ID3D12GraphicsCommandList* Reset();

	private:

		ComPtr<ID3D12GraphicsCommandList>           m_pCmdList;    // �R�}���h���X�g
		std::vector<ComPtr<ID3D12CommandAllocator>> m_pAllocators; // �R�}���h�A���P�[�^
		uint32_t                                    m_Index;       // �A���P�[�^�ԍ�

		CommandList     (const CommandList&) = delete; // �A�N�Z�X�֎~
		void operator = (const CommandList&) = delete; // �A�N�Z�X�֎~
	};
}