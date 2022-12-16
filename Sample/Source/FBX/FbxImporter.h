#pragma once
// <include>
#include "Source/DirectX/DirectX.h"
#include <fbxsdk.h>

// <link FBX SDK library>
// -mt(�}���`�X���b�h�f�o�b�O(MTd))
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

	// <��̒��_�����i�[����\����>
	struct VERTEX {
		XMFLOAT3 Pos;
		XMFLOAT2 uv;
		XMFLOAT3 Normal;
		XMFLOAT4 Tangent;
		Color Color;
	};

	// <GPU(�V�F�[�_��)�֑��鐔�l���܂Ƃ߂��\����>
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
		std::string sName;	//�V�F�C�v��
		float fBlendRate;	//�u�����h���[�g
		std::vector<ShapeAnimationVertex> vVertices;	//�V�F�C�v�l�����̒��_
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
		//�|���S���̐�
		int polygonCount;
		//���_�̐�
		int vertexCount;
		//���_�C���f�b�N�X�̐�
		int indexCount;
		//���_
		VERTEX vertex;
		//���_�C���f�b�N�X�̏���
		int* indexBuffer;
		//UVSet�̐�
		int uvSetCount;
		//UVSet�̖��O
		std::string* uvSetName;
		//�e�N�X�`���p�X
		std::vector <std::string> texturePath;
		//�}�e���A���̖��O
		std::string m_MaterialName;

		std::vector<ShapeAnimation> shape;

		//���_�o�b�t�@
		ComPtr<ID3D11Buffer> pVB;
		//�C���f�b�N�X�o�b�t�@
		ComPtr<ID3D11Buffer> pIB;
	};

	//FBX�̏��
	struct FbxInfo {
		//���b�V��
		std::vector<FbxMesh*> meshes;
		//���b�V���̐�
		int meshCount;
		//�}�e���A��
		std::vector<FbxSurfaceMaterial*> material;
		//�}�e���A���̐�
		int materialCount;
		//UVSet�̐�
		int uvSetCount;
		//UVSet�̖��O
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

	// <�`��>
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

	// <���f���쐬>
	void Create(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		const char* fbxfile_path);

	//���b�V���쐬
	void CreateMesh(ID3D11Device* device);
	//���b�V���擾
	std::vector<FbxMesh*> GetMesh();
	//�}�e���A���擾
	std::vector<FbxSurfaceMaterial*> GetMaterial();
	//���_�擾
	void GetVertex(int meshIndex);
	//UV���W�擾
	void GetUVSetName(int meshIndex);
	//�@���擾
	void GetNormal(int meshIndex);
	//�ڐ��擾
	void GetTangent(int meshIndex);
	//�ڐ��v�Z
	XMFLOAT4 CalcTangent(XMFLOAT3 v1f, XMFLOAT3 v2f, XMFLOAT3 v3f, XMFLOAT2 uv1f, XMFLOAT2 uv2f, XMFLOAT2 uv3f, XMFLOAT3 normalf);
	//�}�e���A���̏���ǂݎ��
	void LoadMaterial(int materialIndex,ID3D11Device* device);
	//�f�B�t���[�Y�e�N�X�`�����擾
	bool LoadTexture(FbxFileTexture* material, std::string& keyword, ID3D11Device* device);
	//�@���e�N�X�`�����擾
	bool LoadNormalTexture(FbxFileTexture* material, std::string& keyword, ID3D11Device* device);
	//���˃e�N�X�`���擾
	bool LoadEmissiveTexture(FbxFileTexture* material,std::string& keyword, ID3D11Device* device);
	//�}�e���A���Z�b�g
	void SetMaterialName(int meshIndex);
	//�e��o�b�t�@���Z�b�g
	void SetBuffer(ID3D11Device* device,int meshIndex);
	//�e�N�X�`�����Z�b�g
	void SetTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* shaderResource);
	//�@���e�N�X�`�����Z�b�g
	void SetNormalTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* shaderResource);
	//���˃e�N�X�`�����Z�b�g
	void SetEmissiveTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* shaderResource);
	//�g�D�[���e�N�X�`�����Z�b�g
	void Settoon(ID3D11DeviceContext* context, ID3D11ShaderResourceView* shaderResource);
	//�V�F�C�v�A�j���[�V�������擾
	void GetShapeAnimation(int meshIndex);
	//FbxMatrix��XMMATRIX�ɕϊ�
	XMMATRIX ConvertMatrix(const FbxMatrix& fbxmat);
	//FbxVector��XMVECTOR�ɕϊ�
	XMVECTOR ConvertVector(const FbxVector4& fbxvec);
	//���_�V�F�[�_��n���֐�
	ID3D11VertexShader* SetVertexShader() { return pVertexShader.Get(); };
	//�s�N�Z���V�F�[�_�[��n���֐�
	ID3D11PixelShader* SetPixelShader() { return pPixelShader.Get(); };
	//�R���X�^���g�o�b�t�@��n���֐�
	ID3D11Buffer** SetConstantBuffer() { return m_constantBuffer.GetAddressOf(); };
	//���X�^���C�U�X�e�[�g��n���֐�
	ID3D11RasterizerState* SetRasterState() { return pRasterizerState.Get(); };

	// <�j��>
	void Destroy();

private:
	void FBX_Import(const char* fbxfile_path);

};

