#include "stdafx.h"
#include "Collision.h"
#include "Mesh.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
//
bool RayIntersectTriangle(XMFLOAT3& xmf3Origin, XMFLOAT3& xmf3Direction, XMFLOAT3& xmf3P0, XMFLOAT3& xmf3P1, XMFLOAT3& xmf3P2, float *pfU, float *pfV, float *pfRayToTriangle)
{
    XMFLOAT3 xmf3Edge1 = Vector3::Subtract(xmf3P1, xmf3P0);
    XMFLOAT3 xmf3Edge2 = Vector3::Subtract(xmf3P2, xmf3P0);
	XMFLOAT3 xmf3P = Vector3::CrossProduct(xmf3Direction, xmf3Edge2);
    float fAngle = Vector3::DotProduct(xmf3Edge1, xmf3P);
    if (::IsZero(fAngle)) return(false);
    float f = 1.0f / fAngle;
	XMFLOAT3 xmf3P0ToOrigin = Vector3::Subtract(xmf3Origin, xmf3P0);
	*pfU = f * Vector3::DotProduct(xmf3P0ToOrigin, xmf3P);
    if ((*pfU < 0.0f) || (*pfU > 1.0f)) return(false);
	XMFLOAT3 xmf3Q = Vector3::CrossProduct(xmf3P0ToOrigin, xmf3Edge1);
	*pfV = f * Vector3::DotProduct(xmf3Direction, xmf3Q);
    if ((*pfV < 0.0f) || ((*pfU + *pfV) > 1.0f)) return(false);
	*pfRayToTriangle = f * Vector3::DotProduct(xmf3Edge2, xmf3Q);

	return(*pfRayToTriangle >= 0.0f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
CCollider::CCollider(UINT nType)
{
	m_nReferences = 0;
	m_nType = nType;
}

CCollider::~CCollider()
{
}

void CCollider::UpdateColliderMesh(ID3D12GraphicsCommandList *pd3dCommandList, CColliderMesh *pColliderMesh)
{
}

void CCollider::Merge(CCollider *pOtherCollider)
{
}

void CCollider::Update(XMFLOAT4X4 *pxmf4x4Transform)
{
}

float CCollider::Intersect(XMFLOAT3 *pxmf3Position, XMFLOAT3 *pxmf3Direction, BYTE nLineType)
{
	return(FLT_MAX);
}

float CCollider::Intersect(CCollider *pOtherCollider)
{
	return(FLT_MAX);
}

float CCollider::Intersect(XMFLOAT4 *pd3dxpPlane)
{
	return(FLT_MAX);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
CAABBCollider::CAABBCollider(CMesh *pMesh) : CCollider(COLLIDER_AABB)
{
	BoundingBox *pCollider = new BoundingBox();
	pCollider->BuildCollider(pMesh);
	m_pCollider = pCollider;
}

CAABBCollider::~CAABBCollider()
{
	if (m_pCollider) delete m_pCollider;
}

void CAABBCollider::UpdateColliderMesh(ID3D12GraphicsCommandList *pd3dCommandList, CColliderMesh *pColliderMesh)
{
	if (m_pCollider) 
	{
		BoundingBox *pAABB = (BoundingBox *)m_pCollider;
		if (pColliderMesh) pColliderMesh->UpdateColliderMesh(pd3dCommandList, &pAABB->m_xmf3Minimum, &pAABB->m_xmf3Maximum);
	}
}

void CAABBCollider::Merge(CCollider *pOtherCollider)
{
	if (m_pCollider) 
	{
		BoundingBox *pAABB = (BoundingBox *)m_pCollider;
		pAABB->Merge((BoundingBox *)pOtherCollider->m_pCollider);
	}
}

void CAABBCollider::Update(XMFLOAT4X4 *pxmf4x4Transform)
{
	if (m_pCollider) 
	{
		BoundingBox *pAABB = (BoundingBox *)m_pCollider;
		pAABB->Update(pxmf4x4Transform);
	}
}

float CAABBCollider::Intersect(XMFLOAT3 *pxmf3Position, XMFLOAT3 *pxmf3Direction, BYTE nLineType)
{
	float fDistance = FLT_MAX;
	if (m_pCollider) 
	{
		BoundingBox *pAABB = (BoundingBox *)m_pCollider;
		fDistance = pAABB->Intersect(pxmf3Position, pxmf3Direction, nLineType);
	}
	return(fDistance);
}

float CAABBCollider::Intersect(CCollider *pOtherCollider)
{
	float fDistance = FLT_MAX;
	if (m_pCollider && pOtherCollider) 
	{
		BoundingBox *pAABB = (BoundingBox *)m_pCollider;
		switch (pOtherCollider->GetType())
		{
			case COLLIDER_AABB:
			{
				BoundingBox *pOtherAABB = (BoundingBox *)pOtherCollider->m_pCollider;
				fDistance = pAABB->Intersect(pOtherAABB);
				break;
			}
			case COLLIDER_PLANE:
			{
				XMFLOAT4 *pOtherPlane = (XMFLOAT4 *)pOtherCollider->m_pCollider;
				fDistance = pAABB->Intersect(pOtherPlane);
				break;
			}
		}
	}
	return(fDistance);
}

float CAABBCollider::Intersect(XMFLOAT4 *pd3dxpPlane)
{
	float fDistance = FLT_MAX;
	if (m_pCollider) 
	{
		BoundingBox *pAABB = (BoundingBox *)m_pCollider;
		fDistance = pAABB->Intersect(pd3dxpPlane);
	}
	return(fDistance);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
COOBBCollider::COOBBCollider(CMesh *pMesh) : CCollider(COLLIDER_OOBB)
{
	OOBB *pCollider = new OOBB();
	pCollider->BuildCollider(pMesh);
	m_pCollider = pCollider;
}

COOBBCollider::~COOBBCollider()
{
	if (m_pCollider) delete m_pCollider;
}

void COOBBCollider::Merge(CCollider *pOtherCollider)
{
	if (m_pCollider) 
	{
		OOBB *pOOBB = (OOBB *)m_pCollider;
		pOOBB->Merge((OOBB *)pOtherCollider->m_pCollider);
	}
}

void COOBBCollider::Update(XMFLOAT4X4 *pxmf4x4Transform)
{
	if (m_pCollider) 
	{
		OOBB *pOOBB = (OOBB *)m_pCollider;
		pOOBB->Update(pxmf4x4Transform);
	}
}

float COOBBCollider::Intersect(XMFLOAT3 *pxmf3Position, XMFLOAT3 *pxmf3Direction, BYTE nLineType)
{
	float fDistance = FLT_MAX;
	if (m_pCollider) 
	{
		OOBB *pOOBB = (OOBB *)m_pCollider;
		fDistance = pOOBB->Intersect(pxmf3Position, pxmf3Direction, nLineType);
	}
	return(fDistance);
}

float COOBBCollider::Intersect(CCollider *pOtherCollider)
{
	float fDistance = FLT_MAX;
	if (m_pCollider && pOtherCollider) 
	{
		OOBB *pOOBB = (OOBB *)m_pCollider;
		switch (pOtherCollider->GetType())
		{
			case COLLIDER_AABB:
			{
				BoundingBox *pOtherAABB = (BoundingBox *)pOtherCollider->m_pCollider;
				fDistance = pOOBB->Intersect(pOtherAABB);
				break;
			}
			case COLLIDER_OOBB:
			{
				OOBB *pOtherOOBB = (OOBB *)pOtherCollider->m_pCollider;
				fDistance = pOOBB->Intersect(pOtherOOBB);
				break;
			}
			case COLLIDER_PLANE:
			{
				XMFLOAT4 *pOtherPlane = (XMFLOAT4 *)pOtherCollider->m_pCollider;
				fDistance = pOOBB->Intersect(pOtherPlane);
				break;
			}
		}
	}
	return(fDistance);
}

float COOBBCollider::Intersect(XMFLOAT4 *pd3dxpPlane)
{
	float fDistance = FLT_MAX;
	if (m_pCollider) 
	{
		OOBB *pOOBB = (OOBB *)m_pCollider;
		fDistance = pOOBB->Intersect(pd3dxpPlane);
	}
	return(fDistance);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
CPlaneCollider::CPlaneCollider(XMFLOAT4 d3dxPlane, float fWidth, float fHeight) : CCollider(COLLIDER_PLANE)
{
	m_pCollider = new XMFLOAT4(d3dxPlane);
	m_fWidth = fWidth; 
	m_fHeight = fHeight;
}

CPlaneCollider::~CPlaneCollider()
{
	if (m_pCollider) delete m_pCollider;
}

void CPlaneCollider::UpdateColliderMesh(ID3D12GraphicsCommandList *pd3dCommandList, CColliderMesh *pColliderMesh)
{
}

void CPlaneCollider::Update(XMFLOAT4X4 *pxmf4x4Transform)
{
}

float CPlaneCollider::Intersect(XMFLOAT3 *pxmf3Position, XMFLOAT3 *pxmf3Direction, BYTE nLineType)
{
	float fDistance = FLT_MAX;
	if (m_pCollider)
	{
		XMFLOAT4 *pxmf4Plane = (XMFLOAT4 *)m_pCollider;
		XMFLOAT3 xmf3Intersected;
		switch (nLineType)
		{
			case LINE_LINE:
				break;
			case LINE_RAY:
				break;
			case LINE_SEGMENT:
			{
				XMFLOAT3 xmf3EndPoint = *pxmf3Position + *pxmf3Direction;
				D3DXPlaneIntersectLine(&xmf3Intersected, pxmf4Plane, pxmf3Position, &xmf3EndPoint);
				break;
			}
		}
	}
	return(fDistance);
}

float CPlaneCollider::Intersect(CCollider *pOtherCollider)
{
	float fDistance = FLT_MAX;
	if (m_pCollider && pOtherCollider) 
	{
		XMFLOAT4 *pxmf4Plane = (XMFLOAT4 *)m_pCollider;
		switch (pOtherCollider->GetType())
		{
			case COLLIDER_AABB:
			{
				BoundingBox *pAABB = (BoundingBox *)pOtherCollider->m_pCollider;
				fDistance = pAABB->Intersect(pxmf4Plane);
				break;
			}
		}
	}
	return(fDistance);
}

