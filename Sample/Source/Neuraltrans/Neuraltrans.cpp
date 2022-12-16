#include "Neuraltrans.h"

Neuraltrans::Neuraltrans()
{
	cv::String modelPathN = "Data/models/eccv16/starry_night.t7";
	cv::String modelPathW = "Data/models/eccv16/the_wave.t7";
	cv::String modelPathM = "Data/models/eccv16/la_muse.t7";
	cv::String modelPathV = "Data/models/eccv16/composition_vii.t7";
	net = cv::dnn::readNetFromTorch(modelPathV);
	net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
	net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
}

void Neuraltrans::Init(ID3D11Device* device,int width,int height)
{
	//シェーダー作成
	ID3DBlob* pCompileVS = NULL;
	ID3DBlob* pCompilePS = NULL;

	//バーテックスシェーダー作成
	D3DCompileFromFile(L"Shader/Neuraltrans/Neuraltrans.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	assert(pCompileVS && "pCompileVS is nullptr");
	(device->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, &pVertexShader));
	//ピクセルシェーダー作成
	D3DCompileFromFile(L"Shader/Neuraltrans/Neuraltrans.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	assert(pCompilePS && "pCompilePS is nullptr");
	(device->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &pPixelShader));

	//頂点レイアウト
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXUV",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	(device->CreateInputLayout(layout, ARRAYSIZE(layout), pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), &pVertexLayout));
	pCompileVS->Release();
	pCompilePS->Release();

	//定数バッファの作成
	D3D11_BUFFER_DESC cb;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = sizeof(Constant_Buffer);
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;
	(device->CreateBuffer(&cb, NULL, &pConstantBuffer));

	//------------テクスチャ作成------------------------

	//バーテックスバッファの作成
	SquareVertex vertices[] =
	{
		{{0.0f,0.0f,0.0f},{0.0f,0.0f}},
		{{(float)width,0.0f,0.0f},{1.0f,0.0f}},
		{{0.0f,(float)height,0.0f},{0.0f,1.0f}},
		{{(float)width,(float)height,0.0f},{1.0f,1.0f}}
	};
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(SquareVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA sub = {};
	sub.pSysMem = vertices;
	device->CreateBuffer(&bd, &sub, &pVertexBuffer);

	//インデックスバッファの作成
	WORD indices[] =
	{
		0,1,2,
		2,1,3,
	};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	sub.pSysMem = indices;
	device->CreateBuffer(&bd, &sub, &pIndexBuffer);

	//テクスチャの作成
	D3D11_TEXTURE2D_DESC td;
	td.Width = width;
	td.Height = height;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DYNAMIC;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	td.MiscFlags = 0;
	device->CreateTexture2D(&td, nullptr, &pTexture);

	
}	

void Neuraltrans::DLtorchModel(ID3D11Device* device,ID3D11DeviceContext* context,IDXGISwapChain* swapchain,ID3D11Texture2D* inputStagingTex,int width,int height)
{
	
	ID3D11Texture2D* inputTex;

	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&inputTex));

	ID3D11Texture2D* pCopyTexture = NULL;

	D3D11_TEXTURE2D_DESC description;
	inputTex->GetDesc(&description);
	description.BindFlags = 0;
	description.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	description.Usage = D3D11_USAGE_STAGING;
	device->CreateTexture2D(&description, NULL, &pCopyTexture);
	
	context->CopyResource(pCopyTexture, inputTex);


	D3D11_MAPPED_SUBRESOURCE ResourceDesc;
	context->Map(pCopyTexture, NULL, D3D11_MAP_READ, 0, &ResourceDesc);

	size_t buffer_size = ResourceDesc.RowPitch * height;

	byte* Read = new byte[buffer_size]();


	if (ResourceDesc.pData)
	{

		CopyMemory(&img.data[0], ResourceDesc.pData, buffer_size);
		
	}

	context->Unmap(pCopyTexture, 0);

	cv::cvtColor(img, img2, cv::COLOR_RGBA2BGR);
	
	blob = cv::dnn::blobFromImage(img2, 1.0, cv::Size(width, height), mean, false, false);
	net.setInput(blob);
	cv::Mat prob = net.forward();
	std::vector<cv::Mat> result;
	cv::dnn::imagesFromBlob(prob, result);
	result[0] += mean;
	result[0] /= 255.0;
	

	D3D11_MAPPED_SUBRESOURCE msr;
	context->Map(pTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	cv::cvtColor(result[0], copyimg, cv::COLOR_BGR2RGBA);

	size_t img_size = msr.RowPitch * height;
	
	CopyMemory(msr.pData, copyimg.data, img_size);

	context->Unmap(pTexture.Get(),0);
	//シェーダーリソースビューの作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srv = {};
	srv.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(pTexture.Get(), &srv, &pShaderResouceView);

	pCopyTexture->Release();
	delete[] Read;
}

void Neuraltrans::Draw(ID3D11DeviceContext* context, ID3D11SamplerState *samp,SimpleMath::Matrix& world, SimpleMath::Matrix& view, SimpleMath::Matrix& proj,int width,int height)
{
	UINT stride = sizeof(SquareVertex);
	UINT offset = 0;
	context->VSSetShader(pVertexShader.Get(), nullptr, 0);
	context->PSSetShader(pPixelShader.Get(), nullptr, 0);
	context->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetInputLayout(pVertexLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	context->PSSetShaderResources(0, 1, pShaderResouceView.GetAddressOf());
	context->PSSetSamplers(0, 1, &samp);

	//頂点の書き換え
	D3D11_MAPPED_SUBRESOURCE msr;
	context->Map(pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	SquareVertex vertices[] =
	{
		{{0.0f,0.0f,0.0f},{0.0f,0.0f}},
		{{(float)width,0.0f,0.0f},{1.0f,0.0f}},
		{{0.0f,(float)height,0.0f},{0.0f,1.0f}},
		{{(float)width,(float)height,0.0f},{1.0f,1.0f}}
	};
	memcpy(msr.pData, vertices, sizeof(vertices));
	context->Unmap(pVertexBuffer.Get(), 0);

	Constant_Buffer cb;
	XMStoreFloat4x4(&cb.world,XMMatrixTranspose(world * proj));
	XMStoreFloat4x4(&cb.view, XMMatrixTranspose(view));
	XMStoreFloat4x4(&cb.proj, XMMatrixTranspose(proj));

	context->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy_s(msr.pData, msr.RowPitch, (void*)(&cb), sizeof(cb));
	context->Unmap(pConstantBuffer, 0);

	//描画
	context->DrawIndexed(6, 0, 0);
	
}

Neuraltrans::~Neuraltrans()
{
}