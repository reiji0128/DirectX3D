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
		/// コンストラクタ
		/// </summary>
		ConstantBuffer();

		/// <summary>
		/// デストラクタ
		/// </summary>
		~ConstantBuffer();

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <param name="pPool">ディスクリプタプール</param>
		/// <param name="size">プールのサイズ</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		bool Init(ID3D12Device* pDevice, D3D::DescriptorPool* pPool, size_t size);

		/// <summary>
		/// 終了処理
		/// </summary>
		void Term();

	// ゲッター //

		/// <summary>
		/// GPU仮想アドレスの取得
		/// </summary>
		/// <returns>GPU仮想アドレス</returns>
		D3D12_GPU_VIRTUAL_ADDRESS GetAddress() const;

		/// <summary>
		/// CPUディスクリプタハンドルの取得
		/// </summary>
		/// <returns>CPUディスクリプタハンドル</returns>
		D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPU() const;

		/// <summary>
		/// GPUディスクリプタハンドルの取得
		/// </summary>
		/// <returns>GPUディスクリプタハンドル</returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU() const;

		/// <summary>
		/// メモリマッピング済みポインタの取得
		/// </summary>
		/// <returns>メモリマッピング済みポインタ</returns>
		void* GetPtr() const;

		/// <summary>
		/// メモリマッピング済みポインタの取得
		/// </summary>
		/// <returns>メモリマッピング済みポインタ</returns>
		template<typename T>
		T* GetPtr() { return reinterpret_cast<T*>(GetPtr()); }

	private:

		ComPtr<ID3D12Resource>          m_pCB;        // 定数バッファ
		D3D::DescriptorHandle*          m_pHandle;    // ディスクリプタハンドル
		D3D::DescriptorPool*            m_pPool;      // ディスクリプタプール
		D3D12_CONSTANT_BUFFER_VIEW_DESC m_Desc;       // 定数バッファビューの構成設定
		void*                           m_pMappedPtr; // マップ済みポインタ

		ConstantBuffer  (const ConstantBuffer&) = delete; // アクセス禁止
		void operator = (const ConstantBuffer&) = delete; // アクセス禁止

	};
}