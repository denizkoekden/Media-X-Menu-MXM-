/*********************************************************************************\
 * ItemInfo.cpp
 * Class for handling Menu Item information
 * (C) 2002 MasterMind
\*********************************************************************************/

#include <xtl.h>
#include "MXM.h"
#include "ItemInfo.h"
#include "CommDebug.h"
#include "xbmd5.h"

bool IsNTSC( void );

CXMLNode	*	CItemInfo::BuildNode( CXMLNode * pParentNode )
{
	CXMLNode * pItemNode;
	TMapStrings::iterator iterValues;

	pItemNode = pParentNode->AddNode( _T("item") );

	if ( pItemNode )
	{
		iterValues = m_mapValues.begin();
		while( iterValues != m_mapValues.end() )
		{
			pItemNode->SetString( NULL, iterValues->first.c_str(), iterValues->second.c_str() );
			iterValues++;
		}
	}
	return pItemNode;
}


bool CItemInfo::TryPasscode( LPCTSTR szPasscode )
{
	bool bReturn = true;
	
	if ( m_sPasscode.GetLength() && m_sPassMD5.GetLength()==0 )
	{
		CXBMD5 md5Pass( (BYTE *)m_sPasscode.c_str(), m_sPasscode.GetLength() );
		m_sPassMD5 = md5Pass.GetMD5String();
	}
	if ( m_sPassMD5.GetLength() )
	{
		CXBMD5 md5Try( (BYTE *)szPasscode, _tcslen(szPasscode) );

		if ( _tcsicmp( m_sPassMD5, md5Try.GetMD5String() ) )
		{
			DEBUG_LINE( _T("TryPasscode FAILED"));
			bReturn = false;
		}
		else
		{
			DEBUG_LINE( _T("TryPasscode SUCCEEDED"));
		}
	}
	//if ( ( m_sPasscode.GetLength() ) &&
	//	 ( m_sPasscode.Compare(szPasscode) ) )
	//{
	//	DEBUG_LINE( _T("TryPasscode FAILED"));
	//	bReturn = false;
	//}
	//else
	//{
	//	DEBUG_LINE( _T("TryPasscode SUCCEEDED"));
	//}
	m_bPasscodeOK = bReturn;
	return bReturn;
}

bool CItemInfo::CanRun( void )
{
	return m_bFileExists;
}

bool CItemInfo::HasPassed( void )
{
	bool bReturn = true;

	if ( m_bPasscodeOK == false )
	{
		// DEBUG_FORMAT( _T("Passcode Check %s/%s"), GetValue(szII_Title).c_str(), GetValue(szII_Passcode).c_str());
		if ( m_sPasscode.GetLength() > 0 || m_sPassMD5.GetLength() > 0 )
		{
			bReturn = m_bPasscodeOK;
		}
		else
		{
			// DEBUG_LINE( _T("Passcode Check - No Passcode"));
			m_bPasscodeOK = true;
		}
	}
	return bReturn;
}

CItemInfo::CItemInfo()
{
	Clear();
}

void CItemInfo::Clear()
{
//	m_iDescrWidth	= 0;
//	m_iTitleWidth	= 0;
	m_iCountryAction	= 0; // "Default"
//	m_iVideoAction		= 0;
	m_iVideoMode	= 0;
//	m_bNTSCMode		= IsNTSC();
	m_pThumbEntry	= NULL;
	m_bFileExists	= false;
	m_bPasscodeOK	= false;
	m_bIsMenu		= false;

	m_bIsAction		= false;
	m_pMenuNode		= NULL;
	m_sPasscode		= _T("");
	m_sPassMD5		= _T("");
	m_sAction		= _T("");
	m_mapValues.clear();
}

CItemInfo::CItemInfo( const CItemInfo &src )
{
	Clear();
	m_mapValues = src.m_mapValues;
//	m_iTitleWidth = src.m_iTitleWidth;
//	m_iDescrWidth = src.m_iDescrWidth;
	m_pThumbEntry = src.m_pThumbEntry;
	m_pMenuNode = src.m_pMenuNode;
	m_sPasscode = src.m_sPasscode;
	m_sPassMD5 = src.m_sPassMD5;
	m_bPasscodeOK = src.m_bPasscodeOK;
	m_sAction = src.m_sAction;
	m_bIsMenu = src.m_bIsMenu;
	m_bIsAction = src.m_bIsAction;
	m_bFileExists = src.m_bFileExists;
//	m_bNTSCMode	= src.m_bNTSCMode;
	m_iVideoMode = src.m_iVideoMode;
	m_iCountryAction	= src.m_iCountryAction;
//	m_iVideoAction		= src.m_iVideoAction;
}

CStdString CItemInfo::GetValue( LPCTSTR szKey )
{
	CStdString sReturn;
	TMapStrings::iterator itKey;
	CStdString sKey;

	if ( this )
	{
		sKey = szKey;
		sKey.MakeLower();
		if ( m_mapValues.size() )
		{
			itKey = m_mapValues.find( sKey );
			if ( itKey != m_mapValues.end() )
			{
				sReturn = itKey->second;
			}
		}
	}
	return sReturn;
}

CStdStringW CItemInfo::GetValueW( LPCTSTR szKey )
{
	CStdStringW swReturn;
	TMapStrings::iterator itKey;
	CStdString sKey;

	sKey = szKey;
	sKey.MakeLower();
	itKey = m_mapValues.find( sKey );
	if ( itKey != m_mapValues.end() )
	{
		swReturn = itKey->second;
	}	
	return swReturn;
}

void CItemInfo::SetValue( CStdString & sKey, CStdString & sValue )
{
	TMapStrings::iterator itKey;

	if ( this )
	{
		sKey.MakeLower();

		if ( sKey.GetLength() )
		{
			itKey = m_mapValues.find( sKey );
			if ( itKey != m_mapValues.end() )
			{
				if ( sValue.GetLength() )
				{
					itKey->second = sValue;
				}
				else
				{
					// Delete the key/value pair
					m_mapValues.erase( sKey );
				}
			}
			else
			{
				// Add the key/value if value is valid
				if ( sValue.GetLength() )
				{
					m_mapValues.insert( TMapStrings::value_type( sKey, sValue ));
				}
			}
		}
	}
}

void CItemInfo::SetValue( LPCTSTR szKey, LPCTSTR szValue )
{
	CStdString sKey, sValue;
	TMapStrings::iterator itKey;

	if ( szKey )
	{
		sKey = szKey;
		sKey.MakeLower();
		itKey = m_mapValues.find( sKey );
		if ( itKey != m_mapValues.end() )
		{
			if ( szValue )
			{
				sValue = szValue;
				itKey->second = sValue;
			}
			else
			{
				// Delete the key/value pair
				m_mapValues.erase( sKey );
			}
		}
		else
		{
			// Add the key/value if value is valid
			if ( szValue )
			{
				sValue = szValue;
				m_mapValues.insert( TMapStrings::value_type( sKey, sValue ));
			}
		}
	}
}
