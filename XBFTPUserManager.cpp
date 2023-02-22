#include "StdAfx.h"
#include "xbftpusermanager.h"

CXBFTPUserManager::CXBFTPUserManager(void)
{
}

CXBFTPUserManager::~CXBFTPUserManager(void)
{
}


bool CXBFTPUserManager::AddEntry( LPCTSTR szName, LPCTSTR szPassword /* = NULL */, LPCTSTR szRoot /* = NULL */ )
{
	bool bReturn = false;

	return bReturn;
}

bool CXBFTPUserManager::AddEntry( CXMLNode * pNode )
{
	bool bReturn = true;


	return bReturn;
}

CXBFTPUser * CXBFTPUserManager::GetEntry( LPCTSTR szName )
{
	CXBFTPUser * pReturn = NULL;

	return pReturn;
}

bool CXBFTPUserManager::ValidateUser( LPCTSTR szName, LPCTSTR szPassword )
{
	bool bReturn = false;

	return bReturn;
}

LPCTSTR CXBFTPUserManager::GetRoot( LPCTSTR szName )
{
	LPCTSTR szReturn = NULL;

	return szReturn;
}
