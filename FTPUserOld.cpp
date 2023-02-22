

#include <xtl.h>
#include <stdio.h>
#include "ftpServer.h"


CXBFTPUser::CXBFTPUser(SOCKET sock, CXBFTPServer *pSrv) : 	m_bLoggedOn(false),	m_bPassive(false),	m_bCanDelete(false),	m_bRest(false), 	m_nRest(0),//	m_pasvSock(-1),	m_pSrv(pSrv){//	m_sockCmd = sock;}


#if 0

CXBFTPUser::CXBFTPUser(SOCKET sock, CXBFTPServer *pSrv) : 	m_sockCmd (sock), 	m_pSrv (pSrv),	m_bLoggedOn (false),	m_bPassive (false),	m_bCanDelete (false),	m_bRest (false), 	m_nRest (0),	m_pasvSock (-1){	m_curdir[0] = 0;	m_bCfgIP = m_pSrv->m_bCfgIP;	if (m_bCfgIP)	{		m_h1 = m_pSrv->m_h1;		m_h2 = m_pSrv->m_h2;		m_h3 = m_pSrv->m_h3;		m_h4 = m_pSrv->m_h4;	}}CXBFTPUser::~CXBFTPUser (){	if (m_pasvSock != -1)	{		shutdown (m_pasvSock, 2);	}	shutdown (m_sockCmd, 2);}
DWORD CXBFTPUser::Serve ()
{
    DoLogin ();
    DoWelcome ();
    while (m_bLoggedOn)
    {
		char *cmd, *arg;
		if (GetCommand (&cmd, &arg) == S_OK)
		{
			DoCommand (cmd, arg);
		    HeapFree (GetProcessHeap (), 0, cmd);
		    HeapFree (GetProcessHeap (), 0, arg);
		}
		else
		{
			m_bLoggedOn = false;
		}
    }
    DoLogout ();
    return S_OK;
}

DWORD CXBFTPUser::Send (SOCKET sock, char *buff, int len)
{
    int nBytes = 0;
    while (true)  // We must return to finish...
    {
        nBytes = send (sock, buff, len, 0);
        if (nBytes < 0)
            return E_FAIL;
        if (nBytes < len)
        {
            // We've not transmitted everything.  We must inc the pointer.
            buff += nBytes;
            len  -= nBytes;
        }
        else 
            return S_OK;
    }
}

DWORD CXBFTPUser::GetFile (char *filename, bool bAppend)
{
    if (!m_bPassive)
    {
        SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;

        addr.sin_family = AF_INET;
        addr.sin_port = htons (m_remoteport);
        addr.sin_addr.s_addr = inet_addr (m_remoteip);

        if (connect (sock, (struct sockaddr *)&addr, sizeof (struct sockaddr)) == -1)
            return E_FAIL;

        char buff[4096];
        HANDLE hFile;
        if (!bAppend)
            hFile = ::CreateFile (filename, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        else
        {
            hFile = ::CreateFile (filename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            SetFilePointer (hFile, 0, 0, FILE_END);
        }
        if (hFile == INVALID_HANDLE_VALUE)
        {
            Msg (450, "Cannot access file.");
        }
        int nBytes = recv (sock, buff, 4096, 0);
        while ((nBytes != 0) && (nBytes != -1))
        {
            unsigned long nBytesW = 0, nBytesTotal = 0;
            while ((int)nBytesTotal != nBytes)
            {
                BOOL bWriteSucc = WriteFile (hFile, buff, nBytes, &nBytesW, NULL);
                if (bWriteSucc == 0)
                {
                    Msg (450, "Cannot access file.");
                    return E_FAIL;
                }
                nBytesTotal += nBytesW;
            }
            nBytes = recv (sock, buff, 4096, 0);
        }
        CloseHandle (hFile);
        if (nBytes == 0)
        {
            Msg (226, "Transfer Complete.");
        }
        else
        {
            Msg (426, "Connection closed; transfer aborted.");
            return E_FAIL;
        }
        shutdown (sock, 2);
    }
    else
    {
        // Do passive here.
        if (m_pasvSock != -1)
        {
            // Get the file.
            HANDLE hFile;
            char buff[4096];
            if (!bAppend)
                hFile = ::CreateFile (filename, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
            else
            {
                hFile = ::CreateFile (filename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                SetFilePointer (hFile, 0, 0, FILE_END);
            }
            if (hFile == INVALID_HANDLE_VALUE)
            {
                Msg (450, "Cannot access file.");
            }
            int nBytes = recv (m_pasvSock, buff, 4096, 0);
            while ((nBytes != 0) && (nBytes != -1))
            {
                unsigned long nBytesW = 0, nBytesTotal = 0;
                while ((int)nBytesTotal != nBytes)
                {
                    BOOL bWriteSucc = WriteFile (hFile, buff, nBytes, &nBytesW, NULL);
                    if (bWriteSucc == 0)
                    {
                        Msg (450, "Cannot access file.");
                        return E_FAIL;
                    }
                    nBytesTotal += nBytesW;
                }
                nBytes = recv (m_pasvSock, buff, 4096, 0);
            }
        }
        shutdown (m_pasvSock, 2);
        SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr, *addr2 = NULL;
        int size = sizeof (struct sockaddr_in);

        ZeroMemory (&addr, size);
        addr.sin_family = AF_INET;
        addr.sin_port = htons (m_passivePort);
        addr.sin_addr.s_addr = htonl (INADDR_ANY);
        int ret = bind (sock, (struct sockaddr *)&addr, size);
        
        if (ret != 0)
            return E_FAIL;

        ret = listen (sock, MAXUSERS);
        if (ret != 0)
            return E_FAIL;

        m_pasvSock = accept (sock, (struct sockaddr *)addr2, &size);
        shutdown (sock, 2);
    }
    return S_OK;
}
DWORD CXBFTPUser::SendFile (char *filename, unsigned int iStartAt)
{
    if (!m_bPassive)
    {
        SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;

        addr.sin_family = AF_INET;
        addr.sin_port = htons (m_remoteport);
        addr.sin_addr.s_addr = inet_addr (m_remoteip);
        if (connect (sock, (struct sockaddr *)&addr, sizeof (struct sockaddr)) == -1)
            return E_FAIL;

        char buff[4096];
        HANDLE hFile;
        DWORD dwRead;
        hFile = ::CreateFile (filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        SetFilePointer (hFile, iStartAt, NULL, FILE_BEGIN);
        BOOL bReadSucc = ReadFile (hFile, buff, 4096, &dwRead, NULL);
        while (bReadSucc && (dwRead != 0))
        {
            Send (sock, buff, dwRead);
            bReadSucc = ReadFile (hFile, buff, 4096, &dwRead, NULL);
        }
        shutdown (sock, 2);
    }
    else
    {
        // Do passive here.
        if (m_pasvSock != -1)
        {
            char buff[4096];
            HANDLE hFile;
            DWORD dwRead;
            hFile = ::CreateFile (filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            SetFilePointer (hFile, iStartAt, NULL, FILE_BEGIN);
            BOOL bReadSucc = ReadFile (hFile, buff, 4096, &dwRead, NULL);
            while (bReadSucc && (dwRead != 0))
            {
                Send (m_pasvSock, buff, dwRead);
                bReadSucc = ReadFile (hFile, buff, 4096, &dwRead, NULL);
            }
        }
        shutdown (m_pasvSock, 2);

        SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr, *addr2 = NULL;
        int size = sizeof (struct sockaddr_in);

        ZeroMemory (&addr, size);
        addr.sin_family = AF_INET;
        addr.sin_port = htons (m_passivePort);
        addr.sin_addr.s_addr = htonl (INADDR_ANY);
        int ret = bind (sock, (struct sockaddr *)&addr, size);
        
        if (ret != 0)
            return E_FAIL;

        ret = listen (sock, MAXUSERS);
        if (ret != 0)
            return E_FAIL;

        m_pasvSock = accept (sock, (struct sockaddr *)addr2, &size);
        shutdown (sock, 2);
    }
    return S_OK;
}

DWORD CXBFTPUser::SendData (char *buff, int len)
{
    if (!m_bPassive)
    {
        SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;

        addr.sin_family = AF_INET;
        addr.sin_port = htons (m_remoteport);
        addr.sin_addr.s_addr = inet_addr (m_remoteip);
        if (connect (sock, (struct sockaddr *)&addr, sizeof (struct sockaddr)) == -1)
            return E_FAIL;
        Send (sock, buff, len);
        shutdown (sock, 2);
    }
    else
    {
        // Do something here...
        if (m_pasvSock != -1)
        {
            Send (m_pasvSock, buff, len);
        }
        shutdown (m_pasvSock, 2);

        SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr, *addr2 = NULL;
        int size = sizeof (struct sockaddr_in);

        ZeroMemory (&addr, size);
        addr.sin_family = AF_INET;
        addr.sin_port = htons (m_passivePort);
        addr.sin_addr.s_addr = htonl (INADDR_ANY);
        int ret = bind (sock, (struct sockaddr *)&addr, size);
        
        if (ret != 0)
            return E_FAIL;

        ret = listen (sock, MAXUSERS);
        if (ret != 0)
            return E_FAIL;

        m_pasvSock = accept (sock, (struct sockaddr *)addr2, &size);
        shutdown (sock, 2);
    }
    return S_OK;
}

DWORD CXBFTPUser::Recv (SOCKET sock, char *buff, int len)
{
    int nBytes = 0;  // Look familiar?
    while (true)  // We must return to finish...
    {
        nBytes = recv (sock, buff, len, 0);
        if (nBytes < 0)
            return E_FAIL;
        if (nBytes > 0)
        {
            buff[len] = '\0';
            return S_OK;
        }
        if (nBytes == 0)
        {
            // Hmmm...  Nothing.....  Let's error for now.  The connection has probably been closed.  We'll see if this really happens.
            m_bLoggedOn = false;
            return E_FAIL;
        }
    }
}

void CXBFTPUser::Msg (int val, char *buf)
{
    char bbb[256];
    wsprintf (bbb, "%d %s\x0D\x0A", val, buf);
    Send (m_sockCmd, bbb, strlen (bbb));
}

void CXBFTPUser::DoWelcome ()
{
    char msg[] = "Welcome to MXM's FTP server";
    Msg (230, msg);
}

DWORD CXBFTPUser::DoLogin ()
{
    char msg[] = "MXM FTP ready.";
    Msg (220, msg);
	char *cmd, *arg;
    if (GetCommand (&cmd, &arg) != S_OK)
	{
		Msg (503, "Something is messed up.");
		return E_FAIL;
	}
//  Now we need to check the buffer...  Is it a USER command?
    // First let's extract the command (should be USER)
    if (strcmp(cmd, "USER") != 0)
    {
        Msg (503, "Bad sequence of commands.");
        return E_FAIL;
    }
	strcpy (m_name, arg);
    HeapFree (GetProcessHeap (), 0, cmd); HeapFree (GetProcessHeap (), 0, arg);
    // Send that it's cool to get PW
    if (strcmp (m_name, "anonymous") == 0)
        Msg (331, "Guest login ok, send email as password.");
    else
        Msg (331, "Send password.");
    // Now we get a PW.
    if (GetCommand (&cmd, &arg) != S_OK)
	{
		Msg (503, "Something is messed up.");
        return E_FAIL;
	}
    if (strcmp(cmd, "PASS") != 0)
    {
        Msg (503, "Bad sequence of commands.");
        return E_FAIL;
    }
    // put the Pass into the buffer...
    strcpy (m_pw, arg);
    HeapFree (GetProcessHeap (), 0, cmd); HeapFree (GetProcessHeap (), 0, arg);

    // Validate the name/pw and get the root back.
    if (m_pSrv->ValidateLogin (m_name, m_pw, m_root) != S_OK)
    {
        //  We're not logged in.  Try again.  
        Msg (421, "Unknown user or bad password.");
        return E_FAIL;
    }
    m_bLoggedOn = true;

    return S_OK;
}

void CXBFTPUser::DoLogout ()
{
    // We're going to end.  Let's clean up...
    shutdown (m_sockCmd, 2);
    // The connection is closed.  Let's tell the usermanager to delete us.
    m_pSrv->m_Users.DeleteByHandle (GetID ());
}

DWORD CXBFTPUser::GetCommand (char **cmd, char **arg)
{
    char bbb[256];
    ZeroMemory (bbb, 256);
    if (Recv (m_sockCmd, bbb, 255) != S_OK)
        return E_FAIL;

    char cSpace[] = " ";
    char cEOL[] = "\x0D\x0A";
    char *tempCmd = strtok (bbb, cSpace);
    char *tempArg = strtok (NULL, cEOL);
	if ((tempArg == NULL) || (strlen(tempArg) == 0))
		tempCmd = strtok (tempCmd, cEOL);
	for (unsigned int i = 0; i < strlen (tempCmd); i++)
		tempCmd[i] = toupper (tempCmd[i]);
	int iLen = strlen (tempCmd);
	*cmd = (char *)HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, iLen + 1);
	strcpy (*cmd, tempCmd);
	if (tempArg != NULL)
	{
		*arg = (char *)HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, strlen (tempArg) + 1);
		strcpy (*arg, tempArg);
	}
	else
		*arg = (char *)HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, 1);

	return S_OK;
}

void CXBFTPUser::DoCommand (char *cmd, char *arg)
{
	if (strcmp (cmd, "PORT") == 0)
		doPORT (arg);
	else if ((strcmp (cmd, "PWD") == 0) || (strcmp (cmd, "XPWD") == 0))
		doPWD (arg);
    else if (strcmp (cmd, "EXEC") == 0)
        doEXEC (arg);
	else if (strcmp (cmd, "NOOP") == 0)
        doNOOP (arg);
	else if (strcmp (cmd, "CDUP") == 0)
        doCDUP (arg);
	else if (strcmp (cmd, "QUIT") == 0)
        doQUIT (arg);
	else if (strcmp (cmd, "PASV") == 0)
        doPASV (arg);
	else if (strcmp (cmd, "TYPE") == 0)
        doTYPE (arg);
	else if (strcmp (cmd, "STRU") == 0)
        doSTRU (arg);
	else if (strcmp (cmd, "RETR") == 0)
        doRETR (arg);
	else if (strcmp (cmd, "STOR") == 0)
        doSTOR (arg);
	else if (strcmp (cmd, "STOU") == 0)
        doSTOU (arg);
	else if (strcmp (cmd, "APPE") == 0)
        doAPPE (arg);
	else if (strcmp (cmd, "ALLO") == 0)
        doALLO (arg);
	else if (strcmp (cmd, "REST") == 0)
        doREST (arg);
	else if (strcmp (cmd, "RNFR") == 0)
        doRNFR (arg);
	else if (strcmp (cmd, "RNTO") == 0)
        doRNTO (arg);
	else if (strcmp (cmd, "ABOR") == 0)
        doABOR (arg);
	else if (strcmp (cmd, "DELE") == 0)
        doDELE (arg);
	else if (strcmp (cmd, "RMD") == 0)
        doRMD (arg);
	else if (strcmp (cmd, "MKD") == 0)
        doMKD (arg);
	else if (strcmp (cmd, "CWD") == 0)
        doCWD (arg);
	else if (strcmp (cmd, "LIST") == 0)
        doLIST (arg);
	else if (strcmp (cmd, "NLST") == 0)
        doNLST (arg);
	else if (strcmp (cmd, "SITE") == 0)
        doSITE (arg);
	else if (strcmp (cmd, "SYST") == 0)
        doSYST (arg);
	else if (strcmp (cmd, "STAT") == 0)
        doSTAT (arg);
	else if (strcmp (cmd, "HELP") == 0)
        doHELP (arg);
	else if (strcmp (cmd, "ACCT") == 0)
        doACCT (arg);
	else if (strcmp (cmd, "SMNT") == 0)
        doSMNT (arg);
	else if (strcmp (cmd, "REIN") == 0)
        doREIN (arg);
	else if (strcmp (cmd, "MODE") == 0)
        doMODE (arg);
    else
    {
        char xxx[100];
        wsprintf (xxx, "Command \"%s\" unknown.", cmd);
        Msg (500, xxx);
    }    
}

void CXBFTPUser::doPORT (char *arg)
{
    unsigned int h1, h2, h3, h4, p1, p2;
    sscanf (arg, "%u,%u,%u,%u,%u,%u", &h1, &h2, &h3, &h4, &p1, &p2);
    wsprintf (m_remoteip, "%u.%u.%u.%u", h1, h2, h3, h4);
    m_remoteport = 256*p1 + p2;
    m_bPassive = false;

	Msg (200, "PORT command successful.");
}

void CXBFTPUser::doPWD (char *arg)
{
	char bbb[256];
    if (strlen(m_curdir) == 0)
        Msg (257, "\"\\\" is the cwd.");
    else
    {
	    wsprintf (bbb, "\"%s\" is the cwd.", m_curdir);
	    Msg (257, bbb);
    }
}

char *CXBFTPUser::GetCurrentDir ()
{
    if (strlen (m_curdir) == 0)
        wsprintf (m_scratch, "%s", m_root);
    else
	    wsprintf (m_scratch, "%s%s", m_root, m_curdir);
	return m_scratch;
}

void CXBFTPUser::doNOOP (char *arg)
{
   Msg (200, "NOOP okay.");
}

void CXBFTPUser::doCDUP (char *arg)
{
        int L = strlen (m_curdir);
        for (int i=L; i>=0; i--)
        {
            if (m_curdir[i] == '\\')
            {
                m_curdir[i] = 0;
                break;
            }
        }
    Msg (250, "CWD command successful.");
}

void CXBFTPUser::doREST (char *arg)
{
    m_bRest = true;
    m_nRest = atoi (arg);
    Msg (350, "Restart okay.");
}

void CXBFTPUser::doACCT (char *arg)
{
    Msg (552, "Account information discarded.");
}

void CXBFTPUser::doSMNT (char *arg)
{
    Msg (502, "Command not implemented.");
}

void CXBFTPUser::doREIN (char *arg)
{
    Msg (220, "Service ready for new user.");
    if (DoLogin () != S_OK)
    {
        m_bLoggedOn = false;
    }
}

void CXBFTPUser::doQUIT (char *arg)
{
    m_bLoggedOn = false;
    Msg (221, "Goodbye.");
}

void CXBFTPUser::doPASV (char *arg)
{
    char buffer[128];
    // Let's get a random port above 2048
    unsigned int h1, h2, h3, h4, p1, p2;
    p1 = rand () % 252 + 4; p2 = rand () % 256;
    m_passivePort = p1*256 + p2;
    // Now let's get our IP.
	if (m_bCfgIP)
	{
		h1 = m_h1; h2 = m_h2; h3 = m_h3; h4 = m_h4;
	}
	else
	{
//#ifdef _XBOX
		XNADDR xaddr;
		in_addr iaddr;
		XNKID x;
		XNetGetTitleXnAddr(&xaddr); 
		XNetXnAddrToInAddr (&xaddr, &x, &iaddr);

		h1 = iaddr.S_un.S_un_b.s_b1; h2 = iaddr.S_un.S_un_b.s_b2; h3 = iaddr.S_un.S_un_b.s_b3; h4 = iaddr.S_un.S_un_b.s_b4;
//#else
//		// We're on a windows machine.
//		char bbb[64];
//		gethostname (bbb, 64);
//		struct hostent *h;
//		h = gethostbyname (bbb);
//		char *ip = inet_ntoa(*((struct in_addr *)h->h_addr));
//		sscanf (ip, "%d.%d.%d.%d", &h1, &h2, &h3, &h4);
//#endif
	}

    wsprintf (buffer, "Entering Passive Mode (%d,%d,%d,%d,%d,%d).",(int)h1, (int)h2, (int)h3, (int)h4, (int)p1, (int)p2);
    Msg (227, buffer);
    m_bPassive = true;

    SOCKET sock = socket (AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr, *addr2 = NULL;
    int size = sizeof (struct sockaddr_in);

    ZeroMemory (&addr, size);
    addr.sin_family = AF_INET;
    addr.sin_port = htons (m_passivePort);
    addr.sin_addr.s_addr = htonl (INADDR_ANY);
    int ret = bind (sock, (struct sockaddr *)&addr, size);
    
    ret = listen (sock, MAXUSERS);
    m_pasvSock = accept (sock, (struct sockaddr *)addr2, &size);
    shutdown (sock, 2);
}

void CXBFTPUser::doTYPE (char *arg)
{
    char bbb[256];
    if ((strcmp (arg, "A") == 0) || (strcmp(arg, "a") == 0) || (strcmp(arg, "I") == 0) || (strcmp(arg, "i") == 0))
    {
        wsprintf (bbb, "Mode is set to %s.", arg);
        Msg (200, bbb);
    }
    else
        Msg (501, "Bad parameter.");
}

void CXBFTPUser::doSTRU (char *arg)
{
    if ((strcmp (arg, "F") == 0) || (strcmp (arg, "f") == 0))
        Msg (200, "Structure set to file.");
    else
        Msg (501, "File structures only.");
}

void CXBFTPUser::doMODE (char *arg)
{
    Msg (502, "Command not implemented.");
}

void CXBFTPUser::doRETR (char *arg)
{
    Msg (150, "Opening BINARY mode data connection for file transfer.");
    char fn[1024];
    *fn = 0;
    char *dir = GetCurrentDir ();
    strcat (fn, dir);
    strcat (fn, "\\");
    strcat (fn, arg);
    TranslateDir (fn);
    SendFile (fn, m_nRest);
    Msg (226, "Transfer complete.");
    m_bRest = false; m_nRest = 0;
}

void CXBFTPUser::doSTOR (char *arg)
{
    Msg (150, "Opening BINARY mode data connection for file transfer.");
    char fn[1024];
    *fn = 0;
    char *dir = GetCurrentDir ();
    strcat (fn, dir);
    strcat (fn, "\\");
    strcat (fn, arg);
    TranslateDir (fn);
    GetFile (fn, false);
}

void CXBFTPUser::doSTOU (char *arg)
{
    Msg (502, "Command not implemented.");
}

void CXBFTPUser::doAPPE (char *arg)
{
    Msg (150, "Opening BINARY mode data connection for file transfer.");
    char fn[1024];
    *fn = 0;
    char *dir = GetCurrentDir ();
    strcat (fn, dir);
    strcat (fn, "\\");
    strcat (fn, arg);
    TranslateDir (fn);
    GetFile (fn, true);
}

void CXBFTPUser::doALLO (char *arg)
{
    doNOOP(arg);
}

void CXBFTPUser::doRNFR (char *arg)
{
    char bbb[1024];
    strcpy (bbb, GetCurrentDir ());
    strcat (bbb, "\\");
    strcat (bbb, arg);
    TranslateDir (bbb);

    HANDLE hFile = CreateFile (bbb, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == NULL)
    {
        Msg (550, "File unavailable.");
    }
    else
    {
        strcpy (m_rename, bbb);
        CloseHandle (hFile);
        Msg (350, "Next command must be RNTO.");
    }
}

void CXBFTPUser::doRNTO (char *arg)
{
    char bbb[1024];
    strcpy (bbb, GetCurrentDir ());
    strcat (bbb, "\\");
    strcat (bbb, arg);
    TranslateDir (bbb);

    if (MoveFile (m_rename, bbb))
        Msg (250, "Rename was successful.");
    else
        Msg (553, "Rename was not successful.");
}

void CXBFTPUser::doABOR (char *arg)
{
    Msg (502, "Command not implemented.");
}

void CXBFTPUser::doDELE (char *arg)
{
    if (!m_bCanDelete)
    {
        Msg (550, "Insufficient access.");
        return;
    }

    char bbb[1024];
    strcpy (bbb, GetCurrentDir ());
    strcat (bbb, "\\");
    strcat (bbb, arg);
    TranslateDir (bbb);

    if (DeleteFile (bbb))
        Msg (250, "File deleted.");
    else
        Msg (550, "Delete was not successful.");
}

void CXBFTPUser::doRMD (char *arg)
{
    char bbb[1024];
    strcpy (bbb, GetCurrentDir ());
    strcat (bbb, "\\");
    strcat (bbb, arg);
    TranslateDir (bbb);
    if (RemoveDirectory (bbb))
        Msg (257, "Directory removed successfully.");
    else
        Msg (421, "Error removing directory.");
}

void CXBFTPUser::doMKD (char *arg)
{
    char bbb[1024];

    if (arg[0] == '\\')
    {
        strcpy (bbb, m_root);
        strcat (bbb, "\\");
        strcat (bbb, arg);
    }
    else
    {
        // Get the current path.
        strcpy (bbb, GetCurrentDir ());
        strcat (bbb, "\\");
        strcat (bbb, arg);
    }
    TranslateDir (bbb);

    if (CreateDirectory (bbb, NULL))
        Msg (257, "Directory created successfully.");
    else
        Msg (421, "Error creating directory.");
}

void CXBFTPUser::doCWD (char *arg)
{
    if (arg[0] == '\\')
    {
        // absolute change.
        strcpy(m_curdir, arg + 1);
        
    }
    else if (strcmp (arg, ".") == 0)
    {
    }
    else if (strcmp (arg, "..") == 0)
    {
        doCDUP (arg);
        return;
    }
    else
    {
        strcat (m_curdir, "\\");
        strcat (m_curdir, arg);
    }
    Msg (250, "CWD command successful.");
}

void CXBFTPUser::doLIST (char *arg)
{
    Msg (150, "Opening ASCII mode data connection for list.");
    //Get the directory
    
    char *foo = GetDirectory ();
    //Send the stuff.
    SendData (foo, strlen (foo));
    HeapFree (GetProcessHeap (), 0, foo);
    Msg (226, "Listing completed.");
}

void CXBFTPUser::doNLST (char *arg)
{
    Msg (150, "Opening ASCII mode data connection for list.");

    char *foo = GetDirectory (true);
    //Send the stuff.
    SendData (foo, strlen (foo));
    HeapFree (GetProcessHeap (), 0, foo);
    Msg (226, "Listing completed.");
}

void CXBFTPUser::doSITE (char *arg)
{
    Msg (502, "Command not implemented.");
}

void CXBFTPUser::doSYST (char *arg)
{
    Msg (215, "UNIX (sorta) done by J.P. Patches");
}

void CXBFTPUser::doSTAT (char *arg)
{
    Msg (502, "Command not implemented.");
}

void CXBFTPUser::doHELP (char *arg)
{
    Msg (502, "Command not implemented.");
}

char *CXBFTPUser::GetDirectory (bool bShort)
{
    char *result = NULL;
    const char dir[] = "drwx------ ";
    const char file[] = "-rwx------ ";
    const char group[] = "1 user group ";

    // Let's get us some files.
    WIN32_FIND_DATA wfd;
    char *path = GetCurrentDir ();
    TranslateDir (path);
    if (strlen (path) == 0)
    {
        // Send over canned data.
        char foo[] = "drwx------ 1 user group              0 Oct 26 10:03 c\r\ndrwx------ 1 user group              0 Oct 26 10:03 d\r\ndrwx------ 1 user group              0 Oct 26 10:03 e\r\ndrwx------ 1 user group              0 Oct 26 10:03 f\r\n";
        result = (char *)HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, strlen (foo) + 1);
        strcpy (result, foo);
        return result;
    }
    strcat (path, "\\*");

    HANDLE hFile = FindFirstFile (path, &wfd);

    bool bDone = (hFile == INVALID_HANDLE_VALUE);
    if (bDone)
    {
        result = (char *)HeapAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY, 1);
    }
    int resultlen = 0;
    while (!bDone)
    {
        char *thisfile = (char *)HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, 1024);
        *thisfile = 0;  // start with a empty string.
        // We've got a file.
        if (!bShort)
        {
            if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                strcat (thisfile, "drwx------ ");
            else
                strcat (thisfile, "-rwx------ ");

            strcat (thisfile, "1 user group ");

            // size
            char bbb[512];
            wsprintf (bbb, "%14d ", wfd.nFileSizeLow);
            strcat (thisfile, bbb);

            SYSTEMTIME st;
            FileTimeToSystemTime (&wfd.ftCreationTime, &st);
            switch (st.wMonth)
            {
                case 1:
                    strcat(thisfile, "Jan ");
                    break;
                case 2:
                    strcat(thisfile, "Feb ");
                    break;
                case 3:
                    strcat(thisfile, "Mar ");
                    break;
                case 4:
                    strcat(thisfile, "Apr ");
                    break;
                case 5:
                    strcat(thisfile, "May ");
                    break;
                case 6:
                    strcat(thisfile, "Jun ");
                    break;
                case 7:
                    strcat(thisfile, "Jul ");
                    break;
                case 8:
                    strcat(thisfile, "Aug ");
                    break;
                case 9:
                    strcat(thisfile, "Sep ");
                    break;
                case 10:
                    strcat(thisfile, "Oct ");
                    break;
                case 11:
                    strcat(thisfile, "Nov ");
                    break;
                case 12:
                    strcat(thisfile, "Dec ");
                    break;
            }
            wsprintf (bbb, "%2d %02d:%02d ", st.wDay, st.wHour, st.wMinute);
            strcat (thisfile, bbb);

            strcat (thisfile, wfd.cFileName);
            strcat (thisfile, "\r\n");
#ifdef _DEBUG
            OutputDebugString (thisfile);
#endif
        }
        else
            if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                strcat (thisfile, wfd.cFileName);
                strcat (thisfile, "\r\n");
            }
        char *temp = NULL;
        if (result != NULL)
        {
            temp = (char *)HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, resultlen + 1);
            strcpy (temp, result);
            HeapFree (GetProcessHeap (), 0, result);
        }
        resultlen += strlen (thisfile);
        // Now let's alloc the result 'cuz HeapReAlloc is blowing chunks.
        result = (char *)HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, resultlen + 1);
        if (temp != NULL)
            strcat (result, temp);
        strcat (result, thisfile);
        HeapFree (GetProcessHeap (), 0, thisfile);
        if (temp != NULL)
            HeapFree (GetProcessHeap (), 0, temp);
        bDone = (FindNextFile (hFile, &wfd) == FALSE);
    }
    return result;
}

void CXBFTPUser::SetID (HANDLE h)
{
    m_ID = h;
}

HANDLE CXBFTPUser::GetID ()
{
    return m_ID;
}

void CXBFTPUser::TranslateDir (char *foo)
{
    if (strlen (foo) <= 1)
        return;

    if (foo[0] == '\\')
    {
        foo[0] = foo[1];
        foo[1] = ':';
    }
}

void CXBFTPUser::doEXEC (char *arg)
{
    // We need to launch the argument.
    char *path = GetCurrentDir ();
    strcat (path, "\\");
    TranslateDir (path);
    char *args = NULL;
    strtok (arg, " ");
    args = strtok (NULL, " ");
    strcat (path, arg);
    if (m_pSrv->m_exec != NULL)
        (m_pSrv->m_exec)(m_pSrv->m_execThis, path, args);
    Msg (200, "Ok.");
}

#endif
