// hresult.h: Smart HResult Class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(_HRESULT__)
#define _HRESULT__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//////////////////////////////////////////////////////////////////////////////
/// Smart HResult Objects.
/// Detects failed HRESULT values and throws an exception.
/// Usage:
///		CHResult h;
///		h = any_COM_function();
///		h = other_COM_function();
class CHResult
{
public:
	CHResult()				{ m_count = 0; m_h = S_OK; }

	/// HRESULT of the FAILED operation
	HRESULT GetHResult()	{ return m_h; }
	/// index: allows to distinguish which consecutive operation failed
	int		GetCount()		{ return (*this) ? -1 : m_count; }
	
	/// HRESULT value assignement; detects FAILED condition and throws AfxThrowOleException(h)
	CHResult &operator =(HRESULT h)		
							{ m_count++; m_h = h; if (FAILED(h)) AfxThrowOleException(h); return *this; }

	operator HRESULT()		{ return m_h; }
	operator bool()			{ return SUCCEEDED(m_h); }

private:
	HRESULT m_h;
	int m_count;
};

#endif