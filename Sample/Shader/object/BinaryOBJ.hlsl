cbuffer object : register(b0)
{
	float4x4 gWorld;
	float4x4 gView;
	float4x4 gProj;
	float3 Ambient;
	float3 Diffuse;
	float3 Specular;
};

Texture2D Texture : register(t0);
Texture2D NormalTexture : register(t1);
SamplerState Sampler : register(s0);

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXUV;
};

VS_OUTPUT VS(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXUV)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.pos = mul(pos, gWorld);
	output.pos = mul(output.pos, gView);
	output.pos = mul(output.pos, gProj);

	output.uv = uv;

	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{

	float4 texel = Texture.Sample(Sampler,input.uv);
	if (texel.a <= 0.0)discard;

	return texel;
}