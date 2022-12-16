#pragma once
#include "Source/DirectX/DirectX.h"

#define ALIGN16 _declspec(align(16))

class BinaryOBJ
{
private:
	int m_numMesh;
	int m_numMaterial;

	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};

	struct Material
	{
		CHAR name[256];
		CHAR normaltextureName[256];
		CHAR textureName[256];
		XMFLOAT4 Ka;
		XMFLOAT4 Kd;
		XMFLOAT4 Ks;
	};

	struct Mesh
	{
		DWORD m_numTriangles;
		DWORD faceCount;
		CHAR materialname[200];
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

	ComPtr<ID3D11RasterizerState> pRasterizerState;
	ComPtr<ID3D11InputLayout> pVertexLayout;
	ID3D11Buffer* m_constantBuffer;
	std::vector<ID3D11Buffer*> m_vertexBuffer;
	std::vector<ID3D11Buffer*> m_indexBuffer;
	ComPtr<ID3D11VertexShader> pVertexShader;
	ComPtr<ID3D11PixelShader> pPixelShader;
	std::vector<Mesh> mesh;
	std::vector<Material> material;
	std::vector<std::vector<Vertex>> pVertexBuffer;
	std::vector<std::vector<int>> piFaceBuffer;
	std::vector<ID3D11ShaderResourceView*> m_srv;
	std::vector<ID3D11ShaderResourceView*> m_normal_srv;
	TexMetadata m_info = {};

public:

	void Draw(
		ID3D11DeviceContext* context,
		SimpleMath::Matrix& world,
		SimpleMath::Matrix& view,
		SimpleMath::Matrix& proj
	);

	void CreateBinaryOBJ(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		const char* bin_path
	);

	HRESULT LoadStaticMesh(const char* filename, ID3D11Device* device);
	HRESULT LoadMaterialFromFile(const char* filename, Material** ppMaterial, const char* filepath);
	std::string LoadPath(const char* filepath);
	bool LoadTexture(const char* texturename, ID3D11Device* device, const char* filepath, int materialIndex);
	bool LoadNormalTexture(const char* texturename, ID3D11Device* device, const char* filepath, int materialIndex);

	ID3D11VertexShader* SetVertexShader() { return pVertexShader.Get(); };
	ID3D11PixelShader* SetPixelShader() { return pPixelShader.Get(); };

};