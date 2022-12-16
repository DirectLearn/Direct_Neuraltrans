#include "Fire.h"

ID3D11UnorderedAccessView* g_ppUAV_NULL[1] = { NULL };
ID3D11ShaderResourceView* g_ppSRV_NULL[1] = { NULL };


//hlsl�t�@�C����ǂݍ��݃V�F�[�_�[���쐬����
HRESULT Fire::MakeShader(ID3D11Device* pDevice, LPCWSTR szFileName, LPCSTR szFuncName, LPCSTR szProfileName, void** ppShader, ID3DBlob** ppBlob)
{
	ID3DBlob *pErrors = NULL;
	if (FAILED(D3DCompileFromFile(szFileName, NULL, NULL, szFuncName, szProfileName, NULL, 0, ppBlob, &pErrors)))
	{
		char* p = (char*)pErrors->GetBufferPointer();
		MessageBoxA(0, p, 0, MB_OK);
		return E_FAIL;
	}
	//pErrors->Release();
	char szProfile[3] = { 0 };
	memcpy(szProfile, szProfileName, 2);
	if (strcmp(szProfile, "vs") == 0)
	{
		if (FAILED(pDevice->CreateVertexShader((*ppBlob)->GetBufferPointer(), (*ppBlob)->GetBufferSize(), NULL, (ID3D11VertexShader**)ppShader))) return E_FAIL;
	}
	if (strcmp(szProfile, "ps") == 0)
	{
		if (FAILED(pDevice->CreatePixelShader((*ppBlob)->GetBufferPointer(), (*ppBlob)->GetBufferSize(), NULL, (ID3D11PixelShader**)ppShader))) return E_FAIL;
	}
	if (strcmp(szProfile, "gs") == 0)
	{
		if (FAILED(pDevice->CreateGeometryShader((*ppBlob)->GetBufferPointer(), (*ppBlob)->GetBufferSize(), NULL, (ID3D11GeometryShader**)ppShader))) return E_FAIL;
	}
	if (strcmp(szProfile, "hs") == 0)
	{
		if (FAILED(pDevice->CreateHullShader((*ppBlob)->GetBufferPointer(), (*ppBlob)->GetBufferSize(), NULL, (ID3D11HullShader**)ppShader))) return E_FAIL;
	}
	if (strcmp(szProfile, "ds") == 0)
	{
		if (FAILED(pDevice->CreateDomainShader((*ppBlob)->GetBufferPointer(), (*ppBlob)->GetBufferSize(), NULL, (ID3D11DomainShader**)ppShader))) return E_FAIL;
	}
	if (strcmp(szProfile, "cs") == 0)
	{
		if (FAILED(pDevice->CreateComputeShader((*ppBlob)->GetBufferPointer(), (*ppBlob)->GetBufferSize(), NULL, (ID3D11ComputeShader**)ppShader))) return E_FAIL;
	}
	return S_OK;
}


HRESULT Fire::CreateEmpty3DTexture(int Size, ID3D11Device* pDevice, ID3D11Texture3D** ppTexture, ID3D11ShaderResourceView** ppSRV)
{
	D3D11_TEXTURE3D_DESC descTex;
	ZeroMemory(&descTex, sizeof(descTex));
	descTex.Height = Size;
	descTex.Width = Size;
	descTex.Depth = Size;
	descTex.MipLevels = 1;
	descTex.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	descTex.Usage = D3D11_USAGE_DEFAULT;
	descTex.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	
	pDevice->CreateTexture3D(&descTex, NULL, ppTexture);
	pDevice->CreateShaderResourceView(*ppTexture, NULL, ppSRV);

	return S_OK;
}


void Fire::Init(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, DWORD Width, DWORD Height,
	ID3D11RenderTargetView* pBackRTV, ID3D11DepthStencilView* pBackDSV)
{
	D3D11_RENDER_TARGET_VIEW_DESC rdesc;
	D3D11_TEXTURE2D_DESC desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC sdesc;
	D3D11_BUFFER_DESC bdesc;
	D3D11_SUBRESOURCE_DATA InitData;
	D3D11_UNORDERED_ACCESS_VIEW_DESC udesc;

	m_pDevice =pDevice;
	m_pDeviceContext =pContext;

	m_BackBuffer.pRTV =pBackRTV;
	m_BackBuffer.pDSV =pBackDSV;

	//3D�e�N�X�`���[
	CreateEmpty3DTexture(SIZE + 2, m_pDevice, &m_D_Tex.pTex3D, &m_D_Tex.pSRV);
	for (int i = 0; i < 2; i++)CreateEmpty3DTexture(SIZE + 2, m_pDevice, &m_D[i].pTex3D, &m_D[i].pSRV);
	for (int i = 0; i < 2; i++)CreateEmpty3DTexture(SIZE + 2, m_pDevice, &m_V[i].pTex3D, &m_V[i].pSRV);
	CreateEmpty3DTexture(SIZE + 2, m_pDevice, &m_V_s.pTex3D, &m_V_s.pSRV);
	CreateEmpty3DTexture(SIZE + 2, m_pDevice, &m_V_bs.pTex3D, &m_V_bs.pSRV);

	//�X�g���N�`���[�h�o�b�t�@
	ZeroMemory(&bdesc, sizeof(bdesc));
	bdesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	bdesc.ByteWidth = sizeof(SBUFFER_ELEMENT) * (SIZE + 2) * (SIZE + 2) * (SIZE + 2);
	bdesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bdesc.StructureByteStride = sizeof(SBUFFER_ELEMENT);
	m_pDevice->CreateBuffer(&bdesc, NULL, &m_Prs.pStredBuf);
	m_pDevice->CreateBuffer(&bdesc, NULL, &m_Div.pStredBuf);

	//UAV
	ZeroMemory(&udesc, sizeof(udesc));
	udesc.Format = DXGI_FORMAT_UNKNOWN;
	udesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	udesc.Texture2D.MipSlice = 0;
	udesc.Buffer.NumElements = (SIZE + 2) * (SIZE + 2) * (SIZE + 2);
	m_pDevice->CreateUnorderedAccessView(m_Prs.pStredBuf, &udesc, &m_Prs.pUAV);
	m_pDevice->CreateUnorderedAccessView(m_Div.pStredBuf, &udesc, &m_Div.pUAV);

	//UAV(3D�e�N�X�`���[)
	ZeroMemory(&udesc, sizeof(udesc));
	udesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	udesc.Texture3D.FirstWSlice = 0;
	udesc.Texture3D.WSize = SIZE;
	udesc.Texture3D.MipSlice = 0;
	udesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;

	m_pDevice->CreateUnorderedAccessView(m_D_Tex.pTex3D, &udesc, &m_D_Tex.pUAV);
	for (int i = 0; i < 2; i++)m_pDevice->CreateUnorderedAccessView(m_D[i].pTex3D, &udesc, &m_D[i].pUAV);
	for (int i = 0; i < 2; i++)m_pDevice->CreateUnorderedAccessView(m_V[i].pTex3D, &udesc, &m_V[i].pUAV);
	m_pDevice->CreateUnorderedAccessView(m_V_s.pTex3D, &udesc, &m_V_s.pUAV);
	m_pDevice->CreateUnorderedAccessView(m_V_bs.pTex3D, &udesc, &m_V_bs.pUAV);

	//�V�F�[�_�[�ǂݍ���
	ID3DBlob* pCompiledShader = NULL;
	MakeShader(m_pDevice, L"Shader/fire/Fire.hlsl", "AddSource", "cs_5_0", (void**)&m_AddSourceShader.pComputeShader, &pCompiledShader);
	MakeShader(m_pDevice, L"Shader/fire/Fire.hlsl", "AdvectDensity", "cs_5_0", (void**)&m_AdvectDensityShader.pComputeShader, &pCompiledShader);
	MakeShader(m_pDevice, L"Shader/fire/Fire.hlsl", "Project1", "cs_5_0", (void**)&m_Project1Shader.pComputeShader, &pCompiledShader);
	MakeShader(m_pDevice, L"Shader/fire/Fire.hlsl", "Project2", "cs_5_0", (void**)&m_Project2Shader.pComputeShader, &pCompiledShader);
	MakeShader(m_pDevice, L"Shader/fire/Fire.hlsl", "Project3", "cs_5_0", (void**)&m_Project3Shader.pComputeShader, &pCompiledShader);
	MakeShader(m_pDevice, L"Shader/fire/Fire.hlsl", "Project4", "cs_5_0", (void**)&m_Project4Shader.pComputeShader, &pCompiledShader);
	MakeShader(m_pDevice, L"Shader/fire/Fire.hlsl", "AdvectVelocity", "cs_5_0", (void**)&m_AdvectVelocityShader.pComputeShader, &pCompiledShader);
	MakeShader(m_pDevice, L"Shader/fire/Fire.hlsl", "AdvectBack", "cs_5_0", (void**)&m_AdvectBackShader.pComputeShader, &pCompiledShader);
	MakeShader(m_pDevice, L"Shader/fire/Fire.hlsl", "MacCormackScheme", "cs_5_0", (void**)&m_MacCormackShader.pComputeShader, &pCompiledShader);
	MakeShader(m_pDevice, L"Shader/fire/Fire.hlsl", "Boundary1", "cs_5_0", (void**)&m_Boundary1Shader.pComputeShader, &pCompiledShader);
	MakeShader(m_pDevice, L"Shader/fire/Fire.hlsl", "Boundary2", "cs_5_0", (void**)&m_Boundary2Shader.pComputeShader, &pCompiledShader);

	//�R���X�^���g�o�b�t�@
	bdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdesc.ByteWidth = sizeof(CS_BUFFER);
	bdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bdesc.MiscFlags = 0;
	bdesc.StructureByteStride = 0;
	bdesc.Usage = D3D11_USAGE_DYNAMIC;
	m_pDevice->CreateBuffer(&bdesc, NULL, &m_pConstantBuffer);

	//�o�b�t�@�p�T���v���[
	D3D11_SAMPLER_DESC SamDesc;
	ZeroMemory(&SamDesc, sizeof(D3D11_SAMPLER_DESC));
	SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	m_pDevice->CreateSamplerState(&SamDesc, &m_pSamLinear);

	//1�p�X�p�@�e�N�X�`���[A�̃����_�[�^�[�Q�b�g�֘A�쐬
	{
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = Width;
		desc.Height = Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		m_pDevice->CreateTexture2D(&desc, NULL, &m_TextureA.pTex2D);
		//���̃e�N�X�`���ɑ΂������_�[�^�[�Q�b�g�r���[(RTV)���쐬
		ZeroMemory(&rdesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		rdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rdesc.Texture2DArray.MipSlice = 0;
		m_pDevice->CreateRenderTargetView(m_TextureA.pTex2D, &rdesc, &m_TextureA.pRTV);
		//���̃e�N�X�`���[�ɑ΂��V�F�[�_�[���\�[�X�r���[�𐶐�
		ZeroMemory(&sdesc, sizeof(sdesc));
		sdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		sdesc.Texture2D.MipLevels = 1;
		m_pDevice->CreateShaderResourceView(m_TextureA.pTex2D, &sdesc, &m_TextureA.pSRV);
		//���̃e�N�X�`���[�̃����_�[�^�[�Q�b�g���ɔ����f�v�X�X�e���V���e�N�X�`���[�쐬
		desc.Width = Width;
		desc.Height = Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		m_pDevice->CreateTexture2D(&desc, NULL, &m_TextureA.pDSTex2D);
		//����DSV���쐬
		m_pDevice->CreateDepthStencilView(m_TextureA.pDSTex2D, NULL, &m_TextureA.pDSV);
	}
	//2�p�X�p�@�e�N�X�`���[B�̃����_�[�^�[�Q�b�g�֘A�쐬
	{
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = Width;
		desc.Height = Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		m_pDevice->CreateTexture2D(&desc, NULL, &m_TextureB.pTex2D);
		//���̃e�N�X�`���[�ɑ΂������_�[�^�[�Q�b�g�r���[���쐬
		ZeroMemory(&rdesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		rdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rdesc.Texture2DArray.MipSlice = 0;
		m_pDevice->CreateRenderTargetView(m_TextureB.pTex2D, &rdesc, &m_TextureB.pRTV);
		//���̃e�N�X�`���[�ɑ΂��V�F�[�_�[���\�[�X�r���[���쐬
		ZeroMemory(&sdesc, sizeof(sdesc));
		sdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		sdesc.Texture2D.MipLevels = 1;
		m_pDevice->CreateShaderResourceView(m_TextureB.pTex2D, &sdesc, &m_TextureB.pSRV);
		//���̃e�N�X�`���[�̃����_�[�^�[�Q�b�g���ɔ����f�v�X�X�e���V���e�N�X�`���[���쐬
		desc.Width = Width;
		desc.Height = Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		m_pDevice->CreateTexture2D(&desc, NULL, &m_TextureB.pDSTex2D);
		//����DSV���쐬
		m_pDevice->CreateDepthStencilView(m_TextureB.pDSTex2D, NULL, &m_TextureB.pDSV);
	}
	//���X�^���C�Y�ݒ�
	D3D11_RASTERIZER_DESC rdc;
	ZeroMemory(&rdc, sizeof(rdc));
	rdc.CullMode = D3D11_CULL_BACK;
	rdc.FillMode = D3D11_FILL_SOLID;
	m_pDevice->CreateRasterizerState(&rdc, &m_pFrontFace);
	rdc.CullMode = D3D11_CULL_FRONT;
	rdc.FillMode = D3D11_FILL_SOLID;
	m_pDevice->CreateRasterizerState(&rdc, &m_pBackFace);

	//�e�N�X�`���[�p�T���v���[�쐬
	ZeroMemory(&SamDesc, sizeof(D3D11_SAMPLER_DESC));
	SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	SamDesc.BorderColor[0] = 0; SamDesc.BorderColor[1] = 0; SamDesc.BorderColor[2] = 0; SamDesc.BorderColor[3] = 0;
	m_pDevice->CreateSamplerState(&SamDesc, &m_pSampleLinear);

	//3D�e�N�X�`���[���t�@�C������ǂݍ���
	CreateEmpty3DTexture(100, m_pDevice, &m_pTexture3D, &m_pTexture3D_SRV);

	//��ʃ|���S���֘A�̊e��I�u�W�F�N�g�쐬

	//�V�F�[�_�[�ǂݍ���
	ID3DBlob* pErrors = NULL;

	if (FAILED(MakeShader(m_pDevice, L"Shader/fire/VolumeRender.hlsl", "VS", "vs_5_0", (void**)&m_ScreenPoly.pVertexShader, &pCompiledShader))) return;

	//���_�C���v�b�g���C�A�E�g���`
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);
	//���_�C���v�b�g���C�A�E�g���쐬
	if (FAILED(m_pDevice->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), &m_ScreenPoly.pVertexLayout)))
	{
		return;
	}
	pCompiledShader->Release();

	if (FAILED(MakeShader(m_pDevice, L"Shader/fire/VolumeRender.hlsl", "PS", "ps_5_0", (void**)&m_ScreenPoly.pPixelShader, &pCompiledShader))) return;

	pCompiledShader->Release();

	//�R���X�^���g�o�b�t�@�쐬
	D3D11_BUFFER_DESC cb;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = sizeof(CBUFFER_F);
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;

	m_pDevice->CreateBuffer(&cb, NULL, &m_ScreenPoly.pCBuffer);

	//���_�o�b�t�@�쐬
	VERTEX_F vertexData[] =
	{
		XMFLOAT3(-1,-1,0),
		XMFLOAT3(-1,1,0),
		XMFLOAT3(1,-1,0),
		XMFLOAT3(1,1,0)
	};

	bdesc.Usage = D3D11_USAGE_DEFAULT;
	bdesc.ByteWidth = sizeof(VERTEX_F) * 4;
	bdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bdesc.CPUAccessFlags = 0;
	bdesc.MiscFlags = 0;
	InitData.pSysMem = vertexData;
	if (FAILED(m_pDevice->CreateBuffer(&bdesc, &InitData, &m_ScreenPoly.pVBuffer)))
	{
		return;
	}

	//�L���[�u�֘A�̊e��I�u�W�F�N�g

	//�V�F�[�_�[�ǂݍ���
	if (FAILED(MakeShader(m_pDevice, L"Shader/fire/Maketexture.hlsl", "VS", "vs_5_0", (void**)&m_Cube.pVertexShader, &pCompiledShader))) return;

	//���_�C���v�b�g���C�A�E�g���쐬

	D3D11_INPUT_ELEMENT_DESC layout2[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	numElements = sizeof(layout2) / sizeof(layout2[0]);
	//���_�C���v�b�g���C�A�E�g���쐬
	if (FAILED(m_pDevice->CreateInputLayout(layout2, numElements, pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), &m_Cube.pVertexLayout)))
	{
		return;
	}
	pCompiledShader->Release();

	if (FAILED(MakeShader(m_pDevice, L"Shader/fire/Maketexture.hlsl", "PS", "ps_5_0", (void**)&m_Cube.pPixelShader, &pCompiledShader))) return;
	pCompiledShader->Release();

	//�R���X�^���g�o�b�t�@�쐬
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = sizeof(CBUFFER_F);
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(m_pDevice->CreateBuffer(&cb, NULL, &m_Cube.pCBuffer)))
	{
		return;
	}
	//���_�o�b�t�@�쐬
	VERTEX_F VertexData2[] =
	{
		XMFLOAT3(-0.5,-0.5,0.5),
		XMFLOAT3(-0.5,-0.5,-0.5),
		XMFLOAT3(-0.5,0.5,0.5),
		XMFLOAT3(-0.5,0.5,-0.5),
		XMFLOAT3(0.5,-0.5,0.5),
		XMFLOAT3(0.5,-0.5,-0.5),
		XMFLOAT3(0.5,0.5,0.5),
		XMFLOAT3(0.5,0.5,-0.5),
	};
	for (int i = 0; i < 8; i++)
	{
		VertexData2[i].Pos.x += 0.5;
		VertexData2[i].Pos.y += 0.5;
		VertexData2[i].Pos.z += 0.5;
	}
	bdesc.Usage = D3D11_USAGE_DEFAULT;
	bdesc.ByteWidth = sizeof(VERTEX_F) * 8;
	bdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bdesc.CPUAccessFlags = 0;
	bdesc.MiscFlags = 0;
	InitData.pSysMem = VertexData2;
	if (FAILED(m_pDevice->CreateBuffer(&bdesc, &InitData, &m_Cube.pVBuffer)))
		return;

	//�C���f�b�N�X�o�b�t�@���쐬
	int IndexData[] =
	{
		2,1,0,
		2,3,1,
		1,4,0,
		1,5,4,
		3,5,1,
		3,7,5,
		3,2,6,
		7,3,6,
		2,0,4,
		6,2,4,
		5,6,4,
		7,6,5
	};
	bdesc.Usage = D3D11_USAGE_DEFAULT;
	bdesc.ByteWidth = sizeof(int) * 12 * 3;
	bdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bdesc.CPUAccessFlags = 0;
	bdesc.MiscFlags = 0;
	InitData.pSysMem = IndexData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	if (FAILED(m_pDevice->CreateBuffer(&bdesc, &InitData, &m_Cube.pIBuffer)))
		return;

	//�A���t�@�u�����h�p�u�����h�X�e�[�g
	D3D11_BLEND_DESC bd;
	bd.IndependentBlendEnable = false;
	bd.AlphaToCoverageEnable = false;
	bd.RenderTarget[0].BlendEnable = true;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_pDevice->CreateBlendState(&bd, &m_pBlendState);
}


void Fire::AddDensitySource(XMFLOAT4& Sphere, XMFLOAT4& Value)
{
	m_Add = true;
	m_Source.DensitySphere = Sphere;
	XMVECTOR vecvalue = XMLoadFloat4(&Value);
	XMVECTOR vecsource = vecvalue * TIME_STEP;
	XMStoreFloat4(&m_Source.DensitySource, vecsource);
}

void Fire::AddVelocitySource(XMFLOAT4& Pos, XMFLOAT4& Value)
{
	m_Add = true;
	m_Source.VelocitySphere = Pos;
	XMVECTOR vecvalue = XMLoadFloat4(&Value);
	XMVECTOR vecsource = vecvalue * TIME_STEP;
	XMStoreFloat4(&m_Source.VelocitySource, vecsource);
}

void Fire::Unbind()
{
	ID3D11UnorderedAccessView* ppUAV_NULL[1] = { NULL };
	ID3D11ShaderResourceView* ppSRV_NULL[1] = { NULL };
	for (int i = 0; i < 8; i++)
	{
		m_pDeviceContext->CSSetUnorderedAccessViews(i, 1, ppUAV_NULL, 0);
		m_pDeviceContext->CSSetShaderResources(i, 1, ppSRV_NULL);
	}
}

void Fire::Solve()
{
	m_pDeviceContext->CSSetSamplers(1, 1, &m_pSamLinear);
	//���x�\���o�[
	SwapOBJ(m_V[0], m_V[1]);

	//Project
	m_pDeviceContext->CSSetUnorderedAccessViews(4, 1, &m_Div.pUAV, 0);
	m_pDeviceContext->CSSetUnorderedAccessViews(3, 1, &m_Prs.pUAV, 0);
	m_pDeviceContext->CSSetShaderResources(1, 1, &m_V[0].pSRV);
	m_pDeviceContext->CSSetShader(m_Project1Shader.pComputeShader, NULL, 0);
	m_pDeviceContext->Dispatch(10, 10, 10);
	Unbind();

	//Projection2
	m_pDeviceContext->CSSetUnorderedAccessViews(3, 1, &m_Prs.pUAV, 0);
	m_pDeviceContext->CSSetUnorderedAccessViews(4, 1, &m_Div.pUAV, 0);
	m_pDeviceContext->CSSetShader(m_Project2Shader.pComputeShader, NULL, 0);
	for (int i = 0; i < 5; i++) m_pDeviceContext->Dispatch(10, 10, 10);
	Unbind();

	//Projection3
	m_pDeviceContext->CSSetShaderResources(1, 1, &m_V[0].pSRV);
	m_pDeviceContext->CSSetUnorderedAccessViews(1, 1, &m_V[1].pUAV, 0);
	m_pDeviceContext->CSSetUnorderedAccessViews(3, 1, &m_Prs.pUAV, 0);
	m_pDeviceContext->CSSetShader(m_Project3Shader.pComputeShader, NULL, 0);
	m_pDeviceContext->Dispatch(10, 10, 10);
	Unbind();

	//Swap
	SwapOBJ(m_V[0], m_V[1]);

	//boundary
	m_pDeviceContext->CSSetShaderResources(1, 1, &m_V[0].pSRV);
	m_pDeviceContext->CSSetUnorderedAccessViews(1, 1, &m_V[1].pUAV, 0);
	m_pDeviceContext->CSSetUnorderedAccessViews(3, 1, &m_Prs.pUAV, 0);
	m_pDeviceContext->CSSetShader(m_Boundary1Shader.pComputeShader, NULL, 0);
	m_pDeviceContext->Dispatch(10, 10, 1);
	Unbind();

	//Advect
	m_pDeviceContext->CSSetShaderResources(1, 1, &m_V[0].pSRV);
	m_pDeviceContext->CSSetUnorderedAccessViews(2, 1, &m_V_s.pUAV, 0);
	m_pDeviceContext->CSSetShader(m_AdvectVelocityShader.pComputeShader, NULL, 0);
	m_pDeviceContext->Dispatch(10, 10, 10);
	Unbind();

	//Advect Back
	m_pDeviceContext->CSSetShaderResources(1, 1, &m_V[0].pSRV);
	m_pDeviceContext->CSSetShaderResources(2, 1, &m_V_s.pSRV);
	m_pDeviceContext->CSSetUnorderedAccessViews(3, 1, &m_V_bs.pUAV, 0);
	m_pDeviceContext->CSSetShader(m_AdvectBackShader.pComputeShader, NULL, 0);
	m_pDeviceContext->Dispatch(10, 10, 10);
	Unbind();

	//MacCormack
	m_pDeviceContext->CSSetShaderResources(1, 1, &m_V[0].pSRV);
	m_pDeviceContext->CSSetUnorderedAccessViews(1, 1, &m_V[1].pUAV, 0);
	m_pDeviceContext->CSSetShaderResources(2, 1, &m_V_s.pSRV);
	m_pDeviceContext->CSSetShaderResources(3, 1, &m_V_bs.pSRV);
	m_pDeviceContext->CSSetShader(m_MacCormackShader.pComputeShader, NULL, 0);
	m_pDeviceContext->Dispatch(10, 10, 10);
	Unbind();

	//���x�\���o�[

	//Advect
	m_pDeviceContext->CSSetShaderResources(1, 1, &m_V[0].pSRV);
	m_pDeviceContext->CSSetUnorderedAccessViews(5, 1, &m_D_Tex.pUAV, 0);
	m_pDeviceContext->CSSetShaderResources(0, 1, &m_D[0].pSRV);
	m_pDeviceContext->CSSetUnorderedAccessViews(0, 1, &m_D[1].pUAV, 0);
	SwapOBJ(m_D[0], m_D[1]);
	m_pDeviceContext->CSSetShader(m_AdvectDensityShader.pComputeShader, NULL, 0);
	m_pDeviceContext->Dispatch(10, 10, 10);
	Unbind();


	//���x�A���x�𒍓�
	if (m_Add)
	{
		D3D11_MAPPED_SUBRESOURCE pData;
		m_pDeviceContext->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData);
		memcpy_s(pData.pData, pData.RowPitch, (void*)(&m_Source), sizeof(m_Source));
		m_pDeviceContext->Unmap(m_pConstantBuffer, 0);
		m_pDeviceContext->CSSetShaderResources(0, 1, &m_D[1].pSRV);
		m_pDeviceContext->CSSetUnorderedAccessViews(0,1,&m_D[0].pUAV,0);
		m_pDeviceContext->CSSetShaderResources(1, 1, &m_V[0].pSRV);
		m_pDeviceContext->CSSetUnorderedAccessViews(1, 1, &m_V[1].pUAV, 0);

		m_pDeviceContext->CSSetShader(m_AddSourceShader.pComputeShader, NULL, 0);
		m_pDeviceContext->CSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		m_pDeviceContext->Dispatch(1, 1, 1);
		Unbind();
	}

	m_pDeviceContext->CSSetShader(NULL, NULL, 0);

	ZeroMemory(&m_Source, sizeof(m_Source));
	m_Add = false;
}

void Fire::DrawFluid(SimpleMath::Matrix& mView, SimpleMath::Matrix& mProj)
{
	SimpleMath::Matrix m_World;
	//���[���h�g�����X�t�H�[��
	m_World = XMMatrixScaling(0.3,0.3,0.3) * XMMatrixRotationY(0) * XMMatrixTranslation(0.6,1,0.01);
	float ClearColor[4] = { 0,0,1,1 };
	m_pDeviceContext->OMSetBlendState(m_pBlendState, NULL, 0xffffffff);
	//1�p�X
	{
		//�����_�[�^�[�Q�b�g���e�N�X�`���[A�ɕύX
		m_pDeviceContext->OMSetRenderTargets(1, &m_TextureA.pRTV, m_TextureA.pDSV);
		//m_pDeviceContext->ClearRenderTargetView(m_TextureA.pRTV, ClearColor);
		m_pDeviceContext->ClearDepthStencilView(m_TextureA.pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

		//�t�����g�t�F�C�X�̂݃����_�����O����悤�ɃJ�����O���[�h���Z�b�g
		m_pDeviceContext->RSSetState(m_pFrontFace);

		//�g�p����V�F�[�_�[�̃Z�b�g
		m_pDeviceContext->VSSetShader(m_Cube.pVertexShader, NULL, 0);
		m_pDeviceContext->PSSetShader(m_Cube.pPixelShader, NULL, 0);
		//�V�F�[�_�[�̃R���X�^���g�o�b�t�@�Ɋe��f�[�^�[��n��
		D3D11_MAPPED_SUBRESOURCE pData;
		CBUFFER_F cb;
		if (SUCCEEDED(m_pDeviceContext->Map(m_Cube.pCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
		{
			//���[���h�s��A�J�����A�ˉe�s���n��
			XMMATRIX m = m_World * mView * mProj;
			XMStoreFloat4x4(&cb.mWVP, XMMatrixTranspose(m));
			memcpy_s(pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb));
			m_pDeviceContext->Unmap(m_Cube.pCBuffer, 0);
		}
		//���݂̃V�F�[�_�[�ɂ��̃R���X�^���g�o�b�t�@���Z�b�g
		m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_Cube.pCBuffer);
		m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_Cube.pCBuffer);
		//���_�C���v�b�g���C�A�E�g���Z�b�g
		m_pDeviceContext->IASetInputLayout(m_Cube.pVertexLayout);
		//�v���~�e�B�u�g�|���W�[���Z�b�g
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//���_�o�b�t�@���Z�b�g
		UINT stride = sizeof(VERTEX_F);
		UINT offset = 0;
		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_Cube.pVBuffer, &stride, &offset);
		//�C���f�b�N�X�o�b�t�@���Z�b�g
		m_pDeviceContext->IASetIndexBuffer(m_Cube.pIBuffer, DXGI_FORMAT_R32_UINT, 0);
		//�����_�����O
		m_pDeviceContext->DrawIndexed(12 * 3, 0, 0);
	}
	//2�p�X
	{
		//�����_�����O�^�[�Q�b�g���e�N�X�`���[B�ɕύX
		m_pDeviceContext->OMSetRenderTargets(1, &m_TextureB.pRTV, m_TextureB.pDSV);
		//m_pDeviceContext->ClearRenderTargetView(m_TextureB.pRTV, ClearColor);
		m_pDeviceContext->ClearDepthStencilView(m_TextureB.pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

		//�o�b�N�t�F�C�X�̂݃����_�����O����悤�ɃJ�����O���[�h���Z�b�g
		m_pDeviceContext->RSSetState(m_pBackFace);

		//�g�p����e�N�X�`���̃Z�b�g
		m_pDeviceContext->VSSetShader(m_Cube.pVertexShader, NULL, 0);
		m_pDeviceContext->PSSetShader(m_Cube.pPixelShader, NULL, 0);
		//�V�F�[�_�[�̃R���X�^���g�o�b�t�@�[�Ɋe��f�[�^��n��
		D3D11_MAPPED_SUBRESOURCE pData;
		CBUFFER_F cb;
		if (SUCCEEDED(m_pDeviceContext->Map(m_Cube.pCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
		{
			//���[���h�A�J�����A�ˉe�s���n��
			XMMATRIX m = m_World * mView * mProj;
			XMStoreFloat4x4(&cb.mWVP, XMMatrixTranspose(m));
			memcpy_s(pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb));
			m_pDeviceContext->Unmap(m_Cube.pCBuffer, 0);
		}
		//���݂̃V�F�[�_�[�ɂ��̃R���X�^���g�o�b�t�@���Z�b�g
		m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_Cube.pCBuffer);
		m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_Cube.pCBuffer);
		//���_�C���v�b�g���C�A�E�g���Z�b�g
		m_pDeviceContext->IASetInputLayout(m_Cube.pVertexLayout);
		//�v���~�e�B�u�g�|���W�[���Z�b�g
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//���_�o�b�t�@���Z�b�g
		UINT stride = sizeof(VERTEX_F);
		UINT offset = 0;
		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_Cube.pVBuffer, &stride, &offset);
		//�C���f�b�N�X�o�b�t�@���Z�b�g
		m_pDeviceContext->IASetIndexBuffer(m_Cube.pIBuffer, DXGI_FORMAT_R32_UINT, 0);
		//�����_�����O
		m_pDeviceContext->DrawIndexed(12 * 3, 0, 0);

	}
	//3�p�X
	{
		//�����_�[�^�[�Q�b�g���o�b�N�o�b�t�@�[�ɖ߂�
		m_pDeviceContext->OMSetRenderTargets(1, &m_BackBuffer.pRTV, m_BackBuffer.pDSV);
		//m_pDeviceContext->ClearRenderTargetView(m_BackBuffer.pRTV, ClearColor);//��ʃN���A
		m_pDeviceContext->ClearDepthStencilView(m_BackBuffer.pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);//�[�x�o�b�t�@�N���A

		m_pDeviceContext->RSSetState(m_pFrontFace);
		m_pDeviceContext->PSSetSamplers(0, 1, &m_pSampleLinear);

		//�g�p����V�F�[�_�[�̃Z�b�g	
		m_pDeviceContext->VSSetShader(m_ScreenPoly.pVertexShader, NULL, 0);
		m_pDeviceContext->PSSetShader(m_ScreenPoly.pPixelShader, NULL, 0);

		//�p�X�P�ƃp�X�Q�ō쐬�����e�N�X�`���[2�����V�F�[�_�[�ɃZ�b�g
		m_pDeviceContext->PSSetShaderResources(0, 1, &m_TextureA.pSRV);
		m_pDeviceContext->PSSetShaderResources(1, 1, &m_TextureB.pSRV);
		//3D�e�N�X�`���[���V�F�[�_�[�ɃZ�b�g		
		m_pDeviceContext->PSSetShaderResources(2,1,&m_pTexture3D_SRV);
		m_pDeviceContext->PSSetShaderResources(2, 1, &m_D_Tex.pSRV);

		//�V�F�[�_�[�̃R���X�^���g�o�b�t�@�[�Ɋe��f�[�^��n��
		D3D11_MAPPED_SUBRESOURCE pData;
		CBUFFER_F cb;
		if (SUCCEEDED(m_pDeviceContext->Map(m_ScreenPoly.pCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
		{
			//���[���h�A�J�����A�ˉe�s���n��
			XMMATRIX m = m_World * mView * mProj;
			XMStoreFloat4x4(&cb.mWVP, XMMatrixTranspose(m));
			memcpy_s(pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb));
			m_pDeviceContext->Unmap(m_ScreenPoly.pCBuffer, 0);
		}
		//���݂̃V�F�[�_�[�ɂ��̃R���X�^���g�o�b�t�@�[���Z�b�g
		m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_ScreenPoly.pCBuffer);
		m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_ScreenPoly.pCBuffer);
		//���_�C���v�b�g���C�A�E�g���Z�b�g
		m_pDeviceContext->IASetInputLayout(m_Cube.pVertexLayout);
		//�v���~�e�B�u�E�g�|���W�[���Z�b�g
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//���_�o�b�t�@���Z�b�g
		UINT stride = sizeof(VERTEX_F);
		UINT offset = 0;
		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_Cube.pVBuffer, &stride, &offset);
		//�C���f�b�N�X�o�b�t�@���Z�b�g
		m_pDeviceContext->IASetIndexBuffer(m_Cube.pIBuffer, DXGI_FORMAT_R32_UINT, 0);
		//�����_�����O
		m_pDeviceContext->DrawIndexed(12 * 3, 0, 0);

	}
}