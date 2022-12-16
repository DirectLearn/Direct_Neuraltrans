#pragma once
// <include>
#include "Source/DirectX/DirectX.h"
#include <fbxsdk.h>

// <link FBX SDK library>
// -mt(マルチスレッドデバッグ(MTd))
#pragma comment(lib, "libfbxsdk-md.lib")
#pragma comment(lib, "zlib-md.lib")
#pragma comment(lib, "libxml2-md.lib")
#pragma comment(lib, "libfbxsdk.lib")

#define ALIGN16 _declspec(align(16))

// <FBX Model class>
class FBX_Model
{
private:

	struct Color
	{
		float Red;
		float Green;
		float Blue;
		float Alpha;

		Color(float red, float green, float blue, float alpha)
		{
			Red = red;
			Green = green;
			Blue = blue;
			Alpha = alpha;
		}

		Color()
		{
			Red = Green = Blue = Alpha = 1.0f;
		}

	};

	// <一つの頂点情報を格納する構造体>
	struct VERTEX {
		XMFLOAT3 Pos;
		XMFLOAT2 uv;
		XMFLOAT3 Normal;
		XMFLOAT4 Tangent;
		Color Color;
	};

	// <GPU(シェーダ側)へ送る数値をまとめた構造体>
	struct CONSTANT_BUFFER {
		ALIGN16 XMFLOAT4X4 world;
		ALIGN16 XMFLOAT3X3 wit;
		ALIGN16 XMFLOAT4X4 view;
		ALIGN16 XMFLOAT4X4 proj;
		ALIGN16 XMFLOAT4 ambient;
		ALIGN16 XMFLOAT4 diffuse;
		ALIGN16 XMFLOAT4 specular;
	};

	struct ShapeAnimationVertex
	{
		XMFLOAT3 vPos;
	};

	struct ShapeAnimation
	{
		std::string sName;	//シェイプ名
		float fBlendRate;	//ブレンドレート
		std::vector<ShapeAnimationVertex> vVertices;	//シェイプ考慮時の頂点
	};

	struct Transform_Matrix_Data
	{
		XMFLOAT4X4 GeometryOffset;
		XMFLOAT4X4 ReferenceGlobalInit;
		XMFLOAT4X4 ClusterGlobalInit;
		XMFLOAT4X4 ClusterGlobalCurrent;
		int index;
		double weight;
		XMFLOAT4X4* Defomat;
		XMFLOAT4 Controlvec;
		XMFLOAT4 Shapevec;
	};

	struct MeshData
	{
		std::vector<VERTEX> vertices;
		//ポリゴンの数
		int polygonCount;
		//頂点の数
		int vertexCount;
		//頂点インデックスの数
		int indexCount;
		//頂点
		VERTEX vertex;
		//頂点インデックスの順番
		int* indexBuffer;
		//UVSetの数
		int uvSetCount;
		//UVSetの名前
		std::string* uvSetName;
		//テクスチャパス
		std::vector <std::string> texturePath;
		//マテリアルの名前
		std::string m_MaterialName;

		std::vector<ShapeAnimation> shape;

		//頂点バッファ
		ComPtr<ID3D11Buffer> pVB;
		//インデックスバッファ
		ComPtr<ID3D11Buffer> pIB;
	};

	//FBXの情報
	struct FbxInfo {
		//メッシュ
		std::vector<FbxMesh*> meshes;
		//メッシュの数
		int meshCount;
		//マテリアル
		std::vector<FbxSurfaceMaterial*> material;
		//マテリアルの数
		int materialCount;
		//UVSetの数
		int uvSetCount;
		//UVSetの名前
		std::string* uvSetName;
	};


	struct ObjMaterial
	{
		ObjMaterial()
		{
			for (int i = 0; i < 4; i++)
			{
				Ambient[i] = 1.0f;
				Diffuse[i] = 1.0f;
				Specular[i] = 1.0f;
			}
			//TextureKeyWord = "";
			//TextureName = "";
		}

		void SetAmbient(float r, float g, float b, float factor)
		{
			Ambient[0] = r;
			Ambient[1] = g;
			Ambient[2] = b;
			Ambient[3] = factor;
		}

		void SetDiffuse(float r, float g, float b, float factor)
		{
			Diffuse[0] = r;
			Diffuse[1] = g;
			Diffuse[2] = b;
			Diffuse[3] = factor;
		}

		void SetSpecular(float r, float g, float b, float factor)
		{
			Specular[0] = r;
			Specular[1] = g;
			Specular[2] = b;
			Specular[3] = factor;
		}

		float Ambient[4];
		float Diffuse[4];
		float Specular[4];
		float Alpha;
		//std::string TextureKeyWord;
		//std::string TextureName;
	};



private:
	ComPtr<ID3D11RasterizerState> pRasterizerState;
	ComPtr<ID3D11VertexShader> pVertexShader;
	ComPtr<ID3D11VertexShader> pDepthVertexShader;
	ComPtr<ID3D11VertexShader> pNormalVertexShader;
	ComPtr<ID3D11VertexShader> pSSAOVertexShader;
	ComPtr<ID3D11InputLayout> pVertexLayout;
	ComPtr<ID3D11PixelShader> pPixelShader;
	ComPtr<ID3D11PixelShader> pSSAOPixelShader;
	ComPtr<ID3D11PixelShader> pDepthPixelShader;
	ComPtr<ID3D11PixelShader> pNormalPixelShader;
	
	ComPtr<ID3D11Buffer> m_constantBuffer;
	ComPtr<ID3D11SamplerState> state;
	ComPtr<ID3D11SamplerState> SphereState;

	FbxManager* m_fbxManager = nullptr;
	FbxScene* m_fbxScene = NULL;
	FbxNode* m_meshNode = NULL;
	FbxMesh* m_mesh = NULL;
	ID3D11Buffer* VerBuffer = NULL;
	ID3D11Buffer* IndBuffer = NULL;

	int AnimStackNumber = 2;
	FbxTime FrameTime, timeCount, start, stop;
	FbxInfo m_fbxInfo;
	std::vector<MeshData> m_meshInfo;
	std::map<std::string, ObjMaterial> m_Materials;
	std::map<std::string, ID3D11ShaderResourceView*> m_Textures;
	std::map<std::string, ID3D11ShaderResourceView*> m_NormalTextures;
	std::map<std::string, ID3D11ShaderResourceView*> m_MaterialLinks;
	std::map<std::string, ID3D11ShaderResourceView*> m_NormalLinks;
	std::map<std::string, ID3D11ShaderResourceView*> m_EmissiveTextures;
	std::map<std::string, ID3D11ShaderResourceView*> m_EmissiveLinks;

	TexMetadata m_info = {};



public:
	FBX_Model();
	~FBX_Model();

	// <描画>
	void Draw(
		ID3D11DeviceContext* context,
		XMMATRIX& world,
		XMMATRIX& view,
		XMMATRIX& proj);

	void Draw(
		ID3D11DeviceContext* context,
		SimpleMath::Matrix& world,
		SimpleMath::Matrix& view,
		SimpleMath::Matrix& proj,
		int shapeindex,
		float blendparsent,
		float frame,
		int animationNumber
	);

	void DrawWeapon(
		ID3D11DeviceContext* context,
		SimpleMath::Matrix& world,
		SimpleMath::Matrix& view,
		SimpleMath::Matrix& proj,
		SimpleMath::Matrix& LightCamera,
		SimpleMath::Vector3& eye,
		SimpleMath::Vector3& light,
		float frame,
		int animaitionNumber
	);

	// <モデル作成>
	void Create(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		const char* fbxfile_path);

	//メッシュ作成
	void CreateMesh(ID3D11Device* device);
	//メッシュ取得
	std::vector<FbxMesh*> GetMesh();
	//マテリアル取得
	std::vector<FbxSurfaceMaterial*> GetMaterial();
	//頂点取得
	void GetVertex(int meshIndex);
	//UV座標取得
	void GetUVSetName(int meshIndex);
	//法線取得
	void GetNormal(int meshIndex);
	//接線取得
	void GetTangent(int meshIndex);
	//接線計算
	XMFLOAT4 CalcTangent(XMFLOAT3 v1f, XMFLOAT3 v2f, XMFLOAT3 v3f, XMFLOAT2 uv1f, XMFLOAT2 uv2f, XMFLOAT2 uv3f, XMFLOAT3 normalf);
	//マテリアルの情報を読み取る
	void LoadMaterial(int materialIndex,ID3D11Device* device);
	//ディフューズテクスチャを取得
	bool LoadTexture(FbxFileTexture* material, std::string& keyword, ID3D11Device* device);
	//法線テクスチャを取得
	bool LoadNormalTexture(FbxFileTexture* material, std::string& keyword, ID3D11Device* device);
	//放射テクスチャ取得
	bool LoadEmissiveTexture(FbxFileTexture* material,std::string& keyword, ID3D11Device* device);
	//マテリアルセット
	void SetMaterialName(int meshIndex);
	//各種バッファをセット
	void SetBuffer(ID3D11Device* device,int meshIndex);
	//テクスチャをセット
	void SetTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* shaderResource);
	//法線テクスチャをセット
	void SetNormalTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* shaderResource);
	//放射テクスチャをセット
	void SetEmissiveTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* shaderResource);
	//トゥーンテクスチャをセット
	void Settoon(ID3D11DeviceContext* context, ID3D11ShaderResourceView* shaderResource);
	//シェイプアニメーションを取得
	void GetShapeAnimation(int meshIndex);
	//FbxMatrixをXMMATRIXに変換
	XMMATRIX ConvertMatrix(const FbxMatrix& fbxmat);
	//FbxVectorをXMVECTORに変換
	XMVECTOR ConvertVector(const FbxVector4& fbxvec);
	//頂点シェーダを渡す関数
	ID3D11VertexShader* SetVertexShader() { return pVertexShader.Get(); };
	//ピクセルシェーダーを渡す関数
	ID3D11PixelShader* SetPixelShader() { return pPixelShader.Get(); };
	//コンスタントバッファを渡す関数
	ID3D11Buffer** SetConstantBuffer() { return m_constantBuffer.GetAddressOf(); };
	//ラスタライザステートを渡す関数
	ID3D11RasterizerState* SetRasterState() { return pRasterizerState.Get(); };

	// <破棄>
	void Destroy();

private:
	void FBX_Import(const char* fbxfile_path);

};

