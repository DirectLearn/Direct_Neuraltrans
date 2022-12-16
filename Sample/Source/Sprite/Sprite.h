#pragma once
#include "Source/DirectX/DirectX.h"

class Sprite
{
private:
	ComPtr<ID3D11ShaderResourceView> m_texture;
	ComPtr<ID3D11Resource> resource;
	ComPtr<ID3D11Texture2D> sprite;
	std::unique_ptr<CommonStates> m_states;
	std::unique_ptr<SpriteBatch> m_spriteBatch;
	XMFLOAT2 m_origin;
public:
	void Init(const wchar_t* filepath,ID3D11DeviceContext* context,ID3D11Device* device);
	void DrawSprite(XMFLOAT2 pos,ID3D11DepthStencilState* depthstate,float scale);
};