#include "Font.h"

void Font::Init(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* fontfile)
{
	Sprite_Batch = std::make_unique<SpriteBatch>(context);
	Sprite_Font = std::make_unique<SpriteFont>(device, fontfile);
}

void Font::DrawFont(const wchar_t* str, XMFLOAT2 pos, XMVECTOR color, float rotation, float scale, ID3D11DepthStencilState* depthstate)
{
	Sprite_Batch->Begin(DirectX::SpriteSortMode_Deferred,NULL, NULL, depthstate);
	Sprite_Font->DrawString(Sprite_Batch.get(), str, pos, color, rotation, XMFLOAT2(0, 0), scale, SpriteEffects_None, 0.0);
	Sprite_Batch->End();
}
