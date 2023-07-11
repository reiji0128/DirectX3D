#pragma once
#include<d3d12.h>
#include<atomic>
#include"ComPtr.h"
#include"Pool.h"

// 前方宣言
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

		D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU; // CPUディスクリプタハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU; // GPUディスクリプタハンドル

		bool HasCPU() const { return HandleCPU.ptr != 0; }

		bool HasGPU() const { return HandleGPU.ptr != 0; }
	};

	class DescriptorPool
	{
	public:

		/// <summary>
		/// 生成処理
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <param name="pDesc">ディスクリプタヒープの構成設定</param>
		/// <param name="ppPool">ディスクリプタプールの格納先</param>
		/// <returns>
		/// true  : 生成に成功
		/// false : 生成に失敗
		/// </returns>
		static bool Create(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc, DescriptorPool** ppPool);

		/// <summary>
		/// 参照カウントを増やす
		/// </summary>
		void AddRef();

		/// <summary>
		/// 解放処理
		/// </summary>
		void Release();

		/// <summary>
		/// ディスクリプタハンドルの割り当て
		/// </summary>
		/// <returns>割り当てられたディスクリプタハンドル</returns>
		DescriptorHandle* AllocHandle();

		/// <summary>
		/// ディスクリプタハンドルの解放
		/// </summary>
		/// <param name="pHandle"></param>
		void FreeHandle(DescriptorHandle*& pHandle);

		// ゲッター //

		/// <summary>
		/// 参照カウントの取得
		/// </summary>
		/// <returns></returns>
		uint32_t GetCount() const;

		/// <summary>
		/// 利用可能なハンドル数の取得
		/// </summary>
		/// <returns>利用可能なハンドル数</returns>
		uint32_t GetAvailableHandleCount() const;

		/// <summary>
		/// 割り当て済みのハンドル数の取得
		/// </summary>
		/// <returns>割り当て済みのハンドル数</returns>
		uint32_t GetAllocatedHandleCount() const;

		/// <summary>
		/// ハンドル総数の取得
		/// </summary>
		/// <returns>ハンドル総数</returns>
		uint32_t GetHandleCount() const;

		/// <summary>
		/// ディスクリプタヒープの取得
		/// </summary>
		/// <returns>ディスクリプタヒープ</returns>
		ID3D12DescriptorHeap* const GetHeap() const;

	private:

		/// <summary>
		/// コンストラクタ
		/// </summary>
		DescriptorPool();

		/// <summary>
		/// デストラクタ
		/// </summary>
		~DescriptorPool();

		std::atomic<uint32_t>          m_RefCount;       // 参照カウント
		System::Pool<DescriptorHandle> m_Pool;           // ディスクリプタハンドルプール
		ComPtr<ID3D12DescriptorHeap>   m_pHeap;          // ディスクリプタヒープ
		uint32_t                       m_DescriptorSize; // ディスクリプタサイズ

		DescriptorPool  (const DescriptorPool&) = delete; // アクセス禁止
		void operator = (const DescriptorPool&) = delete; // アクセス禁止
	};
}