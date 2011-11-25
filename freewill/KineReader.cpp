//------------------------------------------------------------------------------
// Project: FreeWill+
// Module:  kinereader.cpp
// Author:  Marek Mittmann
// Date:    25.11.2004 (last modification: 22.09.2005)
// Desc.:   
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "kinereader.h"
#include "strformat.h"
#include <iostream>

using namespace std;

const int MAX_STR_LEN		= 255;
const int MAX_STR_BUFF_SIZE	= 256;


CKineReader::CKineReader():
	CKineFile(),
	m_pSink(NULL),
	m_nCount(0)
{
//------------------------------------------------------------------------------
// Added by: Jarek Francik
// Comment: increment component counter (see factory.cpp for details)
	//m_pSink = 0;
	//m_nCount = 0;
}

CKineReader::~CKineReader()
{
//------------------------------------------------------------------------------
// End of Added by: Jarek Francik
}
	
void CKineReader::Read(const char* szFileName)
{
	m_bBinaryMode = true;
	if (stricmp(ExtractFileExt(szFileName), DefaultFileExt()+1) != 0)
		m_bBinaryMode = false;
	Open(szFileName);
	try
	{
		if (m_bBinaryMode)
			ReadBinScene();
		Close();
	}
	catch(...)
	{
		Close();
		throw;
	}	
}

void CKineReader::ErrUnexpectedSymbol(Token symbol)
{
	OnError();
	int nPos = (int)m_stream.tellg();
	throw CKineReaderException(FormatStr(MSG_E_UNEXPECTED_SYMBOL_AT, 
		TokenToString(symbol), nPos));
}

void CKineReader::ErrWrongValue()
{
	OnError();
	int nPos = (int)m_stream.tellg();
	throw CKineReaderException(FormatStr(MSG_E_WRONG_VALUE_AT, nPos));
}

void CKineReader::ReadBinToken(Token& tk)
{
	m_stream.read((char*)&tk, SIZE_OF_TOKEN);
	if (m_stream.eof())
		tk = TK_EOF;	
}

void CKineReader::PutBinTokenBack()
{
	if (m_stream.eof())
	{
		m_stream.clear();
		m_stream.seekg(-SIZE_OF_TOKEN, ios_base::end);
	}
	else
		m_stream.seekg(-SIZE_OF_TOKEN, ios_base::cur);
}

void CKineReader::ReadBinStr(char* szResult)
{
	size_t len;
	m_stream.read((char*)&len, sizeof(size_t));
	len++;
	if (len > MAX_STR_LEN)
		len = MAX_STR_LEN;
	m_stream.read(szResult, (int)len);
}

const char* CKineReader::CrtObjectName()
{
	return NULL;
}

const char* CKineReader::CrtObjectClass()
{
	return NULL;
}

int CKineReader::ItemIndex()
{
	return 0;
}

void CKineReader::ReadBinProperty(bool bStandard, Token& tkStdName, char* szName, 
	Token& tkType, Value& val)
{
	static char sPropValue[MAX_STR_BUFF_SIZE];
	
	if (bStandard)
	{
		ReadBinToken(tkStdName);	
		strcpy(szName, TokenToString(tkStdName));
	}
	else
	{
		tkStdName = TK_UNDEFINED;
		ReadBinStr(szName);
	}
	tkType = TK_UNDEFINED;
	ReadBinToken(tkType);	
	switch (tkType)
	{
		case TK_QUAT:
			m_stream.read((char*)&val, 4*sizeof(float));
			break;
		case TK_POINT:
			m_stream.read((char*)&val, 3*sizeof(float));
			break;
		case TK_FLOAT:
			m_stream.read((char*)&val, sizeof(float));
			break;
		case TK_INT:
		case TK_RGB:
			m_stream.read((char*)&val, sizeof(int));				
			break;
		case TK_BOOL:
			m_stream.read((char*)&val, sizeof(bool));				
			break;
		case TK_STRING:
			ReadBinStr(sPropValue);
			val.asString = sPropValue;
			break;
		default:
			ErrUnexpectedSymbol(tkType);
	}
}

void CKineReader::ReadBinScene()
{
	Token tk = TK_UNDEFINED;
	char sName[MAX_STR_BUFF_SIZE], sFormat[MAX_STR_BUFF_SIZE];

	ReadBinToken(tk);	
	if (tk != TK_SCENE)
		ErrUnexpectedSymbol(tk);
	ReadBinStr(sName);
	ReadBinStr(sFormat);
	
	OnReadScene(sName, sFormat);
	
	ReadBinToken(tk);	
	if (tk == TK_LINK_NODES)
		ReadBinNodes();
		
	OnEndScene();
}

void CKineReader::ReadBinNodes()
{
	Token tk = TK_UNDEFINED;
	int nNodesCount = 0;
	
	m_stream.read((char*)&nNodesCount, sizeof(int));
	
	OnReadNodes(nNodesCount);
	
	for (int i=0; i < nNodesCount; i++)
	{
		ReadBinToken(tk);	
		switch (tk)
		{
			case TK_LIGHT:
				ReadBinLight();
				break;
			case TK_CAMERA:
				ReadBinCamera();
				break;
			case TK_OBJECT:
				ReadBinObject();
				break;
			default:
				ErrUnexpectedSymbol(tk);
		}	
	}
	
	OnEndNodes();
}

void CKineReader::ReadBinLight()
{
	Token tk = TK_UNDEFINED, tkType = TK_UNDEFINED;
	char sName[MAX_STR_BUFF_SIZE], sClass[MAX_STR_BUFF_SIZE];
	bool bActive = false, bIsTarget = false;
	float vfPosition[3] = {0, 0, 0};
	float vfDirection[3] = {0, 0, 0};
	float vfTargetPos[3] = {0, 0, 0};
	float vfColor[3] = {0, 0, 0};
	float fPower = 0;
	
	ReadBinStr(sName);
	ReadBinStr(sClass);
	ReadBinToken(tkType);	
	
	while (true)
	{
		Token tkPropType = TK_UNDEFINED;
		char sPropName[MAX_STR_BUFF_SIZE];
		Value propVal;
		
		ReadBinToken(tk);	
		if (tk != TK_PROPERTY && tk != TK_STD_PROPERTY)
		{
			PutBinTokenBack();
			break;
		}
		ReadBinProperty(tk == TK_STD_PROPERTY, tk, sPropName, tkPropType, propVal);
		switch (tk)
		{
			case TK_ACTIVE:
				if (tkPropType != TK_BOOL)
					ErrWrongValue();
				bActive = propVal.asBool;
				break;
			case TK_TARGET_POSITION:
				if (tkPropType != TK_POINT)
					ErrWrongValue();
				bIsTarget = true;
				vfTargetPos[0] = propVal.asPoint.x;
				vfTargetPos[1] = propVal.asPoint.y;
				vfTargetPos[2] = propVal.asPoint.z;
				break;
			case TK_POSITION:
				if (tkPropType != TK_POINT)
					ErrWrongValue();
				vfPosition[0] = propVal.asPoint.x;
				vfPosition[1] = propVal.asPoint.y;
				vfPosition[2] = propVal.asPoint.z;
				break;
			case TK_DIRECTION:
				if (tkPropType != TK_POINT)
					ErrWrongValue();
				vfDirection[0] = propVal.asPoint.x;
				vfDirection[1] = propVal.asPoint.y;
				vfDirection[2] = propVal.asPoint.z;
				break;
			case TK_COLOR:
				if (tkPropType != TK_POINT)
					ErrWrongValue();
				vfColor[0] = propVal.asPoint.x;
				vfColor[1] = propVal.asPoint.y;
				vfColor[2] = propVal.asPoint.z;
				break;
			case TK_POWER:
				if (tkPropType != TK_FLOAT)
					ErrWrongValue();
				fPower = propVal.asFloat;
				break;
		}
	}
	
	OnReadLight(sName, sClass, tkType, bActive, vfPosition, vfDirection, 
		bIsTarget, vfTargetPos, vfColor, fPower);
}

void CKineReader::ReadBinCamera()
{
	Token tk = TK_UNDEFINED;
	char sName[MAX_STR_BUFF_SIZE], sClass[MAX_STR_BUFF_SIZE];
	bool bIsOrtho = false, bIsTarget = false;
	float vfPosition[3] = {0, 0, 0};
	float vfUpVector[3] = {0, 0, 0};
	float vfDirection[3] = {0, 0, 0};
	float vfTargetPos[3] = {0, 0, 0};
	float fDistance = 0, fFOV = 0, fClipNear = 0, fClipFar = 0;
	
	ReadBinStr(sName);
	ReadBinStr(sClass);
	
	while (true)
	{
		Token tkPropType = TK_UNDEFINED;
		char sPropName[MAX_STR_BUFF_SIZE];
		Value propVal;
		
		ReadBinToken(tk);	
		if (tk != TK_PROPERTY && tk != TK_STD_PROPERTY)
		{
			PutBinTokenBack();
			break;
		}
		ReadBinProperty(tk == TK_STD_PROPERTY, tk, sPropName, tkPropType, propVal);
		switch (tk)
		{
			case TK_ISORTHO:
				if (tkPropType != TK_BOOL)
					ErrWrongValue();
				bIsOrtho = propVal.asBool;
				break;
			case TK_TARGET_POSITION:
				if (tkPropType != TK_POINT)
					ErrWrongValue();
				bIsTarget = true;
				vfTargetPos[0] = propVal.asPoint.x;
				vfTargetPos[1] = propVal.asPoint.y;
				vfTargetPos[2] = propVal.asPoint.z;
				break;
			case TK_POSITION:
				if (tkPropType != TK_POINT)
					ErrWrongValue();
				vfPosition[0] = propVal.asPoint.x;
				vfPosition[1] = propVal.asPoint.y;
				vfPosition[2] = propVal.asPoint.z;
				break;
			case TK_DIRECTION:
				if (tkPropType != TK_POINT)
					ErrWrongValue();
				vfDirection[0] = propVal.asPoint.x;
				vfDirection[1] = propVal.asPoint.y;
				vfDirection[2] = propVal.asPoint.z;
				break;
			case TK_UPVECTOR:
				if (tkPropType != TK_POINT)
					ErrWrongValue();
				vfUpVector[0] = propVal.asPoint.x;
				vfUpVector[1] = propVal.asPoint.y;
				vfUpVector[2] = propVal.asPoint.z;
				break;
			case TK_DISTANCE:
				if (tkPropType != TK_FLOAT)
					ErrWrongValue();
				fDistance = propVal.asFloat;
			case TK_FOV:
				if (tkPropType != TK_FLOAT)
					ErrWrongValue();
				fFOV = propVal.asFloat;
			case TK_CLIP_NEAR:
				if (tkPropType != TK_FLOAT)
					ErrWrongValue();
				fClipNear = propVal.asFloat;
			case TK_CLIP_FAR:
				if (tkPropType != TK_FLOAT)
					ErrWrongValue();
				fClipFar = propVal.asFloat;
				break;
		}
	}
	
	OnReadCamera(sName, sClass, bIsOrtho, vfPosition, vfDirection, vfUpVector,
		fDistance, bIsTarget, vfTargetPos, fFOV, fClipNear, fClipFar);
}

void CKineReader::ReadBinObject()
{
	Token tk = TK_UNDEFINED, tkPropType = TK_UNDEFINED;
	char sName[MAX_STR_BUFF_SIZE], sClass[MAX_STR_BUFF_SIZE]; 
	char sPropName[MAX_STR_BUFF_SIZE];
	bool bVisible = false, bRenderable = false, bWasOnObject = false;
	float mfTransform[4][3];
	float vfPos[3] = {0, 0, 0};
	float vfRot[4] = {0, 0, 0, 0};
	float vfScale[3] = {0, 0, 0};
	float vfBBoxMin[3] = {0, 0, 0};
	float vfBBoxMax[3] = {0, 0, 0};
	Value propVal;
	
	ReadBinStr(sName);
	ReadBinStr(sClass);
	
	while (true)
	{
		ReadBinToken(tk);	
		if (!bWasOnObject && tk != TK_TRANSFORM_MATRIX && tk != TK_STD_PROPERTY)
		{
			OnReadObject(sName, sClass, bVisible, bRenderable, mfTransform, 
				vfPos, vfRot, vfScale, vfBBoxMin, vfBBoxMax);
			bWasOnObject = true;
		}
		switch (tk)
		{
			case TK_STD_PROPERTY:
				ReadBinProperty(true, tk, sPropName, tkPropType, propVal);
				switch (tk)
				{
					case TK_VISIBLE:
						if (tkPropType != TK_BOOL)
							ErrWrongValue();
						bVisible = propVal.asBool;
						break;
					case TK_RENDERABLE:
						if (tkPropType != TK_BOOL)
							ErrWrongValue();
						bRenderable = propVal.asBool;
						break;
					case TK_POSITION:
						if (tkPropType != TK_POINT)
							ErrWrongValue();
						vfPos[0] = propVal.asPoint.x;
						vfPos[1] = propVal.asPoint.y;
						vfPos[2] = propVal.asPoint.z;
						break;
					case TK_ROTATION:
						if (tkPropType != TK_QUAT)
							ErrWrongValue();
						vfRot[0] = propVal.asQuat.x;
						vfRot[1] = propVal.asQuat.y;
						vfRot[2] = propVal.asQuat.z;
						vfRot[3] = propVal.asQuat.w;
						break;
					case TK_SCALE:
						if (tkPropType != TK_POINT)
							ErrWrongValue();
						vfScale[0] = propVal.asPoint.x;
						vfScale[1] = propVal.asPoint.y;
						vfScale[2] = propVal.asPoint.z;
						break;
					case TK_BBOX_MIN:
						if (tkPropType != TK_POINT)
							ErrWrongValue();
						vfBBoxMin[0] = propVal.asPoint.x;
						vfBBoxMin[1] = propVal.asPoint.y;
						vfBBoxMin[2] = propVal.asPoint.z;
						break;
					case TK_BBOX_MAX:
						if (tkPropType != TK_POINT)
							ErrWrongValue();
						vfBBoxMax[0] = propVal.asPoint.x;
						vfBBoxMax[1] = propVal.asPoint.y;
						vfBBoxMax[2] = propVal.asPoint.z;
						break;
				}		
				break;
			case TK_PROPERTY:
				ReadBinProperty(false, tk, sPropName, tkPropType, propVal);
				OnReadProperty(sPropName, tkPropType, propVal);
				break;
			case TK_TRANSFORM_MATRIX:
				for (int i=0; i < 4; i++)
				{
					ReadBinToken(tk);	
					if (tk != TK_VERTEX)
						ErrUnexpectedSymbol(tk);
					m_stream.read((char*)mfTransform[i], 3*sizeof(float));
				}
				break;
			case TK_MESH:
				ReadBinMesh();
				break;
			case TK_LINK_NODES:
				ReadBinNodes();	
				break;
			case TK_MATERIAL:
				ReadBinMaterial();
				break;
			default:
				PutBinTokenBack();
				OnEndObject();
				return;
		}
	}
}

void CKineReader::ReadBinMaterial()
{
	Token tk = TK_UNDEFINED;
	bool bTwoSided = false, bTextured = false, bSelfIllumColorOn = false;
	float vfAmbient[3] = {0.3f, 0.3f, 0.3f};
	float vfDiffuse[3] = {0.9f, 0.9f, 0.9f};
	float vfSpecular[3] = {0.9f, 0.9f, 0.9f};
	float vfSelfIllumColor[3] = {0, 0, 0};
	float fShininess = 10.0f, fShinStrength = 1.0f;
	float fSelfIllumination = 0.0f, fOpacity = 1.0f;
	
	while (true)
	{
		Token tkPropType = TK_UNDEFINED;
		char sPropName[MAX_STR_BUFF_SIZE];
		Value propVal;
		
		ReadBinToken(tk);	
		if (tk != TK_PROPERTY && tk != TK_STD_PROPERTY)
		{
			PutBinTokenBack();
			break;
		}
		ReadBinProperty(tk == TK_STD_PROPERTY, tk, sPropName, tkPropType, propVal);
		switch (tk)
		{
			case TK_TWO_SIDED:
				if (tkPropType != TK_BOOL)
					ErrWrongValue();
				bTwoSided = propVal.asBool;
				break;
			case TK_TEXTURED:
				if (tkPropType != TK_BOOL)
					ErrWrongValue();
				bTextured = propVal.asBool;
				break;
			case TK_ILLUM_COLOR_ON:
				if (tkPropType != TK_BOOL)
					ErrWrongValue();
				bSelfIllumColorOn = propVal.asBool;
				break;
			case TK_DIFFUSE:
				if (tkPropType != TK_POINT)
					ErrWrongValue();
				vfDiffuse[0] = propVal.asPoint.x;
				vfDiffuse[1] = propVal.asPoint.y;
				vfDiffuse[2] = propVal.asPoint.z;
				break;
			case TK_AMBIENT:
				if (tkPropType != TK_POINT)
					ErrWrongValue();
				vfAmbient[0] = propVal.asPoint.x;
				vfAmbient[1] = propVal.asPoint.y;
				vfAmbient[2] = propVal.asPoint.z;
				break;
			case TK_SPECULAR:
				if (tkPropType != TK_POINT)
					ErrWrongValue();
				vfSpecular[0] = propVal.asPoint.x;
				vfSpecular[1] = propVal.asPoint.y;
				vfSpecular[2] = propVal.asPoint.z;
				break;
			case TK_ILLUM_COLOR:
				if (tkPropType != TK_POINT)
					ErrWrongValue();
				vfSelfIllumColor[0] = propVal.asPoint.x;
				vfSelfIllumColor[1] = propVal.asPoint.y;
				vfSelfIllumColor[2] = propVal.asPoint.z;
				break;
			case TK_SHININESS:
				if (tkPropType != TK_FLOAT)
					ErrWrongValue();
				fShininess = propVal.asFloat;
			case TK_SHIN_STRENGTH:
				if (tkPropType != TK_FLOAT)
					ErrWrongValue();
				fShinStrength = propVal.asFloat;
			case TK_ILLUMINATION:
				if (tkPropType != TK_FLOAT)
					ErrWrongValue();
				fSelfIllumination = propVal.asFloat;
			case TK_TRANSPARENCY:
				if (tkPropType != TK_FLOAT)
					ErrWrongValue();
				fOpacity = 1.0f-propVal.asFloat;
		}
	}
	
	OnReadMaterial(vfAmbient, vfDiffuse, vfSpecular, 
		fShininess, fShinStrength, fOpacity,
		fSelfIllumination, bSelfIllumColorOn, vfSelfIllumColor, 
		bTwoSided, bTextured);
	
	ReadBinToken(tk);	
	if (tk == TK_TEXTURE)
	{
		char sName[MAX_STR_BUFF_SIZE], sType[MAX_STR_BUFF_SIZE];
		float fUTile = 0, fVTile = 0;
		int nSize = 0;
		
		ReadBinStr(sName);
		ReadBinStr(sType);
		m_stream.read((char*)&fUTile, sizeof(float));
		m_stream.read((char*)&fVTile, sizeof(float));
		m_stream.read((char*)&nSize, sizeof(int));

		const char* sFileName = 
			ConcatStrs(ExtractFilePath(FileName(), true), IncludeFileExt(sName, sType));
		char* pData = NULL;
		if (nSize > 0)
			pData = new char [nSize];
		if (pData != NULL)
			m_stream.read(pData, nSize);
		else
			m_stream.seekg(nSize, ios_base::cur);

		OnReadTexture(sFileName, sType, pData, nSize, fUTile, fVTile);
		
		if (pData != NULL)
			delete pData;
	}
	else
		PutBinTokenBack();
}

void CKineReader::ReadBinMesh()
{
	Token tk = TK_UNDEFINED;
	int nVertsCount = 0, nFacesCount = 0, nTexVertsCount = 0;
	int nSmoothGroup = 0, nEdges = 0, nBonesCount = 0;
	float vf[3];
	int viVerts[3], viTexVerts[3] = {0, 0, 0};
	float mfNormals[3][3];
	char vsBoneNamesBuff[256][MAX_STR_BUFF_SIZE];
	char* vsBoneNames[256];
	float vfBoneWeights[256];
	bool bVisible = false;
	
	m_stream.read((char*)&nVertsCount, sizeof(int));				
	m_stream.read((char*)&nFacesCount, sizeof(int));				
	m_stream.read((char*)&nTexVertsCount, sizeof(int));

	// TODO: remove it
	nVertsCount *= 2;
	
	OnReadMesh(nVertsCount, nFacesCount, nTexVertsCount);

	while (nVertsCount+nFacesCount+nTexVertsCount > 0)
	{
		ReadBinToken(tk);	
		switch (tk)
		{
			case TK_VERTEX:
				if (nVertsCount <= 0)
					ErrUnexpectedSymbol(tk);
				m_stream.read((char*)vf, 3*sizeof(float));
				nBonesCount = 0;
				while (nBonesCount < 256)
				{
					ReadBinToken(tk);	
					if (tk != TK_BONE)
					{
						PutBinTokenBack();
						break;
					}
					ReadBinStr(vsBoneNamesBuff[nBonesCount]);
					vsBoneNames[nBonesCount] = vsBoneNamesBuff[nBonesCount]; 
					m_stream.read((char*)(vfBoneWeights+nBonesCount), sizeof(float));
					nBonesCount++;
				}
				OnReadVertex(vf, nBonesCount, vsBoneNames, vfBoneWeights);
				nVertsCount--;
				break;
			case TK_FACE:
				if (nFacesCount <= 0)
					ErrUnexpectedSymbol(tk);
				m_stream.read((char*)&nSmoothGroup, sizeof(int));
				m_stream.read((char*)&bVisible, sizeof(bool));
				m_stream.read((char*)&nEdges, sizeof(int));
				ReadBinToken(tk);	
				if (tk != TK_VERTICES)
					ErrUnexpectedSymbol(tk);
				m_stream.read((char*)viVerts, 3*sizeof(int));
				ReadBinToken(tk);	
				if (tk == TK_TEXVERTICES)
					m_stream.read((char*)viTexVerts, 3*sizeof(int));
				else
					PutBinTokenBack();
				ReadBinToken(tk);	
				if (tk != TK_FACE_NORMAL)
					ErrUnexpectedSymbol(tk);
				m_stream.read((char*)vf, 3*sizeof(float));
				for (int i=0; i < 3; i++)
				{
					ReadBinToken(tk);	
					if (tk != TK_VERTEX_NORMAL)
						ErrUnexpectedSymbol(tk);
					m_stream.read((char*)mfNormals[i], 3*sizeof(float));
				}
				OnReadFace(nSmoothGroup, bVisible, nEdges, 
					viVerts, viTexVerts, vf, mfNormals);
				nFacesCount--;
				break;
			case TK_TEXVERTEX:
				if (nTexVertsCount <= 0)
					ErrUnexpectedSymbol(tk);
				m_stream.read((char*)vf, 3*sizeof(float));
				OnReadTexVertex((float*)vf);
				nTexVertsCount--;
				break;
			default:
				ErrUnexpectedSymbol(tk);
		}
	}

	OnEndMesh();					
}

//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Added by: Jarek Francik
// Comment: Implementation of IFileIn

HRESULT _stdcall CKineReader::Read(IFileSink *pSink, LPOLESTR pFilename)
{ 
	m_pSink = pSink;
	m_nCount = 0;
	try
	{
		CKineReader::Read(WStrToStr(pFilename)); 
	}
	catch (FWERROR *e)
	{
		return e->nCode;
	}
	catch (CKineReaderException e)
	{
		std::string str = e.Message();
		std::wstring wstr(str.begin(), str.end());
		const wchar_t *p = wstr.c_str();
		return ERROR(FILE_E_FILESYNTAX, 1, (FWULONG*)(&p));
	}
	catch (CKineFileException e)
	{
		std::string str = e.Message();
		std::wstring wstr(str.begin(), str.end());
		const wchar_t *p = wstr.c_str();
		return ERROR(FILE_E_FILESYSTEM, 1, (FWULONG*)(&p));
	}
	catch (...)
	{
		return ERROR(FW_E_UNIDENTIFIED);
	}
	return S_OK; 
}

// End of Added by: Jarek Francik
//------------------------------------------------------------------------------

void CKineReader::OnReadScene(const char* szName, const char* szFormat)
{
	if (!m_pSink) return;
	HRESULT h = S_OK;
	h = m_pSink->OnBeginScene(StrToWStr(szName), StrToWStr(szFormat));
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }
}

void CKineReader::OnEndScene()
{
	if (!m_pSink) return;
	HRESULT h = S_OK;
	h = m_pSink->OnEndScene();
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }
}

void CKineReader::OnReadNodes(int nCount)
{
	if (!m_pSink) return;
}

void CKineReader::OnEndNodes()
{
	if (!m_pSink) return;
}

void CKineReader::OnReadObject(const char* szName, const char* szClass,
	bool bVisible, bool bRenderable, float mfMatrix[4][3], 
	float vfPos[3], float vfRot[4], float vfScale[3],
	float vfBBoxMin[3], float vfBBoxMax[3])
{
	if (!m_pSink) return;

	HRESULT h = S_OK;

	if (!m_nCount)
		h = m_pSink->OnBeginObject(StrToWStr(szName), StrToWStr(szClass));
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }
	m_nCount++;

	FWQUAT qRot = { -vfRot[0], -vfRot[1], -vfRot[2], vfRot[3] };
	FWMATRIX m;
	m[0][0] = mfMatrix[0][0]; m[0][1] = mfMatrix[0][1]; m[0][2] = mfMatrix[0][2]; m[0][3] = 0.0f;
	m[1][0] = mfMatrix[1][0]; m[1][1] = mfMatrix[1][1]; m[1][2] = mfMatrix[1][2]; m[1][3] = 0.0f;
	m[2][0] = mfMatrix[2][0]; m[2][1] = mfMatrix[2][1]; m[2][2] = mfMatrix[2][2]; m[2][3] = 0.0f;
	m[3][0] = mfMatrix[3][0]; m[3][1] = mfMatrix[3][1]; m[3][2] = mfMatrix[3][2]; m[3][3] = 1.0f;
	h = m_pSink->OnBeginNode(StrToWStr(szName), StrToWStr(szClass), m, *(FWVECTOR3*)vfPos, qRot, *(FWVECTOR3*)vfScale, bVisible);
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }
//  @@@ 8-06-2005: new support for OnBB!
//	h = m_pSink->OnBB(*(FWVECTOR*)vfBBoxMin, *(FWVECTOR*)vfBBoxMax);
//	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }
}

void CKineReader::OnEndObject()
{
	if (!m_pSink) return;

	HRESULT h = S_OK;

	m_nCount--;
	if (!m_nCount)
		h = m_pSink->OnEndObject();
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }

	h = m_pSink->OnEndNode();
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }
}

void CKineReader::OnReadLight(const char* szName, const char* szClass, Token tkType,
	bool bActive, float vfPosition[3], float vfDirection[3], 
	bool bIsTarget, float vfTargetPos[3], float vfColor[3], float fPower)
{
	if (!m_pSink) return;

	FWCOLOR cColor = { vfColor[0], vfColor[1], vfColor[2], 1.0f };
	HRESULT h = S_OK;
	h = m_pSink->OnLight(StrToWStr(szName), StrToWStr(szClass), bIsTarget, 
		*(FWVECTOR3*)vfPosition, bIsTarget ? *(FWVECTOR3*)vfTargetPos : *(FWVECTOR3*)vfDirection, cColor, fPower, bActive);
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }
}

void CKineReader::OnReadCamera(const char* szName, const char* szClass, bool bIsOrtho, 
	float vfPosition[3], float vfDirection[3], float vfUpVector[3], 
	float fDistance, bool bIsTarget, float vfTargetPos[3],
	float fFOV, float fClipNear, float fClipFar)
{
	if (!m_pSink) return;

	HRESULT h = S_OK;
	h = m_pSink->OnCamera(StrToWStr(szName), StrToWStr(szClass), bIsTarget, 
		*(FWVECTOR3*)vfPosition, bIsTarget ? *(FWVECTOR3*)vfTargetPos : *(FWVECTOR3*)vfDirection, *(FWVECTOR3*)vfUpVector, 
		fFOV,fClipNear, fClipFar, fDistance, bIsOrtho);
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }
}

void CKineReader::OnReadMaterial(float vfAmbient[3], float vfDiffuse[3], float vfSpecular[3], 
	float fShininess, float fShinStrength, float fOpacity, float fSelfIllumination, 
	bool bSelfIllumColorOn, float vfSelfIllumColor[3], bool bTwoSided, bool bTextured)
{
	if (!m_pSink) return;

	FWCOLOR cAmbient  = { vfAmbient[0],  vfAmbient[1],  vfAmbient[2],  fOpacity };
	FWCOLOR cDiffuse  = { vfDiffuse[0],  vfDiffuse[1],  vfDiffuse[2],  fOpacity };
	FWCOLOR cSpecular = { vfSpecular[0], vfSpecular[1], vfSpecular[2], fOpacity };
	FWCOLOR cIllum    = { vfSelfIllumColor[0], vfSelfIllumColor[1], vfSelfIllumColor[2], fSelfIllumination };
	HRESULT h = S_OK;
	h = m_pSink->OnMaterial(cAmbient, cDiffuse, cSpecular, 
		fShininess, fShinStrength, fOpacity, fSelfIllumination, 
		bSelfIllumColorOn, cIllum, bTwoSided, bTextured);
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }
}

void CKineReader::OnReadTexture(const char* szName, const char* szType, 
	const char* pData, int nSize, float fUTile, float fVTile)
{
	if (!m_pSink) return;
	
	HRESULT h = S_OK;
	h = m_pSink->OnTexture(StrToWStr(szName), StrToWStr(szType), (BYTE*)pData, nSize, fUTile, fVTile);
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }
}

void CKineReader::OnReadProperty(const char* szName, Token tkType, Value& value)
{
	if (!m_pSink) return;

	HRESULT h = S_OK;
	switch (tkType)
	{
		case TK_QUAT:
			h = m_pSink->OnQuatProperty(StrToWStr(szName), *(FWQUAT*)(&value.asQuat));
			break;
		case TK_POINT:
			h = m_pSink->OnVectorProperty(StrToWStr(szName), *(FWVECTOR*)(&value.asPoint));
			break;
		case TK_FLOAT:
			h = m_pSink->OnFloatProperty(StrToWStr(szName), value.asFloat);
			break;
		case TK_INT:
			h = m_pSink->OnLongProperty(StrToWStr(szName), (FWULONG)value.asInt);
			break;
		case TK_RGB:
			{
				FWCOLOR color = { (float)GetRValue(value.asRGB) / 255.0f, (float)GetGValue(value.asRGB) / 255.0f, (float)GetBValue(value.asRGB) / 255.0f };
				h = m_pSink->OnColorProperty(StrToWStr(szName), color);
			}
			break;
		case TK_BOOL:
			h = m_pSink->OnLongProperty(StrToWStr(szName), (FWULONG)value.asBool);
			break;
		case TK_STRING:
			h = m_pSink->OnStringProperty(StrToWStr(szName), StrToWStr(value.asString));
			break;
		default:
			break;
	}
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }
}

void CKineReader::OnReadMesh(int nVertsCount, int nFacesCount, int nTexVertsCount)
{
	if (!m_pSink) return;
	HRESULT h = S_OK;
	h = m_pSink->OnBeginMesh(nVertsCount, nFacesCount, nTexVertsCount);
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }

	// initialise bounding box info
	m_bBoxAdded = false;
}

void CKineReader::OnEndMesh()
{
	if (!m_pSink) return;
	HRESULT h = S_OK;
	h = m_pSink->OnEndMesh();
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }

	// use bounding box info
	if (m_bBoxAdded)
	{
		h = m_pSink->OnBB(*(FWVECTOR*)m_vfBBoxMin, *(FWVECTOR*)m_vfBBoxMax);
		if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }
	}
}

void CKineReader::OnReadVertex(float vf[3], int nBonesCount, 
	char* vszBoneNames[], float vfWeights[])
{
	if (!m_pSink) return;

	LPOLESTR ppsBoneNames[256];
	for (int i = 0; i < nBonesCount; i++)
		ppsBoneNames[i] = StrToWStr(vszBoneNames[i]);
	HRESULT h = S_OK;
	h = m_pSink->OnVertex(*(FWVECTOR*)vf, nBonesCount, ppsBoneNames, vfWeights);
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }

	// update bounding box info
	if (!m_bBoxAdded)
	{
		memcpy(m_vfBBoxMin, vf, 3*sizeof(float));
		memcpy(m_vfBBoxMax, vf, 3*sizeof(float));
		m_bBoxAdded = true;
	}
	else
	{
		if (vf[0] < m_vfBBoxMin[0]) m_vfBBoxMin[0] = vf[0];
		if (vf[1] < m_vfBBoxMin[1]) m_vfBBoxMin[1] = vf[1];
		if (vf[2] < m_vfBBoxMin[2]) m_vfBBoxMin[2] = vf[2];
		if (vf[0] > m_vfBBoxMax[0]) m_vfBBoxMax[0] = vf[0];
		if (vf[1] > m_vfBBoxMax[1]) m_vfBBoxMax[1] = vf[1];
		if (vf[2] > m_vfBBoxMax[2]) m_vfBBoxMax[2] = vf[2];
	}
}

void CKineReader::OnReadTexVertex(float vf[3])
{
	if (!m_pSink) return;
	HRESULT h = S_OK;
	h = m_pSink->OnTexVertex(*(FWVECTOR*)vf);
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }
}

void CKineReader::OnReadFace(int nSmoothGroup, bool bVisible, int nEdges, 
	int viVerts[3], int viTexVerts[3], float vfFaceNormal[3], float mfVertNormals[3][3])
{
	if (!m_pSink) return;

	HRESULT h = S_OK;
	h = m_pSink->OnFace(viVerts[0], viVerts[1], viVerts[2], viTexVerts[0], viTexVerts[1], viTexVerts[2], (FWVECTOR*)mfVertNormals);
	if (FAILED(h)) { FWERROR *e; FWDevice()->GetLastError(&e); throw e; }
}

void CKineReader::OnError()
{
}
