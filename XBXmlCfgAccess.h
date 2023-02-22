/*********************************************************************************\
 * XBXmlCfgAccess.h
 * Xbox-specific ini file access routines
 * (C) 2002 MasterMind
\*********************************************************************************/

#if !defined(__XBXMLCFGACCESS_H__)
#define __XBXMLCFGACCESS_H__


#include "MappedAccess.h"
#include "StdString.h"
#include "RawCfgAccess.h"
#include "XmlStream.h"
#include "utils.h"

class CXBXmlCfgAccess : public CRawCfgAccess
{
public:
	CXBXmlCfgAccess( LPCTSTR szFileName = NULL, BOOL bDebug = FALSE  );
	int		Load( LPCTSTR szName = NULL, BOOL bDebug = FALSE );
	virtual int		Save( LPCTSTR szName = NULL );
};




#endif