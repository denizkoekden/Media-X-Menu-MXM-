
#include <xtl.h>
#include "xmltree.h"
#include "bintree.h"


LPCTSTR GetToken( int iID, LPCTSTR * szTokenList )
{
	LPCTSTR  pszReturn = NULL;
	int iIndex = 0;

	if ( szTokenList )
	{
		while( szTokenList[iIndex] )
		{
			if ( iIndex == iID )
			{
				pszReturn = szTokenList[iIndex];
				break;
			}
			iIndex++;
		}
	}
	return pszReturn;
}


bool BuildBinTreeAttributes( LPCTSTR * szTokenList, int ** ppiBinTree, TMapStrings & lsAttributes, bool bLowerNames )
{
	bool bReturn = false;
	int iNameID, iValueID;
	CStdString sName, sValue;

	if ( ppiBinTree && *ppiBinTree && szTokenList )
	{
		while( (**ppiBinTree) != -1 )
		{
			iNameID = (**ppiBinTree);
			(*ppiBinTree)++;
			iValueID = (**ppiBinTree);
			(*ppiBinTree)++;
			sName = GetToken( iNameID, szTokenList );
			sValue = GetToken( iValueID, szTokenList );
			UnScramble(sValue);
			if ( sName.GetLength() )
			{
				lsAttributes[sName] = sValue;
			}
		}
		(*ppiBinTree)++;
	}
	return bReturn;
}


CXMLElement * BuildBinTreeElement( LPCTSTR * szTokenList, int ** ppiBinTree, CXMLNode * pParent, bool bLowerNames )
{
	CXMLElement * pReturn = NULL;
	int iNameID, iValueID;
	CStdString sName, sValue;

	if ( ppiBinTree && *ppiBinTree && szTokenList )
	{
		iNameID = **ppiBinTree;
		(*ppiBinTree)++;
		if ( iNameID != -1 )
		{
			iValueID = **ppiBinTree;
			(*ppiBinTree)++;
			sName = GetToken( iNameID, szTokenList );
			sValue = GetToken( iValueID, szTokenList );
			UnScramble(sValue);
			if ( sName.GetLength() )
			{
				pReturn = pParent->AddElement( sName, sValue );
				if ( pReturn )
				{
					BuildBinTreeAttributes( szTokenList, ppiBinTree, pReturn->m_msAttributes, bLowerNames );
				}
			}
		}
	}

	return pReturn;
}



CXMLNode * BuildBinTreeNode( LPCTSTR * szTokenList, int ** ppiBinTree, CXMLNode * pParent, bool bLowerNames )
{
	CXMLNode * pReturn = NULL;
	int iNameID;
	CStdString sName;

	if ( ppiBinTree && *ppiBinTree && szTokenList )
	{
		iNameID = **ppiBinTree;
		(*ppiBinTree)++;

		if ( iNameID != -1 )
		{
			sName = GetToken( iNameID, szTokenList );
			if ( sName.GetLength() )
			{
				// We have a node! Create one to return
				if ( pParent )
				{
					pReturn = pParent->AddNode(sName);
				}
				else
				{
					pReturn = new CXMLNode(NULL, sName );
				}
				if ( pReturn )
				{
					// Parse out attributes
					BuildBinTreeAttributes( szTokenList, ppiBinTree, pReturn->m_msAttributes, bLowerNames );
					while( BuildBinTreeElement( szTokenList, ppiBinTree, pReturn, bLowerNames ) )
					{
					}
					while( BuildBinTreeNode( szTokenList, ppiBinTree, pReturn, bLowerNames ) )
					{
					}
				}
			}
		}
	}
	return pReturn;
}
