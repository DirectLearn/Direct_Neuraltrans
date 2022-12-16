#include "binaryobject.h"


void BinaryOBJ::Draw(
	ID3D11DeviceContext* context,
	SimpleMath::Matrix& world,
	SimpleMath::Matrix& view,
	SimpleMath::Matrix& proj
)
{
	context->VSSetShader(pVertexShader.Get(), NULL, 0);
    context->PSSetShader(pPixelShader.Get(), NULL, 0);
	context->IASetInputLayout(pVertexLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	context->PSSetConstantBuffers(0, 1, &m_constantBuffer);
	context->RSSetState(pRasterizerState.Get());

	for (DWORD i = 0; i < m_numMesh; i++)
	{
		UINT stride = sizeof(Vertex);
		UINT offsets = 0;

		context->IASetVertexBuffers(0, 1, &m_vertexBuffer[i], &stride, &offsets);
		ConstantBuffer cb;
		D3D11_MAPPED_SUBRESOURCE pdata;
		//コンスタントバッファの内容を更新
		XMStoreFloat4x4(&cb.world, XMMatrixTranspose(world));
		XMStoreFloat4x4(&cb.view, XMMatrixTranspose(view));
		XMStoreFloat4x4(&cb.proj, XMMatrixTranspose(proj));

		for (DWORD j = 0; j < m_numMaterial; j++)
		{
			if (strcmp(mesh[i].materialname, material[j].name) == 0)
			{
				cb.ambient = XMFLOAT4(material[j].Ka.x, material[j].Ka.y, material[j].Ka.z, material[j].Ka.w);
				cb.diffuse = XMFLOAT4(material[j].Kd.x, material[j].Kd.y, material[j].Kd.z, material[j].Kd.w);
				cb.specular = XMFLOAT4(material[j].Ks.x, material[j].Ks.y, material[j].Ks.z, material[j].Ks.w);

				if (SUCCEEDED(context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata)))
				{
					memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), sizeof(cb));
					context->Unmap(m_constantBuffer, 0);
				}


				//テクスチャをセット
				context->PSSetShaderResources(
					0,
					1,
					&m_srv[j]
				);

				context->PSSetShaderResources(
					1,
					1,
					&m_normal_srv[j]
				);
			}
		}
		//インデックスバッファをセット
		context->IASetIndexBuffer(m_indexBuffer[i], DXGI_FORMAT_R32_UINT, 0);
		context->DrawIndexed(mesh[i].faceCount * 3, 0, 0);
	}
}


void BinaryOBJ::CreateBinaryOBJ(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	const char* objfile_path
)
{
	//<シェーダーの設定>
	ID3DBlob* pCompileVS = NULL;
	ID3DBlob* pCompilePS = NULL;

	//頂点シェーダー作成
	D3DCompileFromFile(L"Shader/object/BinaryOBJ.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	assert(pCompileVS && "pCompileVS is nullptr!");
	(device->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, &pVertexShader));
	D3DCompileFromFile(L"Shader/object/BinaryOBJ.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	assert(pCompilePS && "pCompilePS is nullptr!");
	(device->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &pPixelShader));

	//頂点レイアウト
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXUV",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0}
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

	// <ラスタライザの設定>
	D3D11_RASTERIZER_DESC rdc = {};
	rdc.CullMode = D3D11_CULL_NONE;
	rdc.FillMode = D3D11_FILL_SOLID;
	//rdc.FrontCounterClockwise = TRUE;
	//rdc.MultisampleEnable = TRUE;
	rdc.DepthClipEnable = true;
	(device->CreateRasterizerState(&rdc, &pRasterizerState));

	//バイナリOBJファイル読み込み
	if (FAILED(LoadStaticMesh(objfile_path, device)))
	{
		MessageBox(0, L"メッシュ作成失敗", NULL, MB_OK);
	}
}

bool BinaryOBJ::LoadTexture(const char* texturename, ID3D11Device* device, const char* filepath, int materialIndex)
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
	if (FAILED(CreateShaderResourceView(device, image->GetImages(), image->GetImageCount(), m_info, &m_srv[materialIndex])))
	{
		//失敗
		m_info = {};
		return false;
	}

	//成功
	return true;
}

bool BinaryOBJ::LoadNormalTexture(const char* texturename, ID3D11Device* device, const char* filepath, int materialIndex)
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
	if (FAILED(CreateShaderResourceView(device, image->GetImages(), image->GetImageCount(), m_info, &m_normal_srv[materialIndex])))
	{
		//失敗
		m_info = {};
		return false;
	}

	//成功
	return true;
}

std::string BinaryOBJ::LoadPath(const char* filename)
{
	std::string file = filename;
	file.erase(file.rfind('/') + 1);
	return file;
}

HRESULT BinaryOBJ::LoadStaticMesh(const char* filename, ID3D11Device* device)
{
	//バイナリファイルから頂点・マテリアル情報を読み込む

	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (!in)
	{
		MessageBox(0, L"バイナリファイルが読み込めません", NULL, MB_OK);
	}

	in.read(reinterpret_cast<char*>(&m_numMesh), sizeof(m_numMesh));
	in.read(reinterpret_cast<char*>(&m_numMaterial), sizeof(m_numMaterial));

	mesh.resize(m_numMesh);
	material.resize(m_numMaterial);

	in.read(reinterpret_cast<char*>(&mesh[0]), mesh.size() * sizeof(Mesh));

	piFaceBuffer.resize(m_numMesh);
	pVertexBuffer.resize(m_numMesh);
	for (int i = 0; i < m_numMesh; i++)
	{
		piFaceBuffer[i].resize(mesh[i].m_numTriangles * 3);
		pVertexBuffer[i].resize(mesh[i].m_numTriangles * 3);
		in.read(reinterpret_cast<char*>(&piFaceBuffer[i][0]), piFaceBuffer[i].size() * sizeof(int));
		in.read(reinterpret_cast<char*>(&pVertexBuffer[i][0]), pVertexBuffer[i].size() * sizeof(Vertex));
	}

	in.read(reinterpret_cast<char*>(&material[0]), material.size() * sizeof(Material));
	in.close();

	//テクスチャが格納されているパスを作る
	std::string path = LoadPath(filename);
	const char* filepath = path.c_str();

	m_srv.resize(m_numMaterial);
	m_normal_srv.resize(m_numMaterial);

	//テクスチャ読み込み
	for (int i = 0; i < m_numMaterial; i++)
	{
		if (LoadTexture(material[i].textureName, device, filepath, i) == false)
		{
			MessageBox(0, L"テクスチャ読み込み失敗", NULL, MB_OK);
		}

		if (LoadNormalTexture(material[i].normaltextureName, device, filepath, i) == false)
		{
			MessageBox(0, L"法線テクスチャ読み込み失敗", NULL, MB_OK);
		}
	}


	m_indexBuffer.resize(m_numMesh);
	m_vertexBuffer.resize(m_numMesh);

	//メッシュの数だけバッファを作成
	for (int i = 0; i < m_numMesh; i++)
	{
		//インデックスバッファを作成
		const UINT indexbufferSize = sizeof(int) * mesh[i].faceCount * 3;
		D3D11_BUFFER_DESC buffer_desc;
		buffer_desc.ByteWidth = indexbufferSize;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		buffer_desc.CPUAccessFlags = 0;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA sub_resource;
		sub_resource.pSysMem = piFaceBuffer[i].data();

		if (FAILED(device->CreateBuffer(&buffer_desc, &sub_resource, &m_indexBuffer[i])))
		{
			MessageBox(0, L"インデックスバッファの作成に失敗しました", NULL, MB_OK);
		}

		//バーテックスバッファ作成
		const UINT vertexbufferSize = sizeof(Vertex) * mesh[i].m_numTriangles * 3;
		D3D11_BUFFER_DESC bd;
		bd.ByteWidth = vertexbufferSize;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA sb;
		sb.pSysMem = pVertexBuffer[i].data();

		if (FAILED(device->CreateBuffer(&bd, &sb, &m_vertexBuffer[i])))
		{
			MessageBox(0, L"バーテックスバッファの作成に失敗しました", NULL, MB_OK);
		}
	}

	return S_OK;
}