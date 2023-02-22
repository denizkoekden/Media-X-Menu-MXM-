#include "StdAfx.h"
#include "xbnetsession.h"
#include "XBNet.h"

CXBNetSession::CXBNetSession( SOCKET sock, CXBNetServer * pServer ) :
	m_eventHandler(NULL)
	,m_sockCmd(sock)
	,m_hID(NULL)
	,m_pServer(pServer)
	,m_iRefCount(0)
	,m_hThreadEnded(NULL)
{
	m_sqOutput.clear();
	m_hThreadEnded = CreateEvent( NULL, false, false, NULL );
	InitializeCriticalSection(&m_csCmdCtrl);
}


int CXBNetSession::Release( void )
{
	int iRefCount = 0;
	if ( this )
	{
		iRefCount = m_iRefCount;
		if ( m_iRefCount )
		{
			iRefCount = m_iRefCount--;
		}
		if ( m_iRefCount <= 0 )
		{
			delete this;
		}
	}
	return iRefCount;
}

CXBNetSession::~CXBNetSession(void)
{
	TRACE( _T("Deleting NetSession 0x%08x\r\n"), this );
	if ( m_hThreadEnded )
	{
		TRACE( _T("Closing NetSession Event Handle 0x%08x\r\n"), m_hThreadEnded );
		CloseHandle( m_hThreadEnded );
	}
	if ( m_sockCmd != INVALID_SOCKET )
	{
		// Clean up!!
		_SockClose(m_sockCmd);
		m_sockCmd = INVALID_SOCKET;
	}
	if ( m_hID )
	{
		CloseHandle( m_hID );
	}
	DeleteCriticalSection(&m_csCmdCtrl);
}

// Set the IP our session is connecting to
void CXBNetSession::SetIP(struct sockaddr addrSession)
{
	m_addrSession = addrSession;
}

// Initialize thread(s), other session information
HRESULT CXBNetSession::Initialize(void)
{
	HRESULT hr = E_FAIL;
	HANDLE hThread;

	hThread = CreateThread( NULL, (1024*48), ThreadHandlerProc, this, CREATE_SUSPENDED, NULL );
	if ( hThread != INVALID_HANDLE_VALUE )
	{
//		if ( g_MenuInfo.m_iFTPPriority < 5 )
//		{
//			g_MenuInfo.m_iFTPPriority = 5;
//		}
//		if ( g_MenuInfo.m_iFTPPriority > 11 )
//		{
//			g_MenuInfo.m_iFTPPriority = 11;
//		}
//		SetThreadPriority( hThread, g_MenuInfo.m_iFTPPriority );
		m_hID = hThread;
		hr = S_OK;
//		bReturn = true;
	}

	return hr;
}

// Begin Session  - starts thread(s)
HRESULT CXBNetSession::Start(void)
{
	HRESULT hr = E_FAIL;

	if ( m_hID != INVALID_HANDLE_VALUE )
	{
		::ResumeThread( m_hID );
		hr = S_OK;
	}

	FireSessionStarted();

	return hr;
}

// Ends session, gracefully, unless forced
HRESULT CXBNetSession::Stop(bool bForce)
{
	HRESULT hr = S_OK;
	// Can't count on "this" existing after we stop thread 
	HANDLE hEndEvent = m_hThreadEnded;
	HANDLE hThread = m_hID;

	// This will soon be gone
	m_hThreadEnded = NULL;
	m_hID = NULL;

	m_bContinue = false;

	if ( m_sockCmd != INVALID_SOCKET )
	{
		int iTimeout = 0;
		setsockopt( m_sockCmd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&iTimeout, sizeof(int) );
	}

	DWORD dwResult = WaitForSingleObject( hEndEvent, 1000 );

	if ( m_sockCmd != INVALID_SOCKET )
	{
		// Clean up!!
		_SockClose(m_sockCmd);
		m_sockCmd = INVALID_SOCKET;
	}
	if ( hThread )
	{
		CloseHandle( hThread );
	}

	TRACE( _T("Closing NetSession Event Handle 0x%08x\r\n"), hEndEvent );
	CloseHandle(hEndEvent);

	return hr;
}

// Tell event handler we started our session
void CXBNetSession::FireSessionStarted(void)
{
	if ( m_eventHandler )
	{
		m_eventHandler->SessionStarted(this);
	}
}

// Tell event handler we ended our session
void CXBNetSession::FireSessionEnded(void)
{
	// We may be gone after this....
	CXBNetSessionEvents * pEventHandler = m_eventHandler;
	if ( pEventHandler )
	{
		pEventHandler->SessionEnded(this);
	}
}

// Set the event handler for our session
void CXBNetSession::SetEventHandler(CXBNetSessionEvents * pEventHandler)
{
	m_eventHandler = pEventHandler;
}


DWORD CXBNetSession::ThreadHandlerProc( LPVOID pParam )
{
	CXBNetSession * pThis = (CXBNetSession *)pParam;

	return pThis->Serve();
}

DWORD CXBNetSession::Serve ( void )
{
	DWORD dwReturn = 0;
	CStdString sCmd, sArg;
	HANDLE hEndEvent = m_hThreadEnded;

	m_bContinue = true;

	DoGreeting();

	while( m_bContinue )
	{
		Sleep(1);
		if ( GetCommand( sCmd, sArg ) == S_OK )
		{
			DoCommand( sCmd, sArg );
		}
		else
		{
			m_bContinue = false;
		}
	}

	FireSessionEnded();

	SetEvent(hEndEvent);

	return dwReturn;
}

HRESULT CXBNetSession::GetCommand( CStdString & sCmd, CStdString & sArg )
{
	HRESULT hrReturn = E_FAIL;
	CStdString sLine;
	int iPos = -1;

	if ( RecvLine( sLine ) )
	{
		iPos = sLine.Find( _T(' ') );
		if ( iPos > 0 )
		{
			sCmd = sLine.Left( iPos );
			sArg = sLine.Mid( iPos+1 );
		}
		else
		{
			iPos = sLine.Find( _T('\t') );
			if ( iPos > 0 )
			{
				sCmd = sLine.Left( iPos );
				sArg = sLine.Mid( iPos+1 );
			}
			else
			{
				// no arg...
				sCmd = sLine;
				sArg = _T("");
			}
		}
		sCmd.Trim();
		sArg.Trim();
		hrReturn = S_OK;
	}
	return hrReturn;
}


bool CXBNetSession::IsRecvReady( SOCKET sock )
{
	bool bReturn = false;
	fd_set fdRead;
	timeval timeVal;

	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;
	FD_ZERO( &fdRead );
	FD_SET( sock, &fdRead );
	if ( select( 1, &fdRead, NULL, NULL, &timeVal ) != 0 )
	{
		bReturn = true;
	}
	return bReturn;
}


bool CXBNetSession::RecvLine( CStdString & sLine )
{
	bool bReturn = true;
	bool bFinish = false;

	ZeroMemory( m_pszCmdBuffer, 1024 );
	if ( RecvCmd( (LPSTR)m_pszCmdBuffer, 1023 ) == S_OK )
	{
		// 30 Second timeout
		sLine = m_pszCmdBuffer;
		while ( ( sLine.Find( _T('\r') ) == -1 ) && ( sLine.Find( _T('\n') ) == -1 ) && ( sLine.GetLength() < 4096 ) )
		{
			ZeroMemory( m_pszCmdBuffer, 1024 );
			if ( RecvCmd( m_pszCmdBuffer, 1023 ) == S_OK )
			{
				sLine += m_pszCmdBuffer;
			}
//			else
//			{
//				bReturn = false;
//			}
		}
		sLine.Trim();
	}
	else
	{
		bReturn = false;
	}
	return bReturn;
}

HRESULT	CXBNetSession::RecvCmd( LPSTR szBuff, int iLen )
{
	HRESULT hrReturn = E_FAIL;
	int iBytes = 0;
	char * szData = (char *)szBuff;

	ZeroMemory( szData, iLen );

	EnterCriticalSection(&m_csCmdCtrl);
	while( !IsRecvReady(m_sockCmd) )
	{
		LeaveCriticalSection(&m_csCmdCtrl);
		// Do we send anything at this point?
		// If so, let's send it.
		if ( m_sqOutput.size() )
		{
			CStdString sCmd = m_sqOutput.front();
			m_sqOutput.pop_front();
			SendCmd( sCmd.c_str(), sCmd.GetLength() );
		}
		else
		{
			Sleep(10);
		}
		EnterCriticalSection(&m_csCmdCtrl);
	}
	iBytes = recv( m_sockCmd, szData, iLen-1, 0 );
	LeaveCriticalSection(&m_csCmdCtrl);
	if ( iBytes > 0 )
	{
		szData[iLen] = 0;
		hrReturn = S_OK;
	}
	else if ( iBytes == 0 )
	{
		szData[0] = 0;
		hrReturn = E_FAIL;
		// m_bLoggedOn = false;
	}
	return hrReturn;
}

HRESULT	CXBNetSession::SendCmd( LPCSTR szBuff, int iLen )
{
	HRESULT hrReturn = E_FAIL;
	int iBytes;
	char * szData = (char *)szBuff;

	while( true )
	{
		EnterCriticalSection(&m_csCmdCtrl);
		iBytes = send( m_sockCmd, szData, iLen, 0 );
		LeaveCriticalSection(&m_csCmdCtrl);
		if ( iBytes> 0 && iBytes < iLen )
		{
			szData += iBytes;
			iLen -= iBytes;
		}
		else if ( iBytes == iLen )
		{
			hrReturn = S_OK;
			break;
		}
		else
		{
			hrReturn = E_FAIL; // WSAGetLastError();
			break;
		}
	}
	return hrReturn;
}

HRESULT CXBNetSession::Recv( SOCKET sock, LPSTR szBuff, int iLen )
{
	HRESULT hrReturn = E_FAIL;
	int iBytes = 0;
	char * szData = (char *)szBuff;

	ZeroMemory( szData, iLen );

//	while( !IsRecvReady() )
//	{
//		Sleep(1);
//	}
	iBytes = recv( sock, szData, iLen-1, 0 );
	if ( iBytes > 0 )
	{
		szData[iLen] = 0;
		hrReturn = S_OK;
	}
	else if ( iBytes == 0 )
	{
		szData[0] = 0;
		hrReturn = E_FAIL;
		// m_bLoggedOn = false;
	}
	return hrReturn;
}

HRESULT CXBNetSession::Send( SOCKET sock, LPCSTR szBuff, int iLen )
{
	HRESULT hrReturn = E_FAIL;
	int iBytes;
	char * szData = (char *)szBuff;

	while( true )
	{
		iBytes = send( sock, szData, iLen, 0 );
		if ( iBytes> 0 && iBytes < iLen )
		{
			szData += iBytes;
			iLen -= iBytes;
		}
		else if ( iBytes == iLen )
		{
			hrReturn = S_OK;
			break;
		}
		else
		{
			hrReturn = E_FAIL; // WSAGetLastError();
			break;
		}
	}
	return hrReturn;
}

HRESULT CXBNetSession::DoCommand( LPCTSTR szCmd, LPCTSTR szArg )
{
	HRESULT hr = S_OK;
	return hr;
}

void CXBNetSession::Msg( int iCode, LPCTSTR szLine, ... )
{
	CStdString sFmtOut;
	CStdString sMsg;

	if ( szLine && _tcslen( szLine ) )
	{
		va_list argList;
		va_start(argList, szLine);

//		if ( argList )
//		{
			sFmtOut.FormatV(szLine, argList);
//		}
//		else
//		{
//			// No arguments
//			sFmtOut = szLine;
//		}
		va_end(argList);

		sMsg.Format( _T("%d %s\r\n" ), iCode, sFmtOut.c_str() );
	}
	else
	{
		sMsg.Format( _T("%d\r\n" ), iCode );
	}
//	m_sqOutput.push_back( sMsg );
 	SendCmd( sMsg, sMsg.GetLength() );
}

void CXBNetSession::QueueMsg( int iCode, LPCTSTR szLine, ... )
{
	CStdString sFmtOut;
	CStdString sMsg;

	if ( szLine && _tcslen( szLine ) )
	{
		va_list argList;
		va_start(argList, szLine);

//		if ( argList )
//		{
			sFmtOut.FormatV(szLine, argList);
//		}
//		else
//		{
//			// No arguments
//			sFmtOut = szLine;
//		}
		va_end(argList);

		sMsg.Format( _T("%d %s\r\n" ), iCode, sFmtOut.c_str() );
	}
	else
	{
		sMsg.Format( _T("%d\r\n" ), iCode );
	}
	m_sqOutput.push_back( sMsg );
	// SendCmd( sMsg, sMsg.GetLength() );
}

void CXBNetSession::DoGreeting( void )
{
}
