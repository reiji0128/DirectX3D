#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <assimp/scene.h>
#include <string>
#include <vector>

namespace Resource
{
	struct ResMaterial
	{
		DirectX::XMFLOAT3 Diffuse;      // 拡散反射成分
		DirectX::XMFLOAT3 Specular;     // 鏡面反射成分
		float             Alpha;        // 透過成分
		float             Shininess;    // 鏡面反射強度
		std::wstring      DiffuseMap;   // ディフューズマップファイルパス
		std::wstring      SpecularMap;  // スペキュラーマップファイルパス
		std::wstring      ShininessMap; // シャイネスマップファイルパス
		std::wstring      NormalMap;    // 法線マップファイルパス
	};

	class MeshVertex
	{
	public:

		DirectX::XMFLOAT3 Position;  // 位置座標
		DirectX::XMFLOAT3 Normal;    // 法線
		DirectX::XMFLOAT2 TexCoord;  // テクスチャ座標
		DirectX::XMFLOAT3 Tangent;   // 接線ベクトル

		MeshVertex() = default;

		MeshVertex(DirectX::XMFLOAT3 const& position,
			       DirectX::XMFLOAT3 const& normal,
			       DirectX::XMFLOAT2 const& texCoord,
			       DirectX::XMFLOAT3 const& tangent)
			:Position(position)
			,Normal  (normal)
			,TexCoord(texCoord)
			,Tangent (tangent)
		{
		}

		static const D3D12_INPUT_LAYOUT_DESC InputLayout;

	private:

		static const int InputElementCount = 4;
		static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];

	};

	struct ResMesh
	{
		std::vector<MeshVertex> Vertices;   // 頂点データ
		std::vector<uint32_t>   Indices;    // 頂点インデックス
		uint32_t                MaterialID; // マテリアル番号
	};

	class MeshLoader
	{
	public:

		/// <summary>
		/// コンストラクタ
		/// </summary>
		MeshLoader();

		/// <summary>
		// デストラクタ
		/// </summary>
		~MeshLoader();

		/// <summary>
		/// メッシュのロード
		/// </summary>
		/// <param name="filename"></param>
		/// <param name="meshes"></param>
		/// <param name="materials"></param>
		/// <returns></returns>
		bool Load(const wchar_t* filename,
			      std::vector<ResMesh>& meshes,
			      std::vector<ResMaterial>& materials);

	private:

		/// <summary>
		/// メッシュデータの解析
		/// </summary>
		/// <param name="dstMesh"></param>
		/// <param name="pSrcMesh"></param>
		void ParseMesh(ResMesh& dstMesh, const aiMesh* pSrcMesh);

		/// <summary>
		/// マテリアルデータの解析
		/// </summary>
		/// <param name="dstMaterial"></param>
		/// <param name="pSrcMaterial"></param>
		void ParseMaterial(ResMaterial& dstMaterial, const aiMaterial* pSrcMaterial);

		const aiScene* m_pScene = nullptr;  // シーンデータ
	};

	bool LoadMesh(const wchar_t*            filename,
		          std::vector<ResMesh>&     meshes,
		          std::vector<ResMaterial>& materials);
}

namespace System
{
	/// <summary>
	///  wchar_t から char型(UTF-8)に変換
	/// </summary>
	/// <param name="value"></param>
	/// <returns></returns>
	std::string ToUTF8(const std::wstring& value);

	/// <summary>
	/// std::wstring型に変換
	/// </summary>
	/// <param name="path">aiString型のパス</param>
	/// <returns>std::wsting型のパス</returns>
	std::wstring Convert(const aiString& path);
}