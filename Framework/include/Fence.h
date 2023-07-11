#pragma once
#include <d3d12.h>
#include "ComPtr.h"

namespace D3D
{
	class Fence
	{
	public:

		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		Fence();

		/// <summary>
		/// �f�X�g���N�^
		/// </summary>
		~Fence();

		/// <summary>
		/// ����������
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		bool Init(ID3D12Device* pDevice);

		/// <summary>
		/// �I������
		/// </summary>
		void Term();

		/// <summary>
		/// �V�O�i����ԂɂȂ�܂Ŏw�肳�ꂽ���ԑҋ@
		/// </summary>
		/// <param name="pQueue">�R�}���h�L���[</param>
		/// <param name="timeout">�^�C���A�E�g����(�~���b)</param>
		void Wait(ID3D12CommandQueue* pQueue, UINT timeout);

		/// <summary>
		/// �V�O�i����ԂɂȂ�܂őҋ@
		/// </summary>
		/// <param name="pQueue">�R�}���h�L���[</param>
		void Sync(ID3D12CommandQueue* pQueue);

	private:

		ComPtr<ID3D12Fence> m_pFence;  // �t�F���X
		HANDLE              m_Event;   // �C�x���g
		UINT                m_Counter; // �J�E���^�[

		Fence           (const Fence&) = delete; // �A�N�Z�X�֎~
		void operator = (const Fence&) = delete; // �A�N�Z�X�֎~
	};
}