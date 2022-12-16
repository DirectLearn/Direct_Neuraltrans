#pragma once
#include "Source/DirectX/DirectX.h"

#define SIZE 100
#define TIME_STEP 0.005
#define DENSITY_COEF 0.02
#define VISCOSITY_COEFF 0.001

#define SwapOBJ(a,b) {OBJECT tmp;memcpy(&tmp,&a,sizeof(OBJECT));memcpy(&a,&b,sizeof(OBJECT));memcpy(&b,&tmp,sizeof(OBJECT));}


//必要なオブジェクト
struct OBJECT
{
	ID3D11InputLayout* pVertexLayout;
	ID3D11VertexShader* pVertexShader;
	ID3D11PixelShader* pPixelShader;
	ID3D11ComputeShader* pComputeShader;
	ID3D11Buffer* pCBuffer;
	ID3D11Buffer* pVBuffer;
	ID3D11Buffer* pIBuffer;
	ID3D11Texture2D* pTex2D;
	ID3D11Texture2D* pDSTex2D;
	ID3D11Texture3D* pTex3D;
	ID3D11Buffer* pStredBuf;
	ID3D11RenderTargetView* pRTV;
	ID3D11ShaderResourceView* pSRV;
	ID3D11DepthStencilView* pDSV;
	ID3D11UnorderedAccessView* pUAV;
};


//ストラクチャードバッファの要素
struct SBUFFER_ELEMENT
{
	float f;
	float f0;
};


//コンスタントバッファ
struct CS_BUFFER
{
	XMFLOAT4 DensitySphere;
	XMFLOAT4 DensitySource;
	XMFLOAT4 VelocitySphere;
	XMFLOAT4 VelocitySource;
};


struct CBUFFER_F
{
	XMFLOAT4X4 mWVP;
	XMFLOAT4X4 mW;
};

struct VERTEX_F
{
	XMFLOAT3 Pos;
};


class Fire
{

private:
	OBJECT m_D[2];
	OBJECT m_V[2];
	OBJECT m_V_s;
	OBJECT m_V_bs;
	OBJECT m_Prs;
	OBJECT m_Div;

	OBJECT m_D_Tex;

	OBJECT m_AddSourceShader;
	OBJECT m_AdvectDensityShader;
	OBJECT m_AdvectVelocityShader;
	OBJECT m_AdvectBackShader;
	OBJECT m_MacCormackShader;
	OBJECT m_Project1Shader;
	OBJECT m_Project2Shader;
	OBJECT m_Project3Shader;
	OBJECT m_Project4Shader;
	OBJECT m_Boundary1Shader;
	OBJECT m_Boundary2Shader;

	CS_BUFFER m_Source;

	bool m_Add;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	ID3D11Buffer* m_pConstantBuffer;
	ID3D11SamplerState* m_pSamLinear;
	ID3D11SamplerState* m_pSampleLinear;
	ID3D11RasterizerState* m_pFrontFace;
	ID3D11RasterizerState* m_pBackFace;
	ID3D11BlendState* m_pBlendState;

	OBJECT m_BackBuffer;	//通常のレンダーターゲット
	OBJECT m_TextureA;		//テクスチャーA
	OBJECT m_TextureB;		//テクスチャーB
	OBJECT m_Cube;			//ボックスメッシュ
	OBJECT m_ScreenPoly;	//ボリュームレンダー
	ComPtr<ID3D11Texture3D> m_pTexture3D;
	ComPtr<ID3D11ShaderResourceView> m_pTexture3D_SRV;
	float m_DColor[SIZE][SIZE][SIZE];

public:
	void Init(ID3D11Device*, ID3D11DeviceContext*, DWORD, DWORD, ID3D11RenderTargetView*, ID3D11DepthStencilView*);

	void AddDensitySource(XMFLOAT4&, XMFLOAT4&);
	void AddVelocitySource(XMFLOAT4&, XMFLOAT4&);
	void Solve();
	void Unbind();
	HRESULT MakeShader(ID3D11Device*, LPCWSTR, LPCSTR, LPCSTR, void**, ID3DBlob**);
	HRESULT CreateEmpty3DTexture(int, ID3D11Device*, ID3D11Texture3D**, ID3D11ShaderResourceView**);
	void DrawFluid(SimpleMath::Matrix&, SimpleMath::Matrix&);
};