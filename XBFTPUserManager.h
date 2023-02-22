#pragma once

#include "XBFTPUser.h"
#include <list>
#include "xmltree.h"

using namespace std;


typedef list<CXBFTPUser> TXBFTPUserList;

class CXBFTPUserManager
{
	TXBFTPUserList	m_userEntryList;
public:
	CXBFTPUserManager(void);
	virtual ~CXBFTPUserManager(void);

	int				GetUserCount( void );
	bool			AddEntry( LPCTSTR szName, LPCTSTR szPassword = NULL, LPCTSTR szRoot = NULL );
	bool			AddEntry( CXMLNode * pNode );
	CXBFTPUser *	GetEntry( LPCTSTR szName );
	bool			ValidateUser( LPCTSTR szName, LPCTSTR szPassword );
	LPCTSTR			GetRoot( LPCTSTR szName );
};
