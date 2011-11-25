//------------------------------------------------------------------------------
// Project: FreeWill+
// Module:  kinefile.cpp
// Author:  Marek Mittmann
// Date:    4.02.2003 (last modification: 6.10.2005)
// Desc.:   
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include "kinefile.h"
#include "strformat.h"

using namespace std;

//-- constants --------------------------------------------------------------

// Format idenifiers and file extensions

const char
	*IDS_XML_FORMAT		= "KinePlus.XML.1.00",
	*IDS_BIN_FORMAT		= "KinePlus.BIN.1.00",
	*IDS_XML_EXT		= ".xml",
	*IDS_BIN_EXT		= ".3d",
	
// Error messages

	*MSG_E_OPEN_FILE					= "Cannot open file %s",
	*MSG_E_READ_FILE					= "Read error",
	*MSG_E_WRITE_FILE					= "Write error",
	
	*MSG_E_SYNTAX_ERROR					= "Syntax error",
	*MSG_E_ERROR_AT_POS					= "%s at line %d, column %d",
	*MSG_E_UNEXPECTED_CHAR				= "Unexpected character '%c'",
	*MSG_E_UNKNOWN_IDENTIFIER			= "Unknown identifier '%s'",
	*MSG_E_EXPECTED_ATTRIBUTE			= "Expected attribute '%s'",
	*MSG_E_EXPECTED_ATTRIBUTE_OF_TYPE	= "Expected value of type '%s' for attribute '%s'",
	*MSG_E_EXPECTED_VALUE_OF_TYPE		= "Expected value of type '%s'",
	*MSG_E_UNTERMINATED_STRING			= "Unterminated string",			
	*MSG_E_EXPECTED_TAG_BEGIN			= "Expected tag '%s'",			
	*MSG_E_EXPECTED_TAG_END				= "Expected end of '%s'",
	*MSG_E_EXPECTED_IDENTIFIER			= "Expected symbol '%s'",
	*MSG_E_UNEXPECTED_SYMBOL_AT			= "Unexpected symbol '%s' at position %d",			
	*MSG_E_WRONG_VALUE_AT				= "Wrong value at position %d";


//-- CKineFileException class ---------------------------------------------------

char* NewStr(const char* szSource)
{
	char* szRes = new char [strlen(szSource)+sizeof(char)];
	if (szRes != NULL)
		strcpy(szRes, szSource);
	return szRes;
}

void CopyStr(char*& pszDestination, const char* szSource)
{
	if (pszDestination != NULL)
		delete pszDestination;
	pszDestination = NewStr(szSource);
}

CKineFileException::CKineFileException():
	m_nCode(0),
	m_szMsg(NULL)
{
}

CKineFileException::CKineFileException(const char* szMessage, int nCode):
	m_nCode(nCode),
	m_szMsg(NewStr(szMessage))
{
}

CKineFileException::CKineFileException(const CKineFileException& e):
	m_nCode(e.m_nCode),
	m_szMsg(NewStr(e.m_szMsg))
{
}

CKineFileException::~CKineFileException()
{
	if (m_szMsg != NULL)
		delete m_szMsg;
}

CKineFileException& CKineFileException::operator=(const CKineFileException& e)
{
	if (&e != this)
	{
		m_nCode = e.m_nCode;
		CopyStr(m_szMsg, e.m_szMsg);
	}
	return *this;
}

//-- CKineFile class ------------------------------------------------------------

CKineFile::TokenRec CKineFile::m_tokensTable[] = {
	{ TK_SCENE,				"scene"				},
	{ TK_OBJECT,			"object"			},
	{ TK_PROPERTY,			"prop"				},
	{ TK_STD_PROPERTY,		"prop"				},
	{ TK_TRANSFORM_MATRIX,	"transform"			},
	{ TK_LINK_NODES,		"nodes"				},
	{ TK_MESH,				"mesh"				},
	{ TK_CAMERA,			"camera"			},
	{ TK_LIGHT,				"light"				},
	{ TK_MATERIAL,			"material"			},
	{ TK_TEXTURE,			"texture"			},
	{ TK_DATA,				"data"				},
	{ TK_VERTICES,			"verts"				},
	{ TK_TEXVERTICES,		"tverts"			},
	{ TK_VERTEX,			"v"					},
	{ TK_FACE,				"face"				},
	{ TK_FACE_NORMAL,		"fnormal"			},
	{ TK_VERTEX_NORMAL,		"vnormal"			},
	{ TK_TEXVERTEX,			"tv"				},
	{ TK_POINT,				"point"				},
	{ TK_QUAT,				"quat"				},
	{ TK_MATRIX12,			"matrix12"			},
	{ TK_NAME,				"name"				},
	{ TK_FORMAT,			"format"			},
	{ TK_TYPE,				"type"				},
	{ TK_CLASS,				"class"				},
	{ TK_SIZE,				"size"				},
	{ TK_COUNT,				"count"				},
	{ TK_NUMVERTS,			"numverts"			},
	{ TK_NUMFACES,			"numfaces"			},
	{ TK_NUMTEXTUREVERTS,	"numtverts"			},
	{ TK_ID,				"id"				},
	{ TK_WEIGHT,			"weight"			},
	{ TK_BONE,				"bone"				},
	{ TK_VISIBLE,			"visible"			},
	{ TK_SMOOTHING_GROUP,	"group"				},
	{ TK_COLOR,				"color"				},
	{ TK_EDGES,				"edges"				},
	{ TK_INT,				"int"				},
	{ TK_FLOAT,				"float"				},
	{ TK_RGB,				"rgb"				},
	{ TK_BOOL,				"bool"				},
	{ TK_STRING,			"string"			},
	{ TK_POSITION,			"pos"				},
	{ TK_ROTATION,			"rot"				},
	{ TK_SCALE,				"scale"				},
	{ TK_BBOX_MIN,			"bboxmin"			},
	{ TK_BBOX_MAX,			"bboxmax"			},
	{ TK_RENDERABLE,		"renderable"		},
	{ TK_ISORTHO,			"isortho"			},
	{ TK_DIRECTION,			"dir"				},
	{ TK_UPVECTOR,			"upvector"			},
	{ TK_DISTANCE,			"distance"			},
	{ TK_TARGET_POSITION,	"targetpos"			},
	{ TK_FOV,				"fov"				},
	{ TK_CLIP_NEAR,			"clipnear"			},
	{ TK_CLIP_FAR,			"clipfar"			},
	{ TK_ACTIVE,			"active"			},
	{ TK_POWER,				"power"				},
	{ TK_SPOT,				"spot"				},
	{ TK_DIRECTIONAL,		"directional"		},
	{ TK_AMBIENT,			"ambient"			},
	{ TK_DIFFUSE,			"diffuse"			},
	{ TK_SPECULAR,			"specular"			},
	{ TK_SHININESS,			"shininess"			},
	{ TK_SHIN_STRENGTH,		"shinstrength"		},
	{ TK_TRANSPARENCY,		"transparency"		},
	{ TK_ILLUMINATION,		"illumination"		},
	{ TK_ILLUM_COLOR,		"illumcolor"		},
	{ TK_ILLUM_COLOR_ON,	"illumcoloron"		},
	{ TK_TWO_SIDED,			"twosided"			},
	{ TK_TEXTURED,			"textured"			},
	{ TK_ANIMATED_MESH,		"animated_mesh"		},
	{ TK_UTILE,				"utile"				},
	{ TK_VTILE,				"vtile"				},
	{ TK_TRUE,				"true"				},
	{ TK_FALSE,				"false"				},
	{ TK_EQUAL,				"="					},
	{ TK_COMMA,				","					}
};

map<string, CKineFile::Token> CKineFile::m_stringToTokenMap;
map<CKineFile::Token, string> CKineFile::m_tokenToStringMap;

CKineFile::CKineFile():
	m_nIndentSize(1),
	m_nIndentPos(0),
	m_bBinaryMode(false)
{
	Initialize();
	m_stream.precision(6);
	m_stream.flags(ios_base::boolalpha | ios_base::showbase | 
	               ios_base::showpoint | ios_base::fixed);
}

CKineFile::~CKineFile()
{
	Close();
}

void CKineFile::Initialize()
{
	if (m_tokenToStringMap.empty())
	{
		for (int i = 0; i < sizeof(m_tokensTable)/sizeof(TokenRec); i++)
		{
			m_tokenToStringMap.insert(map<Token, string>::value_type(
				m_tokensTable[i].token, m_tokensTable[i].szText));
			m_stringToTokenMap.insert(map<string, Token>::value_type(
				m_tokensTable[i].szText, m_tokensTable[i].token));
		}
	}
}

void CKineFile::Open(const char* szFileName, bool bWriteMode)
{
	Close();
	m_fileName = IncludeFileExt(szFileName, DefaultFileExt());
	m_stream.clear();
	m_stream.open(m_fileName.c_str(), 
	              (bWriteMode ? (ios_base::out | ios_base::trunc) : ios_base::in) |
	              (m_bBinaryMode ? ios_base::binary : 0));
	if (!m_stream.good())
		throw CKineFileException(FormatStr(MSG_E_OPEN_FILE, m_fileName.c_str()));
	m_recentlyWritten = TK_UNDEFINED;
}

void CKineFile::Close()
{
	m_stream.close();
	m_fileName.clear();
}

const char* CKineFile::FormatIDS() const
{ 
	return (m_bBinaryMode ? IDS_BIN_FORMAT : IDS_XML_FORMAT); 
}

const char* CKineFile::DefaultFileExt() const	
{ 
	return (m_bBinaryMode ? IDS_BIN_EXT : IDS_XML_EXT); 
}

void CKineFile::SetBinaryMode(bool bValue)
{
	Close();
	m_bBinaryMode = bValue;
}


//-- CKineBasicWriter class ---------------------------------------------------

CKineBasicWriter::CKineBasicWriter():
	CKineFile(),
	m_bHexCodedFloats(false)
{
}

CKineBasicWriter::CKineBasicWriter(const char* szFileName):
	CKineFile(),
	m_bHexCodedFloats(false)
{
	CKineFile::Open(szFileName, true);
}

CKineBasicWriter::~CKineBasicWriter()
{
}

void CKineBasicWriter::Open(const char* szFileName)
{
	CKineFile::Open(szFileName, true);
}

void CKineBasicWriter::SetHexCodedFloats(bool bValue)
{
	m_bHexCodedFloats = bValue;
}

void CKineBasicWriter::WriteTagBegin(Token tag)
{
	if (m_bBinaryMode)
	{
		int iValue = /* TK_BEGIN | */ tag;
		m_stream.write((char*)&iValue, SIZE_OF_TOKEN);
	}
	else
	{
		map<Token, string>::iterator it = m_tokenToStringMap.find(tag);
		if (m_recentlyWritten & TK_BEGIN)
			m_stream << '>' << endl;
		else if (m_recentlyWritten != TK_UNDEFINED)
			m_stream << endl;
		for (int i = 0; i < m_nIndentPos; i++)
			m_stream.put(' ');
		m_stream << '<' << TokenToString(tag);
		IncIndent();
		m_recentlyWritten = (Token)(tag | TK_BEGIN);
	}
}

void CKineBasicWriter::WriteTagEnd(Token tag)
{
	if (m_bBinaryMode)
	{
		//int iValue = TK_END | tag;
		//m_stream.write((char*)&iValue, SIZE_OF_TOKEN);
	}
	else
	{
		map<Token, string>::iterator it = m_tokenToStringMap.find(tag);
		DecIndent();
		if (m_recentlyWritten & TK_BEGIN)
			m_stream << "/>";
		else 
		{
			if (m_recentlyWritten & TK_END || m_recentlyWritten == TK_BYTE_VALUE)
			{
				m_stream << endl;
				for (int i = 0; i < m_nIndentPos; i++)
					m_stream.put(' ');
			}
			m_stream << "</" << TokenToString(tag) << '>';
		}
		m_recentlyWritten = (Token)(tag | TK_END);
	}
}

void CKineBasicWriter::WriteComment(const char* szText)
{
	if (!m_bBinaryMode)
		m_stream << "<!-- " << szText << " -->" << endl;
}

void CKineBasicWriter::WriteStrAttrib(Token tag, const char* szValue)
{
	if (m_bBinaryMode)
	{
		size_t nLen = strlen(szValue)*sizeof(char);
		m_stream.write((char*)&nLen, sizeof(size_t));
		m_stream.write(szValue != NULL ? szValue : "", (std::streamsize) nLen+sizeof(char));
	}
	else
	{
		m_stream << ' ' << TokenToString(tag) << "=\"";
		m_stream << (szValue != NULL ? szValue : "") << "\"";
	}
}

void CKineBasicWriter::WriteFloatAttrib(Token tag, float fValue)
{
	if (m_bBinaryMode)
		m_stream.write((char*)&fValue, sizeof(float));
	else
	{
		if (m_bHexCodedFloats)
		{
			m_stream << ' ' << TokenToString(tag) << "=\"0x";
			m_stream << hex << noshowbase << setfill('0') << setw(8);
			m_stream << (*(unsigned int*)&fValue) << showbase << "\"";
		}
		else
			m_stream << ' ' << TokenToString(tag) << "=\"" << fValue << "\"";
	}
}

void CKineBasicWriter::WriteIntAttrib(Token tag, int iValue, int nBase)
{
	if (m_bBinaryMode)
		m_stream.write((char*)&iValue, sizeof(int));
	else
	{
		m_stream << setbase(nBase) << uppercase;
		m_stream << ' ' << TokenToString(tag) << "=\"" << iValue << "\"" << nouppercase;
	}
}

void CKineBasicWriter::WriteBoolAttrib(Token tag, bool bValue)
{
	if (m_bBinaryMode)
		m_stream.write((char*)&bValue, sizeof(bool));
	else
		m_stream << ' ' << TokenToString(tag) << "=\"" << bValue << "\"";
}

void CKineBasicWriter::WriteTokenAttrib(Token tag, Token tkValue)
{
	if (m_bBinaryMode)
		m_stream.write((char*)&tkValue, SIZE_OF_TOKEN);
	else
		m_stream << ' ' << TokenToString(tag) << "=\"" << TokenToString(tkValue) << "\"";
}

void CKineBasicWriter::WriteStrData(const char* szValue)
{
	if (m_bBinaryMode)
	{
		size_t nLen = strlen(szValue)*sizeof(char);
		m_stream.write((char*)&nLen, sizeof(size_t));
		m_stream.write(szValue != NULL ? szValue : "", (std::streamsize) nLen+sizeof(char));
	}
	else
	{
		m_stream << '>' << (szValue != NULL ? szValue : "");
		m_recentlyWritten = TK_STRING_VALUE;
	}
}
	
void CKineBasicWriter::WriteIntData(int iValue, int nBase)
{
	if (m_bBinaryMode)
		m_stream.write((char*)&iValue, sizeof(int));
	else
	{
		m_stream << '>' << setbase(nBase) << iValue;
		m_recentlyWritten = TK_INT_VALUE;
	}
}
	
void CKineBasicWriter::WriteBoolData(bool bValue)
{
	if (m_bBinaryMode)
		m_stream.write((char*)&bValue, sizeof(bool));
	else
	{
		m_stream << '>' << bValue;
		m_recentlyWritten = TK_BOOL_VALUE;
	}
}
	
void CKineBasicWriter::WriteIntVectorData(int* vi, int nLen, int nBase)
{
	if (m_bBinaryMode)
		m_stream.write((char*)vi, nLen*sizeof(int));
	else
	{
		m_stream.put('>');
		if (nLen > 0)
		{
			m_stream << setbase(nBase);
			for (int i=0; i < nLen-1; i++)
				m_stream << vi[i] << ", ";
			m_stream << vi[nLen-1];
		}
		m_recentlyWritten = TK_INT_VALUE;
	}
}
	
void CKineBasicWriter::WriteFloatVectorData(float* v, int nLen)
{
	if (m_bBinaryMode)
		m_stream.write((char*)v, nLen*sizeof(float));
	else
	{
		m_stream.put('>');
		if (m_bHexCodedFloats)
		{
			if (nLen > 0)
			{
				m_stream << "0x";
				m_stream << hex << noshowbase << setfill('0');
				for (int i=0; i < nLen; i++)
					m_stream << setw(8) << (*(unsigned int*)(v+i));
				m_stream << showbase;
			}
		}
		else
		{
			if (nLen > 0)
			{
				for (int i=0; i < nLen-1; i++)
					m_stream << v[i] << ", ";
				m_stream << v[nLen-1];
			}
		}
		m_recentlyWritten = TK_FLOAT_VALUE;
	}
}

void CKineBasicWriter::WriteByteData(int byValue)
{
	if (m_bBinaryMode)
		m_stream.write((char*)&byValue, 1);
	else
	{
		if (m_recentlyWritten != TK_BYTE_VALUE || m_nBytesInLine >= 32)
		{
			if (m_recentlyWritten & TK_BEGIN)
				m_stream.put('>');
			m_nBytesInLine = 0;
			m_stream.put('\n');
			for (int i = 0; i < m_nIndentPos; i++)
				m_stream.put(' ');
		}
		char hi = ((byValue >> 4) & 0xF), lo = (byValue & 0xF);
		m_stream.put(hi < 10 ? hi+'0' : hi-10+'a');
		m_stream.put(lo < 10 ? lo+'0' : lo-10+'a');
		m_nBytesInLine++;
		m_recentlyWritten = TK_BYTE_VALUE;
	}
}