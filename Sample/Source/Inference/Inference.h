#pragma once
#include "Source/DirectX/DirectX.h"
#include <cstddef>
#include<cstdint>
#define PixelSize 32
#define FilterSize 3
#define PoolingSize 2
#define Num_Filter 10
#define PoolingPixel 16
#define MiddNeuron 100
#define OutNeuron 10
#pragma comment(lib, "jpeg.lib")
#pragma comment(lib,"opencv_world460.lib")
//#pragma comment(lib,"opencv_world460d.lib")
#pragma comment(lib,"tensorflow.lib")

class Inference
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
		XMFLOAT4 color;
		XMFLOAT2 uv;
	};



	//-------テクスチャ表示用変数--------

	ComPtr<ID3D11VertexShader> pVertexShader;
	ComPtr<ID3D11PixelShader> pPixelShader;
	ComPtr<ID3D11InputLayout> pVertexLayout;
	ComPtr<ID3D11Buffer> pVertexBuffer;
	ComPtr<ID3D11Buffer> pIndexBuffer;
	ID3D11Buffer* pConstantBuffer;
	ComPtr<ID3D11Texture2D> pTexture;
	ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
	ComPtr<ID3D11RasterizerState> pRasterizer;
	byte srcData[PixelSize * PixelSize * 4] = { 0 };
	int primal_pixel[PixelSize * PixelSize];

	FILE* wfp = NULL;
	FILE* rfp = NULL;

	//テクスチャを画像ファイルに変換するための変数
	struct jpeg_compress_struct imginfo;
	struct jpeg_error_mgr jerr;

	bool backflag = false;
	int returnvalue;

	//推論のための変数
	TF_Graph* graph;
	TF_Status* status;
	TF_SessionOptions* SessionOpts;
	TF_Buffer* RunOpts;
	TF_Session* Session;
	TF_Output input_op;
	TF_Output out_op;
	float maxindex[10];

public:
	void Init(ID3D11Device* device,ID3D11DeviceContext* context);
	void CreateTexture(ID3D11Device* device,ID3D11DeviceContext* context);
	void CreateEmptyTexture(ID3D11Device* device, UINT xSize, UINT ySize, UINT zSize,ID3D11Texture3D **ppTexture,ID3D11ShaderResourceView** ppSRV);
	void CreateFilterSRV(ID3D11Device* device,UINT BufferSize,ID3D11Buffer** ppfilter,ID3D11UnorderedAccessView** ppfilterUAV,ID3D11UnorderedAccessView** ppTexture);
	void CreateBiasSRV(ID3D11Device* device, UINT BufferSize, ID3D11Buffer** ppbias);
	void CreateIm2colSRV(ID3D11Device* device, UINT BufferSize, ID3D11Buffer** ppIm2col);
	void CreatePoolSRV(ID3D11Device* device, UINT BufferSize, ID3D11Buffer** ppPool);
	void CreateMiddleWeightSRV(ID3D11Device* device, UINT BufferSize, ID3D11Buffer** ppmidweight);
	void CreateMiddleBiasSRV(ID3D11Device* device, UINT BufferSize, ID3D11Buffer** ppmidbias);
	void CreateOutInputSRV(ID3D11Device* device, UINT BufferSize, ID3D11Buffer** ppoutin);
	void CreateOutWeightSRV(ID3D11Device* device, UINT BufferSize, ID3D11Buffer** ppoutweight);
	void CreateOutBiasSRV(ID3D11Device* device, UINT BufferSize, ID3D11Buffer** ppoutbias);
	void CreateConvolutionOutUAV(ID3D11Device* device, UINT BufferSize, ID3D11Buffer** pResult, ID3D11UnorderedAccessView** ppUAVOut);
	void CreateMiddleLayerOutUAV(ID3D11Device* device, UINT BufferSize, ID3D11Buffer** pResult, ID3D11UnorderedAccessView** ppUAVOut);
	void CreateOutLayerOutUAV(ID3D11Device* device, UINT BufferSize, ID3D11Buffer** pResult, ID3D11UnorderedAccessView** ppUAVOut);
	void Draw(ID3D11DeviceContext* context,SimpleMath::Matrix& world,SimpleMath::Matrix& view,SimpleMath::Matrix& proj,SimpleMath::Matrix& ScreenMat,HWND h_Wnd,int Height,int Width);
	void displayGraphInfo(const char* file);
	//TF_Buffer* ReadBufferFromFile(const char* file);
	TF_Tensor* CreateTensor(TF_DataType data_type, const std::int64_t* dims, std::size_t num_dims, const void* data, std::size_t len);
	int ExecuteInference(ID3D11DeviceContext* context,ID3D11Device* device,ID3D11DepthStencilState* depthstate);
	ID3D11VertexShader* SetVertexShader() { return pVertexShader.Get(); };
	ID3D11PixelShader* SetPixelShader() { return pPixelShader.Get(); };
};