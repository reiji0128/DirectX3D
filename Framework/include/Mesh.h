#pragma once
#include "ResMesh.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace Render
{
	class Mesh
	{
	public:

		/// <summary>
		/// コンストラクタ
		/// </summary>
		Mesh();

		/// <summary>
		/// デストラクタ
		/// </summary>
		virtual ~Mesh();

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <param name="resource">リソースメッシュ</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		bool Init(ID3D12Device* pDevice, const Resource::ResMesh& resource);

		/// <summary>
		/// 終了処理
		/// </summary>
		void Term();

		/// <summary>
		/// 描画処理
		/// </summary>
		/// <param name="pCmdList">コマンドリスト</param>
		void Draw(ID3D12GraphicsCommandList* pCmdList);

	// ゲッター //

		/// <summary>
		/// マテリアルIDの取得
		/// </summary>
		/// <returns>マテリアルID</returns>
		uint32_t GetMaterialID() const { return m_MaterialID; }

	private:

		Resource::VertexBuffer m_VB; // 頂点バッファ
		Resource::IndexBuffer  m_IB; // インデックスバッファ
		uint32_t       m_MaterialID; // マテリアルID
		uint32_t       m_IndexCount; // インデックス数

		Mesh            (const Mesh&) = delete; // アクセス禁止
		void operator = (const Mesh&) = delete; // アクセス禁止
	};
}