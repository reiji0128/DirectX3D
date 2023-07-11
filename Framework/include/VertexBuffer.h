#pragma once
#include <d3d12.h>
#include "ComPtr.h"

namespace Resource
{
	class VertexBuffer
	{
	public:

		/// <summary>
		/// コンストラクタ
		/// </summary>
		VertexBuffer();

		/// <summary>
		/// デストラクタ
		/// </summary>
		~VertexBuffer();

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <param name="size">頂点バッファのサイズ</param>
		/// <param name="stride">1頂点当たりのサイズ</param>
		/// <param name="pInitData">初期化データ</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		bool Init(ID3D12Device* pDevice, size_t size, size_t stride, const void* pInitData = nullptr);

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <param name="size">頂点数</param>
		/// <param name="pInitData">初期化データ</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		template<typename T>
		bool Init(ID3D12Device* pDevice, size_t count, const T* pInitData = nullptr)
		{
			return Init(pDevice, sizeof(T) * count,sizeof(T), pInitData);
		}

		/// <summary>
		/// 終了処理
		/// </summary>
		void Term();

		/// <summary>
		/// メモリマッピング
		/// </summary>
		/// <returns></returns>
		void* Map() const;

		/// <summary>
		/// メモリマッピング
		/// </summary>
		/// <returns></returns>
		template<typename T>
		T* Map() const { return reinterpret_cast<T*>(Map()); }

		/// <summary>
		/// メモリマッピングの解除
		/// </summary>
		void Unmap();

	// ゲッター //

		/// <summary>
		/// 頂点バッファビューの取得
		/// </summary>
		/// <returns></returns>
		D3D12_VERTEX_BUFFER_VIEW GetView() const { return m_View; }

		const D3D12_VERTEX_BUFFER_VIEW* GetViewP() const { return &m_View; }

	private:

		ComPtr<ID3D12Resource>   m_pVB;  // 頂点バッファ
		D3D12_VERTEX_BUFFER_VIEW m_View; // 頂点バッファビュー

		VertexBuffer    (const VertexBuffer&) = delete; // アクセス禁止
		void operator = (const VertexBuffer&) = delete; // アクセス禁止

	};
	
}