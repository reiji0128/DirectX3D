#include "RootSignature.h"
#include "Logger.h"

/// <summary>
/// コンストラクタ
/// </summary>
D3D::RootSignature::RootSignature()
{
}

/// <summary>
/// デストラクタ
/// </summary>
D3D::RootSignature::~RootSignature()
{
	Term();
}

/// <summary>
/// 初期化処理
/// </summary>
/// <param name="pDevice">デバイスのポインタ</param>
/// <param name="pDesc">ルートシグネチャデスクのポインタ</param>
/// <returns>
/// true  : 初期化成功
/// false : 初期化失敗
/// </returns>
bool D3D::RootSignature::Init(ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC* pDesc)
{
	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3DBlob> pErrorBlob;

	// シリアライズ
	auto hr = D3D12SerializeRootSignature(pDesc,
		                                  D3D_ROOT_SIGNATURE_VERSION_1,
		                                  pBlob.GetAddressOf(),
		                                  pErrorBlob.GetAddressOf());

	if (FAILED(hr))
	{
		ELOG("Error : D3D12SerializeRootSignature() Failed. recode = 0x%x", hr);
		return false;
	}

	// ルートシグネチャを生成
	hr = pDevice->CreateRootSignature(0,
		                              pBlob->GetBufferPointer(),
		                              pBlob->GetBufferSize(),
		                              IID_PPV_ARGS(m_RootSignature.GetAddressOf()));

	if (FAILED(hr))
	{
		ELOG("Error : Root Signature Create Failed. retcode = 0x%x", hr);
		return false;
	}

	return true;
}

/// <summary>
/// 終了処理
/// </summary>
void D3D::RootSignature::Term()
{
	m_RootSignature.Reset();
}

/// <summary>
/// コンストラクタ
/// </summary>
D3D::RootSignature::Desc::Desc()
	:m_Desc()
	,m_Flags(0)
{
	// 各シェーダステージのサンプラーの使用設定
	for (auto i = 0; i < 5; ++i)
	{
		m_DenyStage[i] = true;
	}
}

/// <summary>
/// デストラクタ
/// </summary>
D3D::RootSignature::Desc::~Desc()
{
	m_Ranges  .clear();
	m_Samplers.clear();
	m_Params  .clear();
}

/// <summary>
/// 開始処理
/// </summary>
/// <param name="count"></param>
/// <returns></returns>
D3D::RootSignature::Desc& D3D::RootSignature::Desc::Begin(int count)
{
	m_Flags = 0;

	for (auto i = 0; i < 5; ++i)
	{
		m_DenyStage[i] = true;
	}

	memset(&m_Desc, 0, sizeof(m_Desc));

	m_Samplers.clear();
	m_Ranges.resize(count);
	m_Params.resize(count);

	return *this;
}

/// <summary>
/// 終了処理
/// </summary>
/// <returns></returns>
D3D::RootSignature::Desc& D3D::RootSignature::Desc::End()
{
	if (m_DenyStage[0])
	{
		m_Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
	}

	if (m_DenyStage[1])
	{
		m_Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	}

	if (m_DenyStage[2])
	{
		m_Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	}

	if (m_DenyStage[3])
	{
		m_Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	}

	if (m_DenyStage[4])
	{
		m_Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
	}

	m_Desc.NumParameters     = UINT(m_Params.size());
	m_Desc.pParameters       = m_Params.data();
	m_Desc.NumStaticSamplers = UINT(m_Samplers.size());
	m_Desc.pStaticSamplers   = m_Samplers.data();
	m_Desc.Flags             = D3D12_ROOT_SIGNATURE_FLAGS(m_Flags);

	return *this;
}

/// <summary>
/// 定数バッファビューの設定
/// </summary>
/// <param name="stage"></param>
/// <param name="index"></param>
/// <param name="reg"></param>
/// <returns></returns>
D3D::RootSignature::Desc& D3D::RootSignature::Desc::SetCBV(ShaderStage stage, int index, uint32_t reg)
{
	SetParam(stage, index, reg, D3D12_DESCRIPTOR_RANGE_TYPE_CBV);
	return *this;
}

/// <summary>
/// シェーダリソースビューの設定
/// </summary>
/// <param name="stage"></param>
/// <param name="index"></param>
/// <param name="reg"></param>
/// <returns></returns>
D3D::RootSignature::Desc& D3D::RootSignature::Desc::SetSRV(ShaderStage stage, int index, uint32_t reg)
{
	SetParam(stage, index, reg, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	return *this;
}

/// <summary>
/// アンオーダー度アクセスビューの設定
/// </summary>
/// <param name="stage"></param>
/// <param name="index"></param>
/// <param name="reg"></param>
/// <returns></returns>
D3D::RootSignature::Desc& D3D::RootSignature::Desc::SetUAV(ShaderStage stage, int index, uint32_t reg)
{
	SetParam(stage, index, reg, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	return *this;
}

/// <summary>
/// スティックサンプラーの設定
/// </summary>
/// <param name="stage"></param>
/// <param name="index"></param>
/// <param name="reg"></param>
/// <returns></returns>
D3D::RootSignature::Desc& D3D::RootSignature::Desc::SetSmp(ShaderStage stage, int index, uint32_t reg)
{
	SetParam(stage, index, reg, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER);
	return *this;
}

/// <summary>
/// スタティックサンプラーの追加
/// </summary>
/// <param name="stage"></param>
/// <param name="reg"></param>
/// <param name="state"></param>
/// <returns></returns>
D3D::RootSignature::Desc& D3D::RootSignature::Desc::AddStaticSmp(ShaderStage stage, uint32_t reg, SamplerState state)
{
	D3D12_STATIC_SAMPLER_DESC desc = {};

	desc.MipLODBias       = D3D12_DEFAULT_MIP_LOD_BIAS;
	desc.MaxAnisotropy    = 1;
	desc.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
	desc.BorderColor      = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	desc.MinLOD           = 0.0f;
	desc.MaxLOD           = D3D12_FLOAT32_MAX;
	desc.ShaderRegister   = reg;
	desc.RegisterSpace    = 0;
	desc.ShaderVisibility = D3D12_SHADER_VISIBILITY(stage);
	CheckStage(stage);

	switch (state)
	{
	case SamplerState::PointWrap:
		{
			desc.Filter   = D3D12_FILTER_MIN_MAG_MIP_POINT;
			desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		}
		break;

	case SamplerState::PointClamp:
		{
			desc.Filter   = D3D12_FILTER_MIN_MAG_MIP_POINT;
			desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		}
		break;

		case SamplerState::LinearWrap:
		{
			desc.Filter   = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		}
		break;

	case SamplerState::LinearClamp:
		{
			desc.Filter   = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		}
		break;

	case SamplerState::AnisotropicWrap:
		{
			desc.Filter   = D3D12_FILTER_ANISOTROPIC;
			desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			desc.MaxAnisotropy = D3D12_MAX_MAXANISOTROPY;
		}
		break;

	case SamplerState::AnisotropicClamp:
		{
			desc.Filter   = D3D12_FILTER_ANISOTROPIC;
			desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			desc.MaxAnisotropy = D3D12_MAX_MAXANISOTROPY;
		}
		break;
	}

	m_Samplers.push_back(desc);
	return *this;
}

/// <summary>
/// 入力レイアウトを許可
/// </summary>
/// <returns></returns>
D3D::RootSignature::Desc& D3D::RootSignature::Desc::AllowIL()
{
	m_Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	return *this;
}

/// <summary>
/// ストリームアウトプットを許可
/// </summary>
/// <returns></returns>
D3D::RootSignature::Desc& D3D::RootSignature::Desc::AlloSO()
{
	m_Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
	return *this;
}

/// <summary>
/// シェーダステージのチェック
/// </summary>
/// <param name="stage">チェックするシェーダステージ</param>
void D3D::RootSignature::Desc::CheckStage(ShaderStage stage)
{
	auto index = int(stage - 1);

	if (0 <= index && index < 5)
	{
		m_DenyStage[index] = false;
	}
}

/// <summary>
/// パラメータの設定
/// </summary>
/// <param name="stage"></param>
/// <param name="index"></param>
/// <param name="reg"></param>
/// <param name="type"></param>
void D3D::RootSignature::Desc::SetParam(ShaderStage stage, int index, uint32_t reg, D3D12_DESCRIPTOR_RANGE_TYPE type)
{
	if (index >= m_Params.size())
	{
		return;
	}

	m_Ranges[index].RangeType                           = type;
	m_Ranges[index].NumDescriptors                      = 1;
	m_Ranges[index].BaseShaderRegister                  = reg;
	m_Ranges[index].RegisterSpace                       = 0;
	m_Ranges[index].OffsetInDescriptorsFromTableStart   = 0;

	m_Params[index].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_Params[index].DescriptorTable.NumDescriptorRanges = 1;
	m_Params[index].DescriptorTable.pDescriptorRanges   = &m_Ranges[index];
	m_Params[index].ShaderVisibility                    = D3D12_SHADER_VISIBILITY(stage);

	CheckStage(stage);
}
