#pragma once
#include <d3d12.h>
#include "ComPtr.h"
#include <ResourceUploadBatch.h>

// 前方宣言
namespace D3D
{
	class DescriptorHandle;
	class DescriptorPool;
}

namespace Resource
{
	class Texture
	{
	public:

		/// <summary>
		/// コンストラクタ
		/// </summary>
		Texture();

		/// <summary>
		/// デストラクタ
		/// </summary>
		~Texture();

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <param name="pPool">ディスクリプタプール</param>
		/// <param name="filename">ファイル名</param>
		/// <param name="batch">更新バッチ. テクスチャの更新に必要なデータを格納</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		bool Init(ID3D12Device*                 pDevice, 
			      D3D::DescriptorPool*          pPool, 
			      const wchar_t*                filename, 
			      DirectX::ResourceUploadBatch& batch);

		/// <summary>
		/// 初期化処理(SRGBフォーマットを使用)
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <param name="pPool">ディスクリプタプール</param>
		/// <param name="filename">ファイル名</param>
		/// <param name="isSRGB">SRGBフォーマットを使用するか</param>
		/// <param name="batch">更新バッチ</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		bool Init(ID3D12Device*                 pDevice,
			      D3D::DescriptorPool*          pPool,
			      const wchar_t*                filename,
			      bool                          isSRGB,
			      DirectX::ResourceUploadBatch& batch);

		/// <summary>
		/// 初期化処理(キューブマップを使用)
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <param name="pPool">ディスクリプタプール</param>
		/// <param name="pDesc">構造設定</param>
		/// <param name="isCube">キューブマップか</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		bool Init(ID3D12Device*              pDevice, 
			      D3D::DescriptorPool*       pPool, 
			      const D3D12_RESOURCE_DESC* pDesc, 
			      D3D12_RESOURCE_STATES      initState,
			      bool                       isCube);

		/// <summary>
		/// 初期化処理(SRGBフォーマット・キューブマップを使用)
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <param name="pPool">ディスクリプタプール</param>
		/// <param name="pDesc">構成設定</param>
		/// <param name="initState"></param>
		/// <param name="isCube">キューブマップを使用するか</param>
		/// <param name="isSRGB">SRGBフォーマットを使用するか</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		bool Init(ID3D12Device*              pDevice,
			      D3D::DescriptorPool*       pPool,
			      const D3D12_RESOURCE_DESC* pDesc,
			      D3D12_RESOURCE_STATES      initState,
			      bool                       isCube,
			      bool                       isSRGB);

		/// <summary>
		/// 終了処理
		/// </summary>
		void Term();

	// ゲッター //

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

	private:

		/// <summary>
		/// シェーダーリソースビューの設定の取得
		/// </summary>
		/// <param name="isCube">キューブマップか</param>
		/// <returns>シェーダーリソースビューの設定</returns>
		D3D12_SHADER_RESOURCE_VIEW_DESC GetViewDesc(bool isCube);

		ComPtr<ID3D12Resource> m_pTex;    // テクスチャ
		D3D::DescriptorHandle* m_pHandle; // ディスクリプタハンドル
		D3D::DescriptorPool*   m_pPool;   // ディスクリプタプール

		Texture         (const Texture&) = delete; // アクセス禁止
		void operator = (const Texture&) = delete; // アクセス禁止
	};
}