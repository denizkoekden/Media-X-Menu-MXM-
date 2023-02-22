// IniToXmlConvertDlg.h : header file
//

#if !defined(AFX_INITOXMLCONVERTDLG_H__BB9797CD_E026_440C_B4A0_56E99B2F6E81__INCLUDED_)
#define AFX_INITOXMLCONVERTDLG_H__BB9797CD_E026_440C_B4A0_56E99B2F6E81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CIniToXmlConvertDlg dialog

class CIniToXmlConvertDlg : public CDialog
{
// Construction
public:
	CIniToXmlConvertDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CIniToXmlConvertDlg)
	enum { IDD = IDD_INITOXMLCONVERT_DIALOG };
	CButton	m_btConvert;
	CString	m_sInputFile;
	CString	m_sOutputFile;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIniToXmlConvertDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CIniToXmlConvertDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnObrowseBt();
	afx_msg void OnIbrowseBt();
	afx_msg void OnConvertBt();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INITOXMLCONVERTDLG_H__BB9797CD_E026_440C_B4A0_56E99B2F6E81__INCLUDED_)
