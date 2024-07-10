//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"

CGameObject::CGameObject(int nMeshes)
{
	m_xmf4x4World = Matrix4x4::Identity();

	m_xmBoundingBox = BoundingBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f));

	m_nMeshes = nMeshes;
	m_ppMeshes = NULL;
	if (m_nMeshes > 0)
	{
		m_ppMeshes = new CMesh * [m_nMeshes];
		for (int i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = NULL;
	}
}

CGameObject::~CGameObject()
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Release();
			m_ppMeshes[i] = NULL;
		}
		delete[] m_ppMeshes;
	}

	ReleaseShaderVariables();

	if (m_pShader)
	{
		m_pShader->ReleaseShaderVariables();
		m_pShader->Release();
	}
}

void CGameObject::SetMesh(int nIndex, CMesh* pMesh)
{
	if (m_ppMeshes)
	{
		if (m_ppMeshes[nIndex]) m_ppMeshes[nIndex]->Release();
		m_ppMeshes[nIndex] = pMesh;
		BoundingBox::CreateMerged(m_xmBoundingBox, m_xmBoundingBox, pMesh->GetBoundingBox());
		if (pMesh) pMesh->AddRef();
	}
}

void CGameObject::SetShader(CShader *pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CGameObject::Animate(float fTimeElapsed)
{
}

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(2, 16, &xmf4x4World, 0);
}

void CGameObject::ReleaseShaderVariables()
{
}

BoundingBox CGameObject::GetBoundingBox()
{
	BoundingBox xmBoundingBox;
	m_xmBoundingBox.Transform(xmBoundingBox, XMLoadFloat4x4(&m_xmf4x4World));

	return(xmBoundingBox);
}

BoundingOrientedBox CGameObject::GetOrientedBoundingBox()
{
	BoundingOrientedBox xmBoundingOrientedBox;
	BoundingOrientedBox::CreateFromBoundingBox(xmBoundingOrientedBox, m_xmBoundingBox);
	xmBoundingOrientedBox.Transform(xmBoundingOrientedBox, XMLoadFloat4x4(&m_xmf4x4World));

	return(xmBoundingOrientedBox);
}

BoundingOrientedBox CGameObject::GetOrientedBoundingBox(BoundingBox& xmBoundingBox)
{
	BoundingOrientedBox xmBoundingOrientedBox;
	BoundingOrientedBox::CreateFromBoundingBox(xmBoundingOrientedBox, xmBoundingBox);
	xmBoundingOrientedBox.Transform(xmBoundingOrientedBox, XMLoadFloat4x4(&m_xmf4x4World));

	return(xmBoundingOrientedBox);
}

bool CGameObject::IsVisibleBoundingBox(CCamera* pCamera)
{
	bool bIsVisible = true;

	if (m_ppMeshes)
	{
		BoundingOrientedBox xmBoundingOrientedBox = GetOrientedBoundingBox();
		if (pCamera) bIsVisible = pCamera->IsInFrustum(xmBoundingOrientedBox);
	}

	return(bIsVisible);
}

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (IsVisibleBoundingBox(pCamera))
	{
		OnPrepareRender();

		if (m_pShader)
		{
			m_pShader->UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
			m_pShader->Render(pd3dCommandList, pCamera);
		}

		if (m_ppMeshes)
		{
			bool bIsVisible = true;

			for (int i = 0; i < m_nMeshes; i++)
			{
				if (m_ppMeshes[i])
				{
/*
					BoundingBox xmBoundingBox = m_ppMeshes[i]->GetBoundingBox();
					BoundingOrientedBox xmBoundingOrientedBox = GetOrientedBoundingBox(xmBoundingBox);
					if (pCamera) bIsVisible = pCamera->IsInFrustum(xmBoundingOrientedBox);
*/
					if (bIsVisible) m_ppMeshes[i]->Render(pd3dCommandList);
				}
			}
		}
	}
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->ReleaseUploadBuffers();
		}
	}
}

#define _WITH_RAY_BY_TRANSFORM

void CGameObject::GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4 *pxmf4x4World, XMFLOAT4X4& xmf4x4View, XMFLOAT3 *pxmf3PickRayOrigin, XMFLOAT3 *pxmf3PickRayDirection)
{
	XMFLOAT4X4 xmf4x4WorldView = (pxmf4x4World) ? Matrix4x4::Multiply(*pxmf4x4World, xmf4x4View) : xmf4x4View;
	XMFLOAT4X4 xmf4x4Inverse = Matrix4x4::Inverse(xmf4x4WorldView);

#ifdef _WITH_RAY_BY_TRANSFORM
	XMFLOAT3 xmf3CameraOrigin(0.0f, 0.0f, 0.0f);
	*pxmf3PickRayOrigin = Vector3::TransformCoord(xmf3CameraOrigin, xmf4x4Inverse);
	*pxmf3PickRayDirection= Vector3::TransformCoord(xmf3PickPosition, xmf4x4Inverse);
	*pxmf3PickRayDirection = Vector3::Normalize(Vector3::Subtract(*pxmf3PickRayDirection, *pxmf3PickRayOrigin));
#else
	pxmf3PickRayDirection->x = xmf3PickPosition.x * xmf4x4Inverse._11 + xmf3PickPosition.y * xmf4x4Inverse._21 + xmf3PickPosition.z * xmf4x4Inverse._31;
	pxmf3PickRayDirection->y = xmf3PickPosition.x * xmf4x4Inverse._12 + xmf3PickPosition.y * xmf4x4Inverse._22 + xmf3PickPosition.z * xmf4x4Inverse._32;
	pxmf3PickRayDirection->z = xmf3PickPosition.x * xmf4x4Inverse._13 + xmf3PickPosition.y * xmf4x4Inverse._23 + xmf3PickPosition.z * xmf4x4Inverse._33;
	pxmf3PickRayOrigin->x = xmf4x4Inverse._41;
	pxmf3PickRayOrigin->y = xmf4x4Inverse._42;
	pxmf3PickRayOrigin->z = xmf4x4Inverse._43;
	*pxmf3PickRayDirection = Vector3::Normalize(*pxmf3PickRayDirection);
#endif
}

int CGameObject::PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float *pfHitDistance)
{
	int nIntersects = 0;
	if (m_ppMeshes)
	{
		XMFLOAT3 xmf3PickRayOrigin, xmf3PickRayDirection;
		GenerateRayForPicking(xmf3PickPosition, &m_xmf4x4World, xmf4x4View, &xmf3PickRayOrigin, &xmf3PickRayDirection);

		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
			{
				float fHitDistance = 0.0f;
				int nIntersect = m_ppMeshes[i]->CheckRayIntersection(xmf3PickRayOrigin, xmf3PickRayDirection, pfHitDistance);
				if (nIntersect)
				{
					nIntersects += nIntersect;
					if (fHitDistance < *pfHitDistance) *pfHitDistance = fHitDistance;
				}
			}
		}
	}

	return(nIntersects);
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CRotatingObject::CRotatingObject(int nMeshes) : CGameObject(nMeshes)
{
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 15.0f;
}

CRotatingObject::~CRotatingObject()
{
}

void CRotatingObject::Animate(float fTimeElapsed)
{
	CGameObject::Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);
}

void CRotatingObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CRevolvingObject::CRevolvingObject(int nMeshes) : CGameObject(nMeshes)
{
	m_xmf3RevolutionAxis = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_fRevolutionSpeed = 0.0f;
}

CRevolvingObject::~CRevolvingObject()
{
}

void CRevolvingObject::Animate(float fTimeElapsed)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3RevolutionAxis), XMConvertToRadians(m_fRevolutionSpeed * fTimeElapsed));
	m_xmf4x4World = Matrix4x4::Multiply(m_xmf4x4World, mtxRotate);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color) : CGameObject(0)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;

	m_nMeshes = cxBlocks * czBlocks;
	m_ppMeshes = new CMesh * [m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = NULL;

	CHeightMapGridMesh* pHeightMapGridMesh = NULL;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, xStart, zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
			SetMesh(x + (z * cxBlocks), pHeightMapGridMesh);
		}
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTerrainShader* pShader = new CTerrainShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);
}

CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}
