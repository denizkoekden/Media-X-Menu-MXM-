// MXMConfigDlg.h : header file
//

#if !defined(AFX_MXMCONFIGDLG_H__F758B712_749D_402C_9E98_9DA54863D9F5__INCLUDED_)
#define AFX_MXMCONFIGDLG_H__F758B712_749D_402C_9E98_9DA54863D9F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMXMConfigDlg dialog

class CMXMConfigDlg : public CDialog
{
// Construction
public:
	CMXMConfigDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMXMConfigDlg)
	enum { IDD = IDD_MXMCONFIG_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMXMConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMXMConfigDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MXMCONFIGDLG_H__F758B712_749D_402C_9E98_9DA54863D9F5__INCLUDED_)
