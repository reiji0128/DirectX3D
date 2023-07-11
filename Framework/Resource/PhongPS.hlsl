static const float F_PI = 3.141596535f;

struct VSOutput
{
	float4 Position : SV_POSITION; // �ʒu���W
	float2 TexCoord : TEXCOORD;    // �e�N�X�`�����W
	float3 Normal   : NORMAL;      // �@���x�N�g��
	float3 WorldPos : WORLD_POS;   // ���[���h��Ԃł̈ʒu���W
};

struct PSOutput
{
	float4 Color : SV_TARGET0; // �o�̓J���[
};


cbuffer LightBuffer : register(b1)
{
	float3 LightPosition  : packoffset(c0); // ���C�g�ʒu
	float3 LightColor     : packoffset(c1); // ���C�g�J���[
	float3 CameraPosition : packoffset(c2); // �J�����ʒu
}

cbuffer MaterialBuffer : register(b2)
{
	float3 BaseColor : packoffset(c0);   // ��{�F
	float  Alpha     : packoffset(c0.w); // ���ߓx
	float  Metallic  : packoffset(c1);   // �����x
	float  Shininess : packoffset(c1.y); // ���ʔ��ˋ��x
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