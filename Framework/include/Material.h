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
			TEXTURE_USAGE_DIFFUSE = 0, // �f�B�t���[�Y�}�b�v�Ƃ��ė��p
			TEXTURE_USAGE_SPECULAR,    // �X�y�L�����[�}�b�v�Ƃ��ė��p
			TEXTURE_USAGE_SHININESS,   // �V���C�l�X�}�b�v�Ƃ��ė��p
			TEXTURE_USAGE_NORMAL,      // �@���}�b�v�Ƃ��ė��p

			TEXTURE_USAGE_BASE_COLOR,  // �x�[�X�J���[�}�b�v�Ƃ��ė��p
			TEXTURE_USAGE_METALLIC,    // ���^���b�N�}�b�v�Ƃ��ė��p
			TEXTURE_USAGE_ROUGHNESS,   // ���t�l�X�}�b�v�Ƃ��ė��p

			TEXTURE_USAGE_COUNT
		};

		/// <summary>
		/// �R���X�g���N�^
		/// </summary>
		Material();

		/// <summary>
		/// �f�X�g���N�^
		/// </summary>
		~Material();

		/// <summary>
		/// ����������
		/// </summary>
		/// <param name="pDevice">�f�o�C�X</param>
		/// <param name="pPool">�f�B�X�N���v�^�v�[��</param>
		/// <param name="buffSize">1�}�e���A��������̒萔�o�b�t�@�̃T�C�Y</param>
		/// <param name="count">�}�e���A����</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
		/// </returns>
		bool Init(ID3D12Device* pDevice, D3D::DescriptorPool* pPool, size_t bufferSize, size_t count);

		/// <summary>
		/// �I������
		/// </summary>
		void Term();

		/// <summary>
		/// �e�N�X�`���̐ݒ�
		/// </summary>
		/// <param name="index">�}�e���A���ԍ�</param>
		/// <param name="usage">�e�N�X�`���̎g�p���@</param>
		/// <param name="path">�e�N�X�`���̃t�@�C���p�X</param>
		/// <param name="batch">���\�[�X�A�b�v���[�h�o�b�`</param>
		/// <returns>
		/// true  : �ݒ�ɐ���
		/// false : �ݒ�Ɏ��s
		/// </returns>
		bool SetTexture(size_t index, TEXTURE_USAGE usage, const std::wstring& path, DirectX::ResourceUploadBatch& batch);

	// �Q�b�^�[ //

		/// <summary>
		/// �萔�o�b�t�@�̃|�C���^�̎擾
		/// </summary>
		/// <param name="index">�擾����}�e���A���ԍ�</param>
		/// <returns>�}�e���A���ԍ��ɑΉ�����|�C���^</returns>
		void* GetBufferPtr(size_t index) const;

		/// <summary>
		/// �萔�o�b�t�@�̃|�C���^�̎擾
		/// </summary>
		/// <param name="index">�擾����}�e���A���ԍ�</param>
		/// <returns>�}�e���A���ԍ��ɑΉ�����|�C���^</returns>
		template<typename T>
		T* GetBufferPtr(size_t index) const { return reinterpret_cast<T*>(GetBufferPtr(index)); }

		/// <summary>
		/// �萔�o�b�t�@��GPU���z�A�h���X�̎擾
		/// </summary>
		/// <param name="index">�}�e���A���ԍ�</param>
		/// <returns>�}�e���A���ԍ��ɑΉ�����萔�o�b�t�@��GPU���z�A�h���X</returns>
		D3D12_GPU_VIRTUAL_ADDRESS GetBufferAddress(size_t index) const;

		/// <summary>
		/// �e�N�X�`���n���h���̎擾
		/// </summary>
		/// <param name="index">�}�e���A���ԍ�</param>
		/// <param name="usage">�e�N�X�`���̎g�p���@</param>
		/// <returns>�}�e���A���ԍ��ɑΉ�����e�N�X�`���̃f�B�X�N���v�^�n���h��</returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle(size_t index, TEXTURE_USAGE usage) const;

		/// <summary>
		/// �}�e���A�����̎擾
		/// </summary>
		/// <returns>�}�e���A����</returns>
		size_t GetCount() const;

	private:

		struct Subset
		{
			ConstantBuffer*             pConstantBuffer;                    // �萔�o�b�t�@
			D3D12_GPU_DESCRIPTOR_HANDLE TextureHandle[TEXTURE_USAGE_COUNT]; // �e�N�X�`���n���h��
		};

		std::map <std::wstring, Texture*> m_pTexture; // �e�N�X�`��
		std::vector<Subset>               m_Subset;   // �T�u�Z�b�g
		ID3D12Device*                     m_pDevice;  // �f�o�C�X
		D3D::DescriptorPool*              m_pPool;    // �f�B�X�N���v�^�v�[��

		Material        (const Material&) = delete; // �A�N�Z�X�֎~
		void operator = (const Material&) = delete; // �A�N�Z�X�֎~
	};

	constexpr auto TU_DIFFUSE    = Material::TEXTURE_USAGE_DIFFUSE;
	constexpr auto TU_SPECULAR   = Material::TEXTURE_USAGE_SPECULAR;
	constexpr auto TU_SHININESS  = Material::TEXTURE_USAGE_SHININESS;
	constexpr auto TU_NORMAL     = Material::TEXTURE_USAGE_NORMAL;

	constexpr auto TU_BASE_COLOR = Material::TEXTURE_USAGE_BASE_COLOR;
	constexpr auto TU_METALLIC   = Material::TEXTURE_USAGE_METALLIC;
	constexpr auto TU_ROUGHNESS  = Material::TEXTURE_USAGE_ROUGHNESS;
}