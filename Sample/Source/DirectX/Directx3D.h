#pragma once

#include "DirectX.h"

class Texture;
class FBX_Model;

//2D用頂点構造体
struct VertexType2D
{
	DirectX::XMFLOAT3 Pos;	//座標
	DirectX::XMFLOAT2 UV;	//UV座標
};

struct ConstantBuffer
{
	XMFLOAT4X4 m_W;
	XMFLOAT4X4 m_V;
	XMFLOAT4X4 m_P;
};

//------------------------------
//Direct3Dクラス
//------------------------------
class Direct3D {
public:
	//変数は本来はprivateで隠すべき

	//Direct3Dデバイス
	ComPtr<ID3D11Device> m_device;
	//Direct3Dデバイスコンテキスト
	ComPtr<ID3D11DeviceContext> m_deviceContext;
	//スワップチェイン
	ComPtr<IDXGISwapChain> m_swapChain;
	//バックバッファのRTビュー
	ComPtr<ID3D11RenderTargetView> m_backBufferView;
	//深度マップテクスチャ用レンダーターゲット
	ComPtr<ID3D11RenderTargetView> m_pDepthMap_RTV;
	//SSAOレンダーターゲット
	ComPtr<ID3D11RenderTargetView> m_pSSAO_RTV;
	//通常描画用レンダーターゲット
	ComPtr<ID3D11RenderTargetView> m_pNormal_RTV;
	//ラスタライザステート
	ComPtr<ID3D11RasterizerState> m_pRasterizerState;
	//ブレンドステート
	ComPtr<ID3D11BlendState> BlendState;
	//レンダーターゲット・デプスステンシルビューに設定するテクスチャ
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
	//深度マップ用テクスチャーSRV
	ComPtr<ID3D11ShaderResourceView> m_pDepthMap_TexSRV;
	//SSAO用SRV
	ComPtr<ID3D11ShaderResourceView> m_pSSAO_TexSRV;
	//通常描画用テクスチャーSRV
	ComPtr<ID3D11ShaderResourceView> m_pNormalTexSRV;
	//サンプラ
	ComPtr<ID3D11SamplerState> SamplerState;
	ComPtr<ID3D11SamplerState> ToonState;
	//レンダーターゲット格納用配列
	//std::vector<ID3D11RenderTargetView*> RenderTargetViews;
	//デプスステンシルビュー格納用配列
	//std::vector<ID3D11DepthStencilView*> DepthStencilViews;

	//-------------------------------------
	//Direct3Dを初期化し、使用できるようにする関数
	//hWnd	:ウインドウハンドル
	//width	:画面の幅
	//height:画面の高さ
	//----------------------------------------
	bool Initialize(HWND hWnd, int width, int height);

	int ScreenWidth;
	int ScreenHeight;
	int DepthWidth;
	int DepthHeight;
	HWND h_Wnd;

	//2D描画用のシェーダー
	ComPtr<ID3D11VertexShader> m_spriteVS = nullptr;
	ComPtr<ID3D11PixelShader> m_spritePS = nullptr;
	ComPtr<ID3D11InputLayout> m_spriteInputLayout = nullptr;

	ComPtr<ID3D11Buffer> m_vbSquare,m_cb;

	//2D描画モードにする
	void ChangeMode_2D();
	//2D描画
	//tex	:テクスチャ
	//x		:x座標
	//y		:y座標
	//w		:幅
	//h		:高さ
	void Draw2D(const Texture& tex, float x, float y, float w, float h);
	//-----------------------------------------
	//今回このクラスはどこからでもアクセスできるように
	//シングルパターンにする
	//以下、シングルパターンのコード
	//-------------------------------------------

	//fbxファイルからモデルを作成する
	void FbxCreate(const FBX_Model& fbx);

	//座標変換
	void UpdateWorldMatrix(XMMATRIX& World, XMMATRIX& View, XMMATRIX& Proj);

private:
	//唯一のインスタンス用のポインタ
	static inline Direct3D* s_instance;
	//コンストラクタはprivateにする
	Direct3D(){}
public:
	//インスタンス作成
	static void CreateInstance()
	{
		DeleteInstance();

		s_instance = new Direct3D();
	}
	//インスタンス削除
	static void DeleteInstance()
	{
		if (s_instance != nullptr)
		{
			delete s_instance;
			s_instance = nullptr;
		}
	}
	//唯一のインスタンスを取得
	static Direct3D& GetInstance()
	{
		return *s_instance;
	}
};

//Direct3Dの唯一のインスタンスを簡単に取得するためのマクロ
#define D3D Direct3D::GetInstance()