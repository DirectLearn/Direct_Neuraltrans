cbuffer ConstantBuffer : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
}

struct VS_INPUT
{
	float4 pos : POSITION;
	float2 uv : TEXUV;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXUV;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = mul(input.pos, world);
	//output.pos = mul(output.pos, view);
	//output.pos = mul(output.pos, proj);
	output.uv = input.uv;
	return output;
}

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float4 texel = Texture.Sample(Sampler,input.uv);
	return texel;
}