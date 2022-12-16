#pragma once

#include "Source/DirectX/DirectX.h"

class OBJ {
private:
	struct MyVertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};

	//オリジナルマテリアル構造体
	struct MyMaterial
	{
		CHAR name[256];
		XMFLOAT4 Ka; //アンビエント
		XMFLOAT4 Kd; //ディフューズ
		XMFLOAT4 Ks; //スペキュラー
		CHAR normaltextureName[256];//法線テクスチャファイル名
		CHAR textureName[256];	//テクスチャーファイル名
		ComPtr<ID3D11ShaderResourceView> m_srv = nullptr;
		ComPtr<ID3D11ShaderResourceView> m_normal_srv = nullptr;
		DWORD numFace;
	};

	struct MyMesh
	{
		DWORD m_numVertices;
		DWORD m_numNormals;
		DWORD m_numUVs;
		DWORD m_numTriangles;
		DWORD m_numMaterial;
		DWORD faceCount;
		ComPtr<ID3D11Buffer> m_vertexBuffer;
		ComPtr<ID3D11Buffer> m_indexBuffer;
		MyMaterial* m_material;
		MyVertex* pVertexBuffer;
		XMFLOAT3* pCoord;
		XMFLOAT3* pNormal;
		XMFLOAT2* pUv;
		int* piFaceBuffer;
		std::string materialname;
	};

	struct ConstantBuffer
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 proj;
		XMFLOAT4X4 LightCamera;
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT4 specular;
	};

private:
	DWORD m_numVertices;
	DWORD m_numTriangles;
	DWORD m_numMaterial;
	DWORD m_numMesh;
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ID3D11Buffer** m_ppindexBuffer;
	ID3D11Buffer* m_constantBuffer;
	ID3D11RasterizerState* pRasterizerState;
	ComPtr<ID3D11VertexShader> pVertexShader;
	ComPtr<ID3D11PixelShader> pPixelShader;
	ComPtr<ID3D11VertexShader> pDepthVertexShader;
	ComPtr<ID3D11PixelShader> pDepthPixelShader;
	ComPtr<ID3D11VertexShader> pSSAOVertexShader;
	ComPtr<ID3D11PixelShader> pSSAOPixelShader;
	ComPtr<ID3D11InputLayout> pVertexLayout;

	MyMaterial* m_material;
	std::vector<MyMesh> meshes;
	ComPtr<ID3D11ShaderResourceView> m_srv = nullptr;
	//画像情報
	TexMetadata m_info = {};

public:
	void Draw(
		ID3D11DeviceContext* context,
		SimpleMath::Matrix& world,
		SimpleMath::Matrix& view,
		SimpleMath::Matrix& proj,
		SimpleMath::Matrix& LightCamera
	);

	void CreateObj(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		const char* obj_path
	);

	HRESULT LoadStaticMesh(const char* filename, ID3D11Device* device);
	HRESULT LoadMaterialFromFile(const char* filename, MyMaterial** ppMaterial, const char* filepath);
	std::string LoadPath(const char* filepath);
	bool LoadTexture(const char* texturename, ID3D11Device* device, const char* filepath,int materialIndex);
	bool LoadNormalTexture(const char* texturename, ID3D11Device* device, const char* filepath, int materialIndex);

	ID3D11VertexShader* SetGeneralVertexShader() { return pVertexShader.Get(); };
	ID3D11PixelShader* SetGeneralPixelShader() { return pPixelShader.Get(); };
	ID3D11VertexShader* SetDepthVertexShader() { return pDepthVertexShader.Get(); };
	ID3D11PixelShader* SetDepthPixelShader() { return pDepthPixelShader.Get(); };
	ID3D11VertexShader* SetSSAOVertexShader() { return pSSAOVertexShader.Get(); };
	ID3D11PixelShader* SetSSAOPixelShader() { return pSSAOPixelShader.Get(); };
};