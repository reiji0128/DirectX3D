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
		DirectX::XMFLOAT3 Diffuse;      // �g�U���ː���
		DirectX::XMFLOAT3 Specular;     // ���ʔ��ː���
		float             Alpha;        // ���ߐ���
		float             Shininess;    // ���ʔ��ˋ��x
		std::wstring      DiffuseMap;   // �f�B�t���[�Y�}�b�v�t�@�C���p�X
		std::wstring      SpecularMap;  // �X�y�L�����[�}�b�v�t�@�C���p�X
		std::wstring      ShininessMap; // �V���C�l�X�}�b�v�t�@�C���p�X
		std::wstring      NormalMap;    // �@���}�b�v�t�@�C���p�X
	};

	class MeshVertex
	{
	public:

		DirectX::XMFLOAT3 Position;  // �ʒu���W
		DirectX::XMFLOAT3 Normal;    // �@��
		DirectX::XMFLOAT2 TexCoord;  // �e�N�X�`�����W
		DirectX::XMFLOAT3 Tangent;   // �ڐ��x�N�g��

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
		std::vector<MeshVertex> Vertices;   // ���_�f�[�^
		std::vector<uint32_t>   Indices;    // ���_�C���f�b�N�X
		uint32_t                MaterialID; // �}�e���A���ԍ�
	};

	class MeshLoader
	{
	public:

		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		MeshLoader();

		/// <summary>
		// �f�X�g���N�^
		/// </summary>
		~MeshLoader();

		/// <summary>
		/// ���b�V���̃��[�h
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
		/// ���b�V���f�[�^�̉��
		/// </summary>
		/// <param name="dstMesh"></param>
		/// <param name="pSrcMesh"></param>
		void ParseMesh(ResMesh& dstMesh, const aiMesh* pSrcMesh);

		/// <summary>
		/// �}�e���A���f�[�^�̉��
		/// </summary>
		/// <param name="dstMaterial"></param>
		/// <param name="pSrcMaterial"></param>
		void ParseMaterial(ResMaterial& dstMaterial, const aiMaterial* pSrcMaterial);

		const aiScene* m_pScene = nullptr;  // �V�[���f�[�^
	};

	bool LoadMesh(const wchar_t*            filename,
		          std::vector<ResMesh>&     meshes,
		          std::vector<ResMaterial>& materials);
}

namespace System
{
	/// <summary>
	///  wchar_t ���� char�^(UTF-8)�ɕϊ�
	/// </summary>
	/// <param name="value"></param>
	/// <returns></returns>
	std::string ToUTF8(const std::wstring& value);

	/// <summary>
	/// std::wstring�^�ɕϊ�
	/// </summary>
	/// <param name="path">aiString�^�̃p�X</param>
	/// <returns>std::wsting�^�̃p�X</returns>
	std::wstring Convert(const aiString& path);
}