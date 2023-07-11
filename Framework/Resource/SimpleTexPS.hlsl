struct VSOutput
{
	float4 Position : SV_POSITION; // 位置座標
	float2 TexCoord : TEXCOORD;    // テクスチャ座標
};

struct PSOutput
{
	float4 Color : SV_TARGET0;
};

SamplerState ColorSmp : register(s0);
Texture2D    ColorMap : register(t0);

PSOutput main(VSOutput input)
{
	PSOutput output = (PSOutput)0;

	output.Color = ColorMap.Sample(ColorSmp, input.TexCoord);

	return output;
}