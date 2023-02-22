// BinToCSourceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BinToCSource.h"
#include "BinToCSourceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


DWORD FileLength( LPCTSTR szFilename );
BOOL  FileExists( LPCTSTR szFilename );

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
// CBinToCSourceDlg dialog

CBinToCSourceDlg::CBinToCSourceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBinToCSourceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBinToCSourceDlg)
	m_sBinFile = _T("");
	m_sSrcFile = _T("");
	m_bMakeHeader = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBinToCSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBinToCSourceDlg)
	DDX_Text(pDX, IDC_BINFILE_ET, m_sBinFile);
	DDX_Text(pDX, IDC_SRCFILE_ET, m_sSrcFile);
	DDX_Check(pDX, IDC_MAKEHEADER_CK, m_bMakeHeader);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBinToCSourceDlg, CDialog)
	//{{AFX_MSG_MAP(CBinToCSourceDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BINFILEBRWS_BT, OnBinfilebrwsBt)
	ON_BN_CLICKED(IDC_SRCFILEBWR_BT, OnSrcfilebwrBt)
	ON_BN_CLICKED(IDC_CONVERT_BT, OnConvertBt)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBinToCSourceDlg message handlers

BOOL CBinToCSourceDlg::OnInitDialog()
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

void CBinToCSourceDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CBinToCSourceDlg::OnPaint() 
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
HCURSOR CBinToCSourceDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CBinToCSourceDlg::OnBinfilebrwsBt() 
{
	UpdateData( TRUE );
	CFileDialog dlgFileInput( TRUE, NULL, m_sBinFile, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, _T("All files (*.*)|*.*||"), this );

	if ( dlgFileInput.DoModal() == IDOK )
	{
		m_sBinFile = dlgFileInput.GetPathName();
	}
	UpdateData( FALSE );
}

void CBinToCSourceDlg::OnSrcfilebwrBt() 
{
	UpdateData( TRUE );
	CFileDialog dlgFileSource( FALSE, _T(".cpp"), m_sSrcFile, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("C/C++ source files (*.cpp; *.c)|*.cpp; *.c||"), this );

	if ( dlgFileSource.DoModal() == IDOK )
	{
		m_sSrcFile = dlgFileSource.GetPathName();
	}
	UpdateData( FALSE );
}

void WriteDataLine( CStdioFile &fStdOut, BYTE *pbyData, DWORD dwLength, BOOL bEndComma = TRUE )
{
	int iIndex;
	CString sTemp;

	for( iIndex=0; iIndex<dwLength; iIndex++ )
	{
		sTemp.Format( _T(" 0x%02x"), pbyData[iIndex] );
		fStdOut.WriteString( sTemp );
		if ( ((iIndex+1)<dwLength ) || bEndComma )
		{
			fStdOut.WriteString( _T(",") );
		}
	}
	fStdOut.WriteString( _T("\n") );
}

void CBinToCSourceDlg::OnConvertBt() 
{
	DWORD dwLength;
	CString sTemp;
	CString sHeaderFile;

	UpdateData( TRUE );
	if ( m_sSrcFile.GetLength() && m_sBinFile.GetLength() )
	{
		if ( FileExists( m_sBinFile ) )
		{
			CFile fileInput;

			dwLength = FileLength( m_sBinFile );

			if ( m_bMakeHeader )
			{

				sHeaderFile = m_sSrcFile;
				if (	sHeaderFile.Replace( _T(".c"), _T(".h") ) ||
						sHeaderFile.Replace( _T(".C"), _T(".h") ) ||
						sHeaderFile.Replace( _T(".cpp"), _T(".h") ) ||
						sHeaderFile.Replace( _T(".CPP"), _T(".h") ) )
				{
					CStdioFile fileHeader;

					if ( fileHeader.Open( sHeaderFile, CFile::modeWrite|CFile::modeCreate ) )
					{
						sTemp.Format( _T("\n\n\n\n\nextern BYTE pbData[%d];\n\n\n"), dwLength );
						fileHeader.WriteString( sTemp );
					}
					else
					{
						sHeaderFile = _T("");
					}
				}
				else
				{
					sHeaderFile = _T("");
				}
			}
			if ( fileInput.Open( m_sBinFile, CFile::modeRead|CFile::typeBinary ) )
			{
				CStdioFile fileOutput;
				DWORD dwIndex;
				DWORD dwRead;
				BYTE byData[32];

				if ( fileOutput.Open( m_sSrcFile, CFile::modeWrite|CFile::modeCreate ) )
				{
					if ( sHeaderFile.GetLength() )
					{
						sTemp.Format( _T("\n\n#include \"%s\"\n\n"), (LPCTSTR)sHeaderFile );
						fileOutput.WriteString( sTemp );
					}

					sTemp.Format( _T("\n\n/* Data Created from: %s */\n\n"), m_sSrcFile );
					fileOutput.WriteString( sTemp );

					sTemp.Format( _T("\n\nBYTE pbData[%d] = {\n"), dwLength );
					fileOutput.WriteString( sTemp );
					for( dwIndex=0; dwIndex<dwLength; dwIndex+=32 )
					{
						dwRead = fileInput.Read( byData, 32 );
						WriteDataLine( fileOutput, byData, dwRead, (dwRead==32)?TRUE:FALSE );	
					}
					fileOutput.WriteString( _T("};\n\n") );
				}
			}
		}
	}
}


BOOL  FileExists( LPCTSTR szFilename )
{
	BOOL bReturn = FALSE;

	DWORD dwAttributes;

	if ( szFilename && _tcslen( szFilename ) )
	{

		dwAttributes = GetFileAttributes( szFilename );
		if ( dwAttributes != (DWORD)-1 )
		{
			bReturn = TRUE;
		}
	}

	return bReturn;
}

DWORD FileLength( LPCTSTR szFilename )
{
	DWORD dwReturn = 0;

	if ( FileExists(szFilename) )
	{
		HANDLE hTemp;
		hTemp = CreateFile( szFilename, 
					   GENERIC_READ, 
					   FILE_SHARE_READ, 
					   NULL,
					   OPEN_EXISTING,
					   FILE_ATTRIBUTE_NORMAL,
					   NULL );
		if ( hTemp != INVALID_HANDLE_VALUE )
		{
			dwReturn = GetFileSize( hTemp, NULL );
		}
		CloseHandle( hTemp );
	}
	return dwReturn;
}

