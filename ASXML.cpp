

#include "StdAfx.h"
#include "ASXML.h"
#include "MenuInfo.h"

CStdString GetLocationPart( LPCTSTR szLocation, int iPart, int & iIndex )
{
	CStdString sReturn, sPart;
	CStdString sLoc(szLocation);
	int iPos;

	sLoc.Trim();
	iIndex=-1;
	iPos = sLoc.Find( _T('.') );
	if ( iPos >= 0 )
	{
		sPart = sLoc.Left( iPos );
		sPart.Trim();
		sLoc = sLoc.Mid( iPos+1 );
		sLoc.Trim();
		while( iPart && sLoc.GetLength() )
		{
			iPos = sLoc.Find( _T('.') );
			if ( iPos >= 0 )
			{
				sPart = sLoc.Left( iPos );
				sPart.Trim();
				sLoc = sLoc.Mid( iPos+1 );
				sLoc.Trim();
			}
			else
			{
				sPart = sLoc;
				sLoc = _T("");
			}
			iPart--;
		}
		if ( iPart ) // Didn't get to that part!
		{
			sPart = _T("");
			sReturn = _T("");
		}
		else
		{
			iPos = sPart.Find( _T(':') );
			if ( iPos >= 0 )
			{
				iIndex = _tcstoul( sPart.Mid( iPos+1 ), NULL, 10 );
				sReturn = sPart.Left( iPos );
			}
			else
			{
				sReturn = sPart;
				iIndex = 0;
			}
		}
	}
	else
	{
		if ( iPart == 0 )
		{
			sPart = sLoc;
			iPos = sPart.Find( _T(':') );
			if ( iPos >= 0 )
			{
				iIndex = _tcstoul( sPart.Mid( iPos+1 ), NULL, 10 );
				sReturn = sPart.Left( iPos );
			}
			else
			{
				sReturn = sPart;
				iIndex = 0;
			}
		}
		else
		{
			sPart = _T("");
		}
	}
	return sReturn;
}


void ASXML_Create( LPCTSTR szHandleName, LPCTSTR szMainNodeName, TXMLMap & xmlMap )
{
	if ( szHandleName && _tcslen( szHandleName ) )
	{
		ASXML_Close( szHandleName, xmlMap );
		
		CStdString sNewName(szMainNodeName);
		sNewName.MakeLower();
		CXMLNode * pNewNode = new CXMLNode( NULL, sNewName );
		
		if ( pNewNode )
		{
			xmlMap[szHandleName] = new CASXmlInfo( pNewNode, true );
		}
	}
}

void ASXML_Open( LPCTSTR szHandleName, LPCTSTR szFilename, LPCTSTR szPath, TXMLMap & xmlMap )
{
	if ( szHandleName && _tcslen( szHandleName ) )
	{
		ASXML_Close( szHandleName, xmlMap );
		xmlMap[szHandleName] = new CASXmlInfo( szFilename, szPath );
	}
}

void ASXML_Close( LPCTSTR szHandleName, TXMLMap & xmlMap )
{
	// Find szHandleName, if it's there... close old one and open new one in it's place
	TXMLMap::iterator iterXml;
	iterXml = xmlMap.find( szHandleName );
	if ( iterXml != xmlMap.end() )
	{
		// Close out old item
		delete iterXml->second;
		xmlMap.erase( iterXml );
	}
}

CASXmlInfo * ASXML_GetInfo( LPCTSTR szHandleName, TXMLMap & xmlMap )
{
	CASXmlInfo * pReturn = NULL;
	TXMLMap::iterator iterXml;
	iterXml = xmlMap.find( szHandleName );
	if ( iterXml != xmlMap.end() )
	{
		// Close out old item
		pReturn =  iterXml->second;
	}
	return pReturn;
}

void ASXML_ClearList( TXMLMap & xmlMap )
{
	CASXmlInfo * pReturn = NULL;
	TXMLMap::iterator iterXml;
	iterXml = xmlMap.begin();
	while ( iterXml != xmlMap.end() )
	{
		// Close out old item
		pReturn =  iterXml->second;
		if ( pReturn )
		{
			delete 	pReturn;
			iterXml->second = NULL;
		}
		iterXml++;
	}
	xmlMap.clear();
}


void ASXML_Save( LPCTSTR szHandleName, LPCTSTR szFilename, LPCTSTR szPath, TXMLMap & xmlMap )
{
	CASXmlInfo * pReturn = ASXML_GetInfo( szHandleName, xmlMap );
	
	if ( pReturn )
	{
		pReturn->Save( szFilename, szPath );
	}
	//if ( pReturn && pReturn->m_pMainNode )
	//{
	//	CStdString sFilename(szFilename);
	//	if ( sFilename.GetLength() == 0 )
	//	{
	//		// We use XML's filename
	//		if ( _tcsicmp( pReturn->m_sFilename, _T("::MXM") ) )
	//		{
	//		}
	//		else if ( _tcsicmp( m_sFilename, _T("::PREFS") ) )
	//		{
	//		}
	//		else if ( _tcsicmp( m_sFilename, _T("::BIOSMD5") ) )
	//		{
	//		}
	//		else if ( _tcsicmp( m_sFilename, _T("::MENU") ) )
	//		{
	//		}
	//		else if ( _tcsicmp( m_sFilename, _T("::MENU_CACHE") ) )
	//		{
	//		}
	//		else
	//		{
	//			sFilename = pReturn->m_sFilename;
	//		}
	//	}
	//
	//	if ( sFilename.GetLength() != 0 )
	//	{
	//		pReturn->m_pMainNode->SaveNode( szFilename );
	//	}
	//}
}


void ASXML_SaveNode( LPCTSTR szHandleName, LPCTSTR szFilename, LPCTSTR szPath, TXMLMap & xmlMap )
{
	CASXmlInfo * pReturn = ASXML_GetInfo( szHandleName, xmlMap );
	
	if ( pReturn )
	{
		pReturn->SaveNode( szFilename, szPath );
	}
	//&& pReturn->m_pCurrNode )
	//{
	//	if ( szFilename && _tcslen(szFilename) )
	//	{
	//		pReturn->m_pCurrNode->SaveNode( szFilename );
	//	}
	//}
}



CASXmlInfo::CASXmlInfo( void ) :
	m_pCurrNode(NULL)
	,m_pCurrElement(NULL)
	,m_pMainNode(NULL)
	,m_bAttached(false)
{
}

CASXmlInfo::~CASXmlInfo()
{
	if ( m_bAttached && m_pMainNode )
	{
		delete m_pMainNode;
	}
}

CASXmlInfo::CASXmlInfo( CXMLNode * pNode, bool bAttached ) :
	m_pCurrNode(NULL)
	,m_pCurrElement(NULL)
	,m_pMainNode(NULL)
	,m_bAttached(false)
{
	m_pMainNode = pNode;
	m_bAttached = bAttached;
}

CASXmlInfo::CASXmlInfo( const CASXmlInfo & src ) :
	m_pCurrNode(NULL)
	,m_pCurrElement(NULL)
	,m_pMainNode(NULL)
	,m_bAttached(false)
{
	if ( src.m_pMainNode && src.m_bAttached )
	{
		// Copy node!!
		m_pMainNode = new CXMLNode( NULL, src.m_pMainNode->m_sName.c_str() );
		*m_pMainNode = *(src.m_pMainNode);
		m_bAttached = true;
	}
	else
	{
		m_pCurrElement = src.m_pCurrElement;
		m_pCurrNode = src.m_pCurrNode;
		m_pMainNode = src.m_pMainNode;
		m_bAttached = false;
	}
	m_sFilename = src.m_sFilename;

}

CStdString GetInternalXMLFilename( LPCTSTR szFilename, LPCTSTR szPath )
{
	CStdString sReturn(szFilename);

	if ( sReturn.GetLength() )
	{
		if ( _tcsicmp( szFilename, _T("::MXM") ) == 0 )
		{
			sReturn = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("mxm.xml") );
		}
		else if ( _tcsicmp( szFilename, _T("::PREFS") ) == 0 )
		{
			sReturn = _T("U:\\pref.xml");
		}
		else if ( _tcsicmp( szFilename, _T("::BIOSMD5") ) == 0 )
		{
			sReturn = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("BIOSMD5.xml") );
		}
		else if ( _tcsicmp( szFilename, _T("::MENU") ) == 0 )
		{
			sReturn = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("menu.xml") );
		}
		else if ( _tcsicmp( szFilename, _T("::MENU_CACHE") ) == 0 )
		{
			sReturn = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("cache_menu.xml") );
		}
		else if ( _tcsnicmp( szFilename, _T("::"), 2 )  )
		{
			if ( szPath && _tcslen( szPath ) )
			{
				sReturn = MakeFullFilePath( szPath, szFilename );
			}
		}
	}
	return sReturn;
}

CASXmlInfo::CASXmlInfo( LPCTSTR szFilename, LPCTSTR szPath ) :
	m_sFilename( szFilename ),
	m_pCurrNode(NULL)
	,m_pCurrElement(NULL)
	,m_pMainNode(NULL)
	,m_bAttached(false)
{
	CXMLLoader xmlLoader(NULL,true);

	if ( _tcsicmp( m_sFilename, _T("::MXM") ) == 0 )
	{
		m_pMainNode = g_MenuInfo.m_pCfgInfo;
	}
	else if ( _tcsicmp( m_sFilename, _T("::PREFS") ) == 0 )
	{
		m_pMainNode = g_MenuInfo.m_pPrefInfo;
	}
	else if ( _tcsicmp( m_sFilename, _T("::BIOSMD5") ) == 0 )
	{
		m_sFilename = GetInternalXMLFilename( szFilename, szPath );
	}
	else if ( _tcsicmp( m_sFilename, _T("::MENU") ) == 0 )
	{
		m_sFilename = GetInternalXMLFilename( szFilename, szPath );
	}
	else if ( _tcsicmp( m_sFilename, _T("::MENU_CACHE") ) == 0 )
	{
		m_sFilename = GetInternalXMLFilename( szFilename, szPath );
	}
	if ( m_pMainNode == NULL )
	{
		m_pMainNode = xmlLoader.LoadXMLNodes( m_sFilename );
		if ( m_pMainNode )
		{
			xmlLoader.DetachMainNode();
			m_bAttached = true;
		}		
	}
}

bool CASXmlInfo::Save( LPCTSTR szFilename, LPCTSTR szPath )
{
	bool bReturn = false;
	CStdString sFilename = GetInternalXMLFilename( szFilename, szPath );

	if ( sFilename.GetLength() )
	{
		if ( CheckPointers(false) )
		{
			CXMLNode * pRootNode = m_pMainNode->GetRootNode();
			if ( pRootNode )
			{
				bReturn = pRootNode->SaveNode( sFilename );
			}
		}
	}
	return bReturn;
}

bool CASXmlInfo::SaveNode( LPCTSTR szFilename, LPCTSTR szPath )
{
	bool bReturn = false;
	CStdString sFilename = MakeFullFilePath( szFilename, szPath );

	if ( sFilename.GetLength() )
	{
		if ( CheckPointers(false) )
		{
			bReturn = m_pCurrNode->SaveNode( sFilename );
		}
	}
	return bReturn;
}

bool CASXmlInfo::SetNodePtr( LPCTSTR szNodeIndicator )
{
	CXMLNode * pNode = NULL;

	pNode = GetNodeByLocation( szNodeIndicator, m_pCurrNode );
	if ( pNode )
	{
		m_pCurrNode = pNode;
	}
	return pNode?true:false;
}

bool CASXmlInfo::CreateNode( LPCTSTR szNodeIndicator )
{
	CXMLNode * pNode = NULL;
	
	pNode = m_pCurrNode->AddNode( szNodeIndicator );
	if ( pNode )
	{
		m_pCurrNode = pNode;
		m_pCurrElement = pNode->GetElement(0);
	}
	return pNode?true:false;
}

bool CASXmlInfo::SetElementPtr( LPCTSTR szElementIndicator )
{
	int iPos, iIndex=0;
	CStdString sIndicator;
	bool bReturn = false;
	CXMLNode * pNode = m_pCurrNode;
	CXMLElement * pElement = m_pCurrElement;

	iPos = sIndicator.ReverseFind( _T('.') );
	if ( iPos >= 0  )
	{
		CStdString sNodeInd = sIndicator.Left( iPos );
		sIndicator = sIndicator.Mid( iPos+1 );
		pNode = GetNodeByLocation( szElementIndicator, m_pCurrNode );
		// SetNodePtr( sNodeInd );
	}
	iPos = sIndicator.Find( _T(':') );
	if( iPos >= 0 )
	{
		iIndex = _tcstol( sIndicator.Mid( iPos+1 ), NULL, 10 );
		sIndicator = sIndicator.Left( iPos );
	}
	if ( pNode )
	{
		if ( sIndicator.GetLength() )
		{
			if ( (pElement = pNode->GetElement(sIndicator, iIndex)) )
			{
				bReturn = true;
			}
		}
		else
		{
			if ( (pElement = pNode->GetElement(iIndex)) )
			{
				bReturn = true;
			}
		}
		if ( bReturn )
		{
			m_pCurrNode = pNode;
			m_pCurrElement = pElement;
		}
	}
	return bReturn;
}

bool CASXmlInfo::CreateElement( LPCTSTR szElementIndicator )
{
	CXMLElement * pElement = NULL;
	
	pElement = m_pCurrNode->AddElement( szElementIndicator );
	if ( pElement )
	{
		m_pCurrElement = pElement;
	}
	return pElement?true:false;
}

int CASXmlInfo::GetElementCount( LPCTSTR szName )
{
	int iReturn = 0;
	
	if ( CheckPointers(false) )
	{
		iReturn = m_pCurrNode->GetElementCount(szName);
	}
	
	return iReturn;
}

int CASXmlInfo::GetNodeCount( LPCTSTR szName )
{
	int iReturn = 0;
	
	if ( CheckPointers(false) )
	{
		iReturn = m_pCurrNode->GetNodeCount(szName);
	}
	
	return iReturn;
}

int CASXmlInfo::GetElementAttrCount( void )
{
	int iReturn = 0;
	
	if ( CheckPointers(true) )
	{
		iReturn = m_pCurrElement->m_msAttributes.size();
	}
	
	return iReturn;
}

int CASXmlInfo::GetNodeAttrCount( void )
{
	int iReturn = 0;
	
	if ( CheckPointers(false) )
	{
		iReturn = m_pCurrNode->m_msAttributes.size();
	}
	
	return iReturn;
}

bool CASXmlInfo::CheckPointers( bool bCheckElement )
{
	bool bReturn = false;
	if ( m_pCurrNode == NULL )
	{
		m_pCurrNode = m_pMainNode;
	}
	else
	{
		if ( !bCheckElement )
		{
			bReturn = true;
		}
	}
	if ( m_pCurrNode == NULL )
	{
		m_pCurrElement = NULL;
	}
	else
	{
		if ( m_pCurrElement == NULL )
		{
			m_pCurrElement = m_pCurrNode->GetElement(0);
		}
		if ( bCheckElement )
		{
			if ( m_pCurrElement )
			{
				bReturn = true;
			}
		}
		else
		{
			bReturn = true;
		}
	}
	return bReturn;
}

bool CASXmlInfo::SetElementValue( LPCTSTR szLocation, LPCTSTR szValue )
{
	bool bReturn = false;
	CStdString sLoc( szLocation );
	if ( sLoc.Find(_T('.')) < 0 )
	{
		// OK, seems to be a valid location
		int iIndex;
		sLoc = GetLocationPart( szLocation, 0, iIndex );
		if ( CheckPointers(false) )
		{
			CXMLElement * pElement;
		
			if ( sLoc.GetLength() )
			{
				pElement = m_pCurrNode->GetElement( sLoc, iIndex );
			}
			else
			{
				pElement = m_pCurrNode->GetElement( iIndex );
			}
			if ( pElement == NULL && sLoc.GetLength() )
			{
				pElement = m_pCurrNode->AddElement( sLoc );				
			}
			if ( pElement )
			{
				pElement->SetString( szValue );
				bReturn = true;
			}
		}
	}
	return bReturn;
}

bool CASXmlInfo::SetElementAttr( LPCTSTR szLocation, LPCTSTR szValue )
{
	bool bReturn = false;
	if ( CheckPointers(true) )
	{
		m_pCurrElement->SetString( szValue, szLocation );
	}
	return bReturn;
}

bool CASXmlInfo::SetNodeAttr( LPCTSTR szLocation, LPCTSTR szValue )
{
	bool bReturn = false;
	if ( CheckPointers(false) )
	{
		m_pCurrNode->SetString( NULL, szLocation, szValue, true );
	}
	return bReturn;
}

bool CASXmlInfo::SetValue( LPCTSTR szLocation, LPCTSTR szValue, bool bPrefAttr )
{
	bool bReturn = false;
	// Walk through the parts...
	CStdString sSection, sKey, sPart;

	if ( CheckPointers( false ) )
	{
		CXMLNode * pNode = m_pMainNode;
		CXMLNode * pTryNode = NULL;
		int iIndex, iPart = 1;
		int iKeyIndex = 0, iSectionIndex = 0;
		sPart = GetLocationPart( szLocation, 0, iIndex );
		if ( iIndex >= 0 ) // Has to be something there.... even if it's "nothing"
		{
			if ( sPart.GetLength() && (sPart[0] != _T('!')) && (szLocation[0]!=_T('.')) ) // Not root... hangs from current
			{
				pNode = m_pCurrNode;
			}
			else
			{
				sPart = GetLocationPart( szLocation, 1, iIndex );
				iPart = 2;
			}
			if ( iIndex >= 0 )
			{
				sKey = sPart;
				iKeyIndex = iIndex;
				sPart = GetLocationPart( szLocation, iPart++, iIndex );
				if ( iIndex >= 0 )
				{
					sSection = sKey;
					iSectionIndex = iKeyIndex;
					sKey = sPart;
					iKeyIndex = iIndex;
					sPart = GetLocationPart( szLocation, iPart++, iIndex );
					if ( iIndex >= 0 )
					{
						while( iIndex >= 0 && pNode )
						{
							if ( sSection.GetLength() )
							{
								pTryNode = pNode->GetNode( sSection, iSectionIndex );
								if ( pTryNode )
								{
									pNode = pTryNode;
								}
								else
								{
									pNode = pNode->AddNode( sSection );
								}
							}
							else
							{
								// If this fails, too bad... Nothing we can do!
								pNode = pNode->GetNode( iSectionIndex );
							}
							sSection = sKey;
							iSectionIndex = iKeyIndex;
							sKey = sPart;
							iKeyIndex = iIndex;
							sPart = GetLocationPart( szLocation, iPart++, iIndex );
						}
					}
					if ( pNode )
					{
						CXMLElement * pElement = NULL;

						if ( ( iSectionIndex > 0 ) || ( iKeyIndex > 0 ))
						{
							// More complicated...
							if ( iKeyIndex > 0 ) // Force element here...
							{
								pNode = pNode->GetNode( sSection, iSectionIndex );
								if ( pNode )
								{
									pElement = pNode->GetElement( sKey, iKeyIndex );
									if ( pElement )
									{
										pElement->m_sValue = szValue;
										bReturn = true;
									}
									else
									{
										pNode = pNode->GetNode( sKey, iKeyIndex );
										if ( pNode )
										{
											pNode->SetString( NULL, _T("value"), szValue );
											bReturn = true;
										}
									}
								}
							}
							else
							{
								pElement = pNode->GetElement( sSection, iSectionIndex );
								if ( pElement )
								{
									pElement->SetString( szValue, sKey );
									bReturn = true;
								}
								else
								{
									pNode = pNode->GetNode( sSection, iSectionIndex );
									if ( pNode )
									{
										pNode->SetString( NULL, sKey, szValue, bPrefAttr );
										bReturn = true;
									}
								}
							}
						}
						else
						{
							// No section or key index... 
							pNode->SetString( sSection, sKey, szValue, bPrefAttr );
							bReturn = true;
						}
					}
				}
				else
				{
					pNode->SetString( NULL, sKey, szValue, bPrefAttr );
					bReturn = true;
				}
			}
			else
			{
				// End of the line.... get the value here.
				pNode->SetString(NULL, NULL, szValue, bPrefAttr );
				bReturn = true;
			}
		}
	}	

	return bReturn;
}

CStdString CASXmlInfo::GetValue( LPCTSTR szLocation, LPCTSTR szDefault )
{
	CStdString sReturn(szDefault);

	// Walk through the parts...
	CStdString sSection, sKey, sPart;

	if ( CheckPointers( false ) )
	{
		CXMLNode * pNode = m_pMainNode;
		int iIndex, iPart = 1;
		int iKeyIndex = 0, iSectionIndex = 0;
		sPart = GetLocationPart( szLocation, 0, iIndex );
		if ( iIndex >= 0 ) // Has to be something there.... even if it's "nothing"
		{
			if ( sPart.GetLength() && (sPart[0] != _T('!')) && (szLocation[0]!=_T('.')) ) // Not root... hangs from current
			{
				pNode = m_pCurrNode;
			}
			else
			{
				sPart = GetLocationPart( szLocation, 1, iIndex );
				iPart = 2;
			}
			if ( iIndex >= 0 )
			{
				sKey = sPart;
				iKeyIndex = iIndex;
				sPart = GetLocationPart( szLocation, iPart++, iIndex );
				if ( iIndex >= 0 )
				{
					sSection = sKey;
					iSectionIndex = iKeyIndex;
					sKey = sPart;
					iKeyIndex = iIndex;
					sPart = GetLocationPart( szLocation, iPart++, iIndex );
					if ( iIndex >= 0 )
					{
						while( iIndex >= 0 && pNode )
						{
							if ( sSection.GetLength() )
							{
								pNode = pNode->GetNode( sSection, iSectionIndex );
							}
							else
							{
								pNode = pNode->GetNode( iSectionIndex );
							}
							sSection = sKey;
							iSectionIndex = iKeyIndex;
							sKey = sPart;
							iKeyIndex = iIndex;
							sPart = GetLocationPart( szLocation, iPart++, iIndex );
						}
					}
					if ( pNode )
					{
						CXMLElement * pElement = NULL;
						if ( sSection.GetLength() )
						{
							pElement = pNode->GetElement( sSection, iSectionIndex );
						}
						else
						{
							pElement = pNode->GetElement( iSectionIndex );
						}
						if ( pElement )
						{
							if ( pElement->HasValue( sKey ) )
							{
								sReturn = pElement->GetString( szDefault, sKey );
							}
							else
							{
								if ( sSection.GetLength() )
								{
									pNode = pNode->GetNode( sSection, iSectionIndex );
								}
								else
								{
									pNode = pNode->GetNode( iSectionIndex );
								}
								if ( pNode )
								{
									sReturn = pNode->GetString( NULL, sKey, szDefault );
								}
							}
						}
						else
						{
							if ( sSection.GetLength() )
							{
								pNode = pNode->GetNode( sSection, iSectionIndex );
							}
							else
							{
								pNode = pNode->GetNode( iSectionIndex );
							}
							if ( pNode )
							{
								sReturn = pNode->GetString( NULL, sKey, szDefault );
							}
						}
					}
				}
				else
				{
					sReturn = pNode->GetString(NULL, sKey, szDefault );
				}
			}
			else
			{
				// End of the line.... get the value here.
				sReturn = pNode->GetString(NULL, NULL, szDefault );
			}
		}
	}	
	return sReturn;
}

CXMLElement * CASXmlInfo::GetElementByLocation( LPCTSTR szLocation )
{
	CXMLElement * pElement = NULL;
	CStdString sLoc(szLocation);
	CStdString sName;
	CXMLNode * pNode = NULL;
	int iPos, iIndex;
	
	iPos = sLoc.ReverseFind( _T('.') );
	if ( iPos >= 0 )
	{
		// Named element
		sName = sLoc.Mid( iPos+1 );
		sLoc = sLoc.Left( iPos );
	}
	else
	{
		sName = sLoc;
		sLoc = _T("");
	}
	if ( sLoc.GetLength() )
	{
		pNode = GetNodeByLocation( sLoc, m_pCurrNode );
	}
	else
	{
		if ( CheckPointers( false ) )
		{
			pNode = m_pCurrNode;
		}
	}
	if ( pNode )
	{
		// Now get remainder...
		sName = GetLocationPart( sName, 0, iIndex );
		if ( sName.GetLength() )
		{
			pElement = pNode->GetElement( sName, iIndex );
		}
		else
		{
			pElement = pNode->GetElement( iIndex );
		}
	}
	return pElement;
}

CXMLNode * CASXmlInfo::GetNodeByLocation( LPCTSTR szLocation, CXMLNode * pCurrNode )
{
	CXMLNode * pNode = NULL;
	CStdString sLoc(szLocation), sIndex, sName;
	int iIndex, iPos;
	CXMLNode * pParent = NULL;

	if ( pCurrNode == NULL )
	{
		pCurrNode = m_pCurrNode;
	}

	if( sLoc.GetLength() && CheckPointers(false) )
	{
		if ( sLoc.Compare( _T("+") ) == 0 ) // Increment index...
		{
			if ( pParent = pCurrNode->GetParent() )
			{
				iIndex = pCurrNode->GetIndex(false);
				pNode = pParent->GetNode( iIndex+1 );
			}
		}
		if ( sLoc.Compare( _T(".+") ) == 0 ) // Increment index...
		{
			if ( pParent = pCurrNode->GetParent() )
			{
				iIndex = pCurrNode->GetIndex(true);
				pNode = pParent->GetNode( pCurrNode->m_sName, iIndex+1 );
			}
		}
		else if ( sLoc.Compare( _T("-") ) == 0 ) // Decrement Index
		{
			if ( pParent = pCurrNode->GetParent() )
			{
				iIndex = pCurrNode->GetIndex(false);
				if ( iIndex > 0 )
				{
					pNode = pParent->GetNode( iIndex-1 );
				}
			}
		}
		else if ( sLoc.Compare( _T(".-") ) == 0 ) // Decrement Named Index
		{
			if ( pParent = pCurrNode->GetParent() )
			{
				iIndex = pCurrNode->GetIndex(true);
				if ( iIndex > 0 )
				{
					pNode = pParent->GetNode( pCurrNode->m_sName, iIndex-1 );
				}
			}
		}
		else if ( sLoc.Compare( _T(":") ) == 0 ) // First Index
		{
			if ( pParent = pCurrNode->GetParent() )
			{
				pNode = pParent->GetNode( 0 );
			}
		}
		else if ( sLoc.Compare( _T(".:") ) == 0 ) // First Index
		{
			if ( pParent = pCurrNode->GetParent() )
			{
				pNode = pParent->GetNode( pCurrNode->m_sName, 0 );
			}
		}
		else if ( sLoc[0] == _T('!') ) // From Root....
		{
			iPos = sLoc.Find( _T('.') );
			if ( iPos > 0 )
			{
				sLoc = sLoc.Mid(iPos+1);
			}
			else
			{
				sLoc = sLoc.Mid(1);
			}
			if ( sLoc.GetLength() )
			{
				pNode = GetNodeByLocation( sLoc, m_pMainNode->GetRootNode() );
			}
		}
		else if ( sLoc[0] == _T('@') ) // From Main....
		{
			iPos = sLoc.Find( _T('.') );
			if ( iPos > 0 )
			{
				sLoc = sLoc.Mid(iPos+1);
			}
			else
			{
				sLoc = sLoc.Mid(1);
			}
			if ( sLoc.GetLength() )
			{
				pNode = GetNodeByLocation( sLoc, m_pMainNode );
			}
		}
		else if ( sLoc[0] == _T(':') ) // Indexed From current, non-named....
		{
			sLoc = sLoc.Mid(1);
			if ( sLoc.GetLength() )
			{
				iPos = sLoc.Find( _T('.') );
				if ( iPos >= 0 )
				{
					if ( iPos )
					{
						sIndex = sLoc.Left( iPos );
						iIndex = _tcstol( sIndex, NULL, 10 );
					}
					else
					{
						iIndex = 0;
					}
					sLoc = sLoc.Mid( iPos+1 );
				}
				else
				{
					iIndex = 0;
					sLoc = _T("");
				}
				pParent = pCurrNode->GetParent();
				if ( pParent )
				{
					pNode = pParent->GetNode( iIndex );
				}
				else if ( iIndex == 0 )
				{
					pNode = pCurrNode;
				}
				else
				{
					pNode = NULL;
				}
				if ( pNode && sLoc.GetLength() )
				{
					pNode = GetNodeByLocation( sLoc, pNode );
				}
				
			}
			else
			{
				// First item!
				pParent = pCurrNode->GetParent();
				if ( pParent )
				{
					pNode = pParent->GetNode( 0 );
				}
			}
//			pNode = GetNodeByLocation( sLoc, pCurrNode );
		}
		else if ( _tcsncmp( sLoc, _T(".:"), 2 ) == 0 ) // Indexed From current, named....
		{
//			sLoc = sLoc.Mid(1);
//			pNode = GetNodeByLocation( sLoc, pCurrNode );
			sLoc = sLoc.Mid(2);
			if ( sLoc.GetLength() )
			{
				iPos = sLoc.Find( _T('.') );
				if ( iPos >= 0 )
				{
					if ( iPos )
					{
						sIndex = sLoc.Left( iPos );
						iIndex = _tcstol( sIndex, NULL, 10 );
					}
					else
					{
						iIndex = 0;
					}
					sLoc = sLoc.Mid( iPos+1 );
				}
				else
				{
					iIndex = 0;
					sLoc = _T("");
				}
				pParent = pCurrNode->GetParent();
				if ( pParent )
				{
					pNode = pParent->GetNode( pCurrNode->m_sName, iIndex );
				}
				else if ( iIndex == 0 )
				{
					pNode = pCurrNode;
				}
				else
				{
					pNode = NULL;
				}
				if ( pNode && sLoc.GetLength() )
				{
					pNode = GetNodeByLocation( sLoc, pNode );
				}
				
			}
			else
			{
				// First item!
				pParent = pCurrNode->GetParent();
				if ( pParent )
				{
					pNode = pParent->GetNode( pCurrNode->m_sName, 0 );
				}
			}
		}
		else if ( sLoc[0] == _T('.') ) // Hmmm... this is not right, but we'll allow it
		{
			sLoc = sLoc.Mid(1);
			if ( sLoc.GetLength() )
			{
				pNode = GetNodeByLocation( sLoc, pCurrNode );
			}
			else
			{
				pNode = pCurrNode;
			}
		}
		else
		{
			// OK, this should be a NAMED item
			iPos = sLoc.Find( _T('.') );
			if ( iPos >= 0 )
			{
				sName = sLoc.Left( iPos );
				sLoc = sLoc.Mid( iPos+1 );
			}
			else
			{
				sName = sLoc;
				sLoc = _T("");
			}
			if ( sName.GetLength() )
			{
				iPos = sName.Find( _T(":") );
				iIndex = 0;
				if ( iPos >= 0 )
				{
					sIndex = sName.Mid( iPos+1 );
					sName = sName.Left( iPos );
					iIndex = _tcstol( sIndex, NULL, 10 );
				}
				pNode = pCurrNode->GetNode( sName, iIndex );
				if ( sLoc.GetLength() )
				{
					pNode = GetNodeByLocation( sLoc, pNode );
				}
			}
		}
	}
	return pNode;
}

CStdString CASXmlInfo::GetCurrentNodeLocation( void )
{
	CStdString sReturn, sTemp;
	int iCount;
	
	if ( CheckPointers( false ) )
	{
		CXMLNode * pNode = m_pCurrNode;
		CXMLNode * pParent;
		// sReturn = pNode->m_sName;
		while( pNode )
		{
			pParent = pNode->GetParent();
			iCount = pParent->GetNodeCount( pNode->m_sName );
			if ( iCount > 1 )
			{
				iCount = pNode->GetIndex( true );
				sTemp.Format( _T("%s:%d"), pNode->m_sName.c_str(), iCount );
			}
			else
			{
				sTemp = pNode->m_sName;
			}
			if ( sReturn.GetLength() )
			{
				sReturn.Format( _T("%s.%s"), sTemp.c_str(), sReturn.c_str() );
				// sReturn = sTemp + sReturn;
			}
			else
			{
				sReturn = sTemp;
			}
			pNode = pParent;
		}
		sReturn.Format( _T("!%s"), sReturn.c_str() );
		
	}	
	return sReturn;
}

CStdString CASXmlInfo::GetCurrentElementLocation( void )
{
	CStdString sReturn, sTemp;
	
	if ( CheckPointers( true ) )
	{
		sReturn = GetCurrentNodeLocation();
		if ( m_pCurrNode->GetElementCount( m_pCurrElement->m_sName ) > 1 )
		{
			sTemp.Format( _T("%s:%d"), m_pCurrElement->m_sName.c_str(), m_pCurrElement->GetIndex(true) );
			sReturn += _T(".");
			sReturn += sTemp;
		}
		else
		{
			sReturn += _T(".");
			sReturn += m_pCurrElement->m_sName;
		}
	}
	else
	{
		sReturn = _T(".");
	}
	return sReturn;
}
