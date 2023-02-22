// IniToXmlConvert.h : main header file for the INITOXMLCONVERT application
//

#if !defined(AFX_INITOXMLCONVERT_H__B4D0C206_6A55_4696_A01C_7641DD7ACB32__INCLUDED_)
#define AFX_INITOXMLCONVERT_H__B4D0C206_6A55_4696_A01C_7641DD7ACB32__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CIniToXmlConvertApp:
// See IniToXmlConvert.cpp for the implementation of this class
//

class CIniToXmlConvertApp : public CWinApp
{
public:
	CIniToXmlConvertApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIniToXmlConvertApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CIniToXmlConvertApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INITOXMLCONVERT_H__B4D0C206_6A55_4696_A01C_7641DD7ACB32__INCLUDED_)
