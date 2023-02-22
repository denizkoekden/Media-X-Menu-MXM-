#include "StdAfx.h"
#include "XBNet.h"
#include "xbnetserver.h"

CXBNetServer::CXBNetServer(void)
: m_wPort(23)
, m_bServerRunning(false)
, m_iMaxUsersAllowed(1)
, m_iCurrentNumberOfUsers(0)
, m_bShutdown(false)
{
	::WSAStartup( MAKEWORD(2,2), &m_WSAData );
}

CXBNetServer::~CXBNetServer(void)
{
	Stop();
	::WSACleanup();
}

// Cranks up the server, if not already running
HRESULT CXBNetServer::Start(void)
{
	HRESULT hr = E_FAIL;
	if ( !m_bServerRunning )
	{
		struct sockaddr_in addr;
		int iSize = sizeof( struct sockaddr_in );

		m_sockListen = socket ( AF_INET, SOCK_STREAM, 0 );
		if ( m_sockListen != INVALID_SOCKET )
		{
			ZeroMemory( &addr, iSize );
			addr.sin_family = AF_INET;
			addr.sin_port = htons( m_wPort );
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
			int iRet = bind( m_sockListen, (struct sockaddr *)&addr, iSize );
			if ( iRet == 0 )
			{
				iRet = _SockListen( m_sockListen, m_iMaxUsersAllowed+1 );
				if ( iRet == 0 )
				{
					m_hServerThread = CreateThread( NULL, 0, ThreadHandlerProc, this, 0, NULL );
					hr = S_OK;
				}
			}
		}
		if ( SUCCEEDED(hr) )
		{
			m_bServerRunning = true;
		}
	}
	else
	{
		hr = S_OK;
	}
	return hr;
}

// Shuts down the FTP server, useful to reconfigure the server
HRESULT CXBNetServer::Stop(void)
{
	HRESULT hr = E_NOTIMPL;
	if ( m_bServerRunning )
	{
		m_bShutdown = true;

		if ( SUCCEEDED(hr) )
		{
			m_bServerRunning = false;
		}
		_SockClose( m_sockListen );
		m_sockListen = INVALID_SOCKET;
	}
	return hr;
}


// Callback for main thread
DWORD CXBNetServer::ThreadHandlerProc(LPVOID pParam)
{
	CXBNetServer * pThis = (CXBNetServer *)pParam;

	return pThis->ThreadHandler();
}

// Handles the actual thread, relative to the object as a member
DWORD CXBNetServer::ThreadHandler(void)
{
	DWORD dwReturn = 0;
	SOCKET sockNew;
	struct sockaddr_in * addr = NULL;
	int iSize = sizeof(struct sockaddr_in);

	m_bShutdown = false;
	while( !m_bShutdown )
	{
		iSize = sizeof(struct sockaddr_in);
		sockNew = _SockAccept( m_sockListen, (struct sockaddr *)addr, &iSize );
		if ( sockNew != -1 )
		{
//			struct sockaddr_in addr2;
//			iSize = sizeof(struct sockaddr_in);

			// Must do security here to make sure this is a valid IP and not blocked
			// Then we add session
			SpawnSession( sockNew );
#if 0
			CFTPThread * pThread = new CFTPThread( sockNew, this );

			if ( pThread )
			{
				if ( getpeername( sockNew, (struct sockaddr *)&addr2, &size ) == -1 )
				{
					// Failed to get peername
				}
				pThread->SetIP( &addr2 );
				pThread->SpawnThread();
				if ( m_threadMgr.AddThread( pThread ) )
				{
					pThread->ResumeThread();
				}
				else
				{
					// Failed! Adding Thread object to manager
					delete pThread;
				}
			}
			else
			{
				// Failed! Object Creation
			}
#endif
		}
	}
	return dwReturn;
}


// Spawn the session if it has a valid IP address
HRESULT CXBNetServer::SpawnSession(SOCKET sockSession)
{
	HRESULT hr = E_FAIL;
	struct sockaddr addr2;
	int iSize = sizeof(struct sockaddr);

	if ( getpeername( sockSession, &addr2, &iSize ) != -1 )
	{
		// Got IP, seems to be a valid connection, so create new session and continue
		hr = CreateSession( sockSession, &addr2 );
	}
	return hr;
}

HRESULT CXBNetServer::CreateSession( SOCKET sockSession, struct sockaddr * addrSession )
{
	HRESULT hr = E_FAIL;

//	CXBNetSession * pSession = new CXBNetSession( sockSession, this );
	CXBNetSession * pSession = NewSession( sockSession );

	if ( pSession )
	{
		pSession->SetIP( *addrSession );
		pSession->Initialize();
		if ( m_sessionMgr.AddSession( pSession ) )
		{
			pSession->Start();
			hr = S_OK;
		}
		else
		{
			// Failed! Adding Session object to manager
			delete pSession;
		}
	}
	return hr;
}


CXBNetSession * CXBNetServer::NewSession( SOCKET sockSession )
{
	CXBNetSession * pReturn = NULL;

	pReturn = new CXBNetSession( sockSession, this );

	return pReturn;
}


// Configure using an XML-based binary tree node
void CXBNetServer::Configure(CXMLNode * pNode)
{
	if ( pNode )
	{
		// Configure server here... configure users as well.
		m_wPort = (WORD)pNode->GetLong( NULL, _T("ServerPort"), 21 );
	}
}
