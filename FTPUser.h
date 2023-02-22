

#ifndef __XFTPUSER_H__
#define __XFTPUSER_H__

// class CFTPServer;

class CFTPUser
{
public:	CFTPUser( SOCKET sock,CFTPServer * pSrv );	~CFTPUser();#if 0public:	CFTPUser(SOCKET sock, CFTPServer *pSrv);	~CFTPUser ();	DWORD Serve ();	void SetID (HANDLE h);	HANDLE GetID ();	DWORD GetFilePassive ();	char m_name[32];	char m_pw[32];	char m_ip[32];	char m_root[256];	char m_curdir[256];	char m_scratch[512];	char m_remoteip[20];	short m_remoteport;	bool m_bCfgIP;	unsigned int m_h1, m_h2, m_h3, m_h4;	CFTPServer *m_pSrv;private:	SOCKET m_sockCmd, m_sockData; // Command and Data sockets.  Command comes from the ctor, data is created when needed.	bool m_bLoggedOn;	bool m_bPassive;	bool m_bCanDelete;	short m_passivePort;	HANDLE m_pasvSvr;	SOCKET m_pasvSock;	bool m_bRest;  // Are we resuming?	unsigned int m_nRest; // where?	HANDLE m_ID;	char m_rename[256];	// Helper methods.  Getting the sockets and stuff.	DWORD DoLogin ();	void DoWelcome ();	DWORD GetCommand (char **cmd, char **arg);	void DoCommand (char *cmd, char *arg);	void DoLogout ();	DWORD SendData (char *, int);	DWORD SendFile (char *filename, unsigned int iStartAt);	DWORD GetFile (char *filename, bool bAppend);	char *GetCurrentDir ();	char *GetDirectory (bool bShort = false);	void TranslateDir (char *);		// Transfer functions...  Sending data across a socket.	DWORD Send (SOCKET sock, char *buff, int len);	DWORD Recv (SOCKET sock, char *buff, int len);		void Msg (int val, char *buf);	// Here are the commands!	void doACCT (char *arg);	void doCDUP (char *arg);	void doSMNT (char *arg);	void doREIN (char *arg);	void doQUIT (char *arg);	void doPORT (char *arg);	void doPASV (char *arg);	void doTYPE (char *arg);	void doSTRU (char *arg);	void doMODE (char *arg);	void doRETR (char *arg);	void doSTOR (char *arg);	void doSTOU (char *arg);	void doAPPE (char *arg);	void doALLO (char *arg);	void doREST (char *arg);	void doRNFR (char *arg);	void doRNTO (char *arg);	void doABOR (char *arg);	void doDELE (char *arg);	void doRMD (char *arg);	void doMKD (char *arg);	void doPWD (char *arg);	void doCWD (char *arg);	void doLIST (char *arg);	void doNLST (char *arg);	void doSITE (char *arg);	void doSYST (char *arg);	void doSTAT (char *arg);	void doHELP (char *arg);	void doNOOP (char *arg);	void doEXEC (char *arg);#endif};



#endif
