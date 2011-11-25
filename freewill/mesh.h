// mesh.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__MESH_H)
#define __MESH_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"
#include "meshplus.h"
#include "kinetemplate.h"

#include <map>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// CMesh

class CMesh : public CTKineChild<FWUNKNOWN<
							BASECLASS<IMesh, IKineChild>,
							IID_IMesh, IMesh,
							IID_IKineChild, IKineChild,
							IID_IKineObj3D, IKineObj3D> >
{
public:
	// buffer initialization & query
	virtual HRESULT _stdcall GetBuffers(IMeshVertexBuffer**, IMeshFaceBuffer**);
	virtual HRESULT _stdcall PutBuffers(IMeshVertexBuffer*,  IMeshFaceBuffer*);
	virtual HRESULT _stdcall IsReady() { return (m_pVertexBuffer && m_pFaceBuffer) ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall GetDictionary(IMeshDictionary**);
	virtual HRESULT _stdcall PutDictionary(IMeshDictionary*);

	// visibility
	virtual HRESULT _stdcall IsVisible()					{ return m_bOn ? S_OK : S_FALSE; }		
	virtual HRESULT _stdcall PutVisible(BOOL bOn)			{ m_bOn = bOn; return S_OK; }

	// buffer range & parameters
	virtual HRESULT _stdcall GetVertexFirst(FWULONG *p)		{ *p = m_nVertexFirst; return S_OK; }
	virtual HRESULT _stdcall GetVertexNum(FWULONG *p)			{ *p = m_nVertexNum; return S_OK; }
	virtual HRESULT _stdcall GetVertexItemSize(FWULONG *p)	{ *p = m_nVertexItemSize; return S_OK; }
	virtual HRESULT _stdcall GetFaceFirst(FWULONG *p)			{ *p = m_nFaceFirst; return S_OK; }
	virtual HRESULT _stdcall GetFaceNum(FWULONG *p)			{ *p = m_nFaceNum; return S_OK; }
	virtual HRESULT _stdcall GetFaceItemSize(FWULONG *p)		{ *p = m_nFaceItemSize; return S_OK; }

	// caps
	virtual HRESULT _stdcall GetVertexFormat(/*[out]*/ FWULONG *nFormat, /*[out]*/ FWULONG *nBones, /*[out]*/ FWULONG *nTextures, /*[out]*/ FWULONG *nVertexSize);
	virtual HRESULT _stdcall GetVertexCaps(enum MESH_VERTEXID nFlag, FWULONG nIndex, /*[out]*/ FWULONG *pOffset, /*[out,retval]*/ FWULONG *pSize);
	virtual HRESULT _stdcall GetFaceFormat(/*[out]*/ FWULONG *nSizeOfVertexIndex);
	virtual HRESULT _stdcall SupportsVertexBlending();
	virtual HRESULT _stdcall SupportsIndexedVertexBlending();
	virtual HRESULT _stdcall SupportsSubmeshedVertexBlending();

	// support for submeshes
	virtual HRESULT _stdcall GetSubmeshInfo(FWULONG *pnSubmeshNum, FWULONG *pnBoneNum, FWULONG **pSubmeshLen, FWULONG **pSubmeshBones);
	
	// byte buffers
	virtual HRESULT _stdcall Open(/*[out]*/ BYTE **ppVertexBytes, /*[out]*/ BYTE **ppFaceBytes);
	virtual HRESULT _stdcall Close();

	// mesh transform
	virtual HRESULT _stdcall GetTransform(ITransform **pVal);
	virtual HRESULT _stdcall PutTransform(ITransform *newVal);

	// operations
	virtual HRESULT _stdcall SetVertexXYZ(FWULONG iVertex, FWFLOAT x, FWFLOAT y, FWFLOAT z);
	virtual HRESULT _stdcall SetVertexXYZVector(FWULONG iVertex, FWVECTOR v);
	virtual HRESULT _stdcall SetVertexPointSize(FWULONG iVertex, FWULONG);
	virtual HRESULT _stdcall SetVertexDiffuse(FWULONG iVertex, FWULONG);
	virtual HRESULT _stdcall SetVertexSpecular(FWULONG iVertex, FWULONG);
	virtual HRESULT _stdcall SetVertexTexture(FWULONG iVertex, FWULONG iTexture, FWULONG);
	virtual HRESULT _stdcall SetVertexTextureUV(FWULONG iVertex, FWULONG iTexture, FWFLOAT u, FWFLOAT v);
	virtual HRESULT _stdcall SetFace(FWULONG iFace, FWULONG iVertexA, FWULONG iVertexB, FWULONG iVertexC);

	virtual HRESULT _stdcall SetMaterial(IMaterial *pMaterial);
	virtual HRESULT _stdcall GetMaterial(IMaterial **ppMaterial);

	virtual HRESULT _stdcall SupportNormal(FWULONG nLimit);
	virtual HRESULT _stdcall AddNormal(/*[in, out]*/ FWULONG *index, FWFLOAT x, FWFLOAT y, FWFLOAT z);
	virtual HRESULT _stdcall AddNormalVector(/*[in, out]*/ FWULONG *index, FWVECTOR v);
	
	virtual HRESULT _stdcall SupportBlendWeight(FWFLOAT fMinWeight, FWULONG nMinVertexNum);
	virtual HRESULT _stdcall AddBlendWeight(FWULONG iVertex, FWFLOAT fWeight, LPOLESTR pBoneName);

	// Reproduction - disabled (stuff inherited from IKineObj3D)
	
	virtual HRESULT _stdcall Reproduce(IKineObj3D **p)  { return ERROR(E_NOTIMPL); }
	virtual HRESULT _stdcall CanReproduce()				{ return S_FALSE; }

	DECLARE_FACTORY_CLASS(Mesh, Mesh)
	FW_RTTI(Mesh)

	// helper functions for blend weight support
private:
	HRESULT BWFinalize();
	HRESULT BWCopyToBufIndexed();
	HRESULT BWCopyToBufSubmeshed();
	HRESULT BWDisposeStuff();
	void BWNormalizeVertex(FWULONG iVertex);
	FWULONG BWVertexFromFace(FWULONG iFace, FWULONG iVertex);
	void BWVertexToFace(FWULONG iFace, FWULONG iVertex, FWULONG iValue);

protected:
	// buffers & dictionary
	IMeshVertexBuffer *m_pVertexBuffer;
	IMeshFaceBuffer *m_pFaceBuffer;
	IMeshDictionary *m_pDictionary;
	IMaterial *m_pMaterial;

	// byte buffers
	BYTE *m_pVertexBytes;
	FWULONG m_nVertexFirst;
	FWULONG m_nVertexNum;
	FWULONG m_nVertexMaxSize;
	FWULONG m_nVertexItemSize;		// size of vertex

	BYTE *m_pFaceBytes;
	FWULONG m_nFaceFirst;
	FWULONG m_nFaceNum;
	FWULONG m_nFaceMaxSize;
	FWULONG m_nFaceItemSize;		// size of face

	bool m_bHasBytes;

	// visibility
	BOOL m_bOn;

	// mesh transform
	ITransform *m_pTransform;

	// General Buffer Data
	FWULONG m_offsetXYZ, m_sizeXYZ;				// vertex buffer caps: XYZ
	FWULONG m_offsetPointSize, m_sizePointSize;	// vertex buffer caps: Point Size
	FWULONG m_offsetDiffuse, m_sizeDiffuse;		// vertex buffer caps: Diffuse
	FWULONG m_offsetSpecular, m_sizeSpecular;		// vertex buffer caps: Specular
	FWULONG m_offsetTexture[16], m_sizeTexture[16];// vertex buffer caps: Texture
	bool m_bFace32;								// face buffer format: 32-bit index flag

	// Submesh Data
	FWULONG m_nSubmeshNum;		// total number of submeshes
	FWULONG *m_pSubmeshLen;		// start face indices for submeshes (dim: m_nSubmeshNum)
	FWULONG *m_pSubmeshBones;		// bone indices for submeshes (dim: m_nSubmeshNum * m_nBoneNum)

	// Number of Bones (used only by Submesh Support functions)
	FWULONG m_nBoneNum;			// number of bones

	// AddNormal helper data
	FWULONG *m_pCtrl;				// control buffer; contains index of the next synonim; 
	FWULONG m_nCtrlSize;			// size of m_pCtrl buffer
	FWULONG m_nNormalOffset;		// offset to normal position in a vertex struct. Normals to be written as FWFLOAT[3]

	// AddBlendWeight data
	bool m_bBWOn;				// control flag (blend weight support is on)
	FWULONG m_nBWVertexNum;		// number of vertices supported (max of m_nVertexNum, nMinVertexNum)
	FWFLOAT *m_pBWWeights;		// buffer of weights (dim: nVertexNum x nBones)
	FWULONG *m_pBWIndices;		// buffer of indices (dim: nVertexNum x nBones)
	FWFLOAT m_fBWMinWeight;		// minimal weight

public:
	CMesh();
	~CMesh();
};


#endif
