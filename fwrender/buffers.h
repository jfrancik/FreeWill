// buffers.h
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(__BUFFERSDX_H)
#define __BUFFERSDX_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"
#include "meshplus.h"

struct IDirect3DDevice9;
struct IDirect3DVertexBuffer9;
struct IDirect3DIndexBuffer9;

////////////////////////////////////////////////////////////////////////////////////////////////////
// CMeshDX9VertexBuffer

class CMeshDX9VertexBuffer : public FWUNKNOWN<
										IMeshVertexBuffer,
											IID_IMeshVertexBuffer, IMeshVertexBuffer, 
											IID_IMeshBuffer, IMeshBuffer>
{
public:
	// IFWUnknown - Context Functions
	virtual HRESULT _stdcall PutContextObject(FWULONG index, REFIID iid, void *pUnknown);
	virtual HRESULT _stdcall GetContextObject(FWULONG index, /*[out]*/ IID *pIID, /*[out]*/ void **pUnknown);

	// IMeshBuffer implementation
public:
	// Attributes
	virtual HRESULT _stdcall GetItemSize(FWULONG *p)		{ *p = m_nItemSize; return S_OK; }
	virtual HRESULT _stdcall GetBufSize(FWULONG *p)		{ *p = m_nBufSize; return S_OK; }
	virtual HRESULT _stdcall GetDataSize(FWULONG *p)		{ *p = m_nDataSize; return S_OK; }
	virtual HRESULT _stdcall PutDataSize(FWULONG n)		{ m_nDataSize = n; return S_OK; }
	virtual HRESULT _stdcall GetFreeSize(FWULONG *p)		{ *p = m_nBufSize - m_nDataSize; return S_OK; }

	virtual HRESULT _stdcall Open(enum MESH_OPENMODES mode, FWULONG nFirst, FWULONG nSize, /*[out, retval]*/ BYTE **pBytes);
	virtual HRESULT _stdcall OpenRead(/*[out, retval]*/ BYTE **pBytes)		{ return Open(MESH_OPEN_READ, 0, -1, pBytes); }
	virtual HRESULT _stdcall OpenAppend(/*[out, retval]*/ BYTE **pBytes)	{ return Open(MESH_OPEN_APPEND, 0, 0, pBytes); }
	virtual HRESULT _stdcall Close(FWULONG nLength);

	virtual HRESULT _stdcall BeforeRendering();
	virtual HRESULT _stdcall AfterRendering();

	// IMeshVertexBuffer implementation

	// Buffer construction - see MESH_VERTEX_???? constant definitions
	virtual HRESULT _stdcall Create(FWULONG nSize, FWULONG nFormat, FWULONG nBones, FWULONG nTextures);
	virtual HRESULT _stdcall Destroy();

	// Attributes: buffer length (count of vertices allocated), vertex size, vertex capabilities (fields)
	virtual HRESULT _stdcall GetFormat(/*[out]*/ FWULONG *nFormat, /*[out]*/ FWULONG *nBones, /*[out]*/ FWULONG *nTextures);
	virtual HRESULT _stdcall GetCaps(enum MESH_VERTEXID nFlag, FWULONG nIndex, /*[out]*/ FWULONG *pOffset, /*[out,retval]*/ FWULONG *pSize);

	DECLARE_FACTORY_CLASS(MeshDX9VertexBuffer, MeshVertexBuffer)
	FW_RTTI(MeshDX9VertexBuffer)

	// error helper
	HRESULT D3DError(HRESULT nD3DErrorCode);

protected:
	FWULONG m_nItemSize;		// vertex size
	FWULONG m_nBufSize;		// size of the buffer (in items)
	FWULONG m_nDataSize;		// effective length of the data

protected:
	FWULONG m_nFormat;	// actual creation format flags
	FWULONG m_nBones;		// actual count of bones
	FWULONG m_nTextures;	// actual count of texture coordinates

	struct DESCRIPTION_TAG {
		FWULONG offset;
		FWULONG size;
	} m_description[MESH_VERTEXID_RESERVED1];

	IDirect3DDevice9 *m_pDevice;		// DX device
	IDirect3DVertexBuffer9 *m_pBuffer;	// DX buffer
	FWULONG m_fvf;						// DX FVF: flexible vertex format

public:
	CMeshDX9VertexBuffer() : m_nItemSize(0), m_nBufSize(0), m_nDataSize(0), m_pDevice(NULL), m_pBuffer(NULL)		
										{ memset(&m_description, 0, sizeof(m_description)); }
	~CMeshDX9VertexBuffer()				{ Destroy(); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CMeshDX9FaceBuffer

class CMeshDX9FaceBuffer : public FWUNKNOWN<
										IMeshFaceBuffer,
											IID_IMeshFaceBuffer, IMeshFaceBuffer, 
											IID_IMeshBuffer, IMeshBuffer>
{
public:
	// IFWUnknown - Context Functions
	virtual HRESULT _stdcall PutContextObject(FWULONG index, REFIID iid, void *pUnknown);
	virtual HRESULT _stdcall GetContextObject(FWULONG index, /*[out]*/ IID *pIID, /*[out]*/ void **pUnknown);

	// IMeshBuffer implementation
public:
	// Attributes
	virtual HRESULT _stdcall GetItemSize(FWULONG *p)		{ *p = m_nItemSize; return S_OK; }
	virtual HRESULT _stdcall GetBufSize(FWULONG *p)		{ *p = m_nBufSize; return S_OK; }
	virtual HRESULT _stdcall GetDataSize(FWULONG *p)		{ *p = m_nDataSize; return S_OK; }
	virtual HRESULT _stdcall PutDataSize(FWULONG n)		{ m_nDataSize = n; return S_OK; }
	virtual HRESULT _stdcall GetFreeSize(FWULONG *p)		{ *p = m_nBufSize - m_nDataSize; return S_OK; }

	virtual HRESULT _stdcall Open(enum MESH_OPENMODES mode, FWULONG nFirst, FWULONG nSize, /*[out, retval]*/ BYTE **pBytes);
	virtual HRESULT _stdcall OpenRead(/*[out, retval]*/ BYTE **pBytes)		{ return Open(MESH_OPEN_READ, 0, -1, pBytes); }
	virtual HRESULT _stdcall OpenAppend(/*[out, retval]*/ BYTE **pBytes)	{ return Open(MESH_OPEN_APPEND, 0, 0, pBytes); }
	virtual HRESULT _stdcall Close(FWULONG nLength);

	virtual HRESULT _stdcall BeforeRendering();
	virtual HRESULT _stdcall AfterRendering();

	// IMeshFaceBuffer implementation
	virtual HRESULT _stdcall Create(FWULONG nSize);
	virtual HRESULT _stdcall Destroy();
	virtual HRESULT _stdcall GetFormat(FWULONG *nBitsPerIndex);

	DECLARE_FACTORY_CLASS(MeshDX9FaceBuffer, MeshFaceBuffer)
	FW_RTTI(MeshDX9FaceBuffer)

protected:
	FWULONG m_nItemSize;		// vertex size
	FWULONG m_nBufSize;		// size of the buffer (in items)
	FWULONG m_nDataSize;		// effective length of the data


protected:
	IDirect3DDevice9 *m_pDevice;		// DX device
	IDirect3DIndexBuffer9 *m_pBuffer;	// DX buffer

public:
	CMeshDX9FaceBuffer() : m_nItemSize(0), m_nBufSize(0), m_nDataSize(0), m_pDevice(NULL), m_pBuffer(NULL)		
							{ }
	~CMeshDX9FaceBuffer()	{ Destroy(); }
};

#endif
