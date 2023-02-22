// BinToCSourceDlg.h : header file
//

#if !defined(AFX_BINTOCSOURCEDLG_H__E5863AB4_6BB5_4462_9B95_167369315816__INCLUDED_)
#define AFX_BINTOCSOURCEDLG_H__E5863AB4_6BB5_4462_9B95_167369315816__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CBinToCSourceDlg dialog

class CBinToCSourceDlg : public CDialog
{
// Construction
public:
	CBinToCSourceDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CBinToCSourceDlg)
	enum { IDD = IDD_BINTOCSOURCE_DIALOG };
	CString	m_sBinFile;
	CString	m_sSrcFile;
	BOOL	m_bMakeHeader;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBinToCSourceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CBinToCSourceDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBinfilebrwsBt();
	afx_msg void OnSrcfilebwrBt();
	afx_msg void OnConvertBt();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BINTOCSOURCEDLG_H__E5863AB4_6BB5_4462_9B95_167369315816__INCLUDED_)
