#pragma once
//#define PY_SSIZE_T_CLEAN

//Direct3Dのライブラリを使用できるようにする
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

//Direct3Dの型・クラス・関数などを呼べるようにする
//#include<Python.h>
#include <d3d11.h>
#include <d3dcompiler.h>

//DirectXMath(数学ライブラリ)を使用できるようにする
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <random>
#define DX_PI 3.14159265358979323846
#define DegreeToRadian(degree)((degree) * (DX_PI / 180.0))
//DirectXテクスチャライブラリを使用できるようにする
#include "DirectXTex.h"

#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

//ComPtrを使用できるようにする
#include<wrl/client.h>
#include<map>
#include<fstream>
#include<iostream>
#include<string>
//#include<stdio.h>
//#include<errno.h>
//#include<limits.h>
//#include<assert.h>
//#include<stdlib.h>
using Microsoft::WRL::ComPtr;
using namespace DirectX;

//SpriteBatchとSpriteFontをインクルード
#include<SpriteBatch.h>
#include<SpriteFont.h>
#include<CommonStates.h>
#include<jpeglib.h>
#include<opencv2/opencv.hpp>
#include<tensorflow/c/c_api.h>

//#include "tensorflow/core/public/session.h"

//自作のDirectX関係のヘッダーをインクルード
#include "Directx3D.h"
#include "Texture.h"
#include "Source/FBX/FbxImporter.h"
#include "Source/SkyDome/SkyDome.h"
#include "Source/Fire/Fire.h"
#include "Source/Font/Font.h"
#include "Source/Sprite/Sprite.h"
#include "Source/Line/Line.h"
#include "Source/OBJ/objectImporter.h"
#include "Source/OBJ/binaryobject.h"
#include "Source/Text/input_text.h"
#include "Source/Inference/Inference.h"
#include "Source/Scene/Battle/Battle.h"
#include "Source/Neuraltrans/Neuraltrans.h"
#include "Source/Depth_Estimation/Depth_Estimation.h"