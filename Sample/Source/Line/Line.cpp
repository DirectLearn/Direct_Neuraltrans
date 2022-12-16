#include "Line.h"


void Line::Draw(ID3D11DeviceContext* context, SimpleMath::Matrix& m_World, SimpleMath::Matrix& m_View, SimpleMath::Matrix& m_Proj)
{
	//使用するシェーダーのセット
	context->VSSetShader(m_pVertexShader.Get(), NULL, 0);
	context->PSSetShader(m_pPixelShader.Get(), NULL, 0);

	//頂点インプットレイアウトをセット
	context->IASetInputLayout(m_pVertexLayout.Get());
	//プリミティブトポロジーをセット
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	//シェーダーにコンスタントバッファをセット
	context->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	context->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	//バーテックスバッファセット
	UINT stride = sizeof(lineVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);

	//シェーダーのコンスタントバッファに各種データを渡す
	D3D11_MAPPED_SUBRESOURCE pData;
	line_ConstantBuffer cb;
	XMStoreFloat4x4(&cb.mWorld, XMMatrixTranspose(m_World));
	XMStoreFloat4x4(&cb.mView, XMMatrixTranspose(m_View));
	XMStoreFloat4x4(&cb.mProj, XMMatrixTranspose(m_Proj));
	context->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData);
	memcpy_s(pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb));
	context->Unmap(m_pConstantBuffer, 0);

	//レンダリング
	context->Draw(2, 0);
}

void Line::Init(ID3D11Device* device)
{
	//シェーダー読み込み
	ID3DBlob* pCompileVS = NULL;
	ID3DBlob* pCompilePS = NULL;

	//頂点シェーダー
	D3DCompileFromFile(L"Shader/line/line.hlsl", NULL, NULL, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	assert(pCompileVS && "pCompileVS is nullptr");
	device->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, m_pVertexShader.GetAddressOf());
	//ピクセルシェーダー
	D3DCompileFromFile(L"Shader/line/line.hlsl", NULL, NULL, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	assert(pCompilePS && "pCompilePS is nullptr");
	device->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, m_pPixelShader.GetAddressOf());

	//頂点レイアウト
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	(device->CreateInputLayout(layout, ARRAYSIZE(layout), pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), m_pVertexLayout.GetAddressOf()));
	pCompileVS->Release();
	pCompilePS->Release();

	//コンスタントバッファ
	D3D11_BUFFER_DESC cb;
	cb.ByteWidth = sizeof(line_ConstantBuffer);
	cb.Usage = D3D11_USAGE_DYNAMIC;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	(device->CreateBuffer(&cb, NULL, &m_pConstantBuffer));

	Createline(device);
}

void Line::Createline(ID3D11Device* device)
{
	//バーテックスバッファ作成
	lineVertex vertices[] =
	{
		{{-0.2f,0.5f,0.0f}},
		{{0.2f,0.5f,0.0f}},
	};

	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(lineVertex) * 2;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;
	device->CreateBuffer(&bd, &InitData, m_pVertexBuffer.GetAddressOf());
}

