#pragma once
//#define PY_SSIZE_T_CLEAN

//Direct3D�̃��C�u�������g�p�ł���悤�ɂ���
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

//Direct3D�̌^�E�N���X�E�֐��Ȃǂ��Ăׂ�悤�ɂ���
//#include<Python.h>
#include <d3d11.h>
#include <d3dcompiler.h>

//DirectXMath(���w���C�u����)���g�p�ł���悤�ɂ���
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <random>
#define DX_PI 3.14159265358979323846
#define DegreeToRadian(degree)((degree) * (DX_PI / 180.0))
//DirectX�e�N�X�`�����C�u�������g�p�ł���悤�ɂ���
#include "DirectXTex.h"

#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

//ComPtr���g�p�ł���悤�ɂ���
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

//SpriteBatch��SpriteFont���C���N���[�h
#include<SpriteBatch.h>
#include<SpriteFont.h>
#include<CommonStates.h>
#include<jpeglib.h>
#include<opencv2/opencv.hpp>
#include<tensorflow/c/c_api.h>

//#include "tensorflow/core/public/session.h"

//�����DirectX�֌W�̃w�b�_�[���C���N���[�h
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