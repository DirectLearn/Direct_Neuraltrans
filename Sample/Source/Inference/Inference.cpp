#include "Inference.h"
Text input;

void Inference::Init(ID3D11Device* device,ID3D11DeviceContext* context)
{
	//シェーダー作成
	ID3DBlob* pCompileVS = NULL;
	ID3DBlob* pCompilePS = NULL;

	//バーテックスシェーダー作成
	D3DCompileFromFile(L"Shader/Inference/InferenceTexture.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	assert(pCompileVS && "pCompileVS is nullptr");
	(device->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, &pVertexShader));
	//ピクセルシェーダー作成
	D3DCompileFromFile(L"Shader/Inference/InferenceTexture.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	assert(pCompilePS && "pCompilePS is nullptr");
	(device->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &pPixelShader));


	//頂点レイアウト
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXUV",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	(device->CreateInputLayout(layout, ARRAYSIZE(layout), pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), &pVertexLayout));
	pCompileVS->Release();
	pCompilePS->Release();

	//定数バッファの作成
	D3D11_BUFFER_DESC cb;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = sizeof(Constant_Buffer);
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;
	(device->CreateBuffer(&cb, NULL, &pConstantBuffer));

	//テクスチャ作成
	CreateTexture(device,context);


}




void Inference::CreateTexture(ID3D11Device* device,ID3D11DeviceContext* context)
{
	//バーテックスバッファの作成
	SquareVertex vertices[] =
	{
		{{-0.5f,0.5f,0.0f},{1.0f,1.0f,1.0f,1.0f},{0.0f,0.0f}},
		{{0.5f,0.5f,0.0f},{1.0f,1.0f,1.0f,1.0f},{1.0f,0.0f}},
		{{-0.5f,-0.5f,0.0f},{1.0f,1.0f,1.0f,1.0f},{0.0f,1.0f}},
		{{0.5f,-0.5f,0.0f},{1.0f,1.0f,1.0f,1.0f},{1.0f,1.0f}}
	};
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(SquareVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA sub = {};
	sub.pSysMem = vertices;
	device->CreateBuffer(&bd, &sub, &pVertexBuffer);

	
	//インデックスバッファの作成
	WORD indices[] =
	{
		0,1,2,
		2,1,3,
	};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	sub.pSysMem = indices;
	device->CreateBuffer(&bd, &sub, &pIndexBuffer);
	

	//テクスチャの作成
	D3D11_TEXTURE2D_DESC td;
	td.Width = PixelSize;
	td.Height = PixelSize;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DYNAMIC;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	td.MiscFlags = 0;
	device->CreateTexture2D(&td, nullptr, &pTexture);
	
	D3D11_MAPPED_SUBRESOURCE msr;
	context->Map(pTexture.Get(),0,D3D11_MAP_WRITE_DISCARD,0,&msr);

	//乱数生成
	std::random_device seed;
	std::mt19937 random(seed());
	std::uniform_int_distribution<> number(0, 0);//0～255までのランダムな数

	
	for (int i = 0; i < PixelSize * PixelSize * 4; i += 4)
	{
		srcData[i] =  number(random);//Red
		srcData[i + 1] =  number(random);//Green
		srcData[i + 2] =  number(random);//Blue
	}
	//memcpy_s(msr.pData,msr.RowPitch,(void*)(&srcData),sizeof(srcData));
	memcpy(msr.pData, srcData, sizeof(srcData));
	context->Unmap(pTexture.Get(), 0);

	//ラスタライザの作成
	D3D11_RASTERIZER_DESC ras = {};
	ras.FillMode = D3D11_FILL_SOLID;
	ras.CullMode = D3D11_CULL_NONE;
	device->CreateRasterizerState(&ras, &pRasterizer);

	//シェーダーリソースビューの作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srv = {};
	srv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(pTexture.Get(), &srv, &pShaderResourceView);



}



void Inference::Draw(ID3D11DeviceContext* context,SimpleMath::Matrix& world, SimpleMath::Matrix& view, SimpleMath::Matrix& proj,SimpleMath::Matrix& ScreenMat,HWND h_Wnd,int Height,int Width)
{
	UINT stride = sizeof(SquareVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetInputLayout(pVertexLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	context->PSSetShaderResources(0, 1, pShaderResourceView.GetAddressOf());
	context->RSSetState(pRasterizer.Get());

	//テクスチャ書き換え
	D3D11_MAPPED_SUBRESOURCE msr;


	
	//乱数生成
	if (GetKeyState(VK_LBUTTON) & 0x80)
	{
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(h_Wnd, &pt);
		SimpleMath::Vector3 screenpos;
		screenpos.x = pt.x;
		screenpos.y = pt.y;
		screenpos.z = 1.0;
		SimpleMath::Vector3 transscreen = XMVector3Transform(screenpos, ScreenMat);

	
			std::random_device seed;
			std::mt19937 random(seed());
			std::uniform_int_distribution<> number(255, 255);//白

			if (transscreen.x / (Width/2) >= 0 && (-transscreen.y / (Height/2)) >= 0 && transscreen.x / (Width/2) <= Width && (-transscreen.y / (Height/2)) <= Height)
			{
				int basepos1 = (4 * (PixelSize-1) * (1 + 2 * (int)((-transscreen.y / (Height / 2)) / (int)(Height / (PixelSize-1))))) + (4 * 2* (int)((-transscreen.y / (Height/2)) / (int)(Height/(PixelSize-1))));
				int basepos2 = (4 * (PixelSize-1) * (1 + 2 * (int)((-transscreen.y / (Height / 2)) / (int)(Height / (PixelSize-1))))) + (4 * 2* (int)((-transscreen.y / (Height/2)) / (int)(Height/(PixelSize-1))))+1;
				int basepos3 = (4 * (PixelSize-1) * (1 + 2 * (int)((-transscreen.y / (Height / 2)) / (int)(Height / (PixelSize-1))))) + (4 * 2* (int)((-transscreen.y / (Height/2)) / (int)(Height/(PixelSize-1))))+2;

				int cursorpos1 = ((int)((transscreen.x / (Width/2)) / (int)(Width/(PixelSize-1))) + ((int)((-transscreen.y / (Height/2)) / (int)(Height/(PixelSize-1))) * (PixelSize))) * 4;
				int cursorpos2 = ((int)((transscreen.x / (Width/2)) / (int)(Width/(PixelSize-1))) + ((int)((-transscreen.y / (Height/2)) / (int)(Height/(PixelSize-1))) * (PixelSize))) * 4;
				int cursorpos3 = ((int)((transscreen.x / (Width/2)) / (int)(Width/(PixelSize-1))) + ((int)((-transscreen.y / (Height/2)) / (int)(Height/(PixelSize-1))) * (PixelSize))) * 4;
				
		
				srcData[basepos1 - cursorpos1 -1] = number(random);//Red
				srcData[basepos2 - cursorpos2 -1] = number(random);//Green
				srcData[basepos3 - cursorpos3 -1] = number(random);//Blue
				srcData[basepos1 - cursorpos1] = number(random);//Red
				srcData[basepos2 - cursorpos2] = number(random);//Green
				srcData[basepos3 - cursorpos3] = number(random);//Blue
				srcData[basepos1 - cursorpos1+1] = number(random);//Red
				srcData[basepos2 - cursorpos2+1] = number(random);//Green
				srcData[basepos3 - cursorpos3+1] = number(random);//Blue

			}
			//memcpy_s(msr.pData,msr.RowPitch,(void*)(&srcData),sizeof(srcData));
	}

	input.Update_Key();

	if (input.PushBackSpace())
	{
		//乱数生成
		std::random_device seed;
		std::mt19937 random(seed());
		std::uniform_int_distribution<> number(0, 0);//0～255までのランダムな数


		for (int i = 0; i < PixelSize * PixelSize * 4; i += 4)
		{
			srcData[i] = number(random);//Red
			srcData[i + 1] = number(random);//Green
			srcData[i + 2] = number(random);//Blue
		}
	}

	context->Map(pTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy(msr.pData, srcData, sizeof(srcData));
	context->Unmap(pTexture.Get(), 0);


	//頂点の書き換え
	context->Map(pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	SquareVertex vertices[] =
	{
		{{-0.5f,0.5f,0.0f},{1.0f,1.0f,1.0f,1.0f},{0.0f,0.0f}},
		{{0.5f,0.5f,0.0f},{1.0f,1.0f,1.0f,1.0f},{1.0f,0.0f}},
		{{-0.5f,-0.5f,0.0f},{1.0f,1.0f,1.0f,1.0f},{0.0f,1.0f}},
		{{0.5f,-0.5f,0.0f},{1.0f,1.0f,1.0f,1.0f},{1.0f,1.0f}}
	};
	memcpy(msr.pData, vertices, sizeof(vertices));
	context->Unmap(pVertexBuffer.Get(), 0);


	Constant_Buffer cb;
	XMStoreFloat4x4(&cb.world, XMMatrixTranspose(world));
	XMStoreFloat4x4(&cb.view, XMMatrixTranspose(view));
	XMStoreFloat4x4(&cb.proj, XMMatrixTranspose(proj));


	context->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy_s(msr.pData, msr.RowPitch, (void*)(&cb), sizeof(cb));
	context->Unmap(pConstantBuffer, 0);

	//描画
	context->DrawIndexed(6, 0,0);
}

/*
static void DeallocateBuffer(void* data, size_t)
{
	std::free(data);
}

TF_Buffer* Inference::ReadBufferFromFile(const char* file)
{
	const auto f = fopen_s(&rfp,file, "rb");
	if (rfp == nullptr)
	{
		return nullptr;
	}

	std::fseek(rfp, 0, SEEK_END);
	const auto fsize = ftell(rfp);
	std::fseek(rfp, 0, SEEK_SET);

	if (fsize < 1)
	{
		std::fclose(rfp);
		return nullptr;
	}

	const auto data = std::malloc(fsize);
	std::fread(data, fsize, 1, rfp);
	std::fclose(rfp);

	TF_Buffer* buf = TF_NewBuffer();
	buf->data = data;
	buf->length = fsize;
	buf->data_deallocator = DeallocateBuffer;

	return buf;
}
*/

TF_Tensor* Inference::CreateTensor(
	TF_DataType data_type,
	const std::int64_t* dims,
	std::size_t num_dims,
	const void* data,
	std::size_t len)
{
	if (dims == nullptr || data == nullptr)
	{
		return nullptr;
	}

	TF_Tensor* tensor = TF_AllocateTensor(data_type, dims, static_cast<int>(num_dims), len);
	if (tensor == nullptr)
	{
		return nullptr;
	}

	void* tensor_data = TF_TensorData(tensor);
	if (tensor_data == nullptr)
	{
		return nullptr;
	}

	std::memcpy(tensor_data, data, std::min(len, TF_TensorByteSize(tensor)));

	return tensor;
}

int maxIndex(float nums[], int n) {
	float max_value; /* 最大値 */
	int max_index; /* 最大値を持つ要素の添字 */
	int i;

	/* nums[0]を最大値と仮定する */
	max_value = nums[0];

	/* 現状の最大値の存在する要素に合わせて添字も設定 */
	max_index = 0;

	for (i = 0; i < n; i++) {
		if (nums[i] > max_value) {
			/* 最大値よりもnums[i]の方が大きければ最大値を更新 */
			max_value = nums[i];

			/* 最大値の存在する要素に合わせて添字も更新 */
			max_index = i;
		}
	}

	/* 最大値の存在する要素の添字を返却 */
	return max_index;
}

void Inference::displayGraphInfo(const char* file)
{
	if (file == nullptr)
	{
		assert("file is null");
	}

	/*
	TF_Buffer* buffer = ReadBufferFromFile(file);
	if (buffer == nullptr)
	{
		assert("buffer is null");
	}
	*/

	graph = TF_NewGraph();
	status = TF_NewStatus();
	SessionOpts = TF_NewSessionOptions();
	RunOpts = NULL;
	std::array<char const*, 1> tags{ "serve" };
	const char* saved_model_dir = "Data/CNN/data/saved_model";

	Session = TF_LoadSessionFromSavedModel(SessionOpts, RunOpts, saved_model_dir, tags.data(), tags.size(), graph, NULL, status);



	//TF_ImportGraphDefOptions* opts = TF_NewImportGraphDefOptions();

	//TF_GraphImportGraphDef(graph, buffer, opts, status);
	//TF_DeleteImportGraphDefOptions(opts);
	//TF_DeleteBuffer(buffer);

	if (TF_GetCode(status) != TF_OK)
	{
		TF_DeleteGraph(graph);
		graph = nullptr;
	}

	//TF_DeleteStatus(status);

}


int Inference::ExecuteInference(ID3D11DeviceContext* context, ID3D11Device* device,ID3D11DepthStencilState* depthstate)
{


	if (input.PushReturn())
	{
		//ExecuteCS(context, pComputeShader.Get(), pShaderResourceView.Get(),sampler, pResultBufUAV.Get(), 32, 32, 1);
		//context->CopyResource(pCopyBuf.Get(), pResultBuf.Get());
		//D3D11_MAPPED_SUBRESOURCE MappedResource;
		//context->Map(pCopyBuf.Get(), 0, D3D11_MAP_READ, 0, &MappedResource);
		//Structured_Buffer* p = reinterpret_cast<Structured_Buffer*>(MappedResource.pData);

		fopen_s(&wfp, "Inferencetexture.txt", "w");
		//const int* pixeldata = reinterpret_cast<const int*>(&srcData);
		//コンピュートシェーダーの結果をファイルに出力
		for (int i = 0; i < PixelSize * PixelSize * 4; i += 4)
		{
			if (i % 128 == 0)
			{
				fputs("\n", wfp);
			}
			//TexSource.push_back(srcData[i]);
			fprintf(wfp, "%d ", srcData[i]);
		}
		fclose(wfp);

		int pixCount = PixelSize;
		fopen_s(&rfp, "Inferencetexture.txt", "r");
		while (!feof(rfp))
		{

			if (fscanf_s(rfp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
				&primal_pixel[pixCount - 1], &primal_pixel[pixCount - 2], &primal_pixel[pixCount - 3], &primal_pixel[pixCount - 4], &primal_pixel[pixCount - 5], &primal_pixel[pixCount - 6]
				, &primal_pixel[pixCount - 7], &primal_pixel[pixCount - 8], &primal_pixel[pixCount - 9], &primal_pixel[pixCount - 10], &primal_pixel[pixCount - 11], &primal_pixel[pixCount - 12],
				&primal_pixel[pixCount - 13], &primal_pixel[pixCount - 14], &primal_pixel[pixCount - 15], &primal_pixel[pixCount - 16], &primal_pixel[pixCount - 17], &primal_pixel[pixCount - 18],
				&primal_pixel[pixCount - 19], &primal_pixel[pixCount - 20], &primal_pixel[pixCount - 21], &primal_pixel[pixCount - 22], &primal_pixel[pixCount - 23], &primal_pixel[pixCount - 24],
				&primal_pixel[pixCount - 25], &primal_pixel[pixCount - 26], &primal_pixel[pixCount - 27], &primal_pixel[pixCount - 28], &primal_pixel[pixCount - 29], &primal_pixel[pixCount - 30],
				&primal_pixel[pixCount - 31], &primal_pixel[pixCount - 32]) == PixelSize)
			{
				pixCount += PixelSize;
			}

		}
		fclose(rfp);


		//出力ファイル設定
			//エラーハンドルにデフォルト値を設定
		imginfo.err = jpeg_std_error(&jerr);

		//jpegオブジェクトの初期化
		jpeg_create_compress(&imginfo);

		const char* filename = "Data/CNN/output.jpg";
		FILE* fp;
		fopen_s(&fp,filename, "wb");
		if (fp == NULL)
		{
			fprintf(stderr, "cannot open %s\n", filename);
		}
		jpeg_stdio_dest(&imginfo, fp);

		//画像のパラメータの設定
		imginfo.image_width = PixelSize;
		imginfo.image_height = PixelSize;
		imginfo.input_components = 3;
		imginfo.in_color_space = JCS_RGB;
		jpeg_set_defaults(&imginfo);
		jpeg_set_quality(&imginfo, 75, TRUE);

		//圧縮開始
		jpeg_start_compress(&imginfo, TRUE);

		//RGB値の設定
		JSAMPARRAY img = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * PixelSize);
		for (int i = 0; i < PixelSize; i++)
		{
			img[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * 3 * PixelSize);
			for (int j = 0; j < PixelSize; j++)
			{
				img[i][j * 3 + 0] = primal_pixel[(i * PixelSize) + j];
				img[i][j * 3 + 1] = primal_pixel[(i * PixelSize) + j];
				img[i][j * 3 + 2] = primal_pixel[(i * PixelSize) + j];
			}
		}

		//書き込む
		jpeg_write_scanlines(&imginfo, img, PixelSize);

		//圧縮完了
		jpeg_finish_compress(&imginfo);

		//jpegオブジェクトの破棄
		jpeg_destroy_compress(&imginfo);

		for (int i = 0; i < PixelSize; i++)
		{
			free(img[i]);
		}
		free(img);
		fclose(fp);

		cv::Mat image = cv::imread("Data/CNN/output.jpg");
		cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
		cv::resize(image, image, cv::Size(32, 32));
		image = ~image;
		image.convertTo(image, CV_32FC1, 1.0 / 255);

		displayGraphInfo("Data/CNN/train/keras_metadata.pb");

		//input_tensor
		input_op = { TF_GraphOperationByName(graph,"serving_default_conv2d_input"),0 };
		if (input_op.oper == nullptr)
		{
			assert("Can't init input_op");
		}

		//output_tensor
		out_op = { TF_GraphOperationByName(graph,"StatefulPartitionedCall"),0 };
		if (out_op.oper == nullptr)
		{
			assert("Can't init output_op");
		}

		const std::vector<std::int64_t> input_dims = { 1,32,32,1 };
		std::vector<float> input_vals;
		image.reshape(0, 1).copyTo(input_vals);

		TF_Tensor* input_tensor = CreateTensor(TF_FLOAT,
			input_dims.data(), input_dims.size(),
			input_vals.data(), input_vals.size() * sizeof(float));



		TF_Tensor* output_tensor = nullptr;

		//run session
		TF_SessionRun(
			Session,
			nullptr,
			&input_op,
			&input_tensor,
			1,
			&out_op,
			&output_tensor,
			1,
			nullptr,
			0,
			nullptr,
			status
		);

		if (TF_GetCode(status) != TF_OK)
		{
			assert("Error run session");
		}

		
		TF_CloseSession(Session, status);
		if (TF_GetCode(status) != TF_OK)
		{
			assert("Error close session");
		}

		TF_DeleteSession(Session, status);
		if (TF_GetCode(status) != TF_OK)
		{
			assert("Error delete session");
		}


		const auto probs = static_cast<float*>(TF_TensorData(output_tensor));

		fopen_s(&wfp, "inferenceResult.txt", "w");
		for (int i = 0; i < 10; i++)
		{
			fprintf(wfp, "%f\n", probs[i]);
			maxindex[i] = probs[i];
		}
		fclose(wfp);

		returnvalue = maxIndex(maxindex, 10);
		/*
		size_t pos = 0;
		TF_Operation* oper;
		fopen_s(&wfp, "graphdata.txt", "w");
		while ((oper = TF_GraphNextOperation(graph, &pos)) != nullptr)
		{
			fprintf_s(wfp, "%s\n", TF_OperationName(oper));
		}
		fclose(wfp);
		*/

		//TF_DeleteSession(Session, status);
		TF_DeleteGraph(graph);
		TF_DeleteTensor(input_tensor);
		TF_DeleteTensor(output_tensor);
		TF_DeleteBuffer(RunOpts);
		TF_DeleteSessionOptions(SessionOpts);
		TF_DeleteStatus(status);

	
		//fopen_s(&rfp, "Data/CNN/returninference.txt", "r");
		//fscanf_s(rfp,"%d",&returnvalue);
		//fclose(rfp);
		//cv::waitKey(0);
	}
	return returnvalue;
}