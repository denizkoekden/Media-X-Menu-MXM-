

#pragma once

#include "StdString.h"

CStdString EncodeURIFragment( LPCTSTR szFragment, int iFragmentID );
CStdString DecodeURIFragment( LPCTSTR szFragment, bool bHandleAmpersand = true );

// scheme://login:pass@server.tld/path/file?query
class CURI
{
	CStdString	m_sAuth;
	CStdString	m_sPathQuery;
public:
	CURI( LPCTSTR szURI );
	void Split( LPCTSTR szURI );
	CStdString GetPathQuery( void );
	CStdString GetMerged( void );
	CStdString GetServer( void );
	DWORD		GetPort( void ) { return m_dwPort; };
	DWORD		m_dwPort;
	CStdString	m_sURI;
	CStdString	m_sScheme;
	CStdString	m_sLogin;
	CStdString	m_sPassword;
	CStdString	m_sServer;
	CStdString	m_sPath;
	CStdString	m_sQuery;
};

