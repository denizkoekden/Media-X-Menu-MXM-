
#pragma once

#ifdef _XBOX


#define _SockConnect	connect
#define _SockAccept		accept
#define _SockClose		closesocket
#define _SockListen		listen

#else


int XNetRandom( unsigned char *pb, unsigned int cb );


#define XNET_GET_XNADDR_NONE			0x00000000
#define XNET_GET_XNADDR_DHCP			0x00000001
#define XNET_GET_XNADDR_DNS				0x00000002
#define XNET_GET_XNADDR_ETHERNET		0x00000004
#define XNET_GET_XNADDR_ONLINE			0x00000008
#define XNET_GET_XNADDR_PENDING			0x00000010
#define XNET_GET_XNADDR_PPPOE			0x00000020
#define XNET_GET_XNADDR_STATIC			0x00000040
#define XNET_GET_XNADDR_GATEWAY			0x00000080

typedef struct {
    IN_ADDR ina;
    IN_ADDR inaOnline;
    WORD wPortOnline;
    BYTE abEnet[6];
    BYTE abOnline[20];
} XNADDR;

typedef struct {
    BYTE ab[8];
} XNKID;

int _SockConnect( SOCKET s, const struct sockaddr FAR *name, int namelen );
SOCKET _SockAccept( SOCKET s, struct sockaddr FAR *addr, int FAR *addrlen );
int _SockClose( SOCKET s );
int _SockListen( SOCKET s, int backlog );



DWORD XNetGetTitleXnAddr( XNADDR *pxna );
int XNetXnAddrToInAddr( const XNADDR *pxna, const XNKID *pxnkid, IN_ADDR *pina );
// bool FileExists( LPCTSTR szFilename );
#endif
