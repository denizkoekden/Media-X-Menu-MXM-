/*********************************************************************************\
 * CommDebug.cpp
 * Class for handling debug info via networking
 * (C) 2003 MasterMind
\*********************************************************************************/


#include "debugclient.h" 
#include "CommDebug.h"
#include "MXM.h"



#ifdef XDEBUG

#define MB	(1024*1024)

void DEBUG_MEM_REPORT( void )
{
    MEMORYSTATUS stat;

	// Get the memory status.
    GlobalMemoryStatus( &stat );

    // Setup the output string.
	DEBUG_FORMAT( "%9d  total MB of virtual memory.\n%9d  free  MB of virtual memory.\n"
			"%9d  total    of physical memory.\n"
			"%9d  free     of physical memory.\n" 
			"%9d  total    of paging file.\n" 
			"%9d  free     of paging file.\n" 
			"%9d%% percent of memory is in use.\n",
			( stat.dwTotalVirtual / MB ),
			( stat.dwAvailVirtual / MB ),
			( stat.dwTotalPhys ),
			( stat.dwAvailPhys ),
			( stat.dwTotalPageFile / MB ),
			( stat.dwAvailPageFile / MB ), stat.dwMemoryLoad );
}

#ifdef USECOMMDBG
CDebugClient g_debugClient;
#endif

void InitDebug( char * szIPAddr, unsigned int uiPort, int iBlocking  )
{
#ifdef USECOMMDBG
	g_debugClient.Init( szIPAddr, uiPort, iBlocking );
#endif
}


void DEBUG_WAIT( void )
{
#ifdef USECOMMDBG
	g_debugClient.WaitKey();
#endif
}

void DEBUG_LINE( LPCTSTR szString )
{
//	int iLen;
	CStdStringA sOutput( szString );

	sOutput += _T("\r\n");

#ifdef USECOMMDBG
	g_debugClient.Send( (void *)sOutput.c_str(), sOutput.GetLength() );
#else
	OutputDebugString( sOutput );
#endif
}


void DEBUG_FORMAT(LPCTSTR szFmt, ...)
{
	CStdString sFmtOut;
	va_list argList;
	va_start(argList, szFmt);

	sFmtOut.FormatV(szFmt, argList);
	va_end(argList);
	DEBUG_LINE( sFmtOut );
}

void _DumpElement( CXMLElement * pElement, int iLevel )
{
	CStdString sSpacer;
	TMapStrings::iterator iterAttr;


	if ( iLevel )
	{
		sSpacer.Format( _T("%*s"), iLevel*2, _T("") );
	}
	iLevel++;

	// First, display name of node...
	DEBUG_FORMAT( _T("%sELEMENT: (%s)=(%s)"), sSpacer.c_str(), pElement->m_sName.c_str(), pElement->m_sValue.c_str() );

	// Next, display attributes:
	DEBUG_FORMAT( _T("%s  Attributes (%d)"), sSpacer.c_str(), pElement->m_msAttributes.size() );
	iterAttr = pElement->m_msAttributes.begin();
	while(iterAttr != pElement->m_msAttributes.end() )
	{
		DEBUG_FORMAT( _T("%s  * (%s)=(%s)"), sSpacer.c_str(), iterAttr->first.c_str(), iterAttr->second.c_str() );
		iterAttr++;
	}
}

void DumpNode( CXMLNode * pNode, int iLevel )
{
#if 0
	CStdString sSpacer;
	TMapStrings::iterator iterAttr;
	TXMLElementList::iterator iterElem;
	TXMLNodeList::iterator iterNode;

	if ( iLevel )
	{
		sSpacer.Format( _T("%*s"), iLevel*2, _T("") );
	}

	// First, display name of node...
	DEBUG_FORMAT( _T("%sNODE: (%s)"), sSpacer.c_str(), pNode->m_sName.c_str() );

	// Next, display attributes:
	DEBUG_FORMAT( _T("%s  Attributes (%d)"), sSpacer.c_str(), pNode->m_msAttributes.size() );
	iterAttr = pNode->m_msAttributes.begin();
	while(iterAttr != pNode->m_msAttributes.end() )
	{
		DEBUG_FORMAT( _T("%s  * (%s)=(%s)"), sSpacer.c_str(), iterAttr->first.c_str(), iterAttr->second.c_str() );
		iterAttr++;
	}

	// Next, display elements:
	DEBUG_FORMAT( _T("%s  Elements (%d)"), sSpacer.c_str(), pNode->m_msElements.size() );
	iterElem = pNode->m_msElements.begin();
	while(iterElem != pNode->m_msElements.end() )
	{
		_DumpElement( iterElem->GetElementPtr(), iLevel + 1 );
		iterElem++;
	}

	// Finally, do all subnodes
	DEBUG_FORMAT( _T("%s  Subnodes (%d)"), sSpacer.c_str(), pNode->m_nlNodes.size() );
	iterNode = pNode->m_nlNodes.begin();
	while(iterNode != pNode->m_nlNodes.end() )
	{
		DumpNode( iterNode->GetNodePtr(), iLevel + 1 );
		iterNode++;
	}
#endif

}



// ULONGLONG g_u64PerfTimer;
DWORD g_dwPerfTimer = 0;

void DBGPerfResetTimer( void );

void PerfGetPerformance( LPCTSTR szNamedPoint, BOOL bReset )
{
	CStdString sReturn;

//	ULONGLONG u64Difference;
	DWORD dwTime;

	dwTime = GetTickCount()-g_dwPerfTimer;

//	u64Difference = ReadTimeStampCounter()-g_u64PerfTimer;
//	u64Difference /= (ULONGLONG)1000;
//	dwTime = (DWORD)u64Difference;
	sReturn.Format( _T("==========Performance: %dms (%s)==========="), (int)dwTime, szNamedPoint );
	if ( bReset )
	{
		DBGPerfResetTimer();
	}
	DEBUG_LINE( sReturn );
}

void DBGPerfResetTimer( void )
{
	g_dwPerfTimer = GetTickCount();
	DEBUG_LINE(_T("----PerfTimer reset----") );
}


#else

void DumpNode( CXMLNode * pNode, int iLevel ) {}
void DEBUG_FORMAT(LPCTSTR szFmt, ...) {}
void DEBUG_LINE( LPCTSTR szString ) {}

ULONGLONG g_u64PerfTimer = 0;

void DBGPerfResetTimer( void )
{
	g_u64PerfTimer = ReadTimeStampCounter();
}

void PerfGetPerformance( LPCTSTR szNamedPoint, BOOL bReset )
{
	CStdString sReturn;

	ULONGLONG u64Difference, u64TimerVal;
	DWORD dwTime;

	if ( g_u64PerfTimer == 0 )
	{
		DBGPerfResetTimer();
	}

//	dwTime = GetTickCount()-g_dwPerfTimer;

	u64TimerVal = ReadTimeStampCounter();
	if ( u64TimerVal < g_u64PerfTimer )
	{
		DBGPerfResetTimer();
		u64TimerVal = ReadTimeStampCounter();
	}
	u64Difference = u64TimerVal-g_u64PerfTimer;
	u64Difference /= (ULONGLONG)1000000;
	dwTime = (DWORD)u64Difference;
	sReturn.Format( _T("==========Performance: %dms (%s)===========\r\n"), (int)dwTime, szNamedPoint );
	if ( bReset )
	{
		DBGPerfResetTimer();
	}
	OutputDebugString( sReturn );
}

// void PerfResetTimer( void ){}
void DEBUG_WAIT( void ) {}
void InitDebug( char * szIPAddr, unsigned int uiPort, int iBlocking  ) {}
void DEBUG_MEM_REPORT( void ) {}

#endif
