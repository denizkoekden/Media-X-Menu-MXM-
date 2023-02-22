#pragma once

#include "XBNetSession.h"
#include <list>

using namespace std;


typedef list<CXBNetSession *> TNetSessionList;

class CXBNetSessionManager : virtual public CXBNetSessionEvents
{
protected:
	TNetSessionList		m_sessionList;
public:
	CXBNetSessionManager(void);
	virtual ~CXBNetSessionManager(void);
	// Adds a Network Server Session to the list
	virtual bool AddSession(CXBNetSession * pSession);
	// Tells all managed sessions we are closing and they should shutdown - Blocks until finished
	virtual HRESULT StopSessions(DWORD dwTimeout = 1000, bool bForce = false);
	virtual void	SessionStarted( CXBNetSession * pThis );
	virtual void	SessionEnded( CXBNetSession * pThis );
	CXBNetSession *	DetachByHandle( HANDLE hHandle, bool bAddRef = false );
	CXBNetSession *	DetachByIndex( int iIndex, bool bAddRef = false );
	CXBNetSession *	GetByHandle( HANDLE hHandle );
	CXBNetSession *	GetByIndex( int iIndex );
	void			DeleteByHandle( HANDLE hHandle );
	int				GetCount( void );
};
