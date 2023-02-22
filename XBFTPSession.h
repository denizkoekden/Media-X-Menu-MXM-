#pragma once

#include "XBNetSession.h"

class CXBFTPServer;

#define FTPSESS_LOGINUSER	0
#define FTPSESS_LOGINPASS	1
#define FTPSESS_COMMAND		2
#define FTPSESS_TRANSFER	3
#define FTPSESS_SHUTDOWN	4

#define FTPUSERPERM_CANDELETE	0x80000000
#define FTPUSERPERM_LOGINOK		0x00000001

class CXBFTPSession;

class CXBFTPSessionEvents
{
public:
	virtual DWORD	CMD( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szCmd, LPCTSTR szArgs, CStdString & sResult ) = 0;
	virtual DWORD	EXEC( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szFilename, LPCTSTR szArgs ) = 0;
	virtual HRESULT	SITE( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szArgs, CStdString & sResult ) = 0;
	virtual bool	AllowAnonymous( CXBFTPSession * pThis ) = 0;
	virtual DWORD	ValidateUser( CXBFTPSession * pThis, LPCTSTR szUser, LPCTSTR szPass ) = 0;
};

// Transfer Thread States
#define XTS_SHUTDOWN		0
#define XTS_PASV			1
#define XTS_ABORT			2
#define XTS_RCV				4
#define XTS_XMT				5
#define XTS_IDLE			6


class CXBFTPSession : virtual public CXBNetSession
{
protected:
	CXBFTPSessionEvents	*	m_eventFTPHandler;
	CXBFTPServer *	m_pFTPServer;
	int				m_iLoginTries;
	int				m_iFTPCmdState;
	int				m_iLoginRetries;
	WORD			m_wPassivePort;
	bool			m_bPassive;

	void			BuildHostIP( void );
	CStdString		GetDirectory (bool bShort);

	bool			m_bCfgIP;
	WORD			m_h1, m_h2, m_h3, m_h4;

	bool			m_bRest;
	DWORD			m_dwRest;
	bool			m_bAppend;

	HANDLE			m_hTransferThread;

	char	*		m_pbTransferBuffer;
	WORD			m_wRemotePort;

	CStdString		m_sRemoteIP;


	SOCKET			m_sockPasv;
	SOCKET			m_sockListen;

	CStdString		m_sCurDir;
	CStdString		m_sRename;

	bool			m_bCanDelete;

	CStdString		m_sLoginName;

	int				m_iXferThreadState;
	HANDLE			m_hXferThreadEvent;
	HANDLE			m_hXferThreadDone;
	SOCKET			m_sockXfer;

	bool			m_bXferFile;
	HANDLE			m_hXferFile;
	LPBYTE			m_pbXferBuffer;
	DWORD			m_dwXferBufferAllocSize;
	DWORD			m_dwXferBufferSize;
	bool			AllocateXferBuffer( DWORD dwSize );


	// Callback for main thread
	static DWORD WINAPI XferThreadHandlerProc(LPVOID pParam);
	// Handles the actual thread, relative to the object as a member
	DWORD XferThreadHandler(void);
	void	doPWDLine( int iResult );

	void	XferThreadStartup( void );
	void	XferThreadShutdown( void );

	bool	SetPASVBinding( void );
	SOCKET	ConnectToXferPort ( void );
	HRESULT SendDataFile (SOCKET sock, HANDLE hFile, long iStartAt = 0 );
	HRESULT GetDataFile (SOCKET sock, HANDLE hFile, long iStartAt = 0 );
	void	AbortTransfer( void );
	bool	HandlePASV( void );
	bool	HandleXmt( void );
	bool	HandleRcv( void );
	

public:
	CXBFTPSession(SOCKET sock, CXBFTPServer * pServer);
	virtual ~CXBFTPSession(void);
	virtual HRESULT	DoCommand( LPCTSTR szCmd = NULL, LPCTSTR szArg = NULL );
	virtual void DoGreeting( void );
	void	BeginLogin( bool bInitial );
	virtual HRESULT Initialize(void);
	HRESULT SendData (BYTE *buff, int iLen);
	HRESULT SendDataBuffer (SOCKET sock, BYTE *buff, int iLen);
	void	InfoMsg( LPCTSTR szString );

	// Ends session, gracefully, unless forced
	virtual HRESULT Stop( bool bForce = false );

	virtual bool	FireAllowAnonymous( void );
	virtual DWORD	FireValidateUser( LPCTSTR szUser, LPCTSTR szPass );
	virtual HRESULT FireSITE( LPCTSTR szPath, LPCTSTR szArgs, CStdString & sResult );
	virtual DWORD FireCMD( LPCTSTR szPath, LPCTSTR szCmd, LPCTSTR szArgs, CStdString & sResult );
	virtual DWORD FireEXEC( LPCTSTR szPath, LPCTSTR szFilename, LPCTSTR szArgs );

	// Set the event handler for our session
	void SetFTPEventHandler(CXBFTPSessionEvents * pEventHandler);

protected:
	// Display "Goodbye" message and end session
	void SayGoodbye(void);

	void doSIZE( LPCTSTR szArg );
	void doPORT( LPCTSTR szArg );
	void doPWD( LPCTSTR szArg );
	void doEXEC( LPCTSTR szArg );
	void doNOOP( LPCTSTR szArg );
	void doCDUP( LPCTSTR szArg );
	void doPASV( LPCTSTR szArg );
	void doTYPE( LPCTSTR szArg );
	void doSTRU( LPCTSTR szArg );
	void doRETR( LPCTSTR szArg );
	void doSTOR( LPCTSTR szArg );
	void doSTOU( LPCTSTR szArg );
	void doAPPE( LPCTSTR szArg );
	void doALLO( LPCTSTR szArg );
    void doREST( LPCTSTR szArg );
    void doRNFR( LPCTSTR szArg );
    void doRNTO( LPCTSTR szArg );
	void doABOR( LPCTSTR szArg );
	void doDELE( LPCTSTR szArg );
	void doRMD( LPCTSTR szArg );
	void doMKD( LPCTSTR szArg );
	void doCWD( LPCTSTR szArg );
	void doLIST( LPCTSTR szArg );
	void doNLST( LPCTSTR szArg );
	void doSITE( LPCTSTR szArg );
	void doSYST( LPCTSTR szArg );
	void doSTAT( LPCTSTR szArg );
	void doHELP( LPCTSTR szArg );
	void doACCT( LPCTSTR szArg );
	void doSMNT( LPCTSTR szArg );
	void doREIN( LPCTSTR szArg );
	void doMODE( LPCTSTR szArg );
	// Initiate the transfer thread into sending a file over the data socket
	HRESULT SendFile(LPCTSTR szFilename, DWORD dwOffset = 0);
	// Send a buffer using the transfer thread over the data socket
	HRESULT SendBuffer(LPBYTE pbBuffer, DWORD dwSize, DWORD dwOffset = 0);
	// Recieve a file over the data socket using the transfer thread
	HRESULT RecvFile(LPCTSTR szFilename, DWORD dwOffset = 0);
	// Receive a buffer using the transfer thread over the data socket
	HRESULT RecvBuffer(LPBYTE pbBuffer, DWORD dwSize, DWORD dwOffset = 0);

	HRESULT	GetFile( LPCTSTR szFilename, bool bAppend );
	SOCKET	GetPasvSocket( void );
	bool	InitiatePasvListen( void );
	
public:
};
