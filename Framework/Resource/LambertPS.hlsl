static const float F_PI = 3.141596535f;

struct VSOutput
{
	float4 Position : SV_POSITION; // �ʒu���W
	float2 TexCoord : TEXCOORD;    // �e�N�X�`�����W
	float3 Normal   : NORMAL;      // �@���x�N�g��
	float4 WorldPos : WORLD_POS;   // ���[���h��Ԃł̈ʒu���W
};

struct PSOutput
{
	float4 Color : SV_TARGET0; // �o�̓J���[
};

cbuffer LightBuffer : register(b1)
{
	float3 LightPosition : packoffset(c0); // ���C�g�ʒu
	float3 LightColor    : packoffset(c1); // ���C�g�J���[
}

cbuffer MaterialBuffer : register(b2)
{
	float3 Diffuse : packoffset(c0);   // �g�U���˗�
	float  Alpha   : packoffset(c0.w); // ���ߓx
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