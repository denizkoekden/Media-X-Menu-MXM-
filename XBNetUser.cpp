
#include "StdAfx.h"
#include "xbnetuser.h"

CXBNetUser::CXBNetUser(LPCTSTR szUserName, LPCTSTR szPassword, LPCTSTR szHomeDir, DWORD dwFlags ) :
	m_sUserName(szUserName)
	,m_sPassword(szPassword)
	,m_dwFlags(dwFlags)
	,m_sHomeDir(szHomeDir)
{
}


CXBNetUser::CXBNetUser( const CXBNetUser & src )
{
	m_sUserName = src.m_sUserName;
	m_sPassword = src.m_sPassword;
	m_sHomeDir = src.m_sHomeDir;
	m_dwFlags = src.m_dwFlags;
}

CXBNetUser::~CXBNetUser(void)
{
}

CXBNetUserList::CXBNetUserList() :
		m_pLastAdded(NULL)
{
}

bool	CXBNetUserList::AddEntry( LPCTSTR szName, LPCTSTR szPassword, LPCTSTR szRoot, DWORD dwFlags )
{
	CStdString sName(szName);
	bool bReturn = false;

	sName.Trim();

	if ( sName.GetLength() )
	{
		CXBNetUser *pEntry = NULL;

		pEntry = GetEntry( sName );
		if ( pEntry == NULL )
		{
			m_userEntryList.push_back( CXBNetUser( sName, szPassword, szRoot, dwFlags ) );
			m_pLastAdded = m_userEntryList.back().GetPtr();
			bReturn = true;
		}
	}
	return bReturn;
}


bool	CXBNetUserList::AddEntry( CXMLNode * pNode )
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

CXBNetUser * CXBNetUserList::GetEntry( LPCTSTR szName )
{
	CXBNetUser * pReturnEntry = NULL;
	CXBNetUser * pEntry = NULL;
	TXBNetUserList::iterator iterList;

	iterList = m_userEntryList.begin();
	while( (pReturnEntry == NULL) && (iterList != m_userEntryList.end()) )
	{
		pEntry = iterList->GetPtr();
		if ( pEntry->m_sUserName.Compare( szName ) == 0 )
		{
			pReturnEntry = pEntry;
		}
		iterList++;
	}
	return pReturnEntry;
}

CXBNetUser * CXBNetUserList::GetValidEntry( LPCTSTR szName, LPCTSTR szPassword )
{
	CXBNetUser * pReturnEntry = NULL;
	CXBNetUser * pEntry = NULL;
	TXBNetUserList::iterator iterList;

	iterList = m_userEntryList.begin();
	while( (pReturnEntry == NULL) && (iterList != m_userEntryList.end()) )
	{
		pEntry = iterList->GetPtr();
		if ( pEntry->m_sUserName.Compare( szName ) == 0 )
		{
			if ( pEntry->m_sPassword.Compare( szPassword ) )
			{
				pReturnEntry = NULL;
				iterList = m_userEntryList.end();
				break;
			}
			else
			{
				pReturnEntry = pEntry;
			}
		}
		else
		{
			iterList++;
		}
	}
	return pReturnEntry;
}

bool	CXBNetUserList::ValidateUser( LPCTSTR szName, LPCTSTR szPassword )
{
	CXBNetUser * pEntry = GetEntry( szName );
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

LPCTSTR CXBNetUserList::GetRoot( LPCTSTR szName )
{
	CXBNetUser * pEntry = GetEntry( szName );
	LPCTSTR szRoot = _T("");

	if ( pEntry )
	{
		szRoot = pEntry->m_sHomeDir;
	}
	return szRoot;
}


int CXBNetUserList::GetUserCount( void )
{
	int iReturn = m_userEntryList.size();

	return iReturn;
}

