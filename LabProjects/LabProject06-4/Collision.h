#pragma once

#define LINE_LINE			0x01
#define LINE_RAY			0x02
#define LINE_SEGMENT		0x03

#define COLLIDER_AABB		0x01
#define COLLIDER_OOBB		0x02
#define COLLIDER_PLANE		0x03
#define COLLIDER_SPHERE		0x04
#define COLLIDER_CAPSULE	0x05
#define COLLIDER_MESH		0x06

/////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMesh;

class CColliderMesh;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
class CCollider
{
private:
	int								m_nReferences;

	UINT							m_nType;

public:
    void							*m_pCollider;

public:
	CCollider(UINT nType);
	virtual ~CCollider();

	void AddRef() { m_nReferences++; }
	void Release() { m_nReferences--; if (m_nReferences <= 0) delete this; }

	int GetType() { return(m_nType); }

	virtual void UpdateColliderMesh(ID3D12GraphicsCommandList *pd3dCommandList, CColliderMesh *pColliderMesh);

	virtual void Merge(CCollider *pOtherCollider);
	virtual void Update(XMFLOAT4X4 *pxmf4x4Transform);

	virtual float Intersect(CCollider *pOtherCollider);
    virtual float Intersect(XMFLOAT4 *pxmf4Plane);
	virtual float Intersect(XMFLOAT3 *pxmf3Position, XMFLOAT3 *pxmf3Direction, BYTE nLineType);
};

/////////////////////////////////////////////////////////////////////////////////////////////////
//
class CAABBCollider : public CCollider
{
public:
	CAABBCollider(CMesh *pMesh);
	virtual ~CAABBCollider();

	virtual void UpdateColliderMesh(ID3D12GraphicsCommandList *pd3dCommandList, CColliderMesh *pColliderMesh);

	virtual void Merge(CCollider *pOtherCollider);
	virtual void Update(XMFLOAT4X4 *pxmf4x4Transform);

	virtual float Intersect(CCollider *pOtherCollider);
    virtual float Intersect(XMFLOAT4 *pxmf4Plane);
	virtual float Intersect(XMFLOAT3 *pxmf3Position, XMFLOAT3 *pxmf3Direction, BYTE nLineType);
};

/////////////////////////////////////////////////////////////////////////////////////////////////
//
class COOBBCollider : public CCollider
{
public:
	COOBBCollider(CMesh *pMesh);
	virtual ~COOBBCollider();

	virtual void Merge(CCollider *pOtherCollider);
	virtual void Update(XMFLOAT4X4 *pxmf4x4Transform);

	virtual float Intersect(CCollider *pOtherCollider);
    virtual float Intersect(XMFLOAT4 *pxmf4Plane);
	virtual float Intersect(XMFLOAT3 *pxmf3Position, XMFLOAT3 *pxmf3Direction, BYTE nLineType);
};

/////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSphereCollider : public CCollider
{
public:
	CSphereCollider(CMesh *pMesh);
	virtual ~CSphereCollider();

	virtual void UpdateColliderMesh(ID3D12GraphicsCommandList *pd3dCommandList, CColliderMesh *pColliderMesh);

	virtual void Merge(CCollider *pOtherCollider);
	virtual void Update(XMFLOAT4X4 *pxmf4x4Transform);

	virtual float Intersect(CCollider *pOtherCollider);
    virtual float Intersect(XMFLOAT4 *pxmf4Plane);
	virtual float Intersect(XMFLOAT3 *pxmf3Position, XMFLOAT3 *pxmf3Direction, BYTE nLineType);
};

/////////////////////////////////////////////////////////////////////////////////////////////////
//
class CPlaneCollider : public CCollider
{
public:
	float							m_fWidth;
	float							m_fHeight;

public:
	CPlaneCollider(XMFLOAT4 d3dxPlane, float fWidth, float fHeight);
	virtual ~CPlaneCollider();

	virtual void UpdateColliderMesh(ID3D12GraphicsCommandList *pd3dCommandList, CColliderMesh *pColliderMesh);

	virtual void Update(XMFLOAT4X4 *pxmf4x4Transform);
	virtual float Intersect(CCollider *pOtherCollider);
	virtual float Intersect(XMFLOAT3 *pxmf3Position, XMFLOAT3 *pxmf3Direction, BYTE nLineType);
};

