// fwrender.cpp : Defines the entry point for the DLL application.
// 

#include "stdafx.h" 

#include "buffers.h"
#include "renderer.h"
#include "texture.h"

#include "fwrender.h"
#include "fwrender_i.c"
#include "transplus_i.c"
#include "meshplus_i.c"
#include "matplus_i.c"
#include "common_i.c"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Factory Map 

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
MIDL_DEFINE_GUID(IID, IID_IRndrGeneric,0x30D1CBA6,0x37EF,0x4d60,0x94,0x75,0xD4,0xC7,0x9D,0x14,0x7A,0xD7);

BEGIN_FACTORY_MAP
	FACTORY_CLASS(MeshDX9VertexBuffer,  L"Mesh Plus DirectX9-based Vertex Buffer", L"MeshPlus.VertexBuffer", L"MeshPlus.VertexBuffer.1")
	FACTORY_CLASS(MeshDX9FaceBuffer,  L"Mesh Plus DirectX9-based Face Buffer", L"MeshPlus.FaceBuffer", L"MeshPlus.FaceBuffer.1")
	FACTORY_CLASS(DX9Texture,  L"Mat Plus DirectX9-based Texture Object", L"MatPlus.Texture", L"MatPlus.Texture.1")
	FACTORY_CLASS(DX9Renderer,  L"FWRender DirectX9-based Renderer", L"FWRender.Renderer", L"FWRender.Renderer.1")
END_FACTORY_MAP

BEGIN_FREEWILL_MAP
	FREEWILL_CLASS(L"Renderer",			L"Generic",		DX9Renderer)
END_FREEWILL_MAP

#ifdef USE_MFC
class CMyApp : public CWinApp
{
    public: virtual BOOL InitInstance() { DllMainHelper(AfxGetInstanceHandle(), DLL_PROCESS_ATTACH); return TRUE; }
};
CMyApp myApp;
#else 
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    return DllMainHelper(hModule, reason);
}
#endif 


