struct VSInput
{
	float2 Position : POSITION;  // �ʒu���W 
	float2 TexCoord : TEXCOORD;  // �e�N�X�`�����W
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput) 0;
	
	output.Position = float4(input.Position, 0.0f, 1.0f);
	output.TexCoord = input.TexCoord;
	
	return output;
}