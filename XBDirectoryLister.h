#pragma once

#include "StdString.h"

CStdString TranslateFilenameToFileSys( LPCTSTR szCurrPath, LPCTSTR szFilename );
CStdString TranslateFilenameToFTP( LPCTSTR szCurrPath, LPCTSTR szFilename );
CStdString ConvertSlashes( LPCTSTR szString );
CStdString CombinePathAndRelative( LPCTSTR szPath, LPCTSTR szName );
CStdString BackupPathSection( LPCTSTR szPath );


class CXBDirectoryLister
{
public:
	CXBDirectoryLister(LPCTSTR szCurrentDirectory);
	virtual ~CXBDirectoryLister(void);
protected:
	int					m_iDriveIndex;
	WIN32_FIND_DATA		m_ffData;
	HANDLE				m_hFindHandle;
	// Current "real" directory path this object is pointing to
	CStdString m_sCurrentRealDirectory;
	// Current "ftp" directory path this object is pointing to
	CStdString m_sCurrentFTPDirectory;
	// If true, we are in the "root" directory
	bool	m_bIsRoot;
	bool	m_bIsValid;
public:
	// Resets the directory list to the first item
	void Reset(void);
	// Returns the next file and stats in the directory listing
	bool GetNext(CStdString & sEntryName, DWORD & dwAttributes, ULONGLONG & u64Size, FILETIME & ftFileTime );
	// With FTP Named file, convert to a "real" filename
	CStdString TranslateToFileSys(LPCTSTR szFilename);
	// If directory is valid, this will return true
	bool IsValid(void);
	void	Close( void );
};
