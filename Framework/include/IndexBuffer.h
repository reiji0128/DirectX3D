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
		/// コンストラクタ
		/// </summary>
		IndexBuffer();

		/// <summary>
		/// デストラクタ
		/// </summary>
		~IndexBuffer();

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <param name="size">インデックスバッファサイズ</param>
		/// <param name="pInitData">初期化データ</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		bool Init(ID3D12Device* pDevice, size_t size, const uint32_t* pInitData = nullptr);

		/// <summary>
		/// 終了処理
		/// </summary>
		void Term();

		/// <summary>
		/// メモリマッピング
		/// </summary>
		/// <returns></returns>
		uint32_t* Map();

		/// <summary>
		/// メモリマッピングの解除
		/// </summary>
		void Unmap();

	// ゲッター // 

		/// <summary>
		/// インデックスバッファビューの取得
		/// </summary>
		/// <returns>インデックスバッファビュー</returns>
		D3D12_INDEX_BUFFER_VIEW GetView() const { return m_View; }

	private:

		ComPtr<ID3D12Resource>  m_pIB;  // インデックスバッファ
		D3D12_INDEX_BUFFER_VIEW m_View; // インデックスバッファビュー

		IndexBuffer     (const IndexBuffer&) = delete; // アクセス禁止
		void operator = (const IndexBuffer&) = delete; // アクセス禁止
	};
}