
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

	char	m_name[32];
	DWORD	DoLogin ();


class CFTPUserFile

class CFTPUserList
class CFTPUserMgr
{
private:
	CFTPUserList	* m_pList;
public:
	CFTPUserMgr ();
	~CFTPUserMgr();
    void Add (CXBFTPUser *pUser, HANDLE hHandle);

class CXBFTPServer


#endif