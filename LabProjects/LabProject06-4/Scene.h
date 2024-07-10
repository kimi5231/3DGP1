//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"

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

	CGameObject**				m_ppObjects = NULL;
	int							m_nObjects = 0;

	CObjectsShader				*m_pShaders = NULL;
	int							m_nShaders = 0;

	CHeightMapTerrain*			m_pTerrain = NULL;
};
