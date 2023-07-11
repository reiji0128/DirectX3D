#pragma once
#include "DescriptorPool.h"
#include <ResourceUploadBatch.h>
#include "Texture.h"
#include "ConstantBuffer.h"
#include <map>

namespace Resource
{
	class Material
	{
	public:

		enum TEXTURE_USAGE
		{
			TEXTURE_USAGE_DIFFUSE = 0, // ディフューズマップとして利用
			TEXTURE_USAGE_SPECULAR,    // スペキュラーマップとして利用
			TEXTURE_USAGE_SHININESS,   // シャイネスマップとして利用
			TEXTURE_USAGE_NORMAL,      // 法線マップとして利用

			TEXTURE_USAGE_BASE_COLOR,  // ベースカラーマップとして利用
			TEXTURE_USAGE_METALLIC,    // メタリックマップとして利用
			TEXTURE_USAGE_ROUGHNESS,   // ラフネスマップとして利用

			TEXTURE_USAGE_COUNT
		};

		/// <summary>
		/// コンストラクタ
		/// </summary>
		Material();

		/// <summary>
		/// デストラクタ
		/// </summary>
		~Material();

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <param name="pPool">ディスクリプタプール</param>
		/// <param name="buffSize">1マテリアル当たりの定数バッファのサイズ</param>
		/// <param name="count">マテリアル数</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		bool Init(ID3D12Device* pDevice, D3D::DescriptorPool* pPool, size_t bufferSize, size_t count);

		/// <summary>
		/// 終了処理
		/// </summary>
		void Term();

		/// <summary>
		/// テクスチャの設定
		/// </summary>
		/// <param name="index">マテリアル番号</param>
		/// <param name="usage">テクスチャの使用方法</param>
		/// <param name="path">テクスチャのファイルパス</param>
		/// <param name="batch">リソースアップロードバッチ</param>
		/// <returns>
		/// true  : 設定に成功
		/// false : 設定に失敗
		/// </returns>
		bool SetTexture(size_t index, TEXTURE_USAGE usage, const std::wstring& path, DirectX::ResourceUploadBatch& batch);

	// ゲッター //

		/// <summary>
		/// 定数バッファのポインタの取得
		/// </summary>
		/// <param name="index">取得するマテリアル番号</param>
		/// <returns>マテリアル番号に対応するポインタ</returns>
		void* GetBufferPtr(size_t index) const;

		/// <summary>
		/// 定数バッファのポインタの取得
		/// </summary>
		/// <param name="index">取得するマテリアル番号</param>
		/// <returns>マテリアル番号に対応するポインタ</returns>
		template<typename T>
		T* GetBufferPtr(size_t index) const { return reinterpret_cast<T*>(GetBufferPtr(index)); }

		/// <summary>
		/// 定数バッファのGPU仮想アドレスの取得
		/// </summary>
		/// <param name="index">マテリアル番号</param>
		/// <returns>マテリアル番号に対応する定数バッファのGPU仮想アドレス</returns>
		D3D12_GPU_VIRTUAL_ADDRESS GetBufferAddress(size_t index) const;

		/// <summary>
		/// テクスチャハンドルの取得
		/// </summary>
		/// <param name="index">マテリアル番号</param>
		/// <param name="usage">テクスチャの使用方法</param>
		/// <returns>マテリアル番号に対応するテクスチャのディスクリプタハンドル</returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle(size_t index, TEXTURE_USAGE usage) const;

		/// <summary>
		/// マテリアル数の取得
		/// </summary>
		/// <returns>マテリアル数</returns>
		size_t GetCount() const;

	private:

		struct Subset
		{
			ConstantBuffer*             pConstantBuffer;                    // 定数バッファ
			D3D12_GPU_DESCRIPTOR_HANDLE TextureHandle[TEXTURE_USAGE_COUNT]; // テクスチャハンドル
		};

		std::map <std::wstring, Texture*> m_pTexture; // テクスチャ
		std::vector<Subset>               m_Subset;   // サブセット
		ID3D12Device*                     m_pDevice;  // デバイス
		D3D::DescriptorPool*              m_pPool;    // ディスクリプタプール

		Material        (const Material&) = delete; // アクセス禁止
		void operator = (const Material&) = delete; // アクセス禁止
	};

	constexpr auto TU_DIFFUSE    = Material::TEXTURE_USAGE_DIFFUSE;
	constexpr auto TU_SPECULAR   = Material::TEXTURE_USAGE_SPECULAR;
	constexpr auto TU_SHININESS  = Material::TEXTURE_USAGE_SHININESS;
	constexpr auto TU_NORMAL     = Material::TEXTURE_USAGE_NORMAL;

	constexpr auto TU_BASE_COLOR = Material::TEXTURE_USAGE_BASE_COLOR;
	constexpr auto TU_METALLIC   = Material::TEXTURE_USAGE_METALLIC;
	constexpr auto TU_ROUGHNESS  = Material::TEXTURE_USAGE_ROUGHNESS;
}