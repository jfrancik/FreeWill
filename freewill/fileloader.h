// fileloader.h
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(__FILELOADER_H)
#define __FILELOADER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"
#include "fileplus.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CFileLoader

class CFileLoader : public FWUNKNOWN<
								BASECLASS<IFileLoader, IFileSink>,
									IID_IFileLoader, IFileLoader, IID_IFileSink, IFileSink>
{
public:
	// IFileSink implementation

	// Reading the entire scene
	virtual HRESULT _stdcall OnBeginScene(LPOLESTR szName, LPOLESTR szFormat);
	virtual HRESULT _stdcall OnEndScene();
	// Reading the Scene Meshed Object main node
	virtual HRESULT _stdcall OnBeginObject(LPOLESTR szName, LPOLESTR szClass);
	virtual HRESULT _stdcall OnEndObject();
	// Reading a kinematic or mesh object
	virtual HRESULT _stdcall OnBeginNode(LPOLESTR szName, LPOLESTR szClass,
		FWMATRIX matrix, FWVECTOR vPos, FWQUAT qRot, FWVECTOR vScale, BOOL bVisible);
	virtual HRESULT _stdcall OnEndNode();
	// Reading the bounding box data
	virtual HRESULT _stdcall OnBB(FWVECTOR vMin, FWVECTOR vMax);
	// Reading the mesh internals
	virtual HRESULT _stdcall OnBeginMesh(FWULONG nVertsCount, FWULONG nFacesCount, FWULONG nTexVertsCount);	
	virtual HRESULT _stdcall OnEndMesh();
	virtual HRESULT _stdcall OnVertex(FWVECTOR vVertex, FWULONG nBonesCount, LPOLESTR *ppsBoneNames, FWFLOAT *pfWeights);	
	virtual HRESULT _stdcall OnTexVertex(FWVECTOR vTexVector);
	virtual HRESULT _stdcall OnFace(FWULONG iVertexA, FWULONG iVertexB, FWULONG iVertexC, 
					   FWULONG iTexVertexA, FWULONG iTexVertexB, FWULONG iTexVertexC,
					   FWVECTOR *pvNormals);
	// Reading various objects
	virtual HRESULT _stdcall OnLight(LPOLESTR szName, LPOLESTR szClass,
		BOOL bIsTarget, FWVECTOR vEye, FWVECTOR vAtDir, 
		FWCOLOR cColor, FWFLOAT fPower, BOOL bActive);
	virtual HRESULT _stdcall OnCamera(LPOLESTR szName, LPOLESTR szClass, 
		BOOL bIsTarget, FWVECTOR vEye, FWVECTOR vAtDir, FWVECTOR vUp,
		FWFLOAT fFOV, FWFLOAT fClipNear, FWFLOAT fClipFar, FWFLOAT fDistance, 
		BOOL bIsOrtho);
	virtual HRESULT _stdcall OnMaterial(FWCOLOR cAmbient, FWCOLOR cDiffuse, FWCOLOR cSpecular,
		FWFLOAT fShininess, FWFLOAT fShinStrength, FWFLOAT fOpacity, 
		FWFLOAT fSelfIllumination, BOOL bSelfIllumColorOn, FWCOLOR cSelfIllumColor, 
		BOOL bTwoSided, BOOL bTextured);
	virtual HRESULT _stdcall OnTexture(LPOLESTR szName, LPOLESTR szType, 
		BYTE* pData, FWULONG nSize, FWFLOAT fUTile, FWFLOAT fVTile);
	virtual HRESULT _stdcall OnLongProperty(LPOLESTR szName, LONG);
	virtual HRESULT _stdcall OnFloatProperty(LPOLESTR szName, FWFLOAT);
	virtual HRESULT _stdcall OnStringProperty(LPOLESTR szName, LPOLESTR);
	virtual HRESULT _stdcall OnVectorProperty(LPOLESTR szName, FWVECTOR);
	virtual HRESULT _stdcall OnQuatProperty(LPOLESTR szName, FWQUAT);
	virtual HRESULT _stdcall OnColorProperty(LPOLESTR szName, FWCOLOR);

	// IFileLoader implementation

	// Load Operations
	virtual HRESULT _stdcall LoadScene(LPOLESTR pFilename, IScene *pScene);
	virtual HRESULT _stdcall LoadObject(LPOLESTR pFilename, LPOLESTR pName, ISceneObject *pObject);

	// The Tuning
	virtual HRESULT _stdcall PutCutPrefix(LPOLESTR p);
	virtual HRESULT _stdcall GetCutPrefix(/*[out, retval]*/ LPOLESTR *p);
	virtual HRESULT _stdcall PutCutPrefixAuto(BOOL b);
	virtual HRESULT _stdcall GetCutPrefixAuto(/*[out, retval]*/ BOOL *p);
	// Configuring the Input File
	virtual HRESULT _stdcall CreateDefFile(LPOLESTR pFilename);
	virtual HRESULT _stdcall GetFile(/*[out, retval]*/ IFileIn**);
	virtual HRESULT _stdcall PutFile(IFileIn*);

public:
	DECLARE_FACTORY_CLASS(FileLoader, FileLoader)
	FW_RTTI(FileLoader)

protected:
	IFileIn *m_pFileIn;

	IScene *m_pLoadScene;				// the scene to be loaded (NULL if LoadObject)
	ISceneObject *m_pLoadObject;		// the object to be loaded (NULL if LoadScene)
	LPOLESTR m_pLoadName;				// name of the object to be loaded

	ISceneObject *m_pObject;			// Scene Meshed Object being currently loaded
	BOOL m_bVisible;
	FWMATRIX m_matrix;
	IMesh *m_pMesh;
	IMesh *m_pMeshForMaterial;
	IKineNode *m_pBone;
	FWULONG m_iV, m_iF, m_iTexV;

	// tuning data
	BOOL m_bCutPrefixAuto;
	LPOLESTR m_pCutPrefix;

public:
	CFileLoader();
	~CFileLoader();
};

#endif
