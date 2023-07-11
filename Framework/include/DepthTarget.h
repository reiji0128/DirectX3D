#pragma once
#include <d3d12.h>
#include "ComPtr.h"
#include <cstdint>

// 前方宣言
namespace D3D
{
	class DescriptorHandle;
	class DescriptorPool;
}

namespace D3D
{
	class DepthTarget
	{
	public:

		/// <summary>
		/// コンストラクタ
		/// </summary>
		DepthTarget();

		/// <summary>
		/// デストラクタ
		/// </summary>
		~DepthTarget();

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <param name="pPoolDSV">ディスクリプタプール</param>
		/// <param name="width">幅</param>
		/// <param name="height">高さ</param>
		/// <param name="format">ピクセルフォーマット</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		bool Init(ID3D12Device*   pDevice,
			      DescriptorPool* pPoolDSV,
			      DescriptorPool* pPoolSRV,
			      uint32_t        width,
			      uint32_t        height,
			      DXGI_FORMAT     format,
			      float           clearDepth,
			      uint8_t         clearStencil);

		/// <summary>
		/// 終了処理
		/// </summary>
		void Term();

		void ClearView(ID3D12GraphicsCommandList* pCmdList);

	// ゲッター //

		/// <summary>
		/// ディスクリプタハンドル(DSV)の取得
		/// </summary>
		/// <returns>ディスクリプタハンドル(DSV)</returns>
		DescriptorHandle* GetHandleDSV() const { return m_pHandleDSV; }

		/// <summary>
		/// ディスクリプタハンドル(SRV)の取得
		/// </summary>
		/// <returns>ディスクリプタハンドル(SRV)</returns>
		DescriptorHandle* GetHandleSRV() const { return m_pHandleSRV; }

		/// <summary>
		/// リソースの取得
		/// </summary>
		/// <returns>リソース</returns>
		ID3D12Resource* GetResource() const { return m_pTarget.Get(); }

		/// <summary>
		/// リソース設定の取得
		/// </summary>
		/// <returns>リソース設定</returns>
		D3D12_RESOURCE_DESC GetDesc() const;

		/// <summary>
		/// 深度ステンシルビューの設定の取得
		/// </summary>
		/// <returns>深度ステンシルビューの設定</returns>
		D3D12_DEPTH_STENCIL_VIEW_DESC GetDSVDesc() const { return m_DSVDesc; }

		/// <summary>
		/// シェーダリソースビューの設定の取得
		/// </summary>
		/// <returns></returns>
		D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const { return m_SRVDesc; }

	private:

		ComPtr<ID3D12Resource>          m_pTarget;      // リソース
		DescriptorHandle*               m_pHandleDSV;   // ディスクリプターハンドル(DSV)
		DescriptorHandle*               m_pHandleSRV;   // ディスクリプターハンドル(SRV)
		DescriptorPool*                 m_pPoolDSV;     // ディスクリプタプール(DSV)
		DescriptorPool*                 m_pPoolSRV;     // ディスクリプタプール(SRV)
		D3D12_DEPTH_STENCIL_VIEW_DESC   m_DSVDesc;      // 深度ステンシルビューの設定
		D3D12_SHADER_RESOURCE_VIEW_DESC m_SRVDesc;      // シェーダリソースビューの設定
		float                           m_ClearDepth;	// クリア深度
		uint8_t                         m_ClearStencil; // クリアステンシル

		DepthTarget     (const DepthTarget&) = delete; // アクセス禁止
		void operator = (const DepthTarget&) = delete; // アクセス禁止
	};
}