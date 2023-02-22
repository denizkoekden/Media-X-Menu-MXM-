#pragma once

#include "xbnetserver.h"
#include "xbftpsession.h"
#include "xbnetuser.h"

class CXBFTPServerEvents
{
public:
	virtual DWORD	CMD( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szCmd, LPCTSTR szArgs, CStdString & sResult ) = 0;
	virtual DWORD	EXEC( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szFilename, LPCTSTR szArgs ) = 0;
	virtual HRESULT	SITE( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szArgs, CStdString & sResult ) = 0;
};

class CXBFTPServer : virtual public CXBNetServer, virtual public CXBFTPSessionEvents
{
public:
	CXBFTPServer(void);
	virtual ~CXBFTPServer(void);

	CXBNetUserList	m_userManager;

// Access Functions

	// Returns the number of anonymous users currently allowed to log in
	int GetMaxAnonUsersAllowed( void )	{ return m_iMaxAnonUsersAllowed;	};

	// Configure using an XML-based binary tree node
	virtual void Configure(CXMLNode * pNode);

	int			GetXferPriority( void );
	void		SetXferPriority( int iPriority );

	// Set the event handler for our session
	void SetFTPEventHandler(CXBFTPServerEvents * pEventHandler);

protected:
	// The maximum number of anonymous users allowed logged in at one time.
	int				m_iXferPriority;
	int				m_iMaxAnonUsersAllowed;
	int				m_iCurrentAnonCount;
	CStdString		m_sAnonRoot;
	CXBFTPServerEvents * m_pServerEvents;
protected:
//	virtual HRESULT CreateSession( SOCKET sockSession, struct sockaddr * addrSession );
	virtual CXBNetSession * NewSession( SOCKET sockSession );

	virtual DWORD	CMD( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szCmd, LPCTSTR szArgs, CStdString & sResult );
	virtual DWORD	EXEC( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szFilename, LPCTSTR szArgs );
	virtual HRESULT	SITE( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szArgs, CStdString & sResult );
	virtual bool	AllowAnonymous( CXBFTPSession * pThis );
	virtual DWORD	ValidateUser( CXBFTPSession * pThis, LPCTSTR szUser, LPCTSTR szPass );

};
