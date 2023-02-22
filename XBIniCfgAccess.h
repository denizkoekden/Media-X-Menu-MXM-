/*********************************************************************************\
 * XBIniCfgAccess.h
 * Xbox-specific ini file access routines
 * (C) 2002 MasterMind
\*********************************************************************************/

#if !defined(__XBINIRAWCFGACCESS_H__)
#define __XBINIRAWCFGACCESS_H__


#include "MappedAccess.h"
#include "StdString.h"
#include "RawCfgAccess.h"

class CXBIniCfgAccess : public CRawCfgAccess
{
public:
	CXBIniCfgAccess( LPCTSTR szFileName = NULL, BOOL bDebug = FALSE  );
	int		Load( LPCTSTR szName = NULL, BOOL bDebug = FALSE );
	virtual int		Save( LPCTSTR szName = NULL );
};




#endif