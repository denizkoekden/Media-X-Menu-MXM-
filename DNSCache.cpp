

#include "StdAfx.h"
#include "DNSCache.h"
#include "xmltree.h"

struct hostent* gethostbyname(const char* _name);


TDNSMap g_dnsMap;
bool	g_bCacheDirty = false;

void DNS_ClearCache( void );

DWORD GetDottedIP( LPCTSTR szVal )
{
	DWORD dwReturn = 0; // INADDR_NONE;
	CStdString sVal(szVal), sPart;
	int iPos = 0;
	DWORD dwPart;
//	IN_ADDR addrVal;

	// addrVal.S_un.S_un_b.s_b1 = 
	iPos = sVal.Find( _T('.') );
	if ( iPos >= 0 )
	{
		sPart = sVal.Left( iPos );
		sVal = sVal.Mid( iPos+1 );
		dwPart = _tcstoul( sPart, NULL, 10 )&0xff;
		dwReturn = dwPart&0xff;
		iPos = sVal.Find( _T('.') );
		if ( iPos >= 0 )
		{
			sPart = sVal.Left( iPos );
			sVal = sVal.Mid( iPos+1 );
			dwPart = _tcstoul( sPart, NULL, 10 )&0xff;
			dwReturn |= (dwPart<<8);
		}
		iPos = sVal.Find( _T('.') );
		if ( iPos >= 0 )
		{
			sPart = sVal.Left( iPos );
			sVal = sVal.Mid( iPos+1 );
			dwPart = _tcstoul( sPart, NULL, 10 )&0xff;
			dwReturn |= (dwPart<<16);
		}
		dwPart = _tcstoul( sVal, NULL, 10 )&0xff;
		dwReturn |= (dwPart<<24);
	}
	else
	{
		dwReturn = _tcstoul( sVal, NULL, 10 );
	}
	return dwReturn;
}

void DNS_ClearCache( void )
{
	TDNSMap::iterator iterDNS = g_dnsMap.begin();
	
	if ( g_dnsMap.size() )
	{
		while( iterDNS != g_dnsMap.end() )
		{
			if ( iterDNS->second )
			{
				delete (CDNSEntry*)(iterDNS->second);
			}
			iterDNS++;
		}
	}
	g_dnsMap.clear();
}

CDNSEntry * DNS_GetEntry( LPCTSTR szServer )
{
	CDNSEntry * pReturn = NULL;
	CStdString sServer(szServer);
	
	sServer.MakeLower();
	sServer.Trim();
	
	if ( sServer.GetLength() )
	{
		TDNSMap::iterator iterDNS = g_dnsMap.find(sServer);
		if ( iterDNS != g_dnsMap.end() )
		{
			pReturn = iterDNS->second;
		}
	}
	return pReturn;
}

CDNSEntry * DNS_AddEntry( LPCTSTR szServer )
{
	CDNSEntry * pReturn = DNS_GetEntry(szServer);
	
	if ( pReturn == NULL )
	{
		CStdString sServer(szServer);
		
		sServer.MakeLower();
		sServer.Trim();
		pReturn = new CDNSEntry;
		if ( pReturn )
		{
			g_dnsMap[sServer] = pReturn;
			if ( pReturn != DNS_GetEntry( szServer ) )
			{
				delete pReturn;
				pReturn = NULL;
			}
		}
	}
	return pReturn;
}

DWORD DNS_Lookup( LPCTSTR szServer )
{
	DWORD dwReturn = INADDR_NONE;
	CDNSEntry * pEntry = DNS_GetEntry( szServer );
	
	if ( pEntry == NULL )
	{
		// Create an entry - default is "session"
		pEntry = DNS_AddEntry( szServer );
	}
	
	if ( pEntry )
	{
		dwReturn = pEntry->GetNextServer(false);
	}
	
	if ( dwReturn == INADDR_NONE )
	{
		// Couldn't get a valid entry, for whatever reason
		// Try and get them from the net
		dwReturn  = INADDR_NONE;
		XNDNS * pxndns = NULL;

		// Prime the DNS lookup...
		gethostbyname( szServer );

		int err = XNetDnsLookup( szServer, NULL, &pxndns);
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
				int iIndex=0;
				DWORD dwDNS;
				for( iIndex=0; iIndex< (int)(pxndns->cina); iIndex++ )
				{
					dwDNS = pxndns->aina[iIndex].S_un.S_addr;
					if ( dwDNS && ( dwDNS != INADDR_NONE ) )
					{
						pEntry->AddServerIP( dwDNS );
					}
				}
				dwReturn = pEntry->GetNextServer(false); // pxndns->aina[0].S_un.S_addr;
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
	}
	if ( ( dwReturn == INADDR_NONE ) && pEntry )
	{
		dwReturn = pEntry->GetNextServer( true );
	}	
	return dwReturn;
}

void DNS_SaveCache( bool bOnlyIfDirty )
{
	CXMLNode xmlHosts( NULL, _T("hosts") );
	CStdString sIP;

	if ( g_dnsMap.size() && (!bOnlyIfDirty||g_bCacheDirty) )
	{
		OutputDebugString( _T("Saving DNS Cache\r\n") );
		TDNSMap::iterator iterDNS = g_dnsMap.begin();
		
		while( iterDNS != g_dnsMap.end() )
		{
			if ( iterDNS->second )
			{
				CXMLNode * pNewNode = xmlHosts.AddNode( _T("host") );
				if ( pNewNode )
				{
					CDNSEntry * pEntry = iterDNS->second;
					pNewNode->SetString( NULL, _T("name"), iterDNS->first, true );
					switch( pEntry->m_iPersistence )
					{
						case -1:
							pNewNode->SetString( NULL, _T("persistence"), _T("dynamic"), false );
							break;
						case 1:
							pNewNode->SetString( NULL, _T("persistence"), _T("static"), false );
							break;
						default:
							pNewNode->SetString( NULL, _T("persistence"), _T("session"), false );
							break;
					}
					int iIndex;
					CXMLElement * pElement;
					for( iIndex=0; iIndex<16; iIndex++ )
					{
						if ( pEntry->m_dwaKnownIPs[iIndex] && pEntry->m_dwaKnownIPs[iIndex] != INADDR_NONE )
						{
							pElement = pNewNode->AddElement( _T("knownip") );
							if ( pElement )
							{
								sIP = MakeIPString(pEntry->m_dwaKnownIPs[iIndex]);
								pElement->SetString( sIP );
							}
						}
					}
				}	
			}
			iterDNS++;
		}
		g_bCacheDirty = false;
		xmlHosts.SaveNode( _T("u:\\hosts.xml") );
	}
}

void DNS_LoadCache( void )
{
	CXMLLoader xmlLoader(NULL,true);
	CXMLNode * pHosts = NULL;

	if ( FileExists( _T("U:\\hosts.xml") ) )
	{
		pHosts = xmlLoader.LoadXMLNodes( _T("U:\\hosts.xml") );
		if ( pHosts )
		{
			CXMLNode * pHost = NULL;
			CStdString sVal, sIP;
			CDNSEntry * pEntry;
			int iIndex = 0;

			while( pHost = pHosts->GetNode( _T("host"), iIndex++ ) )
			{
				sVal = pHost->GetString( NULL, _T("name"), _T(""), true );
				if ( sVal.GetLength() )
				{
					pEntry = DNS_AddEntry( sVal );
					sVal = pHost->GetString( NULL, _T("persistence"), _T("session"), true );
					if ( _tcsicmp( sVal, _T("static") ) == 0 )
					{
						pEntry->m_iPersistence = 1;
						pEntry->m_bValid = true;
					}
					else if ( _tcsicmp( sVal, _T("dynamic") ) == 0 )
					{
						pEntry->m_iPersistence = -1;
					}
					else
					{
						pEntry->m_iPersistence = 0;
					}
					CXMLElement * pElemIP = NULL;
					DWORD dwIP;
					int iIPIndex = 0;
					while( pElemIP = pHost->GetElement(_T("knownip"), iIPIndex++) )
					{
						sVal = pElemIP->GetString( _T(""), NULL, true );
						dwIP = GetDottedIP( sVal );
						sIP = MakeIPString(dwIP);
						if ( dwIP && ( dwIP != INADDR_NONE ) )
						{
							pEntry->AddServerIP(dwIP);
						}
					}
					if ( pEntry->m_iPersistence == 1 )
					{
						pEntry->m_bValid = true;
					}
					else
					{
						// reset this for dynamic or session on load
						pEntry->m_bValid = false;
					}
					// pEntry->m_iPersistence
				}
			}
		}
	}
	g_bCacheDirty = false;
}



CDNSEntry::CDNSEntry() :
	m_bValid(false),
	m_iIPIndex(0),
	m_iPersistence(0)
{
	int iIndex;
	// FILETIME	m_dwLastAccess;
	for( iIndex=0; iIndex<16; iIndex++ )
	{
		m_dwaKnownIPs[iIndex] = INADDR_NONE;
	}
}

DWORD CDNSEntry::GetNextServer( bool bForce )
{
	DWORD dwReturn = INADDR_NONE;
	int iIndex;
	
	if ( m_bValid )
	{
		for( iIndex=0; iIndex<16; iIndex++ )
		{
			if ( m_dwaKnownIPs[m_iIPIndex&0x0f] && (m_dwaKnownIPs[m_iIPIndex&0x0f] != INADDR_NONE) )
			{
				dwReturn = m_dwaKnownIPs[m_iIPIndex&0x0f];
				break;
			}
			m_iIPIndex++;
		}
		m_iIPIndex = (m_iIPIndex+1)&0x0f;
	}
	if ( dwReturn && (dwReturn != INADDR_NONE) && !bForce )
	{
		switch( m_iPersistence )
		{
			case -1: // Dynamic... always reject non-forced
				dwReturn = INADDR_NONE;
				break;
			default: // Rest are ok. Leave them be.
				break;
		}
	}
	return dwReturn;
}

void CDNSEntry::AddServerIP( DWORD dwServerIP )
{
	int iIndex;

	if ( dwServerIP && (dwServerIP != INADDR_NONE) )
	{
		for( iIndex=0; iIndex<16; iIndex++ )
		{
			if ( m_dwaKnownIPs[iIndex] == dwServerIP )
			{
				m_bValid = true;
				break;
			}
		}
		if ( iIndex == 16 ) // ServerIP not found...
		{
			// Find free slot
			for( iIndex=0; iIndex<16; iIndex++ )
			{
				if ( m_dwaKnownIPs[(m_iIPIndex+iIndex)&0x0f] = INADDR_NONE )
				{
					m_dwaKnownIPs[(m_iIPIndex+iIndex)&0x0f] = dwServerIP;
					m_bValid = true;
					g_bCacheDirty = true;
					break;
				}
			}
			if ( iIndex == 16 )
			{
				m_dwaKnownIPs[m_iIPIndex&0x0f] = dwServerIP;
				m_bValid = true;
				g_bCacheDirty = true;
			}
		}
	}
}




