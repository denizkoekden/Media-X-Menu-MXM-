#include "StdAfx.h"
#include "xbdirectorylister.h"

int g_iFindHandlesOpened = 0;

CStdString ConvertSlashes( LPCTSTR szString )
{
	CStdString sReturn(szString);
	int iLength;

	sReturn.Replace( _T("/"), _T("\\") ); 
	sReturn.Replace( _T("\\\\"), _T("\\") ); 
	sReturn.Replace( _T("\\\\"), _T("\\") ); 
	sReturn.Replace( _T("\\\\"), _T("\\") ); 

	// Remove trailing backslashes from item (We don't want them in directory names, unless ROOT)

	iLength = sReturn.GetLength();
	if ( iLength>1 )
	{
		if ( sReturn[iLength-1] == _T('\\') )
		{
			sReturn = sReturn.Left( iLength-1 );
		}
	}
	return sReturn;
}

CStdString BackupPathSection( LPCTSTR szPath )
{
	CStdString sReturn( szPath );
	int iPos;

	iPos = sReturn.ReverseFind( _T('\\') );

	// Never back up beyond root!!
	if ( iPos > 0 )
	{
		sReturn = sReturn.Left( iPos );
	}
	else
	{
		sReturn = _T("\\");
	}
	return sReturn;
}

CStdString CombinePathAndRelative( LPCTSTR szPath, LPCTSTR szName )
{
	CStdString sReturn, sName(szName), sPath(szPath);

	if ( sName.GetLength() )
	{
		if ( sPath.GetLength() )
		{
			if ( sName[0] != _T('\\') )
			{
				while( _tcsnicmp( sName, _T("..\\"), 3 ) == 0 )
				{
					sPath = BackupPathSection(sPath);
					sName = sName.Mid(3);
				}
				// Get rid of these
				sName.Replace( _T(".\\"), _T("") );
				if ( sPath.GetLength() > 1 )
				{
					sReturn.Format( _T("%s\\%s"), sPath.c_str(), sName.c_str() );
				}
				else
				{
					// Yuck
					sReturn.Format( _T("%s%s"), sPath.c_str(), sName.c_str() );
				}
			}
			else
			{
				// Absolute path. No need to reference anything here
				sReturn = sName;
			}
		}
		else
		{
			sReturn = sName;
		}
	}
	else
	{
		sReturn = sPath;
	}
	return sReturn;
}

static CStdString FixDriveReference( LPCTSTR szString )
{
	CStdString sReturn(szString);
	TCHAR tcDrive;
	
	if ( sReturn.GetLength() )
	{
		if ( sReturn.GetLength() > 1 )
		{
			if ( _istalpha( sReturn[1] ) && ( sReturn[0] == _T('\\') ) )
			{
				tcDrive = _totupper( sReturn[1] );
				sReturn.Format( _T("%c:%s"), tcDrive, sReturn.Mid(2).c_str() );
			}
		}
	}
	return sReturn;
}

CStdString TranslateFilenameToFileSys( LPCTSTR szCurrPath, LPCTSTR szFilename )
{
	CStdString sReturn, sFilename, sPath;

	sPath = ConvertSlashes(szCurrPath);
	sFilename = ConvertSlashes(szFilename);
	sReturn = CombinePathAndRelative( sPath, sFilename );
	sReturn = FixDriveReference( sReturn );
	if ( sReturn.GetLength() < 3 )
	{
		if ( _istalpha( sReturn[0] ) )
		{
			if ( sReturn.GetLength() < 2 )
			{
				sReturn += _T(":\\");
			}
			else
			{
				if ( sReturn[1] == _T(':') )
				{
					sReturn += _T('\\');
				}
			}
		}
	}

	return sReturn;
}

CStdString TranslateFilenameToFTP( LPCTSTR szCurrPath, LPCTSTR szFilename )
{
	CStdString sReturn;

	return sReturn;
}

//class CXBDirectoryLister
//{
//	CXBDirectoryLister( LPCTSTR szCurrentDirectory );
//	void Reset();
//	bool GetNext( CStdString & sEntryName, DWORD & dwAttributes, ULONGULONG & u64Size );
//	CStdString TranslateToFileSys( LPCTSTR szFilename );
//}



CXBDirectoryLister::CXBDirectoryLister(LPCTSTR szCurrentDirectory) :
	m_bIsRoot(false)
	,m_bIsValid(false)
	,m_hFindHandle(INVALID_HANDLE_VALUE)
	,m_sCurrentFTPDirectory(szCurrentDirectory)
{
	m_sCurrentRealDirectory = TranslateFilenameToFileSys(m_sCurrentFTPDirectory,NULL);
	if ( m_sCurrentRealDirectory.GetLength() == 1 && m_sCurrentRealDirectory[0] == _T('\\') )
	{
		m_bIsRoot = true;
		m_bIsValid = true;
	}
	else if ( m_sCurrentRealDirectory.GetLength() == 3 && _istalpha(m_sCurrentRealDirectory[0]) && (m_sCurrentRealDirectory[1] == _T(':')) && (m_sCurrentRealDirectory[2] == _T('\\')) )
	{
		m_bIsValid = true;
	}
	else if ( m_sCurrentRealDirectory.GetLength() > 3 && _istalpha(m_sCurrentRealDirectory[0]) && (m_sCurrentRealDirectory[1] == _T(':')) && (m_sCurrentRealDirectory[2] == _T('\\')) )
	{
		DWORD dwAttributes;
		// Need to check to see if it's valid as a directory!
		dwAttributes = GetFileAttributes( m_sCurrentRealDirectory );
		if ( (dwAttributes != (DWORD)-1) && (dwAttributes&FILE_ATTRIBUTE_DIRECTORY) )
		{
			m_bIsValid = true;
		}
	}
	Reset();
}


void	CXBDirectoryLister::Close( void )
{
	if ( m_hFindHandle != INVALID_HANDLE_VALUE )
	{
		g_iFindHandlesOpened--;
		DEBUG_FORMAT( _T("CXBDir: Closing Find Handle (%s) (%d)\r\n"), m_sCurrentRealDirectory, g_iFindHandlesOpened );
		FindClose(m_hFindHandle);
		m_hFindHandle = INVALID_HANDLE_VALUE;
	}
}

CXBDirectoryLister::~CXBDirectoryLister(void)
{
	Close();
}

// Resets the directory list to the first item
void CXBDirectoryLister::Reset(void)
{
	if ( m_bIsValid )
	{
		if ( m_bIsRoot )
		{
			m_iDriveIndex = 0;
		}
		else
		{

			Close();
			g_iFindHandlesOpened++;
			DEBUG_FORMAT( _T("CXBDir: Opening Find Handle (%s) (%d)\r\n"), m_sCurrentRealDirectory, g_iFindHandlesOpened );
			m_hFindHandle = FindFirstFile( TranslateToFileSys(_T("*")), &m_ffData );
		}
	}
}

// static LPCTSTR s_szDriveLetters = _T("CDEFGHIJKLMNOXYZ");
static LPCTSTR s_szDriveLetters = _T("CDEFGXYZ");


// Returns the next file and stats in the directory listing
bool CXBDirectoryLister::GetNext(CStdString & sEntryName, DWORD & dwAttributes, ULONGLONG & u64Size, FILETIME & ftFileTime )
{
	bool bReturn = false;
	if ( m_bIsValid )
	{
		if ( m_bIsRoot )
		{
			// m_iDriveIndex = 0;
			if ( _tcslen( s_szDriveLetters ) > (DWORD)m_iDriveIndex )
			{
				SYSTEMTIME sysTime;

				GetLocalTime( &sysTime );
				SystemTimeToFileTime( &sysTime, &ftFileTime );
				sEntryName.Format( _T("%c"), s_szDriveLetters[m_iDriveIndex] );
				u64Size = 0;
				dwAttributes = FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_READONLY;
				m_iDriveIndex++;
				bReturn = true;
			}
		}
		else
		{
			if ( m_hFindHandle != INVALID_HANDLE_VALUE )
			{
				if ( _tcscmp( _T("."), m_ffData.cFileName ) == 0 )
				{
					// Skip this one and look for the next item in the menu!
					if ( !FindNextFile( m_hFindHandle, &m_ffData ) )
					{
						Close();
					}
					else
					{
						bReturn = GetNext(sEntryName, dwAttributes, u64Size, ftFileTime );
					}
				}
				else if ( _tcscmp( _T(".."), m_ffData.cFileName ) == 0 )
				{
					// Skip this one and look for the next item in the menu!
					if ( !FindNextFile( m_hFindHandle, &m_ffData ) )
					{
						Close();
					}
					else
					{
						bReturn = GetNext(sEntryName, dwAttributes, u64Size, ftFileTime );
					}
				}
				else
				{
					sEntryName = m_ffData.cFileName;
					u64Size = m_ffData.nFileSizeLow;
					dwAttributes = m_ffData.dwFileAttributes;
					ftFileTime = m_ffData.ftLastWriteTime;
					bReturn = true;
					if ( !FindNextFile( m_hFindHandle, &m_ffData ) )
					{
						Close();
					}
				}
			}
		}
	}
	return bReturn;
}

// With FTP Named file, convert to a "real" filename
CStdString CXBDirectoryLister::TranslateToFileSys(LPCTSTR szFilename)
{
	CStdString sReturn;

	if ( m_bIsValid )
	{
		sReturn = TranslateFilenameToFileSys(m_sCurrentFTPDirectory,szFilename);
	}
	return sReturn;
}

// If directory is valid, this will return true
bool CXBDirectoryLister::IsValid(void)
{
	return m_bIsValid;
}
