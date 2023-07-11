static const float F_PI = 3.141596535f;

struct VSOutput
{
	float4 Position : SV_POSITION; // 位置座標
	float2 TexCoord : TEXCOORD;    // テクスチャ座標
	float3 Normal   : NORMAL;      // 法線ベクトル
	float4 WorldPos : WORLD_POS;   // ワールド空間での位置座標
};

struct PSOutput
{
	float4 Color : SV_TARGET0; // 出力カラー
};

cbuffer LightBuffer : register(b1)
{
	float3 LightPosition : packoffset(c0); // ライト位置
	float3 LightColor    : packoffset(c1); // ライトカラー
}

cbuffer MaterialBuffer : register(b2)
{
	float3 Diffuse : packoffset(c0);   // 拡散反射率
	float  Alpha   : packoffset(c0.w); // 透過度
}

SamplerState WrapSmp  : register(s0);
Texture2D    ColorMap : register(t0);

PSOutput main(VSOutput input)
{
	PSOutput output = (PSOutput)0;

	float3 N = normalize(input.Normal);
	float3 L = normalize(LightPosition - input.WorldPos.xyz);

	float NL = saturate(dot(N, L));

	float4 color   = ColorMap.Sample(WrapSmp, input.TexCoord);
	float3 diffuse = Diffuse * (1.0 / F_PI);

	output.Color = float4(LightColor * color.rgb * diffuse * NL, color.a * Alpha);

	return output;
}