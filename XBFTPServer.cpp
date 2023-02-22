#include "StdAfx.h"
#include "xbftpserver.h"

// Model:
// Starts thread to monitor connections
// Each connection gets two threads:
// 1) Command Thread
// 2) Transfer Thread
// While Transfer Thread is actively transferring data, only ABORT and QUIT works.
// Callback in place for unknown commands and SITE commands
// When a user connects, they will be given their own operating thread
// We will want to check the incoming IP and be able to block from a list, if required.

// FTPServer will determine incoming sockets and who will get a session
// FTPSession objects handling logging in, data transfers, and command channels
// FTPSessionMgr will handle organizing the sessions
// FTPUser will define an individual user, as defined by the configuration
// FTPUserMgr will manage the users

CXBFTPServer::CXBFTPServer(void) : 
	m_iMaxAnonUsersAllowed(0)
	,m_iXferPriority(7)
	,m_pServerEvents(NULL)
{
	m_wPort = 21;
}

CXBFTPServer::~CXBFTPServer(void)
{
}

int CXBFTPServer::GetXferPriority( void )
{
	int iReturn = m_iXferPriority;

	if ( iReturn < 5 )
	{
		iReturn = 5;
	}
	else if ( iReturn > 11 )
	{
		iReturn = 11;
	}

	return iReturn;
}

CXBNetSession * CXBFTPServer::NewSession( SOCKET sockSession )
{
	CXBFTPSession * pReturn = NULL;

	pReturn = new CXBFTPSession( sockSession, this );

	if ( pReturn )
	{
		pReturn->SetFTPEventHandler( this );
	}

	return (CXBNetSession *)pReturn;
}


void CXBFTPServer::SetFTPEventHandler(CXBFTPServerEvents * pEventHandler)
{
	m_pServerEvents = pEventHandler;
}

DWORD CXBFTPServer::EXEC( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szFilename, LPCTSTR szArgs )
{
	DWORD dwReturn = 500;
	if ( m_pServerEvents )
	{
		dwReturn = m_pServerEvents->EXEC( pThis, szPath, szFilename, szArgs );
	}
	else
	{
		TRACE( _T("EXEC Fired: Session: 0x%08x CWD:(%s) FILENAME:(%s) ARGS:(%s)\r\n"), pThis, szPath, szFilename, szArgs );
	}
	return dwReturn;
}

DWORD CXBFTPServer::CMD( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szCmd, LPCTSTR szArgs, CStdString & sResult )
{
	DWORD dwReturn = 500;
	if ( m_pServerEvents )
	{
		dwReturn = m_pServerEvents->CMD( pThis, szPath, szCmd, szArgs, sResult );
	}
	else
	{
		TRACE( _T("CMD Fired: Session: 0x%08x CWD:(%s) FILENAME:(%s) ARGS:(%s)\r\n"), pThis, szPath, szCmd, szArgs );
	}
	return dwReturn;
}

HRESULT	CXBFTPServer::SITE( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szArgs, CStdString & sResult )
{
	HRESULT hr = S_OK;

	if ( m_pServerEvents )
	{
		hr = m_pServerEvents->SITE( pThis, szPath, szArgs, sResult );
	}
	else
	{
		TRACE( _T("SITE Fired: Session: 0x%08x CWD:(%s) ARGS:(%s)\r\n"), pThis, szPath, szArgs );
	}
	return hr;
}


bool	CXBFTPServer::AllowAnonymous( CXBFTPSession * pThis )
{
	return m_iMaxAnonUsersAllowed?true:false;
}

DWORD	CXBFTPServer::ValidateUser( CXBFTPSession * pThis, LPCTSTR szUser, LPCTSTR szPass )
{
	DWORD dwReturn = 0;
	bool bAnon = false;

	if ( _tcsicmp( szUser, _T("anonymous") ) == 0 )
	{
		if ( AllowAnonymous( pThis ) )
		{
			bAnon = true;
			// Now, how many have logged in currently?
			// Meh. We need to understand the interaction in sessions better yet.
			// For now, we won't track this
			// m_iCurrentAnonCount++;
			dwReturn = FTPUSERPERM_LOGINOK;
		}
	}

	if ( !bAnon )
	{
		// Need to test for proper validation with allowed users.
		//
		CXBNetUser * pUser = m_userManager.GetValidEntry( szUser, szPass );

		if ( pUser )
		{
			// We could do further checking here....
			dwReturn = FTPUSERPERM_LOGINOK|FTPUSERPERM_CANDELETE;
		}
	}
	return dwReturn;
}

void CXBFTPServer::SetXferPriority( int iPriority )
{
	if ( iPriority < 0 ) // Normal
	{
		iPriority = 8;
	}
	else if ( iPriority < 5 ) // Floor it
	{
		iPriority = 5;
	}
	else if ( iPriority > 11 ) // Max it
	{
		iPriority = 11;
	}
	m_iXferPriority = iPriority;
}

// Configure using an XML-based binary tree node
void CXBFTPServer::Configure(CXMLNode * pNode)
{
	// Get port using base function
	CXBNetServer::Configure(pNode);
	CStdString sTemp;

	if ( pNode )
	{
		// Configure server here... configure users as well.
		m_iMaxAnonUsersAllowed = (pNode->GetBool( NULL, _T("AllowAnon"), TRUE ) == TRUE )?1:0;
		m_sAnonRoot = pNode->GetString( NULL, _T("AnonRoot"), _T("/") );

		sTemp = pNode->GetString( NULL, _T("FTPPriority"), _T("") );
		if ( sTemp.Compare( _T("abovenormal") ) == 0 )
		{
			m_iXferPriority = 9;
		}
		else if ( sTemp.Compare( _T("belownormal") ) == 0 )
		{
			m_iXferPriority = 7;
		}
		else if ( sTemp.Compare( _T("high") ) == 0 )
		{
			m_iXferPriority = 10;
		}
		else if ( sTemp.Compare( _T("highest") ) == 0 )
		{
			m_iXferPriority = 11;
		}
		else if ( sTemp.Compare( _T("low") ) == 0 )
		{
			m_iXferPriority = 6;
		}
		else if ( sTemp.Compare( _T("lowest") ) == 0 )
		{
			m_iXferPriority = 5;
		}
		else
		{
			m_iXferPriority = 8;
		}

		// m_iXferPriority = pNode->GetLong( NULL, _T("Priority"), 7 );
		CXMLNode * pUserNode = NULL;
		int iIndex=0;

		while( pUserNode = pNode->GetNode( _T("user"), iIndex++ ) )
		{
			// Configure for the user
			m_userManager.AddEntry( pUserNode );
			CXBNetUser * pUser = m_userManager.GetLastAdded();
			if ( pUser )
			{
				// Add flags at this point...
			}
		}
	}
	if ( m_userManager.GetUserCount() == 0 )
	{
		m_userManager.AddEntry( _T("xbox"), _T("xbox"), _T("/") );
	}
}

