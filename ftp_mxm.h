
#ifndef __FTP_MXM_H__
#define __FTP_MXM_H__

#include "StdString.h"
#include "xmltree.h"
#include "FTPUserEntries.h"

class CFTPServer;


typedef HRESULT (*CallbackExec) (PVOID pContext, LPCTSTR szPath, LPCTSTR szCmdLine );
typedef void	(*CallbackLogon) (PVOID pContext );
typedef void	(*CallbackLogoff) (PVOID pContext );

// Analogous to CFTPUser
class CFTPThread
{
	CFTPServer *	m_pServer;
	char			m_pszCmdBuffer[1024];
	int				m_iCmdBufferTailPos;
	int				m_iCmdBufferHeadPos;
	char	*		m_pbTransferBuffer;
	CStdString		m_sIP;
	CStdString		m_sHostIP;
	HANDLE			m_hID;
	int				m_iRefCount;
	CStdString		m_sPassword;
	CStdString		m_sName;
	CStdString		m_sRoot;
	CStdString		m_sCurDir;
	CStdString		m_sCurDirBackup;
	CStdString		m_sRemoteIP;
	WORD			m_wRemotePort;
	bool			m_bCfgIP;
	WORD			m_h1, m_h2, m_h3, m_h4;
	bool			m_bLoggedOn;
	bool			m_bPassive;
	bool			m_bCanDelete;
	WORD			m_wPassivePort;
	bool			m_bRest;
	DWORD			m_dwRest;
	HANDLE			m_pasvSvr;
	SOCKET			m_sockCmd;
	SOCKET			m_sockData;
	SOCKET			m_sockPasv;
	SOCKET			m_sockListen;
	CStdString		m_sRename;
	SOCKET			GetPasvSocket( void );
	static DWORD	WINAPI ThreadHandlerProc( LPVOID pParam );
	bool			RecvLine( CStdString & sLine );
	void			SetCurDir( LPCTSTR szCurDir );
	CStdString		GetCurDir( void );
	void			GetIP( void );
	bool			InitiatePasvListen( void );
public:

	CFTPThread( SOCKET sock, CFTPServer * pServer );
	~CFTPThread();

	CStdString GetCurFTPDir( void ) { return m_sCurDir; };
	CStdString TranslateFTPDirToXboxDir( LPCTSTR szDir );
	CStdString GetDirectory (bool bShort = false);

	HRESULT GetFile( LPCTSTR szFilename, bool bAppend );
	HRESULT SendFile (LPCTSTR szFilename, unsigned int iStartAt);
	HRESULT	SendData (BYTE *buff, int iLen);

	HRESULT Recv( SOCKET sock, LPSTR szBuff, int iLen );
	HRESULT Send( SOCKET sock, LPCSTR szBuff, int iLen );

	void doSIZE( LPCTSTR szArg );
	void doPORT( LPCTSTR szArg );
	void doPWD( LPCTSTR szArg );
	void doEXEC( LPCTSTR szArg );
	void doNOOP( LPCTSTR szArg );
	void doCDUP( LPCTSTR szArg );
	void doQUIT( LPCTSTR szArg );
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

	int		AddRef( void );
	int		Release( void );
	HANDLE	GetID( void ) { return m_hID; };
	bool	GetUserInfo( CStdString & sName, CStdString & sPassword, CStdString & sCurDir, CStdString & sIP );

	void SetIP( sockaddr_in * pAddr );
	void ResumeThread( void );
	bool SpawnThread( void );

	HRESULT		Serve ( void );

	HRESULT DoCommand( LPCTSTR szCmd, LPCTSTR szArg );
	HRESULT GetCommand( CStdString & sCmd, CStdString & sArg );

	void	Msg( int iCode, LPCTSTR szLine );
	void	InfoMsg( LPCTSTR szString );

	HRESULT	DoLogin( void );
	HRESULT	DoWelcome( void );
	HRESULT	DoLogout( void );

};

typedef list<CFTPThread *> TFTPThreadList;

class CFTPThreadManager
{
public:
	TFTPThreadList		m_threadList;
	CFTPThreadManager();
	~CFTPThreadManager();
	bool			AddThread( CFTPThread *pThread );
	CFTPThread *	DetachByHandle( HANDLE hHandle, bool bAddRef = false );
	CFTPThread *	DetachByIndex( int iIndex, bool bAddRef = false );
	CFTPThread *	GetByHandle( HANDLE hHandle );
	CFTPThread *	GetByIndex( int iIndex );
	void			DeleteByHandle( HANDLE hHandle );
	int				GetCount( void );
};

class CFTPServer
{
	CFTPThreadManager	m_threadMgr;
	CFTPUserEntryList	m_userMgr;

	bool				m_bShutdown;
	bool				m_bRunning;

	bool				m_bCfgIP;


	SOCKET				m_sockListen;
	HANDLE				m_hServerThread;

	static DWORD	WINAPI ThreadHandlerFunc( LPVOID pParam );
	DWORD			ThreadHandler( void );

	CallbackExec		m_callbackExec;
	CallbackLogon		m_callbackLogon;
	CallbackLogoff		m_callbackLogoff;

	PVOID				m_ctxExec;
	PVOID				m_ctxLogon;
	PVOID				m_ctxLogoff;


public:
	WORD				m_wPort;
	CStdString			m_sAnonRoot;
	bool				m_bAllowAnon;

	bool	AllowAnon( void ) { return m_bAllowAnon; };
	CFTPServer( WORD wPort = 21 );
	void	DeleteThreadByHandle( HANDLE hID );
	void	Configure( CXMLNode * pNode );
	void	Configure( LPCTSTR szFile );
	HRESULT	Start( void );
	HRESULT Stop( void );
	HRESULT ValidateLogon( LPCTSTR szName, LPCTSTR szPassword, CStdString & sRoot );
	int		GetUserCount( void ) { return m_threadMgr.GetCount(); };
	bool	GetUserInfo( int iIndex, CStdString & sName, CStdString & sPassword, CStdString & sCurDir, CStdString & sIP );

	void	SetCallbackExec( CallbackExec cbExec, PVOID pCtx );
	void	SetCallbackLogon( CallbackLogon cbLogon, PVOID pCtx );
	void	SetCallbackLogoff( CallbackLogoff cbLogoff, PVOID pCtx );
	void	FireLogon( void );
	void	FireLogoff( void );
	HRESULT FireExec( LPCTSTR szPath, LPCTSTR szCmdLine );
};





#endif //  __FTP_MXM_H__

