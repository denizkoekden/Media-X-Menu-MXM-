/*********************************************************************************\
 * RawCfgAccess.h
 * Class used to abstract access to ini/registry and xml configuration items
 * (C) 2002 MasterMind
\*********************************************************************************/

#if !defined(__RAWCFGACCESS_H__)
#define __RAWCFGACCESS_H__

#include "MappedAccess.h"
#include "StdString.h"


class CRawCfgAccess
{
public:
	CRawCfgAccess( LPCTSTR szName = NULL );

	virtual CStdString GetString( LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szDefault );
	virtual BOOL	GetBool( LPCTSTR szSection, LPCTSTR szKey, BOOL bDefault );
	virtual long	GetLong( LPCTSTR szSection, LPCTSTR szKey, long lDefault );
	virtual DWORD	GetDWORD( LPCTSTR szSection, LPCTSTR szKey, DWORD dwDefault );

	virtual void	SetString( LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szValue );
	virtual void	SetBool( LPCTSTR szSection, LPCTSTR szKey, BOOL bValue );
	virtual void	SetLong( LPCTSTR szSection, LPCTSTR szKey, long lValue );
	virtual void	SetDWORD( LPCTSTR szSection, LPCTSTR szKey, DWORD dwValue );

	virtual	int		Load( LPCTSTR szName = NULL );
	virtual	int		Save( LPCTSTR szName = NULL );

	virtual int		GetNumberSections( void );
	virtual int		GetNumberKeys( LPCTSTR szSection );
	virtual CStdString	GetSectionName( int iPos );
	virtual CStdString	GetKeyName( LPCTSTR szSection, int iPos );
	void Clear( void );

protected:
	CStdString			m_szName;
	CMappedAccess	m_mapSections;
};

#endif