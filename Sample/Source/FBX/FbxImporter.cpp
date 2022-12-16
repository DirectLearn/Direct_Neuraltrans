#include "FbxImporter.h"

Texture toon;


FBX_Model::FBX_Model()
{
}

FBX_Model::~FBX_Model()
{
	Destroy();
}

XMMATRIX FBX_Model::ConvertMatrix(const FbxMatrix& fbxmat)
{
	XMFLOAT4X4 mat;
	XMMATRIX fmat;
	mat._11 = float(fbxmat[0][0]); mat._12 = float(fbxmat[1][0]); mat._13 = float(fbxmat[2][0]); mat._14 = float(fbxmat[3][0]);
	mat._21 = float(fbxmat[0][1]); mat._22 = float(fbxmat[1][1]); mat._23 = float(fbxmat[2][1]); mat._24 = float(fbxmat[3][1]);
	mat._31 = float(fbxmat[0][2]); mat._32 = float(fbxmat[1][2]); mat._33 = float(fbxmat[2][2]); mat._34 = float(fbxmat[3][2]);
	mat._41 = float(fbxmat[0][3]); mat._42 = float(fbxmat[1][3]); mat._43 = float(fbxmat[2][3]); mat._44 = float(fbxmat[3][3]);

	mat._11 = float(fbxmat[0][0]); mat._12 = float(fbxmat[0][1]); mat._13 = float(fbxmat[0][2]); mat._14 = float(fbxmat[0][3]);
	mat._21 = float(fbxmat[1][0]); mat._22 = float(fbxmat[1][1]); mat._23 = float(fbxmat[1][2]); mat._24 = float(fbxmat[1][3]);
	mat._31 = float(fbxmat[2][0]); mat._32 = float(fbxmat[2][1]); mat._33 = float(fbxmat[2][2]); mat._34 = float(fbxmat[2][3]);
	mat._41 = float(fbxmat[3][0]); mat._42 = float(fbxmat[3][1]); mat._43 = float(fbxmat[3][2]); mat._44 = float(fbxmat[3][3]);

	fmat = XMLoadFloat4x4(&mat);

	return fmat;
}

XMVECTOR FBX_Model::ConvertVector(const FbxVector4& fbxvec)
{
	XMFLOAT4 fvec;
	XMVECTOR vec;
	fvec.x = fbxvec[0];
	fvec.y = fbxvec[1];
	fvec.z = fbxvec[2];
	fvec.w = fbxvec[3];
	vec = XMLoadFloat4(&fvec);
	return vec;
}


void FBX_Model::Draw(ID3D11DeviceContext* context, DirectX::XMMATRIX& world,
	DirectX::XMMATRIX& view, DirectX::XMMATRIX& proj)
{
	
	// ----- Animation -----
	timeCount += FrameTime;
	if (timeCount > stop) timeCount = start;

	// �ړ��A��]�A�g��̂��߂̍s����쐬
	FbxNode* pNode = m_fbxScene->GetRootNode();
	FbxMatrix globalPosition = pNode->EvaluateGlobalTransform(timeCount);
	FbxVector4 t0 = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	FbxVector4 r0 = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	FbxVector4 s0 = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
	FbxAMatrix geometryOffset = FbxAMatrix(t0, r0, s0);

	for (int i = 0; i < m_fbxInfo.meshCount; i++)
	{

		// �e���_�Ɋ|���邽�߂̍ŏI�I�ȍs��̔z��
		FbxMatrix* clusterDeformation = new FbxMatrix[m_mesh->GetControlPointsCount()];
		memset(clusterDeformation, 0, sizeof(FbxMatrix) * m_mesh->GetControlPointsCount());

		FbxSkin* skinDeformer = (FbxSkin*)m_mesh->GetDeformer(0, FbxDeformer::eSkin);
		int clusterCount = skinDeformer->GetClusterCount();
		// �e�N���X�^����e���_�ɉe����^���邽�߂̍s��쐬
		for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
			// �N���X�^(�{�[��)�̎��o��
			FbxCluster* cluster = skinDeformer->GetCluster(clusterIndex);
			FbxMatrix vertexTransformMatrix;
			FbxAMatrix referenceGlobalInitPosition;
			FbxAMatrix clusterGlobalInitPosition;
			FbxMatrix clusterGlobalCurrentPosition;
			FbxMatrix clusterRelativeInitPosition;
			FbxMatrix clusterRelativeCurrentPositionInverse;
			cluster->GetTransformMatrix(referenceGlobalInitPosition);
			referenceGlobalInitPosition *= geometryOffset;
			cluster->GetTransformLinkMatrix(clusterGlobalInitPosition);
			clusterGlobalCurrentPosition = cluster->GetLink()->EvaluateGlobalTransform(timeCount);
			clusterRelativeInitPosition = clusterGlobalInitPosition.Inverse() * referenceGlobalInitPosition;
			clusterRelativeCurrentPositionInverse = globalPosition.Inverse() * clusterGlobalCurrentPosition;
			vertexTransformMatrix = clusterRelativeCurrentPositionInverse * clusterRelativeInitPosition;
			// ��ō�����s��Ɋe���_���̉e���x(�d��)���|���Ă��ꂼ��ɉ��Z
			for (int i = 0; i < cluster->GetControlPointIndicesCount(); i++) {
				int index = cluster->GetControlPointIndices()[i];
				double weight = cluster->GetControlPointWeights()[i];
				FbxMatrix influence = vertexTransformMatrix * weight;
				clusterDeformation[index] += influence;
			}
		}



		delete[] clusterDeformation;
	}
	// ---------------------


	D3D11_MAPPED_SUBRESOURCE pdata;
	CONSTANT_BUFFER cb;
	// �p�����[�^�̎󂯓n��(�萔)
	XMStoreFloat4x4(&cb.world,XMMatrixTranspose(world));
	XMStoreFloat4x4(&cb.view, XMMatrixTranspose(view));
	XMStoreFloat4x4(&cb.proj, XMMatrixTranspose(proj));
	context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
	memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), sizeof(cb));
	context->Unmap(m_constantBuffer.Get(), 0);


	// �`����s
	context->DrawIndexed(m_mesh->GetPolygonVertexCount(), 0, 0);
}

void FBX_Model::Draw(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix& world, DirectX::SimpleMath::Matrix& view, DirectX::SimpleMath::Matrix& proj,
	int shapindex,float bledparsent,float frame,int animationNumber)
{
	FbxArray<FbxString*> AnimStackNameArray;
	m_fbxScene->FillAnimStackNameArray(AnimStackNameArray);
	AnimStackNumber = animationNumber;
	FbxAnimStack* AnimationStack = m_fbxScene->FindMember<FbxAnimStack>(AnimStackNameArray[AnimStackNumber]->Buffer());
	m_fbxScene->SetCurrentAnimationStack(AnimationStack);



	Settoon(context, toon.m_srv.Get());

	// <�I�u�W�F�N�g�̔��f>
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	

	D3D11_MAPPED_SUBRESOURCE pdata;
	// ----- Animation -----
	FrameTime.SetTime(0, 0, 0, frame);


	for (int i = 0; i < m_fbxInfo.meshCount; i++)
	{

		// �e���_�Ɋ|���邽�߂̍ŏI�I�ȍs��̔z��
		FbxMatrix* clusterDeformation = new FbxMatrix[m_fbxInfo.meshes[i]->GetControlPointsCount()];
		memset(clusterDeformation, 0, sizeof(FbxMatrix) * m_fbxInfo.meshes[i]->GetControlPointsCount());
		XMMATRIX* Defomat = new XMMATRIX[m_fbxInfo.meshes[i]->GetControlPointsCount()];
		memset(Defomat, 0, sizeof(XMMATRIX) * m_fbxInfo.meshes[i]->GetControlPointsCount());

		// �ړ��A��]�A�g��̂��߂̍s����쐬
		FbxNode* pNode = m_fbxInfo.meshes[i]->GetNode();
		FbxMatrix globalPosition = pNode->EvaluateGlobalTransform(FrameTime);
		FbxVector4 t0 = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
		FbxVector4 r0 = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
		FbxVector4 s0 = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
		FbxAMatrix geometryOffset = FbxAMatrix(t0, r0, s0);

		FbxSkin* skinDeformer = (FbxSkin*)m_fbxInfo.meshes[i]->GetDeformer(0, FbxDeformer::eSkin);
		int clusterCount = skinDeformer->GetClusterCount();

		// �e�N���X�^����e���_�ɉe����^���邽�߂̍s��쐬
		for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
			// �N���X�^(�{�[��)�̎��o��
			FbxCluster* cluster = skinDeformer->GetCluster(clusterIndex);
			FbxMatrix vertexTransformMatrix;
			FbxAMatrix referenceGlobalInitPosition;
			FbxAMatrix clusterGlobalInitPosition;
			FbxMatrix clusterGlobalCurrentPosition;
			FbxMatrix clusterRelativeInitPosition;
			FbxMatrix clusterRelativeCurrentPositionInverse;
			FbxAMatrix geometryglobalInit;
			cluster->GetTransformMatrix(referenceGlobalInitPosition);
			geometryglobalInit = referenceGlobalInitPosition * geometryOffset;
			cluster->GetTransformLinkMatrix(clusterGlobalInitPosition);
			clusterGlobalCurrentPosition = cluster->GetLink()->EvaluateGlobalTransform(FrameTime);
			clusterRelativeInitPosition = clusterGlobalInitPosition.Inverse() * geometryglobalInit;
			clusterRelativeCurrentPositionInverse = globalPosition.Inverse() * clusterGlobalCurrentPosition;
			vertexTransformMatrix = clusterRelativeCurrentPositionInverse * clusterRelativeInitPosition;


			// ��ō�����s��Ɋe���_���̉e���x(�d��)���|���Ă��ꂼ��ɉ��Z
			for (int cnt = 0; cnt < cluster->GetControlPointIndicesCount(); cnt++) {
				int index = cluster->GetControlPointIndices()[cnt];
				double weight = cluster->GetControlPointWeights()[cnt];
				FbxMatrix influence = vertexTransformMatrix * weight;
				XMMATRIX infmat = ConvertMatrix(influence);

				Defomat[index] += infmat;
				clusterDeformation[index] += influence;
			}

		}
		// �ŏI�I�Ȓ��_���W���v�Z��VERTEX�ɕϊ�
		for (int cnt = 0; cnt < m_fbxInfo.meshes[i]->GetControlPointsCount(); cnt++) {
			FbxVector4 outVertex = clusterDeformation[cnt].MultNormalize(m_fbxInfo.meshes[i]->GetControlPointAt(cnt));
			//FbxVector4 fv = m_fbxInfo.meshes[i]->GetControlPointAt(cnt);
			XMVECTOR xv = ConvertVector(m_fbxInfo.meshes[i]->GetControlPointAt(cnt));
			XMVECTOR finalvec = XMVector3Transform(xv, Defomat[cnt]);
			XMFLOAT4 finalflo;
			XMStoreFloat4(&finalflo,finalvec);


			m_meshInfo[i].vertices[cnt].Pos.x = finalflo.x;
			m_meshInfo[i].vertices[cnt].Pos.y = finalflo.y;
			m_meshInfo[i].vertices[cnt].Pos.z = finalflo.z;

			
			if (i == 0)
			{
				XMFLOAT4 shapevertex;
				XMStoreFloat4(&shapevertex, xv);
				shapevertex.x += m_meshInfo[i].shape[shapindex].vVertices[cnt].vPos.x * bledparsent;
				shapevertex.y += m_meshInfo[i].shape[shapindex].vVertices[cnt].vPos.y * bledparsent;
				shapevertex.z += m_meshInfo[i].shape[shapindex].vVertices[cnt].vPos.z * bledparsent;
				XMVECTOR shapevec = XMLoadFloat4(&shapevertex);
				XMVECTOR govec = XMVector3Transform(shapevec, Defomat[cnt]);
				XMFLOAT4 endvec;
				XMStoreFloat4(&endvec, govec);
				m_meshInfo[i].vertices[cnt].Pos.x = endvec.x;
				m_meshInfo[i].vertices[cnt].Pos.y = endvec.y;
				m_meshInfo[i].vertices[cnt].Pos.z = endvec.z;
			}

		}

		delete[] Defomat;
		delete[] clusterDeformation;

		// ---------------------
		UINT strides = sizeof(VERTEX);
		UINT offsets = 0;

		context->VSSetShader(pVertexShader.Get(), NULL, 0);
		context->PSSetShader(pPixelShader.Get(), NULL, 0);
		context->VSSetConstantBuffers(1, 1, m_constantBuffer.GetAddressOf());
		context->PSSetConstantBuffers(1, 1, m_constantBuffer.GetAddressOf());
		context->IASetVertexBuffers(0, 1, m_meshInfo[i].pVB.GetAddressOf(), &strides, &offsets);
		context->IASetIndexBuffer(m_meshInfo[i].pIB.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetInputLayout(pVertexLayout.Get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->RSSetState(pRasterizerState.Get());

		CONSTANT_BUFFER cb;
		// �p�����[�^�̎󂯓n��(�萔)
		XMStoreFloat4x4(&cb.world, XMMatrixTranspose(world));
		XMMATRIX inv = XMMatrixInverse(nullptr, world);
		XMStoreFloat3x3(&cb.wit, XMMatrixTranspose(inv));
		XMStoreFloat4x4(&cb.view, XMMatrixTranspose(view));
		XMStoreFloat4x4(&cb.proj, XMMatrixTranspose(proj));
		


		cb.ambient = XMFLOAT4(m_Materials[m_meshInfo[i].m_MaterialName].Ambient[0], m_Materials[m_meshInfo[i].m_MaterialName].Ambient[1],
			m_Materials[m_meshInfo[i].m_MaterialName].Ambient[2], m_Materials[m_meshInfo[i].m_MaterialName].Ambient[3]);

		cb.diffuse = XMFLOAT4(m_Materials[m_meshInfo[i].m_MaterialName].Diffuse[0], m_Materials[m_meshInfo[i].m_MaterialName].Diffuse[1],
			m_Materials[m_meshInfo[i].m_MaterialName].Diffuse[2], m_Materials[m_meshInfo[i].m_MaterialName].Diffuse[3]);

		cb.specular = XMFLOAT4(m_Materials[m_meshInfo[i].m_MaterialName].Specular[0], m_Materials[m_meshInfo[i].m_MaterialName].Specular[1],
			m_Materials[m_meshInfo[i].m_MaterialName].Specular[2], m_Materials[m_meshInfo[i].m_MaterialName].Specular[3]);


		//�e�N�X�`���ݒ�
		if (m_MaterialLinks.count(m_meshInfo[i].m_MaterialName) > 0)
		{
			SetTexture(context, m_MaterialLinks[m_meshInfo[i].m_MaterialName]);
			SetNormalTexture(context, m_NormalLinks[m_meshInfo[i].m_MaterialName]);
			SetEmissiveTexture(context, m_EmissiveLinks[m_meshInfo[i].m_MaterialName]);
		}
		else
		{
			SetTexture(context, nullptr);
			SetNormalTexture(context, nullptr);
			SetEmissiveTexture(context, nullptr);
		}
		
		context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
		memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), sizeof(cb));
		context->Unmap(m_constantBuffer.Get(), 0);


		//�p�����[�^�̎󂯓n��(���_)
		context->Map(m_meshInfo[i].pVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
		memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&m_meshInfo[i].vertices[0]), sizeof(VERTEX)* m_fbxInfo.meshes[i]->GetControlPointsCount());
		context->Unmap(m_meshInfo[i].pVB.Get(), 0);
	
		

		// �`����s
		context->DrawIndexed(m_fbxInfo.meshes[i]->GetPolygonVertexCount(), 0, 0);
	}
}


void FBX_Model::DrawWeapon(
	ID3D11DeviceContext* context,
	SimpleMath::Matrix& world,
	SimpleMath::Matrix& view,
	SimpleMath::Matrix& proj,
	SimpleMath::Matrix& LightCamera,
	SimpleMath::Vector3& eye,
	SimpleMath::Vector3& light,
	float frame,
	int animationNumber
)
{
	FbxArray<FbxString*> AnimStackNameArray;
	m_fbxScene->FillAnimStackNameArray(AnimStackNameArray);
	AnimStackNumber = animationNumber;
	FbxAnimStack* AnimationStack = m_fbxScene->FindMember<FbxAnimStack>(AnimStackNameArray[AnimStackNumber]->Buffer());
	m_fbxScene->SetCurrentAnimationStack(AnimationStack);


	Settoon(context, toon.m_srv.Get());

	// <�I�u�W�F�N�g�̔��f>
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;


	D3D11_MAPPED_SUBRESOURCE pdata;
	// ----- Animation -----
	FrameTime.SetTime(0, 0, 0, frame);

	int weaponindex = 2;


		// �e���_�Ɋ|���邽�߂̍ŏI�I�ȍs��̔z��
		FbxMatrix* clusterDeformation = new FbxMatrix[m_fbxInfo.meshes[weaponindex]->GetControlPointsCount()];
		memset(clusterDeformation, 0, sizeof(FbxMatrix) * m_fbxInfo.meshes[weaponindex]->GetControlPointsCount());
		XMMATRIX* Defomat = new XMMATRIX[m_fbxInfo.meshes[weaponindex]->GetControlPointsCount()];
		memset(Defomat, 0, sizeof(XMMATRIX) * m_fbxInfo.meshes[weaponindex]->GetControlPointsCount());

		// �ړ��A��]�A�g��̂��߂̍s����쐬
		FbxNode* pNode = m_fbxInfo.meshes[weaponindex]->GetNode();
		FbxMatrix globalPosition = pNode->EvaluateGlobalTransform(FrameTime);
		FbxVector4 t0 = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
		FbxVector4 r0 = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
		FbxVector4 s0 = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
		FbxAMatrix geometryOffset = FbxAMatrix(t0, r0, s0);

		FbxSkin* skinDeformer = (FbxSkin*)m_fbxInfo.meshes[weaponindex]->GetDeformer(0, FbxDeformer::eSkin);
		int clusterCount = skinDeformer->GetClusterCount();

		// �e�N���X�^����e���_�ɉe����^���邽�߂̍s��쐬
		for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
			// �N���X�^(�{�[��)�̎��o��
			FbxCluster* cluster = skinDeformer->GetCluster(clusterIndex);
			FbxMatrix vertexTransformMatrix;
			FbxAMatrix referenceGlobalInitPosition;
			FbxAMatrix clusterGlobalInitPosition;
			FbxMatrix clusterGlobalCurrentPosition;
			FbxMatrix clusterRelativeInitPosition;
			FbxMatrix clusterRelativeCurrentPositionInverse;
			FbxAMatrix geometryglobalInit;
			cluster->GetTransformMatrix(referenceGlobalInitPosition);
			geometryglobalInit = referenceGlobalInitPosition * geometryOffset;
			cluster->GetTransformLinkMatrix(clusterGlobalInitPosition);
			clusterGlobalCurrentPosition = cluster->GetLink()->EvaluateGlobalTransform(FrameTime);
			clusterRelativeInitPosition = clusterGlobalInitPosition.Inverse() * geometryglobalInit;
			clusterRelativeCurrentPositionInverse = globalPosition.Inverse() * clusterGlobalCurrentPosition;
			vertexTransformMatrix = clusterRelativeCurrentPositionInverse * clusterRelativeInitPosition;


			// ��ō�����s��Ɋe���_���̉e���x(�d��)���|���Ă��ꂼ��ɉ��Z
			for (int cnt = 0; cnt < cluster->GetControlPointIndicesCount(); cnt++) {
				int index = cluster->GetControlPointIndices()[cnt];
				double weight = cluster->GetControlPointWeights()[cnt];
				FbxMatrix influence = vertexTransformMatrix * weight;
				XMMATRIX infmat = ConvertMatrix(influence);

				Defomat[index] += infmat;
				clusterDeformation[index] += influence;
			}

		}
		// �ŏI�I�Ȓ��_���W���v�Z��VERTEX�ɕϊ�
		for (int cnt = 0; cnt < m_fbxInfo.meshes[weaponindex]->GetControlPointsCount(); cnt++) {
			FbxVector4 outVertex = clusterDeformation[cnt].MultNormalize(m_fbxInfo.meshes[weaponindex]->GetControlPointAt(cnt));
			//FbxVector4 fv = m_fbxInfo.meshes[i]->GetControlPointAt(cnt);
			XMVECTOR xv = ConvertVector(m_fbxInfo.meshes[weaponindex]->GetControlPointAt(cnt));
			XMVECTOR finalvec = XMVector3Transform(xv, Defomat[cnt]);
			XMFLOAT4 finalflo;
			XMStoreFloat4(&finalflo, finalvec);


			m_meshInfo[weaponindex].vertices[cnt].Pos.x = finalflo.x;
			m_meshInfo[weaponindex].vertices[cnt].Pos.y = finalflo.y;
			m_meshInfo[weaponindex].vertices[cnt].Pos.z = finalflo.z;


		}

		delete[] Defomat;
		delete[] clusterDeformation;

		// ---------------------
		UINT strides = sizeof(VERTEX);
		UINT offsets = 0;


		context->IASetVertexBuffers(0, 1, m_meshInfo[weaponindex].pVB.GetAddressOf(), &strides, &offsets);
		context->IASetIndexBuffer(m_meshInfo[weaponindex].pIB.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetInputLayout(pVertexLayout.Get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//context->RSSetState(pRasterizerState);

		CONSTANT_BUFFER cb;
		// �p�����[�^�̎󂯓n��(�萔)
		XMStoreFloat4x4(&cb.world, XMMatrixTranspose(world));
		XMMATRIX inv = XMMatrixInverse(nullptr, world);
		XMStoreFloat3x3(&cb.wit, XMMatrixTranspose(inv));
		XMStoreFloat4x4(&cb.view, XMMatrixTranspose(view));
		XMStoreFloat4x4(&cb.proj, XMMatrixTranspose(proj));


		cb.ambient = XMFLOAT4(m_Materials[m_meshInfo[weaponindex].m_MaterialName].Ambient[0], m_Materials[m_meshInfo[weaponindex].m_MaterialName].Ambient[1],
			m_Materials[m_meshInfo[weaponindex].m_MaterialName].Ambient[2], m_Materials[m_meshInfo[weaponindex].m_MaterialName].Ambient[3]);

		cb.diffuse = XMFLOAT4(m_Materials[m_meshInfo[weaponindex].m_MaterialName].Diffuse[0], m_Materials[m_meshInfo[weaponindex].m_MaterialName].Diffuse[1],
			m_Materials[m_meshInfo[weaponindex].m_MaterialName].Diffuse[2], m_Materials[m_meshInfo[weaponindex].m_MaterialName].Diffuse[3]);

		cb.specular = XMFLOAT4(m_Materials[m_meshInfo[weaponindex].m_MaterialName].Specular[0], m_Materials[m_meshInfo[weaponindex].m_MaterialName].Specular[1],
			m_Materials[m_meshInfo[weaponindex].m_MaterialName].Specular[2], m_Materials[m_meshInfo[weaponindex].m_MaterialName].Specular[3]);


		//�e�N�X�`���ݒ�
		if (m_MaterialLinks.count(m_meshInfo[weaponindex].m_MaterialName) > 0)
		{
			SetTexture(context, m_MaterialLinks[m_meshInfo[weaponindex].m_MaterialName]);
			SetNormalTexture(context, m_NormalLinks[m_meshInfo[weaponindex].m_MaterialName]);
			SetEmissiveTexture(context, m_EmissiveLinks[m_meshInfo[weaponindex].m_MaterialName]);
		}
		else
		{
			SetTexture(context, nullptr);
			SetNormalTexture(context, nullptr);
			SetEmissiveTexture(context, nullptr);
		}

		context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
		memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), sizeof(cb));
		context->Unmap(m_constantBuffer.Get(), 0);


		//�p�����[�^�̎󂯓n��(���_)
		context->Map(m_meshInfo[weaponindex].pVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
		memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&m_meshInfo[weaponindex].vertices[0]), sizeof(VERTEX) * m_fbxInfo.meshes[weaponindex]->GetControlPointsCount());
		context->Unmap(m_meshInfo[weaponindex].pVB.Get(), 0);



		// �`����s
		context->DrawIndexed(m_fbxInfo.meshes[weaponindex]->GetPolygonVertexCount(), 0, 0);
	
}


void FBX_Model::Create(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	const char* fbxfile_path)
{


	// <�V�F�[�_�̐ݒ�>
	ID3DBlob* pCompileVS = NULL;
	ID3DBlob* pCompilePS = NULL;
	//�ŏI�`��V�F�[�_�[�쐬
	//�o�[�e�b�N�X�V�F�[�_�[�쐬
	D3DCompileFromFile(L"Shader/model/model.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	assert(pCompileVS && "pCompileVS is nullptr !");
	(device->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, &pVertexShader));
	//�s�N�Z���V�F�[�_�[�쐬
	D3DCompileFromFile(L"Shader/model/model.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	assert(pCompilePS && "pCompilePS is nullptr !");
	(device->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &pPixelShader));


	// <���_���C�A�E�g>
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"TEXUV",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TANGENT",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
	};
	(device->CreateInputLayout(layout, ARRAYSIZE(layout), pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), &pVertexLayout));
	pCompileVS->Release();
	pCompilePS->Release();

	// <�萔�o�b�t�@�̐ݒ�>
	D3D11_BUFFER_DESC cb;
	cb.ByteWidth = sizeof(CONSTANT_BUFFER);
	cb.Usage = D3D11_USAGE_DYNAMIC;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	(device->CreateBuffer(&cb, NULL, &m_constantBuffer));

	// <FBX�ǂݍ���>
	FBX_Import(fbxfile_path);
	// <���_�f�[�^�̎��o��>
	//FBX_GetVertex();
	
	//<Mesh�쐬>
	CreateMesh(device);
	

	// <���X�^���C�U�̐ݒ�>
	D3D11_RASTERIZER_DESC rdc = {};
	rdc.CullMode = D3D11_CULL_NONE;
	rdc.FillMode = D3D11_FILL_SOLID;
	rdc.DepthClipEnable = true;
	(device->CreateRasterizerState(&rdc, &pRasterizerState));


}

void FBX_Model::Destroy()
{
	m_fbxScene->Destroy();
	m_fbxManager->Destroy();
}

void FBX_Model::FBX_Import(const char* fbxfile_path)
{
	// <FBX�ǂݍ���>
	m_fbxManager = FbxManager::Create();
	m_fbxScene = FbxScene::Create(m_fbxManager, "fbxscene");
	FbxString FileName(fbxfile_path);
	FbxImporter* fbxImporter = FbxImporter::Create(m_fbxManager, "imp");
	if (fbxImporter->Initialize(FileName.Buffer(), -1, m_fbxManager->GetIOSettings()) == false)
	{
		MessageBox(0, L"fbx�t�@�C���̓ǂݍ��݂Ɏ��s", NULL, MB_OK);
	}

	int lSDKMajor, lSDKMinor, lSDKRevision;
	int lFileMajor, lFileMinor, lFileRevision;
	FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);
	fbxImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

	char str[512] = { 0 };
	sprintf_s(str, "�{�v���O������FBX ver %d.%d.%d \n �ǂݍ��ރt�@�C����FBX ver %d.%d.%d",
		lSDKMajor, lSDKMinor, lSDKRevision, lFileMajor, lFileMinor, lFileRevision);

	fbxImporter->Import(m_fbxScene);
	int animationCount = fbxImporter->GetAnimStackCount();
	fbxImporter->Destroy();

	//�V�[���̃|���S�����O�p�ɂ���
	FbxGeometryConverter geometryConverter(m_fbxManager);
	geometryConverter.Triangulate(m_fbxScene, true);


}



void FBX_Model::CreateMesh(ID3D11Device* device)
{
	MeshData mesh_data;
	m_fbxInfo.meshes = GetMesh();
	m_fbxInfo.material = GetMaterial();
	m_meshInfo.resize(m_fbxInfo.meshCount);

	for (int i = 0; i < m_fbxInfo.materialCount; i++)
	{
		LoadMaterial(i,device);
	}


	for (int i = 0; i < m_fbxInfo.meshCount; i++)
	{
		GetVertex(i);
		GetUVSetName(i);
		GetNormal(i);
		GetTangent(i);
		GetShapeAnimation(i);
		SetBuffer(device, i);
		SetMaterialName(i);
	}

	toon.Load("Data/toon/toon03.bmp");

}

std::vector<FbxMesh*> FBX_Model::GetMesh() {
	//���b�V���̐����擾
	m_fbxInfo.meshCount = m_fbxScene->GetSrcObjectCount<FbxMesh>();
	std::vector<FbxMesh*> meshes;
	for (int i = 0; i < m_fbxInfo.meshCount; i++)
	{
		//i�Ԗڂ̃��b�V�����擾
		FbxMesh* mesh = m_fbxScene->GetSrcObject<FbxMesh>(i);
		meshes.emplace_back(mesh);
	}
	return meshes;
}

std::vector<FbxSurfaceMaterial*> FBX_Model::GetMaterial() {
	//�}�e���A���̐����擾
	m_fbxInfo.materialCount = m_fbxScene->GetSrcObjectCount<FbxSurfaceMaterial>();
	std::vector<FbxSurfaceMaterial*> materials;

	for (int i = 0; i < m_fbxInfo.materialCount; i++)
	{
		//i�Ԗڂ̃}�e���A�����擾
		FbxSurfaceMaterial* material = m_fbxScene->GetSrcObject<FbxSurfaceMaterial>(i);
		materials.emplace_back(material);
	}
	return materials;
}

void FBX_Model::LoadMaterial(int materialIndex,ID3D11Device* device)
{
	ObjMaterial entry_material;
	enum class MaterialOrder
	{
		Ambient,
		Diffuse,
		Specular,
		MaxOrder,
	};

	FbxDouble3 colors[(int)MaterialOrder::MaxOrder];
	FbxDouble factors[(int)MaterialOrder::MaxOrder];
	FbxProperty prop = m_fbxInfo.material[materialIndex]->FindProperty(FbxSurfaceMaterial::sAmbient);
	if (m_fbxInfo.material[materialIndex]->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		const char* element_check_list[] =
		{
			FbxSurfaceMaterial::sAmbient,
			FbxSurfaceMaterial::sDiffuse,
			FbxSurfaceMaterial::sSpecular,
		};

		const char* factor_check_list[] =
		{
			FbxSurfaceMaterial::sAmbientFactor,
			FbxSurfaceMaterial::sDiffuseFactor,
			FbxSurfaceMaterial::sSpecularFactor,
		};

		for (int i = 0; i < 3; i++)
		{
			prop = m_fbxInfo.material[materialIndex]->FindProperty(element_check_list[i]);
			if (prop.IsValid())
			{
				colors[i] = prop.Get<FbxDouble3>();
			}
			else
			{
				colors[i] = FbxDouble3(1.0, 1.0, 1.0);
			}

			prop = m_fbxInfo.material[materialIndex]->FindProperty(factor_check_list[i]);
			if (prop.IsValid())
			{
				factors[i] = prop.Get<FbxDouble>();
			}
			else
			{
				factors[i] = 1.0;
			}
		}
	}

	FbxDouble3 color = colors[(int)MaterialOrder::Ambient];
	FbxDouble factor = factors[(int)MaterialOrder::Ambient];
	entry_material.SetAmbient((float)color[0], (float)color[1], (float)color[2], (float)factor);

	color = colors[(int)MaterialOrder::Diffuse];
	factor = factors[(int)MaterialOrder::Diffuse];
	entry_material.SetDiffuse((float)color[0], (float)color[1], (float)color[2], (float)factor);

	color = colors[(int)MaterialOrder::Diffuse];
	factor = factors[(int)MaterialOrder::Diffuse];
	entry_material.SetDiffuse((float)color[0], (float)color[1], (float)color[2], (float)factor);

	color = colors[(int)MaterialOrder::Specular];
	factor = factors[(int)MaterialOrder::Specular];
	entry_material.SetSpecular((float)color[0], (float)color[1], (float)color[2], (float)factor);

	m_Materials[m_fbxInfo.material[materialIndex]->GetName()] = entry_material;

	//�e�N�X�`���ǂݍ���
	//Diffuse�v���p�e�B���擾
	prop = m_fbxInfo.material[materialIndex]->FindProperty(FbxSurfaceMaterial::sDiffuse);
	FbxFileTexture* texture = nullptr;
	std::string keyword;
	int texture_num = prop.GetSrcObjectCount<FbxFileTexture>();
	if (texture_num > 0)
	{
		//prop����FbxFileTexture���擾
		texture = prop.GetSrcObject<FbxFileTexture>(0);
	}
	else
	{
		//FbxLayeredTexture����FbxFileTexturea���擾
		int layer_num = prop.GetSrcObjectCount<FbxLayeredTexture>();
		if (layer_num > 0)
		{
			texture = prop.GetSrcObject<FbxFileTexture>(0);
		}
	}

	if (texture != nullptr && LoadTexture(texture, keyword, device) == true)
	{
		//�ǂݍ��񂾃e�N�X�`���ƃ}�e���A���̊֌W���o���Ă���
		m_MaterialLinks[m_fbxInfo.material[materialIndex]->GetName()] = m_Textures[keyword];
	}

	//�@���e�N�X�`�����擾
	prop = m_fbxInfo.material[materialIndex]->FindProperty(FbxSurfaceMaterial::sNormalMap);
	FbxFileTexture* normaltexture = nullptr;
	std::string normalkeyword;
	texture_num = prop.GetSrcObjectCount<FbxFileTexture>();
	if (texture_num > 0)
	{
		normaltexture = prop.GetSrcObject<FbxFileTexture>(0);
	}
	else
	{
		int layer_num = prop.GetSrcObjectCount<FbxLayeredTexture>();
		if (layer_num > 0)
		{
			normaltexture = prop.GetSrcObject<FbxFileTexture>(0);
		}
	}

	if (normaltexture != nullptr && LoadNormalTexture(normaltexture, normalkeyword, device) == true)
	{
		m_NormalLinks[m_fbxInfo.material[materialIndex]->GetName()] = m_NormalTextures[normalkeyword];
	}

	//�G�~�b�V�u�e�N�X�`�����擾
	prop = m_fbxInfo.material[materialIndex]->FindProperty(FbxSurfaceMaterial::sEmissive);
	FbxFileTexture* emissivetexture = nullptr;
	std::string emissivekeyword;
	texture_num = prop.GetSrcObjectCount<FbxFileTexture>();
	if (texture_num > 0)
	{
		emissivetexture = prop.GetSrcObject<FbxFileTexture>(0);
	}
	else
	{
		int layer_num = prop.GetSrcObjectCount<FbxLayeredTexture>();
		if (layer_num > 0)
		{
			emissivetexture = prop.GetSrcObject<FbxFileTexture>(0);
		}
	}

	if (emissivetexture != nullptr && LoadEmissiveTexture(emissivetexture, emissivekeyword, device) == true)
	{
		m_EmissiveLinks[m_fbxInfo.material[materialIndex]->GetName()] = m_EmissiveTextures[emissivekeyword];
	}
}


void Replace(char search_char, char replace_char, char* buffer)
{
	int len = (int)strlen(buffer);

	for (int i = 0; i < len; i++)
	{
		if (buffer[i] == search_char)
		{
			buffer[i] = replace_char;
		}
	}
}

void Split(char split_char, char* buffer, std::vector<std::string>& out)
{
	int count = 0;
	if (buffer == nullptr)
	{
		return;
	}

	int start_point = 0;

	while (buffer[count] != '\0')
	{
		if (buffer[count] == split_char)
		{
			if (start_point != count)
			{
				char split_str[256] = { 0 };
				strncpy_s(split_str, 256, &buffer[start_point], count - start_point);
				out.emplace_back(split_str);
			}
			else
			{
				out.emplace_back("");
			}
			start_point = count + 1;
		}
		count++;
	}

	if (start_point != count)
	{
		char split_str[256] = { 0 };
		strncpy_s(split_str, 256, &buffer[start_point], count - start_point);
		out.emplace_back(split_str);
	}
}


bool FBX_Model::LoadTexture(FbxFileTexture* texture, std::string& keyword, ID3D11Device* device)
{
	if (texture == nullptr)
	{
		return false;
	}

	//�t�@�C�������擾
	std::string file_path = texture->GetRelativeFileName();

	//�t�@�C������
	char buffer[256];
	ZeroMemory(buffer, sizeof(char) * 256);
	memcpy(buffer, file_path.c_str(), sizeof(char) * 256);

	//�L������
	Replace('\\', '/', buffer);
	std::vector<std::string> split_list;
	std::string replace_file_name = buffer;
	//�u/�v�ŕ���
	Split('/', buffer, split_list);

	std::string root_path = "Data/avatar/";
	std::string path = root_path + replace_file_name;

	setlocale(LC_CTYPE, "jpn");
	wchar_t wFilename[256];
	size_t ret;
	mbstowcs_s(&ret, wFilename, path.c_str(), 256);

	char* file_name;
	size_t size = 0;
	FbxUTF8ToAnsi(split_list[split_list.size() - 1].c_str(), file_name, &size);

	if (m_Textures.count(file_name) > 0 && m_Textures[file_name] != nullptr)
	{
		FbxFree(file_name);
		return true;
	}

	//WIC�摜��ǂݍ���
	auto image = std::make_unique<ScratchImage>();
	if (FAILED(LoadFromWICFile(wFilename, WIC_FLAGS_NONE, &m_info, *image)))
	{
		//���s
		m_info ={};
		return false;
	}

	//�~�b�v�}�b�v�̐���
	if (m_info.mipLevels == 1)
	{
		auto mipChain = std::make_unique<ScratchImage>();
		if (SUCCEEDED(GenerateMipMaps(image->GetImages(), image->GetImageCount(), image->GetMetadata(),
			TEX_FILTER_DEFAULT, 0, *mipChain)))
		{
			image = std::move(mipChain);
		}
	}

	//���\�[�X�ƃV�F�[�_�[���\�[�X�r���[�𐶐�
	if (FAILED(CreateShaderResourceView(device, image->GetImages(), image->GetImageCount(), m_info, &m_Textures[file_name])))
	{
		m_info = {};
		return false;
	}

	keyword = file_name;
	FbxFree(file_name);
	return true;
}


bool FBX_Model::LoadNormalTexture(FbxFileTexture* texture, std::string& keyword, ID3D11Device* device)
{
	if (texture == nullptr)
	{
		return false;
	}

	//�t�@�C�������擾
	std::string file_path = texture->GetRelativeFileName();

	//�t�@�C������
	char buffer[256];
	ZeroMemory(buffer, sizeof(char) * 256);
	memcpy(buffer, file_path.c_str(), sizeof(char) * 256);

	//�L������
	Replace('\\', '/', buffer);
	std::vector<std::string> split_list;
	std::string replace_file_name = buffer;
	//�u/�v�ŕ���
	Split('/', buffer, split_list);

	std::string root_path = "Data/";
	std::string path = root_path + replace_file_name;

	setlocale(LC_CTYPE, "jpn");
	wchar_t wFilename[256];
	size_t ret;
	mbstowcs_s(&ret, wFilename, path.c_str(), 256);

	char* file_name;
	size_t size = 0;
	FbxUTF8ToAnsi(split_list[split_list.size() - 1].c_str(), file_name, &size);

	if (m_NormalTextures.count(file_name) > 0 && m_NormalTextures[file_name] != nullptr)
	{
		FbxFree(file_name);
		return true;
	}

	//WIC�摜��ǂݍ���
	auto image = std::make_unique<ScratchImage>();
	if (FAILED(LoadFromWICFile(wFilename, WIC_FLAGS_NONE, &m_info, *image)))
	{
		//���s
		m_info = {};
		return false;
	}

	//�~�b�v�}�b�v�̐���
	if (m_info.mipLevels == 1)
	{
		auto mipChain = std::make_unique<ScratchImage>();
		if (SUCCEEDED(GenerateMipMaps(image->GetImages(), image->GetImageCount(), image->GetMetadata(),
			TEX_FILTER_DEFAULT, 0, *mipChain)))
		{
			image = std::move(mipChain);
		}
	}

	//���\�[�X�ƃV�F�[�_�[���\�[�X�r���[�𐶐�
	if (FAILED(CreateShaderResourceView(device, image->GetImages(), image->GetImageCount(), m_info, &m_NormalTextures[file_name])))
	{
		m_info = {};
		return false;
	}

	keyword = file_name;
	FbxFree(file_name);
	return true;
}


bool FBX_Model::LoadEmissiveTexture(FbxFileTexture* texture, std::string& keyword, ID3D11Device* device)
{
	if (texture == nullptr)
	{
		return false;
	}

	//�t�@�C�������擾
	std::string file_path = texture->GetRelativeFileName();

	//�t�@�C������
	char buffer[256];
	ZeroMemory(buffer, sizeof(char) * 256);
	memcpy(buffer, file_path.c_str(), sizeof(char) * 256);

	//�L������
	Replace('\\', '/', buffer);
	std::vector<std::string> split_list;
	std::string replace_file_name = buffer;
	//�u/�v�ŕ���
	Split('/', buffer, split_list);

	std::string root_path = "Data/";
	std::string path = root_path + replace_file_name;

	setlocale(LC_CTYPE, "jpn");
	wchar_t wFilename[256];
	size_t ret;
	mbstowcs_s(&ret, wFilename, path.c_str(), 256);

	char* file_name;
	size_t size = 0;
	FbxUTF8ToAnsi(split_list[split_list.size() - 1].c_str(), file_name, &size);

	if (m_NormalTextures.count(file_name) > 0 && m_EmissiveTextures[file_name] != nullptr)
	{
		FbxFree(file_name);
		return true;
	}

	//WIC�摜��ǂݍ���
	auto image = std::make_unique<ScratchImage>();
	if (FAILED(LoadFromWICFile(wFilename, WIC_FLAGS_NONE, &m_info, *image)))
	{
		//���s
		m_info = {};
		return false;
	}

	//�~�b�v�}�b�v�̐���
	if (m_info.mipLevels == 1)
	{
		auto mipChain = std::make_unique<ScratchImage>();
		if (SUCCEEDED(GenerateMipMaps(image->GetImages(), image->GetImageCount(), image->GetMetadata(),
			TEX_FILTER_DEFAULT, 0, *mipChain)))
		{
			image = std::move(mipChain);
		}
	}

	//���\�[�X�ƃV�F�[�_�[���\�[�X�r���[�𐶐�
	if (FAILED(CreateShaderResourceView(device, image->GetImages(), image->GetImageCount(), m_info, &m_EmissiveTextures[file_name])))
	{
		m_info = {};
		return false;
	}

	keyword = file_name;
	FbxFree(file_name);
	return true;
}


void FBX_Model::GetVertex(int meshIndex)
{
	//���b�V���Ɋ܂܂�钸�_���W���擾
	VERTEX vertbox;
	FbxVector4* vtx = m_fbxInfo.meshes[meshIndex]->GetControlPoints();
	m_meshInfo[meshIndex].vertexCount = m_fbxInfo.meshes[meshIndex]->GetControlPointsCount();
	for (int vIdx = 0; vIdx < m_meshInfo[meshIndex].vertexCount; vIdx++) {
		vertbox.Pos.x = (float)vtx[vIdx][0];
		vertbox.Pos.y = (float)vtx[vIdx][1];
		vertbox.Pos.z = (float)vtx[vIdx][2];

		//�ǉ�
		m_meshInfo[meshIndex].vertices.push_back(vertbox);
	}
	m_meshInfo[meshIndex].indexBuffer = m_fbxInfo.meshes[meshIndex]->GetPolygonVertices();
}

void FBX_Model::GetUVSetName(int meshIndex)
{
	FbxStringList uvsetName;
	//���b�V���Ɋ܂܂��UVSet�������ׂĎ擾
	m_fbxInfo.meshes[meshIndex]->GetUVSetNames(uvsetName);
	//UVSet�̐����擾
	FbxArray<FbxVector2> uvset;
	//UVSet�̖��O����UVSet���擾����
	//����̓}���`�e�N�X�`���ɑΉ����Ȃ��̂ōŏ��̖��O���g��
	m_fbxInfo.meshes[meshIndex]->GetPolygonVertexUVs(uvsetName.GetStringAt(0), uvset);
	
	for (int i = 0; i < uvset.Size(); i++)
	{
		m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[i]].uv.x = uvset[i][0];
		m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[i]].uv.y = 1.0 - uvset[i][1];
	}
}

void FBX_Model::GetNormal(int meshIndex)
{
	FbxArray<FbxVector4> normals;
	//�@�����擾
	m_fbxInfo.meshes[meshIndex]->GetPolygonVertexNormals(normals);
	//�@���̐����擾
	int normalCount = normals.Size();
	for (int i = 0; i < normalCount; i++)
	{
		//���_�C���f�b�N�X�ɑΉ��������_�ɒl����
		m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[i]].Normal.x = normals[i][0];
		m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[i]].Normal.y = normals[i][1];
		m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[i]].Normal.z = normals[i][2];
	}
}



void FBX_Model::GetTangent(int meshIndex)
{
	for (int k = 0; k < m_fbxInfo.meshes[meshIndex]->GetPolygonCount();k++)
	{
		XMFLOAT3 v1 = m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[k * 3]].Pos;
		XMFLOAT3 v2 = m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[k * 3 + 1]].Pos;
		XMFLOAT3 v3 = m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[k * 3 + 2]].Pos;

		XMFLOAT2 t1 = m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[k * 3]].uv;
		XMFLOAT2 t2 = m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[k * 3 + 1]].uv;
		XMFLOAT2 t3 = m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[k * 3 + 2]].uv;

		XMFLOAT3 n1 = m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[k * 3]].Normal;
		XMFLOAT3 n2 = m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[k * 3 + 1]].Normal;
		XMFLOAT3 n3 = m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[k * 3 + 2]].Normal;
		XMFLOAT3 vN = XMFLOAT3((n1.x + n2.x + n3.x) / 3, (n1.y + n2.y + n3.y) / 3, (n1.z + n2.z + n3.z) / 3);

		XMFLOAT4 tangent = CalcTangent(v1, v2, v3, t1, t2, t3, vN);

		m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[k * 3]].Tangent = tangent;
		m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[k * 3 + 1]].Tangent = tangent;
		m_meshInfo[meshIndex].vertices[m_meshInfo[meshIndex].indexBuffer[k * 3 + 2]].Tangent = tangent;
	}
}

void FBX_Model::GetShapeAnimation(int meshIndex)
{
	const FbxVector4* l_aDefaultVertices = m_fbxInfo.meshes[meshIndex]->GetControlPoints();

	//�V�F�C�v�A�j���[�V�����f�[�^���擾
	int l_iShapeCount = m_fbxInfo.meshes[meshIndex]->GetShapeCount();

	//�ő�̃V�F�C�v�A�j���[�V���������T�C�Y����
	m_meshInfo[meshIndex].shape.resize(l_iShapeCount);
	int l_iShapeNoOffset = 0;

	//�V�F�C�v�A�j���[�V�����̃O���[�v��
	//�V�F�C�v�����Ă����͂���
	int lBlendShapeDeformerCount = m_fbxInfo.meshes[meshIndex]->GetDeformerCount(FbxDeformer::eBlendShape);
	for (int lBlendShapeIndex = 0; lBlendShapeIndex < lBlendShapeDeformerCount; ++lBlendShapeIndex)
	{
		FbxBlendShape* lBlendShape = (FbxBlendShape*)m_fbxInfo.meshes[meshIndex]->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);

		int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
		for (int lChannelIndex = 0; lChannelIndex < lBlendShapeChannelCount; ++lChannelIndex)
		{
			FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);
			if (lChannel)
			{
				int l_iTargetShapeCount = lChannel->GetTargetShapeCount();
				for (int iTargetShapeIndex = 0; iTargetShapeIndex < l_iTargetShapeCount; ++iTargetShapeIndex)
				{
					FbxShape* l_pShape = lChannel->GetTargetShape(iTargetShapeIndex);

					m_meshInfo[meshIndex].shape.at(l_iShapeNoOffset).sName = (char*)l_pShape->GetName();	//�^�[�Q�b�g�V�F�C�v��

					//�I������Ă��郁�b�V���̑S���_�擾
					int l_iControlPointsCount = l_pShape->GetControlPointsCount();	//���_�C���f�b�N�X
					FbxVector4* l_vControlPoints = l_pShape->GetControlPoints();	//���_���W

					m_meshInfo[meshIndex].shape.at(l_iShapeNoOffset).vVertices.resize(l_iControlPointsCount);	//�V�F�C�v���K������Ă��郁�b�V�������T�C�Y

					for (int j = 0; j < l_iControlPointsCount; j++)
					{
						ShapeAnimationVertex l_vInitVtx;
						l_vInitVtx.vPos.x = 0.0f;
						l_vInitVtx.vPos.y = 0.0f;
						l_vInitVtx.vPos.z = 0.0f;
						//������
						m_meshInfo[meshIndex].shape.at(l_iShapeNoOffset).vVertices.at(j) = l_vInitVtx;
					}

					//�A�j���[�V�������K������Ă���Fbx�ł̎����O�p�`��̒��_�C���f�b�N�X
					int l_iIndicesCount = l_pShape->GetControlPointIndicesCount();	//�K�����Ă��钸�_�C���f�b�N�X
					int* l_aIndices = l_pShape->GetControlPointIndices();	//�K�����Ă��钸�_�C���f�b�N�X�z��

					//�V�F�C�v�A�j���[�V�������K�p����Ă��钸�_�ԍ������ω��������_���l�ߍ���
					for (int i = 0; i < l_iIndicesCount; i++)
					{
						int iActiveIdx = l_aIndices[i];

						//�V�F�C�v�K�p���̒��_�̏ꏊ�Ɗ�{�̒��_�̏ꏊ�����������Ĉړ��ʂ𒊏o
						ShapeAnimationVertex l_vShapeMoveVec;
						l_vShapeMoveVec.vPos.x = (float)l_vControlPoints[iActiveIdx].mData[0] - (float)l_aDefaultVertices[iActiveIdx].mData[0];
						l_vShapeMoveVec.vPos.y = (float)l_vControlPoints[iActiveIdx].mData[1] - (float)l_aDefaultVertices[iActiveIdx].mData[1];
						l_vShapeMoveVec.vPos.z = (float)l_vControlPoints[iActiveIdx].mData[2] - (float)l_aDefaultVertices[iActiveIdx].mData[2];

						//�V�F�C�v�A�j���[�V�����̒��_���l�ߍ���ł���
						m_meshInfo[meshIndex].shape.at(l_iShapeNoOffset).vVertices.at(iActiveIdx) = l_vShapeMoveVec;
					}
					//�I�t�Z�b�g�����Z���A�ʂ̃u�����h�V�F�C�v�̃O���[�v�ɓ����Ă��Ă���̔z��ɓ��ꂷ��
					l_iShapeNoOffset++;
				}
			}
		}
	}
}




XMFLOAT4 FBX_Model::CalcTangent(XMFLOAT3 v1f, XMFLOAT3 v2f, XMFLOAT3 v3f, XMFLOAT2 uv1f, XMFLOAT2 uv2f, XMFLOAT2 uv3f, XMFLOAT3 normalf)
{
	XMVECTOR v1 = XMLoadFloat3(&v1f);
	XMVECTOR v2 = XMLoadFloat3(&v2f);
	XMVECTOR v3 = XMLoadFloat3(&v3f);
	XMVECTOR normal = XMLoadFloat3(&normalf);

	XMVECTOR uv1 = XMLoadFloat2(&uv1f);
	XMVECTOR uv2 = XMLoadFloat2(&uv2f);
	XMVECTOR uv3 = XMLoadFloat2(&uv3f);

	XMVECTOR tangent;
	XMVECTOR bitangent;

	XMVECTOR edge1 = v2 - v1;
	XMVECTOR edge2 = v3 - v1;

	edge1 = XMVector3Normalize(edge1);
	edge2 = XMVector3Normalize(edge2);

	XMFLOAT3 edge1f, edge2f;
	XMStoreFloat3(&edge1f, edge1);
	XMStoreFloat3(&edge2f, edge2);

	XMVECTOR uvEdge1 = uv2 - uv1;
	XMVECTOR uvEdge2 = uv3 - uv1;

	uvEdge1 = XMVector2Normalize(uvEdge1);
	uvEdge2 = XMVector2Normalize(uvEdge2);

	XMFLOAT2 uvEdge1f, uvEdge2f;
	XMStoreFloat2(&uvEdge1f, uvEdge1);
	XMStoreFloat2(&uvEdge2f, uvEdge2);
	float det = (uvEdge1f.x * uvEdge2f.y) - (uvEdge1f.y * uvEdge2f.x);
	det = 1.0f / det;

	XMFLOAT3 tangentf, bitangentf;

	tangentf.x = (uvEdge2f.y * edge1f.x - uvEdge1f.y * edge2f.x) * det;
	tangentf.y = (uvEdge2f.y * edge1f.y - uvEdge1f.y * edge2f.y) * det;
	tangentf.z = (uvEdge2f.y * edge1f.z - uvEdge1f.y * edge2f.z) * det;

	bitangentf.x = (-uvEdge2f.x * edge1f.x + uvEdge1f.x * edge2f.x) * det;
	bitangentf.y = (-uvEdge2f.x * edge1f.y + uvEdge1f.x * edge2f.y) * det;
	bitangentf.z = (-uvEdge2f.x * edge1f.z + uvEdge1f.x * edge2f.z) * det;

	tangent = XMLoadFloat3(&tangentf);
	bitangent = XMLoadFloat3(&bitangentf);

	tangent = XMVector3Normalize(tangent);
	bitangent = XMVector3Normalize(bitangent);

	XMVECTOR binormal = XMVector3Cross(normal, tangent);
	XMVECTOR dotp = XMVector3Dot(binormal, bitangent);
	float dot = XMVectorGetW(dotp);
	float w = (dot >= 0.0f) ? 1.0f : -1.0f;

	XMStoreFloat3(&tangentf, tangent);

	return XMFLOAT4(tangentf.x, tangentf.y, tangentf.z, w);
}

void FBX_Model::SetMaterialName(int meshIndex)
{
	//�}�e���A�����Ȃ���ΏI���


	if (m_fbxInfo.meshes[meshIndex]->GetElementMaterialCount() == 0)
	{
		m_meshInfo[meshIndex].m_MaterialName ="";
		return;
	}

	//Mesh���̃}�e���A�������擾
	FbxLayerElementMaterial* material = m_fbxInfo.meshes[meshIndex]->GetElementMaterial(0);
	int index = material->GetIndexArray().GetAt(0);
	FbxSurfaceMaterial* surface_material = m_fbxInfo.meshes[meshIndex]->GetNode()->GetSrcObject<FbxSurfaceMaterial>(index);
	if (surface_material != nullptr)
	{
		m_meshInfo[meshIndex].m_MaterialName = surface_material->GetName();
	}
	else
	{
		m_meshInfo[meshIndex].m_MaterialName = "";
	}
}

void FBX_Model::SetBuffer(ID3D11Device* device,int meshIndex)
{
	//���_�o�b�t�@�쐬
	D3D11_BUFFER_DESC buffer_desc;
	buffer_desc.ByteWidth = sizeof(VERTEX) * m_fbxInfo.meshes[meshIndex]->GetControlPointsCount();
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;

	if (FAILED(device->CreateBuffer(&buffer_desc, NULL, &m_meshInfo[meshIndex].pVB)))
	{
		MessageBox(0, L"���_�o�b�t�@�̍쐬�Ɏ��s���܂����B", NULL, MB_OK);
	}

	//�C���f�b�N�X�o�b�t�@�쐬
	D3D11_BUFFER_DESC buffer_desc2;
	buffer_desc2.ByteWidth = sizeof(UINT) * m_fbxInfo.meshes[meshIndex]->GetPolygonVertexCount();
	buffer_desc2.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc2.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buffer_desc2.CPUAccessFlags = 0;
	buffer_desc2.MiscFlags = 0;
	buffer_desc2.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA sub_resource;
	sub_resource.pSysMem = m_fbxInfo.meshes[meshIndex]->GetPolygonVertices();
	
	if (FAILED(device->CreateBuffer(&buffer_desc2, &sub_resource, &m_meshInfo[meshIndex].pIB)))
	{
		MessageBox(0, L"�C���f�b�N�X�o�b�t�@�̍쐬�Ɏ��s���܂����B", NULL, MB_OK);
	}
}

void FBX_Model::SetTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* shaderResource)
{

	//PixelShader�Ŏg�p����e�N�X�`���̐ݒ�
	context->PSSetShaderResources(
		0,
		1,
		&shaderResource
	);
	
}

void FBX_Model::SetNormalTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* shaderResource)
{
	//PixelShader�Ŏg�p����e�N�X�`���̐ݒ�
	context->PSSetShaderResources(
		1,
		1,
		&shaderResource
	);

}

void FBX_Model::SetEmissiveTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* shaderResource)
{

	context->PSSetShaderResources(
		2,
		1,
		&shaderResource
	);

}

void FBX_Model::Settoon(ID3D11DeviceContext* context, ID3D11ShaderResourceView* shaderResource)
{
	context->PSGetShaderResources(
		3,
		1,
		&shaderResource
	);
}