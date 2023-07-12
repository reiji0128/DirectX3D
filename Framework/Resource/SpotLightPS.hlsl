#include "BRDF.hlsli"

#ifndef  MIN_DIST
#define  MIN_DIST (0.01)
#endif

struct VSOutput
{
	float4   Position        : SV_POSITION;       // 位置座標
	float2   TexCoord        : TEXCOORD;          // テクスチャ座標
	float3   WorldPos        : WORLD_POS;         // ワールド空間の位置座標
	float3x3 InvTangentBasis : INV_TANGENT_BASIS; // 接線空間への基底変換行列の逆行列
};

struct PSOutput
{
	float4 Color : SV_TARGET0; // 出力カラー
};

cbuffer CbLight : register(b1)
{
	float3 LightPosition     : packoffset(c0);   // ライトの座標
	float  LightInvSqrRadius : packoffset(c0.w); // ライトの逆2乗半径
	float3 LightColor        : packoffset(c1);   // ライトのカラー
	float  LightIntensity    : packoffset(c1.w); // ライトの強度
	float3 LightForward      : packoffset(c2);   // ライトの前方ベクトル
	float  LightAngleScale   : packoffset(c2.w); // ライトの角度減衰スケール
	float  LightAngleOffset  : packoffset(c3);   // ライトの角度オフセット
	int    LightType         : packoffset(c3.y); // ライトのタイプ
}

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

//-----------------------------------------------------------------------------
//      距離を元に滑らかに減衰させます.
//-----------------------------------------------------------------------------
float SmoothDistanceAttenuation(float squaredDistance, // ライトへの距離の2乗
                                float invSqrAttRadius) // ライト半径の2乗の逆数
{
	float factor = squaredDistance * invSqrAttRadius;
	float smoothFactor = saturate(1.0f - factor * factor);
	
	return smoothFactor * smoothFactor;
}

//-----------------------------------------------------------------------------
//      距離減衰を求めます.
//-----------------------------------------------------------------------------
float CalcDistanceAttenuation(float3 unnormalizedLightVector, // ライト位置とオブジェクト位置の差分ベクトル
                              float  invSqrAttRadius)         // ライト半径の2乗の逆数
{
	float sqrDist = dot(unnormalizedLightVector, unnormalizedLightVector);
	float attenuation = 1.0f / (max(sqrDist, MIN_DIST * MIN_DIST));
	attenuation *= SmoothDistanceAttenuation(sqrDist, invSqrAttRadius);

	return attenuation;
}

///----------------------------------------------------------------
///   角度減衰を求めます
///----------------------------------------------------------------
float CalcAngleAttenuation(float3 unnormalizedLightVector, // オブジェクト位置とライト位置の差分ベクトル(向きはライトの照射方向)
                           float3 lightDir,                // 正規化済みのライトの照射方向ベクトル
                           float  lightAngleScale,         // スポットライトの角度減衰スケール
                           float  lightAngleOffset)        // スポットライトの角度オフセット
{
	// CPU側で次の値を計算しておく
	// float lightAngleScale = 1.0f / max(0.001f, (cosInner - cosOuter));
    // float lightAngleOffset  = -cosOuter * lightAngleScale;
    // 上記における
    // cosInnerは内角の余弦
    // cosOuterは外角の余弦
	
	float cd = dot(lightDir, unnormalizedLightVector);
	float attenuation = saturate(cd * lightAngleScale + lightAngleOffset);
	
	// なめらかに変化させる
	attenuation *= attenuation;
	
	return attenuation;
}

///----------------------------------------------------------------
///   スポットライトの評価
///----------------------------------------------------------------
float3 EvaluateSpotLight(float3 N,                // 法線ベクトル
                         float3 worldPos,         // ワールド空間のオブジェクト位置
                         float3 lightPos,         // ライトの位置
                         float  lightInvRadiusSq, // ライトの逆2乗半径
                         float3 lightForward,     // ライトの前方ベクトル
                         float3 lightColor,       // ライトカラー
                         float  lightAngleScale,  // ライトの減衰スケール
                         float  lightAngleOffset) // ライトの角度オフセット
{
	float3 unnormalizedLightVector = lightPos - worldPos;
	float3 L = normalize(unnormalizedLightVector);
	float sqrDist = dot(unnormalizedLightVector, unnormalizedLightVector);
	float att = 1.0f / max(sqrDist, MIN_DIST * MIN_DIST);
	att *= CalcAngleAttenuation(-L, lightForward, lightAngleScale, lightAngleOffset);
	
	return saturate(dot(N, L)) * lightColor * att / F_PI;
}

///----------------------------------------------------------------
///   [Lagarde2014]によるスポットライトの評価
///----------------------------------------------------------------
float3 EvaluateSpotLightLagarde(float3 N,
                                float3 worldPos,
                                float3 lightPos,
                                float  lightInvRadiusSq,
                                float3 lightForward,
                                float3 lightColor,
                                float  lightAngleScale,
                                float  lightAngleOffset)
{
	float3 unnormalizedLightVector = lightPos - worldPos;
	float3 L = normalize(unnormalizedLightVector);
	float att = 1.0f;
	att *= CalcDistanceAttenuation(unnormalizedLightVector, lightInvRadiusSq);
	att *= CalcAngleAttenuation(L, lightForward, lightAngleScale, lightAngleOffset);
	
	return saturate(dot(N, L)) * lightColor * att / F_PI;
}

PSOutput main(VSOutput input)
{
	PSOutput output = (PSOutput) 0;

	float3 L = normalize(LightPosition - input.WorldPos);
	float3 V = normalize(CameraPosition - input.WorldPos);
	float3 H = normalize(V + L);
	float3 N = NormalMap.Sample(NormalSmp, input.TexCoord).xyz * 2.0f - 1.0f;
	N = mul(input.InvTangentBasis, N);

	float NV = saturate(dot(N, V));
	float NH = saturate(dot(N, H));
	float NL = saturate(dot(N, L));

	float3 baseColor = BaseColorMap.Sample(BaseColorSmp, input.TexCoord).rgb;
	float  metallic  = MetallicMap.Sample(MetallicSmp, input.TexCoord).r;
	float  roughness = RoughnessMap.Sample(RoughnessSmp, input.TexCoord).r;

	float3 Kd = baseColor * (1.0f - metallic);
	float3 diffuse = ComputeLambert(Kd);

	float3 Ks = baseColor * metallic;
	float3 specular = ComputeGGX(Ks, roughness, NH, NV, NL);

	float3 BRDF = (diffuse + specular);
	float3 lit = 0;
    
	if (LightType == 0)
	{
		lit = EvaluateSpotLight(
            N,
            input.WorldPos,
            LightPosition,
            LightInvSqrRadius,
            LightForward,
            LightColor,
            LightAngleScale,
            LightAngleOffset) * LightIntensity;
	}
	else
	{
		lit = EvaluateSpotLightLagarde(
            N,
            input.WorldPos,
            LightPosition,
            LightInvSqrRadius,
            LightForward,
            LightColor,
            LightAngleScale,
            LightAngleOffset) * LightIntensity;
	}

	output.Color.rgb = lit * BRDF;
	output.Color.a = 1.0f;

	return output;
}