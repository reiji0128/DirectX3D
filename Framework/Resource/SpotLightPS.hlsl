#include "BRDF.hlsli"

#ifndef  MIN_DIST
#define  MIN_DIST (0.01)
#endif

struct VSOutput
{
	float4   Position        : SV_POSITION;       // �ʒu���W
	float2   TexCoord        : TEXCOORD;          // �e�N�X�`�����W
	float3   WorldPos        : WORLD_POS;         // ���[���h��Ԃ̈ʒu���W
	float3x3 InvTangentBasis : INV_TANGENT_BASIS; // �ڐ���Ԃւ̊��ϊ��s��̋t�s��
};

struct PSOutput
{
	float4 Color : SV_TARGET0; // �o�̓J���[
};

cbuffer CbLight : register(b1)
{
	float3 LightPosition     : packoffset(c0);   // ���C�g�̍��W
	float  LightInvSqrRadius : packoffset(c0.w); // ���C�g�̋t2�攼�a
	float3 LightColor        : packoffset(c1);   // ���C�g�̃J���[
	float  LightIntensity    : packoffset(c1.w); // ���C�g�̋��x
	float3 LightForward      : packoffset(c2);   // ���C�g�̑O���x�N�g��
	float  LightAngleScale   : packoffset(c2.w); // ���C�g�̊p�x�����X�P�[��
	float  LightAngleOffset  : packoffset(c3);   // ���C�g�̊p�x�I�t�Z�b�g
	int    LightType         : packoffset(c3.y); // ���C�g�̃^�C�v
}

cbuffer CbCamera : register(b2)
{
	float3 CameraPosition : packoffset(c0); // �J�����ʒu
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
//      ���������Ɋ��炩�Ɍ��������܂�.
//-----------------------------------------------------------------------------
float SmoothDistanceAttenuation(float squaredDistance, // ���C�g�ւ̋�����2��
                                float invSqrAttRadius) // ���C�g���a��2��̋t��
{
	float factor = squaredDistance * invSqrAttRadius;
	float smoothFactor = saturate(1.0f - factor * factor);
	
	return smoothFactor * smoothFactor;
}

//-----------------------------------------------------------------------------
//      �������������߂܂�.
//-----------------------------------------------------------------------------
float CalcDistanceAttenuation(float3 unnormalizedLightVector, // ���C�g�ʒu�ƃI�u�W�F�N�g�ʒu�̍����x�N�g��
                              float  invSqrAttRadius)         // ���C�g���a��2��̋t��
{
	float sqrDist = dot(unnormalizedLightVector, unnormalizedLightVector);
	float attenuation = 1.0f / (max(sqrDist, MIN_DIST * MIN_DIST));
	attenuation *= SmoothDistanceAttenuation(sqrDist, invSqrAttRadius);

	return attenuation;
}

///----------------------------------------------------------------
///   �p�x���������߂܂�
///----------------------------------------------------------------
float CalcAngleAttenuation(float3 unnormalizedLightVector, // �I�u�W�F�N�g�ʒu�ƃ��C�g�ʒu�̍����x�N�g��(�����̓��C�g�̏Ǝ˕���)
                           float3 lightDir,                // ���K���ς݂̃��C�g�̏Ǝ˕����x�N�g��
                           float  lightAngleScale,         // �X�|�b�g���C�g�̊p�x�����X�P�[��
                           float  lightAngleOffset)        // �X�|�b�g���C�g�̊p�x�I�t�Z�b�g
{
	// CPU���Ŏ��̒l���v�Z���Ă���
	// float lightAngleScale = 1.0f / max(0.001f, (cosInner - cosOuter));
    // float lightAngleOffset  = -cosOuter * lightAngleScale;
    // ��L�ɂ�����
    // cosInner�͓��p�̗]��
    // cosOuter�͊O�p�̗]��
	
	float cd = dot(lightDir, unnormalizedLightVector);
	float attenuation = saturate(cd * lightAngleScale + lightAngleOffset);
	
	// �Ȃ߂炩�ɕω�������
	attenuation *= attenuation;
	
	return attenuation;
}

///----------------------------------------------------------------
///   �X�|�b�g���C�g�̕]��
///----------------------------------------------------------------
float3 EvaluateSpotLight(float3 N,                // �@���x�N�g��
                         float3 worldPos,         // ���[���h��Ԃ̃I�u�W�F�N�g�ʒu
                         float3 lightPos,         // ���C�g�̈ʒu
                         float  lightInvRadiusSq, // ���C�g�̋t2�攼�a
                         float3 lightForward,     // ���C�g�̑O���x�N�g��
                         float3 lightColor,       // ���C�g�J���[
                         float  lightAngleScale,  // ���C�g�̌����X�P�[��
                         float  lightAngleOffset) // ���C�g�̊p�x�I�t�Z�b�g
{
	float3 unnormalizedLightVector = lightPos - worldPos;
	float3 L = normalize(unnormalizedLightVector);
	float sqrDist = dot(unnormalizedLightVector, unnormalizedLightVector);
	float att = 1.0f / max(sqrDist, MIN_DIST * MIN_DIST);
	att *= CalcAngleAttenuation(-L, lightForward, lightAngleScale, lightAngleOffset);
	
	return saturate(dot(N, L)) * lightColor * att / F_PI;
}

///----------------------------------------------------------------
///   [Lagarde2014]�ɂ��X�|�b�g���C�g�̕]��
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