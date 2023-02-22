// FTPUserEntries.h

#ifndef __FTPUSERENTRIES_H__
#define __FTPUSERENTRIES_H__

#include <xtl.h>
#include "StdString.h"
#include <list>
#include "xmltree.h"

using namespace std;


class CFTPUserEntry
{
public:
	CStdString	m_sName;
	CStdString	m_sPassword;
	CStdString	m_sRootDir;
	CFTPUserEntry( LPCTSTR szName = NULL, LPCTSTR szPassword = NULL, LPCTSTR szRoot = NULL );
	CFTPUserEntry( const CFTPUserEntry & src );
	CFTPUserEntry * GetPtr( void ) { return this; };
};


typedef list<CFTPUserEntry> TFTPUserEntryList;

class CFTPUserEntryList
{
	TFTPUserEntryList	m_userEntryList;
public:
	bool	AddEntry( LPCTSTR szName, LPCTSTR szPassword = NULL, LPCTSTR szRoot = NULL );
	bool	AddEntry( CXMLNode * pNode );
	CFTPUserEntry * GetEntry( LPCTSTR szName );
	bool	ValidateUser( LPCTSTR szName, LPCTSTR szPassword );
	LPCTSTR GetRoot( LPCTSTR szName );

};


#endif //  __FTPUSERENTRIES_H__

