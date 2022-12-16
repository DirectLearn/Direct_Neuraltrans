#pragma once
#include "Source/DirectX/DirectX.h"

#define ALIGN16 _declspec(align(16))

struct lineVertex
{
	XMFLOAT3 Pos;
};

struct line_ConstantBuffer
{
	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
};

class Line
{
private:
	ComPtr<ID3D11InputLayout> m_pVertexLayout;
	ComPtr<ID3D11VertexShader> m_pVertexShader;
	ComPtr<ID3D11PixelShader> m_pPixelShader;
	ID3D11Buffer* m_pConstantBuffer;
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
public:
	void Init(ID3D11Device* device);
	void Draw(ID3D11DeviceContext* context, SimpleMath::Matrix& m_World, SimpleMath::Matrix& m_view, SimpleMath::Matrix& m_proj);
	void Createline(ID3D11Device* device);
};