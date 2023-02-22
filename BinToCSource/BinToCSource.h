// BinToCSource.h : main header file for the BINTOCSOURCE application
//

#if !defined(AFX_BINTOCSOURCE_H__C3C2986D_9354_48AD_9D61_368DD5A53E3C__INCLUDED_)
#define AFX_BINTOCSOURCE_H__C3C2986D_9354_48AD_9D61_368DD5A53E3C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CBinToCSourceApp:
// See BinToCSource.cpp for the implementation of this class
//

class CBinToCSourceApp : public CWinApp
{
public:
	CBinToCSourceApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBinToCSourceApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CBinToCSourceApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BINTOCSOURCE_H__C3C2986D_9354_48AD_9D61_368DD5A53E3C__INCLUDED_)
