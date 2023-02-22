
#include "StdAfx.h"

#include "xbftpsession.h"
#include "xbdirectorylister.h"
#include "xmltree.h"
#include "XBFTPServer.h"
#include "XBNet.h"

#define FTP_WRITEBLOCKSIZE 32768
// 32768

// #define HARDTEST 1

LPCTSTR szMonths[] = { 
	_T("Ben"), _T("Jan"), _T("Feb"), _T("Mar"), 
	_T("Apr"), _T("May"), _T("Jun"), _T("Jul"), 
	_T("Aug"), _T("Sep"), _T("Oct"), _T("Nov"), 
	_T("Dec") };


#define __XFS_THREADED__	0

/*


FTP Sessions:

This handles all of the incoming commands specific to the FTP Server, and also performs
transfers to/from the Server to the client.

We need to have a few things from the Server's parent application:

Top Level Directory is provided in a special manner.
We have two differnet cases here, PC and Xbox that we want deal with. 

For the Xbox, we need a C:, D:, E:, and F:, plus the X:, Y: and Z: temporary partitions
We will also want to dynamically handle dynamic and relative name translation


CStdString TranslateFilenameToFileSys( LPCTSTR szCurrPath, LPCTSTR szFilename );
CStdString TranslateFilenameToFTP( LPCTSTR szCurrPath, LPCTSTR szFilename );

class CXBDirectoryLister
{
	CXBDirectoryLister( LPCTSTR szCurrentDirectory );
	void Reset();
	bool GetNext( CStdString & sEntryName, DWORD & dwAttributes, ULONGULONG & u64Size );
	CStdString TranslateToFileSys( LPCTSTR szFilename );
}


When downloading or uploading files, we have to be concerned as to whether we are in PASV mode ot PORT mode.
PORT mode means that we are initiating the connection on our end, and the Client will accept the connection
on the port given to us. PASV mode means that we are listening for connections on a port WE give to them.

When we are in PASV mode, we will tell the transfer thread that we need to listen to an incoming thread.
How do we signal this? Use and event that the transfer thread will wait on.
m_hTransferAction will be the thread event signal, and the thread will then look at what action needs 
to be taken:
 XT_SHUTDOWN
 XT_PASV
 XT_PASV_RCV
 XT_PASV_XMT
 XT_RCV
 XT_XMT

 Before each signalling event, the proper data will be filled out.

 Transfer_SetPASV()
 Transfer_SetRcv( LPCTSTR szFilename )
 Transfer_SetXmt( LPCTSTR szFilename )

 PORT:
    Transfer_CancelPASV();
 	m_sRemoteIP
	m_wRemotePort
	m_bPassive = false;
 PASV
	m_bPassive = true;


Transfer_CancelPASV()
{
	if ( m_bPassive )
	{
		Cancel listen on PASV port
	}
}

TransferThread()
{
	bool bNotShutdown = true;
	while( bNotShutdown )
	{
	}
}




*/

CXBFTPSession::CXBFTPSession(SOCKET sock, CXBFTPServer * pServer) :
	CXBNetSession( sock, (CXBNetServer *)pServer )
	,m_pFTPServer(pServer)
	,m_iFTPCmdState(FTPSESS_LOGINUSER)
	,m_eventFTPHandler(NULL)
	,m_bCfgIP(false)
	,m_iLoginRetries(0)
	,m_bRest(false)
	,m_bPassive(false)
	,m_pbTransferBuffer(NULL)
	,m_wRemotePort(999)
	,m_wPassivePort(999)
	,m_h1(0)
	,m_h2(0)
	,m_h3(0)
	,m_h4(0)
	,m_iXferThreadState(XTS_IDLE)
	,m_hXferThreadEvent(NULL)
	,m_hXferThreadDone(NULL)
	,m_bXferFile(false)
	,m_hXferFile(NULL)
	,m_pbXferBuffer(NULL)
	,m_dwXferBufferAllocSize(0)
	,m_dwXferBufferSize(0)
	,m_sockXfer(INVALID_SOCKET)
	,m_bCanDelete(false)
	,m_sockPasv(INVALID_SOCKET)
	,m_sockListen(INVALID_SOCKET)
	,m_dwRest(0)
	,m_iLoginTries(0)
	,m_sCurDir(_T("/"))
{
#if __XFS_THREADED__	
	m_hXferThreadEvent = CreateEvent( NULL, false, false, NULL );
#endif	
	m_hXferThreadDone = CreateEvent( NULL, false, false, NULL );
	m_pbTransferBuffer = new char [FTP_WRITEBLOCKSIZE];
	
	XferThreadStartup();
}

CXBFTPSession::~CXBFTPSession(void)
{
	// Kill any current transfer processes!
	AbortTransfer();

	// Now kill the thread entirely!
	XferThreadShutdown();

	if ( m_pbTransferBuffer )
	{
		delete [] m_pbTransferBuffer;
	}
	if ( m_hXferThreadEvent )
	{
		CloseHandle( m_hXferThreadEvent );
	}
	if ( m_hXferThreadDone )
	{
		CloseHandle( m_hXferThreadDone );
	}
	if ( m_hXferFile )
	{
		CloseHandle( m_hXferFile );
	}
	if ( m_pbXferBuffer )
	{
		delete [] m_pbXferBuffer;
	}
}

bool CXBFTPSession::AllocateXferBuffer( DWORD dwSize )
{
	bool bReturn = false;

	if ( dwSize )
	{
		if ( m_pbXferBuffer )
		{
			if ( m_dwXferBufferAllocSize < (dwSize+1) )
			{
				delete [] m_pbXferBuffer;
				m_pbXferBuffer = NULL;
				m_dwXferBufferSize = 0;
				m_dwXferBufferAllocSize = 0;
			}
			else
			{
				m_dwXferBufferSize = dwSize;
				ZeroMemory( m_pbXferBuffer, m_dwXferBufferAllocSize );
				bReturn = true;
			}
		}
		if ( !m_pbXferBuffer )
		{
			m_pbXferBuffer = new BYTE[dwSize+1];
			if ( m_pbXferBuffer )
			{
				m_dwXferBufferSize = dwSize;
				m_dwXferBufferAllocSize = dwSize;
				ZeroMemory( m_pbXferBuffer, dwSize+1 );
				bReturn = true;
			}
		}
	}
	else
	{
		if ( m_pbXferBuffer )
		{
			delete [] m_pbXferBuffer;
			m_pbXferBuffer = NULL;
		}
		m_dwXferBufferSize = 0;
		m_dwXferBufferAllocSize = 0;
		bReturn = true;
	}
	return bReturn;
}

void CXBFTPSession::XferThreadStartup( void )
{
	BOOL bSet = FALSE;

#if __XFS_THREADED__
	m_iXferThreadState = XTS_IDLE;
	m_hTransferThread = CreateThread( NULL, 0, XferThreadHandlerProc, this, 0, NULL );

	if ( m_hTransferThread && m_pFTPServer )
	{
		bSet = SetThreadPriority( m_hTransferThread, m_pFTPServer->GetXferPriority() );
	}
#else
	bSet = SetThreadPriority( m_hID, m_pFTPServer->GetXferPriority() );
#endif

}

void CXBFTPSession::XferThreadShutdown( void )
{
	if ( m_hTransferThread )
	{
		m_iXferThreadState = XTS_SHUTDOWN;
		SetEvent( m_hXferThreadEvent );
		if ( WaitForSingleObject( m_hXferThreadDone, 1000 ) != WAIT_OBJECT_0 )
		{
#ifdef _XBOX
			// Xbox cannot TerminateThread!!
			m_iXferThreadState = XTS_SHUTDOWN;
			SetEvent( m_hXferThreadEvent );
			WaitForSingleObject( m_hXferThreadDone, 2000 );
#else
			TerminateThread( m_hTransferThread, 0 );
#endif
		}
		m_hTransferThread = NULL;
	}
}


bool	CXBFTPSession::HandlePASV( void )
{
	bool bReturn = false;
	
	if ( m_sockListen != INVALID_SOCKET )
	{
		struct sockaddr_in *addr2 = NULL;
		int size = sizeof (struct sockaddr_in);
		m_sockPasv = _SockAccept(m_sockListen, (struct sockaddr *)addr2, &size );
		if ( m_sockPasv != INVALID_SOCKET )
		{
			bReturn = true;
		}
	}
	return bReturn;
}

bool	CXBFTPSession::HandleXmt( void )
{
	bool bReturn = false;
	
	SOCKET sock = ConnectToXferPort();
	if ( sock != INVALID_SOCKET )
	{
		m_iFTPCmdState = FTPSESS_TRANSFER;
		if ( m_bXferFile )
		{
			// File handle should already be open at this point...
			if ( m_hXferFile )
			{
				if ( !m_bRest )
				{
					m_dwRest = 0;
				}
				if ( SUCCEEDED( SendDataFile( sock, m_hXferFile, m_dwRest ) ) )
				{
					Msg (226, _T("Transfer finished successfully. Data Connection Closed."));
				}
				else
				{
					Msg (450, "Unknown Problem; transfer aborted.");
				}
				// One way or another, the SendDataFile closes m_hXferFile, so NULL it
				m_hXferFile = NULL;
			}
		}
		else
		{
			if ( SUCCEEDED( SendDataBuffer( sock, m_pbXferBuffer, m_dwXferBufferSize ) ) )
			{
				Msg (226, _T("Transfer finished successfully. Data Connection Closed."));
			}
			else
			{
				Msg (450, "Unknown Problem; transfer aborted.");
			}
		}
		m_iFTPCmdState = FTPSESS_COMMAND;
	}
	return bReturn;
}

bool	CXBFTPSession::HandleRcv( void )
{
	bool bReturn = false;

	SOCKET sock = ConnectToXferPort();
	
	if ( sock != INVALID_SOCKET )
	{
		m_iFTPCmdState = FTPSESS_TRANSFER;
		if ( m_bXferFile )
		{
			// File handle should already be open at this point...
			if ( m_hXferFile )
			{
				if ( !m_bRest )
				{
					m_dwRest = 0;
				}
				if ( SUCCEEDED( GetDataFile( sock, m_hXferFile, m_bAppend?(-1):m_dwRest ) ) )
				{
					Msg (226, _T("Transfer finished successfully. Data Connection Closed."));
				}
				else
				{
					Msg (450, "Unknown Problem; transfer aborted.");
				}
				// One way or another, the SendDataFile closes m_hXferFile, so NULL it
				m_hXferFile = NULL;
			}
		}
		else
		{
			Msg (450, "Unknown Problem; transfer aborted.");
		}
		m_iFTPCmdState = FTPSESS_COMMAND;
	}
	
	return bReturn;
}


// Handles the actual thread, relative to the object as a member
DWORD CXBFTPSession::XferThreadHandler(void)
{
	DWORD dwReturn = 0;
	DWORD dwSignal;
	//SOCKET sockNew;
	//struct sockaddr_in * addr = NULL;
	//int iSize = sizeof(struct sockaddr_in);
	bool bNotShutdown = true;
	while( bNotShutdown )
	{
		if( (dwSignal = WaitForSingleObject( m_hXferThreadEvent, 100 )) == WAIT_OBJECT_0 )
		{
			// Sleep( 50 ); // Try and calm things down
			switch( m_iXferThreadState )
			{
				case XTS_SHUTDOWN:
					break;
				case XTS_PASV:
					// Wait on assigned PASV socket, unless otherwise a problem
					HandlePASV();
					break;
				case XTS_RCV:
					{
						HandleRcv();
#if 0					
					//	TRACE ( _T("Beginning RCV operation\r\n") );

						SOCKET sock = ConnectToXferPort();
						if ( sock != INVALID_SOCKET )
						{
							m_iFTPCmdState = FTPSESS_TRANSFER;
							if ( m_bXferFile )
							{
								// File handle should already be open at this point...
								if ( m_hXferFile )
								{
									if ( !m_bRest )
									{
										m_dwRest = 0;
									}
									if ( SUCCEEDED( GetDataFile( sock, m_hXferFile, m_bAppend?(-1):m_dwRest ) ) )
									{
										Msg (226, _T("Transfer finished successfully. Data Connection Closed."));
									}
									else
									{
										Msg (450, "Unknown Problem; transfer aborted.");
									}
									// One way or another, the SendDataFile closes m_hXferFile, so NULL it
									m_hXferFile = NULL;
								}
							}
							else
							{
								Msg (450, "Unknown Problem; transfer aborted.");
							}
							m_iFTPCmdState = FTPSESS_COMMAND;
						}
#endif						
					}
					break;
				case XTS_XMT:
					{
						// TRACE ( _T("Beginning XMT operation\r\n") );
						HandleXmt();
#if 0
						SOCKET sock = ConnectToXferPort();
						if ( sock != INVALID_SOCKET )
						{
							m_iFTPCmdState = FTPSESS_TRANSFER;
							if ( m_bXferFile )
							{
								// File handle should already be open at this point...
								if ( m_hXferFile )
								{
									if ( !m_bRest )
									{
										m_dwRest = 0;
									}
									if ( SUCCEEDED( SendDataFile( sock, m_hXferFile, m_dwRest ) ) )
									{
										Msg (226, _T("Transfer finished successfully. Data Connection Closed."));
									}
									else
									{
										Msg (450, "Unknown Problem; transfer aborted.");
									}
									// One way or another, the SendDataFile closes m_hXferFile, so NULL it
									m_hXferFile = NULL;
								}
							}
							else
							{
								if ( SUCCEEDED( SendDataBuffer( sock, m_pbXferBuffer, m_dwXferBufferSize ) ) )
								{
									Msg (226, _T("Transfer finished successfully. Data Connection Closed."));
								}
								else
								{
									Msg (450, "Unknown Problem; transfer aborted.");
								}
							}
							m_iFTPCmdState = FTPSESS_COMMAND;
						}
#endif						
					}
					break;
				case XTS_ABORT:
				default:
					// Reset state from unknown or abort (we must have finished abort, if we are here!!)
					m_iXferThreadState = XTS_IDLE;
				case XTS_IDLE:
					Sleep(5);
					break;
			}
		}
		else if ( dwSignal == WAIT_TIMEOUT )
		{
			Sleep(1);
		}
		else // Must have been abandoned, or worse... shutdown!
		{
			bNotShutdown = false;
		}
		if ( m_iXferThreadState == XTS_SHUTDOWN )
		{
			// No matter what... shut down!
			bNotShutdown = false;
		}
	}
	if ( m_sockListen != INVALID_SOCKET )
	{
		shutdown (m_sockListen, 2);
		_SockClose( m_sockListen );
		m_sockListen = INVALID_SOCKET;
	}
	if ( m_sockPasv != INVALID_SOCKET )
	{
		shutdown (m_sockPasv, 2);
		_SockClose( m_sockPasv );
		m_sockPasv = INVALID_SOCKET;
	}
	SetEvent( m_hXferThreadDone );
	return dwReturn;
}



void CXBFTPSession::BeginLogin( bool bInitial )
{
	// Reset login retries
	m_iLoginRetries = 0;
	m_iFTPCmdState = FTPSESS_LOGINUSER;
	if ( bInitial )
	{
		// EVOX: 220 Please enter your login name now.
		Msg( 220, _T("MXM FTP ready.") );
	}
	else
	{
		Msg(220, _T("Service ready for new user."));
	}
}


void CXBFTPSession::DoGreeting( void )
{
	BeginLogin(true);
}

void CXBFTPSession::AbortTransfer( void )
{
	int iNewTimeout = 0;
	if ( m_iXferThreadState == XTS_RCV )
	{
		m_iXferThreadState = XTS_ABORT;
		// End the GetDataFile routine by killing the timeout on the socket!
		if ( m_sockXfer != INVALID_SOCKET )
		{
			// Kill the Recv timeout on this socket...
			setsockopt( m_sockXfer, SOL_SOCKET, SO_RCVTIMEO, (const char *)(&iNewTimeout), sizeof(int) );
		}
	}
	else if ( m_iXferThreadState == XTS_XMT )
	{
		m_iXferThreadState = XTS_ABORT;
		// End the SendDataFile routine by killing the timeout on the socket!
		if ( m_sockXfer != INVALID_SOCKET )
		{
			BOOL bNoLinger = TRUE;
			// Kill the Recv timeout on this socket...
			setsockopt( m_sockXfer, SOL_SOCKET, SO_DONTLINGER, (const char *)(&bNoLinger), sizeof(BOOL) );
			setsockopt( m_sockXfer, SOL_SOCKET, SO_SNDTIMEO, (const char *)(&iNewTimeout), sizeof(int) );
		}
	}
}

HRESULT	CXBFTPSession::DoCommand( LPCTSTR szCmd, LPCTSTR szArg )
{
	HRESULT hr = E_FAIL;
	CStdString sCmd(szCmd), sArg(szArg);
	CStdString sTemp;
	TCHAR tcFirst;
	int iLength;

	if ( sCmd.GetLength() )
	{
		tcFirst = sCmd.GetAt(0);
	}
	iLength = sCmd.GetLength();
	// First, trim any non-alphanumerics from the front
	while( iLength && !_istalnum(tcFirst&0x0ff) )
	{
		sTemp = sCmd.Mid(1);
		sCmd = sTemp;
		iLength = sCmd.GetLength();
		if ( iLength )
		{
			tcFirst = sCmd.GetAt(0);
		}
	}
	switch(m_iFTPCmdState)
	{
		case FTPSESS_LOGINUSER:
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
					if ( (_tcsicmp( sArg, _T("anonymous") ) == 0 ) ) // && m_pServer->AllowAnon() )
					{
						Msg( 331, _T("Guest login ok, send email as password.") );
					}
					else
					{
						// EVOX: 331 Password required for xbox.
						Msg( 331, _T("Send password.") );
					}

					m_sLoginName = sArg;

					m_iFTPCmdState = FTPSESS_LOGINPASS;
				}
			}
			break;
		case FTPSESS_LOGINPASS:
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
					DWORD dwResult = FireValidateUser( m_sLoginName, sArg );
					// EVOX: 230 User xbox logged in , proceed.
					if ( dwResult & FTPUSERPERM_LOGINOK )
					{
						if ( dwResult & FTPUSERPERM_CANDELETE )
						{
							m_bCanDelete = true;
						}
						Msg( 230, _T("Welcome to MXM's FTP server") );
						m_iFTPCmdState = FTPSESS_COMMAND;
					}
					else
					{
						m_iFTPCmdState = FTPSESS_LOGINUSER;
						m_iLoginTries++;
						Msg( 421, _T("Unknown user or bad password.") );
						if ( m_iLoginTries < 3 )
						{
							BeginLogin(true);
						}
						else
						{
							Msg( 421, _T("Too many login attempts.") );
							SayGoodbye();
						}
					}
				}
			}
			break;
		case FTPSESS_COMMAND:
			if ( _tcsicmp( sCmd, _T("quit") ) == 0 )
			{
				SayGoodbye();
			}
			else if ( _tcsicmp( sCmd, _T("abor") ) == 0 )
			{
				Msg (502, _T("No effect."));
			}
			else if ( _tcsicmp( szCmd, _T("port") ) == 0 )
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
				Msg( 500, _T("Command \"%s\" unknown."), szCmd );
			}
			break;
		case FTPSESS_TRANSFER:
			if ( _tcsicmp( sCmd, _T("quit") ) == 0 )
			{
				AbortTransfer();
				SayGoodbye();
			}
			else if ( _tcsicmp( sCmd, _T("abor") ) == 0 )
			{
				AbortTransfer();
				// We need to tell transfer thread to stop!!
			//	Msg (502, _T("No effect."));
			}
			else
			{
				Msg( 500, _T("Only ABOR and QUIT available. (%s)"), sCmd.c_str() );
			}
			break;
		case FTPSESS_SHUTDOWN:
			break;
	}


	return hr;
}

HRESULT CXBFTPSession::Stop( bool bForce )
{
//	SayGoodbye();
	HRESULT hr;
	Msg (221, _T("Goodbye."));
	hr = CXBNetSession::Stop(bForce);
	return hr;
}


// Display "Goodbye" message and end session
void CXBFTPSession::SayGoodbye(void)
{
	Msg (221, _T("Goodbye."));
	m_iFTPCmdState = FTPSESS_SHUTDOWN;
	m_bContinue = false;
}

void CXBFTPSession::doSIZE( LPCTSTR szArg )
{
	Msg (502, _T("Command not implemented."));
}

void CXBFTPSession::doPORT( LPCTSTR szArg )
{
	unsigned int h1, h2, h3, h4, p1, p2;

	_stscanf( szArg, _T("%u,%u,%u,%u,%u,%u"), &h1, &h2, &h3, &h4, &p1, &p2);
	m_sRemoteIP.Format( _T("%u.%u.%u.%u"), h1, h2, h3, h4);
	m_wRemotePort = 256*p1 + p2;
	m_bPassive = false;

	Msg (200, _T("PORT command successful."));
}

void CXBFTPSession::doPWD( LPCTSTR szArg )
{
	doPWDLine( 257 );
}


void CXBFTPSession::doPWDLine( int iResult )
{
	CStdString sDir = m_sCurDir;

	if ( sDir.GetLength() > 1 )
	{
		sDir += _T("/");
	}
	Msg( iResult, _T("\"%s\" is current directory."), sDir.c_str() );
}


void CXBFTPSession::doEXEC( LPCTSTR szArg )
{
	CStdString sPath = TranslateFilenameToFileSys( m_sCurDir, NULL );
	CStdString sArg(szArg);
	CStdString sCmd, sArgs;
	int iPos;

	sArg.Trim();
	sPath += _T("\\");

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

	Msg (200, _T("Ok. EXEC: (%s) (%s) (%s)"), sPath.c_str(), sCmd.c_str(), sArgs.c_str() );
	FireEXEC( sPath, sCmd, sArgs );

//	m_pServer->FireExec( sCmd, sArgs );
//	Msg (502, _T("Command not implemented."));
}

void CXBFTPSession::doNOOP( LPCTSTR szArg )
{
	Msg (200, _T("NOOP okay."));
}

void CXBFTPSession::doCDUP( LPCTSTR szArg )
{
	CStdString sNewPath = ConvertSlashes(m_sCurDir);

	
	sNewPath = BackupPathSection( sNewPath );
	sNewPath.Replace( _T("\\"), _T("/") );

	m_sCurDir = sNewPath;

	doPWDLine(250);
	// Msg( 250, _T("\"%s\/" is the cwd."), m_sCurDir.c_str() );
//	Msg (250, _T("CWD command successful."));
}


void CXBFTPSession::BuildHostIP( void )
{
	// Now let's get our IP.
	if ( !m_bCfgIP )
	{
		XNADDR xaddr;
		in_addr iaddr;
		XNKID x;
		XNetGetTitleXnAddr(&xaddr); 
		
		
		XNetXnAddrToInAddr (&xaddr, &x, &iaddr);

		
		m_h1 = xaddr.ina.S_un.S_un_b.s_b1; m_h2 = xaddr.ina.S_un.S_un_b.s_b2; m_h3 = xaddr.ina.S_un.S_un_b.s_b3; m_h4 = xaddr.ina.S_un.S_un_b.s_b4;
		// m_h1 = iaddr.S_un.S_un_b.s_b1; m_h2 = iaddr.S_un.S_un_b.s_b2; m_h3 = iaddr.S_un.S_un_b.s_b3; m_h4 = iaddr.S_un.S_un_b.s_b4;
		m_bCfgIP= true;
	}
}

void CXBFTPSession::doPASV( LPCTSTR szArg )
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
	m_wPassivePort = p1*256 + p2;

	if ( !m_bCfgIP )
	{
		BuildHostIP();
	}

	h1 = m_h1; h2 = m_h2; h3 = m_h3; h4 = m_h4;
//	SetPASVBinding();
	m_bPassive = true;
	
#ifdef HARDTEST
	wsprintf (buffer, "Entering Passive Mode (127,0,0,1,%d,%d).", (int)p1, (int)p2);
#else
	wsprintf (buffer, "Entering Passive Mode (%d,%d,%d,%d,%d,%d).",(int)h1, (int)h2, (int)h3, (int)h4, (int)p1, (int)p2);
#endif
	Msg (227, buffer);
	
	if ( __XFS_THREADED__ )
	{
		m_iXferThreadState = XTS_PASV;
		SetEvent( m_hXferThreadEvent );
		Sleep(10); // Give the transfer thread time to set up PASV, **BEFORE** displaying message
		// and allowing client time to try an attempt to establish the connection
	}
	else
	{

		if ( m_sockPasv && (m_sockPasv != INVALID_SOCKET) )
		{
			_SockClose( m_sockPasv );
		}
		m_sockPasv = INVALID_SOCKET;

		InitiatePasvListen();

	//	HandlePASV();
	}
	// InitiatePasvListen();
}

void CXBFTPSession::doTYPE( LPCTSTR szArg )
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

void CXBFTPSession::doSTRU( LPCTSTR szArg )
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

void CXBFTPSession::doRETR( LPCTSTR szArg )
{
	CStdString sFilename;
	CStdString sArg(szArg);

	sFilename = TranslateFilenameToFileSys( m_sCurDir, sArg );

	if ( FileExists( sFilename ) )
	{
		Msg (150, _T("Opening BINARY mode data connection for file transfer.") );
		// SendFile(sFilename, m_dwRest);

		m_hXferFile = ::CreateFile (sFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		m_bXferFile = true;
		m_iXferThreadState = XTS_XMT;
		if ( __XFS_THREADED__ )
		{
			// Sleep( 50 );
			SetEvent(m_hXferThreadEvent);
		}
		else
		{
			HandleXmt();
			if ( m_sockListen != INVALID_SOCKET )
			{
				shutdown (m_sockListen, 2);
				_SockClose( m_sockListen );
				m_sockListen = INVALID_SOCKET;
			}
			if ( m_sockPasv != INVALID_SOCKET )
			{
				shutdown (m_sockPasv, 2);
				_SockClose( m_sockPasv );
				m_sockPasv = INVALID_SOCKET;
			}
			SetEvent( m_hXferThreadDone );
		}
		// Msg (226, _T("Transfer complete."));
	}
	else
	{
		Msg (500, _T("File (%s) does not exist."), sFilename.c_str() );
	}
	m_bRest = false; m_dwRest = 0;
}

void CXBFTPSession::doALLO( LPCTSTR szArg )
{
	doNOOP(szArg);
}

void CXBFTPSession::doREST( LPCTSTR szArg )
{
	m_bRest = true;
	m_dwRest = _tstol(szArg);
	Msg (350, _T("Restart okay."));
}

void CXBFTPSession::doRNFR( LPCTSTR szArg )
{
	CStdString sFilename;
	CStdString sArg(szArg);

	sFilename = TranslateFilenameToFileSys( m_sCurDir, sArg );

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

void CXBFTPSession::doRNTO( LPCTSTR szArg )
{
	CStdString sFilename;
	CStdString sArg(szArg);
//	DWORD dwError;

	sFilename = TranslateFilenameToFileSys( m_sCurDir, sArg );

	if ( m_sRename.GetLength() && FileExists( m_sRename ) )
	{
		if (MoveFile (m_sRename, sFilename))
		{
			Msg (250, _T("Rename was successful."));
		}
		else
		{
			Msg (553, _T("Rename was not successful (0x%08x)."), GetLastError() );
		}
	}
	else
	{
		Msg (553, _T("File (%s) does not exist."), m_sRename );
	}
	m_sRename = _T("");
}

void CXBFTPSession::doDELE( LPCTSTR szArg )
{
	CStdString sArg(szArg);
	CStdString sFilename;

	sFilename = TranslateFilenameToFileSys( m_sCurDir, sArg );

	if (!m_bCanDelete)
	{
		Msg (550, _T("Insufficient access."));
	}
	else
	{
		DWORD dwAttr = GetFileAttributes( sFilename );
		if ( dwAttr == (DWORD)(-1) )
		{
			// Yeah, file didn't exist, but why break their hearts?  It's the end result that matters
			Msg (250, _T("File deleted."));
		}
		else
		{ 
			if ( dwAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_DIRECTORY) )
			{
				Msg (550, _T("Delete was not successful."));
			}
			else
			{
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
	}
}

void CXBFTPSession::doRMD( LPCTSTR szArg )
{
	CStdString sFilename;
	CStdString sArg(szArg);

	sFilename = TranslateFilenameToFileSys( m_sCurDir, sArg );

	if ( ( sFilename.GetLength()>3 ) && RemoveDirectory (sFilename))
	{
		Msg (257, _T("Directory removed successfully."));
	}
	else
	{
		Msg (421, _T("Error removing directory (0x%08x)."), GetLastError() );
	}
}

void CXBFTPSession::doMKD( LPCTSTR szArg )
{
	CStdString sFilename;
	CStdString sArg(szArg);

	sFilename = TranslateFilenameToFileSys( m_sCurDir, sArg );

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
			// Nested, perhaps? Try our UberCreateDirectory routine.
			if ( SUCCEEDED( MakePath( sFilename ) ) )
			{
				bSuccess = true;
			}
			else
			{
				bSuccess = false;
			}
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
		Msg (421, _T("Error creating directory (0x%08x)."), GetLastError() );
	}
}

void CXBFTPSession::doCWD( LPCTSTR szArg )
{
	bool bDisplayMsg = true;
	CStdString sArg(szArg);
	CStdString sRealPath;
	CStdString sFTPPath;

	if ( sArg.Compare( _T(".") ) == 0 )
	{
	}
	else if ( sArg.Compare( _T("..") ) == 0 )
	{
		doCDUP(sArg);
		bDisplayMsg = false;
	}
	else
	{
		sFTPPath = CombinePathAndRelative( m_sCurDir, ConvertSlashes(sArg) );
		sRealPath = TranslateFilenameToFileSys( sFTPPath, NULL );
		sFTPPath.Replace( _T("\\"), _T("/") );

		if ( sFTPPath.Compare( _T("/") ) )
		{
			DWORD dwAttr = GetFileAttributes( sRealPath );
			if ( (dwAttr!=(DWORD)(-1)) && (dwAttr&FILE_ATTRIBUTE_DIRECTORY) )
			{
				m_sCurDir = sFTPPath;
			}
			else
			{
				bDisplayMsg = false;
			}
		}
		else
		{
			m_sCurDir = _T("/");
		}
	}


	if ( bDisplayMsg )
	{
		doPWDLine(250);
		// Msg( 250, _T("\"%s\/" is the cwd."), m_sCurDir.c_str() );
	//	Msg (250, _T("CWD command successful."));
	}
	else
	{
		Msg (553, _T("Requested action not taken. Directory not found"));
	}
}

void CXBFTPSession::doLIST( LPCTSTR szArg )
{
	Msg (150, _T("Opening ASCII mode data connection for ls."));

	CStdString sDirectoryList = GetDirectory(false);
	if ( AllocateXferBuffer( sDirectoryList.GetLength()) )
	{
		memcpy( m_pbXferBuffer, (BYTE*)sDirectoryList.c_str(), sDirectoryList.GetLength() );
		// m_pbXferBuffer[sDirectoryList.GetLength()] = 0;
		// OutputDebugString( (LPCSTR)m_pbXferBuffer );
		m_bXferFile = false;
		m_iXferThreadState = XTS_XMT;
		// Sleep( 50 );
		if ( __XFS_THREADED__ )
		{
			SetEvent(m_hXferThreadEvent);
		}
		else
		{
			HandleXmt();
			if ( m_sockListen != INVALID_SOCKET )
			{
				shutdown (m_sockListen, 2);
				_SockClose( m_sockListen );
				m_sockListen = INVALID_SOCKET;
			}
			if ( m_sockPasv != INVALID_SOCKET )
			{
				shutdown (m_sockPasv, 2);
				_SockClose( m_sockPasv );
				m_sockPasv = INVALID_SOCKET;
			}
			SetEvent( m_hXferThreadDone );
		}
	}
//	SendData ((BYTE*)sDirectoryList.c_str(), sDirectoryList.GetLength());
	// Msg (226, _T("Listing completed."));
//	Msg (226, _T("Transfer finished successfully. Data Connection Closed."));
}

void CXBFTPSession::doNLST( LPCTSTR szArg )
{
	Msg (150, _T("Opening ASCII mode data connection for ls."));

	CStdString sDirectoryList = GetDirectory(true);
	if ( AllocateXferBuffer(sDirectoryList.GetLength()) )
	{
		memcpy( m_pbXferBuffer, (BYTE*)sDirectoryList.c_str(), sDirectoryList.GetLength() );
		m_bXferFile = false;
		m_iXferThreadState = XTS_XMT;
		// Sleep( 50 );
		if ( __XFS_THREADED__ )
		{
			SetEvent(m_hXferThreadEvent);
		}
		else
		{
			HandleXmt();
			if ( m_sockListen != INVALID_SOCKET )
			{
				shutdown (m_sockListen, 2);
				_SockClose( m_sockListen );
				m_sockListen = INVALID_SOCKET;
			}
			if ( m_sockPasv != INVALID_SOCKET )
			{
				shutdown (m_sockPasv, 2);
				_SockClose( m_sockPasv );
				m_sockPasv = INVALID_SOCKET;
			}
			SetEvent( m_hXferThreadDone );
		}
	}
//	SendData ((BYTE*)sDirectoryList.c_str(), sDirectoryList.GetLength());
	// Msg (226, _T("Listing completed."));
//	Msg (226, _T("Transfer finished successfully. Data Connection Closed."));
}

void CXBFTPSession::doSITE( LPCTSTR szArg )
{
	CStdString sPath = TranslateFilenameToFileSys( m_sCurDir, NULL );
	CStdString sArg(szArg);
	CStdString sResponse;
	HRESULT hr;

	if ( sPath.Left(1).Compare( _T("\\") ) )
	{
		sPath += _T("\\");
	}	

	hr = FireSITE( sPath, sArg, sResponse );
	if ( SUCCEEDED( hr ) )
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
		if ( sResponse.GetLength() )
		{
			// Display as information here...
			Msg ( 599, sResponse );
		}
		else if ( hr == E_NOTIMPL )
		{
			Msg (502, _T("Command not implemented."));
		}
		else
		{
			Msg (502, _T("Command Failed."));
		}
	}

//	Msg (200, _T("Ok. EXEC: (%s) (%s) (%s)"), sPath.c_str(), sCmd.c_str(), sArgs.c_str() );
//	Msg (502, _T("Command not implemented."));
}

void CXBFTPSession::doSYST( LPCTSTR szArg )
{
	Msg (215, _T("UNIX Type: L8"));
}

void CXBFTPSession::doSTOR( LPCTSTR szArg )
{
	CStdString sFilename;
	CStdString sArg(szArg);
	bool bContinue = true;

	sFilename = TranslateFilenameToFileSys( m_sCurDir, sArg );

	if ( m_bRest && m_dwRest )
	{
		// Got to be there!!
		if ( !FileExists( sFilename ) )
		{
			bContinue = false;
		}
	}
	if ( bContinue )
	{
		Msg (150, _T("Opening BINARY mode data connection for file transfer.") );
		// SendFile(sFilename, m_dwRest);
		if ( m_bRest && m_dwRest )
		{
			m_hXferFile = ::CreateFile (sFilename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		}
		else
		{
			m_hXferFile = ::CreateFile (sFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		}
		m_bAppend = false;
		m_bXferFile = true;
		m_iXferThreadState = XTS_RCV;

		if ( __XFS_THREADED__ )
		{		
		
		// Sleep( 50 );
			SetEvent(m_hXferThreadEvent);
		}
		else
		{
			HandleRcv();
			if ( m_sockListen != INVALID_SOCKET )
			{
				shutdown (m_sockListen, 2);
				_SockClose( m_sockListen );
				m_sockListen = INVALID_SOCKET;
			}
			if ( m_sockPasv != INVALID_SOCKET )
			{
				shutdown (m_sockPasv, 2);
				_SockClose( m_sockPasv );
				m_sockPasv = INVALID_SOCKET;
			}
			SetEvent( m_hXferThreadDone );
		}
//		Msg (226, _T("Transfer complete."));
	}
	else
	{
		Msg (500, _T("File (%s) does not exist."), sFilename.c_str() );
	}
	m_bRest = false; m_dwRest = 0;
}

void CXBFTPSession::doSTOU( LPCTSTR szArg )
{
	Msg (502, _T("Command not implemented."));
}

void CXBFTPSession::doAPPE( LPCTSTR szArg )
{
	CStdString sFilename;
	CStdString sArg(szArg);

	sFilename = TranslateFilenameToFileSys( m_sCurDir, sArg );

	if ( FileExists( sFilename ) )
	{
		Msg (150, _T("Opening BINARY mode data connection for file transfer.") );
		// SendFile(sFilename, m_dwRest);
		m_hXferFile = ::CreateFile (sFilename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if ( m_hXferFile && m_hXferFile !=INVALID_HANDLE_VALUE )
		{
			SetFilePointer ( m_hXferFile, 0, 0, FILE_END);
		}
		m_bAppend = true;
		m_bXferFile = true;
		m_iXferThreadState = XTS_RCV;
		// Sleep( 50 );
		if ( __XFS_THREADED__ )
		{
			SetEvent(m_hXferThreadEvent);
		}
		else
		{
			HandleRcv();
			if ( m_sockListen != INVALID_SOCKET )
			{
				shutdown (m_sockListen, 2);
				_SockClose( m_sockListen );
				m_sockListen = INVALID_SOCKET;
			}
			if ( m_sockPasv != INVALID_SOCKET )
			{
				shutdown (m_sockPasv, 2);
				_SockClose( m_sockPasv );
				m_sockPasv = INVALID_SOCKET;
			}
			SetEvent( m_hXferThreadDone );
		}
//		Msg (226, _T("Transfer complete."));
	}
	else
	{
		Msg (500, _T("File (%s) does not exist."), sFilename.c_str() );
	}
	m_bRest = false; m_dwRest = 0;
}


void CXBFTPSession::doABOR( LPCTSTR szArg )
{
	Msg (502, _T("Command not implemented."));
}

void CXBFTPSession::doSTAT( LPCTSTR szArg )
{
	Msg (502, _T("Command not implemented."));
}

void CXBFTPSession::doHELP( LPCTSTR szArg )
{
	Msg (502, _T("Command not implemented."));
}

void CXBFTPSession::doACCT( LPCTSTR szArg )
{
	Msg (552, _T("Account information discarded."));
}

void CXBFTPSession::doSMNT( LPCTSTR szArg )
{
	Msg (502, _T("Command not implemented."));
}

void CXBFTPSession::doREIN( LPCTSTR szArg )
{
	BeginLogin(false);
}

void CXBFTPSession::doMODE( LPCTSTR szArg )
{
	Msg (502, _T("Command not implemented."));
}

// Callback for main thread
DWORD CXBFTPSession::XferThreadHandlerProc(LPVOID pParam)
{
	CXBFTPSession * pThis = (CXBFTPSession *)pParam;

	return pThis->XferThreadHandler();
}

//// Initiate the transfer thread into sending a file over the data socket
//HRESULT CXBFTPSession::SendFile(LPCTSTR szFilename, DWORD dwOffset)
//{
//	return E_NOTIMPL;
//}
//
//// Send a buffer using the transfer thread over the data socket
//HRESULT CXBFTPSession::SendBuffer(LPBYTE pbBuffer, DWORD dwSize, DWORD dwOffset)
//{
//	return E_NOTIMPL;
//}
//
//// Recieve a file over the data socket using the transfer thread
//HRESULT CXBFTPSession::RecvFile(LPCTSTR szFilename, DWORD dwOffset)
//{
//	return E_NOTIMPL;
//}
//
//// Recieve a buffer using the transfer thread over the data socket
//HRESULT CXBFTPSession::RecvBuffer(LPBYTE pbBuffer, DWORD dwSize, DWORD dwOffset)
//{
//	return E_NOTIMPL;
//}
//

//
//HRESULT CXBFTPSession::GetFile( LPCTSTR szFilename, bool bAppend )
//{
//	HRESULT hrReturn = E_FAIL;
//	bool bSucceeded = false;
//	DWORD	dwBuffSize = FTP_WRITEBLOCKSIZE;
//	char * buff = m_pbTransferBuffer;
//	char smallbuff[4096];
//
//	if ( buff == NULL )
//	{
//		buff = smallbuff;
//		dwBuffSize = 4096;
//	}
//
//	if ( !m_bPassive )
//	{
//		SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
//		struct sockaddr_in addr;
//		addr.sin_family = AF_INET;
//		addr.sin_port = htons (m_wRemotePort);
//		addr.sin_addr.s_addr = inet_addr (m_sRemoteIP);
//
//		if (connect (sock, (struct sockaddr *)&addr, sizeof (struct sockaddr)) != -1)
//		{
//			HANDLE hFile;
//			if (!bAppend)
//			{
//				hFile = ::CreateFile (szFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
//			}
//			else
//			{
//				hFile = ::CreateFile (szFilename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//				SetFilePointer (hFile, 0, 0, FILE_END);
//			}
//			
//			if (hFile == INVALID_HANDLE_VALUE)
//			{
//				DWORD dwError = GetLastError();
//				Msg( 450, _T("Cannot access file.") );
//			}
//			else
//			{
//				int nBytes = recv (sock, buff, dwBuffSize, 0);
//				while ((nBytes != 0) && (nBytes != -1))
//				{
//					unsigned long nBytesW = 0, nBytesTotal = 0;
//					while ((int)nBytesTotal != nBytes)
//					{
//						BOOL bWriteSucc = WriteFile (hFile, buff, nBytes, &nBytesW, NULL);
//						if (bWriteSucc == 0)
//						{
//							Msg (450, _T("Cannot access file.") );
//							return E_FAIL;
//						}
//						nBytesTotal += nBytesW;
//					}
//					nBytes = recv (sock, buff, dwBuffSize, 0);
//				}
//				CloseHandle (hFile);
//				if (nBytes == 0)
//				{
//					Msg (226, _T("Transfer Complete.") );
//					bSucceeded = true;
//				}
//				else
//				{
//					Msg (426, "Connection closed; transfer aborted.");
//					// return E_FAIL;
//				}
//			}
//			shutdown (sock, 2);
//			closesocket(sock);
//			hrReturn = S_OK;
//		}
//	}
//	else
//	{
//		GetPasvSocket();
//		if ( m_sockPasv != INVALID_SOCKET)
//		{
//			HANDLE hFile;
//
//			if (!bAppend)
//			{
//				hFile = ::CreateFile (szFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
//			}
//			else
//			{
//				hFile = ::CreateFile (szFilename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//				SetFilePointer (hFile, 0, 0, FILE_END);
//			}
//			if (hFile == INVALID_HANDLE_VALUE)
//			{
//				Msg (450, _T("Cannot access file."));
//			}
//			else
//			{
//				int nBytes = recv (m_sockPasv, buff, dwBuffSize, 0);
//				while ((nBytes != 0) && (nBytes != -1))
//				{
//					unsigned long nBytesW = 0, nBytesTotal = 0;
//					while ((int)nBytesTotal != nBytes)
//					{
//						BOOL bWriteSucc = WriteFile (hFile, buff, nBytes, &nBytesW, NULL);
//						if (bWriteSucc == 0)
//						{
//							Msg (450, _T("Cannot access file."));
//							hrReturn = E_FAIL;
//							return E_FAIL;
//						}
//						nBytesTotal += nBytesW;
//					}
//					nBytes = recv (m_sockPasv, buff, dwBuffSize, 0);
//				}
//				CloseHandle (hFile);
//				if (nBytes == 0)
//				{
//					Msg (226, _T("Transfer Complete.") );
//					bSucceeded = true;
//				}
//				else
//				{
//					Msg (426, "Connection closed; transfer aborted.");
//					// return E_FAIL;
//				}
//			}
//			shutdown (m_sockPasv, 2);
//			closesocket( m_sockPasv );
//			m_sockPasv = INVALID_SOCKET;
//			m_bPassive = false;
//
//		}
//
//		// InitiatePasvListen();
//
//		//// Restart the PASV listen() call to get our socket
//		//SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
//		//struct sockaddr_in addr, *addr2 = NULL;
//		//int size = sizeof (struct sockaddr_in);
//
//		//ZeroMemory (&addr, size);
//		//addr.sin_family = AF_INET;
//		//addr.sin_port = htons (m_wPassivePort);
//		//addr.sin_addr.s_addr = htonl (INADDR_ANY);
//		//int ret = bind (sock, (struct sockaddr *)&addr, size);
//		//if (ret != 0)
//		//{
//		//	bSucceeded = false;
//		//	// return E_FAIL;
//		//}
//		//else
//		//{
//		//	ret = listen (sock, MAXUSERS);
//		//	if (ret != 0)
//		//	{
//		//		bSucceeded = false;
//		//	}
//		//	else
//		//	{
//		//		m_sockPasv = accept (sock, (struct sockaddr *)addr2, &size);
//		//		shutdown (sock, 2);
//		//		closesocket( sock );
//		//	}
//		//}
//		if ( bSucceeded )
//		{
//			hrReturn = S_OK;
//		}
//
//	}
//	return hrReturn;
//}
//


SOCKET CXBFTPSession::GetPasvSocket( void )
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
			m_sockPasv = _SockAccept(m_sockListen, (struct sockaddr *)addr2, &size );
			sockReturn = m_sockPasv;
			shutdown (m_sockListen, 2);
			_SockClose( m_sockListen );
			m_sockListen = INVALID_SOCKET;
		}
	}
	return sockReturn;
}


// Initiates the binding of the PASV port to listen for incoming transfer connections
bool CXBFTPSession::SetPASVBinding( void )
{
	bool bReturn = false;

	// Restart the PASV listen() call to get our socket
	if ( m_sockListen != INVALID_SOCKET )
	{
		shutdown (m_sockListen, 2);
		_SockClose( m_sockListen );
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
		ret = _SockListen(m_sockListen, 4);
		if (ret == 0)
		{
			bReturn = true;
		}
	}
	return bReturn;
}


bool CXBFTPSession::InitiatePasvListen( void )
{
	bool bReturn = false;

	if ( SetPASVBinding() )
	{
		GetPasvSocket();
		bReturn = true;
	}
	return bReturn;
}


static LPCTSTR GetAttrString( DWORD dwAttr )
{
	LPCTSTR szReturn;

	if ( dwAttr&FILE_ATTRIBUTE_DIRECTORY )
	{
		if ( dwAttr&FILE_ATTRIBUTE_READONLY )
		{
			szReturn = _T("dr--r--r--");
		}
		else
		{
			szReturn = _T("drw-r--r--");
		}
	}
	else
	{
		if ( dwAttr&FILE_ATTRIBUTE_READONLY )
		{
			szReturn = _T("-r--r--r--");
		}
		else
		{
			szReturn = _T("-rw-r--r--");
		}
	}

	return szReturn;
}

CStdString CXBFTPSession::GetDirectory (bool bShort)
{
	CStdString sReturn;
	//char *result = NULL;
	//const char dir[] = "drwx------ ";
	//const char file[] = "-rwx------ ";
	//const char group[] = "1 user group ";

	CXBDirectoryLister dirLister( m_sCurDir );


	dirLister.Reset();
	CStdString sName;
	DWORD dwAttr;
	ULONGLONG u64Size;
	FILETIME ftTime;
	CStdString sDirLine;
	CStdString sEntryTime;
	CStdString sEntryAttr;
	DWORD dwSize;

	if (!bShort)
	{
		sReturn = _T("drw-r--r--   1 XBOX      XBOX        0 Oct 26 10:03 ..\r\n");
	}
	else
	{
		sReturn = _T("..\r\n");
	}
	while( dirLister.GetNext( sName, dwAttr, u64Size, ftTime ) )
	{
		if (!bShort)
		{
			dwSize = (DWORD)u64Size;
			SYSTEMTIME st;
			FILETIME ft;

			FileTimeToLocalFileTime (&ftTime, &ft);
			FileTimeToSystemTime(&ft, &st);
			sEntryTime.Format( _T("%s %02d %02d:%02d"), szMonths[st.wMonth], st.wDay, st.wHour, st.wMinute );
			
			sDirLine.Format( _T("%s   1 XBOX      XBOX        %d %s %s\r\n"), GetAttrString(dwAttr),
						dwSize,
						sEntryTime.c_str(),
						sName.c_str() );
		}
		else
		{
			sDirLine.Format( _T("%s\r\n"), sName.c_str() );
		}
		sReturn += sDirLine;
	}
	return sReturn;
}

SOCKET CXBFTPSession::ConnectToXferPort ( void )
{
	SOCKET sock = INVALID_SOCKET;
	if ( m_bPassive )
	{
		if ( m_sockPasv == INVALID_SOCKET )
		{
			if ( m_sockListen != INVALID_SOCKET )
			{
				struct sockaddr_in *addr2 = NULL;
				int size = sizeof (struct sockaddr_in);
				m_sockPasv = _SockAccept(m_sockListen, (struct sockaddr *)addr2, &size );
				sock = m_sockPasv;
			}
		}
		else
		{
			sock = m_sockPasv;
		}
	}
	else
	{
		sock = socket (AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in addr;

		addr.sin_family = AF_INET;
		addr.sin_port = htons (m_wRemotePort);
		addr.sin_addr.s_addr = inet_addr (m_sRemoteIP);
		if (_SockConnect (sock, (struct sockaddr *)&addr, sizeof (struct sockaddr)) == -1)
		{
			if ( m_sockPasv == sock )
			{
				m_sockPasv = INVALID_SOCKET;
			}
			_SockClose( sock );
			sock = INVALID_SOCKET;
		}
	}
	return sock;
}

HRESULT CXBFTPSession::SendDataBuffer (SOCKET sock, BYTE *buff, int iLen)
{
	HRESULT hr = E_FAIL;
	if ( sock != INVALID_SOCKET )
	{
		if ( buff && iLen )
		{
			Send (sock, (LPCSTR)buff, iLen);
			hr = S_OK;
		}
		shutdown (sock, 2);
		if ( m_sockPasv == sock )
		{
			m_sockPasv = INVALID_SOCKET;
		}
		_SockClose(sock);
	}
	return hr;
}

HRESULT CXBFTPSession::GetDataFile (SOCKET sock, HANDLE hFile, long iStartAt )
{
	HRESULT hr = E_FAIL;
	DWORD	dwBuffSize = FTP_WRITEBLOCKSIZE;
	char * buff = m_pbTransferBuffer;
	char smallbuff[4096];
	

	m_sockXfer = sock;
	if ( buff == NULL )
	{
		buff = smallbuff;
		dwBuffSize = 4096;
	}

	if ( sock != INVALID_SOCKET )
	{
		if ( hFile )
		{
//			DWORD dwRead;
			hr = S_OK;
			if ( iStartAt != -1 )
			{
				SetFilePointer (hFile, iStartAt, NULL, FILE_BEGIN);
			}

			int nBytes = recv (sock, buff, dwBuffSize, 0);

			hr = S_OK;

			// m_iXferThreadState == XTS_RCV

			while ( (m_iXferThreadState == XTS_RCV) && (hr==S_OK) && (nBytes != 0) && (nBytes != -1))
			{
				unsigned long nBytesW, nBytesTotal;
				nBytesTotal = 0;
				nBytesW = 0;
				while ( (m_iXferThreadState == XTS_RCV) && (hr==S_OK) && ((int)nBytesTotal != nBytes))
				{
					BOOL bWriteSucc = WriteFile (hFile, buff, nBytes, &nBytesW, NULL);
					if (bWriteSucc == 0)
					{
						// Msg (450, _T("Cannot access file."));
						hr = E_FAIL;
						nBytes = -1;
						break;
					}
					else
					{
						nBytesTotal += nBytesW;
					}
				}
				nBytesW = 0;
				if ( hr == S_OK )
				{
					nBytes = recv (sock, buff, dwBuffSize, 0);
				}
			}
			CloseHandle( hFile );
		}
		shutdown (sock, 2);
		if ( m_sockPasv == sock )
		{
			m_sockPasv = INVALID_SOCKET;
		}
		_SockClose(sock);
	}
	if ( m_iXferThreadState != XTS_RCV )
	{
		hr = E_FAIL;
	}
	m_iXferThreadState = XTS_IDLE;
	m_sockXfer = INVALID_SOCKET;
	return hr;
}


HRESULT CXBFTPSession::SendDataFile (SOCKET sock, HANDLE hFile, long iStartAt )
{
	HRESULT hr = E_FAIL;
	DWORD	dwBuffSize = FTP_WRITEBLOCKSIZE;
	char * buff = m_pbTransferBuffer;
	char smallbuff[4096];
    ULONGLONG ul64ReadTimeStamp, ul64SentTimeStamp;
	ULONGLONG ul64ReadAccum = 0i64, ul64SentAccum = 0i64;
	DWORD dwBytesRead = 0, dwBytesSent = 0;


	m_sockXfer = sock;
	if ( buff == NULL )
	{
		buff = smallbuff;
		dwBuffSize = 4096;
	}

	if ( sock != INVALID_SOCKET )
	{
		if ( hFile )
		{
			DWORD dwRead;
			hr = S_OK;
			if ( iStartAt != -1 )
			{
				SetFilePointer (hFile, iStartAt, NULL, FILE_BEGIN);
			}

			ul64ReadTimeStamp = ReadTimeStampCounter();
			BOOL bReadSucc = ReadFile (hFile, buff, dwBuffSize, &dwRead, NULL);
			ul64ReadAccum += ReadTimeStampCounter()-ul64ReadTimeStamp;
			dwBytesRead = dwRead;

			while ( 
#if __XFS_THREADED__
				(m_iXferThreadState == XTS_XMT) && 
#endif
				bReadSucc && (dwRead != 0))
			{
				ul64SentTimeStamp = ReadTimeStampCounter();
				if( SUCCEEDED( Send (sock, buff, dwRead) ) )
				{
					ul64SentAccum += ReadTimeStampCounter()-ul64SentTimeStamp;
					dwBytesSent += dwRead;

					ul64ReadTimeStamp = ReadTimeStampCounter();
					if ( ( bReadSucc = ReadFile (hFile, buff, dwBuffSize, &dwRead, NULL) ) == false )
					{
						if ( GetLastError() != ERROR_HANDLE_EOF )
						{
							hr = E_FAIL;
						}
					}					
					ul64ReadAccum += ReadTimeStampCounter()-ul64ReadTimeStamp;
					dwBytesRead += dwRead;
				}
				else
				{
					ul64SentAccum += ReadTimeStampCounter()-ul64SentTimeStamp;
					// dwBytesSent += dwRead;
					hr = E_FAIL;
					bReadSucc = false;
				}
			}
			CloseHandle( hFile );
		}
		shutdown (sock, 2);
		if ( m_sockPasv == sock )
		{
			m_sockPasv = INVALID_SOCKET;
		}
		_SockClose(sock);
	}
	if ( m_iXferThreadState != XTS_XMT )
	{
		hr = E_FAIL;
	}

	// Calculate bytes per second sent...
	ul64ReadAccum += 500i64;
	ul64ReadAccum /= 1000i64; // Milliseconds
	FLOAT fReadTime = (FLOAT)ul64ReadAccum;
	fReadTime /= 1000.0;

	ul64SentAccum += 500i64;
	ul64SentAccum /= 1000i64; // Milliseconds
	FLOAT fSentTime = (FLOAT)ul64SentAccum;
	fSentTime /= 1000.0;

	FLOAT fChars = (FLOAT)dwBytesRead;
	FLOAT fCPSRead = fChars/fReadTime;
	FLOAT fCPSSent = fChars/fSentTime;



	m_iXferThreadState = XTS_IDLE;
	m_sockXfer = INVALID_SOCKET;
	Sleep(50);
	return hr;
}


HRESULT CXBFTPSession::SendData (BYTE *buff, int iLen)
{
	HRESULT hrReturn = S_OK;

	if ( buff && iLen )
	{
		if (!m_bPassive)
		{
			SOCKET sock = ConnectToXferPort();
			m_sockXfer = sock;
			//struct sockaddr_in addr;

			//addr.sin_family = AF_INET;
			//addr.sin_port = htons (m_wRemotePort);
			//addr.sin_addr.s_addr = inet_addr (m_sRemoteIP);
			//if (connect (sock, (struct sockaddr *)&addr, sizeof (struct sockaddr)) == -1)
			//{
			//	return E_FAIL;
			//}
			if ( sock != INVALID_SOCKET )
			{
				Send (sock, (LPCSTR)buff, iLen);
				shutdown (sock, 2);
				if ( m_sockPasv == sock )
				{
					m_sockPasv = INVALID_SOCKET;
				}
				_SockClose(sock);
			}
			else
			{
				hrReturn = E_FAIL;
			}
		}
		else
		{
			GetPasvSocket();
			if (m_sockPasv != INVALID_SOCKET)
			{
				Send (m_sockPasv, (LPSTR)buff, iLen);
				shutdown (m_sockPasv, 2);
				_SockClose(m_sockPasv);
			}
			m_sockPasv = INVALID_SOCKET;
			m_bPassive = false;
		}
	}
	else
	{
		hrReturn = E_FAIL;
	}
	if ( m_iXferThreadState != XTS_XMT )
	{
		hrReturn = E_FAIL;
	}
	m_iXferThreadState = XTS_IDLE;
	m_sockXfer = INVALID_SOCKET;
	Sleep(50);
	return hrReturn;
}

void CXBFTPSession::SetFTPEventHandler(CXBFTPSessionEvents * pEventHandler)
{
	m_eventFTPHandler = pEventHandler;
}

DWORD CXBFTPSession::FireEXEC( LPCTSTR szPath, LPCTSTR szFilename, LPCTSTR szArgs )
{
	DWORD dwReturn = 200;
	if ( m_eventFTPHandler )
	{
		dwReturn = m_eventFTPHandler->EXEC( this, szPath, szFilename, szArgs );
	}
	return dwReturn;
}

DWORD CXBFTPSession::FireCMD( LPCTSTR szPath, LPCTSTR szCMD, LPCTSTR szArgs, CStdString & sResult )
{
	DWORD dwReturn = 200;
	if ( m_eventFTPHandler )
	{
		dwReturn = m_eventFTPHandler->CMD( this, szPath, szCMD, szArgs, sResult );
	}
	return dwReturn;
}

HRESULT CXBFTPSession::FireSITE( LPCTSTR szPath, LPCTSTR szArgs, CStdString & sResult )
{
	HRESULT hr = S_OK;
	if ( m_eventFTPHandler )
	{
		hr = m_eventFTPHandler->SITE( this, szPath, szArgs, sResult );
	}
	return hr;
}

bool	CXBFTPSession::FireAllowAnonymous( void )
{
	bool bResult = true;

	if ( m_eventFTPHandler )
	{
		bResult = m_eventFTPHandler->AllowAnonymous( this );
	}
	return bResult;
}

DWORD	CXBFTPSession::FireValidateUser( LPCTSTR szUser, LPCTSTR szPass )
{
	DWORD dwResult = 0;

	if ( m_eventFTPHandler )
	{
		dwResult = m_eventFTPHandler->ValidateUser( this, szUser, szPass );
	}
	return dwResult;
}


void CXBFTPSession::InfoMsg( LPCTSTR szString )
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
						sTemp.Format( _T(" %s"), sTemp.c_str() );
					}
					sMsg.Format( _T("230- %s\r\n"), sTemp.c_str() );
					// sMsg.Format( _T("%s\r\n"), sTemp );
				}
			}
			SendCmd( sMsg, sMsg.GetLength() );
		}
		if ( sString.GetLength() )
		{
			sMsg.Format( _T("230- %s\r\n"), sString.c_str() );
			SendCmd( sMsg, sMsg.GetLength() );
			// Msg( 230, sString );
		}
	}
}

HRESULT CXBFTPSession::Initialize(void)
{
	HRESULT hr = CXBNetSession::Initialize();
	BOOL bSet = FALSE;
	
	if ( SUCCEEDED(hr) )
	{
#if __XFS_THREADED__
#else
		bSet = SetThreadPriority( m_hID, m_pFTPServer->GetXferPriority() );
#endif
	}
	return hr;
}
