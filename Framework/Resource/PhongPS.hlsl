static const float F_PI = 3.141596535f;

struct VSOutput
{
	float4 Position : SV_POSITION; // 位置座標
	float2 TexCoord : TEXCOORD;    // テクスチャ座標
	float3 Normal   : NORMAL;      // 法線ベクトル
	float3 WorldPos : WORLD_POS;   // ワールド空間での位置座標
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
}

cbuffer MaterialBuffer : register(b2)
{
	float3 BaseColor : packoffset(c0);   // 基本色
	float  Alpha     : packoffset(c0.w); // 透過度
	float  Metallic  : packoffset(c1);   // 金属度
	float  Shininess : packoffset(c1.y); // 鏡面反射強度
}

SamplerState WrapSmp  : register(s0);
Texture2D    ColorMap : register(t0);

PSOutput main(VSOutput input)
{
	PSOutput output = (PSOutput)0;

	float3 N = normalize(input.Normal);
	float3 L = normalize(LightPosition  - input.WorldPos);
	float3 V = normalize(CameraPosition - input.WorldPos);
	float3 R = normalize(-V + 2.0f * dot(N, V) * N);

	float NL = saturate(dot(N, L));
	float LR = saturate(dot(L, R));

	float4 color    = ColorMap.Sample(WrapSmp, input.TexCoord);
	float3 diffuse  = BaseColor * (1.0f - Metallic) * (1.0f / F_PI);
	float3 specular = BaseColor * Metallic * ((Shininess + 2.0) / (2.0f * F_PI)) * pow(LR, Shininess);

	output.Color = float4(LightColor * color.rgb * (diffuse + specular) * NL, color.a * Alpha);
	
	return output;
}