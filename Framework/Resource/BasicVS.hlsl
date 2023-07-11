struct VSInput
{
	float3 Position : POSITION;  // �ʒu���W
	float3 Normal   : NORMAL;    // �@���x�N�g��
	float2 TexCoord : TEXCOORD;  // �e�N�X�`�����W
	float3 Tangent  : TANGENT;   // �ڐ��x�N�g��
};

struct VSOutput
{
	float4   Position        : SV_POSITION;       // �ʒu���W
	float2   TexCoord        : TEXCOORD;          // �e�N�X�`�����W
	float3   WorldPos        : WORLD_POS;         // ���[���h��Ԃ̈ʒu���W
	float3x3 InvTangentBasis : INV_TANGENT_BASIS; // �ڐ���Ԃւ̊��ϊ��s��̋t�s��
};

// �ϊ��s��p�̃R���X�^���g�o�b�t�@�[
cbuffer CbTransform : register(b0)
{
	float4x4 View : packoffset(c0); // �r���[�s��ł�.
	float4x4 Proj : packoffset(c4); // �ˉe�s��ł�.
};

// ���b�V���p�̃R���X�^���g�o�b�t�@�[
cbuffer CbMesh : register(b1)
{
	float4x4 World : packoffset(c0); // ���[���h�s��ł�.
};

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput) 0;

	float4 localPos = float4(input.Position, 1.0f);
	float4 worldPos = mul(World, localPos);
	float4 viewPos  = mul(View, worldPos);
	float4 projPos  = mul(Proj, viewPos);

	output.Position = projPos;
	output.TexCoord = input.TexCoord;
	output.WorldPos = worldPos.xyz;

    // ���x�N�g��
	float3 N = normalize(mul((float3x3) World, input.Normal));
	float3 T = normalize(mul((float3x3) World, input.Tangent));
	float3 B = normalize(cross(N, T));

    // ���ϊ��s��̋t�s��
	output.InvTangentBasis = transpose(float3x3(T, B, N));

	return output;
}