#include "framework.h"
//GameSystemクラスを使えるようにする
#include "GameSystem.h"
//DirectXクラスを使えるようにする
#include "Source/DirectX/DirectX.h"



//Texture m_tex;
//Texture m_tex2;
//FBX_Model fbxmodel;
//FBX_Model ruby;
//FBX_Model fbxmodel2;
//OBJ building;
//BinaryOBJ map;
//SkyDome skydome;
//Line line;
//Fire fire;
//Font font;
//Sprite sprite;
//Sprite sprite_ruby_a;
//Sprite sprite_ruby_b;
//Text text;
Battle battle;

//ゲームの初期設定を行う
void GameSystem::Initialize()
{
	battle.Init(
		D3D.m_device.Get(), 
		D3D.m_deviceContext.Get(), 
		D3D.m_backBufferView.Get(), 
		D3D.m_pBackBuffer_DSTexDSV.Get(),
		D3D.ScreenWidth,
		D3D.ScreenHeight);
}

//このゲームの時間を進める(処理を実行する)
void GameSystem::Execute()
{
	battle.Draw(
		D3D.ScreenHeight, 
		D3D.ScreenWidth, 
		D3D.m_deviceContext.Get(),
		D3D.m_device.Get(), 
		D3D.m_backBufferView.Get(), 
		D3D.m_pNormal_RTV.Get(),
		D3D.pBackBuffer.Get(),
		D3D.m_pStaging_Tex.Get(), 
		D3D.m_pBackBuffer_DSTexDSV.Get(), 
		D3D.m_pDepthStencilState.Get(),
		D3D.SamplerState.Get(), 
		D3D.h_Wnd,
		D3D.m_swapChain.Get());
}