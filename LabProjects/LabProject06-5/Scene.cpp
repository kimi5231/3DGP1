//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

CSubScene::CSubScene()
{
}

CSubScene::~CSubScene()
{
}

void CSubScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CHeightMapTerrain* pTerrain, float xStart, float zStart, float fWidth, float fLength)
{
	CSphereMeshDiffused* pSphereMesh = new CSphereMeshDiffused(pd3dDevice, pd3dCommandList, 6.0f, 20, 20);

	CDiffusedShader* pShader = new CDiffusedShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	float fxPitch = 12.0f * 2.5f;
	float fyPitch = 12.0f * 2.5f;
	float fzPitch = 12.0f * 2.5f;

	int xObjects = int(fWidth / fxPitch), zObjects = int(fLength / fzPitch);
	m_nObjects = xObjects * zObjects;
	m_ppObjects = new CGameObject * [m_nObjects];

	CRotatingObject* pRotatingObject = NULL;
	for (int i = 0, x = 0; x < xObjects; x++)
	{
		for (int z = 0; z < zObjects; z++)
		{
			pRotatingObject = new CRotatingObject();
			pRotatingObject->SetMesh(0, pSphereMesh);

			float xPosition = xStart + x * fxPitch;
			float zPosition = zStart + z * fzPitch;
			float fHeight = pTerrain->GetHeight(xPosition, zPosition);
			pRotatingObject->SetPosition(xPosition, fHeight + (10.0f * fyPitch) + 6.0f, zPosition);
			pRotatingObject->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
			pRotatingObject->SetRotationSpeed(36.0f * (i % 10) + 36.0f);
			pRotatingObject->SetShader(pShader);

			m_ppObjects[i++] = pRotatingObject;
		}
	}
}

void CSubScene::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}

	for (int i = 0; i < m_nSubScenes; i++) m_ppSubScenes[i]->ReleaseObjects();
	if (m_ppSubScenes) delete[] m_ppSubScenes;
}

void CSubScene::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++)
		{
			if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
		}
	}

	for (int i = 0; i < m_nSubScenes; i++)
	{
		if (m_ppSubScenes[i]) m_ppSubScenes[i]->ReleaseUploadBuffers();
	}
}

void CSubScene::AnimateObjects(float fTimeElapsed)
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++)
		{
			if (m_ppObjects[j]) m_ppObjects[j]->Animate(fTimeElapsed);
		}
	}

	for (int i = 0; i < m_nSubScenes; i++)
	{
		if (m_ppSubScenes[i]) m_ppSubScenes[i]->AnimateObjects(fTimeElapsed);
	}
}

CGameObject* CSubScene::PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance)
{
	int nIntersected = 0;
	*pfNearHitDistance = FLT_MAX;
	float fHitDistance = FLT_MAX;
	CGameObject* pIntersectedObject = NULL, * pNearestObject = NULL;
	for (int j = 0; j < m_nObjects; j++)
	{
		nIntersected = m_ppObjects[j]->PickObjectByRayIntersection(xmf3PickPosition, xmf4x4View, &fHitDistance);
		if ((nIntersected > 0) && (fHitDistance < *pfNearHitDistance))
		{
			*pfNearHitDistance = fHitDistance;
			pNearestObject = m_ppObjects[j];
		}
	}

	for (int i = 0; i < m_nSubScenes; i++)
	{
		pIntersectedObject = m_ppSubScenes[i]->PickObjectByRayIntersection(xmf3PickPosition, xmf4x4View, &fHitDistance);
		if (pIntersectedObject && (fHitDistance < *pfNearHitDistance))
		{
			*pfNearHitDistance = fHitDistance;
			pNearestObject = pIntersectedObject;
		}
	}

	return(pNearestObject);
}

void CSubScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (pCamera->IsInFrustum(m_xmBoundingBox))
	{
		for (int j = 0; j < m_nObjects; j++)
		{
			if (m_ppObjects[j])
			{
				m_ppObjects[j]->UpdateShaderVariables(pd3dCommandList);
				m_ppObjects[j]->Render(pd3dCommandList, pCamera);
			}
		}

		for (int i = 0; i < m_nSubScenes; i++)
		{
			if (m_ppSubScenes[i]) m_ppSubScenes[i]->Render(pd3dCommandList, pCamera);
		}
	}
}

void CSubScene::SplitSubScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nSplitLevels, CHeightMapTerrain* pTerrain)
{
	if (nSplitLevels > 0)
	{
		m_nSubScenes = 4;
		m_ppSubScenes = new CSubScene * [m_nSubScenes];
		for (int i = 0; i < m_nSubScenes; i++) m_ppSubScenes[i] = new CSubScene();

		float xHalf = m_xmBoundingBox.Extents.x * 0.5f;
		float yHalf = m_xmBoundingBox.Extents.y;
		float zHalf = m_xmBoundingBox.Extents.z * 0.5f;

		m_ppSubScenes[0]->SetBoundingBox(XMFLOAT3(m_xmBoundingBox.Center.x - xHalf, m_xmBoundingBox.Center.y, m_xmBoundingBox.Center.z - zHalf), XMFLOAT3(xHalf, yHalf, zHalf));
		m_ppSubScenes[1]->SetBoundingBox(XMFLOAT3(m_xmBoundingBox.Center.x - xHalf, m_xmBoundingBox.Center.y, m_xmBoundingBox.Center.z + zHalf), XMFLOAT3(xHalf, yHalf, zHalf));
		m_ppSubScenes[2]->SetBoundingBox(XMFLOAT3(m_xmBoundingBox.Center.x + xHalf, m_xmBoundingBox.Center.y, m_xmBoundingBox.Center.z - zHalf), XMFLOAT3(xHalf, yHalf, zHalf));
		m_ppSubScenes[3]->SetBoundingBox(XMFLOAT3(m_xmBoundingBox.Center.x + xHalf, m_xmBoundingBox.Center.y, m_xmBoundingBox.Center.z + zHalf), XMFLOAT3(xHalf, yHalf, zHalf));

		m_ppSubScenes[0]->SplitSubScene(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nSplitLevels - 1, pTerrain);
		m_ppSubScenes[1]->SplitSubScene(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nSplitLevels - 1, pTerrain);
		m_ppSubScenes[2]->SplitSubScene(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nSplitLevels - 1, pTerrain);
		m_ppSubScenes[3]->SplitSubScene(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nSplitLevels - 1, pTerrain);
	}
	else
	{
		float xStart = m_xmBoundingBox.Center.x - m_xmBoundingBox.Extents.x;
		float zStart = m_xmBoundingBox.Center.z - m_xmBoundingBox.Extents.z;

		BuildObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pTerrain, xStart, zStart, m_xmBoundingBox.Extents.x * 2.0f, m_xmBoundingBox.Extents.z * 2.0f);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//
CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	XMFLOAT4 xmf4Color(0.0f, 0.2f, 0.0f, 0.0f);
	XMFLOAT3 xmf3Scale(8.0f, 4.0f, 8.0f);
#ifdef _WITH_TERRAIN_PARTITION
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("HeightMap.raw"), 257, 257, 17, 17, xmf3Scale, xmf4Color);
#else
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("HeightMap.raw"), 257, 257, 257, 257, xmf3Scale, xmf4Color);
#endif

	float fTerrainWidth = m_pTerrain->GetWidth(), fTerrainLength = m_pTerrain->GetLength();

	float fxPitch = 12.0f * 2.5f;
	float fyPitch = 12.0f * 2.5f;
	float fzPitch = 12.0f * 2.5f;
//*
	int xShaders = int(fTerrainWidth / (fxPitch * 5)), zShaders = int(fTerrainLength / (fzPitch * 5));
	m_nShaders = xShaders * zShaders;
	m_pShaders = new CObjectsShader[m_nShaders];
	for (int i = 0, x = 0; x < xShaders; x++)
	{
		for (int z = 0; z < zShaders; z++)
		{
			m_pShaders[i].CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
			m_pShaders[i++].BuildObjects(pd3dDevice, pd3dCommandList, m_pTerrain, x * (fxPitch * 5), z * (fzPitch * 5), (fxPitch * 5), (fzPitch * 5));
		}
	}
//*/

	float xHalf = fTerrainWidth * 0.5f;
	float zHalf = fTerrainLength * 0.5f;
	float yHalf = (255.0f * xmf3Scale.y) * 0.85f;

	m_pSubScene = new CSubScene;
	m_pSubScene->SetBoundingBox(XMFLOAT3(xHalf, 0.0f, zHalf), XMFLOAT3(xHalf, yHalf, zHalf));
	m_pSubScene->SplitSubScene(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 3, m_pTerrain);
}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[3];
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[0].Constants.Num32BitValues = 16;
	pd3dRootParameters[0].Constants.ShaderRegister = 0; //Player
	pd3dRootParameters[0].Constants.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 32;
	pd3dRootParameters[1].Constants.ShaderRegister = 1; //Camera
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[2].Constants.Num32BitValues = 16;
	pd3dRootParameters[2].Constants.ShaderRegister = 2; //GameObjects
	pd3dRootParameters[2].Constants.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	if (m_pTerrain) delete m_pTerrain;

	if (m_pShaders)
	{
		for (int i = 0; i < m_nShaders; i++) m_pShaders[i].ReleaseShaderVariables();
		for (int i = 0; i < m_nShaders; i++) m_pShaders[i].ReleaseObjects();
		delete[] m_pShaders;
	}

	if (m_pSubScene) m_pSubScene->ReleaseObjects();
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].AnimateObjects(fTimeElapsed);
	}

	if (m_pSubScene) m_pSubScene->AnimateObjects(fTimeElapsed);
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

	for (int i = 0; i < m_nShaders; i++) m_pShaders[i].ReleaseUploadBuffers();

	if (m_pSubScene) m_pSubScene->ReleaseUploadBuffers();
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::ProcessInput(UCHAR *pKeysBuffer)
{
	return(false);
}

CGameObject *CScene::PickObjectPointedByCursor(int xClient, int yClient, CCamera *pCamera)
{
	if (!pCamera) return(NULL);

	XMFLOAT4X4 xmf4x4View = pCamera->GetViewMatrix();
	XMFLOAT4X4 xmf4x4Projection = pCamera->GetProjectionMatrix();
	D3D12_VIEWPORT d3dViewport = pCamera->GetViewport();

	XMFLOAT3 xmf3PickPosition;
	xmf3PickPosition.x = (((2.0f * xClient) / d3dViewport.Width) - 1) / xmf4x4Projection._11;
	xmf3PickPosition.y = -(((2.0f * yClient) / d3dViewport.Height) - 1) / xmf4x4Projection._22;
	xmf3PickPosition.z = 1.0f;

	int nIntersected = 0;
	float fHitDistance = FLT_MAX, fNearestHitDistance = FLT_MAX;
	CGameObject *pIntersectedObject = NULL, *pNearestObject = NULL;
	for (int i = 0; i < m_nShaders; i++)
	{
		pIntersectedObject = m_pShaders[i].PickObjectByRayIntersection(xmf3PickPosition, xmf4x4View, &fHitDistance);
		if (pIntersectedObject && (fHitDistance < fNearestHitDistance))
		{
			fNearestHitDistance = fHitDistance;
			pNearestObject = pIntersectedObject;
		}
	}

	pIntersectedObject = m_pSubScene->PickObjectByRayIntersection(xmf3PickPosition, xmf4x4View, &fHitDistance);
	if (pIntersectedObject && (fHitDistance < fNearestHitDistance))
	{
		fNearestHitDistance = fHitDistance;
		pNearestObject = pIntersectedObject;
	}

	return(pNearestObject);
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].Render(pd3dCommandList, pCamera);
	}

	if (m_pSubScene) m_pSubScene->Render(pd3dCommandList, pCamera);
}

