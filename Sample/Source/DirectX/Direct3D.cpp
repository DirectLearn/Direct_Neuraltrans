#include "framework.h"

#include "Directx3D.h"

#include <windows.h>

bool Direct3D::Initialize(HWND hWnd, int width, int height)
{
	ScreenWidth = width;
	ScreenHeight = height;
	DepthWidth = width * 2;
	DepthHeight = height * 2;
	h_Wnd = hWnd;

	//---------------------------------------------------------------------
	//�t�@�N�g���[�쐬(�r�f�I�O���t�B�b�N�̐ݒ�̗񋓂�w��Ɏg�p�����I�u�W�F�N�g
	//--------------------------------------------------------------------
	ComPtr<IDXGIFactory> factory;

	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
	{
		return false;
	}

	//--------------------------------------------------------------------
	//�f�o�C�X����(��Ƀ��\�[�X�쐬���Ɏg�p����I�u�W�F�N�g)
	//-------------------------------------------------------------------
	UINT creationFlags = 0;
#ifdef _DEBUG
	//DEBUG�r���h����Direct3D�̃f�o�b�O��L���ɂ���
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1, //Direct3D 11.1 ShaderModel 5
		D3D_FEATURE_LEVEL_11_0, //Direct3D 11.0 ShaderModel 5
		D3D_FEATURE_LEVEL_10_1, //Direct3D 10.1 ShaderModel 4
		D3D_FEATURE_LEVEL_10_0, //Direct3D 10.0 ShaderModel 4
		D3D_FEATURE_LEVEL_9_3,  //Direct3D 9.3 ShaderModel 3
		D3D_FEATURE_LEVEL_9_2,  //Direct3D 9.2 ShaderModel 3
		D3D_FEATURE_LEVEL_9_1,  //Direct3D 9.1 ShaderModel 3
	};

	//�f�o�C�X�ƂŃf�o�C�X�R���e�L�X�g���쐬
	D3D_FEATURE_LEVEL futureLevel;
	if (FAILED(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creationFlags,
		featureLevels,
		_countof(featureLevels),
		D3D11_SDK_VERSION,
		&m_device,
		&futureLevel,
		&m_deviceContext
	)))
	{
		return false;
	}

	//--------------------------------------------------------
	//�X���b�v�`�F�C���쐬(�t�����g�o�b�t�@�ɕ\���\�ȃo�b�N�o�b�t�@��������)
	//-------------------------------------------------------------------
	DXGI_SWAP_CHAIN_DESC scDesc = {};	//�X���b�v�`�F�[���̐ݒ�f�[�^
	scDesc.BufferDesc.Width = width;
	scDesc.BufferDesc.Height = height;
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scDesc.BufferDesc.RefreshRate.Numerator = 0;
	scDesc.BufferDesc.RefreshRate.Denominator = 1;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//�o�b�t�@�̎g�p���@
	scDesc.BufferCount = 2;
	scDesc.OutputWindow = hWnd;
	scDesc.Windowed = TRUE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//�X���b�v�`�F�C���̍쐬
	if (FAILED(factory->CreateSwapChain(m_device.Get(), &scDesc, &m_swapChain)))
	{
		return false;
	}

	//�X�e���V���p�e�N�X�`���[�̐ݒ�(�[�w�o�b�t�@)
	D3D11_TEXTURE2D_DESC hTexture2dDesc;
	hTexture2dDesc.Width = scDesc.BufferDesc.Width;
	hTexture2dDesc.Height = scDesc.BufferDesc.Height;
	hTexture2dDesc.MipLevels = 1;
	hTexture2dDesc.ArraySize = 1;
	hTexture2dDesc.Format = DXGI_FORMAT_D32_FLOAT;
	hTexture2dDesc.SampleDesc.Count =1;
	hTexture2dDesc.SampleDesc.Quality = 0;
	hTexture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	hTexture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	hTexture2dDesc.CPUAccessFlags = 0;
	hTexture2dDesc.MiscFlags = 0;
	//�X�e���V���p�e�N�X�`���̍쐬�i�[�w�o�b�t�@�j
	if (FAILED(m_device->CreateTexture2D(&hTexture2dDesc, NULL, &m_pTexture2DDepthStencil)))
	{
		return false;
	}
	

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	//�X�e���V���X�e�[�g�̏������i�R�c�p�j
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	//�X�e���V���X�e�[�g�ݒ�
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//�X�e���V���X�e�[�g�쐬
	if (FAILED(m_device->CreateDepthStencilState(&depthStencilDesc, &m_pDepthStencilState)))
	{
		return false;
	}

	

	//�X�e���V���^�[�Q�b�g�쐬
	D3D11_DEPTH_STENCIL_VIEW_DESC hDepthStencilViewDesc;
	hDepthStencilViewDesc.Format = hTexture2dDesc.Format;
	hDepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	hDepthStencilViewDesc.Flags = 0;
	if (FAILED(m_device->CreateDepthStencilView(m_pTexture2DDepthStencil.Get(), &hDepthStencilViewDesc, &m_pBackBuffer_DSTexDSV)))
	{
		return false;
	}


	//�X���b�v�`�F�C������o�b�N�o�b�t�@���\�[�X�擾
	if (FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
	{
		return false;
	}

	//�o�b�N�o�b�t�@���\�[�X�p��RTV���쐬
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = scDesc.BufferDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	if (FAILED(m_device->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, &m_backBufferView)))
	{
		return false;
	}

	m_deviceContext->OMSetRenderTargets(1, m_backBufferView.GetAddressOf(), 0);


	//�[�x�}�b�v�e�N�X�`���[���쐬
	D3D11_TEXTURE2D_DESC tdesc;
	ZeroMemory(&tdesc, sizeof(D3D11_TEXTURE2D_DESC));
	tdesc.Width = ScreenWidth;
	tdesc.Height = ScreenHeight;
	tdesc.MipLevels = 1;
	tdesc.ArraySize = 1;
	tdesc.Format = DXGI_FORMAT_R32_FLOAT;
	tdesc.SampleDesc.Count = 1;
	tdesc.SampleDesc.Quality = 0;
	tdesc.Usage = D3D11_USAGE_DEFAULT;
	tdesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	tdesc.CPUAccessFlags = 0;

	if (FAILED(m_device->CreateTexture2D(&tdesc, NULL, &m_pDepthMap_Tex)))
	{
		return false;
	}

	//�[�x�}�b�v�e�N�X�`���[�p�����_�[�^�[�Q�b�g�r���[�쐬
	D3D11_RENDER_TARGET_VIEW_DESC DescRT;
	DescRT.Format = DXGI_FORMAT_R32_FLOAT;
	DescRT.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	DescRT.Texture2D.MipSlice = 0;

	if (FAILED(m_device->CreateRenderTargetView(m_pDepthMap_Tex.Get(), &DescRT, &m_pDepthMap_RTV)))
	{
		return false;
	}

	hTexture2dDesc.Width = ScreenWidth;
	hTexture2dDesc.Height = ScreenHeight;
	hTexture2dDesc.MipLevels = 1;
	hTexture2dDesc.ArraySize = 1;
	hTexture2dDesc.Format = DXGI_FORMAT_D32_FLOAT;
	hTexture2dDesc.SampleDesc.Count = 1;
	hTexture2dDesc.SampleDesc.Quality = 0;
	hTexture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	hTexture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	hTexture2dDesc.CPUAccessFlags = 0;
	hTexture2dDesc.MiscFlags = 0;

	//�[�x�}�b�v�e�N�X�`���������_�[�^�[�Q�b�g�ɂ���ۂ̃f�v�X�X�e���V���r���[�p�̃e�N�X�`���[���쐬
	if (FAILED(m_device->CreateTexture2D(&hTexture2dDesc, NULL,&m_pDepthMap_DSTex)))
	{
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSVRO;
	ZeroMemory(&descDSVRO, sizeof(descDSVRO));
	descDSVRO.Format = DXGI_FORMAT_D32_FLOAT;
	descDSVRO.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	descDSVRO.Flags = 0;

	//�f�v�X�X�e���V���r���[�쐬
	if (FAILED(m_device->CreateDepthStencilView(m_pDepthMap_DSTex.Get(), &descDSVRO, &m_pDepthMap_DSTexDSV)))
	{
		return false;
	}

	//�[�x�}�b�v�e�N�X�`���p�@�V�F�[�_�[���\�[�X�r���[(SRV)�쐬
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = DescRT.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	SRVDesc.Texture2D.MipLevels = 1;

	if (FAILED(m_device->CreateShaderResourceView(m_pDepthMap_Tex.Get(), &SRVDesc, &m_pDepthMap_TexSRV)))
	{
		return false;
	}


	/*
	//SSAO�p�̃e�N�X�`���[�쐬
	tdesc.Width = ScreenWidth;
	tdesc.Height = ScreenHeight;
	tdesc.MipLevels = 1;
	tdesc.ArraySize = 1;
	tdesc.MiscFlags = 0;
	tdesc.Format = DXGI_FORMAT_R32_FLOAT;
	tdesc.SampleDesc.Count = 1;
	tdesc.SampleDesc.Quality = 0;
	tdesc.Usage = D3D11_USAGE_DEFAULT;
	tdesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	tdesc.CPUAccessFlags = 0;

	if (FAILED(m_device->CreateTexture2D(&tdesc, NULL, &m_pSSAO_Tex)))
	{
		return false;
	}

	//SSAO�p�̃����_�[�^�[�Q�b�g�쐬
	DescRT.Format = DXGI_FORMAT_R32_FLOAT;
	DescRT.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	DescRT.Texture2D.MipSlice = 0;

	if (FAILED(m_device->CreateRenderTargetView(m_pSSAO_Tex.Get(), &DescRT, &m_pSSAO_RTV)))
	{
		return false;
	}

	//SSAO�p�̃f�v�X�X�e���V���r���[�̃e�N�X�`���[�쐬
	if (FAILED(m_device->CreateTexture2D(&hTexture2dDesc, NULL, &m_pSSAO_DSTex)))
	{
		return false;
	}

	

	//�f�v�X�X�e���V���r���[�쐬
	if (FAILED(m_device->CreateDepthStencilView(m_pSSAO_DSTex.Get(), &descDSVRO, &m_pSSAO_DSTexDSV)))
	{
		return false;
	}

	//SSAO�p�̃V�F�[�_�[���\�[�X�r���[(SRV)�쐬
	if (FAILED(m_device->CreateShaderResourceView(m_pSSAO_Tex.Get(), &SRVDesc, &m_pSSAO_TexSRV)))
	{
		return false;
	}
	
	*/

	//�ʏ�`��̃e�N�X�`���쐬
	tdesc.Width = ScreenWidth;
	tdesc.Height = ScreenHeight;
	tdesc.MipLevels = 1;
	tdesc.ArraySize = 1;
	tdesc.MiscFlags = 0;
	tdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tdesc.SampleDesc.Count = 1;
	tdesc.SampleDesc.Quality = 0;
	tdesc.Usage = D3D11_USAGE_DEFAULT;
	tdesc.BindFlags = 0;
	tdesc.CPUAccessFlags = 0;

	if (FAILED(m_device->CreateTexture2D(&tdesc, NULL, &m_pNormal_Tex)))
	{
		return false;
	}


	//�ʏ�`��p�̃����_�[�^�[�Q�b�g�쐬
	DescRT.Format = tdesc.Format;
	DescRT.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	DescRT.Texture2D.MipSlice = 0;

	if (FAILED(m_device->CreateRenderTargetView(m_pNormal_Tex.Get(), &DescRT, &m_pNormal_RTV)))
	{
		return false;
	}

	//�R�s�[�p�e�N�X�`���쐬
	tdesc.Usage = D3D11_USAGE_STAGING;
	tdesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_device->CreateTexture2D(&tdesc, NULL, &m_pStaging_Tex)))
	{
		return false;
	}

	//�ʏ�`��p�̃f�v�X�X�e���V���r���[�̃e�N�X�`���[�쐬
	if (FAILED(m_device->CreateTexture2D(&hTexture2dDesc, NULL, &m_pNormal_DSTex)))
	{
		return false;
	}

	//�f�v�X�X�e���V���r���[�쐬
	if (FAILED(m_device->CreateDepthStencilView(m_pNormal_DSTex.Get(), NULL, &m_pNormal_DSTexDSV)))
	{
		return false;
	}

	if (FAILED(m_device->CreateShaderResourceView(m_pNormal_Tex.Get(), NULL, &m_pNormalTexSRV)))
	{
		return false;
	}

	//-------------------------------------------------------
	//�f�o�C�X�R���e�L�X�g�ɕ`��Ɋւ���ݒ���s���Ă���
	//-----------------------------------------------------

	//�o�b�N�o�b�t�@��RT�Ƃ��ăZ�b�g
	//m_deviceContext->OMSetRenderTargets(1, m_backBufferView.GetAddressOf(), m_pBackBuffer_DSTexDSV.Get());

	//�r���[�|�[�g�̐ݒ�
	//���_�V�F�[�_�[��ǂݍ���&�R���p�C��
	D3D11_VIEWPORT vp; //= { 0.0f,0.0f,(float)width,(float)height,0.0f,1.0f };
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_deviceContext->RSSetViewports(1, &vp);

	
	//���X�^���C�U�ݒ�
	D3D11_RASTERIZER_DESC hRasterizerDesc = {
		D3D11_FILL_SOLID,
		D3D11_CULL_NONE,
		FALSE,
		0,
		0.0f,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		FALSE
	};
	if (FAILED(m_device->CreateRasterizerState(&hRasterizerDesc, &m_pRasterizerState)))
	{
		return false;
	}


	//�u�����f�B���O�X�e�[�g����
	D3D11_BLEND_DESC bd;
	bd.IndependentBlendEnable = false;
	bd.AlphaToCoverageEnable = false;
	bd.RenderTarget[0].BlendEnable = false;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;


	m_device->CreateBlendState(&bd, &BlendState);
	
	
	//�T���v������
	D3D11_SAMPLER_DESC samDesc[2];
	samDesc[0].Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samDesc[0].AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc[0].AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc[0].AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//samDesc[0].MipLODBias = 0;
	//samDesc[0].MaxAnisotropy = 1;
	//samDesc[0].ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	//samDesc[0].BorderColor[0] = samDesc[0].BorderColor[1] = samDesc[0].BorderColor[2] = samDesc[0].BorderColor[3] = 1.0;
	//samDesc[0].MinLOD = 0;
	//samDesc[0].MaxLOD = D3D11_FLOAT32_MAX;

	samDesc[1] = samDesc[0];
	samDesc[1].AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samDesc[1].AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samDesc[1].AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	m_device->CreateSamplerState(&samDesc[0], &SamplerState);
	m_device->CreateSamplerState(&samDesc[1], &ToonState);

	/*
	//�V�F�[�_�[�̍쐬
	ComPtr<ID3DBlob> compiledVS;
	if (FAILED(D3DCompileFromFile(L"Shader/SpriteShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0",0, 0, &compiledVS, nullptr)))
	{
		return false;
	}
	//�s�N�Z���V�F�[�_�[��ǂݍ���&�R���p�C��
	ComPtr<ID3DBlob> compiledPS;
	if (FAILED(D3DCompileFromFile(L"Shader/SpriteShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_5_0", 0, 0, &compiledPS, nullptr)))
	{
		return false;
	}

	//���_�V�F�[�_�[�쐬
	if (FAILED(m_device->CreateVertexShader(compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), nullptr, &m_spriteVS)))
	{
		return false;
	}

	//�s�N�Z���V�F�[�_�[�쐬
	if (FAILED(m_device->CreatePixelShader(compiledPS->GetBufferPointer(), compiledPS->GetBufferSize(), nullptr, &m_spritePS)))
	{
		return false;
	}

	//1���_�̏ڍׂȏ��
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXUV",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
	};

	//���_�C���v�b�g���C�A�E�g���쐬
	if (FAILED(m_device->CreateInputLayout(&layout[0], layout.size(), compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), &m_spriteInputLayout)))
	{
		return false;
	}
	*/


	return true;
}


void Direct3D::ChangeMode_2D()
{
	//2D�p���_�V�F�[�_�[�Z�b�g
	m_deviceContext->VSSetShader(m_spriteVS.Get(), 0, 0);
	//2D�p�s�N�Z���V�F�[�_�[�Z�b�g
	m_deviceContext->PSSetShader(m_spritePS.Get(), 0, 0);
	//���̓��C�A�E�g�Z�b�g
	m_deviceContext->IASetInputLayout(m_spriteInputLayout.Get());

	//�l�p�`�p���_�o�b�t�@(����̂�)
	if (m_vbSquare == nullptr)
	{
		D3D11_BUFFER_DESC vbDesc = {};
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbDesc.ByteWidth = sizeof(VertexType2D) * 4;
		vbDesc.MiscFlags = 0;
		vbDesc.StructureByteStride = 0;
		vbDesc.Usage = D3D11_USAGE_DYNAMIC;
		vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		m_device->CreateBuffer(&vbDesc, nullptr, &m_vbSquare);
	}

	//���_�o�b�t�@��`��Ŏg����悤�ɂ���
	UINT stride = sizeof(VertexType2D);
	UINT offset = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, m_vbSquare.GetAddressOf(), &stride, &offset);
	//�v���~�e�B�u�g�|���W�[���Z�b�g
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//�T���v���[�X�e�[�g���쐬���Z�b�g����
	{
		//�ٕ����t�B���^�����O���,Wrap���[�h
		D3D11_SAMPLER_DESC desc = {};
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MipLODBias = 0;
		desc.MaxAnisotropy = 0;
		desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.BorderColor[0] = desc.BorderColor[1] = desc.BorderColor[2] = desc.BorderColor[3] = 0;
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;

		//�X�e�[�g�I�u�W�F�N�g�쐬
		ComPtr<ID3D11SamplerState> state;
		m_device->CreateSamplerState(&desc, &state);
		//�e�V�F�[�_�[��0�ԖڂɃZ�b�g
		m_deviceContext->VSSetSamplers(0, 1, state.GetAddressOf());
		m_deviceContext->PSSetSamplers(0, 1, state.GetAddressOf());
		m_deviceContext->GSSetSamplers(0, 1, state.GetAddressOf());
		m_deviceContext->CSSetSamplers(0, 1, state.GetAddressOf());
	}
}

void Direct3D::Draw2D(const Texture& tex, float x, float y, float w, float h)
{
	float hW = w * 0.5f;
	float hH = h * 0.5f;

	//���_�f�[�^�쐬
	VertexType2D v[4] = {
		{{x - hW,y - hH,0},{0,1}},
		{{x - hW,y + hH,0},{0,0}},
		{{x + hW,y - hH,0},{1,1}},
		{{x + hW,y + hH,0},{1,0}},
	};
	
	//���_�o�b�t�@�Ƀf�[�^����������
	D3D11_MAPPED_SUBRESOURCE pData;
	if (SUCCEEDED(m_deviceContext->Map(m_vbSquare.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		//�f�[�^�R�s�[
		memcpy_s(pData.pData, sizeof(v), &v[0], sizeof(v));

		m_deviceContext->Unmap(m_vbSquare.Get(), 0);
	}

	//�e�N�X�`�����A�X���b�g0�ɃZ�b�g
	m_deviceContext->PSSetShaderResources(0, 1, tex.m_srv.GetAddressOf());

	//���X�^���C�U�X�e�[�g
	m_deviceContext->RSSetState(m_pRasterizerState.Get());

	//�f�v�X�X�e���V���X�e�[�g
	m_deviceContext->OMSetDepthStencilState(m_pDepthStencilState.Get(),0);

	m_deviceContext->Draw(4, 0);
}


void Direct3D::FbxCreate(const FBX_Model& fbx)
{
	
}

void Direct3D::UpdateWorldMatrix(XMMATRIX& World,XMMATRIX& View,XMMATRIX& Proj) {

	ConstantBuffer CBuffer;

	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = sizeof(ConstantBuffer);
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &CBuffer;
	data.SysMemPitch = sizeof(ConstantBuffer);
	data.SysMemSlicePitch = 0;

	if (FAILED(m_device->CreateBuffer(&cbDesc, &data, &m_cb)))
	{
		MessageBox(NULL,L"�萔�o�b�t�@���쐬�ł��܂���ł���",NULL,MB_OK);
	}


	//�s���n��
	D3D11_MAPPED_SUBRESOURCE pData;
	if (SUCCEEDED(m_deviceContext->Map(m_cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{

		XMStoreFloat4x4(&CBuffer.m_W,XMMatrixTranspose(World));
		XMStoreFloat4x4(&CBuffer.m_V, XMMatrixTranspose(View));
		XMStoreFloat4x4(&CBuffer.m_P, XMMatrixTranspose(Proj));

		memcpy_s(pData.pData, pData.RowPitch, (void*)(&CBuffer), sizeof(ConstantBuffer));
		m_deviceContext->Unmap(m_cb.Get(), 0);
	}
	

	//�R���X�^���o�b�t�@���X���b�g�ɃZ�b�g
	m_deviceContext->VSSetConstantBuffers(0, 1, m_cb.GetAddressOf());
	m_deviceContext->PSSetConstantBuffers(0, 1, m_cb.GetAddressOf());
}

