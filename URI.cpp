


#include "StdAfx.h"
#include "URI.h"



CURI::CURI( LPCTSTR szURI )
{
	if ( szURI && _tcslen( szURI ) )
	{
		Split( szURI );
	}
}

#define URIFRAG_AUTH	0
#define URIFRAG_SERVER	1
#define URIFRAG_PATH	2
#define URIFRAG_QUERY	3

LPCTSTR szEncodeChars[] = 
{
	_T("%:=+@?&/-~\""),
	_T("%=+@?&/~\""),
	_T("%=+?&~\""),
	_T("%+?~\"")
};

CStdString EncodeURIFragment( LPCTSTR szFragment, int iFragmentID )
{
	CStdString sReturn(szFragment);
	CStdString sOld, sNew;
	int iPos = 0;

	if ( iFragmentID > URIFRAG_QUERY )
	{
		iFragmentID = URIFRAG_QUERY;
	}
	if ( iFragmentID < 0 )
	{
		iFragmentID = URIFRAG_AUTH;
	}

	LPCTSTR szEncode = szEncodeChars[iFragmentID];

	while( szEncode[iPos] )
	{
		sOld.Format( _T("%c"), szEncode[iPos] );
		sNew.Format( _T("%%%02X"), (szEncode[iPos]&0x0ff) );
		sReturn.Replace( sOld, sNew );
		iPos++;
	}
	// Handle special case: spaces!
	if ( iFragmentID != URIFRAG_SERVER )
	{
		sReturn.Replace( _T(" "), _T("+") );
	}
	else
	{
		sReturn.Replace( _T(" "), _T("%20") );
	}
	return sReturn;
}

CStdString DecodeURIFragment( LPCTSTR szFragment, bool bHandleAmpersand )
{
	CStdString sReturn;
	LPSTR szNewFragment = _tcsdup( szFragment );
	int iPos = 0, iNewPos = 0;
	TCHAR szNumber[3];

	while( szNewFragment && szNewFragment[iPos] )
	{
		if ( szNewFragment[iPos] == _T('+') )
		{
			szNewFragment[iNewPos] = _T(' ');
			iNewPos++;
			iPos++;
		}
		else if ( szNewFragment[iPos] == _T('%') )
		{
			iPos++;
			szNumber[0] = szNewFragment[iPos];
			if ( szNewFragment[iPos] )
			{
				iPos++;
				szNumber[1] = szNewFragment[iPos];
				if ( szNewFragment[iPos] )
				{
					szNumber[2] = 0;
					if ( !bHandleAmpersand && ((TCHAR)_tcstoul( szNumber, NULL, 16 ) == _T('&') ) )
					{
						szNewFragment[iNewPos++] = szNewFragment[iPos-2];
						szNewFragment[iNewPos++] = szNewFragment[iPos-1];
						szNewFragment[iNewPos] = szNewFragment[iPos];
					}
					else
					{
						szNewFragment[iNewPos] = (TCHAR)_tcstoul( szNumber, NULL, 16 );
					}
					iNewPos++; iPos++;
				}
			}
		}
		else
		{
			szNewFragment[iNewPos] = szNewFragment[iPos];
			iNewPos++;
			iPos++;
		}

	}
	szNewFragment[iNewPos] = 0;
	sReturn = szNewFragment;
	return sReturn;
}


CStdString CURI::GetPathQuery( void )
{
	CStdString sReturn;
	
	GetMerged();
	sReturn = m_sPathQuery;
	return sReturn;
}

CStdString CURI::GetServer( void )
{
	CStdString sReturn;
	
	GetMerged();
	sReturn = m_sServer;
	return sReturn;
}

CStdString CURI::GetMerged( void )
{
	CStdString sReturn;

	if ( m_sLogin.GetLength() )
	{
		if ( m_sPassword.GetLength() )
		{
			m_sAuth.Format( _T("%s:%s"), EncodeURIFragment(m_sLogin,URIFRAG_AUTH).c_str(), EncodeURIFragment(m_sPassword,URIFRAG_AUTH).c_str() );
		}
		else
		{
			m_sAuth = EncodeURIFragment(m_sLogin,URIFRAG_AUTH);
		}
	}
	else if ( m_sPassword.GetLength() )
	{
		m_sAuth = EncodeURIFragment(m_sPassword,URIFRAG_AUTH);
	}
	if ( m_sScheme.GetLength() == 0 )
	{
		m_sScheme = _T("http");
	}
	if ( m_sServer.GetLength() == 0 )
	{
		m_sServer = _T("127.0.0.1");
	}
	if ( m_sPath.GetLength() )
	{
		if ( m_sQuery.GetLength() )
		{
			m_sPathQuery.Format( _T("%s?%s"), EncodeURIFragment(m_sPath,URIFRAG_PATH).c_str(), m_sQuery.c_str() ); // ,URIFRAG_QUERY).c_str() );
		}
		else
		{
			m_sPathQuery = EncodeURIFragment(m_sPath,URIFRAG_PATH);
		}
	}
	else
	{
		if ( m_sQuery.GetLength() )
		{
			m_sPathQuery.Format( _T("?%s"), m_sQuery.c_str() ); // EncodeURIFragment(m_sQuery,URIFRAG_QUERY).c_str() );
		}
		else
		{
			m_sPathQuery = _T("/");
		}
	}
	if ( m_sAuth.GetLength() )
	{
		sReturn.Format( _T("%s://%s@%s%s"), m_sScheme.c_str(), m_sAuth.c_str(), EncodeURIFragment(m_sServer,URIFRAG_SERVER).c_str(), m_sPathQuery.c_str() );
	}
	else
	{
		sReturn.Format( _T("%s://%s%s"), m_sScheme.c_str(), EncodeURIFragment(m_sServer,URIFRAG_SERVER).c_str(), m_sPathQuery.c_str() );
	}
	return sReturn;
}

void CURI::Split( LPCTSTR szURI )
{
	CStdString sBody, sTail;
	
	if ( szURI && _tcslen( szURI ) )
	{
		m_sURI = szURI;
	}
	if ( m_sURI.GetLength() )
	{
		int iPos, iStart, iSlashPos, iQueryPos;

		m_dwPort = 0xFFFFFFFF;
		iPos = m_sURI.Find( _T("://") );
		iQueryPos = m_sURI.Find( _T("?") );
		if ( iPos >= 0 )
		{
			iSlashPos = m_sURI.Find( _T("/"), iPos+3 );
		}
		else
		{
			iSlashPos = m_sURI.Find( _T("/") );
		}
		// These are used to check against the AUTH fragment,
		// the @ might appear in the path or query, so we need
		// to make sure we only see an AUTH fragment if it appears
		// BEFORE either of these two markers, if they exist.
		// Damn screwy URI spec!!
		if ( iSlashPos < 0 )
		{
			iSlashPos = m_sURI.GetLength()+1;
		}
		if ( iQueryPos < 0 )
		{
			iQueryPos = m_sURI.GetLength()+1;
		}
		if ( iPos >= 0 )
		{
			m_sScheme = m_sURI.Left( iPos );
			m_sScheme.MakeLower();
			iStart = iPos += 3;
			sBody = m_sURI.Mid( iStart );
		}
		else
		{
			// Default to web protocol
			m_sScheme = _T("http");
		}


		// We only count the "auth" if the '@' happens BEHIND our path
		// and our query...
		iPos = m_sURI.Find( _T('@'), iStart );
		if ( ( iPos >= 0 ) && ( iPos < iSlashPos ) && ( iPos < iQueryPos ) )
		{
			m_sAuth = m_sURI.Mid( iStart, iPos-iStart );
			iStart = iPos+1;
			iPos = m_sAuth.Find(_T(':'));
			if ( iPos >= 0 )
			{
				m_sLogin = DecodeURIFragment(m_sAuth.Left( iPos ));
				m_sPassword = DecodeURIFragment(m_sAuth.Mid( iPos+1 ));
			}
			else
			{
				m_sLogin = DecodeURIFragment(m_sAuth);
				m_sPassword = _T("");
			}
		}
		else
		{
			// No Auth String
			m_sAuth = _T("");
			m_sLogin = _T("");
			m_sPassword = _T("");
		}
		iPos = m_sURI.Find( _T('/'), iStart );
		// We only have a valid path if the query is AFTER the slash!!
		if ( iPos >= 0 && iPos < iQueryPos )
		{
			m_sServer = DecodeURIFragment(m_sURI.Mid( iStart, iPos-iStart ));
			int iPortPos = m_sServer.Find( _T(":") );
			if ( iPortPos >= 0 )
			{
				CStdString sPort = m_sServer.Mid( iPortPos+1 );
				m_sServer = m_sServer.Left( iPortPos );
				m_dwPort = _tcstoul( sPort, NULL, 10 );
			}
			iStart = iPos;
			m_sPathQuery = m_sURI.Mid( iPos );
			iPos = m_sURI.Find( _T('?'), iStart );
			if ( iPos >= 0 )
			{
				m_sPath = DecodeURIFragment(m_sURI.Mid( iStart, iPos-iStart ));
				m_sQuery = m_sURI.Mid( iPos+1 ); // DecodeURIFragment();
			}
			else
			{
				m_sPath = DecodeURIFragment(m_sPathQuery);
				m_sQuery = _T("");
			}
		}
		else
		{
			// No Path... Query?
			m_sPath = _T("");
			iPos = m_sURI.Find( _T('?'), iStart );
			if ( iPos >= 0 )
			{
				m_sServer = DecodeURIFragment(m_sURI.Mid( iStart, iPos-iStart ));
				int iPortPos = m_sServer.Find( _T(":") );
				if ( iPortPos >= 0 )
				{
					CStdString sPort = m_sServer.Mid( iPortPos+1 );
					m_sServer = m_sServer.Left( iPortPos );
					m_dwPort = _tcstoul( sPort, NULL, 10 );
				}
				m_sPathQuery = m_sURI.Mid( iPos+1 );
				m_sQuery = m_sPathQuery; // DecodeURIFragment(m_sPathQuery, false );
			}
			else
			{
				m_sPathQuery = _T("");
				m_sQuery = _T("");
				m_sServer = DecodeURIFragment(m_sURI.Mid( iStart ));
				int iPortPos = m_sServer.Find( _T(":") );
				if ( iPortPos >= 0 )
				{
					CStdString sPort = m_sServer.Mid( iPortPos+1 );
					m_sServer = m_sServer.Left( iPortPos );
					m_dwPort = _tcstoul( sPort, NULL, 10 );
				}
			}
		}
		if ( m_dwPort == 0xFFFFFFFF )
		{
			if ( _tcsicmp( m_sScheme, _T("http") ) == 0 )
			{
				m_dwPort = 80;
			}
			else if ( _tcsicmp( m_sScheme, _T("telnet") ) == 0 )
			{
				m_dwPort = 23;
			}
			else if ( _tcsicmp( m_sScheme, _T("ftp") ) == 0 )
			{
				m_dwPort = 21;
			}
			else if ( _tcsicmp( m_sScheme, _T("pop") ) == 0 )
			{
				m_dwPort = 110;
			}
			else if ( _tcsicmp( m_sScheme, _T("nntp") ) == 0 )
			{
				m_dwPort = 119;
			}
			else
			{
				m_dwPort = 80;
			}
		}
	}
}
