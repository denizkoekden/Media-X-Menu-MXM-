// IniToXmlConvertDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IniToXmlConvert.h"
#include "IniToXmlConvertDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIniToXmlConvertDlg dialog

CIniToXmlConvertDlg::CIniToXmlConvertDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIniToXmlConvertDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIniToXmlConvertDlg)
	m_sInputFile = _T("");
	m_sOutputFile = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CIniToXmlConvertDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIniToXmlConvertDlg)
	DDX_Control(pDX, IDC_CONVERT_BT, m_btConvert);
	DDX_Text(pDX, IDC_INPUT_ET, m_sInputFile);
	DDX_Text(pDX, IDC_OUTPUT_ET, m_sOutputFile);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CIniToXmlConvertDlg, CDialog)
	//{{AFX_MSG_MAP(CIniToXmlConvertDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OBROWSE_BT, OnObrowseBt)
	ON_BN_CLICKED(IDC_IBROWSE_BT, OnIbrowseBt)
	ON_BN_CLICKED(IDC_CONVERT_BT, OnConvertBt)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIniToXmlConvertDlg message handlers

BOOL CIniToXmlConvertDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIniToXmlConvertDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIniToXmlConvertDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIniToXmlConvertDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CIniToXmlConvertDlg::OnObrowseBt() 
{
	UpdateData( TRUE );
	CFileDialog dlgFileOutput( FALSE, _T(".xml"), m_sOutputFile, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("XML Config files (*.xml)|*.xml||"), this );

	if ( dlgFileOutput.DoModal() == IDOK )
	{
		m_sOutputFile = dlgFileOutput.GetPathName();
	}
	UpdateData( FALSE );
	
}

void CIniToXmlConvertDlg::OnIbrowseBt() 
{
	UpdateData( TRUE );
	CFileDialog dlgFileInput( TRUE, _T(".ini"), m_sInputFile, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, _T("INI Config files (*.ini)|*.ini||"), this );

	if ( dlgFileInput.DoModal() == IDOK )
	{
		m_sInputFile = dlgFileInput.GetPathName();
		if ( m_sOutputFile.GetLength() == 0 )
		{
			int iPos;

			m_sOutputFile = dlgFileInput.GetPathName();
			m_sOutputFile.MakeLower();
			iPos = m_sOutputFile.Find( _T(".ini") );
			if ( iPos != -1 )
			{
				m_sOutputFile = dlgFileInput.GetPathName().Left( iPos );
				m_sOutputFile += _T(".xml");
			}
			else
			{
				m_sOutputFile = _T("");
			}
		}
	}
	UpdateData( FALSE );
}

CString XmlTag( LPCTSTR szName, BOOL bOpen, BOOL bNewLine )
{
	CString sTag;

	sTag.Format( _T("<%s%s>%s"), bOpen?_T(""):_T("/"), szName, bNewLine?_T("\n"):_T("") );
	return sTag;
}

void CIniToXmlConvertDlg::OnConvertBt() 
{
	UpdateData( TRUE );
	// Ready to convert the files...?
	LPTSTR strSectionBuffer = NULL;
	LPTSTR strSection = NULL;
	LPTSTR strKeyBuffer = NULL;
	LPTSTR strKey = NULL;
	LPTSTR strValueBuffer = NULL;
	BOOL bSuccess = FALSE;
	
	if ( ( m_sOutputFile.GetLength() ) &&
		 ( m_sInputFile.GetLength() ) &&
		 ( GetFileAttributes( m_sInputFile ) & 0x0ff ) )
	{
		strValueBuffer = new TCHAR [ 1000 ];
		if ( strValueBuffer )
		{
			strSectionBuffer = new TCHAR [64000];
			if ( strSectionBuffer )
			{
				strSection = strSectionBuffer;
				strKeyBuffer = new TCHAR [64000];
				if ( strKeyBuffer )
				{
					CStdioFile fileOutput;

					if ( fileOutput.Open( m_sOutputFile, CFile::modeWrite|CFile::modeCreate ) )
					{
						fileOutput.WriteString( XmlTag( _T("Config"), TRUE, TRUE ) );
						GetPrivateProfileSectionNames( strSectionBuffer, 64000, m_sInputFile );
						while( *strSection )
						{
							fileOutput.WriteString( XmlTag( strSection, TRUE, TRUE ) );
							strKey = strKeyBuffer;
							GetPrivateProfileString( strSection, NULL, _T(""), strKeyBuffer, 64000, m_sInputFile );
							while( *strKey )
							{
								GetPrivateProfileString( strSection, strKey, _T(""), strValueBuffer, 1000, m_sInputFile );
								fileOutput.WriteString( XmlTag( strKey, TRUE, FALSE ) );
								fileOutput.WriteString( strValueBuffer );
								fileOutput.WriteString( XmlTag( strKey, FALSE, TRUE ) );
								strKey += _tcslen( strKey ) + 1;
							}
							fileOutput.WriteString( XmlTag( strSection, FALSE, TRUE ) );
							strSection += _tcslen( strSection ) + 1;
						}
						fileOutput.WriteString( XmlTag( _T("Config"), FALSE, TRUE ) );
						bSuccess = TRUE;
					}
					delete [] strKeyBuffer;
				}
				delete [] strSectionBuffer;
			}
			delete [] strValueBuffer;
		}
	}
	if ( !bSuccess )
	{
		MessageBox( _T("Operation failed") );
	}
	else
	{
		MessageBox( _T("Operation succeeded") );
	}
	UpdateData( FALSE );
}
