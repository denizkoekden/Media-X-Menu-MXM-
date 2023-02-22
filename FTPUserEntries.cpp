


#include <xtl.h>
#include "StdString.h"
#include "FTPUserEntries.h"



CFTPUserEntry::CFTPUserEntry( LPCTSTR szName /* = NULL */, LPCTSTR szPassword /* = NULL */, LPCTSTR szRoot /* = NULL */ ) :
	m_sName(szName)
	,m_sPassword(szPassword)
	,m_sRootDir(szRoot)
{
}

CFTPUserEntry::CFTPUserEntry( const CFTPUserEntry & src )
{
	m_sName		=	src.m_sName;
	m_sPassword	=	src.m_sPassword;
	m_sRootDir	=	src.m_sRootDir;
}


bool	CFTPUserEntryList::AddEntry( LPCTSTR szName, LPCTSTR szPassword, LPCTSTR szRoot )
{
	CStdString sName(szName);
	bool bReturn = false;

	sName.Trim();

	if ( sName.GetLength() )
	{
		CFTPUserEntry *pEntry = NULL;

		pEntry = GetEntry( sName );
		if ( pEntry == NULL )
		{
			m_userEntryList.push_back( CFTPUserEntry( sName, szPassword, szRoot ) );
			bReturn = true;
		}
	}
	return bReturn;
}

bool	CFTPUserEntryList::AddEntry( CXMLNode * pNode )
{
	bool bReturn = false;
	CStdString sName, sPass, sRoot;

	sName = pNode->GetString( NULL, _T("name"), _T("") );
	sPass = pNode->GetString( NULL, _T("password"), _T("") );
	sRoot = pNode->GetString( NULL, _T("root"), _T("") );

	sName.Trim();
	sPass.Trim();
	sRoot.Trim();
	if ( sName.GetLength() && sPass.GetLength() )
	{
		bReturn = AddEntry( sName, sPass, sRoot );
	}
	return bReturn;
}

CFTPUserEntry * CFTPUserEntryList::GetEntry( LPCTSTR szName )
{
	CFTPUserEntry * pReturnEntry = NULL;
	CFTPUserEntry * pEntry = NULL;
	TFTPUserEntryList::iterator iterList;

	iterList = m_userEntryList.begin();
	while( (pReturnEntry == NULL) && (iterList != m_userEntryList.end()) )
	{
		pEntry = iterList->GetPtr();
		if ( pEntry->m_sName.Compare( szName ) == 0 )
		{
			pReturnEntry = pEntry;
		}
		iterList++;
	}
	return pReturnEntry;
}

bool	CFTPUserEntryList::ValidateUser( LPCTSTR szName, LPCTSTR szPassword )
{
	CFTPUserEntry * pEntry = GetEntry( szName );
	bool bReturn = false;

	if ( pEntry )
	{
		if ( pEntry->m_sPassword.Compare( szPassword ) == 0 )
		{
			bReturn = true;
		}
	}
	return bReturn;
}

LPCTSTR CFTPUserEntryList::GetRoot( LPCTSTR szName )
{
	CFTPUserEntry * pEntry = GetEntry( szName );
	LPCTSTR szRoot = _T("");

	if ( pEntry )
	{
		szRoot = pEntry->m_sRootDir;
	}
	return szRoot;
}

