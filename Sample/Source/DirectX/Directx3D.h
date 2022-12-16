#pragma once

#include "DirectX.h"

class Texture;
class FBX_Model;

//2D�p���_�\����
struct VertexType2D
{
	DirectX::XMFLOAT3 Pos;	//���W
	DirectX::XMFLOAT2 UV;	//UV���W
};

struct ConstantBuffer
{
	XMFLOAT4X4 m_W;
	XMFLOAT4X4 m_V;
	XMFLOAT4X4 m_P;
};

//------------------------------
//Direct3D�N���X
//------------------------------
class Direct3D {
public:
	//�ϐ��͖{����private�ŉB���ׂ�

	//Direct3D�f�o�C�X
	ComPtr<ID3D11Device> m_device;
	//Direct3D�f�o�C�X�R���e�L�X�g
	ComPtr<ID3D11DeviceContext> m_deviceContext;
	//�X���b�v�`�F�C��
	ComPtr<IDXGISwapChain> m_swapChain;
	//�o�b�N�o�b�t�@��RT�r���[
	ComPtr<ID3D11RenderTargetView> m_backBufferView;
	//�[�x�}�b�v�e�N�X�`���p�����_�[�^�[�Q�b�g
	ComPtr<ID3D11RenderTargetView> m_pDepthMap_RTV;
	//SSAO�����_�[�^�[�Q�b�g
	ComPtr<ID3D11RenderTargetView> m_pSSAO_RTV;
	//�ʏ�`��p�����_�[�^�[�Q�b�g
	ComPtr<ID3D11RenderTargetView> m_pNormal_RTV;
	//���X�^���C�U�X�e�[�g
	ComPtr<ID3D11RasterizerState> m_pRasterizerState;
	//�u�����h�X�e�[�g
	ComPtr<ID3D11BlendState> BlendState;
	//�����_�[�^�[�Q�b�g�E�f�v�X�X�e���V���r���[�ɐݒ肷��e�N�X�`��
	ComPtr<ID3D11Texture2D> m_pDepthMap_Tex;
	ComPtr<ID3D11Texture2D> m_pDepthMap_DSTex;
	ComPtr<ID3D11Texture2D> m_pTexture2DDepthStencil;
	ComPtr<ID3D11Texture2D> m_pSSAO_Tex;
	ComPtr<ID3D11Texture2D> m_pSSAO_DSTex;
	ComPtr<ID3D11Texture2D> m_pNormal_Tex;
	ComPtr<ID3D11Texture2D> m_pStaging_Tex;
	ComPtr<ID3D11Texture2D> m_pNormal_DSTex;
	ComPtr<ID3D11Texture2D> pBackBuffer;
	ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
	ComPtr<ID3D11DepthStencilView> m_pBackBuffer_DSTexDSV;
	ComPtr<ID3D11DepthStencilView> m_pDepthMap_DSTexDSV;
	ComPtr<ID3D11DepthStencilView> m_pSSAO_DSTexDSV;
	ComPtr<ID3D11DepthStencilView> m_pNormal_DSTexDSV;
	//�[�x�}�b�v�p�e�N�X�`���[SRV
	ComPtr<ID3D11ShaderResourceView> m_pDepthMap_TexSRV;
	//SSAO�pSRV
	ComPtr<ID3D11ShaderResourceView> m_pSSAO_TexSRV;
	//�ʏ�`��p�e�N�X�`���[SRV
	ComPtr<ID3D11ShaderResourceView> m_pNormalTexSRV;
	//�T���v��
	ComPtr<ID3D11SamplerState> SamplerState;
	ComPtr<ID3D11SamplerState> ToonState;
	//�����_�[�^�[�Q�b�g�i�[�p�z��
	//std::vector<ID3D11RenderTargetView*> RenderTargetViews;
	//�f�v�X�X�e���V���r���[�i�[�p�z��
	//std::vector<ID3D11DepthStencilView*> DepthStencilViews;

	//-------------------------------------
	//Direct3D�����������A�g�p�ł���悤�ɂ���֐�
	//hWnd	:�E�C���h�E�n���h��
	//width	:��ʂ̕�
	//height:��ʂ̍���
	//----------------------------------------
	bool Initialize(HWND hWnd, int width, int height);

	int ScreenWidth;
	int ScreenHeight;
	int DepthWidth;
	int DepthHeight;
	HWND h_Wnd;

	//2D�`��p�̃V�F�[�_�[
	ComPtr<ID3D11VertexShader> m_spriteVS = nullptr;
	ComPtr<ID3D11PixelShader> m_spritePS = nullptr;
	ComPtr<ID3D11InputLayout> m_spriteInputLayout = nullptr;

	ComPtr<ID3D11Buffer> m_vbSquare,m_cb;

	//2D�`�惂�[�h�ɂ���
	void ChangeMode_2D();
	//2D�`��
	//tex	:�e�N�X�`��
	//x		:x���W
	//y		:y���W
	//w		:��
	//h		:����
	void Draw2D(const Texture& tex, float x, float y, float w, float h);
	//-----------------------------------------
	//���񂱂̃N���X�͂ǂ�����ł��A�N�Z�X�ł���悤��
	//�V���O���p�^�[���ɂ���
	//�ȉ��A�V���O���p�^�[���̃R�[�h
	//-------------------------------------------

	//fbx�t�@�C�����烂�f�����쐬����
	void FbxCreate(const FBX_Model& fbx);

	//���W�ϊ�
	void UpdateWorldMatrix(XMMATRIX& World, XMMATRIX& View, XMMATRIX& Proj);

private:
	//�B��̃C���X�^���X�p�̃|�C���^
	static inline Direct3D* s_instance;
	//�R���X�g���N�^��private�ɂ���
	Direct3D(){}
public:
	//�C���X�^���X�쐬
	static void CreateInstance()
	{
		DeleteInstance();

		s_instance = new Direct3D();
	}
	//�C���X�^���X�폜
	static void DeleteInstance()
	{
		if (s_instance != nullptr)
		{
			delete s_instance;
			s_instance = nullptr;
		}
	}
	//�B��̃C���X�^���X���擾
	static Direct3D& GetInstance()
	{
		return *s_instance;
	}
};

//Direct3D�̗B��̃C���X�^���X���ȒP�Ɏ擾���邽�߂̃}�N��
#define D3D Direct3D::GetInstance()