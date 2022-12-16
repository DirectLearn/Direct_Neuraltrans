#include "../MultiRender.hlsli"

// <定数バッファ(CPU側からの値受け取り場)>
cbuffer model : register(b1) {
	float4x4 gWorld;    // <変換行列>
	float3x3 g_wit;
	float4x4 gView;
	float4x4 gProj;
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
};

Texture2D Texture : register(t0);
Texture2D NormalTex : register(t1);
Texture2D EmissiveTex : register(t2);
Texture2D toonTex : register(t3);
Texture2D DepthTex : register(t4);
Texture2D SSAOTex : register(t5);
Texture2D GeneralTex : register(t6);

SamplerState Sampler : register(s0);
SamplerState toonSampler : register(s1);

struct SkinIn
{
	float4 pos : POSITION;
	float2 uv : TEXUV;
	float4 normal : NORMAL;
};


struct PSSkinIn
{
	float4 pos : SV_POSITION;
	float2 uv : TEXUV0;
	float3 normal : NORMAL;
};




// <頂点シェーダ>
PSSkinIn VS(SkinIn Input)
{
	PSSkinIn output;



	float4 worldPos = mul(Input.pos,gWorld);
	float4 viewPos  = mul(worldPos, gView);
	float4 projPos  = mul(viewPos, gProj);

	output.pos = projPos;
	output.uv = Input.uv;

	return output;
}

// <ピクセルシェーダ>
float4 PS(PSSkinIn input) : SV_Target
{
	float4 TexColor = Texture.Sample(Sampler, input.uv);
	if (TexColor.a == 0.0)discard;

	return  TexColor;
}


