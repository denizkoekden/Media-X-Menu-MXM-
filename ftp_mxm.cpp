

#include <xtl.h>
#include "utils.h"
#include "ftp_mxm.h"
#include "MenuInfo.h"
#include "CommDebug.h"
#include "MXM.h"
#define MAXUSERS 8

CFTPServer::CFTPServer( WORD wPort ) :
	m_wPort(wPort)
	,m_sAnonRoot("\\")
	,m_bAllowAnon(true)
	,m_bRunning(false)
	,m_hServerThread(NULL)
	,m_sockListen(INVALID_SOCKET)
	,m_callbackExec(NULL)
	,m_callbackLogon(NULL)
	,m_callbackLogoff(NULL)
	,m_ctxExec(NULL)
	,m_ctxLogon(NULL)
	,m_ctxLogoff(NULL)
{
	WSADATA WSAData;
	::WSAStartup( MAKEWORD(2,2), &WSAData );
//		m_userMgr.AddEntry( _T("xbox"), _T("xbox"), _T("\\") );
}

void CFTPServer::Configure( CXMLNode * pNode )
{
	if ( pNode )
	{
		// Configure server here... configure users as well.
		m_bAllowAnon = (pNode->GetBool( NULL, _T("AllowAnon"), TRUE ) == TRUE );
		m_wPort = (WORD)pNode->GetLong( NULL, _T("ServerPort"), 21 );
		m_sAnonRoot = pNode->GetString( NULL, _T("AnonRoot"), _T("\\") );
		CXMLNode * pUserNode = NULL;
		int iIndex=0;

		while( pUserNode = pNode->GetNode( _T("user"), iIndex++ ) )
		{
			// Configure for the user
			m_userMgr.AddEntry( pUserNode );
		}
	}
	if ( GetUserCount() == 0 )
	{
		m_userMgr.AddEntry( _T("xbox"), _T("xbox"), _T("\\") );
	}
}

void CFTPServer::Configure( LPCTSTR szFile )
{
	CXMLLoader ldrConfig;

	Configure( ldrConfig.LoadXMLNodes( szFile ) );
}

HRESULT	CFTPServer::Start( void )
{
	HRESULT hr = E_FAIL;
	if ( !m_bRunning )
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
				iRet = listen( m_sockListen, MAXUSERS );
				if ( iRet == 0 )
				{
					m_hServerThread = CreateThread( NULL, 0, ThreadHandlerFunc, this, 0, NULL );
					hr = S_OK;
				}
			}
		}
	}
	else
	{
		// Already running!
		hr = S_OK;
	}
	return hr;
}


HRESULT CFTPServer::Stop( void )
{
	if ( m_bRunning )
	{
		m_bShutdown = true;
	}
	return S_OK;
}



DWORD CFTPServer::ThreadHandlerFunc( LPVOID pParam )
{
	CFTPServer * pThis = (CFTPServer *)pParam;

	return pThis->ThreadHandler();
}

void CFTPServer::DeleteThreadByHandle( HANDLE hID )
{
	m_threadMgr.DeleteByHandle( hID );
}

HRESULT CFTPServer::ValidateLogon( LPCTSTR szName, LPCTSTR szPassword, CStdString & sRoot )
{
	HRESULT hr = E_FAIL;
	CStdString sName( szName );
	CStdString sPassword( szPassword );

	sName.Trim();
	sPassword.Trim();
	if ( sName.GetLength() && sPassword.GetLength() )
	{
		if ( m_bAllowAnon && ( _tcsicmp(sName, _T("anonymous") ) == 0 ) )
		{
			sRoot = m_sAnonRoot;
			hr = S_OK;
		}
		else
		{
			if ( m_userMgr.ValidateUser( sName, sPassword ) )
			{
				sRoot = m_userMgr.GetRoot( sName );
				hr = S_OK;
			}
		}
		if ( SUCCEEDED( hr ) )
		{
			// Call callback...
			FireLogon();
		}
	}	
	return hr;
}


void CFTPServer::FireLogon( void )
{
	if ( m_callbackLogon )
	{
		m_callbackLogon( m_ctxLogon );
	}
}

void CFTPServer::FireLogoff( void )
{
	if ( m_callbackLogoff )
	{
		m_callbackLogoff( m_ctxLogoff );
	}
}

HRESULT CFTPServer::FireExec( LPCTSTR szPath, LPCTSTR szCmdLine )
{
	HRESULT hr = E_FAIL;
	if ( m_callbackExec )
	{
		hr = m_callbackExec( m_ctxExec, szPath, szCmdLine );
	}
	return hr;
}


void CFTPServer::SetCallbackExec( CallbackExec cbExec, PVOID pCtx )
{
	m_callbackExec = cbExec;
	m_ctxExec = pCtx;
}

void CFTPServer::SetCallbackLogon( CallbackLogon cbLogon, PVOID pCtx )
{
	m_callbackLogon = cbLogon;
	m_ctxLogon = pCtx;
}

void CFTPServer::SetCallbackLogoff( CallbackLogoff cbLogoff, PVOID pCtx )
{
	m_callbackLogoff = cbLogoff;
	m_ctxLogoff = pCtx;
}

// Replaces CFTPServer::Serve()
DWORD CFTPServer::ThreadHandler( void )
{
	DWORD dwReturn = 0;
	SOCKET sockNew;
	struct sockaddr_in * addr = NULL;
	int size = sizeof(struct sockaddr_in);

	m_bShutdown = false;
	while( !m_bShutdown )
	{
		sockNew = accept( m_sockListen, (struct sockaddr *)addr, &size );
		if ( sockNew != -1 )
		{
			struct sockaddr_in addr2;
			size = sizeof(struct sockaddr_in);

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
		}
	}
	return dwReturn;
}

bool CFTPServer::GetUserInfo( int iIndex, CStdString & sName, CStdString & sPassword, CStdString & sCurDir, CStdString & sIP )
{
	bool bReturn = false;
	CFTPThread * pThreadEntry;

	pThreadEntry = m_threadMgr.GetByIndex( iIndex );
	if ( pThreadEntry )
	{
		bReturn = pThreadEntry->GetUserInfo( sName, sPassword, sCurDir, sIP );
		pThreadEntry->Release();
	}
	return bReturn;
}


CFTPThreadManager::CFTPThreadManager()
{
}

CFTPThreadManager::~CFTPThreadManager()
{
	TFTPThreadList::iterator iterList;
	CFTPThread * pThread = NULL;

	iterList = m_threadList.begin();
	while( iterList != m_threadList.end() )
	{
		pThread = (*iterList);
		iterList = m_threadList.erase( iterList );
		pThread->Release();
		if ( iterList == m_threadList.end() )
		{
			iterList = m_threadList.begin();
		}
		iterList++;
	}
}

bool CFTPThreadManager::AddThread( CFTPThread *pThread )
{
	bool bReturn = false;

	if ( pThread )
	{
		pThread->AddRef();
		m_threadList.push_back( pThread );
		bReturn = true;
	}
	return bReturn;
}

CFTPThread *	CFTPThreadManager::DetachByHandle( HANDLE hHandle, bool bAddRef )
{
	CFTPThread *pReturn = NULL;
	TFTPThreadList::iterator iterList;

	iterList = m_threadList.begin();
	while( iterList != m_threadList.end() )
	{
		if ( (*iterList)->GetID() == hHandle )
		{
			pReturn = (*iterList);
			m_threadList.erase( iterList );
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

CFTPThread *	CFTPThreadManager::DetachByIndex( int iIndex, bool bAddRef )
{
	CFTPThread *pReturn = NULL;
	TFTPThreadList::iterator iterList;

	iterList = m_threadList.begin();
	while( iterList != m_threadList.end() )
	{
		if ( iIndex == 0 )
		{
			pReturn = (*iterList);
			m_threadList.erase( iterList );
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

CFTPThread *	CFTPThreadManager::GetByHandle( HANDLE hHandle )
{
	CFTPThread *pReturn = NULL;
	TFTPThreadList::iterator iterList;

	iterList = m_threadList.begin();
	while( iterList != m_threadList.end() )
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

CFTPThread *	CFTPThreadManager::GetByIndex( int iIndex )
{
	CFTPThread *pReturn = NULL;
	TFTPThreadList::iterator iterList;

	iterList = m_threadList.begin();
	while( iterList != m_threadList.end() )
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

void			CFTPThreadManager::DeleteByHandle( HANDLE hHandle )
{
	CFTPThread *pThread;

	// Detach, without a preceding call to AddRef the aforementioned 
	// thread, should cause deletion, if it's OK
	pThread = DetachByHandle( hHandle );
//	if ( pThread )
//	{
//		pThread->Release();
//		// delete pThread;
//	}
}

int CFTPThreadManager::GetCount( void )
{
	return m_threadList.size();
}

#define FTP_WRITEBLOCKSIZE 32768

CFTPThread::CFTPThread( SOCKET sock, CFTPServer * pServer ) :
	m_iRefCount(0)
	,m_bLoggedOn(false)
	,m_bPassive(false)
	,m_bCanDelete(true)
	,m_wPassivePort(0)
	,m_bRest(false)
	,m_dwRest(0)
	,m_pasvSvr(NULL)
	,m_sockCmd(sock)
	,m_sockListen(INVALID_SOCKET)
	,m_sockData(INVALID_SOCKET)
	,m_sockPasv(INVALID_SOCKET)
	,m_hID(NULL)
	,m_bCfgIP(false)
	,m_pbTransferBuffer(NULL)
	,m_pServer(pServer)
{
	DEBUG_FORMAT( _T("FTP[0x%08x]: New Thread Created"), this );
	srand(GetTickCount());
	ZeroMemory( m_pszCmdBuffer, 1024 );
	m_pbTransferBuffer = new char [FTP_WRITEBLOCKSIZE];
	GetIP();
}

void CFTPThread::GetIP( void )
{
	if ( g_MenuInfo.m_bXnaValid )
	{
		m_sHostIP.Format( _T("%d.%d.%d.%d"), g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b1, g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b2, g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b3, g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b4);
		m_h1 = g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b1;
		m_h2 = g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b2;
		m_h3 = g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b3;
		m_h4 = g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b4;
		m_bCfgIP = true;
	}
	else
	{
		if ( g_MenuInfo.m_bNetParamsSet )
		{
			sockaddr_in addr;
			addr.sin_addr.S_un.S_addr = inet_addr (g_MenuInfo.CurrentNetParams.m_sIP);
			m_sHostIP = g_MenuInfo.CurrentNetParams.m_sIP;
			m_h1 = addr.sin_addr.S_un.S_un_b.s_b1;
			m_h2 = addr.sin_addr.S_un.S_un_b.s_b2;
			m_h3 = addr.sin_addr.S_un.S_un_b.s_b3;
			m_h4 = addr.sin_addr.S_un.S_un_b.s_b4;
			m_bCfgIP = true;
		}
		else
		{
			g_MenuInfo.m_dwXnaResult = XNetGetTitleXnAddr(&g_MenuInfo.m_xnaInfo);
			if ( g_MenuInfo.m_xnaInfo.ina.S_un.S_addr && g_MenuInfo.m_bNetParamsSet )
			{
				g_MenuInfo.m_bXnaValid = true;
			}
			if ( g_MenuInfo.m_xnaInfo.ina.S_un.S_addr  )
			{
				m_sHostIP.Format( _T("%d.%d.%d.%d"), g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b1, g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b2, g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b3, g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b4);
				m_h1 = g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b1;
				m_h2 = g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b2;
				m_h3 = g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b3;
				m_h4 = g_MenuInfo.m_xnaInfo.ina.S_un.S_un_b.s_b4;
				m_bCfgIP = true;
			}
			else
			{
				m_sHostIP = _T("0.0.0.0");
			}
		}
	}
}

CStdString CFTPThread::GetCurDir( void )
{
	if ( m_sCurDirBackup.Compare( m_sCurDir ) )
	{
		m_sCurDir = m_sCurDirBackup;
	}
	return 	m_sCurDir;
}

void CFTPThread::SetCurDir( LPCTSTR szCurDir )
{
	CStdString sNewDir(szCurDir);
	sNewDir.Replace( _T("\\\\"), _T("\\") );
	sNewDir.Replace( _T("\\\\"), _T("\\") );
	DEBUG_FORMAT( _T("FTP::SetCurDir(%s) %s->%s [0x%08x]"), szCurDir, m_sCurDir.c_str(), sNewDir.c_str(), this );
	m_sCurDir = sNewDir;
	m_sCurDirBackup = sNewDir;
}

bool CFTPThread::RecvLine( CStdString & sLine )
{
	bool bReturn = false;
//	DWORD dwTimeOut = 0;

	ZeroMemory( m_pszCmdBuffer, 1024 );
	if ( Recv( m_sockCmd, (LPSTR)m_pszCmdBuffer, 1023 ) == S_OK )
	{
		// 30 Second timeout
//		dwTimeOut = GetTickCount()+30000;
		sLine = m_pszCmdBuffer;
		while ( ( sLine.Find( _T('\r') ) == -1 ) && ( sLine.Find( _T('\n') ) == -1 ) && ( sLine.GetLength() < 4096 ) )
		{
			ZeroMemory( m_pszCmdBuffer, 1024 );
			if ( Recv( m_sockCmd, m_pszCmdBuffer, 1023 ) == S_OK )
			{
				sLine += m_pszCmdBuffer;
			}
		}
		sLine.Trim();
		bReturn = true;
	}
	return bReturn;
}

CFTPThread::~CFTPThread()
{
	DEBUG_FORMAT( _T("FTP[0x%08x]: Thread Destroyed"), this );
	if ( m_sockPasv != INVALID_SOCKET )
	{
		shutdown( m_sockPasv, 2 );
		closesocket( m_sockPasv );
	}

	shutdown( m_sockCmd, 2 );
	closesocket( m_sockCmd );

	if ( m_pbTransferBuffer )
	{
		delete [] m_pbTransferBuffer;
	}
}


HRESULT	CFTPThread::Serve ( void )
{
	HRESULT hrReturn = S_OK;

	if ( SUCCEEDED( DoLogin() ) )
	{
		g_MenuInfo.Xbox.m_iNumFTPUsers++;
		m_bLoggedOn = true;
		DoWelcome();
		while( m_bLoggedOn )
		{
			CStdString sCmd, sArg;
			if ( GetCommand( sCmd, sArg ) == S_OK )
			{
				DoCommand( sCmd, sArg );
			}
			else
			{
				m_bLoggedOn = false;
			}
		}
	}
	g_MenuInfo.Xbox.m_iNumFTPUsers--;
	DoLogout();
	return hrReturn;
}



HRESULT CFTPThread::GetFile( LPCTSTR szFilename, bool bAppend )
{
	HRESULT hrReturn = E_FAIL;
	bool bSucceeded = false;
	DWORD	dwBuffSize = FTP_WRITEBLOCKSIZE;
	char * buff = m_pbTransferBuffer;
	char smallbuff[4096];

	if ( buff == NULL )
	{
		buff = smallbuff;
		dwBuffSize = 4096;
	}

	if ( !m_bPassive )
	{
		SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons (m_wRemotePort);
		addr.sin_addr.s_addr = inet_addr (m_sRemoteIP);

		if (connect (sock, (struct sockaddr *)&addr, sizeof (struct sockaddr)) != -1)
		{
			HANDLE hFile;
			if (!bAppend)
			{
				hFile = ::CreateFile (szFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			}
			else
			{
				hFile = ::CreateFile (szFilename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				SetFilePointer (hFile, 0, 0, FILE_END);
			}
			
			if (hFile == INVALID_HANDLE_VALUE)
			{
				DWORD dwError = GetLastError();
				Msg( 450, _T("Cannot access file.") );
			}
			else
			{
				int nBytes = recv (sock, buff, dwBuffSize, 0);
				while ((nBytes != 0) && (nBytes != -1))
				{
					unsigned long nBytesW = 0, nBytesTotal = 0;
					while ((int)nBytesTotal != nBytes)
					{
						BOOL bWriteSucc = WriteFile (hFile, buff, nBytes, &nBytesW, NULL);
						if (bWriteSucc == 0)
						{
							Msg (450, _T("Cannot access file.") );
							return E_FAIL;
						}
						nBytesTotal += nBytesW;
					}
					nBytes = recv (sock, buff, dwBuffSize, 0);
				}
				CloseHandle (hFile);
				if (nBytes == 0)
				{
					Msg (226, _T("Transfer Complete.") );
					bSucceeded = true;
				}
				else
				{
					Msg (426, "Connection closed; transfer aborted.");
					// return E_FAIL;
				}
			}
			shutdown (sock, 2);
			closesocket(sock);
			hrReturn = S_OK;
		}
	}
	else
	{
		GetPasvSocket();
		if ( m_sockPasv != INVALID_SOCKET)
		{
			HANDLE hFile;

			if (!bAppend)
			{
				hFile = ::CreateFile (szFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			}
			else
			{
				hFile = ::CreateFile (szFilename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				SetFilePointer (hFile, 0, 0, FILE_END);
			}
			if (hFile == INVALID_HANDLE_VALUE)
			{
				Msg (450, _T("Cannot access file."));
			}
			else
			{
				int nBytes = recv (m_sockPasv, buff, dwBuffSize, 0);
				while ((nBytes != 0) && (nBytes != -1))
				{
					unsigned long nBytesW = 0, nBytesTotal = 0;
					while ((int)nBytesTotal != nBytes)
					{
						BOOL bWriteSucc = WriteFile (hFile, buff, nBytes, &nBytesW, NULL);
						if (bWriteSucc == 0)
						{
							Msg (450, _T("Cannot access file."));
							hrReturn = E_FAIL;
							return E_FAIL;
						}
						nBytesTotal += nBytesW;
					}
					nBytes = recv (m_sockPasv, buff, dwBuffSize, 0);
				}
				CloseHandle (hFile);
				if (nBytes == 0)
				{
					Msg (226, _T("Transfer Complete.") );
					bSucceeded = true;
				}
				else
				{
					Msg (426, "Connection closed; transfer aborted.");
					// return E_FAIL;
				}
			}
			shutdown (m_sockPasv, 2);
			closesocket( m_sockPasv );
			m_sockPasv = INVALID_SOCKET;
			m_bPassive = false;

		}

		// InitiatePasvListen();

		//// Restart the PASV listen() call to get our socket
		//SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
		//struct sockaddr_in addr, *addr2 = NULL;
		//int size = sizeof (struct sockaddr_in);

		//ZeroMemory (&addr, size);
		//addr.sin_family = AF_INET;
		//addr.sin_port = htons (m_wPassivePort);
		//addr.sin_addr.s_addr = htonl (INADDR_ANY);
		//int ret = bind (sock, (struct sockaddr *)&addr, size);
		//if (ret != 0)
		//{
		//	bSucceeded = false;
		//	// return E_FAIL;
		//}
		//else
		//{
		//	ret = listen (sock, MAXUSERS);
		//	if (ret != 0)
		//	{
		//		bSucceeded = false;
		//	}
		//	else
		//	{
		//		m_sockPasv = accept (sock, (struct sockaddr *)addr2, &size);
		//		shutdown (sock, 2);
		//		closesocket( sock );
		//	}
		//}
		if ( bSucceeded )
		{
			hrReturn = S_OK;
		}

	}
	return hrReturn;
}

SOCKET CFTPThread::GetPasvSocket( void )
{
	SOCKET sockReturn = INVALID_SOCKET;
	struct sockaddr_in *addr2 = NULL;
	int size = sizeof (struct sockaddr_in);

	if ( m_sockPasv != INVALID_SOCKET )
	{
		sockReturn = m_sockPasv;
	}
	else
	{
		if ( m_sockListen == INVALID_SOCKET )
		{
			InitiatePasvListen();
		}
		if ( m_sockListen != INVALID_SOCKET )
		{
			m_sockPasv = accept(m_sockListen, (struct sockaddr *)addr2, &size );
			sockReturn = m_sockPasv;
			shutdown (m_sockListen, 2);
			closesocket( m_sockListen );
			m_sockListen = INVALID_SOCKET;
		}
	}
	return sockReturn;
}

bool CFTPThread::InitiatePasvListen( void )
{
	bool bReturn = false;

	// Restart the PASV listen() call to get our socket
	if ( m_sockListen != INVALID_SOCKET )
	{
		shutdown (m_sockListen, 2);
		closesocket( m_sockListen );
		m_sockListen = INVALID_SOCKET;
	}
	m_sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP );
	// SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP );

	struct sockaddr_in addr; // , *addr2 = NULL;
	int size = sizeof(struct sockaddr_in);

	ZeroMemory (&addr, size);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_wPassivePort);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret = bind (m_sockListen, (struct sockaddr *)&addr, size);

	if (ret == 0)
	{
		ret = listen (m_sockListen, MAXUSERS);
		if (ret == 0)
		{
			// m_sockPasv = accept(sock, (struct sockaddr *)addr2, &size);
			GetPasvSocket();
			//shutdown (m_sockListen, 2);
			//closesocket( m_sockListen );
			//m_sockListen = INVALID_SOCKET;
			bReturn = true;
		}
	}
	return bReturn;
}

void CFTPThread::SetIP( sockaddr_in * pAddr )
{
	if ( pAddr )
	{
		m_sIP.Format( _T("%d.%d.%d.%d"), pAddr->sin_addr.S_un.S_un_b.s_b1, pAddr->sin_addr.S_un.S_un_b.s_b2, pAddr->sin_addr.S_un.S_un_b.s_b3, pAddr->sin_addr.S_un.S_un_b.s_b4);
	}
}

void CFTPThread::ResumeThread( void )
{
	if ( m_hID != INVALID_HANDLE_VALUE )
	{
		::ResumeThread( m_hID );
	}
}

bool CFTPThread::SpawnThread( void )
{
	bool bReturn = false;
	HANDLE hThread;

	hThread = CreateThread( NULL, (1024*48), ThreadHandlerProc, this, CREATE_SUSPENDED, NULL );
	if ( hThread != INVALID_HANDLE_VALUE )
	{
		if ( g_MenuInfo.m_iFTPPriority < 5 )
		{
			g_MenuInfo.m_iFTPPriority = 5;
		}
		if ( g_MenuInfo.m_iFTPPriority > 11 )
		{
			g_MenuInfo.m_iFTPPriority = 11;
		}
		SetThreadPriority( hThread, g_MenuInfo.m_iFTPPriority );
		m_hID = hThread;
		bReturn = true;
	}
	return bReturn;
}


HRESULT CFTPThread::SendFile (LPCTSTR szFilename, unsigned int iStartAt)
{
	HRESULT hrReturn = E_FAIL;
	DWORD	dwBuffSize = FTP_WRITEBLOCKSIZE;
	char * buff = m_pbTransferBuffer;
	char smallbuff[4096];

	if ( buff == NULL )
	{
		buff = smallbuff;
		dwBuffSize = 4096;
	}

	if (!m_bPassive)
	{
		SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons (m_wRemotePort);
		addr.sin_addr.s_addr = inet_addr (m_sRemoteIP);
		if (connect (sock, (struct sockaddr *)&addr, sizeof (struct sockaddr)) == -1)
		{
			return E_FAIL;
		}
		HANDLE hFile;
		DWORD dwRead;

		hFile = ::CreateFile (szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		SetFilePointer (hFile, iStartAt, NULL, FILE_BEGIN);
		BOOL bReadSucc = ReadFile (hFile, buff, dwBuffSize, &dwRead, NULL);
		while (bReadSucc && (dwRead != 0))
		{
			if( SUCCEEDED( Send (sock, buff, dwRead) ) )
			{
				bReadSucc = ReadFile (hFile, buff, dwBuffSize, &dwRead, NULL);
			}
			else
			{
				bReadSucc = false;
			}
		}
		CloseHandle( hFile );
		shutdown (sock, 2);
		closesocket(sock);
		hrReturn = S_OK;
	}
	else
	{
		GetPasvSocket();
		if (m_sockPasv != INVALID_SOCKET)
		{
			HANDLE hFile;
			DWORD dwRead;
			// hFile = ::CreateFile (szFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			hFile = ::CreateFile (szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			SetFilePointer (hFile, iStartAt, NULL, FILE_BEGIN);
			BOOL bReadSucc = ReadFile (hFile, buff, dwBuffSize, &dwRead, NULL);
			while (bReadSucc && (dwRead != 0))
			{
				if( SUCCEEDED( Send (m_sockPasv, buff, dwRead) ) )
				{
					bReadSucc = ReadFile (hFile, buff, dwBuffSize, &dwRead, NULL);
				}
				else
				{
					bReadSucc = false;
				}
			}
			CloseHandle( hFile );
		}
		else
		{
		}
		shutdown (m_sockPasv, 2);
		closesocket(m_sockPasv);
		m_sockPasv = INVALID_SOCKET;
		m_bPassive = false;


		// InitiatePasvListen();

		//SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
		//struct sockaddr_in addr, *addr2 = NULL;
		//int size = sizeof (struct sockaddr_in);
		//ZeroMemory (&addr, size);
		//addr.sin_family = AF_INET;
		//addr.sin_port = htons (m_wPassivePort);
		//addr.sin_addr.s_addr = htonl (INADDR_ANY);
		//int ret = bind (sock, (struct sockaddr *)&addr, size);
		//if (ret != 0)
		//{
		//	return E_FAIL;
		//}
		//ret = listen (sock, MAXUSERS);
		//if (ret != 0)
		//{
		//	return E_FAIL;
		//}
		//m_sockPasv = accept (sock, (struct sockaddr *)addr2, &size);
		//shutdown (sock, 2);
		//closesocket(sock);
		hrReturn = S_OK;
	}
	return hrReturn;
}

HRESULT CFTPThread::SendData (BYTE *buff, int iLen)
{
	HRESULT hrReturn = S_OK;

	if (!m_bPassive)
	{
		SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in addr;

		addr.sin_family = AF_INET;
		addr.sin_port = htons (m_wRemotePort);
		addr.sin_addr.s_addr = inet_addr (m_sRemoteIP);
		if (connect (sock, (struct sockaddr *)&addr, sizeof (struct sockaddr)) == -1)
		{
			return E_FAIL;
		}
		Send (sock, (LPCSTR)buff, iLen);
		shutdown (sock, 2);
		closesocket(sock);
	}
	else
	{
		GetPasvSocket();
		if (m_sockPasv != INVALID_SOCKET)
		{
			Send (m_sockPasv, (LPSTR)buff, iLen);
		}
		shutdown (m_sockPasv, 2);
		closesocket(m_sockPasv);
		m_sockPasv = INVALID_SOCKET;
		m_bPassive = false;

		// InitiatePasvListen();

		//SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
		//struct sockaddr_in addr, *addr2 = NULL;
		//int size = sizeof (struct sockaddr_in);
		//ZeroMemory (&addr, size);
		//addr.sin_family = AF_INET;
		//addr.sin_port = htons (m_wPassivePort);
		//addr.sin_addr.s_addr = htonl (INADDR_ANY);
		//int ret = bind (sock, (struct sockaddr *)&addr, size);
		//if (ret != 0)
		//{
		//	return E_FAIL;
		//}
		//ret = listen (sock, MAXUSERS);
		//if (ret != 0)
		//{
		//	return E_FAIL;
		//}
		//m_sockPasv = accept (sock, (struct sockaddr *)addr2, &size);
		//shutdown (sock, 2);
		//closesocket(sock);

	}
	return hrReturn;
}

HRESULT CFTPThread::DoCommand( LPCTSTR szCmd, LPCTSTR szArg )
{
	HRESULT hrReturn = S_OK;

	DEBUG_FORMAT( _T("FTP: DoCommand:%s %s)"), szCmd, szArg );
	if ( szCmd && _tcslen( szCmd ) )
	{
		if ( _tcsicmp( szCmd, _T("port") ) == 0 )
		{
			doPORT(szArg);
		}
		else if ( ( _tcsicmp( szCmd, _T("pwd") ) == 0 ) || ( _tcsicmp( szCmd, _T("xpwd") ) == 0 ) )
		{
			doPWD(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("size") ) == 0 )
		{
			doSIZE(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("exec") ) == 0 )
		{
			doEXEC(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("noop") ) == 0 )
		{
			doNOOP(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("cdup") ) == 0 )
		{
			doCDUP(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("quit") ) == 0 )
		{
			doQUIT(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("pasv") ) == 0 )
		{
			doPASV(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("type") ) == 0 )
		{
			doTYPE(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("stru") ) == 0 )
		{
			doSTRU(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("retr") ) == 0 )
		{
			doRETR(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("stor") ) == 0 )
		{
			doSTOR(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("stou") ) == 0 )
		{
			doSTOU(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("appe") ) == 0 )
		{
			doAPPE(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("allo") ) == 0 )
		{
			doALLO(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("rest") ) == 0 )
		{
			doREST(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("rnfr") ) == 0 )
		{
			doRNFR(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("rnto") ) == 0 )
		{
			doRNTO(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("abor") ) == 0 )
		{
			doABOR(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("dele") ) == 0 )
		{
			doDELE(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("rmd") ) == 0 )
		{
			doRMD(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("mkd") ) == 0 )
		{
			doMKD(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("cwd") ) == 0 )
		{
			doCWD(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("list") ) == 0 )
		{
			doLIST(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("nlst") ) == 0 )
		{
			doNLST(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("site") ) == 0 )
		{
			doSITE(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("syst") ) == 0 )
		{
			doSYST(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("stat") ) == 0 )
		{
			doSTAT(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("help") ) == 0 )
		{
			doHELP(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("acct") ) == 0 )
		{
			doACCT(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("smnt") ) == 0 )
		{
			doSMNT(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("rein") ) == 0 )
		{
			doREIN(szArg);
		}
		else if ( _tcsicmp( szCmd, _T("mode") ) == 0 )
		{
			doMODE(szArg);
		}
		else
		{
			CStdString sTemp;
			sTemp.Format( _T("Command \"%s\" unknown."), szCmd );
			Msg( 500, sTemp );
#ifdef DEBUG
			sTemp.Format( _T("FTP COMMAND: \"%s\" (%s) unknown."), szCmd, szArg );
			OutputDebugString( sTemp.c_str() );
#endif
		}
	}
	return hrReturn;
}

void CFTPThread::doPORT(LPCTSTR szArg)
{
	unsigned int h1, h2, h3, h4, p1, p2;

	_stscanf( szArg, _T("%u,%u,%u,%u,%u,%u"), &h1, &h2, &h3, &h4, &p1, &p2);
	m_sRemoteIP.Format( _T("%u.%u.%u.%u"), h1, h2, h3, h4);
	m_wRemotePort = 256*p1 + p2;
	m_bPassive = false;

	Msg (200, _T("PORT command successful."));
}

void CFTPThread::doPWD(LPCTSTR szArg)
{
	CStdString sTemp;

	DEBUG_FORMAT( _T("PWD [0x%08x]: (%s)"), this, GetCurDir().c_str());

	if( GetCurDir().GetLength() )
	{
		// Remove double slashes, if any exist...
//		m_sCurDir.Replace( _T("\\\\"), _T("\\") );
//		m_sCurDir.Replace( _T("\\\\"), _T("\\") );
		DEBUG_FORMAT( _T("PWD: (%s)"), GetCurDir().c_str());
		sTemp.Format("\"%s\" is the cwd.", GetCurDir().c_str());
		sTemp.Replace( _T("\\"), _T("/") );
		Msg( 257, sTemp );
	}
	else
	{
		DEBUG_FORMAT( _T("PWD: (\\) [0 len]") );
		// Msg( 257, _T("\"\\\" is the cwd.") );
		Msg( 257, _T("\"/\" is the cwd.") );
	}
}

CStdString CFTPThread::TranslateFTPDirToXboxDir( LPCTSTR szDir )
{
	CStdString sReturn(szDir);

	if ( sReturn.GetLength() > 1 )
	{
		if ( sReturn[0] == _T('\\') )
		{
			if ( szDir[2] )
			{
				sReturn.Format( _T("%c:%s"), szDir[1], &szDir[2] );
			}
			else
			{
				sReturn.Format( _T("%c:"), szDir[1] );
			}
		}
	}
	else
	{
		sReturn = _T("");
	}
	return sReturn;
}

void CFTPThread::doSIZE(LPCTSTR szArg)
{
	CStdString sPath = TranslateFTPDirToXboxDir( GetCurFTPDir() );
	CStdString sArg(szArg);
	CStdString sCmd, sArgs;
	int iPos;

	sArg.Trim();
	sPath += _T("\\");
	sPath.Replace( _T("\\\\"), _T("\\") );
	sPath.Replace( _T("\\\\"), _T("\\") );
	iPos = sArg.Find( _T(' ') );
	if ( iPos > -1 )
	{
		sCmd = sArg.Left( iPos );
		sCmd.Trim();
		sArgs = sArg.Mid( iPos );
		sArgs.Trim();
	}
	else
	{
		sCmd = sArg;
	}
	Msg( 502, _T("size is not implemented.") );
//	m_pServer->FireExec( sCmd, sArgs );
//	Msg (200, _T("Ok."));
}

void CFTPThread::doEXEC(LPCTSTR szArg)
{
	CStdString sPath = TranslateFTPDirToXboxDir( GetCurFTPDir() );
	CStdString sArg(szArg);
	CStdString sCmd, sArgs;
	int iPos;

	sArg.Trim();
	sPath += _T("\\");
	sPath.Replace( _T("\\\\"), _T("\\") );
	sPath.Replace( _T("\\\\"), _T("\\") );
	iPos = sArg.Find( _T(' ') );
	if ( iPos > -1 )
	{
		sCmd = sArg.Left( iPos );
		sCmd.Trim();
		sArgs = sArg.Mid( iPos );
		sArgs.Trim();
	}
	else
	{
		sCmd = sArg;
	}
	m_pServer->FireExec( sCmd, sArgs );
	Msg (200, _T("Ok."));
}

void CFTPThread::doNOOP(LPCTSTR szArg)
{
	Msg (200, _T("NOOP okay."));
}

void CFTPThread::doCDUP(LPCTSTR szArg)
{
	CStdString sCurDir(GetCurDir());

	int iPos = sCurDir.ReverseFind( _T("\\") );

	DEBUG_FORMAT( _T("FTP: CDUP (%s)"), sCurDir.c_str() );

	// Is the slash on the end? If so, remove it
	if ( (sCurDir.GetLength()>1) && (iPos == (sCurDir.GetLength()-1) ))
	{
		sCurDir = sCurDir.Left(iPos);
		iPos = sCurDir.ReverseFind( _T("\\") );
	}

	if ( iPos >= 0 )
	{
		SetCurDir( sCurDir.Left( iPos ) );
	}
	else if ( iPos == 0 )
	{
		SetCurDir( _T("\\") );
	}
	DEBUG_FORMAT( _T("FTP: CDUP -> (%s)"), GetCurDir().c_str() );
	Msg (250, _T("CWD command successful."));
}

void CFTPThread::doQUIT(LPCTSTR szArg)
{
	m_bLoggedOn = false;
	Msg (221, _T("Goodbye."));
}

void CFTPThread::doPASV(LPCTSTR szArg)
{
	char buffer[128];

	// Let's get a random port above 2048
	unsigned int h1, h2, h3, h4, p1, p2;
	unsigned char byPortVals[2];

	XNetRandom( byPortVals, 2 );
	p1 = byPortVals[0];
	if ( p1 < 4 )
	{
		p1 += 4;
	}
	p2 = byPortVals[1];
//	p1 = rand() % 252 + 4; p2 = rand() % 256;
	m_wPassivePort = p1*256 + p2;

	// Now let's get our IP.
	if (m_bCfgIP)
	{
		h1 = m_h1; h2 = m_h2; h3 = m_h3; h4 = m_h4;
	}
	else
	{
		XNADDR xaddr;
		in_addr iaddr;
		XNKID x;
		XNetGetTitleXnAddr(&xaddr); 
		XNetXnAddrToInAddr (&xaddr, &x, &iaddr);

		h1 = iaddr.S_un.S_un_b.s_b1; h2 = iaddr.S_un.S_un_b.s_b2; h3 = iaddr.S_un.S_un_b.s_b3; h4 = iaddr.S_un.S_un_b.s_b4;

	}

	wsprintf (buffer, "Entering Passive Mode (%d,%d,%d,%d,%d,%d).",(int)h1, (int)h2, (int)h3, (int)h4, (int)p1, (int)p2);
	Msg (227, buffer);

	m_bPassive = true;

	InitiatePasvListen();


	//SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
	//struct sockaddr_in addr, *addr2 = NULL;
	//int size = sizeof (struct sockaddr_in);
	//ZeroMemory (&addr, size);
	//addr.sin_family = AF_INET;
	//addr.sin_port = htons (m_wPassivePort);
	//addr.sin_addr.s_addr = htonl (INADDR_ANY);
	//int ret = bind (sock, (struct sockaddr *)&addr, size);
	//ret = listen (sock, MAXUSERS);
	//m_sockPasv = accept (sock, (struct sockaddr *)addr2, &size);
	//shutdown (sock, 2);
	//closesocket(sock);

}

void CFTPThread::doTYPE(LPCTSTR szArg)
{
	CStdString sTemp;
	CStdString sArg(szArg);
	if ((_tcsicmp(szArg, _T("a")) == 0) || (_tcsicmp(szArg, "i") == 0))
	{
		sArg.MakeUpper();
		sTemp.Format( _T("Mode is set to %s."), sArg.c_str() );
		Msg( 200, sTemp );
	}
	else
	{
		Msg (501, _T("Bad parameter."));
	}
}

void CFTPThread::doSTRU(LPCTSTR szArg)
{
	if ( _tcsicmp (szArg, _T("F")) == 0 )
	{
		Msg (200, _T("Structure set to file."));
	}
	else
	{
		Msg (501, _T("File structures only."));
	}
}

void CFTPThread::doRETR(LPCTSTR szArg)
{
	CStdString sFilename;
	CStdString sArg(szArg);

	sArg.Replace( _T("/"), _T("\\") );
	if ( ( sArg.GetLength() >= 3 ) && sArg[0] == _T('\\') && sArg[2] == _T('\\') && _istalpha(sArg[1]) )
	{
		sArg = TranslateFTPDirToXboxDir( sArg );
	}

	sFilename = MakeFullFilePath(TranslateFTPDirToXboxDir( GetCurFTPDir() ), sArg );

//	sFilename.Format( _T("%s\\%s"), TranslateFTPDirToXboxDir( GetCurFTPDir() ).c_str(), sArg.c_str() );
	sFilename.Replace( _T("\\\\"), _T("\\") );
	sFilename.Replace( _T("\\\\"), _T("\\") );
	Msg (150, _T("Opening BINARY mode data connection for file transfer."));
	SendFile(sFilename, m_dwRest);
	Msg (226, _T("Transfer complete."));
	m_bRest = false; m_dwRest = 0;
}

void CFTPThread::doSTOR(LPCTSTR szArg)
{
	CStdString sFilename;
	CStdString sArg(szArg);


	// g_MenuInfo.m_bAutoRenameFTP

	sArg.Replace( _T("/"), _T("\\") );

	if ( ( sArg.GetLength() >= 3 ) && sArg[0] == _T('\\') && sArg[2] == _T('\\') && _istalpha(sArg[1]) )
	{
		sArg = TranslateFTPDirToXboxDir( sArg );
	}
	// sFilename.Format( _T("%s\\%s"), TranslateFTPDirToXboxDir( GetCurFTPDir() ).c_str(), szArg );
	sFilename = MakeFullFilePath(TranslateFTPDirToXboxDir( GetCurFTPDir() ), sArg );
	Msg (150, _T("Opening BINARY mode data connection for file transfer."));
	GetFile (sFilename, false);
}

void CFTPThread::doSTOU(LPCTSTR szArg)
{
	Msg (502, _T("Command not implemented."));
}

void CFTPThread::doAPPE(LPCTSTR szArg)
{
	CStdString sFilename;
	CStdString sArg(szArg);

	sArg.Replace( _T("/"), _T("\\") );
	if ( ( sArg.GetLength() >= 3 ) && sArg[0] == _T('\\') && sArg[2] == _T('\\') && _istalpha(sArg[1]) )
	{
		sArg = TranslateFTPDirToXboxDir( sArg );
	}

	sFilename = MakeFullFilePath(TranslateFTPDirToXboxDir( GetCurFTPDir() ), sArg );

//	sFilename.Format( _T("%s\\%s"), TranslateFTPDirToXboxDir( GetCurFTPDir() ).c_str(), sArg.c_str() );
	sFilename.Replace( _T("\\\\"), _T("\\") );
	sFilename.Replace( _T("\\\\"), _T("\\") );
	Msg (150, _T("Opening BINARY mode data connection for file transfer."));
	GetFile (sFilename, true);
}

void CFTPThread::doALLO(LPCTSTR szArg)
{
	doNOOP(szArg);
}

void CFTPThread::doREST(LPCTSTR szArg)
{
	m_bRest = true;
	m_dwRest = _tstol(szArg);
	Msg (350, _T("Restart okay."));
}

void CFTPThread::doRNFR(LPCTSTR szArg)
{
	CStdString sFilename;
	CStdString sArg(szArg);

	sArg.Replace( _T("/"), _T("\\") );
	if ( ( sArg.GetLength() >= 3 ) && sArg[0] == _T('\\') && sArg[2] == _T('\\') && _istalpha(sArg[1]) )
	{
		sArg = TranslateFTPDirToXboxDir( sArg );
	}

	sFilename = MakeFullFilePath(TranslateFTPDirToXboxDir( GetCurFTPDir() ), sArg );

//	sFilename.Format( _T("%s\\%s"), TranslateFTPDirToXboxDir( GetCurFTPDir() ).c_str(), sArg.c_str() );
	sFilename.Replace( _T("\\\\"), _T("\\") );
	sFilename.Replace( _T("\\\\"), _T("\\") );
	if ( FileExists( sFilename ) )
	{
		m_sRename = sFilename;
		Msg (350, _T("Next command must be RNTO."));
	}
	else
	{
		Msg (550, _T("File unavailable."));
	}
}

void CFTPThread::doRNTO(LPCTSTR szArg)
{
	CStdString sFilename;
	CStdString sArg(szArg);
	DWORD dwError;

	sArg.Replace( _T("/"), _T("\\") );
	if ( ( sArg.GetLength() >= 3 ) && sArg[0] == _T('\\') && sArg[2] == _T('\\') && _istalpha(sArg[1]) )
	{
		sArg = TranslateFTPDirToXboxDir( sArg );
	}

	sFilename = MakeFullFilePath(TranslateFTPDirToXboxDir( GetCurFTPDir() ), sArg );

//	sFilename.Format( _T("%s\\%s"), TranslateFTPDirToXboxDir( GetCurFTPDir() ).c_str(), sArg.c_str() );
	sFilename.Replace( _T("\\\\"), _T("\\") );
	sFilename.Replace( _T("\\\\"), _T("\\") );
	if (MoveFile (m_sRename, sFilename))
	{
		Msg (250, _T("Rename was successful."));
	}
	else
	{
		dwError = GetLastError();
		CStdString sMsg;

		sMsg.Format(_T("Rename was not successful (0x%08x)."), dwError );
		Msg (553, sMsg );
	}
}

void CFTPThread::doABOR(LPCTSTR szArg)
{
	Msg (502, _T("Command not implemented."));
}

void CFTPThread::doDELE(LPCTSTR szArg)
{
	CStdString sArg(szArg);

	sArg.Replace( _T("/"), _T("\\") );
	if ( ( sArg.GetLength() >= 3 ) && sArg[0] == _T('\\') && sArg[2] == _T('\\') && _istalpha(sArg[1]) )
	{
		sArg = TranslateFTPDirToXboxDir( sArg );
	}


	if (!m_bCanDelete)
	{
		Msg (550, _T("Insufficient access."));
	}
	else
	{
		CStdString sFilename;

		sFilename = MakeFullFilePath(TranslateFTPDirToXboxDir( GetCurFTPDir() ), sArg );

//		sFilename.Format( _T("%s\\%s"), TranslateFTPDirToXboxDir( GetCurFTPDir() ).c_str(), sArg.c_str() );
		sFilename.Replace( _T("\\\\"), _T("\\") );
		sFilename.Replace( _T("\\\\"), _T("\\") );
		if (DeleteFile (sFilename))
		{
			Msg (250, _T("File deleted."));
		}
		else
		{
			Msg (550, _T("Delete was not successful."));
		}
	}
}

void CFTPThread::doRMD(LPCTSTR szArg)
{
	CStdString sFilename;
	CStdString sArg(szArg);

	sArg.Replace( _T("/"), _T("\\") );
	if ( ( sArg.GetLength() >= 3 ) && sArg[0] == _T('\\') && sArg[2] == _T('\\') && _istalpha(sArg[1]) )
	{
		sArg = TranslateFTPDirToXboxDir( sArg );
	}



	DEBUG_FORMAT( _T("FTP: RMD %s"), szArg );


	if ( sArg[0] == _T('\\') )
	{
		sFilename = TranslateFTPDirToXboxDir(sArg);
	}
	else
	{
		sFilename = MakeFullFilePath(TranslateFTPDirToXboxDir( GetCurFTPDir() ), sArg );
	//	sFilename.Format( _T("%s\\%s"), TranslateFTPDirToXboxDir( GetCurFTPDir() ).c_str(), sArg.c_str() );
		sFilename.Replace( _T("\\\\"), _T("\\") );
		sFilename.Replace( _T("\\\\"), _T("\\") );
	}
	if ( ( sFilename.GetLength()>3 ) && RemoveDirectory (sFilename))
	{
		Msg (257, _T("Directory removed successfully."));
	}
	else
	{
		DWORD dwError = GetLastError();
		Msg (421, _T("Error removing directory."));
	}
}

void CFTPThread::doMKD(LPCTSTR szArg)
{
	CStdString sFilename;
	CStdString sArg(szArg);

	sArg.Replace( _T("/"), _T("\\") );
	if ( ( sArg.GetLength() >= 3 ) && sArg[0] == _T('\\') && sArg[2] == _T('\\') && _istalpha(sArg[1]) )
	{
		sArg = TranslateFTPDirToXboxDir( sArg );
	}

	DEBUG_FORMAT( _T("FTP: MKD %s"), szArg );

	if ( sArg[0] == _T('\\') )
	{
		if ( m_sRoot.Compare( _T("\\") )|| m_sRoot.GetLength() )
		{
			sFilename.Format( _T("%s%s"), m_sRoot.c_str(), sArg.c_str() );
			sFilename.Replace( _T("\\\\"), _T("\\") );
			sFilename.Replace( _T("\\\\"), _T("\\") );
		}
		else
		{
			sFilename = sArg.c_str();
		}
		sFilename = TranslateFTPDirToXboxDir(sFilename);

	}
	else
	{
		sFilename = MakeFullFilePath(TranslateFTPDirToXboxDir( GetCurFTPDir() ), sArg );
//		sFilename.Format( _T("%s\\%s"), TranslateFTPDirToXboxDir( GetCurFTPDir() ).c_str(), sArg.c_str() );
		sFilename.Replace( _T("\\\\"), _T("\\") );
		sFilename.Replace( _T("\\\\"), _T("\\") );
	}
	DWORD dwAttributes;
	bool bSuccess = false;


	dwAttributes = GetFileAttributes(sFilename);
	if ( dwAttributes == (-1) )
	{
		if ( CreateDirectory (sFilename, NULL) )
		{
			bSuccess = true;
		}
		else
		{
			bSuccess = false;
		}
	}
	else
	{
		if ( dwAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			bSuccess = true;
		}
		else
		{
			bSuccess = false;
		}
	}
	if ( bSuccess )
	{
		Msg (250, _T("Requested file action ok."));
	}
	else
	{
		Msg (421, _T("Error creating directory."));
	}
}

void CFTPThread::doCWD(LPCTSTR szArg)
{
	bool bDisplayMsg = true;
	CStdString sArg(szArg);
	CStdString sRealPath;
	CStdString sFTPPath;

	DEBUG_FORMAT( _T("FTP [0x%08x]: CWD %s  (PWD:%s)"), this, szArg, GetCurDir().c_str() );

	sArg.Replace( _T("/"), _T("\\") );

	while ( sArg.Replace( _T("\\\\"), _T("\\") ) ) ;

	if (sArg[0] == _T('\\'))
	{
		sFTPPath = sArg; // & [1]; // ?
	}
	else if ( sArg.Compare( _T(".") ) == 0 )
	{
	}
	else if ( sArg.Compare( _T("..") ) == 0 )
	{
		doCDUP(sArg);
		bDisplayMsg = false;
	}
	else
	{
		sFTPPath.Format( _T("%s\\%s\\"), GetCurDir().c_str(), sArg.c_str() );
	}

	// Remove double slashes, if any exist...
	sFTPPath.Replace( _T("\\\\"), _T("\\") );
	sFTPPath.Replace( _T("\\\\"), _T("\\") );
	sRealPath = TranslateFTPDirToXboxDir( sFTPPath );
	if ( sRealPath.GetLength() == 2 && sRealPath[1] == _T(':') )
	{
		sRealPath += _T('\\');
	}
	if ( FileExists( sRealPath ) )
	{
		SetCurDir( sFTPPath );
		// m_sCurDir = sFTPPath;
		if ( bDisplayMsg )
		{
			Msg (250, _T("CWD command successful."));
		}
	}
	else
	{
		if ( bDisplayMsg )
		{
			Msg (553, _T("Requested action not taken. Directory not found"));
		}
	}
//	DEBUG_FORMAT( _T("FTP: CWD -> (PWD:%s)"), GetCurDir().c_str() );
}

void CFTPThread::doLIST(LPCTSTR szArg)
{
	Msg (150, _T("Opening ASCII mode data connection for ls."));

	CStdString sDirectoryList = GetDirectory(false);
	SendData ((BYTE*)sDirectoryList.c_str(), sDirectoryList.GetLength());
	// Msg (226, _T("Listing completed."));
	Msg (226, _T("Transfer finished successfully. Data Connection Closed."));
}

void CFTPThread::doNLST(LPCTSTR szArg)
{
	Msg (150, _T("Opening ASCII mode data connection for list."));

	CStdString sDirectoryList = GetDirectory(true);
	SendData ((BYTE*)sDirectoryList.c_str(), sDirectoryList.GetLength());
	Msg (226, _T("Listing completed."));
}

void CFTPThread::doSITE(LPCTSTR szArg)
{
	CStdString sResponse;
	if ( SUCCEEDED( ProcessScriptCommand( szArg, sResponse ) ) )
	{
		if ( sResponse.GetLength() )
		{
			// Display as information here...
			InfoMsg( sResponse );
		}
		Msg (200, _T("Command succeeded."));
	}
	else
	{
		Msg (502, _T("Command not implemented."));
	}
}

void CFTPThread::doSYST(LPCTSTR szArg)
{
	Msg (215, _T("UNIX Type: L8"));
//	Msg (215, _T("UNIX (sorta) done by J.P. Patches"));
}

void CFTPThread::doSTAT(LPCTSTR szArg)
{
	Msg (502, _T("Command not implemented."));
}

void CFTPThread::doHELP(LPCTSTR szArg)
{
	Msg (502, _T("Command not implemented."));
}

void CFTPThread::doACCT(LPCTSTR szArg)
{
	Msg (552, _T("Account information discarded."));
}

void CFTPThread::doSMNT(LPCTSTR szArg)
{
	Msg (502, _T("Command not implemented."));
}

void CFTPThread::doREIN(LPCTSTR szArg)
{
	Msg (220, _T("Service ready for new user."));
	if (DoLogin () != S_OK)
	{
		m_bLoggedOn = false;
	}
}

void CFTPThread::doMODE(LPCTSTR szArg)
{
	Msg (502, _T("Command not implemented."));
}

LPCTSTR szMonths[] = { 
	_T("Ben"), _T("Jan"), _T("Feb"), _T("Mar"), 
	_T("Apr"), _T("May"), _T("Jun"), _T("Jul"), 
	_T("Aug"), _T("Sep"), _T("Oct"), _T("Nov"), 
	_T("Dec") };


CStdString CFTPThread::GetDirectory (bool bShort)
{
	CStdString sReturn;
	//char *result = NULL;
	//const char dir[] = "drwx------ ";
	//const char file[] = "-rwx------ ";
	//const char group[] = "1 user group ";

	CStdString sPath = TranslateFTPDirToXboxDir( GetCurFTPDir() );
	CStdString sWildcard;

	// Let's get us some files.
	WIN32_FIND_DATA wfd;
	//char *path = GetCurrentDir ();
	//TranslateDir (path);
	if ( sPath.GetLength() == 0 )
	{
		sReturn = _T("drw-r--r--   1 XBOX      XBOX        0 Oct 26 10:03 C\r\ndrw-r--r--   1 XBOX      XBOX        0 Oct 26 10:03 D\r\ndrw-r--r--   1 XBOX      XBOX        0 Oct 26 10:03 E\r\ndrw-r--r--   1 XBOX      XBOX        0 Oct 26 10:03 F\r\ndrw-r--r--   1 XBOX      XBOX        0 Oct 26 10:03 X\r\ndrw-r--r--   1 XBOX      XBOX        0 Oct 26 10:03 Y\r\ndrw-r--r--   1 XBOX      XBOX        0 Oct 26 10:03 Z\r\n");
		// sReturn = _T("drwx------ 1 user group              0 Oct 26 10:03 c\r\ndrwx------ 1 user group              0 Oct 26 10:03 d\r\ndrwx------ 1 user group              0 Oct 26 10:03 e\r\ndrwx------ 1 user group              0 Oct 26 10:03 f\r\n");
		// Send over canned data.
		//char foo[] = "drwx------ 1 user group              0 Oct 26 10:03 c\r\ndrwx------ 1 user group              0 Oct 26 10:03 d\r\ndrwx------ 1 user group              0 Oct 26 10:03 e\r\ndrwx------ 1 user group              0 Oct 26 10:03 f\r\n";
		//result = (char *)HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, strlen (foo) + 1);
		//strcpy (result, foo);
	}
	else
	{
		CStdString sDirLine;
		TCHAR szTimestamp[32];
		ZeroMemory( szTimestamp, sizeof(TCHAR)*32 );
		sWildcard = MakeFullFilePath( sPath, _T("*") );
		// sWildcard += _T("\\*");
		// strcat (path, "\\*");
		HANDLE hFile = FindFirstFile (sWildcard, &wfd);
		sReturn = _T("drw-r--r--   1 XBOX      XBOX        0 Oct 26 10:03 ..\r\n");

		bool bDone = (hFile == INVALID_HANDLE_VALUE);

		while (!bDone)
		{
			sDirLine = _T("");

			//char *thisfile = (char *)HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, 1024);
			// *thisfile = 0;  // start with a empty string.
			// We've got a file.
			if (!bShort)
			{
				SYSTEMTIME st;
				FILETIME ft;
				FileTimeToLocalFileTime (&wfd.ftCreationTime, &ft);
				FileTimeToSystemTime (&ft, &st);
				_stprintf( szTimestamp, _T("%s %02d %02d:%02d"), szMonths[st.wMonth], st.wDay, st.wHour, st.wMinute);
				// _stprintf( szTimestamp, _T("%s %2d %02d:%02d"), szMonths[st.wMonth], st.wDay, st.wHour, st.wMinute);

				sDirLine.Format( _T("%crw-r--r--   1 XBOX      XBOX        %d %s %s\r\n"), (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)?_T('d'):_T('-'),
					wfd.nFileSizeLow,
					szTimestamp,
					wfd.cFileName );
				//sDirLine.Format( _T("%crwx------ 1 user group %14d %s %s\r\n"), (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)?_T('d'):_T('-'),
				//	wfd.nFileSizeLow,
				//	szTimestamp,
				//	wfd.cFileName );

				sReturn += sDirLine;
				

			}
			else
			{
				if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					sDirLine.Format( _T("%s\r\n"), wfd.cFileName );
					sReturn += sDirLine;
					//strcat (thisfile, wfd.cFileName);
					//strcat (thisfile, "\r\n");
				}
			}
			bDone = (FindNextFile (hFile, &wfd) == FALSE);
		}
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			CloseHandle( hFile );
		}
	}
	return sReturn;
}

HRESULT CFTPThread::Send( SOCKET sock, LPCSTR szBuff, int iLen )
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

HRESULT CFTPThread::Recv( SOCKET sock, LPSTR szBuff, int iLen )
{
	HRESULT hrReturn = E_FAIL;
	int iBytes;
	char * szData = (char *)szBuff;

// No loop needed here...
//	while( true )
//	{
		iBytes = recv( sock, szData, iLen, 0 );
		if ( iBytes > 0 )
		{
			szData[iLen] = 0;
			hrReturn = S_OK;
		}
		else if ( iBytes == 0 )
		{
			m_bLoggedOn = false;
		}
//	}
	return hrReturn;
}

HRESULT CFTPThread::GetCommand( CStdString & sCmd, CStdString & sArg )
{
	HRESULT hrReturn = E_FAIL;
	char szLineIn[256];
	CStdString sLine;
	int iPos = -1;

	ZeroMemory( szLineIn, 256 );
	if ( RecvLine( sLine ) )
//	if ( Recv( m_sockCmd, szLineIn, 255 ) == S_OK )
	{
//		sLine = szLineIn;
//		sLine.Trim();
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
// For now, allow blank lines
//		if ( sCmd.GetLength() )
//		{
			hrReturn = S_OK;
//		}
	}
	return hrReturn;
}

HRESULT CFTPThread::DoLogin( void )
{
	HRESULT hrReturn = E_FAIL;
	bool bHasName = false;
	bool bHasPass = false;
	int	 iTries = 0;
	CStdString sCmd, sArg;

	Msg( 220, _T("MXM FTP ready.") );
	while( (iTries < 3) && (!bHasPass) )
	{
		if ( SUCCEEDED(GetCommand( sCmd, sArg )) )
		{
			if ( bHasName )
			{
				if ( _tcsicmp( sCmd, _T("pass") ) )
				{
					Msg( 503, _T("Bad sequence of commands - need PASS") );
				}
				else
				{
					// Got a password
					sArg.Trim();
					if ( sArg.GetLength() )
					{
						if ( SUCCEEDED( m_pServer->ValidateLogon( m_sName, sArg, m_sRoot )) )
						{
							bHasPass = true;
							m_sPassword = sArg;
							hrReturn = S_OK;
						}
						else
						{
							iTries++;
							Msg( 421, _T("Unknown user or bad password.") );
						}
					}
				}
			}
			else
			{
				if ( _tcsicmp( sCmd, _T("user") ) )
				{
					Msg( 503, _T("Bad sequence of commands - need USER") );
				}
				else
				{
					// Got a name?
					sArg.Trim();
					if ( sArg.GetLength() )
					{
						if ( (_tcsicmp( sArg, _T("anonymous") ) == 0 ) && m_pServer->AllowAnon() )
						{
							Msg( 331, _T("Guest login ok, send email as password.") );
						}
						else
						{
							Msg( 331, _T("Send password.") );
						}
						m_sName = sArg;
						bHasName = true;
					}
				}
			}
		}
		else
		{
			Msg( 503, _T("Command Parse Error") );
			break;
		}
	}
	return hrReturn;
}

HRESULT CFTPThread::DoWelcome( void )
{
	HRESULT hrReturn = S_OK;

	Msg( 230, _T("Welcome to MXM's FTP server") );
	return hrReturn;
}



HRESULT CFTPThread::DoLogout( void )
{
	HRESULT hrReturn = S_OK;

	shutdown( m_sockCmd, 2 );
	closesocket(m_sockCmd);
	m_pServer->DeleteThreadByHandle(m_hID);
	return hrReturn;
}


void CFTPThread::InfoMsg( LPCTSTR szString )
{
	CStdString sString(szString);
	CStdString sTemp;
	CStdString sMsg;
	bool bFirstLine = true;

	sString.Trim();

	if ( szString && sString.GetLength() )
	{
		sString.Replace( _T("\r\n"), _T("\x01") );
		sString.Replace( _T("\n\r"), _T("\x01") );
		sString.Replace( _T("\n"), _T("\x01") );
		sString.Replace( _T("\x01"), _T("\r") );
		// Now we can parse out string...
		int iPos;
		iPos = sString.Find( _T("\r") );
		while( iPos >= 0 )
		{
			sTemp = sString.Left( iPos );
			sString = sString.Mid( iPos+1);
			iPos = sString.Find( _T("\r") );
			if ( bFirstLine )
			{
				sMsg.Format( _T("230- %s\r\n"), sTemp.c_str() );
				// sMsg.Format( _T("250- %s\r\n"), sTemp );
				bFirstLine = false;
			}
			else
			{
				if ( iPos == -1 && sString.GetLength() == 0 )
				{
					// sMsg.Format( _T("230 %s\r\n"), sTemp );
					sMsg.Format( _T("230- %s\r\n"), sTemp.c_str() );
				}
				else
				{
					if ( _istdigit(sTemp[0]) )
					{
						sTemp = _T(" ")+sTemp;
					}
					sMsg.Format( _T("230- %s\r\n"), sTemp.c_str() );
					// sMsg.Format( _T("%s\r\n"), sTemp );
				}
			}
			Send( m_sockCmd, sMsg, sMsg.GetLength() );
		}
		if ( sString.GetLength() )
		{
			sMsg.Format( _T("230- %s\r\n"), sString.c_str() );
			Send( m_sockCmd, sMsg, sMsg.GetLength() );
			// Msg( 230, sString );
		}
	}
}

void CFTPThread::Msg( int iCode, LPCTSTR szLine )
{
	CStdString sMsg;

	if ( szLine && _tcslen( szLine ) )
	{
		sMsg.Format( _T("%d %s\r\n" ), iCode, szLine );
	}
	else
	{
		sMsg.Format( _T("%d\r\n" ), iCode );
	}
	Send( m_sockCmd, sMsg, sMsg.GetLength() );
}

int		CFTPThread::AddRef( void )
{
	m_iRefCount++;
	return m_iRefCount;
}

int		CFTPThread::Release( void )
{
	int iRefCount;
	m_iRefCount--;
	iRefCount = m_iRefCount; 
	if ( m_iRefCount < 1 )
	{
		iRefCount = 0;
		delete this;
	}
	return iRefCount;
}

bool	CFTPThread::GetUserInfo( CStdString & sName, CStdString & sPassword, CStdString & sCurDir, CStdString & sIP )
{
	bool bReturn = false;


	return bReturn;
}

DWORD CFTPThread::ThreadHandlerProc( LPVOID pParam )
{
	CFTPThread * pThis = (CFTPThread *)pParam;

	return pThis->Serve();
}

