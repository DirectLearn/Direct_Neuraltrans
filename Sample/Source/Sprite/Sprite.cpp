#include "Sprite.h"

void Sprite::Init(const wchar_t* filepath, ID3D11DeviceContext* context,ID3D11Device* device)
{
	m_spriteBatch = std::make_unique<SpriteBatch>(context);
	m_states = std::make_unique<CommonStates>(device);

	CreateWICTextureFromFile(device,filepath,resource.GetAddressOf(),m_texture.ReleaseAndGetAddressOf());

	resource.As(&sprite);

	CD3D11_TEXTURE2D_DESC sprite_desc;
	sprite->GetDesc(&sprite_desc);

	m_origin.x = float(sprite_desc.Width / 2);
	m_origin.y = float(sprite_desc.Height / 2);
}

void Sprite::DrawSprite(XMFLOAT2 pos,ID3D11DepthStencilState* depthstate,float scale)
{
	m_spriteBatch->Begin(SpriteSortMode_Deferred,m_states->NonPremultiplied(),NULL,depthstate);
	m_spriteBatch->Draw(m_texture.Get(), pos, nullptr, Colors::White, 0.f, m_origin,scale, SpriteEffects_None,0.1);
	m_spriteBatch->End();
}