#include "objectImporter.h"

void OBJ::Draw(
	ID3D11DeviceContext* context,
	SimpleMath::Matrix& world,
	SimpleMath::Matrix& view,
	SimpleMath::Matrix& proj,
	SimpleMath::Matrix& LightCamera
)
{


	context->IASetInputLayout(pVertexLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	context->PSSetConstantBuffers(0, 1, &m_constantBuffer);
	//context->VSSetShader(pVertexShader.Get(), NULL, 0);
	//context->PSSetShader(pPixelShader.Get(), NULL, 0);

	for (DWORD i = 0; i < m_numMesh; i++)
	{
		UINT stride = sizeof(MyVertex);
		UINT offsets = 0;

		context->IASetVertexBuffers(0, 1, meshes[i].m_vertexBuffer.GetAddressOf(), &stride, &offsets);
		ConstantBuffer cb;
		D3D11_MAPPED_SUBRESOURCE pdata;
		//コンスタントバッファの内容を更新
		XMStoreFloat4x4(&cb.world, XMMatrixTranspose(world));
		XMStoreFloat4x4(&cb.view, XMMatrixTranspose(view));
		XMStoreFloat4x4(&cb.proj, XMMatrixTranspose(proj));
		XMStoreFloat4x4(&cb.LightCamera, XMMatrixTranspose(LightCamera));

		for (DWORD j = 0; j < m_numMaterial; j++)
		{
			if (strcmp(meshes[i].materialname.c_str(), m_material[j].name) == 0)
			{
				cb.ambient = XMFLOAT4(m_material[j].Ka.x, m_material[j].Ka.y, m_material[j].Ka.z, m_material[j].Ka.w);
				cb.diffuse = XMFLOAT4(m_material[j].Kd.x, m_material[j].Kd.y, m_material[j].Kd.z, m_material[j].Kd.w);
				cb.specular = XMFLOAT4(m_material[j].Ks.x, m_material[j].Ks.y, m_material[j].Ks.z, m_material[j].Ks.w);

				context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
				memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), sizeof(cb));
				context->Unmap(m_constantBuffer, 0);

				//テクスチャをセット
				context->PSSetShaderResources(
					0,
					1,
					m_material[j].m_srv.GetAddressOf()
				);

				context->PSSetShaderResources(
					1,
					1,
					m_material[j].m_normal_srv.GetAddressOf()
				);
			}


		}
		//インデックスバッファをセット
		context->IASetIndexBuffer(meshes[i].m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		context->DrawIndexed(meshes[i].faceCount * 3, 0, 0);
	}
}


void OBJ::CreateObj(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	const char* objfile_path
)
{
	//<シェーダーの設定>
	ID3DBlob* pCompileVS = NULL;
	ID3DBlob* pCompilePS = NULL;
	
	//深度テクスチャ用のシェーダー
	D3DCompileFromFile(L"Shader/object/OBJDepth.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "DepthVS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	assert(pCompileVS && "pCompileVS is nullptr!");
	(device->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, &pDepthVertexShader));
	//D3DCompileFromFile(L"Shader/object/OBJDepth.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "DepthPS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	//assert(pCompilePS && "pCompilePS is nullptr!");
	//(device->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &pDepthPixelShader));

	//SSAOテクスチャ用のシェーダー
	//D3DCompileFromFile(L"Shader/object/OBJSSAO.hlsl", NULL, NULL, "SSAOVS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	//assert(pCompileVS && "pCompileVS is nullptr!");
	//(device->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, &pSSAOVertexShader));
	//D3DCompileFromFile(L"Shader/object/OBJSSAO.hlsl", NULL, NULL, "SSAOPS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	//assert(pCompilePS && "pCompilePS is nullptr!");
	//(device->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &pSSAOPixelShader));

	//通常描画用のシェーダー
	D3DCompileFromFile(L"Shader/object/OBJ.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	assert(pCompileVS && "pCompileVS is nullptr!");
	(device->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, &pVertexShader));
	D3DCompileFromFile(L"Shader/object/OBJ.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	assert(pCompilePS && "pCompilePS is nullptr!");
	(device->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &pPixelShader));

	//頂点レイアウト
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXUV",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
	};
	(device->CreateInputLayout(layout, ARRAYSIZE(layout), pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), &pVertexLayout));
	pCompileVS->Release();
	pCompilePS->Release();

	//定数バッファの設定
	D3D11_BUFFER_DESC cb;
	cb.ByteWidth = sizeof(ConstantBuffer);
	cb.Usage = D3D11_USAGE_DYNAMIC;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	(device->CreateBuffer(&cb, NULL, &m_constantBuffer));

	//OBJファイル読み込み
	if (FAILED(LoadStaticMesh(objfile_path, device)))
	{
		MessageBox(0, L"メッシュ作成失敗", NULL, MB_OK);
	}

}

HRESULT OBJ::LoadMaterialFromFile(const char* filename, MyMaterial** ppMaterial, const char* filepath)
{
	std::string name = filename;
	std::string path = filepath;
	std::string file = path + name;
	const char* readfile = file.c_str();

	//マテリアルファイルを開いて内容を読み込む
	FILE* fp = NULL;
	fopen_s(&fp, readfile, "rt");
	char key[110] = { 0 };
	XMFLOAT4 v(0, 0, 0, 1);

	//マテリアル数を調べる
	m_numMaterial = 0;
	while (!feof(fp))
	{
		//キーワード読み込み
		fscanf_s(fp, "%s ", key, sizeof(key));
		//マテリアル名
		if (strcmp(key, "newmtl") == 0)
		{
			m_numMaterial++;
		}
	}
	MyMaterial* pMaterial = new MyMaterial[m_numMaterial];

	//本読み込み
	fseek(fp, SEEK_SET, 0);
	INT matCount = -1;

	while (!feof(fp))
	{
		//キーワード読み込み
		fscanf_s(fp, "%s ", key, sizeof(key));
		//マテリアル名
		if (strcmp(key, "newmtl") == 0)
		{
			matCount++;
			fscanf_s(fp, "%s ", key, sizeof(key));
			strcpy_s(pMaterial[matCount].name, key);
		}
		//Ka アンビエント
		if (strcmp(key, "Ka") == 0)
		{
			fscanf_s(fp, "%f %f %f", &v.x, &v.y, &v.z);
			pMaterial[matCount].Ka = v;
		}
		//Kd ディフーズ
		if (strcmp(key, "Kd") == 0)
		{
			fscanf_s(fp, "%f %f %f", &v.x, &v.y, &v.z);
			pMaterial[matCount].Kd = v;
		}
		//Ks スペキュラー
		if (strcmp(key, "Ks") == 0)
		{
			fscanf_s(fp, "%f %f %f", &v.x, &v.y, &v.z);
			pMaterial[matCount].Ks = v;
		}
		//map_Bump
		if (strcmp(key, "map_Bump") == 0)
		{
			fscanf_s(fp, "%s", &pMaterial[matCount].normaltextureName, sizeof(pMaterial[matCount].normaltextureName));
		}
		//map_Kd テクスチャー
		if (strcmp(key, "map_Kd") == 0)
		{
			fscanf_s(fp, "%s", &pMaterial[matCount].textureName, sizeof(pMaterial[matCount].textureName));
		}
	}
	fclose(fp);

	*ppMaterial = pMaterial;

	return S_OK;
}


bool OBJ::LoadTexture(const char* texturename, ID3D11Device* device, const char* filepath,int materialIndex)
{
	std::string name = texturename;
	std::string path = filepath;
	std::string file = path + name;
	const char* readfile = file.c_str();

	//マルチバイト文字列からワイド文字列へ変換
	setlocale(LC_CTYPE, "jpn");
	wchar_t wFilename[400];
	size_t ret;
	mbstowcs_s(&ret, wFilename, readfile, 400);

	//WIC画像を読み込み
	auto image = std::make_unique<ScratchImage>();

	const char* extension = strstr(readfile, ".");

	if (extension == NULL)
		return true;

	if (strcmp(extension, ".tga") == 0 || strcmp(extension, ".TGA") == 0)
	{
		if (FAILED(LoadFromTGAFile(wFilename, &m_info, *image)))
		{
			//失敗
			m_info = {};
			return false;
		}
	}
	else
	{
		if (FAILED(LoadFromWICFile(wFilename, WIC_FLAGS_NONE, &m_info, *image)))
		{
			//失敗
			m_info = {};
			return false;
		}
	}

	//ミップマップの生成
	if (m_info.mipLevels == 1)
	{
		auto mipChain = std::make_unique<ScratchImage>();
		if (SUCCEEDED(GenerateMipMaps(image->GetImages(), image->GetImageCount(), image->GetMetadata(),
			TEX_FILTER_DEFAULT, 0, *mipChain)))
		{
			image = std::move(mipChain);
		}
	}

	//リソースとシェーダーリソースビューを生成
	if (FAILED(CreateShaderResourceView(device, image->GetImages(), image->GetImageCount(), m_info, m_material[materialIndex].m_srv.GetAddressOf())))
	{
		//失敗
		m_info = {};
		return false;
	}

	//成功
	return true;
}

bool OBJ::LoadNormalTexture(const char* texturename,ID3D11Device* device,const char* filepath,int materialIndex)
{
	std::string name = texturename;
	std::string path = filepath;
	std::string file = path + name;
	const char* readfile = file.c_str();

	//マルチバイト文字列からワイド文字列へ変換
	setlocale(LC_CTYPE, "jpn");
	wchar_t wFilename[700];
	size_t ret;
	mbstowcs_s(&ret, wFilename, readfile,700);

	//WIC画像を読み込み
	auto image = std::make_unique<ScratchImage>();

	const char* extension = strstr(readfile, ".");

	if (extension == NULL)
		return true;

	if (strcmp(extension, ".tga") == 0 || strcmp(extension, ".TGA") == 0)
	{
		if (FAILED(LoadFromTGAFile(wFilename, &m_info, *image)))
		{
			//失敗
			m_info = {};
			return false;
		}
	}
	else
	{
		if (FAILED(LoadFromWICFile(wFilename, WIC_FLAGS_NONE, &m_info, *image)))
		{
			//失敗
			m_info = {};
			return false;
		}
	}

	//ミップマップの生成
	if (m_info.mipLevels == 1)
	{
		auto mipChain = std::make_unique<ScratchImage>();
		if (SUCCEEDED(GenerateMipMaps(image->GetImages(), image->GetImageCount(), image->GetMetadata(),
			TEX_FILTER_DEFAULT, 0, *mipChain)))
		{
			image = std::move(mipChain);
		}
	}

	//リソースとシェーダーリソースビューを生成
	if (FAILED(CreateShaderResourceView(device, image->GetImages(), image->GetImageCount(), m_info, m_material[materialIndex].m_normal_srv.GetAddressOf())))
	{
		//失敗
		m_info = {};
		return false;
	}

	//成功
	return true;
}

std::string OBJ::LoadPath(const char* filename)
{
	std::string file = filename;
	file.erase(file.rfind('/') + 1);
	return file;
}

HRESULT OBJ::LoadStaticMesh(const char* filename, ID3D11Device* device)
{
	float x, y, z;
	int v1 = 0, v2 = 0, v3 = 0;
	int vn1 = 0, vn2 = 0, vn3 = 0;
	int vt1 = 0, vt2 = 0, vt3 = 0;
	DWORD vertCount = 0;//読み込みカウンタ
	DWORD vnormalCount = 0;//読み込みカウンタ
	DWORD vuvCount = 0;//読み込みカウンタ
	DWORD faceCount = 0;//読み込みカウンタ
	DWORD meshCount = 0;//読み込みカウンタ
	std::string path = LoadPath(filename);
	const char* filepath = path.c_str();

	char key[200] = { 0 };
	//OBJファイルを開いて内容を読み込む
	FILE* fp = NULL;
	fopen_s(&fp, filename, "rt");

	//事前にメッシュ数を調べる
	while (!feof(fp))
	{
		//キーワード読み込み
		fscanf_s(fp, "%s ", key, sizeof(key));
		//メッシュ
		if (strcmp(key, "o") == 0)
		{
			m_numMesh++;
		}
		//頂点
		if (strcmp(key, "v") == 0)
		{
			vertCount++;
		}
		//法線
		if (strcmp(key, "vn") == 0)
		{
			vnormalCount++;
		}
		//テクスチャ座標
		if (strcmp(key, "vt") == 0)
		{
			vuvCount++;
		}
	}

	//メッシュ数分だけ動的配列を確保する
	meshes.resize(m_numMesh);
	fseek(fp, SEEK_SET, 0);

	//事前にメッシュごとの頂点数、ポリゴン数を調べる
	while (!feof(fp))
	{
		//キーワード読み込み
		fscanf_s(fp, "%s ", key, sizeof(key));
		//マテリアル読み込み
		if (strcmp(key, "mtllib") == 0)
		{
			fscanf_s(fp, "%s ", key, sizeof(key));
			LoadMaterialFromFile(key, &m_material, filepath);
		}
		//メッシュ
		if (strcmp(key, "o") == 0)
		{
			meshCount++;
		}
		//頂点
		if (strcmp(key, "v") == 0)
		{
			meshes[meshCount-1].m_numVertices++;
		}
		//法線
		if (strcmp(key, "vn") == 0)
		{
			meshes[meshCount-1].m_numNormals++;
		}
		//テクスチャ座標
		if (strcmp(key, "vt") == 0)
		{
			meshes[meshCount-1].m_numUVs++;
		}
		//フェイス（ポリゴン）
		if (strcmp(key, "f") == 0)
		{
			meshes[meshCount-1].m_numTriangles++;
		}

	}

	//MyVertex* pVertexBuffer = new MyVertex[m_numTriangles * 3];
	XMFLOAT3* pCoord = new XMFLOAT3[vertCount];
	XMFLOAT3* pNormal = new XMFLOAT3[vnormalCount];
	XMFLOAT2* pUv = new XMFLOAT2[vuvCount];

	//本読み込み
	fseek(fp, SEEK_SET, 0);
	vertCount = 0;
	vnormalCount = 0;
	vuvCount = 0;
	faceCount = 0;
	meshCount = 0;

	while (!feof(fp))
	{
		//キーワード読み込み
		ZeroMemory(key, sizeof(key));
		fscanf_s(fp, "%s ", key, sizeof(key));

		//頂点　読み込み
		if (strcmp(key, "v") == 0)
		{
			fscanf_s(fp, "%f %f %f", &x, &y, &z);
			pCoord[vertCount].x = x;
			pCoord[vertCount].y = y;
			pCoord[vertCount].z = z;
			vertCount++;
		}

		//法線読み込み
		if (strcmp(key, "vn") == 0)
		{
			fscanf_s(fp, "%f %f %f", &x, &y, &z);
			pNormal[vnormalCount].x = x;
			pNormal[vnormalCount].y = y;
			pNormal[vnormalCount].z = z;
			vnormalCount++;
		}

		//テクスチャ座標読み込み
		if (strcmp(key, "vt") == 0)
		{
			fscanf_s(fp, "%f %f", &x, &y);
			pUv[vuvCount].x = x;
			pUv[vuvCount].y = 1 - y;
			vuvCount++;
		}
	}

	
	//マテリアルの数だけインデックスバッファーを作成
	//m_ppindexBuffer = new ID3D11Buffer * [m_numMaterial];

	//フェイス　読み込み　バラバラに収録されている可能性があるので、マテリアルを頼りにつなぎ合わせる
	bool boFlag = false;

	for (int i = 0; i < m_numMesh; i++)
	{
		meshes[i].piFaceBuffer = new int[meshes[i].m_numTriangles * 3];
		meshes[i].pVertexBuffer = new MyVertex[meshes[i].m_numTriangles * 3];
	}

	//int* piFaceBuffer = new int[m_numTriangles * 3];
	faceCount = 0;
	meshCount = 0;
	DWORD subFaceCount = 0;
	DWORD materialIndex = 0;
	fseek(fp, SEEK_SET, 0);

	while (!feof(fp))
	{
		//キーワード読み込み
		ZeroMemory(key, sizeof(key));
		fscanf_s(fp, "%s ", key, sizeof(key));

		//メッシュ
		if (strcmp(key, "o") == 0)
		{
			meshCount++;
			faceCount = 0;
		}

		//フェイス読み込み→頂点インデックスに
		if (strcmp(key, "usemtl") == 0)
		{
			fscanf_s(fp, "%s ", key, sizeof(key));
			for (DWORD i=0; i < m_numMaterial; i++)
			{
				if (strcmp(key, m_material[i].name) == 0)
				{
					boFlag = true;
					materialIndex = i;
					meshes[meshCount - 1].materialname = key;
					break;
				}
				else
				{
					boFlag = false;
				}
			}
		}
		if (strcmp(key, "f") == 0 && boFlag == true)
		{
			if (strlen(m_material[materialIndex].textureName) > 0)//テクスチャありサーフェイス
			{
				fscanf_s(fp, "%d/%d/%d %d/%d/%d %d/%d/%d", &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3);
			}
			else
			{
				fscanf_s(fp, "%d/%d %d/%d %d/%d", &v1, &vn1, &v2, &vn2, &v3, &vn3);
			}
			//インデックスバッファー
			meshes[meshCount-1].piFaceBuffer[meshes[meshCount-1].faceCount * 3] = meshes[meshCount-1].faceCount * 3;
			meshes[meshCount-1].piFaceBuffer[meshes[meshCount-1].faceCount * 3 + 1] = meshes[meshCount-1].faceCount * 3 + 1;
			meshes[meshCount-1].piFaceBuffer[meshes[meshCount-1].faceCount * 3 + 2] = meshes[meshCount-1].faceCount * 3 + 2;
			//頂点構造体に代入
			meshes[meshCount-1].pVertexBuffer[meshes[meshCount-1].faceCount * 3].pos = pCoord[v1 - 1];
			meshes[meshCount-1].pVertexBuffer[meshes[meshCount-1].faceCount * 3].normal = pNormal[vn1 - 1];
			meshes[meshCount-1].pVertexBuffer[meshes[meshCount-1].faceCount * 3].uv = pUv[vt1 - 1];
			meshes[meshCount-1].pVertexBuffer[meshes[meshCount-1].faceCount * 3 + 1].pos = pCoord[v2 - 1];
			meshes[meshCount-1].pVertexBuffer[meshes[meshCount-1].faceCount * 3 + 1].normal = pNormal[vn2 - 1];
			meshes[meshCount-1].pVertexBuffer[meshes[meshCount-1].faceCount * 3 + 1].uv = pUv[vt2 - 1];
			meshes[meshCount-1].pVertexBuffer[meshes[meshCount-1].faceCount * 3 + 2].pos = pCoord[v3 - 1];
			meshes[meshCount-1].pVertexBuffer[meshes[meshCount-1].faceCount * 3 + 2].normal = pNormal[vn3 - 1];
			meshes[meshCount-1].pVertexBuffer[meshes[meshCount-1].faceCount * 3 + 2].uv = pUv[vt3 - 1];


			meshes[meshCount - 1].faceCount++;
			//subFaceCount++;
			//faceCount++;
		}
	}
	//if (subFaceCount == 0)//使用されていないマテリアル対策
	//{
	//	m_ppindexBuffer[i] = NULL;
	//	continue;
	//}


	//テクスチャ読み込み
	for (int i = 0; i < m_numMaterial; i++)
	{
		if (LoadTexture(m_material[i].textureName, device, filepath,i) == false)
		{
			MessageBox(0, L"テクスチャ読み込み失敗", NULL, MB_OK);
		}

		if (LoadNormalTexture(m_material[i].normaltextureName, device, filepath, i) == false)
		{
			MessageBox(0, L"法線テクスチャ読み込み失敗", NULL, MB_OK);
		}
	}

	//メッシュの数だけバッファを作成
	for (int i = 0; i < m_numMesh; i++)
	{
		//インデックスバッファを作成
		const UINT indexbufferSize = sizeof(int) * meshes[i].faceCount * 3;
		D3D11_BUFFER_DESC buffer_desc;
		buffer_desc.ByteWidth = indexbufferSize;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		buffer_desc.CPUAccessFlags = 0;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA sub_resource;
		sub_resource.pSysMem = meshes[i].piFaceBuffer;

		if (FAILED(device->CreateBuffer(&buffer_desc, &sub_resource, meshes[i].m_indexBuffer.GetAddressOf())))
		{
			MessageBox(0, L"インデックスバッファの作成に失敗しました。", NULL, MB_OK);
		}


		//m_material[i].numFace = subFaceCount;

		delete[] meshes[i].piFaceBuffer;
		fclose(fp);

		//バーテックスバッファー作成
		const UINT vertexbufferSize = sizeof(MyVertex) * meshes[i].m_numTriangles * 3;
		D3D11_BUFFER_DESC bd;
		bd.ByteWidth = vertexbufferSize;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA sb;
		sb.pSysMem = meshes[i].pVertexBuffer;

		if (FAILED(device->CreateBuffer(&bd, &sb, meshes[i].m_vertexBuffer.GetAddressOf())))
		{
			MessageBox(0, L"バーテックスバッファーの作成に失敗しました。", NULL, MB_OK);
		}

		delete[] meshes[i].pVertexBuffer;
	}

	//一時的な入れ物は、不要
	delete pCoord;
	delete pNormal;
	delete pUv;
	//delete[] pVertexBuffer;

	


	return S_OK;
}