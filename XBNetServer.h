#pragma once

#include "XBNetSessionManager.h"
#include "xmltree.h"


class CXBNetServer
{
public:
	CXBNetServer(void);
	virtual ~CXBNetServer(void);

// Access Functions
	// Returns the number of users currently online
	int	GetNumberUsersOnline( void )	{ return m_iCurrentNumberOfUsers;	};

	// Returns the number of users currently allowed to log in
	int GetMaxUsersAllowed( void )		{ return m_iMaxUsersAllowed;		};

	// Returns port used to listen for incoming connections
	unsigned short GetPort( void )		{ return m_wPort;					};

protected:
	// TCPIP Port used to listen for new connections on
	unsigned short	m_wPort;
	// If true, indicates that the FTP Server has successfully started up
	bool			m_bServerRunning;
	// The maximum number of users allowed to be logged in at one time
	int				m_iMaxUsersAllowed;
	// The total number of users currently online at this moment
	int				m_iCurrentNumberOfUsers;
	// Winsock Startup Structure for reference
	WSADATA			m_WSAData;

	CXBNetSessionManager m_sessionMgr;
public:
	// Cranks up the server, if not already running
	HRESULT Start(void);
	// Shuts down the FTP server, useful to reconfigure the server
	HRESULT Stop(void);
protected:
	// Callback for main thread
	static DWORD WINAPI ThreadHandlerProc(LPVOID pParam);
	// Handles the actual thread, relative to the object as a member
	DWORD ThreadHandler(void);
	// The handle to the main thread handling incoming sessions
	HANDLE m_hServerThread;
	// Indicates to the main server thread that it should shutdown
	bool m_bShutdown;
	// Socket used to listen for new connections on
	SOCKET			m_sockListen;
	virtual HRESULT SpawnSession(SOCKET sockSession);
	virtual HRESULT CreateSession( SOCKET sockSession, struct sockaddr * addrSession );
	virtual CXBNetSession * NewSession( SOCKET sockSession );
public:
	// Configure using an XML-based binary tree node
	virtual void Configure(CXMLNode * pNode);
};
