//------------------------------------------------------------------------------
// Project: FreeWill+
// Module:  kinereader.h
// Author:  Marek Mittmann
// Date:    25.11.2004 (last modification: 22.09.2005)
// Desc.:   
//------------------------------------------------------------------------------

#ifndef _KINEREADER_H__INCLUDED_
#define _KINEREADER_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "kinefile.h"

//------------------------------------------------------------------------------
// Added by: Jarek Francik
// Comment:  two include files added
#include "fileplus.h"
#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"


class CKineReaderException: public CKineFileException
{
public:
	CKineReaderException(const char* szMsg): CKineFileException(szMsg) { }
};


class CKineReader: public CKineFile, public FWUNKNOWN<IFileIn, IID_IFileIn, IFileIn >
{
//------------------------------------------------------------------------------
// Added by: Jarek Francik
// Comment: IFileIn inheritance added to the class
// Comment: Extends the class to implement IUnknown and IFileIn interfaces
public:
	// IFileIn implementation
	virtual HRESULT _stdcall Read(IFileSink *pSink, LPOLESTR pFilename);
private:
	IFileSink *m_pSink;
	FWULONG m_nCount;
public:
	DECLARE_FACTORY_CLASS(KineReader, FileIn)
	FW_RTTI(KineReader)
	FW_ERROR_BEGIN
		FW_ERROR_ENTRY(FILE_E_FILESYSTEM,		L"File system error: %ls", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FILE_E_FILESYNTAX,		L"Syntax error in file: %ls", FW_SEV_CRITICAL)
	FW_ERROR_END

//------------------------------------------------------------------------------
// Added by: Jarek Francik
// Comment: Additional data for generating AABB bounding info

private:
	BOOL m_bBoxAdded;
	float m_vfBBoxMin[3];
	float m_vfBBoxMax[3];
	
// End of Added by: Jarek Francik
//------------------------------------------------------------------------------

public:
	union Value
	{
		bool asBool;
		int asInt;
		unsigned int asRGB;
		float asFloat;
		struct { float x, y, z; } asPoint;
		struct { float x, y, z, w; } asQuat;
		char* asString;
	};

	CKineReader();
	virtual ~CKineReader();
	
	void Read(const char* szFileName);
	
	const char* CrtObjectName();
	const char* CrtObjectClass();
	int ItemIndex();
	
	void OnReadScene(const char* szName, const char* szFormat);
	void OnEndScene();
	void OnReadNodes(int nCount);
	void OnEndNodes();
	void OnReadObject(const char* szName, const char* szClass,
		bool bVisible, bool bRenderable, float mfMatrix[4][3], 
		float vfPos[3], float vfRot[4], float vfScale[3],
		float vfBBoxMin[3], float vfBBoxMax[3]);
	void OnEndObject();
	void OnReadLight(const char* szName, const char* szClass, Token tkType,
		bool bActive, float vfPosition[3], float vfDirection[3], 
		bool bIsTarget, float vfTargetPos[3], float vfColor[3], float fPower);
	void OnReadCamera(const char* szName, const char* szClass, bool bIsOrtho, 
		float vfPosition[3], float vfDirection[3], float vfUpVector[3], 
		float fDistance, bool bIsTarget, float vfTargetPos[3],
		float fFOV, float fClipNear, float fClipFar);
	void OnReadMaterial(float vfAmbient[3], float vfDiffuse[3], float vfSpecular[3],
		float fShininess, float fShinStrength, float fOpacity, 
		float fSelfIllumination, bool bSelfIllumColorOn, float vfSelfIllumColor[3], 
		bool bTwoSided, bool bTextured);
	void OnReadTexture(const char* szName, const char* szType, 
		const char* pData, int nSize, float fUTile, float fVTile);
	void OnReadProperty(const char* szName, Token tkType, Value& value);
	void OnReadMesh(int nVertsCount, int nFacesCount, int nTexVertsCount);
	void OnEndMesh();
	void OnReadVertex(float vf[3], int nBonesCount, 
		char* vszBoneNames[], float vfWeights[]);
	void OnReadTexVertex(float vf[3]);
	void OnReadFace(int nSmoothGroup, bool bVisible, int nEdges, 
		int viVerts[3], int viTexVerts[3], 
		float vfFaceNormal[3], float mfVertNormals[3][3]);
	void OnError();
	
protected:
	void ErrUnexpectedSymbol(Token symbol);
	void ErrWrongValue();
	void ReadBinToken(Token& tk);
	void PutBinTokenBack();
	void ReadBinStr(char* szResult);
	void ReadBinProperty(bool sStandard, Token& tkStdName, char* szName, 
		Token& tkType, Value& val);
	void ReadBinScene();
	void ReadBinNodes();
	void ReadBinLight();
	void ReadBinCamera();
	void ReadBinObject();
	void ReadBinMesh();
	void ReadBinMaterial();
};
 
 #endif