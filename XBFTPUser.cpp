#include "StdAfx.h"
#include "xbftpuser.h"


CXBFTPUser::CXBFTPUser( LPCTSTR szName /* = NULL */, LPCTSTR szPassword /* = NULL */, LPCTSTR szRoot /* = NULL */ ) :
	m_sName(szName)
	,m_sPassword(szPassword)
	,m_sRootDir(szRoot)
{
}

CXBFTPUser::CXBFTPUser( const CXBFTPUser & src )
{
	m_sName		=	src.m_sName;
	m_sPassword	=	src.m_sPassword;
	m_sRootDir	=	src.m_sRootDir;
}

CXBFTPUser::~CXBFTPUser(void)
{
}
