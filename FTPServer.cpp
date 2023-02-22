#include <xtl.h>
#include "ftpserver.h"


#define DEBUGOUT(s) OutputDebugString(s)


DWORD WINAPI ThreadServer (LPVOID lpParameter)
{
	CXBFTPServer *pThis = (CXBFTPServer *)lpParameter;
	return pThis->Serve ();
}

DWORD WINAPI ThreadUser (LPVOID lpParameter)
{
    CXBFTPUser *pThis = (CXBFTPUser *)lpParameter;

    DWORD n = pThis->Serve();

    if (pThis->m_pSrv->m_logoff != NULL)
	{
        (pThis->m_pSrv->m_logoff)(pThis->m_pSrv->m_logoffThis);
	}
    delete pThis;

    return n;
}

CFTPUserFile::CFTPUserFile(char *szName, char *szPass, char *szRoot){    if (szName != NULL)	{	    strcpy (m_name, szName);		strcpy (m_password, szPass);		if (szRoot != NULL)		{			strcpy (m_root, szRoot);		}	    else		{		    strcpy (m_root, "");		}	}};
CFTPUserList::CFTPUserList (CXBFTPUser *ftpUser, HANDLE h, CFTPUserList *ftpUserList ) : 	pUser (ftpUser), 	pNext (ftpUserList),	hUserThread (h) {}CFTPUserList::~CFTPUserList (){}void CFTPUserList::RemoveNext (){    CFTPUserList *p = pNext;    pNext = pNext->pNext;    delete p;}

CFTPUserMgr::CFTPUserMgr () : 
	m_pList(NULL) 
{
}

CFTPUserMgr::~CFTPUserMgr()
{
	if ( m_pList )
	{
		delete 	m_pList;
	}
}
void CFTPUserMgr::Add (CXBFTPUser *pUser, HANDLE hHandle){    CFTPUserList *pUserList = new CFTPUserList (pUser, hHandle, m_pList);    m_pList = pUserList;}void CFTPUserMgr::DeleteByHandle (HANDLE hHandle){	CFTPUserList * pList = m_pList;    if ( m_pList->hUserThread == hHandle )	{		m_pList = m_pList->pNext;		delete pList;	}	else	{		while ((pList->pNext != NULL) && (pList->pNext->hUserThread != hHandle))		{			pList = pList->pNext;		}		if (pList->pNext != NULL)		{			pList->RemoveNext();		}	}}int CFTPUserMgr::GetCount (void){    CFTPUserList *pList = m_pList;    int n = 0;    while (pList != NULL)    {        n++;        pList = pList->pNext;    }    return n;}CXBFTPUser * CFTPUserMgr::GetUser (int iIndex){    CFTPUserList *pList = m_pList;    int n = 0;    while ((pList != NULL) && (n != iIndex))    {        pList = pList->pNext;        n++;    }    if (pList == NULL)	{        return NULL;	}    return pList->pUser;}

void	CXBFTPServer::SetEXECallback( CallbackExec cb, PVOID pv ){	m_exec = cb;	m_execThis = pv;}void	CXBFTPServer::SetLogoffCallback( CallbackLogoff cb, PVOID pv ){	m_logoff = cb;	m_logoffThis = pv;}void	CXBFTPServer::SetLogonCallback( CallbackLogon cb, PVOID pv ){	m_logon = cb;	m_logonThis = pv;}

DWORD	CXBFTPServer::StartServer( void ){	DWORD dwReturn = 0;	return dwReturn;}DWORD	CXBFTPServer::StopServer( void ){	DWORD dwReturn = 0;	return dwReturn;}//DWORD	CXBFTPServer::Serve( void )//{//	DWORD dwReturn = 0;//	return dwReturn;//}
DWORD	CXBFTPServer::ValidateLogin( char *szUser, char *szPass, char *szRoot ){    if ((szUser == NULL) || (szPass == NULL) || (szRoot == NULL))
        return E_FAIL;

    if (m_bAllowAnon && (strcmp (szUser, "anonymous") == 0))
    {
        strcpy (szRoot, m_AnonRoot);
        if (m_logon != NULL)
            (m_logon)(m_logonThis);
        return S_OK;
    }

    int idx = 0;
    while ((idx < m_nFileUsers) && (strcmp (szUser, m_FileUsers[idx].m_name) != 0))
        idx++;

    if (idx >= m_nFileUsers)
        return E_FAIL;
    
    if (strcmp (szPass, m_FileUsers[idx].m_password) != 0)
        return E_FAIL;

    strcpy (szRoot, m_FileUsers[idx].m_root);
    if (m_logon != NULL)
        (m_logon)(m_logonThis);
    return S_OK;
}CXBFTPServer::~CXBFTPServer(){	StopServer();	::WSACleanup();}
CXBFTPServer::CXBFTPServer( char * szUserFile, unsigned short wPort ) :	m_bRunning(false),	m_bShutdown(false),	m_port(wPort),	m_logoff(NULL),	m_logon(NULL){	WSADATA WSAData;	::WSAStartup( MAKEWORD(2,2), &WSAData );	m_FileUsers[0] = CFTPUserFile( "xbox", "xbox", "" );		m_FileUsers[1] = CFTPUserFile( "games", "games", "\\e\\games" );	m_nFileUsers = 2;	m_bAllowAnon = true;	strcpy( m_AnonRoot,"c:");}

DWORD CXBFTPServer::Start()
    // starts the server
{

	DEBUGOUT("Starting Server");
    if (m_bRunning)
        return S_OK;
    m_bRunning = true;

    struct sockaddr_in addr;
    int size = sizeof (struct sockaddr_in);

	DEBUGOUT("Creating listensock");
    m_sockListen = socket (AF_INET, SOCK_STREAM, 0);
    if (m_sockListen == -1)
        return E_FAIL;

    ZeroMemory (&addr, size);
    addr.sin_family = AF_INET;
    addr.sin_port = htons (m_port);
    addr.sin_addr.s_addr = htonl (INADDR_ANY);
	DEBUGOUT("binding");
    int ret = bind (m_sockListen, (struct sockaddr *)&addr, size);

    if (ret != 0)
	{
		DEBUGOUT("Bad bind.");

        return E_FAIL;
	}

    ret = listen (m_sockListen, MAXUSERS);
	DEBUGOUT("Listened...");
    if (ret != 0)
        return E_FAIL;

    m_hThreadServer = CreateThread (NULL, 0, ThreadServer, this, 0, NULL);
	return S_OK;
}

DWORD CXBFTPServer::Stop ()
    // stops the server
{
    if (!m_bRunning)
        return S_OK;

    m_bShutdown = true;

    return S_OK;
}

DWORD CXBFTPServer::Serve ()
  // must be run in it's own thread.  Started by call to .Start ()
{
    SOCKET newSock;
    struct sockaddr_in *addr = NULL;
    int size = sizeof (struct sockaddr_in);

    while (!m_bShutdown) 
    {
        newSock = accept (m_sockListen, (struct sockaddr *)addr, &size);
        if (newSock != -1)
        {
            struct sockaddr_in add2;
            size = sizeof (add2);
            CXBFTPUser *pNew = new CXBFTPUser (newSock, this);
            // Let's get the IP.
            if (getpeername (newSock, (struct sockaddr *)&add2, &size) == -1)
            {
                char xxx[1024];
                wsprintf (xxx, "getpeername failed - %d\n", WSAGetLastError());
                OutputDebugString (xxx);
            }
//#ifdef _XBOX
            wsprintf (pNew->m_ip, "%d.%d.%d.%d", add2.sin_addr.S_un.S_un_b.s_b1, add2.sin_addr.S_un.S_un_b.s_b2, add2.sin_addr.S_un.S_un_b.s_b3, add2.sin_addr.S_un.S_un_b.s_b4);
//#else
//            strcpy (pNew->m_ip, inet_ntoa (*(struct in_addr *)&add2.sin_addr));
//#endif
            HANDLE hThread = CreateThread (NULL, 0, ThreadUser, pNew, CREATE_SUSPENDED, NULL);
            m_Users.Add (pNew, hThread);
            pNew->SetID (hThread);
            ::ResumeThread (hThread);
        }
    }

    return S_OK;
}

int CXBFTPServer::GetUserCount ()
{
    return m_Users.GetCount ();
}

void CXBFTPServer::GetUserInfo (int idx, char **ppszUserName, char **ppszPass, char **ppszCurDir, char **ppszIP)
{
//  Let's get the User.
    CXBFTPUser *u = m_Users.GetUser (idx);
    static char blockedpw[] = "*******";
    static char cd[1024];

    if (u == NULL) 
        return;
    *ppszUserName = u->m_name;
    if (strcmp (*ppszUserName, "anonymous") != 0)
        *ppszPass = blockedpw;
    else
        *ppszPass = u->m_pw;

    strcpy (cd, u->m_root);
    if (strlen (u->m_curdir) == 0)
        strcat (cd, "\\");
    else
        strcat (cd, u->m_curdir);

    *ppszCurDir = cd;
    *ppszIP = u->m_ip;
}

#if 0



void CXBFTPServer::GetUserInfo (int idx, char **ppszUserName, char **ppszPass, char **ppszCurDir, char **ppszIP)
{
//  Let's get the User.
    CXBFTPUser *u = m_Users.GetUser (idx);
    static char blockedpw[] = "*******";
    static char cd[1024];

    if (u == NULL) 
        return;
    *ppszUserName = u->m_name;
    if (strcmp (*ppszUserName, "anonymous") != 0)
        *ppszPass = blockedpw;
    else
        *ppszPass = u->m_pw;

    strcpy (cd, u->m_root);
    if (strlen (u->m_curdir) == 0)
        strcat (cd, "\\");
    else
        strcat (cd, u->m_curdir);

    *ppszCurDir = cd;
    *ppszIP = u->m_ip;
}
#endif
