
#ifndef __FTPSERVER_H__
#define __FTPSERVER_H__


#define MAXUSERS (256)
#define MAXACCOUNTS (8)

class CXBFTPServer;
class CFTPUserList;
class CFTPUserMgr;

typedef HRESULT (*CallbackExec) (PVOID pThis, LPCTSTR szPath, LPCTSTR szCmdLine);
typedef void (*CallbackLogon) (PVOID pThis);
typedef void (*CallbackLogoff) (PVOID pThis);


class CXBFTPUser
{
public:
	CXBFTPUser(SOCKET sock, CXBFTPServer *pSrv);
	~CXBFTPUser();

	char	m_name[32];	char	m_pw[32];	char	m_ip[32];	char	m_root[256];	char	m_curdir[256];	char	m_scratch[512];	char	m_remoteip[20];	WORD	m_remoteport;	bool	m_bCfgIP;	unsigned int m_h1;	unsigned int m_h2;	unsigned int m_h3;	unsigned int m_h4;	CXBFTPServer *m_pSrv;	bool	m_bLoggedOn;	bool	m_bPassive;	bool	m_bCanDelete;	WORD	m_passivePort;	bool	m_bRest;	unsigned int m_nRest;	char	m_rename[256];	HANDLE	m_pasvSvr;	HANDLE	m_ID;	SOCKET	m_pasvSock;	SOCKET	m_sockCmd;	SOCKET	m_sockData;	DWORD	Serve (void);	void	SetID (HANDLE h);	HANDLE	GetID (void);	DWORD	GetFilePassive ();
	DWORD	DoLogin ();	void	DoWelcome ();	DWORD	GetCommand (char **cmd, char **arg);	void	DoCommand (char *cmd, char *arg);	void	DoLogout ();	DWORD	SendData (char *, int);	DWORD	SendFile (char *filename, unsigned int iStartAt);	DWORD	GetFile (char *filename, bool bAppend);	char *	GetCurrentDir ();	char *	GetDirectory (bool bShort = false);	void	TranslateDir (char *);	DWORD	Send (SOCKET sock, char *buff, int len);	DWORD	Recv (SOCKET sock, char *buff, int len);		void Msg (int val, char *buf);	void doACCT (char *arg);	void doCDUP (char *arg);	void doSMNT (char *arg);	void doREIN (char *arg);	void doQUIT (char *arg);	void doPORT (char *arg);	void doPASV (char *arg);	void doTYPE (char *arg);	void doSTRU (char *arg);	void doMODE (char *arg);	void doRETR (char *arg);	void doSTOR (char *arg);	void doSTOU (char *arg);	void doAPPE (char *arg);	void doALLO (char *arg);	void doREST (char *arg);	void doRNFR (char *arg);	void doRNTO (char *arg);	void doABOR (char *arg);	void doDELE (char *arg);	void doRMD (char *arg);	void doMKD (char *arg);	void doPWD (char *arg);	void doCWD (char *arg);	void doLIST (char *arg);	void doNLST (char *arg);	void doSITE (char *arg);	void doSYST (char *arg);	void doSTAT (char *arg);	void doHELP (char *arg);	void doNOOP (char *arg);	void doEXEC (char *arg);};


class CFTPUserFile{public:    char m_name[32];    char m_password[32];    char m_root[256];		CFTPUserFile(char *szName=NULL, char *szPass=NULL, char *szRoot=NULL);};

class CFTPUserList{public:    CXBFTPUser *		pUser;    HANDLE			hUserThread;    CFTPUserList *	pNext;    CFTPUserList (CXBFTPUser *ftpUser = NULL, HANDLE h = NULL, CFTPUserList *ftpUserList = NULL);    ~CFTPUserList ();    void RemoveNext ();};
class CFTPUserMgr
{
private:
	CFTPUserList	* m_pList;
public:
	CFTPUserMgr ();
	~CFTPUserMgr();
    void Add (CXBFTPUser *pUser, HANDLE hHandle);	void DeleteByHandle (HANDLE hHandle);    int GetCount (void);    CXBFTPUser *GetUser (int iIndex);};

class CXBFTPServer{private:	bool	m_bShutdown;	bool	m_bAllowAnon;	char	m_AnonRoot[256];	WORD	m_port;	void *	pUsers;	SOCKET	m_sockListen;	HANDLE	m_hThreadServer;	CFTPUserFile	m_FileUsers[MAXACCOUNTS];	int	m_nFileUsers;public:	CFTPUserMgr m_Users;	bool	m_bRunning;	bool	m_bCfgIP;	unsigned int	m_h1;	unsigned int	m_h2;	unsigned int	m_h3;	unsigned int	m_h4;	void *	m_logonThis;	void *	m_logoffThis;	void *	m_execThis;	CallbackExec m_exec;	CallbackLogon m_logon;	CallbackLogoff m_logoff;	void	SetEXECallback( CallbackExec cb, PVOID pv );	void	SetLogoffCallback( CallbackLogoff cb, PVOID pv );	void	SetLogonCallback( CallbackLogon cb, PVOID pv );	DWORD	Start();	DWORD	Stop();	DWORD	StartServer( void );	DWORD	StopServer( void );	DWORD	Serve( void );	DWORD	ValidateLogin( char *szUser, char *szPass, char *szRoot );	CXBFTPServer( char * szUserFile = NULL, WORD wPort = 21 );	~CXBFTPServer();	int	GetUserCount();	void GetUserInfo( int iIndex, char **ppszUserName, char **szPass, char **ppszCurDir, char **ppszIP);};


#endif
