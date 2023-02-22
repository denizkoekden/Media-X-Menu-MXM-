#pragma once

#include "StdString.h"

class CXBFTPUser
{
public:
	CStdString	m_sName;
	CStdString	m_sPassword;
	CStdString	m_sRootDir;

	CXBFTPUser( LPCTSTR szName = NULL, LPCTSTR szPassword = NULL, LPCTSTR szRoot = NULL );
	CXBFTPUser( const CXBFTPUser & src );
	virtual ~CXBFTPUser(void);

	CXBFTPUser * GetPtr( void ) { return this; };
};
