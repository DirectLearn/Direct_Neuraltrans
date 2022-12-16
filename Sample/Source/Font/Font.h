#pragma once
#include "Source/DirectX/DirectX.h"

class Font
{
private:
	std::unique_ptr<SpriteBatch> Sprite_Batch;
	std::unique_ptr<SpriteFont> Sprite_Font;
public:
	void Init(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* fontfile);
	void DrawFont(const wchar_t* str, XMFLOAT2 pos, XMVECTOR color, float rotation, float scale, ID3D11DepthStencilState* depthstate);
};