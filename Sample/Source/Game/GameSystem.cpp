#include "framework.h"
//GameSystem�N���X���g����悤�ɂ���
#include "GameSystem.h"
//DirectX�N���X���g����悤�ɂ���
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

//�Q�[���̏����ݒ���s��
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

//���̃Q�[���̎��Ԃ�i�߂�(���������s����)
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