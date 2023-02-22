

#include "StdAfx.h"
#include "ASScheduler.h"
#include "MenuInfo.h"
#include "XBStateApp.h"


CActionScript * GetNewAppActionScript( LPCTSTR szActionScript, CActionScript * pParent, bool bIsContext );

/*

SET _success 1
IF#@ ScriptExists _OnLaunch GOTO DoOnLaunchA
GOTO NextLaunch
:DoOnLaunchA
CallScript _OnLaunch
IF %_success% != 1 GOTO CANCELOP
:NextLaunch
IF#@ ContextScriptExists OnLaunch GOTO DoOnLaunchB
GOTO RunApp
:DoOnLaunchB
CallItemScript OnLaunch
IF %_success% != 1 GOTO CANCELOP
:RunApp
LaunchCurrent
:CANCELOP


New commands:
LaunchContext
CallContextScript

New Functions:
ScriptExists
ContextScriptExists

*/

class CScriptScheduleInfo g_ScriptSchedule;

bool g_bSchedulerInit = false;

void InitializeActionScriptScheduler( void )
{
	if ( !g_bSchedulerInit )
	{
		InitializeCriticalSection( &g_ScriptSchedule.m_ScriptScheduleCS );
		g_ScriptSchedule.m_hScriptScheduleEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
		g_ScriptSchedule.m_ActionScriptSchedule.clear();
		g_ScriptSchedule.m_iID = 1;
		// We only need to set this once here, again only after prelaunch fails.
		//SetActionVar( _T("_success"), _T("1"), NULL );
		g_ScriptSchedule.m_bContinueScripts = true;
		// Launch thread here...
		g_ScriptSchedule.m_hThread = CreateThread( NULL, (1024*48), (LPTHREAD_START_ROUTINE)ActionScriptSchedHandlerProc, NULL, 0, NULL );

		
		g_ScriptSchedule.m_bInitialized = true;
		g_bSchedulerInit = true;
	}
}

DWORD ActionScriptSchedHandlerProc( LPVOID pParam )
{
	DWORD dwReturn = 0;
	CASScheduleItem * pScheduleItem = NULL;
	int iScriptCount = 0;
	

	while ( g_ScriptSchedule.m_bContinueScripts )
	{
		pScheduleItem = PopNextScheduledScript(&iScriptCount);
		if ( pScheduleItem == NULL )
		{
			DWORD dwPredictNextSchedule = GetNextScheduledTime();
			SetThreadPriority( g_ScriptSchedule.m_hThread, THREAD_PRIORITY_BELOW_NORMAL );
			// Sleep for a few...
			if ( dwPredictNextSchedule < 5 )
			{
				dwPredictNextSchedule = 5;
			}
			WaitForSingleObject( g_ScriptSchedule.m_hScriptScheduleEvent, dwPredictNextSchedule );
		}
		else
		{
			if ( pScheduleItem->m_pScript )
			{
				// Set "success" somewhere else...
				// pScheduleItem->m_pScript->SetEnvValue( _T("_Success"), _T("1") );
				SetThreadPriority( g_ScriptSchedule.m_hThread, THREAD_PRIORITY_ABOVE_NORMAL );
				g_MenuInfo.m_bInActionScript = true;
				GetStateApp()->m_bInActionScript = true;
				pScheduleItem->m_pScript->ExecuteScript();
				//if ( pScheduleItem->m_bPreLaunch )
				//{
				//	// Check result, and run it if OK
				//	int iSuccess = _tcstol( pScheduleItem->m_pScript->GetEnvValue( _T("_Success") ), NULL, 1 );
				//	if ( iSuccess )
				//	{
				//		// Launch app...
				//	}
				//	else
				//	{
				//		// Reset this. No need to worry about this at this point.
				//		// SetActionVar( _T("_success"), _("1"), NULL );
				//	}
				//}
				GetStateApp()->m_pdrawList = NULL;
				if ( pScheduleItem->m_dwInterval == 0 )
				{
					delete pScheduleItem;
				}
				g_MenuInfo.m_bInActionScript = false;
				GetStateApp()->m_bInActionScript = false;
			}
		}
	}
	return dwReturn;
}


bool RemoveScheduledScript( int iID )
{
	CASScheduleItem * pReturn = NULL;
	TScriptSchedule::iterator iterSchedule;
	bool bReturn = false;
	
	EnterCriticalSection( &g_ScriptSchedule.m_ScriptScheduleCS );
	if ( g_ScriptSchedule.m_ActionScriptSchedule.size() )
	{
		iterSchedule = g_ScriptSchedule.m_ActionScriptSchedule.begin();
		while( iterSchedule != g_ScriptSchedule.m_ActionScriptSchedule.end() )
		{
			if ( (*iterSchedule)->m_iID == iID )
			{
				pReturn = (CASScheduleItem *)(*iterSchedule);
				g_ScriptSchedule.m_ActionScriptSchedule.erase( iterSchedule );
				delete pReturn;
				bReturn = true;
				break;
			}
			iterSchedule++;
		}
	}
	LeaveCriticalSection( &g_ScriptSchedule.m_ScriptScheduleCS );
	
	return bReturn;
}

CASScheduleItem * PopNextScheduledScript( int * piCount )
{
	CASScheduleItem * pReturn = NULL;
	TScriptSchedule::iterator iterSchedule;
	DWORD dwTimeStamp, dwCheck;
	
	EnterCriticalSection( &g_ScriptSchedule.m_ScriptScheduleCS );
	dwTimeStamp = GetTickCount();
	if ( g_ScriptSchedule.m_ActionScriptSchedule.size() )
	{
		iterSchedule = g_ScriptSchedule.m_ActionScriptSchedule.begin();
		while( iterSchedule != g_ScriptSchedule.m_ActionScriptSchedule.end() )
		{
			dwCheck = (*iterSchedule)->m_dwInterval;
			if ( dwCheck )
			{
				dwCheck += (*iterSchedule)->m_dwTimeStamp;
				if ( dwCheck < dwTimeStamp )
				{
					pReturn = (CASScheduleItem *)(*iterSchedule);
					// We don't remove it if it's intervaled, unless count is ZERO
					if ( pReturn->m_iCount < 0 ) // Infinite run count
					{
						// Leave on list...
					}
					else if ( pReturn->m_iCount < 2 )
					{
						g_ScriptSchedule.m_ActionScriptSchedule.erase( iterSchedule );
						pReturn->m_dwInterval = 0;
						pReturn->m_iCount = 0;
					}
					else
					{
						pReturn->m_iCount--;
					}
					break;
				}
			}
			else
			{
				// One is scheduled NOW.
				pReturn = (CASScheduleItem *)(*iterSchedule);
				g_ScriptSchedule.m_ActionScriptSchedule.erase( iterSchedule );
				break;
			}
			iterSchedule++;
		}
	}
	// Reset timestamp for next run...
	if ( pReturn )
	{
		pReturn->m_dwTimeStamp = dwTimeStamp;
	}
	if ( piCount )
	{
		*piCount = g_ScriptSchedule.m_ActionScriptSchedule.size();
	}
	LeaveCriticalSection( &g_ScriptSchedule.m_ScriptScheduleCS );
	
	return pReturn;
}

DWORD GetNextScheduledTime( void )
{
	TScriptSchedule::iterator iterSchedule;
	DWORD dwReturn = 100;
	DWORD dwTimeStamp, dwCheck;
	
	EnterCriticalSection( &g_ScriptSchedule.m_ScriptScheduleCS );
	dwTimeStamp = GetTickCount();
	if ( g_ScriptSchedule.m_ActionScriptSchedule.size() )
	{
		iterSchedule = g_ScriptSchedule.m_ActionScriptSchedule.begin();
		while( iterSchedule != g_ScriptSchedule.m_ActionScriptSchedule.end() )
		{
			dwCheck = (*iterSchedule)->m_dwInterval;
			if ( dwCheck )
			{
				dwCheck += (*iterSchedule)->m_dwTimeStamp;
				if ( dwCheck < (dwTimeStamp+dwReturn) )
				{
					if ( dwCheck <= dwTimeStamp )
					{
						dwReturn = 1;
						break;
					}
					else
					{
						dwReturn = dwCheck-dwTimeStamp;
					}
				}
			}
			else
			{
				// One is scheduled NOW.
				dwReturn = 1;
				break;
			}
			iterSchedule++;
		}
	}
	LeaveCriticalSection( &g_ScriptSchedule.m_ScriptScheduleCS );
	return dwReturn;
}

int ScheduleScript( LPCTSTR szScript, DWORD dwInterval, int iCount, bool bPrelaunch, bool bContext )
{
	int iReturn = 0;
	CASScheduleItem * pItem = NULL;
	
	pItem = new CASScheduleItem(szScript, bContext );
	if ( pItem )
	{
		pItem->m_iID = g_ScriptSchedule.m_iID++;
		iReturn = pItem->m_iID;
		pItem->m_dwInterval = dwInterval;
		pItem->m_iCount = iCount;
		pItem->m_bPreLaunch = bPrelaunch;
		pItem->m_dwTimeStamp = GetTickCount();
		EnterCriticalSection( &g_ScriptSchedule.m_ScriptScheduleCS );
		// Push it onto the back of the list....
		g_ScriptSchedule.m_ActionScriptSchedule.push_back( pItem );
		LeaveCriticalSection( &g_ScriptSchedule.m_ScriptScheduleCS );
		SetEvent( g_ScriptSchedule.m_hScriptScheduleEvent );
	}
	return iReturn;
}

CASScheduleItem::CASScheduleItem( LPCTSTR szScript, bool bContext ) :
	m_pScript(NULL),
	m_iID(-1),
	m_dwTimeStamp(GetTickCount()),
	m_dwInterval(0),
	m_iCount(0),
	m_bPreLaunch(false)
{
	m_pScript = GetNewAppActionScript( szScript, NULL, bContext );
}

CASScheduleItem::~CASScheduleItem()
{
	if ( m_pScript )
	{
		delete m_pScript;
	}
}


