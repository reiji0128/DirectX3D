#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include "ComPtr.h"
#include <cstdint>

namespace D3D
{
	class DescriptorHandle;
	class DescriptorPool;
}

namespace D3D
{
	class ColorTarget
	{
	public:

		/// <summary>
		/// コンストラクタ
		/// </summary>
		ColorTarget();

		/// <summary>
		/// デストラクタ
		/// </summary>
		~ColorTarget();

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <param name="pPoolRTV">ディスクリプタプール</param>
		/// <param name="width">幅</param>
		/// <param name="height">高さ</param>
		/// <param name="format">ピクセルフォーマット</param>
		/// <param name="useSRGB">SRGBを使用するか</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		bool Init(ID3D12Device*   pDevice,
			      DescriptorPool* pPoolRTV, 
			      DescriptorPool* pPoolSRV,
			      uint32_t        width, 
			      uint32_t        height,
				  DXGI_FORMAT     format,
		          float           clearColor[4]);

		/// <summary>
		/// バックバッファーから初期化処理を行う
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <param name="pPoolRTV">ディスクリプタプール</param>
		/// <param name="useSRGB">SRGBを使用するか</param>
		/// <param name="index">バックバッファ番号</param>
		/// <param name="pSwapChain">スワップチェイン</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		bool InitFromBackBuffer(ID3D12Device*   pDevice,
			                    DescriptorPool* pPoolRTV,
								bool            useSRGB,
			                    uint32_t        index,
			                    IDXGISwapChain* pSwapChain);

		/// <summary>
		/// 終了処理
		/// </summary>
		void Term();

		/// <summary>
		/// ビューのクリア
		/// </summary>
		/// <param name="pCmdList">コマンドリスト</param>
		void ClearView(ID3D12GraphicsCommandList* pCmdList);

	// ゲッター //

		/// <summary>
		/// ディスクリプタハンドル(RTV)の取得
		/// </summary>
		/// <returns>ディスクリプタハンドル(RTV)</returns>
		DescriptorHandle* GetHandleRTV() const { return m_pHandleRTV; }

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
		/// レンダーターゲットビューの設定の取得
		/// </summary>
		/// <returns>レンダーターゲットビューの設定</returns>
		D3D12_RENDER_TARGET_VIEW_DESC GetRTVDesc() const { return m_RTVDesc; }

		/// <summary>
		/// シェーダリソースビューの設定の取得
		/// </summary>
		/// <returns>シェーダリソースビューの設定</returns>
		D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const { return m_SRVDesc; }

	private:

		ComPtr<ID3D12Resource>          m_pTarget;       // リソース
		DescriptorHandle*               m_pHandleRTV;    // ディスクリプタハンドル(RTV)
		DescriptorHandle*               m_pHandleSRV;    // ディスクリプタハンドル(SRV)
		DescriptorPool*                 m_pPoolRTV;      // ディスクリプタプール(RTV)
		DescriptorPool*                 m_pPoolSRV;      // ディスクリプタプール(SRV)
		D3D12_RENDER_TARGET_VIEW_DESC   m_RTVDesc;       // レンダーターゲットビューの構成
		D3D12_SHADER_RESOURCE_VIEW_DESC m_SRVDesc;       // シェーダリソースビューの構成
		float                           m_ClearColor[4]; // クリアカラー

		ColorTarget     (const ColorTarget&) = delete; // アクセス禁止
		void operator = (const ColorTarget&) = delete; // アクセス禁止
	};
}