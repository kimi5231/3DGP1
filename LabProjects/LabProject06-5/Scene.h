//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"

class CSubScene
{
public:
	CSubScene();
	~CSubScene();

	CSubScene**					m_ppSubScenes = NULL;
	int							m_nSubScenes = 0;

	CGameObject**				m_ppObjects = NULL;
	int							m_nObjects = 0;

public:
	BoundingBox					m_xmBoundingBox;

	void SplitSubScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nSplitLevels, CHeightMapTerrain* pTerrain);

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CHeightMapTerrain* pTerrain, float xStart, float zStart, float fWidth, float fLength);
	void ReleaseObjects();

	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	void ReleaseUploadBuffers();

	CGameObject* PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance);

	void SetBoundingBox(BoundingBox xmBoundingBox) { m_xmBoundingBox = xmBoundingBox; }
	void SetBoundingBox(XMFLOAT3 xmCenter, XMFLOAT3 xmExtents) { m_xmBoundingBox = BoundingBox(xmCenter, xmExtents); }
};

class CScene
{
public:
    CScene();
    ~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseObjects();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }

	bool ProcessInput(UCHAR *pKeysBuffer);
    void AnimateObjects(float fTimeElapsed);
    void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);

	void ReleaseUploadBuffers();

	CGameObject *PickObjectPointedByCursor(int xClient, int yClient, CCamera *pCamera);

	CHeightMapTerrain* GetTerrain() { return(m_pTerrain); }

protected:
	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = NULL;

	CSubScene*					m_pSubScene = NULL;

	CObjectsShader				*m_pShaders = NULL;
	int							m_nShaders = 0;

	CHeightMapTerrain*			m_pTerrain = NULL;
};
