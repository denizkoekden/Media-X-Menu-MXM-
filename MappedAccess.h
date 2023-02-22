/*********************************************************************************\
 * MappedAccess.h
 * Class used to provide mapped access to configuration items
 * (C) 2002 MasterMind
\*********************************************************************************/

#if !defined(__MAPPEDACCESS_H__)
#define __MAPPEDACCESS_H__



#include "StdString.h"

#pragma warning(disable:4786)
#pragma warning(disable:4503)

#include <iostream>
#include <string>
#include <map>

#include "MXM_Defs.h"

using namespace std;

#pragma warning(disable:4786)
#pragma warning(disable:4503)



class CMappedAccess
{
public:
	CMappedAccess();
	CStdString GetString( LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szDefault );
	void	SetString( LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szValue );
	int		GetNumberSections( void );
	int		GetNumberKeys( LPCTSTR szSection );
	CStdString	GetSectionName( int iPos );
	CStdString	GetKeyName( LPCTSTR szSection, int iPos );
	void Clear( void );
protected:
	TMapSections m_mapSections;
};


#endif