// renderer.h
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(__RENDERER_H)
#define __RENDERER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"
#include "fwrender.h"

#include "AviFile.h"

#include <D3d9.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// CDX9Renderer

class CDX9Renderer : public FWUNKNOWN<IRenderer, 
											IID_IRenderer, IRenderer,
											IID_IRndrGeneric, IRndrGeneric>
{
public:

	// IRndrGeneric implementation
	virtual HRESULT _stdcall GetBuffers(/*[out]*/ IMeshVertexBuffer**, /*[out]*/ IMeshFaceBuffer**);
	virtual HRESULT _stdcall CreateBuffers(BOOL bVertexBuf, BOOL bFaceBuf);

	virtual HRESULT _stdcall GetTargetSize(FWULONG *pX, FWULONG *pY);
	virtual HRESULT _stdcall GetBackBufferSize(FWULONG *pX, FWULONG *pY);
	virtual HRESULT _stdcall GetViewSize(FWULONG *pX, FWULONG *pY);
	virtual HRESULT _stdcall GetAspectRatio(FWFLOAT*);
	virtual HRESULT _stdcall GetViewport(FWULONG *pX, FWULONG *pY, FWULONG *pWidth, FWULONG *pHeight);
	virtual HRESULT _stdcall GetTargetViewport(FWULONG *pX, FWULONG *pY, FWULONG *pWidth, FWULONG *pHeight);
	virtual HRESULT _stdcall PutViewport(FWFLOAT fX, FWFLOAT fY, FWFLOAT fWidth, FWFLOAT fHeight, FWLONG nX, FWLONG nY, FWLONG nWidth, FWLONG nHeight);
	virtual HRESULT _stdcall PutTargetViewport(FWFLOAT fX, FWFLOAT fY, FWFLOAT fWidth, FWFLOAT fHeight, FWLONG nX, FWLONG nY, FWLONG nWidth, FWLONG nHeight);

	virtual HRESULT _stdcall GetResetFlag(/*[out, retval]*/ BOOL *bReset);

	virtual HRESULT _stdcall RenderMesh(struct RNDR_MESHEDOBJ*);

	virtual HRESULT _stdcall SetLight(FWULONG iIndex, BOOL bOn);
	virtual HRESULT _stdcall GetLight(FWULONG iIndex);
	virtual HRESULT _stdcall SetPointLight(FWULONG iIndex, FWCOLOR clrDiff, FWCOLOR clrSpec, FWCOLOR clrAmb, FWVECTOR *pPos, FWFLOAT fRange, FWFLOAT fAtten0, FWFLOAT fAtten1, FWFLOAT fAtten2);
	virtual HRESULT _stdcall SetDirLight(FWULONG iIndex, FWCOLOR clrDiff, FWCOLOR clrSpec, FWCOLOR clrAmb, FWVECTOR *pDir);
	virtual HRESULT _stdcall SetSpotLight(FWULONG iIndex, FWCOLOR clrDiff, FWCOLOR clrSpec, FWCOLOR clrAmb, FWVECTOR *pPos, FWVECTOR *pDir, FWFLOAT fTheta, FWFLOAT fPhi, FWFLOAT fFalloff);
	virtual HRESULT _stdcall SetAmbientLight(FWCOLOR clrAmb);

	virtual HRESULT _stdcall GetViewTransform(/*[out, retval]*/ ITransform **pVal);
	virtual HRESULT _stdcall PutViewTransform(ITransform *newVal);
	virtual HRESULT _stdcall GetProjectionTransform(/*[out, retval]*/ ITransform **pVal);
	virtual HRESULT _stdcall PutProjectionTransform(ITransform *newVal);

	// attributes
	virtual HRESULT _stdcall GetBackColor(/*[out, retval]*/ FWCOLOR *pVal);
	virtual HRESULT _stdcall PutBackColor(FWCOLOR newVal);

	// drawing functions
	virtual HRESULT _stdcall InitDisplay(HWND hWnd, FWULONG nWidth, FWULONG nHeight);
	virtual HRESULT _stdcall InitOffScreen(FWULONG nWidth, FWULONG nHeight);
	virtual HRESULT _stdcall DoneDisplay();
	virtual HRESULT _stdcall DoneOffScreen();
	virtual HRESULT _stdcall GetWindow(HWND *phWnd);
	virtual HRESULT _stdcall PutWindow(HWND hWnd);
	virtual HRESULT _stdcall SetCallback(enum FW_RENDER_CB_TYPE, FW_RENDER_CB_HOOK, FWULONG nParam, void *pParam);
	virtual HRESULT _stdcall IsDeviceAvailable();
	virtual HRESULT _stdcall ResetDevice();
	virtual HRESULT _stdcall ResetDeviceEx(HWND hWnd, FWULONG nWidth, FWULONG nHeight);
	virtual HRESULT _stdcall Clear();
	virtual HRESULT _stdcall BeginFrame();
	virtual HRESULT _stdcall EndFrame();

	virtual HRESULT _stdcall SetTargetToScreen();
	virtual HRESULT _stdcall SetTargetOffScreen();

	virtual HRESULT _stdcall OpenStillFile(LPCTSTR pFilename, enum FW_RENDER_BITMAP fmt);
	virtual HRESULT _stdcall CloseStillFile();
	virtual HRESULT _stdcall OpenMovieFile(LPCTSTR pFilename, FWULONG nFPS);
	virtual HRESULT _stdcall CloseMovieFile();

	// animation control
	virtual HRESULT _stdcall Play();
	virtual HRESULT _stdcall IsPlaying();
	virtual HRESULT _stdcall Pause();
	virtual HRESULT _stdcall IsPaused();
	virtual HRESULT _stdcall Stop();
	virtual HRESULT _stdcall GetAccel(/*[out, retval]*/ FWFLOAT *pA);
	virtual HRESULT _stdcall PutAccel(FWFLOAT nA);
	virtual HRESULT _stdcall GetTotalPlayingTime(/*[out, retval]*/ FWULONG *pnMSec);
	virtual HRESULT _stdcall PutTotalPlayingTime(FWULONG nMSec);
	virtual HRESULT _stdcall GetPlayTime(/*[out, retval]*/ FWULONG *pnMSec);
	virtual HRESULT _stdcall PutPlayTime(FWULONG pnMSec);

	// Low level access
	virtual HRESULT _stdcall GetDeviceHandle(FWULONG nId, /*[out, retval]*/ FWHANDLE *pHandle);

	// texture 
	virtual HRESULT _stdcall CreateTexture(ITexture** ppTexture);

	// error helper
	HRESULT D3DError(HRESULT nD3DErrorCode);

	DECLARE_FACTORY_CLASS(DX9Renderer, Renderer)
	FW_RTTI(DX9Renderer)
	FW_ERROR_BEGIN
		FW_ERROR_ENTRY(REND_E_PLATFORM,		L"DirectX Error", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(REND_E_PLATFORM_EX,	L"DirectX Error: \"%s\"", FW_SEV_CRITICAL)
	FW_ERROR_END

protected:
	IDirect3D9 *m_pD3D;						// DX9
	IDirect3DDevice9 *m_pDevice;			// DX9 Device

	D3DPRESENT_PARAMETERS m_d3dpp;			// presentation parameters

	BOOL m_bFrozen;							// frozen flag, set when device is lost, cleared by successful EndFrame
	BOOL m_bReset;							// reset flag, set by Reset, cleared by EndFrame

	HWND m_hWnd;							// the rendering window
	FWCOLOR m_colorBack;					// the background colour
	D3DCOLOR m_DXBackColor;					// the background colour in DX9 standard

	// array of callback functions and their params
	struct
	{
		FW_RENDER_CB_HOOK pFn;
		FWULONG nParam;
		void *pParam;
	} m_ppfnCB[FW_CB_MAX];

	ITransform *m_pITransformView;			// the View Transform
	ITransform *m_pITransformProjection;	// the Projection Transform

	IMeshVertexBuffer *m_pVertexBuffer;		// the vertex buffer prvided by the renderer
	IMeshFaceBuffer *m_pFaceBuffer;			// the face buffer prvided by the renderer

	DWORD m_nTotalTime;						// the animation total time
	DWORD m_nStartTime;						// the animation start time
	DWORD m_nLastTime;						// time of the last successful frame - used by Pause function
	FWFLOAT m_fAccel;						// the accelaration factor (0.5=2xslower, 1=normal, 2=2xfaster)
	BOOL m_bPause;							// the pause flag

	// OffScreen Device Variables
	bool m_bOnScreen;						// set if offscreen rendering active
	IDirect3DSurface9 *m_pOffsSurface,		// the offscreen rendering surface
	                  *m_pOffsDS,			// the offscreen depth stencil buffer
	                  *m_pBackSurface,		// backup of the rendering back buffer
					  *m_pBackDS;			// backup of the rendering depth stencil buffer
	FWULONG m_nOffsW, m_nOffsH;				// size of the offscreen image (not necessarily the buffer)
	FWSTRING m_pStillFilename;
	FW_RENDER_BITMAP m_fmtStill;
	CAviFile *m_pAviFile;					// AVI file

	// helper functions
	HRESULT CDX9Renderer::__SetInitialRenderStates();


public:
	CDX9Renderer() : 
		m_pD3D(NULL), m_pDevice(NULL), m_hWnd(NULL), 
		m_bFrozen(false), m_bReset(true),
		m_DXBackColor(D3DCOLOR_XRGB(0, 0, 224)), 
        m_pITransformView(NULL), m_pITransformProjection(NULL),
		m_pVertexBuffer(NULL), m_pFaceBuffer(NULL), 
		m_nTotalTime(0), m_nStartTime(0), m_nLastTime(0), m_fAccel(1.0f), m_bPause(false),
		m_bOnScreen(true),
		m_pOffsSurface(NULL), m_pOffsDS(NULL), m_pBackSurface(NULL), m_pBackDS(NULL),
		m_nOffsW(0), m_nOffsH(0), m_pStillFilename(NULL), m_pAviFile(NULL) 
	{ memset(m_ppfnCB, 0, sizeof(m_ppfnCB)); }

	~CDX9Renderer()				{ DoneDisplay(); }
};

#endif
