




#include <xtl.h>
#include "XBSocket.h"

#include "DNSCache.h"



 
/* 

void Connect()
{
	WSADATA WsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &WsaData );
	
	char aa[5000];
	SOCKET m_socket;
	
	m_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	
	int  whathappened=WSAGetLastError ();
	wsprintf(aa, "SOCKET = %ld\n", m_socket);
	
	if (m_socket == INVALID_SOCKET)
	{
		debug("Error at socket()");
		WSACleanup();
		return;
	}
	
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr("66.218.71.198"); // yahoo.com -> "66.218.71.198"
	//service.sin_addr.s_addr = inet_addr("192.168.0.20");
	service.sin_port = htons(80);
	
	int results=connect(m_socket,(sockaddr*) &service,sizeof(struct sockaddr));
	
	send(m_socket, "GET / \r\n", strlen("GET / \r\n"), 0);
	
	int rr = 1;
	while (rr)
	{
		rr = recv(m_socket, aa, 500, 0);
	}
}

*/

CStdString GetXSNewsFeed( void )
{
	// http://www.xbox-scene.com/xbox1data/xbox-scene.xml
	return GetWebFile( _T("www.xbox-scene.com"), _T("/xbox1data/xbox-scene.xml"), 80 );
}

CStdString GetWebFile( LPCTSTR szHost, LPCTSTR szURI, int iPort )
{
	CWinSock winSock;
	CStdString sGetHeader;
	CStdString sReturn;
	int iReturn;

	if ( iPort == -1 )
	{
		iPort = 80;
	}

	if ( winSock.IsValid() )
	{
		// 192.5.41.209
		// http://tycho.usno.navy.mil/ntp.html
		// CXBSockAddr sockAddr( _T("tick.usno.navy.mil"), 123 );
		CXBSockAddr sockAddr( szHost, iPort );
		CXBSocket_MXM sockWeb;
		if ( sockAddr.IsValid()  )
		{
			if ( sockWeb.Connect( sockAddr ) )
			{
				sGetHeader.Format(_T("GET %s \r\n"), szURI);
				// Do our thing here...
				// CXBSocket_MXM will clean up afterward.
				// DoActualSynchronization();
				sockWeb.SendString( sGetHeader );

				char pbBuffer[1025];
				int iLength = sockWeb.RecvData( (BYTE*)pbBuffer, 1024 );
				while ( iLength != SOCKET_ERROR && iLength > 0 )
				{
					pbBuffer[1024] = 0;
					pbBuffer[iLength] = 0;
					sReturn += (char *)pbBuffer;
					Sleep(1000);
					iLength = sockWeb.RecvData( (BYTE*)pbBuffer, 1024 );
				}
			}
		}
	}
	else
	{
		iReturn = winSock.GetError();
	}
	return sReturn;
}


CWinSock::CWinSock( bool bInit, BYTE byVerHigh, BYTE byVerLow ) :
	m_bIsValid(false)
	,m_byVersionHigh(byVerHigh)
	,m_byVersionLow(byVerLow)
{
	ZeroMemory( &m_wsaData, sizeof(WSADATA) );
	if ( bInit )
	{
		m_bIsValid = Initialize( byVerHigh, byVerLow );
	}
}

CWinSock::~CWinSock()
{
	WSACleanup();
}

bool CWinSock::Initialize( BYTE byVerHigh, BYTE byVerLow )
{
	bool bReturn = false;

	if ( m_bIsValid )
	{
		bReturn = true;
	}
	else
	{
		if ( byVerHigh == 255 )
		{
			byVerHigh = m_byVersionHigh;
		}
		else
		{
			m_byVersionHigh = byVerHigh;
		}

		if ( byVerLow == 255 )
		{
			byVerLow = m_byVersionLow;
		}
		else
		{
			m_byVersionLow = byVerLow;
		}

		m_iError = WSAStartup(MAKEWORD(byVerHigh, byVerLow), &m_wsaData);
		if ( m_iError == 0 )
		{
			m_bIsValid = true;
			bReturn = true;
		}
	}

	return bReturn;
}



void CXBSockAddr::Create( DWORD dwAddr, int iPort )
{
	if ( dwAddr &&  (dwAddr != INADDR_NONE) )
	{
		m_sAddrIn.sin_family		= AF_INET;
		m_sAddrIn.sin_addr.s_addr	= dwAddr;
		m_sAddrIn.sin_port			= htons(iPort);

		m_bIsValid = true;
	}
}

CXBSockAddr::CXBSockAddr( DWORD dwAddress, int iPort ) :
	m_bIsValid(false)
{
	Create( dwAddress, iPort );
}


struct hostent* gethostbyname(const char* _name)
{
	//HostEnt* server = new HostEnt;

	//if( gethostname(server->name, 128) && xboxstricmp(server->name, _name) == 0 )
	//{
	//	XNADDR xna;
	//	DWORD dwState;
	//	do
	//	{
	//		dwState = XNetGetTitleXnAddr(&xna);
	//	} while (dwState==XNET_GET_XNADDR_PENDING);

	//	server->addr_list[0] = server->addr;
	//	memcpy(server->addr, &(xna.ina.s_addr), 4);
	//	server->server.h_aliases = 0;
	//	server->server.h_addrtype = AF_INET;
	//	server->server.h_length = 4;
	//	server->server.h_addr_list = new char*[4];
	//	server->server.h_addr_list[0] = server->addr_list[0];
	//	server->server.h_addr_list[1] = 0;
	//	return (struct hostent*)server;
	//}

	WSAEVENT hEvent = WSACreateEvent();
	XNDNS* pDns = NULL;
	INT err = XNetDnsLookup(_name, hEvent, &pDns);
	WaitForSingleObject(hEvent, INFINITE);
	if( pDns && pDns->iStatus == 0 )
	{
		//strcpy(server->name, _name);
		//server->addr_list[0] = server->addr;
		//memcpy(server->addr, &(pDns->aina[0].s_addr), 4);
		//server->server.h_name = server->name;
		//server->server.h_aliases = 0;
		//server->server.h_addrtype = AF_INET;
		//server->server.h_length = 4;
		//server->server.h_addr_list = new char*[4];

		//server->server.h_addr_list[0] = server->addr_list[0];
		//server->server.h_addr_list[1] = 0;
		XNetDnsRelease(pDns);
		pDns = NULL;
//		return (struct hostent*)server;
	}
	if( pDns )
		XNetDnsRelease(pDns);
//	delete server;
	return 0;
}


// We need to add DNS cacheing here....
DWORD DnsLookup( LPCTSTR szDomainName )
{

	return DNS_Lookup( szDomainName );
#if 0
	DWORD dwReturn  = INADDR_NONE;
	XNDNS * pxndns = NULL;

	gethostbyname( szDomainName );
	INT err = XNetDnsLookup( szDomainName, NULL, &pxndns);
	while (pxndns->iStatus == WSAEINPROGRESS)
	{
		// Do something else while lookup is in progress
		Sleep(1);
	}
	if (pxndns->iStatus == 0)
	{
		// Just get first address
		if ( pxndns->cina )
		{
			dwReturn = pxndns->aina[0].S_un.S_addr;
		}
		// Look at pxndns->cina for the number of addresses returned
		// Look at pxndns->aina for the vector of addresses returned
	}
	else
	{
		// An error occurred.  One of the following:
		switch( pxndns->iStatus ) // == WSAHOST_NOT_FOUND )
		{
			case WSAHOST_NOT_FOUND:
				dwReturn  = INADDR_NONE;
				break;
			case WSAETIMEDOUT:
				dwReturn  = INADDR_NONE;
				break;
		}
		// else if (pxndns->iStatus == WSAETIMEDOUT - No response from DNS server(s)
	}
	XNetDnsRelease(pxndns);
	// Don't dereference pxndns after this point
	return dwReturn;
#endif
}


CXBSockAddr::CXBSockAddr( LPCTSTR szAddress, int iPort ) :
	m_bIsValid(false)
{
	DWORD dwAddr = inet_addr(szAddress);
	if ( (dwAddr == 0) || (dwAddr == INADDR_NONE))
	{
		dwAddr = DnsLookup( szAddress );
//		XNetDnsLookup(
	}
	Create( dwAddr, iPort );
}

int	CXBSockAddr::Connect( SOCKET socket )
{
	int iReturn;

	iReturn = connect( socket, (sockaddr*)&m_sAddrIn, sizeof( sockaddr_in ) );

	return iReturn;
}


CXBSocket_MXM::CXBSocket_MXM( bool bCreateSocket, int iProtocol ) :
	m_socket(INVALID_SOCKET)
	,m_bConnected(false)
{
	if ( bCreateSocket )
	{
		CreateSocket( iProtocol );
	}
}

CXBSocket_MXM::~CXBSocket_MXM()
{
	if ( m_bConnected )
	{
		Shutdown();
	}
}

bool CXBSocket_MXM::CreateSocket( int iProtocol )
{
	bool bReturn = false;
	switch ( iProtocol )
	{
		case IPPROTO_TCP:
			m_socket = socket( AF_INET, SOCK_STREAM, iProtocol );
			break;
		case IPPROTO_UDP:
			m_socket = socket( AF_INET, SOCK_DGRAM, iProtocol );
			break;
		case IPPROTO_VDP:
			m_socket = socket( AF_INET, SOCK_DGRAM, iProtocol );
			break;
	}
	bReturn = (m_socket!=INVALID_SOCKET);
	if ( !bReturn )
	{
		int iError = WSAGetLastError();
	}
	return bReturn;
}

bool CXBSocket_MXM::Connect( CXBSockAddr & sockObj )
{
	int wsaError = 0;
	bool bReturn = false;
	if ( m_socket!=INVALID_SOCKET )
	{
		if ( m_bConnected )
		{
			bReturn = true;
		}
		else
		{
			if ( sockObj.Connect(m_socket) != SOCKET_ERROR)
			{
				m_bConnected = true;
				bReturn = true;
			}
			else
			{
				wsaError = WSAGetLastError();
			}
		}
	}
	return bReturn;
}

bool CXBSocket_MXM::Shutdown( void )
{
	bool bReturn = false;

	if ( m_bConnected )
	{
		if (shutdown( m_socket, SD_SEND) != SOCKET_ERROR)
		{
			char szEmptyBuffer[256];
			int iBytesRead;
			// Finish grabbing data...
			while( 1 )
			{
				iBytesRead = recv(m_socket, szEmptyBuffer, 256, 0);

				if ( (iBytesRead == SOCKET_ERROR) || (iBytesRead == 0) )
				{
					break;
				}
			}
			bReturn = true;
		}
		closesocket( m_socket );
	}
	return bReturn;
}

int	CXBSocket_MXM::SendData( BYTE *pbData, int iLength, int iFlags )
{
	int iReturn = 0;

	if ( pbData && (m_socket!=INVALID_SOCKET) && m_bConnected)
	{
		iReturn = send( m_socket, (const char *)pbData, iLength, iFlags );
	}
	return iReturn;
}

int		CXBSocket_MXM::SendString( LPCTSTR szData, int iFlags )
{
	int iReturn = 0;
	CStdString sData(szData);

	if ( szData && sData.GetLength() && (m_socket!=INVALID_SOCKET) &&  m_bConnected)
	{
		iReturn = SendData( (BYTE*)sData.c_str(), sData.GetLength(), iFlags );
	}
	return iReturn;
}


int  CXBSocket_MXM::RecvData( BYTE *pbData, int iLength, int iFlags )
{
	int iReturn = 0;
	if ( pbData && m_socket != INVALID_SOCKET && m_bConnected )
	{
		iReturn = recv( m_socket, (char *)pbData, iLength, iFlags );
	}
	return iReturn;
}


int DoClockSynchronization( LPCTSTR szTimeServer, long lTimeOffset )
{
	int iReturn = 0;
	CWinSock winSock;

	if ( winSock.IsValid() )
	{
		// 192.5.41.209
		// http://tycho.usno.navy.mil/ntp.html
		// CXBSockAddr sockAddr( _T("tick.usno.navy.mil"), 123 );
		CXBSockAddr sockAddr( szTimeServer, 13 );
		CXBSocket_MXM sockClock;

		if ( sockAddr.IsValid()  )
		{
			if ( sockClock.Connect( sockAddr ) )
			{
				// Do our thing here...
				// CXBSocket_MXM will clean up afterward.
				// DoActualSynchronization();
				SYSTEMTIME sysTime;
				char pbBuffer[1024];
				int iLength = sockClock.RecvData( (BYTE*)pbBuffer, 1024 );

				ZeroMemory( &sysTime, sizeof(SYSTEMTIME) );

				if ( iLength != SOCKET_ERROR && iLength < 20 )
				{
					if ( sockClock.SendString( _T("\n\n\n\n") ) != SOCKET_ERROR )
					{
						iLength = sockClock.RecvData( (BYTE*)pbBuffer, 1024 );
					}
				}
				if ( iLength != SOCKET_ERROR && iLength > 0 )
				{
					pbBuffer[iLength] = 0;
					InterpretTimeResponse(pbBuffer, sysTime);
					sysTime.wDayOfWeek = 0;
					sysTime.wMilliseconds = 0;
					if ( lTimeOffset )
					{
						FILETIME fTime;
						ULARGE_INTEGER u64Time, u64Offset;
						bool bSubtract = false;
						DWORD dwResult = 0;

						if ( lTimeOffset < 0 )
						{
							bSubtract = true;
							lTimeOffset = 0-lTimeOffset;
						}
						fTime.dwHighDateTime = 0;
						fTime.dwLowDateTime = 0;
						if ( SystemTimeToFileTime( &sysTime, &fTime ) )
						{

							u64Time.HighPart = fTime.dwHighDateTime;
							u64Time.LowPart = fTime.dwLowDateTime;

							u64Offset.QuadPart = (ULONGLONG)lTimeOffset;
							u64Offset.QuadPart = u64Offset.QuadPart*(ULONGLONG)10000; // milliseconds
							u64Offset.QuadPart = u64Offset.QuadPart*(ULONGLONG)60000; // minutes
							u64Offset.QuadPart = u64Offset.QuadPart*(ULONGLONG)60; // hours
							if ( bSubtract )
							{
								u64Time.QuadPart = u64Time.QuadPart-u64Offset.QuadPart;
							}
							else
							{
								u64Time.QuadPart = u64Time.QuadPart+u64Offset.QuadPart;
							}
							fTime.dwHighDateTime = u64Time.HighPart;
							fTime.dwLowDateTime = u64Time.LowPart;

							FileTimeToSystemTime( &fTime, &sysTime );
						}
						else
						{
							dwResult = GetLastError();
						}

						u64Offset.QuadPart = 0;
					}
					SetNewTime( sysTime);
				}

			}
		}
	}
	else
	{
		iReturn = winSock.GetError();
	}
	return iReturn;
}

void SetNewTime( SYSTEMTIME & sysTime)
{

	FILETIME  NewTime;
	FILETIME  OldTime;

	SystemTimeToFileTime( &sysTime,  &OldTime);
	// FileTimeToSystemTime( &NewTime, &sysTime);
	LocalFileTimeToFileTime( &OldTime, &NewTime );
    NtSetSystemTime(&NewTime,&OldTime);  // sets system time

}


void InterpretTimeResponse( LPCTSTR szBuffer, SYSTEMTIME & sysTime )
{
	CStdString sTimeData(szBuffer);
	long	mjd;			     /* holds parsed received mjd*/
	int		dst,ls,health;		    /* received flags: dst, ls and health */
	char	month[4];
	int		yr=2002,mo=1,dy=1,hr=0,min=0,sec=0;	/* holds parsed received time */	
//	char	Moon;

	int iHours = 0, 
		iMinutes = 0, 
		iSeconds = 0, 
		iMonth = 1, 
		iDay = 1,
		iYear = 2003;
	char cTOD = 'A';

	if ( sTimeData.GetLength() < 25 )
	{
		// MS Timeserver
		_stscanf( sTimeData.c_str(),_T(" %2d:%2d:%2d %cM %d/%d/%4d"),&iHours,&iMinutes,&iSeconds,&cTOD,&iMonth,&iDay,&iYear);		
		
		if((cTOD=='P')||(cTOD=='p'))
		{
			iHours+=12; 
			if( iHours == 24 )
			{
				iHours=12;
			}
		}
		else if( iHours == 12 ) 
		{
			iHours=0;
		}

		// swprintf(m_strDebug,L"Microsoft Time Server Says [%d %2d-%2d] %2d:%2d:%2d",yr,mo,dy,hr,min,sec);
	}
	else if ( sTimeData.GetLength() < 40 )
	{
		// *NIX Timeserver
	 	_stscanf( sTimeData.c_str(),_T("    %c%c%c %2d %2d:%2d:%2d %4d"),&month[1],&month[2],&month[3],&iDay,&iHours,&iMinutes,&iSeconds,&iYear);		

		switch(month[1])
		{
			case 'F':
				iMonth=2;
				break;
			case 'S':
				iMonth=9;
				break;
			case 'O':
				iMonth=10;
				break;
			case 'N':
				iMonth=11;
				break;
			case 'D':
				iMonth=12;
				break;
			
			case 'M':
				iMonth=3; 
				if (month[3]=='y')
					iMonth=5; 
				break;
			case 'A':
				iMonth=4;
				if (month[3]=='g')
					iMonth=8; 
				break;
			case 'J':
				iMonth=6;
				if (month[2]=='a')
				{
					iMonth=1;
				}
				else
				{
					if (month[3]=='l')
						iMonth=7;
				}
				break;
		}
		// swprintf(m_strDebug,L"Unix Time Server Says [%d %2d-%2d] %2d:%2d:%2d",yr,mo,dy,hr,min,sec);
	}
	else
	{
		 _stscanf(sTimeData.c_str(), _T(" %ld %2d-%2d-%2d %2d:%2d:%2d %d %d %d"),&mjd,&iYear,&iMonth,&iDay,&iHours,&iMinutes,&iSeconds,&dst,&ls,&health);		
	}

	sysTime.wDay	= iDay;
	sysTime.wSecond = iSeconds;
	sysTime.wMinute = iMinutes;
	sysTime.wHour	= iHours;
	sysTime.wMonth	= iMonth;
	if ( iYear > 1000 )
	{
		sysTime.wYear	= iYear;
	}
	else
	{
		sysTime.wYear	= iYear+2000;
	}
	if (sysTime.wYear>2100)
	{
		sysTime.wYear = 2003;
	}

}







