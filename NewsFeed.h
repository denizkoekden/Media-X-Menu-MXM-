// NewsFeed.h

#ifndef __NEWSFEED_H__
#define __NEWSFEED_H__

#include "StdString.h"


class CNewsFeed
{
	bool		m_bEnabled;
	DWORD		m_dwRefreshTime;
	DWORD		m_dwRefreshInterval;
	DWORD		m_dwItemTime;
	DWORD		m_dwItemInterval;
	CXMLNode *	m_pxmlNode;
	DWORD		m_dwCategories;
	CStdString	m_sSelectChannel;
	CStdString	m_sURL;
	CStdString	m_sServer;
	CStdString	m_sURI;
	int			m_iPort;

public:
	CStdString	m_sName;
	CNewsFeed();
	~CNewsFeed() {};
	bool		Configure( CXMLNode * pNode );
	void		SetCategory( DWORD dwFlags ) { m_dwCategories |= dwFlags; };
	void		ResetCategory( DWORD dwFlags ) { m_dwCategories &= ~dwFlags; };
	CXMLNode *	GetNode( void )	{ return m_pxmlNode; };

	CStdString	GetItemValue( LPCTSTR szSection, LPCTSTR szKey );
	CStdString	GetChannelValue( LPCTSTR szSection, LPCTSTR szKey );

	void		FixNewsfeed( CXMLNode * pNode );
	int			m_iCurrentChannel;
	int			m_iCurrentItem;
	bool		IsCategory( DWORD dwFlag );
	bool		UpdateFeed( void );
	CXMLNode *	GetChannel( LPCTSTR szChannel );
	CXMLNode *	GetChannel( int iIndex = -1 );
	CXMLNode *	GetItem( LPCTSTR szItem );
	CXMLNode *	GetItem( int iIndex = -1, CXMLNode * pChannel = NULL );
	CXMLNode *	GetItem( int iItem, int iChannel );
	bool		CycleItem( int & iChannel, int & iItem );
};

typedef map<string, CNewsFeed *> TMapFeeds;

class CNewsFeedManager
{
	TMapFeeds		m_nfm;
	HANDLE			m_hNewsFeedThread;
public:
	CNewsFeedManager();
	~CNewsFeedManager();
	bool			Start();
	void			Stop();
	DWORD			RunThread( void );
    static DWORD WINAPI InitNewsFeedThread(LPVOID lpParameter );
	bool			AddNewsFeed( CXMLNode * pNode, bool bMain );
	void			UpdateCycle( DWORD dwCategories );
	CNewsFeed *		GetNewsFeed( LPCTSTR szName );
	CNewsFeed *		GetNewsFeed( int iIndex );
	CStdString		GetNewsItemString( LPCTSTR szKey, LPCTSTR szLocation = NULL );
	CStdString		m_sCategory;
	int				m_iCurrentFeed;
	int				m_iCurrentChannel;
	int				m_iCurrentItem;
	bool			m_bContinueThread;
};

extern CNewsFeedManager g_NewsFeedManager;



#endif