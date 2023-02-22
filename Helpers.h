



#ifndef __HELPERS_H__
#define __HELPERS_H__


#include "MXM.h"
#include "MXM_Defs.h"
#include "xmltree.h"
#include "ItemInfo.h"
#include "XBXmlCfgAccess.h"
#include "XBFont_mxm.h"


CStdString FindMedia( LPCTSTR szDirectory, bool bSS = true );
CXMLNode * GetXDI( void );
void SetPositionInNode( CXMLNode * pNode, int iLeft, int iTop, int iW, int iH, int iX = -1, int iY = -1  );
void SetScalingInNode( CXMLNode * pNode, bool bScaleX, bool bScaleY );
void BuildDefaultSkin( void );
void BuildXDISkin( void );
void BuildOldSkins( CXMLNode * pMXMXML, CXMLNode * pOldSkinXML, LPCTSTR szName, LPCTSTR szBasePath );
bool FindMXM( LPCTSTR szPath );
bool IsDrivePresent( char cDrive );
void SearchForAutos( CItemInfo & pItem );
bool CheckDirectory( LPCTSTR szDirectory, CItemInfo & pItem, bool bKeepD = false, bool bDefaultOnly = false );
bool GetEntry( LPCTSTR szDir, LPCTSTR szExeDir, CXBXmlCfgAccess & iniItemFile, LPCTSTR szSection, CItemInfo & pItem );
LPCTSTR GetGameTitleFromID( DWORD dwID );
LPCTSTR GetGameDescriptionFromID( DWORD dwID, bool bSearchPublisher = true );
void FindSkins ( list<CStdString> & saLocations, LPCTSTR szDirectory );
CStdString GetXbeFile( LPCTSTR szDir, int iIndex );
D3DFORMAT GetTextureFormat( LPDIRECT3DTEXTURE8 pTexture );
FLOAT GetTexturePos( DWORD dwTextureMax, DWORD dwTexturePixPos );
DWORD MakeColor( int iAlpha, int iRed, int iGreen, int iBlue );
DWORD GetPeriodValue(DWORD dwTimeStamp, DWORD dwPeriod );
CStdString FindFile( LPCTSTR szDirectory, LPCTSTR szBaseName, bool bRecurse = true );
bool HasString( TListStrings & slStrings, LPCTSTR szValue, bool bIgnoreCase = true );
void CleanInvalidChars(CXBFont * pFnt, CStdString & sString );




#endif //  __HELPERS_H__