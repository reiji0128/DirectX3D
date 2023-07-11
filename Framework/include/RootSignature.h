#pragma once
#include "ComPtr.h"
#include <d3d12.h>
#include <vector>

namespace D3D
{

	enum ShaderStage
	{
		ALL = 0, // 全ステージ
		VS = 1, // 頂点シェーダ
		HS = 2, // ハルシェーダ
		DS = 3, // ドメインシェーダ
		GS = 4, // ジオメトリシェーダ
		PS = 5, // ピクセルシェーダ
	};

	enum SamplerState
	{
		PointWrap,          // ポイントサンプリング - 繰り返し
		PointClamp,         // ポイントサンプリング - クランプ
		LinearWrap,         // トライリニアサンプリング - 繰り返し
		LinearClamp,        // トライリニアサンプリング - クランプ
		AnisotropicWrap,    // 異方性サンプリング - 繰り返し
		AnisotropicClamp,   // 異方性サンプリング - クランプ
	};



	class RootSignature
	{
	public:

		RootSignature();
		~RootSignature();
		bool Init(ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC* pDesc);
		void Term();
		ID3D12RootSignature* GetPtr() const { return m_RootSignature.Get(); }

		class Desc
		{
		public:

			Desc();
			~Desc();
			Desc& Begin(int count);
			Desc& End();
			Desc& SetCBV(ShaderStage stage, int index, uint32_t reg);
			Desc& SetSRV(ShaderStage stage, int index, uint32_t reg);
			Desc& SetUAV(ShaderStage stage, int index, uint32_t reg);
			Desc& SetSmp(ShaderStage stage, int index, uint32_t reg);
			Desc& AddStaticSmp(ShaderStage stage, uint32_t reg, SamplerState state);
			Desc& AllowIL();
			Desc& AlloSO();
			const D3D12_ROOT_SIGNATURE_DESC* GetDesc() const { return &m_Desc; }

		private:

			void CheckStage(ShaderStage stage);
			void SetParam(ShaderStage stage, int index, uint32_t reg, D3D12_DESCRIPTOR_RANGE_TYPE type);

			std::vector<D3D12_DESCRIPTOR_RANGE>    m_Ranges;       // ディスクリプタの範囲設定
			std::vector<D3D12_STATIC_SAMPLER_DESC> m_Samplers;     // 静的サンプラーの設定
			std::vector<D3D12_ROOT_PARAMETER>      m_Params;       // ルートパラメータ
			D3D12_ROOT_SIGNATURE_DESC              m_Desc;         // ルートシグネチャの設定
			bool                                   m_DenyStage[5]; // シェーダステージでサンプラーの使用を禁止するか
			uint32_t                               m_Flags;        
		};

	private:

		ComPtr<ID3D12RootSignature> m_RootSignature;
	};
}