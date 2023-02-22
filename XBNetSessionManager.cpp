#include "StdAfx.h"
#include "xbnetsessionmanager.h"

CXBNetSessionManager::CXBNetSessionManager(void)
{
}

CXBNetSessionManager::~CXBNetSessionManager(void)
{
	StopSessions();

	if ( m_sessionList.size() )
	{
		// This should 'normally' not be required.
		// Why? Because stopping the sessions should 
		// cause them to fire the event handler and 
		// detach themselves.
		TNetSessionList::iterator iterList;
		CXBNetSession * pThread = NULL;

		iterList = m_sessionList.begin();
		while( iterList != m_sessionList.end() )
		{
			pThread = (*iterList);
			iterList = m_sessionList.erase( iterList );
			pThread->Release();
			if ( iterList == m_sessionList.end() )
			{
				iterList = m_sessionList.begin();
			}
			else
			{
				// iterList++;
			}
		}
	}
}

// Adds a Network Server Session to the list
bool CXBNetSessionManager::AddSession(CXBNetSession * pSession)
{
	bool bReturn = false;

	if ( pSession )
	{
		// We need to be notified when it's ready to go away
		pSession->SetEventHandler(this);

		// pThread->AddRef();
		m_sessionList.push_back( pSession );

		bReturn = true;
	}

	return bReturn;
}

// Tells all managed sessions we are closing and they should shutdown - Blocks until finished or timesout
HRESULT CXBNetSessionManager::StopSessions(DWORD dwTimeout, bool bForce)
{
	HRESULT hr = S_OK;
	int iSize;

	if ( iSize = m_sessionList.size() )
	{
		TNetSessionList::iterator iterList;
		CXBNetSession * pThread = NULL;

		iterList = m_sessionList.begin();
		while( iterList != m_sessionList.end() )
		{
			pThread = (*iterList);
			pThread->Stop(false);
			if ( iSize != m_sessionList.size() )
			{
				iSize = m_sessionList.size();
				iterList = m_sessionList.begin();
			}
			else
			{
				iterList++;
			}
		}
//		Sleep(20);
	}
	return hr;
}


void CXBNetSessionManager::SessionStarted( CXBNetSession * pThis )
{
	TRACE( _T("Started Net Session: 0x%08x\r\n"), (DWORD)pThis );
}

void CXBNetSessionManager::SessionEnded( CXBNetSession * pThis )
{
	// Delete the session from the list.
	DetachByHandle( pThis->GetID() );
//	pThis->Release();
	TRACE( _T("Ended Net Session: 0x%08x\r\n"), (DWORD)pThis );
}

CXBNetSession *	CXBNetSessionManager::DetachByHandle( HANDLE hHandle, bool bAddRef )
{
	CXBNetSession *pReturn = NULL;
	TNetSessionList::iterator iterList;

	iterList = m_sessionList.begin();
	while( iterList != m_sessionList.end() )
	{
		if ( (*iterList)->GetID() == hHandle )
		{
			pReturn = (*iterList);
			m_sessionList.erase( iterList );
			if ( bAddRef == false )
			{
				pReturn->Release();
			}
			break;
		}
		iterList++;
	}
	return pReturn;
}


CXBNetSession *	CXBNetSessionManager::DetachByIndex( int iIndex, bool bAddRef )
{
	CXBNetSession *pReturn = NULL;
	TNetSessionList::iterator iterList;

	iterList = m_sessionList.begin();
	while( iterList != m_sessionList.end() )
	{
		if ( iIndex == 0 )
		{
			pReturn = (*iterList);
			m_sessionList.erase( iterList );
			if ( bAddRef == false )
			{
				pReturn->Release();
			}
			break;
		}
		iIndex--;
		iterList++;
	}
	return pReturn;
}


CXBNetSession *	CXBNetSessionManager::GetByHandle( HANDLE hHandle )
{
	CXBNetSession *pReturn = NULL;
	TNetSessionList::iterator iterList;

	iterList = m_sessionList.begin();
	while( iterList != m_sessionList.end() )
	{
		if ( (*iterList)->GetID() == hHandle )
		{
			pReturn = (*iterList);
			pReturn->AddRef();
			break;
		}
		iterList++;
	}
	return pReturn;
}

CXBNetSession *	CXBNetSessionManager::GetByIndex( int iIndex )
{
	CXBNetSession *pReturn = NULL;
	TNetSessionList::iterator iterList;

	iterList = m_sessionList.begin();
	while( iterList != m_sessionList.end() )
	{
		if ( iIndex == 0 )
		{
			pReturn = (*iterList);
			pReturn->AddRef();
			break;
		}
		iIndex--;
		iterList++;
	}
	return pReturn;
}

void CXBNetSessionManager::DeleteByHandle( HANDLE hHandle )
{
	CXBNetSession *pThread;

	// Detach, without a preceding call to AddRef the aforementioned 
	// thread, should cause deletion, if it's OK
	pThread = DetachByHandle( hHandle );
//	if ( pThread )
//	{
//		pThread->Release();
//		// delete pThread;
//	}
}

int CXBNetSessionManager::GetCount( void )
{
	return m_sessionList.size();
}
