struct VSInput
{
	float2 Position : POSITION;  // 位置座標 
	float2 TexCoord : TEXCOORD;  // テクスチャ座標
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