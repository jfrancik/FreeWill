//------------------------------------------------------------------------------
// Project: FreeWill+
// Module:  kinefile.h
// Author:  Marek Mittmann
// Date:    4.02.2003 (last modification: 6.10.2005)
// Desc.:   
//------------------------------------------------------------------------------

#ifndef _KINEFILE_H__INCLUDED_
#define _KINEFILE_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <string>
#include <fstream>

using namespace std;

//-- constants --------------------------------------------------------------

extern const char
	*IDS_XML_FORMAT,
	*IDS_BIN_FORMAT,
	*IDS_XML_EXT,
	*IDS_BIN_EXT,

	*MSG_E_OPEN_FILE,					
	*MSG_E_READ_FILE,					
	*MSG_E_WRITE_FILE,				
	
	*MSG_E_SYNTAX_ERROR,				
	*MSG_E_ERROR_AT_POS,				
	*MSG_E_UNEXPECTED_CHAR,			
	*MSG_E_UNKNOWN_IDENTIFIER,			
	*MSG_E_EXPECTED_ATTRIBUTE,			
	*MSG_E_EXPECTED_ATTRIBUTE_OF_TYPE,		
	*MSG_E_EXPECTED_VALUE_OF_TYPE,		
	*MSG_E_UNTERMINATED_STRING,			
	*MSG_E_EXPECTED_TAG_BEGIN,			
	*MSG_E_EXPECTED_TAG_END,
	*MSG_E_EXPECTED_IDENTIFIER,
	*MSG_E_UNEXPECTED_SYMBOL_AT,		
	*MSG_E_WRONG_VALUE_AT;
	
	
//-- CKineFileException class -------------------------------------------------

class CKineFileException
{
protected:
	int m_nCode;
	char* m_szMsg;

public:
	CKineFileException();
	CKineFileException(const char* szMessage, int nCode = 0);
	CKineFileException(const CKineFileException& e);
	virtual ~CKineFileException();

	CKineFileException& operator=(const CKineFileException& e);

	int Code() const
	{ 
		return m_nCode; 
	}

	const char* Message() const
	{ 
		return m_szMsg; 
	}
};

//-- CKineFile class ----------------------------------------------------------

class CKineFile
{
public:
	enum Token
	{
		TK_UNDEFINED = 256,
		TK_SCENE, TK_OBJECT, TK_CAMERA, TK_LIGHT, TK_LINK_NODES,
		TK_PROPERTY, TK_STD_PROPERTY, TK_MESH, TK_TRANSFORM_MATRIX, 
		TK_MATERIAL, TK_TEXTURE, TK_DATA,
		TK_VERTEX, TK_BONE, TK_TEXVERTEX, TK_POINT, TK_QUAT, TK_MATRIX12,
		TK_FACE, TK_VERTICES, TK_TEXVERTICES, TK_FACE_NORMAL, TK_VERTEX_NORMAL, 
		TK_NAME, TK_FORMAT, TK_TYPE, TK_CLASS, TK_SIZE,
		TK_COUNT, TK_NUMVERTS, TK_NUMFACES, TK_NUMTEXTUREVERTS,
		TK_ID, TK_WEIGHT, TK_VISIBLE, TK_SMOOTHING_GROUP, TK_EDGES, TK_COLOR,
		TK_TRUE, TK_FALSE,
		TK_INT_VALUE, TK_FLOAT_VALUE, TK_BOOL_VALUE, TK_STRING_VALUE, 
		TK_POINT_VALUE, TK_QUAT_VALUE, TK_BYTE_VALUE,
		TK_INT, TK_FLOAT, TK_RGB, TK_BOOL, TK_STRING,
		
		TK_POSITION, TK_ROTATION, TK_SCALE, TK_BBOX_MIN, TK_BBOX_MAX, TK_RENDERABLE,
		TK_ISORTHO, TK_DIRECTION, TK_UPVECTOR, TK_DISTANCE,
		TK_TARGET_POSITION, TK_FOV, TK_CLIP_NEAR, TK_CLIP_FAR, TK_ACTIVE, TK_POWER,
		TK_SPOT, TK_DIRECTIONAL, TK_AMBIENT,
		TK_DIFFUSE, TK_SPECULAR, TK_SHININESS, TK_SHIN_STRENGTH,
		TK_TRANSPARENCY, TK_ILLUMINATION, TK_ILLUM_COLOR, TK_ILLUM_COLOR_ON,
		TK_TWO_SIDED, TK_TEXTURED, TK_UTILE, TK_VTILE,
		
		TK_EOF, TK_OPEN_TAG, TK_CLOSE_TAG, TK_EQUAL, TK_COMMA,
		TK_ANIMATED_MESH,
		
		TK_BEGIN = 0x8000, TK_END = 0x4000
	};
	
	const static int SIZE_OF_TOKEN = 2;
	
private:
	CKineFile(const CKineFile& o)					{ }
	CKineFile& operator=(const CKineFile& o)		{ }

protected:
	fstream m_stream;
	string m_fileName;

	int m_nIndentSize;
	int m_nIndentPos;
	bool m_bBinaryMode;
	
	struct TokenRec
	{
		Token token;
		const char* szText;
	};

	static TokenRec m_tokensTable[];
	
	// maps
	static map<string, Token> m_stringToTokenMap;
	static map<Token, string> m_tokenToStringMap;

	Token m_recentlyWritten;

public:
	CKineFile();
	virtual ~CKineFile();

	static void Initialize();
	void Open(const char* szFileName, bool bWriteMode = false);
	void Close();
	
	bool IsOpened() const
	{
		return m_stream.is_open();
	}
	
	const char* FileName() const
	{
		return m_fileName.c_str();
	}
	
	int IndentSize() const					
	{ 
		return m_nIndentSize; 
	}
	
	void SetIndentSize(int nValue)			
	{ 
		m_nIndentSize = nValue; 
	}
	
	bool BinaryMode() const					
	{ 
		return m_bBinaryMode; 
	}
	
	void SetBinaryMode(bool bValue = true);
	
	int Precision() const
	{
		return (int)m_stream.precision();
	}
	
	void SetPrecision(int nValue)
	{
		m_stream.precision(nValue);
	}

	void IncIndent()
	{ 
		m_nIndentPos += m_nIndentSize; 
	}

	void DecIndent()
	{ 
		m_nIndentPos -= m_nIndentSize; 
		if (m_nIndentPos < 0) 
			m_nIndentPos = 0; 
	}

	const char* FormatIDS() const;
	const char* DefaultFileExt() const;

	static const char* TokenToString(Token token)
	{
		Initialize();
		map<Token, string>::iterator it = m_tokenToStringMap.find(token);
		return (it != m_tokenToStringMap.end() ? (*it).second.c_str() : "unknown");
	}	 

	static Token StringToToken(const char* szString)
	{
		Initialize();
		map<string, Token>::iterator it = m_stringToTokenMap.find(szString);
		return (it != m_stringToTokenMap.end() ? (*it).second : TK_UNDEFINED);
	}
};

//-- CKineWriter class ------------------------------------------------------

class CKineBasicWriter: public CKineFile
{
private:
	int m_nBytesInLine;
	
protected:
	bool m_bHexCodedFloats;

public:
	CKineBasicWriter();
	CKineBasicWriter(const char* szFileName);
	virtual ~CKineBasicWriter();
	
	void Open(const char* szFileName);

	bool HexCodedFloats() const					
	{ 
		return m_bHexCodedFloats; 
	}
	
	void SetHexCodedFloats(bool bValue = true);
	
	void WriteTagBegin(Token tag);
	void WriteTagEnd(Token tag);
	void WriteComment(const char* szText);

	void WriteStrAttrib(Token tag, const char* szValue);
	void WriteFloatAttrib(Token tag, float fValue);
	void WriteIntAttrib(Token tag, int iValue, int nBase = 10);
	void WriteBoolAttrib(Token tag, bool bValue);
	void WriteTokenAttrib(Token tag, Token tkValue);
	
	void WriteStrData(const char* szValue);	
	void WriteIntData(int iValue, int nBase = 10);	
	void WriteBoolData(bool bValue);
	void WriteIntVectorData(int* vi, int nLen, int nBase);	
	void WriteFloatVectorData(float* v, int nLen);	
	void WriteByteData(int byValue);
	
	void WriteInt3Data(int* vi, int nBase = 10)//, bool bInverse = false)
	{
		//if (bInverse)
		//{
		//	int i = vi[0];
		//	vi[0] = vi[2];
		//	vi[2] = i;
		//}
		WriteIntVectorData(vi, 3, nBase);
	}

	void WriteFloatData(float fValue)
	{
		WriteFloatVectorData(&fValue, 1);
	}
		
	void WriteFloat3Data(float* v)
	{
		WriteFloatVectorData(v, 3);
	}
		
	void WriteFloat4Data(float* v)
	{
		WriteFloatVectorData(v, 4);
	}
};

#endif // _KINEFILE_H__INCLUDED_
