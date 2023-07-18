#pragma once
#include <d3d12.h>
#include "ComPtr.h"

namespace Render
{
	class ImguiRender
	{
	public:

		ImguiRender();

		~ImguiRender();

		bool Init(ID3D12Device* pDevice, HWND hwnd);

		ComPtr<ID3D12DescriptorHeap> GetHeap();

		void Begin();

		void Draw(ID3D12GraphicsCommandList* pCmdList);

		static void ImGuiWinProcHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

	private:

		bool InitWin(HWND hwnd);  // Windowns�p�̏�����
		bool InitDX12(ID3D12Device* pDevice); // DX12�p�̏�����

		ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device* pDevice);   // �q�[�v����
		ComPtr<ID3D12DescriptorHeap> m_Heap;           // �q�[�v�ێ��p
	};
}