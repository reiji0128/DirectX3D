#include "BRDF.hlsli"

#ifndef MIN_DIST
#define MIN_DIST (0.01)
#endif//MIN_DIST

#ifndef OPTIMIZATION
#define OPTIMIZATION (1)
#endif//OPTIMIZATION

struct VSOutput
{
	float4   Position : SV_POSITION;              // 位置座標
	float2   TexCoord : TEXCOORD;                 // テクスチャ座標
	float3   WorldPos : WORLD_POS;                // ワールド空間の位置座標
	float3x3 InvTangentBasis : INV_TANGENT_BASIS; // 接線空間への基底変換行列の逆行列
};

struct PSOutput
{
	float4 Color : SV_TARGET0; // 出力カラー
};

// ライト用のコンスタントバッファー
cbuffer CbLight : register(b1)
{
	float3 LightPosition    : packoffset(c0);   // ライトの座標
	float  LightInvSqRadius : packoffset(c0.w); // ライトの逆2乗半径
	float3 LightColor       : packoffset(c1);   // ライトのカラー
	float  LightIntensity   : packoffset(c1.w); // ライトの強度
}

// カメラ用のコンスタントバッファー
cbuffer CbCamera : register(b2)
{
	float3 CameraPosition : packoffset(c0); // カメラ位置
}

Texture2D    BaseColorMap : register(t0);
SamplerState BaseColorSmp : register(s0);

Texture2D    MetallicMap  : register(t1);
SamplerState MetallicSmp  : register(s1);

Texture2D    RoughnessMap : register(t2);
SamplerState RoughnessSmp : register(s2);

Texture2D    NormalMap    : register(t3);
SamplerState NormalSmp    : register(s3);

/// <summary>距離をもとに滑らかに減衰させる</summary>
/// <param name="squaredDistance">ライトへの距離の2乗</param>
/// <param name="invSqrAttRadius">ライト半径の2乗の逆数</param>
float SmoothDistanceAttenuation(float squaredDistance, float invSqrAttRadius)
{
	float factor = squaredDistance * invSqrAttRadius;
	float smoothFactor = saturate(1.0f - factor * factor);
	
	return smoothFactor * smoothFactor;
}

#ifndef OPTIMIZATION

//-----------------------------------------------------------------------------
//      距離減衰を求めます.
//-----------------------------------------------------------------------------
float GetDistanceAttenuation(float3 unnormalizedLightVector)
{
    float sqrDist = dot(unnormalizedLightVector, unnormalizedLightVector);
    float attenuation = 1.0f / (max(sqrDist, MIN_DIST * MIN_DIST));
    return attenuation;
}

//-----------------------------------------------------------------------------
//      ポイントライトを評価します.
//-----------------------------------------------------------------------------
float3 EvaluatePointLight
(
    float3      N,                      // 法線ベクトル.
    float3      worldPos,               // ワールド空間のオブジェクト位置.
    float3      lightPos,               // ライトの位置.
    float3      lightColor              // ライトカラー.
)
{
    float3 dif = lightPos - worldPos;
    float3 L = normalize(dif);
    float  att = GetDistanceAttenuation(dif);

    return saturate(dot(N, L)) * lightColor * att / (4.0f * F_PI);
}

#else

/// <summary>距離減衰の計算</summary>
/// <param name="unnormalizedLightVector">ライト位置とオブジェクト位置の差分ベクトル</param>
/// <param name="invSqrAttRadius">ライト半径の2乗の逆数</param>
float CalcDistanceAttenuation(float3 unnormalizedLightVector,float invSqrAttRadius)
{
	float sqrDist = dot(unnormalizedLightVector, unnormalizedLightVector);
	float attenuation = 1.0f / (max(sqrDist, MIN_DIST * MIN_DIST));
	
	// 窓関数によって滑らかにゼロになるようにする
	attenuation *= SmoothDistanceAttenuation(sqrDist, invSqrAttRadius);
	
	return attenuation;
}

/// <summary>ポイントライトの評価</summary>
/// <param name="N">法線ベクトル</param>
/// <param name="lightPos">ワールド空間のオブジェクト位置</param>
/// <param name="lightInvRadiusSq">ライト半径の2乗の逆数</param>
/// <param name="lightColor">ライトカラー</param>
float3 EvaluatePointLight(float3 N, float3 worldPos, float3 lightPos, float lightInvRadiusSq, float3 lightColor)
{
	float3 dif = lightPos - worldPos;
	float3 L = normalize(dif);
	float att = CalcDistanceAttenuation(dif, lightInvRadiusSq);
	
	return saturate(dot(N, L)) * lightColor * att / (4.0f * F_PI);
}

#endif

PSOutput main(VSOutput input)
{
	PSOutput output = (PSOutput) 0;
	
	float3 L = normalize(LightPosition  - input.WorldPos);
	float3 V = normalize(CameraPosition - input.WorldPos);
	float3 H = normalize(V + L);
	float3 N = NormalMap.Sample(NormalSmp, input.TexCoord).xyz * 2.0f - 1.0f;
	N = mul(input.InvTangentBasis, N);
	
	float NV = saturate(dot(N, V));
	float NH = saturate(dot(N, H));
	float NL = saturate(dot(N, L));
	
	float3 baseColor = BaseColorMap.Sample(BaseColorSmp, input.TexCoord).rgb;
	float  metallic  = MetallicMap .Sample(MetallicSmp,  input.TexCoord).r;
	float  roughness = RoughnessMap.Sample(RoughnessSmp, input.TexCoord).r;
	
	float3 Kd = baseColor * (1.0f - metallic);
	float3 diffuse = ComputeLambert(Kd);
	
	float3 KS = baseColor * metallic;
	float3 specular = ComputeGGX(KS, roughness, NH, NV, NL);
	
	float3 BRDF = (diffuse + specular);
	
#ifndef OPTIMIZATION
	
	float3 lit = EvaluatePointLight(N, input.WorldPos, LightPosition, LightColor) * LightIntensity;
	
#else	
	
	float3 lit = EvaluatePointLight(N, input.WorldPos, LightPosition,LightInvSqRadius, LightColor) * LightIntensity;
	
#endif	
	
	output.Color.rgb = lit * BRDF;
	output.Color.a   = 1.0f;
	
	return output;
}