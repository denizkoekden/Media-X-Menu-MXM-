

// NewsFeed.cpp

#include <xtl.h>
#include "mxm.h"
#include "NewsFeed.h"
#include "utils.h"
#include "XBSocket.h"

CNewsFeedManager g_NewsFeedManager;

CXMLNode *	CNewsFeed::GetChannel( LPCTSTR szChannel )
{
	CXMLNode * pReturn = NULL;
	int iIndex = -1;
	CStdString sTemp;

	if ( szChannel && _tcslen( szChannel ) )
	{
		// Either channel number or title?
		iIndex = ConvertSigned( szChannel );
		if ( iIndex == 0 )
		{
			iIndex = ConvertUnsigned( szChannel );
			if ( iIndex == 0 )
			{
				if ( (szChannel[0] != _T('0')) || (_tcslen( szChannel ) > 1 ))
				{
					// Scan for channel name
					iIndex = 0;
					while( pReturn = m_pxmlNode->GetNode(_T("channel"), iIndex++ ) )
					{
						sTemp = pReturn->GetString( NULL, _T("title"), _T("") );
						if ( _tcsicmp( sTemp, szChannel ) == 0 )
						{
							// found it!
							break;
						}
					}
				}
			}
		}
		if ( pReturn == NULL )
		{
			pReturn = GetChannel( iIndex );
		}
	}
	else
	{
		// Use current channel
		pReturn = GetChannel();
	}
	return pReturn;
}



CXMLNode *	CNewsFeed::GetItem( LPCTSTR szItem )
{
	CXMLNode * pReturn = NULL;
	CXMLNode * pChannel = NULL;
	int iIndex = -1;
	CStdString sTemp;
	CStdString sChannel, sItem;

	if ( szItem && _tcslen( szItem ) )
	{
		pChannel = NULL;
		sChannel = FetchIDToken( 0, szItem );
		sItem = FetchIDToken( 1, szItem );
		
		if ( pChannel )
		{
			// Either channel number or title?
			iIndex = ConvertSigned( sItem );
			if ( iIndex == 0 )
			{
				iIndex = ConvertUnsigned( sItem );
				if ( iIndex == 0 )
				{
					if ( (sItem[0] != _T('0')) || (sItem.GetLength() > 1 ))
					{
						// Scan for channel name
						iIndex = 0;
						while( pReturn = pChannel->GetNode(_T("item"), iIndex++ ) )
						{
							sTemp = pReturn->GetString( NULL, _T("title"), _T("") );
							if ( _tcsicmp( sTemp, sItem ) == 0 )
							{
								// found it!
								break;
							}
						}
					}
				}
			}
			if ( pReturn == NULL )
			{
				pReturn = GetItem( iIndex, pChannel );
			}
		}
	}
	else
	{
		// Use current channel
		pReturn = GetItem();
	}
	return pReturn;
}


CXMLNode *	CNewsFeed::GetChannel( int iIndex )
{
	CXMLNode * pReturn = NULL;

	if ( iIndex < 0 )
	{
		iIndex = m_iCurrentChannel;
	}

	pReturn = m_pxmlNode->GetNode(_T("channel"), iIndex );

	return pReturn;
}

CXMLNode *	CNewsFeed::GetItem( int iItem, int iChannel )
{
	CXMLNode * pChannel = GetChannel( iChannel );
	CXMLNode * pReturn = NULL;

	if ( pChannel )
	{
		pReturn = GetItem( iItem, pChannel );
	}
	return pReturn;
}


CXMLNode *	CNewsFeed::GetItem( int iIndex, CXMLNode * pChannel )
{
	CXMLNode * pReturn = NULL;

	if ( pChannel == NULL )
	{
		// Use current channel selected in feed
		pChannel = GetChannel(-1);
	}

	if ( pChannel )
	{
		if ( iIndex < 0 )
		{
			iIndex = m_iCurrentItem;
		}
		pReturn = pChannel->GetNode(_T("item"), iIndex );
	}

	return pReturn;
}

CStdString	CNewsFeed::GetItemValue( LPCTSTR szSection, LPCTSTR szKey )
{
	CStdString sReturn;

	return sReturn;
}

CStdString	CNewsFeed::GetChannelValue( LPCTSTR szSection, LPCTSTR szKey )
{
	CStdString sReturn;
	CXMLNode * pChannelNode;

	pChannelNode = GetChannel(szSection);
	return sReturn;
}

bool CNewsFeed::IsCategory( DWORD dwFlag )
{
	bool bReturn = false;

	if ( dwFlag )
	{
		if ( dwFlag&m_dwCategories )
		{
			bReturn = true;
		}
	}
	else
	{
		bReturn = true;
	}
	return bReturn;
}

bool CNewsFeed::CycleItem( int & iChannel, int & iItem )
{
	bool bReturn = false;
	CXMLNode * pChannelNode;
	CXMLNode * pItemNode;

	if ( m_dwItemTime < GetTickCount() )
	{
//		CStdString sTemp;

		m_dwItemTime =  GetTickCount()+m_dwItemInterval;

		pChannelNode = GetChannel( iChannel );

//		sTemp.Format( _T("Cycling Newsfeed! %d Channel=%d 0x%08x"), iItem, iChannel, (DWORD)pChannelNode );
//		OutputDebugString( sTemp );

		if ( pChannelNode )
		{
			iItem++;

			pItemNode = GetItem( iItem, pChannelNode );
		
//			sTemp.Format( _T("Item Node %d Channel=%d 0x%08x"), iItem, iChannel, (DWORD)pItemNode );
//			OutputDebugString( sTemp );

			if ( pItemNode == NULL )
			{
				// No item!
				iChannel++;
				iItem = 0;

				pChannelNode = GetChannel( iChannel );

				if ( pChannelNode )
				{
					while( pChannelNode )
					{
						pItemNode = GetItem( iItem, pChannelNode );
						if ( !pItemNode )
						{
							// No valid items in channel, go to next channel
							iChannel++;
							pChannelNode = GetChannel( iChannel );
						}
						else
						{
							// Found a valid news item, so break
							// and keep settings (valid channel, valid item)
							break;
						}
					}
				}
				else
				{
					// Couldn't fetch the 'next' channel, so reset
					iChannel = 0;
					iItem = 0;
				}
			}
		}
		else
		{
			// Invalid... get first channel and move on!
			iChannel = 0;
			iItem = 0;
		}
		// If 0,0, must have cycled. Return true!!
		if ( ( iChannel == 0 ) && ( iItem == 0 ) )
		{
			bReturn = true;
		}
	}
	return bReturn;
}

CNewsFeed::CNewsFeed() :
	m_pxmlNode(NULL),
	m_dwCategories(0)
{
	m_dwRefreshTime =  GetTickCount();
	m_dwItemTime =  GetTickCount();
	m_dwRefreshInterval = 360000;
	m_bEnabled = false;
	m_dwItemInterval = 20000;
	m_dwCategories = 0;
}


bool CNewsFeed::Configure( CXMLNode * pNode )
{
	bool bReturn = false;

	m_sName = pNode->GetString( NULL, _T("name"), _T(""), true );
	m_sURL = pNode->GetString( NULL, _T("url"), _T(""), true );
	m_bEnabled = (pNode->GetBool( NULL, _T("enable"), TRUE )==TRUE);
	m_dwRefreshInterval = pNode->GetLong( NULL, _T("UpdateIntervalMin"), 45 )*(60000);
	m_dwItemInterval = pNode->GetLong( NULL, _T("ItemIntervalSec"), 5 )*(1000);
	m_sSelectChannel = pNode->GetString( NULL, _T("selectchannel"), _T(""), true );
	m_dwItemTime =  GetTickCount()+m_dwItemInterval;
	
	// Handle CATEGORIES...
	// DWORD		m_dwCategories;
	m_iPort = 80;
	if ( !SplitURL( m_sURL, m_sServer, m_iPort, m_sURI ) )
	{
		m_sServer = _T("");
		m_sURI = _T("");
	}
	if ( m_sServer )
	{
		if ( m_sName.GetLength() == 0 )
		{
			m_sName = m_sURL;
		}
		bReturn = true;
	}
	return bReturn;
}


void CNewsFeed::FixNewsfeed( CXMLNode * pNode )
{
	CXMLNode * pChannel;
	if ( pNode->GetNodeCount( _T("channel") ) == 0 )
	{
		// Make a channel
		pChannel = pNode->AddNode( _T("channel") );
		if ( pChannel )
		{
			pChannel->SetString( NULL, _T("title"), _T("Main") );
		}
	}
	if ( pNode->GetNodeCount( _T("item") ) )
	{
		// OK, move these into our primary channel
		pChannel = GetChannel(0);
		if ( pChannel )
		{
			int iIndex = 0;
			CXMLNode * pItemNode;

			while( pItemNode = pNode->GetNode( _T("item"), 0 ) )
			{
				pChannel->CopyInNode( pItemNode );
				pNode->DeleteNode( pItemNode );
			}
		}
	}
}


// Returns TRUE if a fetch was performed.
// Doesn't fetch if:
//		Feed not enabled
//		Timer not expired && XML Node already exists
bool CNewsFeed::UpdateFeed( void )
{
	static int iFetchCount = 0;
	bool bReturn = false;
	CXMLNode * pNode = NULL;

	if ( m_bEnabled )
	{
		if ( !m_pxmlNode )
		{
			bReturn = true;
		}
		else
		{
			if ( m_dwRefreshInterval  && ( m_dwRefreshTime < GetTickCount() ) )
			{
				bReturn = true;
			}
		}
		if ( bReturn )
		{
			// Fetch web page to update feed...
			CStdString sFetch;

			sFetch = GetWebFile( m_sServer, m_sURI, m_iPort );
			if ( sFetch.GetLength() )
			{
				CStdString sFetchFilename;
				CXMLLoader xmlLoader(NULL, true);

//				sFetchFilename.Format( _T("Z:\\RSSFEED_%03d.xml"), iFetchCount );
		
				pNode = xmlLoader.LoadXMLNodesFromBuffer( sFetch );
				xmlLoader.DetachMainNode();

				FixNewsfeed( pNode );

//				SaveStringToFile( sFetchFilename, sFetch );

				CXMLNode * pTempNode = m_pxmlNode;
				iFetchCount++;

				InterlockedExchangePointer( &m_pxmlNode, pNode );
				if ( pTempNode )
				{
					delete pTempNode;
				}

				bReturn = true;
			}
			else
			{
				bReturn = false;
			}
		}
		if ( bReturn && m_dwRefreshInterval )
		{
			m_dwRefreshTime = GetTickCount()+m_dwRefreshInterval;
		}
		else
		{
			// Just try and fetch it again in a few minutes...
			m_dwRefreshTime = GetTickCount()+300000;
		}
	}
	return bReturn;
}

bool CNewsFeedManager::Start()
{
	m_bContinueThread = true;
	m_iCurrentFeed = 0;
	m_iCurrentChannel = 0;
	m_iCurrentItem = 0;
	if ( m_nfm.size() )
	{
		m_hNewsFeedThread = CreateThread( NULL, 0, InitNewsFeedThread, this, 0, NULL );
	}
	return (m_hNewsFeedThread!=NULL);
}

void CNewsFeedManager::Stop()
{
	m_bContinueThread = false;
}

CNewsFeedManager::CNewsFeedManager()
{
	m_nfm.clear();
	m_hNewsFeedThread = NULL;
	m_iCurrentFeed = 0;
	m_iCurrentChannel = 0;
	m_iCurrentItem = 0;
	m_bContinueThread = true;

}

CNewsFeed * CNewsFeedManager::GetNewsFeed( int iIndex )
{
	CNewsFeed * pReturn = NULL;
	TMapFeeds::iterator iterFeeds;

	if ( iIndex < 0 )
	{
		iIndex = m_iCurrentFeed;
	}

	iterFeeds = m_nfm.begin();
	while( iterFeeds != m_nfm.end() )
	{
		if ( iIndex )
		{
			iIndex--;
		}
		else
		{
			pReturn = iterFeeds->second;
			break;
		}
		iterFeeds++;
	}
	return pReturn;
}

CStdString CNewsFeedManager::GetNewsItemString( LPCTSTR szKey, LPCTSTR szLocation )
{
	CStdString sReturn;

	int	iCurrentFeed = m_iCurrentFeed;
	int	iCurrentChannel = m_iCurrentChannel;
	int	iCurrentItem = m_iCurrentItem;
	CXMLNode * pChannel;
	CXMLNode * pItem;

	CNewsFeed * pNewsFeed = GetNewsFeed( iCurrentFeed );
	if ( pNewsFeed )
	{
		pChannel = pNewsFeed->GetChannel( iCurrentChannel );
		if ( pChannel )
		{
			pItem = pChannel->GetNode( _T("item"), iCurrentItem );
			if ( pItem )
			{
				sReturn = pItem->GetString( NULL, szKey, _T("") );
			}
		}
	}
	return sReturn;
}


CNewsFeedManager::~CNewsFeedManager()
{
}

bool CNewsFeedManager::AddNewsFeed( CXMLNode * pNode, bool bMain )
{
	CNewsFeed * pNewsFeed = NULL;
	bool bReturn = false;

	pNewsFeed = new CNewsFeed;

	if ( pNewsFeed )
	{
		if ( pNewsFeed->Configure(pNode) )
		{
			if ( GetNewsFeed( pNewsFeed->m_sName ) == NULL )
			{
				m_nfm.insert( TMapFeeds::value_type( pNewsFeed->m_sName, pNewsFeed ) );
				bReturn = true;
			}
			else
			{
				// Doh! duplicate... get rid of it!
				delete pNewsFeed;
			}
		}
		else
		{
			delete pNewsFeed;
		}
	}
	return bReturn;
}


DWORD CNewsFeedManager::InitNewsFeedThread(LPVOID lpParameter )
{
	DWORD dwReturn = 0;
	CNewsFeedManager * pThis = (CNewsFeedManager*)lpParameter;

	dwReturn = pThis->RunThread();

	return dwReturn;
}

CNewsFeed *	CNewsFeedManager::GetNewsFeed( LPCTSTR szName )
{
	CNewsFeed * pNewsFeed = NULL;
	TMapFeeds::iterator iterFeeds;

	iterFeeds = m_nfm.begin();
	while( iterFeeds != m_nfm.end() )
	{
		if ( iterFeeds->second )
		{
			if ( _tcsicmp( iterFeeds->second->m_sName, szName) == 0 )
			{
				pNewsFeed = iterFeeds->second;
				break;
			}
		}
		iterFeeds++;
	}
	return pNewsFeed;
}


DWORD CNewsFeedManager::RunThread( void )
{
	DWORD dwReturn = 0;
	CNewsFeed * pNewsFeed = NULL;
	TMapFeeds::iterator iterFeeds;
	int iIndex = 0;

	m_bContinueThread = true;

	while( m_bContinueThread )
	{
		iIndex = 0;
		iterFeeds = m_nfm.begin();
		while( iterFeeds != m_nfm.end() )
		{
			if ( iterFeeds->second )
			{
				if ( iIndex == m_iCurrentFeed )
				{
					if ( iterFeeds->second->CycleItem( m_iCurrentChannel, m_iCurrentItem ) )
					{
						m_iCurrentFeed++;
						if ( !GetNewsFeed(m_iCurrentFeed) )
						{
							m_iCurrentFeed = 0;
						}
					}

				}
				if ( iterFeeds->second->UpdateFeed() )
				{
					Sleep( 1000 );
				}
			}
			iterFeeds++;
			iIndex++;
		}
		Sleep(1000);
	}
	return dwReturn;
}




