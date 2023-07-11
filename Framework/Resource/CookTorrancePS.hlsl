static const float F_PI = 3.141596535f;

struct VSOutput
{
	float4 Position : SV_Position; // 位置座標
	float2 TexCoord : TEXCOORD;    // テクスチャ座標
	float3 Normal   : NORMALE;     // 法線ベクトル
	float3 WorldPos : WORLD_POS;   // ワールド空間の位置座標
};

struct PSOutput
{
	float4 Color : SV_TARGET0; // 出力カラー
};

cbuffer LightBuffer : register(b1)
{
	float3 LightPosition  : packoffset(c0); // ライト位置
	float3 LightColor     : packoffset(c1); // ライトカラー
	float3 CameraPosition : packoffset(c2); // カメラ位置
};

cbuffer MaterialBuffer : register(b2)
{
	float3 BaseColor : packoffset(c0);   // 基本色
	float  Alpha     : packoffset(c0.w); // 透過度
	float  Roughness : packoffset(c1);   // 面の粗さ
	float  Metallic  : packoffset(c1.y); // メタリック
};

SamplerState WrapSmp  : register(s0);
Texture2D    ColorMap : register(t0);

/// <summary>
/// Schlickによるフレネル項の近似式
/// </summary>
float3 SchlichFresnel(float3 specular, float VH)
{
	return specular + (1.0f - specular * pow((1.0f - VH), 5.0f));
}

/// <summary>
// Beckmann分布関数
/// </summary>
float D_Beckmann(float m ,float NH)
{
	float c2 = NH * NH;
	float c4 = c2 * c2;
	float m2 = m * m;
	
	return (1.0f / (m2 * c4)) * exp((-1.0f / m2) * (1.0f - c2));
}

/// <summary>
/// V-cavityによるシャドウイング-マスキング関数
/// </summary>
float G2_Vcavity(float NH, float NV, float NL, float VH)
{
	return min(1.0f, min(2.0f * NH * NV / VH, 2.0f * NH * NL / VH));
}

PSOutput main(VSOutput input)
{
	PSOutput output = (PSOutput) 0;

	float3 N = normalize(input.Normal);
	float3 L = normalize(LightPosition  - input.WorldPos);
	float3 V = normalize(CameraPosition - input.WorldPos);
	float3 R = normalize(-V + 2.0f * dot(N, V) * N);
	float3 H = normalize(V + L);
	
	float NH = saturate(dot(N, H));
	float NV = saturate(dot(N, V));
	float NL = saturate(dot(N, L));
	float VH = saturate(dot(V, H));
	
	float4 color   = ColorMap.Sample(WrapSmp, input.TexCoord);
	float3 Kd      = BaseColor * (1.0f - Metallic);
	float3 diffuse = Kd * (1.0f / F_PI);
	
	float3 Ks = BaseColor * Metallic;
	float  a  = Roughness * Roughness;
	float  D  = D_Beckmann(a, NH);
	float  G2 = G2_Vcavity(NH, NV, NL, VH);
	float3 Fr = SchlichFresnel(Ks, NL);

	float3 specular = (D * G2 * Fr) / (4.0f * NV * NL);
	
	output.Color = float4(LightColor * color.rgb * (diffuse + specular) * NL, color.a * Alpha);
	
	return output;
}