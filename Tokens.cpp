/*********************************************************************************\
 * Tokens.cpp
 * Used to convert ASCII strings into token values
 * Includes support for sets of tokens 
 *
 * (C) 2002 MasterMind
\*********************************************************************************/


#include <xtl.h>
#include "Tokens.h"

struct _TokenSet
{
	LPCTSTR m_szToken;
	long	m_lID;
};


_TokenSet dataTokenSet0[] =
{
	{ _T("title"), TKNMN_TITLE },
	{ _T("menu"),	TKNMN_MENU },
	{ _T("help"),	TKNMN_HELP },
	{ _T("thumbnail"),	TKNMN_THUMBNAIL },
	{ _T("loading"),	TKNMN_LOADING },
	{ NULL, -1 }
};


long GetToken( LPCTSTR szTokenStr, int iSet )
{
	long lReturn = -1;
	CStdString sToken( szTokenStr );
	_TokenSet *pTokenSet = NULL;
		

	if ( szTokenStr && sToken.GetLength() )
	{
		sToken.MakeLower();

		switch( iSet )
		{
			case 0:
				pTokenSet = dataTokenSet0;
				break;
		}
		if ( pTokenSet )
		{
			int iIndex = 0;
			while( pTokenSet[iIndex].m_szToken && (lReturn == -1) )
			{
				if ( sToken.Compare(pTokenSet[iIndex].m_szToken ) == 0 )
				{
					lReturn = pTokenSet[iIndex].m_lID;
				}
				iIndex++;
			}
		}
	}
	return lReturn;
}

CStdString GetTokenString( long lToken, int iSet )
{
	CStdString sReturn( _T("") );
	_TokenSet *pTokenSet = NULL;

	switch( iSet )
	{
		case 0:
			pTokenSet = dataTokenSet0;
			break;
	}
	if ( pTokenSet )
	{
		int iIndex = 0;
		while( pTokenSet[iIndex].m_szToken && (sReturn.GetLength()<1) )
		{
			if ( pTokenSet[iIndex].m_lID == lToken )
			{
				sReturn = pTokenSet[iIndex].m_szToken;
			}
			iIndex++;
		}
	}
	return sReturn;
}



