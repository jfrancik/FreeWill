// Camera.h

#pragma once

interface IKineNode;
interface ISceneCamera;
interface IAction;
interface ISceneCamera;

class CCamera
{
	// Camera position & state information
	bool m_bMoved, m_bRotated, m_bZoomed;	// all clear if the camera is as descibed in m_cp; set when moved, rotated or zoomed

	// FreeWill objects
	IKineNode *m_pBaseBone;			// base bone (a part of the building on which the camera is mounted)
	IKineNode *m_pHandleBone;		// camera handle, used to move and pan
	ISceneCamera *m_pCamera;		// camera, autonomically tilts but makes no other transformations

public:
	CCamera();
	~CCamera();

	void SetBaseBone(IKineNode *pNode, bool bKeepCoord = true);

	ISceneCamera *GetCamera()								{ return m_pCamera; }
	IKineNode *GetHandle()									{ return m_pHandleBone; }

	bool IsReady()											{ return m_pCamera != NULL; }

	void GetCurPos(FWVECTOR &pos);
	void GetCurLocalPos(FWVECTOR &pos);
	
	bool Create();
	bool Destroy();

	// Camera Motions
	void Reset();
	void Pan(FWFLOAT f);
	void Tilt(FWFLOAT f);
	void Zoom(FWFLOAT f);
	void Move(FWFLOAT x, FWFLOAT y, FWFLOAT z, IKineNode *pRef = NULL);

	// adjust the camera after aspect ratio change
	void Adjust(FWFLOAT fAspect);
};

