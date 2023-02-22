#pragma once

class CXBNetSession;

class CXBNetSessionEvents
{
public:
	virtual void	SessionStarted( CXBNetSession * pThis ) = 0;
	virtual void	SessionEnded( CXBNetSession * pThis ) = 0;
};

class CXBNetServer;



class CXBNetSession
{
protected:
	// Event handler using virtual class
	CXBNetSessionEvents	*	m_eventHandler;
	sockaddr 				m_addrSession;
	HANDLE					m_hID;
	CXBNetServer *			m_pServer;
	SOCKET					m_sockCmd;
	TListStrings			m_sqOutput;
	char					m_pszCmdBuffer[1050];
	CRITICAL_SECTION		m_csCmdCtrl;

	bool					IsRecvReady( SOCKET sock );

	static DWORD	WINAPI	ThreadHandlerProc( LPVOID pParam );
	virtual DWORD			Serve( void );
	virtual HRESULT			GetCommand( CStdString & sCmd, CStdString & sArg );
	virtual bool			RecvLine( CStdString & sLine );
	HRESULT					Recv( SOCKET sock, LPSTR szBuff, int iLen );
	HRESULT					Send( SOCKET sock, LPCSTR szBuff, int iLen );
	HRESULT					RecvCmd( LPSTR szBuff, int iLen );
	HRESULT					SendCmd( LPCSTR szBuff, int iLen );
	virtual HRESULT			DoCommand( LPCTSTR szCmd = NULL, LPCTSTR szArg = NULL );
	void					Msg( int iCode, LPCTSTR szLine, ... );
	void					QueueMsg( int iCode, LPCTSTR szLine, ... );

	virtual void			DoGreeting( void );

	bool					m_bContinue;

	int						m_iRefCount;

	HANDLE					m_hThreadEnded;

public:
	CXBNetSession( SOCKET sock, CXBNetServer * pServer );
	virtual ~CXBNetSession(void);

	virtual int AddRef( void ) { return ++m_iRefCount; };
	virtual int Release( void );
	// Set the IP our session is connecting to
	virtual void SetIP(struct sockaddr addrSession);
	// Initialize thread(s), other session information
	virtual HRESULT Initialize(void);
	// Begin Session  - starts thread(s)
	virtual HRESULT Start(void);
	// Ends session, gracefully, unless forced
	virtual HRESULT Stop( bool bForce = false );
	// Tell event handler we started our session
	virtual void FireSessionStarted(void);
	// Tell event handler we ended our session
	virtual void FireSessionEnded(void);
	// Set the event handler for our session
	void SetEventHandler(CXBNetSessionEvents * pEventHandler);
	// Return the ID of the session, based on the main thread ID
	HANDLE	GetID( void ) { return m_hID; };
};
