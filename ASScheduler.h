

#pragma once

#include "ActionScript.h"
#include <list>


 
class CASScheduleItem
{
public:
	int				m_iID;
	CActionScript 	* m_pScript;
	DWORD			m_dwTimeStamp;
	DWORD			m_dwInterval;	// 0=Immediate
	int				m_iCount;	// -1=infinite (if m_dwInterval > 0 )
	bool			m_bPreLaunch;	
	CASScheduleItem( LPCTSTR szScript, bool bContext );
	~CASScheduleItem();
};

extern bool g_bSchedulerInit;

typedef list<CASScheduleItem *> TScriptSchedule;

DWORD ActionScriptSchedHandlerProc( LPVOID pParam );
void InitializeActionScriptScheduler( void );
bool RemoveScheduledScript( int iID );
//CASScheduleItem * PopNextScheduledScript( void );
DWORD GetNextScheduledTime( void );
CASScheduleItem * PopNextScheduledScript( int * piCount );
int ScheduleScript( LPCTSTR szScript, DWORD dwInterval, int iCount, bool bPrelaunch, bool bContext );


// int ScheduleScript( LPCTSTR szScript, DWORD dwInterval, int iCount, bool bPrelaunch );
// bool RemoveScheduledScript( int iID );
// CASScheduleItem * PopNextScheduledScript( void );
// DWORD GetNextScheduledTime( void );

class CScriptScheduleInfo
{
public:
	HANDLE 				m_hScriptScheduleEvent;
	HANDLE				m_hThread;
	CRITICAL_SECTION 	m_ScriptScheduleCS;
	TScriptSchedule 	m_ActionScriptSchedule;
	bool				m_bContinueScripts;
	int					m_iID;
	bool				m_bInitialized;
	CScriptScheduleInfo() :
		m_hScriptScheduleEvent(NULL),
		m_hThread(NULL),
		m_bContinueScripts(true),
		m_iID(1),
		m_bInitialized(false)
	{
		// InitializeCriticalSection(&m_ScriptScheduleCS);
	};
}; // g_ScriptSchedule;

