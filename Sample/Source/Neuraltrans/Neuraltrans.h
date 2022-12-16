#pragma once
#include "Source/DirectX/DirectX.h"
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>


class Neuraltrans
{
private:
	struct Constant_Buffer
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 proj;
	};

	struct SquareVertex
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};

	//-------テクスチャ用変数-------
	ComPtr<ID3D11VertexShader> pVertexShader;
	ComPtr<ID3D11PixelShader> pPixelShader;
	ComPtr<ID3D11InputLayout> pVertexLayout;
	ComPtr<ID3D11Buffer> pVertexBuffer;
	ComPtr<ID3D11Buffer> pIndexBuffer;
	ComPtr<ID3D11Texture2D> pTexture;
	ComPtr<ID3D11ShaderResourceView> pShaderResouceView;
	ID3D11Buffer* pConstantBuffer;
	byte srcData[1280 * 720 * 4] = { 0 };
	cv::dnn::Net net;
	cv::Mat blob;
	cv::Mat convertImg;
	cv::Mat img = cv::Mat::ones(720,1280,CV_8UC4);
	cv::Mat img2 = cv::Mat::ones(720, 1280, CV_8UC3);
	cv::Mat copyimg = cv::Mat::ones(720, 1280, CV_8UC4);
	const cv::Scalar mean = cv::Scalar(103.939, 116.779, 123.680);

	ID3D11Resource* hResource;
	int const BytePerPixel = sizeof(byte);
	int size = 4;
public:
	Neuraltrans();
	void Init(ID3D11Device* device,int width,int height);
	void DLtorchModel(ID3D11Device* device,ID3D11DeviceContext* context, IDXGISwapChain* swapchain,ID3D11Texture2D* inputStagingTex,int width,int height);
	void Draw(ID3D11DeviceContext* context,ID3D11SamplerState* samp ,SimpleMath::Matrix& world, SimpleMath::Matrix& view, SimpleMath::Matrix& proj,int width,int height);
	~Neuraltrans();
};