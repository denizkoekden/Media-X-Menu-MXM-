#pragma once

#include <list>

#include "xmltree.h"

using namespace std;

class CXBNetUser
{
public:
	CXBNetUser( LPCTSTR szUserName = NULL, LPCTSTR szPassword = NULL, LPCTSTR szHomeDir = NULL, DWORD dwFlags = 0 );
	CXBNetUser( const CXBNetUser & src );
	virtual ~CXBNetUser(void);
	CXBNetUser * GetPtr( void ) { return this; };
	// Holds the user name associated with this entry
	CStdString	m_sUserName;
	// Holds the password associated with this entry
	CStdString	m_sPassword;
	CStdString	m_sHomeDir;
	DWORD		m_dwFlags;
};

typedef list<CXBNetUser> TXBNetUserList;

class CXBNetUserList
{
	TXBNetUserList	m_userEntryList;
	CXBNetUser * m_pLastAdded;
public:
	CXBNetUserList();
	CXBNetUser * GetLastAdded( void ) { return m_pLastAdded; };
	bool	AddEntry( LPCTSTR szName, LPCTSTR szPassword = NULL, LPCTSTR szHomeDir = NULL, DWORD dwFlags = 0 );
	bool	AddEntry( CXMLNode * pNode );
	CXBNetUser * GetEntry( LPCTSTR szName );
	CXBNetUser * GetValidEntry( LPCTSTR szName, LPCTSTR szPassword );
	bool	ValidateUser( LPCTSTR szName, LPCTSTR szPassword );
	LPCTSTR GetRoot( LPCTSTR szName );
	int				GetUserCount( void );

};

