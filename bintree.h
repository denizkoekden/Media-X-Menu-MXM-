
// bintree.h


#ifndef __BINTREE_H__
#define __BINTREE_H__


// Usage:
//	CXMLNode * pMainNode;
//
//	int * piBinTree = &g_iBtTreeSample[0];
//
//	pMainNode = BuildBinTreeNode( g_szBtTknsSample, &piBinTree, NULL, true );
//

CXMLNode * BuildBinTreeNode( LPCTSTR * szTokenList, int ** ppiBinTree, CXMLNode * pParent, bool bLowerNames );
CXMLElement * BuildBinTreeElement( LPCTSTR * szTokenList, int ** ppiBinTree, CXMLNode * pParent, bool bLowerNames );
bool BuildBinTreeAttributes( LPCTSTR * szTokenList, int ** ppiBinTree, TMapStrings & lsAttributes, bool bLowerNames );
LPCTSTR GetToken( int iID, LPCTSTR * szTokenList );


#endif