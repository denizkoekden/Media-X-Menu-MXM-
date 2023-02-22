/*********************************************************************************\
 * CommDebug.h
 * Class for handling debug info via networking
 * (C) 2003 MasterMind
\*********************************************************************************/

#ifndef __COMMDEBUG_H__
#define __COMMDEBUG_H__

#include "XmlTree.h"

void InitDebug( char * szIPAddr, unsigned int uiPort, int iBlocking  );
void DEBUG_LINE( LPCTSTR szString );
void DEBUG_FORMAT(LPCTSTR szFmt, ...);
void DEBUG_MEM_REPORT( void );
void PerfGetPerformance( LPCTSTR szNamedPoint, BOOL bReset );
void DumpNode( CXMLNode * pNode, int iLevel = 0 );
void DBGPerfResetTimer( void );

#endif // __COMMDEBUG_H__