

#ifndef __XBSOCKET_H__
#define __XBSOCKET_H__

#include "StdString.h"


extern "C" 
{
	extern void WINAPI HalInitiateShutdown(void);
//	XBOXAPI DWORD WINAPI XWriteTitleInfoAndRebootA(IN LPSTR pszLaunchPath,IN LPSTR pszDDrivePath, IN DWORD dwLaunchDataType, IN DWORD dwTitleId, IN PLAUNCH_DATA pLaunchData);
};

extern "C" 
{
	extern bool WINAPI NtSetSystemTime(LPFILETIME  SystemTime ,LPFILETIME PreviousTime );
//	extern char *XboxKrnlVersion(void);
	extern char *XboxLANKey(void);
};

void SetNewTime( SYSTEMTIME & sysTime);
void InterpretTimeResponse( LPCTSTR szBuffer, SYSTEMTIME & sysTime );
int DoClockSynchronization( LPCTSTR szTimeServer, long lTimeOffset = 0 );

CStdString GetWebFile( LPCTSTR szHost, LPCTSTR szURI, int iPort = -1 );
CStdString GetXSNewsFeed( void );



class CWinSock
{
	WSADATA m_wsaData;
	bool	m_bIsValid;
	BYTE	m_byVersionHigh;
	BYTE	m_byVersionLow;
	int		m_iError;
public:
	CWinSock( bool bInit = true, BYTE byVerHigh = 2, BYTE byVerLow = 2 );
	~CWinSock();
	int		GetError( void ) { return m_iError; };
	bool	IsValid( void ) { return m_bIsValid; };
	bool	Initialize( BYTE byVerHigh = 255, BYTE byVerLow = 255 );
};


class CXBSockAddr
{
protected:
	sockaddr_in m_sAddrIn;
	void Create( DWORD dwAddr, int iPort );
	bool		m_bIsValid;
		
public:
	CXBSockAddr( LPCTSTR szAddress, int iPort );
	CXBSockAddr( DWORD dwAddress, int iPort );
	bool	IsValid( void ) { return m_bIsValid; };
	int	Size( void ) { return sizeof(sockaddr_in); };
	operator sockaddr *() { return (sockaddr *)&m_sAddrIn; };
	int	Connect( SOCKET socket );
};

class CXBSocket_MXM
{
protected:
	SOCKET	m_socket;
	bool	m_bConnected;
public:
	CXBSocket_MXM( bool bCreateSocket = true, int iProtocol = IPPROTO_TCP );
	~CXBSocket_MXM();
	bool CreateSocket( int iProtocol = IPPROTO_TCP );
	bool Connect( CXBSockAddr & sockObj );
	bool Shutdown( void );
	bool IsConnected( void ) { return m_bConnected; };
	int	 SendString( LPCTSTR szData, int iFlags = 0 );
	int	 SendData( BYTE *pbData, int iLength, int iFlags = 0 );
	int  RecvData( BYTE *pbData, int iLength, int iFlags = 0 );
};















#endif //  __XBSOCKET_H__