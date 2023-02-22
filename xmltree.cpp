/*********************************************************************************\
 * xmltree.cpp
 * Support for XML tree nodes and elements 
 * (C) 2002 MasterMind
\*********************************************************************************/

#include "MXM.h"
#include "xmltree.h"
#include "CommDebug.h"


#define XDBG_XML

CXMLLoader::CXMLLoader(CXMLNode	* pMainNode, bool bLowerIDs ) :
	m_bLowerIDs(bLowerIDs),
	m_iLevel(0),
	m_bAutoDelete(false),
	m_pMainElementNode(pMainNode),
	m_pCurrentElementNode(NULL)
{
	if ( m_pMainElementNode == NULL )
	{
		m_bAutoDelete = true;
		m_pMainElementNode = new CXMLNode();
	}
}

CXMLLoader::~CXMLLoader() 
{ 
	if ( m_bAutoDelete && m_pMainElementNode )
	{
		delete m_pMainElementNode; 
	}
}

void	CXMLLoader::DetachMainNode( void )
{
	m_bAutoDelete = false;
}

CXMLNode	*   CXMLLoader::GetRootNode( void ) 
{ 
	return m_pMainElementNode;
}


void CXMLLoader::foundNode		( string & name, string & attributes )
{
	TMapStrings msAttributes;
	CStdString sName(name);

	if ( m_bLowerIDs )
	{
		sName.MakeLower();
	}

#ifdef DBG_XML
	DEBUG_FORMAT( _T("%*sfoundNode: %s (%s)"), m_iLevel, "", name.c_str(), attributes.c_str() );
#endif
	ParseXMLAttributes( attributes, msAttributes, m_bLowerIDs );

	if ( m_iLevel == 0 )
	{
		if ( m_pMainElementNode )
		{
			m_pCurrentElementNode = m_pMainElementNode;
			m_pCurrentElementNode->m_sName = sName;
		}
	}
	else
	{
		m_pCurrentElementNode = m_pCurrentElementNode->AddNode( sName );
	}
	if ( m_pCurrentElementNode )
	{
		m_pCurrentElementNode->m_msAttributes = msAttributes;
	}
	m_iLevel++;
}

void CXMLLoader::terminateNode  ( string & name, string & attributes )
{
#ifdef DBG_XML
	DEBUG_FORMAT( _T("%*sterminateNode: %s (%s)"), m_iLevel, "", name.c_str(), attributes.c_str() );
#endif
	if ( m_iLevel )
	{
		m_iLevel--;
	}
	if( m_pCurrentElementNode->GetParent() )
	{
		m_pCurrentElementNode = m_pCurrentElementNode->GetParent();
	}
}

void CXMLLoader::foundElement	( string & name, string & value, string & attributes )
{
	TMapStrings msAttributes;
	CXMLElement *	pElement;
	CStdString sName(name);
	CStdString sValue(value);

	// Strip whitespace off ends
//	sValue.TrimLeft();
//	sValue.TrimRight();

	if ( m_bLowerIDs )
	{
		sName.MakeLower();
	}

	UnScramble( sValue );

#ifdef DBG_XML
	DEBUG_FORMAT( _T("%*s-foundElement: %s (%s) (%s)"), m_iLevel, "", name.c_str(), value.c_str(), attributes.c_str() );
#endif

	ParseXMLAttributes( attributes, msAttributes, m_bLowerIDs  );
	if ( m_pCurrentElementNode )
	{
		pElement = m_pCurrentElementNode->AddElement(sName, sValue);
		pElement->m_msAttributes = msAttributes;
	}
}

void CXMLLoader::startElement	( string & name, string & value, string & attributes )
{
#ifdef DBG_XML
	DEBUG_FORMAT( _T("%*s-startElement: %s (%s)"), m_iLevel, "", name.c_str(), attributes.c_str() );
#endif
}

void CXMLLoader::endElement		( string & name, string & value, string & attributes )
{
#ifdef DBG_XML
	DEBUG_FORMAT( _T("%*s-endElement: %s (%s)"), m_iLevel, "", name.c_str(), attributes.c_str() );
#endif
}

CXMLNode	*   CXMLLoader::LoadXMLNodesFromBuffer( LPCSTR szBuffer )
{
	if ( m_pMainElementNode )
	{
		if ( szBuffer && _tcslen( szBuffer ) )
		{
			DWORD dwSize = _tcslen( szBuffer );

			if ( dwSize )
			{
				XmlStream xml;
				xml.setSubscriber( *this );
				xml.parse( (char *)szBuffer, dwSize );
			}
		}
	}
	return m_pMainElementNode;
}


CXMLNode *   CXMLLoader::LoadXMLNodes( LPCTSTR szFileName )
{
	if ( m_pMainElementNode )
	{
		ParseFile(szFileName, *this );
	}
	return m_pMainElementNode;
}




CXMLNodeProxy::CXMLNodeProxy(CXMLNode * pParent, LPCTSTR szName )
{
	pNode = new CXMLNode(pParent,szName);
}

CXMLNodeProxy::CXMLNodeProxy(CXMLNode & node)
{
	pNode = new CXMLNode;
	*pNode = node;
}

CXMLNodeProxy::CXMLNodeProxy(const CXMLNodeProxy & nodeproxy)
{
	pNode = new CXMLNode(*nodeproxy.pNode);
}

CXMLNodeProxy::operator CXMLNode&()
{
	return *pNode;
}

CXMLElement::CXMLElement( CXMLNode * pParent, LPCTSTR szName, LPCTSTR szValue ) : 
	m_pParent(pParent), 
	m_sName(szName), 
	m_sValue(szValue) 
{
	m_msAttributes.clear();
}

CXMLElement::CXMLElement( const CXMLElement& src )
{
	m_sName = src.m_sName;
	m_sValue = src.m_sValue;
	m_msAttributes = src.m_msAttributes;
	m_pParent = src.m_pParent;
}


CXMLNode::CXMLNode(CXMLNode * pParent, LPCTSTR szName ) : 
	m_pParent(pParent), 
	m_sName(szName)
{
	m_msAttributes.clear();
	m_msElements.clear();
	m_nlNodes.clear();

	m_sName.MakeLower();
}

CXMLNode::~CXMLNode()
{
//	m_msAttributes.clear();
//	m_msElements.clear();
//	m_nlNodes.clear();
}

CXMLNode::CXMLNode(const CXMLNode& node)
{
	m_pParent = node.m_pParent;
	m_sName = node.m_sName;
	m_msAttributes = node.m_msAttributes;
	m_msElements = node.m_msElements;
	m_nlNodes = node.m_nlNodes;
}


CXMLNodeProxy::~CXMLNodeProxy()
{
	if ( pNode )
	{
		delete pNode;
	}
}

CXMLNode *	CXMLNode::GetRootNode( void )
{
	CXMLNode * pRoot = this;

	while( pRoot && pRoot->m_pParent )
	{
		pRoot = pRoot->m_pParent;
	}
	return pRoot;
}

int	CXMLNode::GetIndex( bool bNamed )
{
	CXMLNode * pNode;
	int iReturn = 0;
	
	if ( m_pParent )
	{
		if ( bNamed )
		{
			pNode = m_pParent->GetNode( m_sName, 0 );
			while( pNode && pNode != this )
			{
				iReturn++;
				pNode = m_pParent->GetNode( m_sName, iReturn );
			}
		}
		else
		{
			pNode = m_pParent->GetNode( 0 );
			while( pNode && pNode != this )
			{
				iReturn++;
				pNode = m_pParent->GetNode( iReturn );
			}
		}
	}
	return iReturn;
}

int	CXMLNode::GetElementCount( LPCTSTR szName )
{
	int iReturn = 0;
	CStdString sName(szName);

	if ( this )
	{

		if ( szName && _tcslen(szName)  )
		{
			sName.MakeLower();
			CXMLElement * pElement;
			TXMLElementList::iterator iterElement;

			iterElement = m_msElements.begin();
			while( iterElement != m_msElements.end() )
			{
				pElement = iterElement->GetElementPtr();
				if ( pElement->m_sName.Compare( sName ) == 0 )
				{
					iReturn++;
				}
				iterElement++;
			}
		}
		else
		{
			iReturn = m_msElements.size();
		}
	}
	return iReturn;
}


int	CXMLNode::GetNodeCount( LPCTSTR szName )
{
	int iReturn = 0;
	CStdString sName(szName);

	if ( this )
	{
		if ( szName && _tcslen(szName) )
		{
			sName.MakeLower();
			CXMLNode * pNode;
			TXMLNodeList::iterator iterNode;

			iterNode = m_nlNodes.begin();
			while( iterNode != m_nlNodes.end() )
			{
				pNode = iterNode->GetNodePtr();
				if ( pNode->m_sName.Compare( sName ) == 0 )
				{
					iReturn++;
				}
				iterNode++;
			}
		}
		else
		{
			iReturn = m_nlNodes.size();
		}
	}
	return iReturn;
}

CStdString	CXMLNode::GetSubValue( LPCTSTR szKey, LPCTSTR szSubKey )
{
	CStdString sValue;
	CXMLNode * pSubNode = NULL;
	CXMLElement * pSubElement = NULL;
	CStdString sKey;
	CStdString sSubKey;

	if ( this )
	{
		sSubKey = szSubKey;
		sSubKey.MakeLower();
		sKey = szKey;
		sKey.MakeLower();

		if ( szSubKey == NULL )
		{
			sValue = GetBasicValue( sKey );
		}
		else
		{
			pSubNode = GetNode(sKey);
			if ( pSubNode )
			{
				sValue = pSubNode->GetBasicValue( sSubKey );
			}
			if ( sValue.GetLength() == 0 )
			{
				pSubElement = GetElement( sKey );
				if ( pSubElement )
				{
					sValue = pSubElement->GetBasicValue( sSubKey );
				}
			}
		}
	}
	return sValue;
}

void CXMLNode::DeleteNode( CXMLNode * pNode )
{
	TXMLNodeList::iterator iterNode;

	if ( this )
	{
		iterNode = m_nlNodes.begin();
		while( iterNode != m_nlNodes.end() )
		{
			if ( iterNode->GetNodePtr() == pNode )
			{
				m_nlNodes.erase(iterNode);
				break;
			}
			iterNode++;
		}
	}
}

void CXMLNode::DeleteElement( CXMLElement * pElement )
{
	TXMLElementList::iterator iterElement;

	if ( this )
	{
		iterElement = m_msElements.begin();
		while( iterElement != m_msElements.end() )
		{
			if ( iterElement->GetElementPtr() == pElement )
			{
				m_msElements.erase(iterElement);
				break;
			}
			iterElement++;
		}
	}
}

bool CXMLNode::HasValue( LPCTSTR szSection, LPCTSTR szKey )
{
	bool bReturn = false;
	CXMLNode * pSectionNode = this;
	CXMLNode * pKeyNode = NULL;
	CXMLElement * pKeyElement;
	CStdString sSection(szSection);
	CStdString sKey(szKey);

	if (this )
	{
		sKey.MakeLower();
		sSection.MakeLower();

		if ( szSection )
		{
			pSectionNode = GetNode( sSection );
		}
		if ( pSectionNode )
		{
			// Check for element first....
			pKeyElement = pSectionNode->GetElement( sKey );
			if ( pKeyElement )
			{
				// Element exists...
				// Definitely has SOME sort of value!!
				bReturn = true;
			}
			// Now for node....
			if ( !bReturn )
			{
				pKeyNode = pSectionNode->GetNode( sKey );
				if ( pKeyNode )
				{
					bReturn = pKeyNode->HasValue( NULL, _T("val") );
					if ( !bReturn )
					{
						bReturn = pKeyNode->HasValue( NULL, _T("value") );
					}
				}
			}
			// Finally, for attribute
			if ( !bReturn )
			{
				TMapStrings::iterator iterAttrib;

				iterAttrib = pSectionNode->m_msAttributes.find( sKey );
				if ( iterAttrib != pSectionNode->m_msAttributes.end() )
				{
					// Found it....
					bReturn = true;
				}
			}
		}
		else
		{
			CXMLElement * pSectionElement;
			// Check for element first....
			pSectionElement = GetElement( sSection );
			if ( pSectionElement )
			{
				// Finally, for attribute
				if ( !bReturn )
				{
					TMapStrings::iterator iterAttrib;

	//				if ( pSectionElement->m_msAttributes.size() )
	//				{
						iterAttrib = pSectionElement->m_msAttributes.find( sKey );
						if ( iterAttrib != pSectionElement->m_msAttributes.end() )
						{
							// Found it....
							bReturn = true;
						}
	//				}
				}
				// bReturn = true;
			}
		}
	}
	return bReturn;
}

CStdString CXMLNode::GetString( LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szDefault, bool bTrim, bool bLower )
{
	CStdString sReturn = szDefault;

	if ( this )
	{
		if ( HasValue( szSection, szKey ) )
		{
			if ( szSection && _tcslen(szSection) )
			{
				sReturn = GetSubValue( szSection, szKey );
			}
			else
			{
				sReturn = GetBasicValue( szKey );
			}
		}
		if ( bTrim )
		{
			sReturn.Trim();
		}
		if ( bLower )
		{
			sReturn.MakeLower();
		}
	}
	return sReturn;
}

BOOL	CXMLNode::GetBool( LPCTSTR szSection, LPCTSTR szKey, BOOL bDefault )
{
	BOOL bReturn = bDefault;
	CStdString sValue;

	sValue = GetString( szSection, szKey, _T("") );
	if ( sValue.GetLength() )
	{
		sValue.Trim();
		sValue.MakeLower();
		if ( sValue.Compare( _T("on") ) == 0 )
		{
			bReturn = TRUE;
		}
		else if ( sValue.Compare( _T("yes") ) == 0 )
		{
			bReturn = TRUE;
		}
		else if ( sValue.Compare( _T("true") ) == 0 )
		{
			bReturn = TRUE;
		}
		else if ( ConvertUnsigned(sValue) != 0 )
		{
			bReturn = TRUE;
		}
		else if ( sValue.Compare( _T("off") ) == 0 )
		{
			bReturn = FALSE;
		}
		else if ( sValue.Compare( _T("no") ) == 0 )
		{
			bReturn = FALSE;
		}
		else if ( sValue.Compare( _T("false") ) == 0 )
		{
			bReturn = FALSE;
		}
		else if ( isdigit(sValue[0]) )
		{
			bReturn = FALSE;
		}

	}
	return bReturn;
}

long	CXMLNode::GetLong( LPCTSTR szSection, LPCTSTR szKey, long lDefault )
{
	long lReturn = lDefault;
	CStdString sValue;

	sValue = GetString( szSection, szKey, _T("") );
	if ( sValue.GetLength() )
	{
		sValue.Trim();
		sValue.MakeLower();
		lReturn = ConvertSigned( sValue );
	}
	return lReturn;
}

DWORD	CXMLNode::GetDWORD( LPCTSTR szSection, LPCTSTR szKey, DWORD dwDefault )
{
	DWORD dwReturn = dwDefault;
	CStdString sValue;

	sValue = GetString( szSection, szKey, _T("") );
	if ( sValue.GetLength() )
	{
		sValue.Trim();
		sValue.MakeLower();
		dwReturn = ConvertUnsigned( sValue );
	}
	return dwReturn;
}

void	CXMLNode::SetString( LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szValue, bool bAttr )
{
	CXMLNode * pSectionNode = this;
	CXMLElement * pSectionElement = NULL;
	CStdString sSection(szSection), sKey(szKey);

	CStdString sValue(szValue);

//	Scramble(sValue);

	if ( this )
	{
		sSection.MakeLower();
		sKey.MakeLower();
		if ( szSection && _tcslen( szSection ) )
		{
			// Get node... or make node.
			pSectionNode = GetNode( sSection );
			if ( !pSectionNode )
			{
				// Try this, if we have our preference set for attributes...
				if ( bAttr )
				{
					pSectionElement = GetElement( sSection );
				}
				if ( pSectionElement == NULL )
				{
					pSectionNode = AddNode( sSection );
				}
			}
			if ( pSectionNode )
			{
				pSectionNode->SetString( NULL, sKey, szValue, bAttr );
			}
			else if ( pSectionElement )
			{
				pSectionElement->SetString( szValue, sKey );
			}
		}
		else
		{
			// set string locally....
			CXMLElement * pElement = NULL;

			if ( szKey && _tcslen(szKey) )
			{
				pElement = GetElement(sKey);
				if ( pElement )
				{
					pElement->SetString( szValue );
				}
				else
				{
					CXMLNode * pNode = NULL;
					pNode = GetNode( sKey );
					if ( pNode )
					{
						pNode->SetString( NULL, NULL, szValue );
					}
					else
					{
						TMapStrings::iterator iterAttrib;

						iterAttrib = m_msAttributes.find( sKey );
						if ( iterAttrib != m_msAttributes.end() )
						{
							iterAttrib->second = sValue;
						}
						else
						{
							if ( bAttr )
							{
								m_msAttributes[sKey] = sValue;
							}
							else
							{
								pElement = AddElement( sKey, szValue );
							}
						}
					}
				}
			}
			else
			{
				// Local
				if ( HasValue( NULL, _T("val") ) )
				{
					SetString( NULL, _T("val"), szValue, bAttr );
				}
				else if ( HasValue( NULL, _T("value") ) )
				{
					SetString( NULL, _T("val"), szValue, bAttr );
				}
				else
				{
					// Settle on this one
					SetString( NULL, _T("val"), szValue, bAttr );
				}
			}
		}
	}
}

void	CXMLNode::SetBool( LPCTSTR szSection, LPCTSTR szKey, BOOL bValue, bool bAttr )
{
	CStdString sValue;

	if ( bValue )
	{
		sValue = _T("true");
	}
	else
	{
		sValue = _T("false");
	}
	SetString( szSection, szKey, sValue, bAttr );
}

void	CXMLNode::SetLong( LPCTSTR szSection, LPCTSTR szKey, long lValue, bool bAttr )
{
	CStdString sValue;

	sValue.Format( _T("%ld"), lValue );
	SetString( szSection, szKey, sValue, bAttr );
}

void	CXMLNode::SetDWORD( LPCTSTR szSection, LPCTSTR szKey, DWORD dwValue, bool bAttr )
{
	CStdString sValue;

	sValue.Format( _T("0x%08x"), dwValue );
	SetString( szSection, szKey, sValue, bAttr );
}




CStdString	CXMLNode::GetBasicValue( LPCTSTR szKey )
{
	CStdString sValue;
	CStdString sKey;
	int iIndex;
	CXMLElement *	pElement	= NULL;
	CXMLNode *		pNode		= NULL;

	if ( this )
	{
		if ( szKey && _tcslen( szKey ) )
		{
			sKey = szKey;
			sKey.MakeLower();
			// First look in attributes:
			sValue = m_msAttributes[sKey];
			if ( sValue.GetLength() == 0 )
			{
				// Now look in elements, check each one
				iIndex = 0;
				pElement = GetElement( sKey, iIndex );
				while( pElement && sValue.GetLength()==0 )
				{
					sValue = pElement->GetBasicValue();
					iIndex++;
					pElement = GetElement( sKey, iIndex );
				}
			}

			if ( sValue.GetLength() == 0 )
			{
				// Now look in nodes, check each one
				iIndex = 0;
				pNode = GetNode( sKey, iIndex );
				while( pNode && sValue.GetLength()==0 )
				{
					sValue = pNode->GetBasicValue( _T("val") );
					if ( sValue.GetLength() == 0 )
					{
						sValue = pNode->GetBasicValue( _T("value") );
					}
					iIndex++;
					pNode = GetNode( sKey, iIndex );
				}
			}
		}
		else
		{
			sValue = GetBasicValue( _T("val") );
			if ( sValue.GetLength() == 0 )
			{
				sValue = GetBasicValue( _T("value") );
			}
		}
	}

//	UnScramble(sValue);
	return sValue;
}



CStdString CXMLElement::GetBasicValue( LPCTSTR szLabel )
{
	CStdString sReturn;
	CStdString sLabel(szLabel);

	if ( this )
	{
		if ( szLabel == NULL )
		{
			if( m_sValue.GetLength() )
			{
				sReturn = m_sValue;
			}
			else
			{
				sReturn = m_msAttributes[_T("val")];
				if ( sReturn.GetLength() == 0 )
				{
					sReturn = m_msAttributes[_T("value")];
				}
			}
		}
		else
		{
			sLabel.MakeLower();
			
			sReturn = m_msAttributes[sLabel];
		}
	}
//	UnScramble(sReturn);
	return sReturn;
}


bool CXMLElement::HasValue( LPCTSTR szAttrKey )
{
	bool bReturn = false;
	CStdString sAttrKey(szAttrKey);

	if ( this )
	{
		if ( szAttrKey && _tcslen( szAttrKey ) )
		{
			sAttrKey.MakeLower();
			TMapStrings::iterator iterAttrib;
			// Verify the value is in there... in this case, part of the 
			// mapped strings.
			iterAttrib = m_msAttributes.find( sAttrKey );
			if ( iterAttrib != m_msAttributes.end() )
			{
				// Found it....
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

CStdString CXMLElement::GetString( LPCTSTR szDefault, LPCTSTR szAttrKey, bool bTrim, bool bLower )
{
	CStdString sReturn;

	if ( this )
	{
		if ( HasValue( szAttrKey ) )
		{
			sReturn = GetBasicValue( szAttrKey );
		}
		else
		{
			sReturn = szDefault;
		}
		if ( bTrim )
		{
			sReturn.Trim();
		}
		if ( bLower )
		{
			sReturn.MakeLower();
		}
	}
	return sReturn;
}

BOOL	CXMLElement::GetBool( BOOL bDefault, LPCTSTR szAttrKey )
{
	BOOL bReturn = bDefault;
	CStdString sValue;

	sValue = GetString( _T(""), szAttrKey );
	if ( sValue.GetLength() )
	{
		sValue.MakeLower();
		sValue.Trim();
		if ( sValue.Compare( _T("on") ) == 0 )
		{
			bReturn = TRUE;
		}
		else if ( sValue.Compare( _T("yes") ) == 0 )
		{
			bReturn = TRUE;
		}
		else if ( sValue.Compare( _T("true") ) == 0 )
		{
			bReturn = TRUE;
		}
		else if ( ConvertUnsigned(sValue) != 0 )
		{
			bReturn = TRUE;
		}
		else if ( sValue.Compare( _T("off") ) == 0 )
		{
			bReturn = FALSE;
		}
		else if ( sValue.Compare( _T("no") ) == 0 )
		{
			bReturn = FALSE;
		}
		else if ( sValue.Compare( _T("false") ) == 0 )
		{
			bReturn = FALSE;
		}
		else if ( isdigit(sValue[0]) )
		{
			bReturn = FALSE;
		}

	}
	return bReturn;
}

long	CXMLElement::GetLong( long lDefault, LPCTSTR szAttrKey )
{
	long lReturn = lDefault;
	CStdString sValue;

	sValue = GetString( _T(""), szAttrKey );
	if ( sValue.GetLength() )
	{
		sValue.MakeLower();
		sValue.Trim();
		lReturn = ConvertSigned( sValue );
	}
	return lReturn;
}

DWORD	CXMLElement::GetDWORD( DWORD dwDefault, LPCTSTR szAttrKey )
{
	DWORD dwReturn = dwDefault;
	CStdString sValue;

	sValue = GetString( _T(""), szAttrKey );
	if ( sValue.GetLength() )
	{
		sValue.MakeLower();
		sValue.Trim();
		dwReturn = ConvertUnsigned( sValue );
	}
	return dwReturn;
}

void	CXMLElement::SetString( LPCTSTR szValue, LPCTSTR szAttrKey )
{
	bool bHasVal, bHasValue;
	CStdString sAttrKey(szAttrKey);
	CStdString sValue(szValue);

//	Scramble( sValue );

	if ( this )
	{
		if ( szAttrKey && _tcslen(szAttrKey) )
		{
			sAttrKey.MakeLower();
			m_msAttributes[sAttrKey] = sValue;
		}
		else
		{
			bHasValue = HasValue( _T("value") );
			if ( bHasValue )
			{
				if ( GetBasicValue( _T("value") ).GetLength() == 0 )
				{
					bHasValue= false;
				}
			}
			bHasVal = HasValue( _T("val") );
			if ( bHasVal )
			{
				if ( GetBasicValue( _T("val") ).GetLength() == 0 )
				{
					bHasVal= false;
				}
			}
			// looking for basic value
			if ( m_sValue.GetLength()  ||
				!(bHasVal|bHasValue) )
			{
				m_sValue = sValue;
			}
			else if ( bHasVal )
			{
				m_msAttributes[_T("val")] = sValue;
			}
			else if ( bHasVal )
			{
				m_msAttributes[_T("value")] = sValue;
			}
		}
	}
}

void	CXMLElement::SetBool( BOOL bValue, LPCTSTR szAttrKey )
{
	CStdString sValue;

	if ( bValue )
	{
		sValue = _T("true");
	}
	else
	{
		sValue = _T("false");
	}
	SetString( sValue, szAttrKey );
}

void	CXMLElement::SetLong( long lValue, LPCTSTR szAttrKey )
{
	CStdString sValue;

	sValue.Format( _T("%ld"), lValue );
	SetString( sValue, szAttrKey );
}

void	CXMLElement::SetDWORD( DWORD dwValue, LPCTSTR szAttrKey )
{
	CStdString sValue;

	sValue.Format( _T("%lu"), dwValue );
	SetString( sValue, szAttrKey );
}


int	CXMLElement::GetIndex( bool bNamed )
{
	CXMLElement * pElement;
	int iReturn = 0;
	
	if ( m_pParent )
	{
		if ( bNamed )
		{
			pElement = m_pParent->GetElement( m_sName, 0 );
			while( pElement && pElement != this )
			{
				iReturn++;
				pElement = m_pParent->GetElement( m_sName, iReturn );
			}
		}
		else
		{
			pElement = m_pParent->GetElement( 0 );
			while( pElement && pElement != this )
			{
				iReturn++;
				pElement = m_pParent->GetElement( iReturn );
			}
		}
	}
	return iReturn;
}


bool CXMLElement::SaveElement( HANDLE hFile )
{
	bool bReturn = false;

	if ( this )
	{
		CStdString sTemp, sLine;
		CStdString sVal;
		TMapStrings::iterator iterAttr;

		sLine.Format( _T("<%s"), m_sName.c_str() );
		iterAttr = m_msAttributes.begin();
		while( iterAttr != m_msAttributes.end() )
		{
			if ( iterAttr->second.size() )
			{
				sVal = iterAttr->second;
				Scramble( sVal );

				sTemp.Format( _T(" %s=\"%s\""), iterAttr->first.c_str(), sVal.c_str() );

				sLine += sTemp;
			}
			iterAttr++;
		}
		sVal = m_sValue;
		Scramble( sVal );
		sTemp.Format( _T(">%s</%s>"), sVal.c_str(), m_sName.c_str() );
		sLine += sTemp;
		sLine.Replace( _T("\r\n"), _T("\n") );
		sLine.Replace( _T("\n"), _T("\r\n") );
		WriteFileLine( hFile, sLine );
		bReturn = true;
	}
	return bReturn;
}

bool CXMLNode::SaveNode( HANDLE hFile )
{
	bool bReturn = false;

	CStdString sTemp, sLine;
	TMapStrings::iterator iterAttr;
	TXMLElementList::iterator iterElement;
	TXMLNodeList::iterator iterNode;
	CStdString sVal;

	if ( this )
	{
		sLine.Format( _T("<%s"), m_sName.c_str() );
		iterAttr = m_msAttributes.begin();
		while( iterAttr != m_msAttributes.end() )
		{
			if ( iterAttr->second.size() )
			{
				sVal = iterAttr->second;
				Scramble(sVal);
				sTemp.Format( _T(" %s=\"%s\""), iterAttr->first.c_str(), sVal.c_str() );
				sLine += sTemp;
			}
			iterAttr++;
		}
		sLine += _T(">");
		WriteFileLine( hFile, sLine );

		iterElement = m_msElements.begin();
		while( iterElement != m_msElements.end() )
		{
			iterElement->SaveElement( hFile );
			iterElement++;
		}
		iterNode = m_nlNodes.begin();
		while( iterNode != m_nlNodes.end() )
		{
			iterNode->GetNodePtr()->SaveNode( hFile );
			iterNode++;
		}
		sLine.Format( _T("</%s>"), m_sName.c_str() );
		WriteFileLine( hFile, sLine );
		bReturn = true;
	}

	return bReturn;
}

bool CXMLNode::SaveNode( LPCTSTR szFilename )
{
	bool bReturn = false;
	HANDLE hFile;

	if ( this )
	{
		hFile = CreateFile( szFilename, 
						GENERIC_WRITE, 
						0, 
						NULL,
						CREATE_ALWAYS,
						FILE_ATTRIBUTE_NORMAL,
						NULL );
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			bReturn = SaveNode( hFile );
			CloseHandle( hFile );
		}
	}

	return bReturn;
}


CXMLElement *	CXMLNode::CopyInElement( const CXMLElement& element )
{
	CXMLElement * pReturn = NULL;

	if ( this )
	{
		CStdString sValue(element.m_sValue);
//		UnScramble( sValue );
		pReturn = AddElement( element.m_sName, sValue );
		if ( pReturn )
		{
			pReturn->m_msAttributes = element.m_msAttributes;
		}
	}
	return pReturn;
}

CXMLElement *	CXMLNode::AddElement( LPCTSTR szName, LPCTSTR szValue  )
{
	CStdString sName(szName);

	if ( this )
	{
		sName.MakeLower();
		if ( szName && _tcslen(szName) )
		{
			m_msElements.insert( m_msElements.end(), CXMLElement( this, sName, szValue ) );
		}
	}
	return GetElement( sName, -1 );
}

CXMLElement *	CXMLNode::GetElement( LPCTSTR szName, int iIndex )
{
	CXMLElement * pReturn = NULL;
	TXMLElementList::iterator iterElement;
	CStdString sName(szName);

	if ( this )
	{
		sName.MakeLower();
		if ( iIndex < 0 )
		{
			iIndex = -1;
		}
		iterElement = m_msElements.begin();
		while( iterElement != m_msElements.end() )
		{
			if ( iterElement->m_sName.Compare( sName ) == 0 )
			{
				if ( iIndex < 0 )
				{
					// Get LAST node
					pReturn = iterElement->GetElementPtr();
					// Keep looking until list is finished!
				}
				else
				{
					if ( iIndex == 0 )
					{
						pReturn = iterElement->GetElementPtr();
						break;
					}
					iIndex--;
				}
			}
			iterElement++;
		}
	}
	return pReturn;
}

CXMLElement *	CXMLNode::GetElement( int iIndex )
{
	CXMLElement * pReturn = NULL;
	TXMLElementList::iterator iterElement;

	if ( this )
	{
		if ( iIndex < 0 )
		{
			iIndex = -1;
		}
		iterElement = m_msElements.begin();
		while( iterElement != m_msElements.end() )
		{
			if ( iIndex < 0 )
			{
				// Get LAST node
				pReturn = iterElement->GetElementPtr();
				// Keep looking until list is finished!
			}
			else
			{
				if ( iIndex == 0 )
				{
					pReturn = iterElement->GetElementPtr();
					break;
				}
				iIndex--;
			}
			iterElement++;
		}
	}
	return pReturn;
}



CXMLNode *	CXMLNode::GetNode( int iIndex )
{
	CXMLNode * pReturn = NULL;
	CXMLNode * pNode;
	TXMLNodeList::iterator iterNode;

	if ( this )
	{
		if ( iIndex < 0 )
		{
			iIndex = -1;
		}
		iterNode = m_nlNodes.begin();
		while( iterNode != m_nlNodes.end() )
		{
			pNode = iterNode->GetNodePtr();
			if ( iIndex < 0 )
			{
				// Get LAST node
				pReturn = pNode;
				// Keep looking until list is finished!
			}
			else
			{
				if ( iIndex == 0 )
				{
					pReturn = pNode;
					break;
				}
				iIndex--;
			}
			iterNode++;
		}
	}
	return pReturn;
}


void CXMLNode::FixElementsParent( void )
{
	TXMLElementList::iterator iterElement;

	if ( this )
	{
		iterElement = m_msElements.begin();
		while( iterElement != m_msElements.end() )
		{
			iterElement->SetParent(this);
			iterElement++;
		}
	}
}

CXMLNode *	CXMLNode::GetNode( LPCTSTR szName, int iIndex )
{
	CXMLNode * pReturn = NULL;
	CXMLNode * pNode;
	TXMLNodeList::iterator iterNode;
	CStdString sName(szName);

	if ( this )
	{
		sName.MakeLower();

		if ( iIndex < 0 )
		{
			iIndex = -1;
		}
		iterNode = m_nlNodes.begin();
		while( iterNode != m_nlNodes.end() )
		{
			pNode = iterNode->GetNodePtr();
			if ( pNode->m_sName.Compare( sName ) == 0 )
			{
				if ( iIndex == -1 )
				{
					// Track this one, might be last
					pReturn = pNode;
					// But keep going, if it isn't
				}
				else
				{
					if ( iIndex == 0 )
					{
						pReturn = pNode;
						break;
					}
					else
					{
						iIndex--;
					}
				}
			}
			iterNode++;
		}
	}
	return pReturn;
}

CXMLNode *	CXMLNode::CopyInNode( const CXMLNode& node, LPCTSTR szName )
{
	CXMLNode * pNode = NULL;
	CXMLNode * pSubNode = NULL;
	CStdString sName = node.m_sName;
	
	if ( this )
	{
		if ( szName && _tcslen( szName ) )
		{
			sName = szName;
		}
		pNode = AddNode( sName );
		if ( pNode )
		{
			pNode->m_msAttributes = node.m_msAttributes;
			pNode->m_msElements = node.m_msElements;

			FixElementsParent();

			TXMLNodeList::iterator iterNode;

			// Copy in each subnode...
			iterNode = ((CXMLNode&)node).m_nlNodes.begin();
			while( iterNode != node.m_nlNodes.end() )
			{
				pSubNode = iterNode->GetNodePtr();
				pNode->CopyInNode( *pSubNode );
				iterNode++;
			}
		}
	}
	return pNode;
}


CXMLNode *	CXMLNode::AddNode( LPCTSTR szName )
{
	CXMLNode * pReturn = NULL;
	CStdString sName(szName);
	
	if ( this )
	{
		sName.MakeLower();
		// D:\MenuX\menu.xdi
		if ( szName && _tcslen(szName) )
		{
			m_nlNodes.insert( m_nlNodes.end(), CXMLNodeProxy( this, sName ) );
		}
	}
	return GetNode( szName, -1 );
}


int	CXMLNode::SetStrings( LPCTSTR szKey, TListStrings & slStrings )
{
//	CXMLElement * pElement;
	TListStrings::iterator iterCurrent = slStrings.begin();
	int iReturn = 0;

	while( iterCurrent != slStrings.end() )
	{
		if ( AddElement( szKey, iterCurrent->c_str() ) )
		{
			iReturn++;
		}
		iterCurrent++;
	}
	return iReturn;
}

int	CXMLNode::GetStrings( LPCTSTR szKey, TListStrings & slStrings )
{
	slStrings.clear();
	CXMLElement * pElement;
	CXMLNode * pNode;
	CStdString sValue;

	int iIndex = 0;


	while( pNode = GetNode( szKey, iIndex++ ) )
	{
		sValue = pNode->GetBasicValue(NULL);
		if ( sValue.GetLength() )
		{
			slStrings.push_back( sValue );
		}
	}

	iIndex = 0;

	while( pElement = GetElement( szKey, iIndex++ ) )
	{
		sValue = pElement->m_sValue;
		if ( sValue.GetLength() )
		{
			slStrings.push_back( sValue );
		}
	}
	return slStrings.size();
}