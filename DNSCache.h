

#pragma once

#include "StdString.h"

#include <map>

using namespace std;


#define DNSC_PER_STATIC		0
#define DNSC_PER_SESSION	1
#define DNSC_PER_DYNAMIC	2

class CDNSEntry
{
public:
	CDNSEntry();
	DWORD 		GetNextServer( bool bForce );
	void		AddServerIP( DWORD dwServerIP );
	bool		m_bValid;
	int			m_iIPIndex;
	int			m_iPersistence;
	FILETIME	m_dwLastAccess;
	DWORD		m_dwaKnownIPs[16];
};

typedef map<CStdString, CDNSEntry *> TDNSMap;

class CDNSCache
{
public:
	CDNSCache( void );

};


void DNS_SaveCache( bool bOnlyIfDirty = false );
void DNS_LoadCache( void );
CDNSEntry * DNS_GetEntry( LPCTSTR szServer );
CDNSEntry * DNS_AddEntry( LPCTSTR szServer );
DWORD DNS_Lookup( LPCTSTR szServer );

/*

<Hosts>
<Host name="something.com">
<Persistence>static</Persistence>
</Host>
<Host name="no-ip.com">
<Persistence>dynamic</Persistence>
</Host>
<Host name="super.com">
<Persistence>static</Persistence>
<KnownIP>214.16.120.4</KnownIP>
</Host>
</Hosts>

*/