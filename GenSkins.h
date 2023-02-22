/*********************************************************************************\
 * GenSkins.h
 * Generic Skin classes
 * (C) 2002 MasterMind
\*********************************************************************************/


#ifndef __GENSKINS_H__
#define __GENSKINS_H__

#include <xtl.h>
#include <map>
#include <list>
#include "XBFont_mxm.h"
#include "ImageSource.h"
#include "StdString.h"
#include "XmlTree.h"



#define LET_IMAGE		0
#define LET_TEXT		1
#define	LET_SPECIAL		2

#define LES_CACHE		0
#define LES_DYNAMIC		1

#define LEJ_LEFTX		0x0000
#define LEJ_TOPY		0x0000
#define LEJ_CENTERX		0x0001
#define LEJ_RIGHTX		0x0002
#define LEJ_CENTERY		0x0004
#define LEJ_BOTTOMY		0x0008
#define LEJ_PROPORTION	0x2000
#define LEJ_STRETCH		0x4000
#define LEJ_SHRINK		0x8000

class CGenericSkin;
class CLayoutElement;

extern CStdString GetElementText( LPCTSTR szSubSource, LPCTSTR szSourceLoc );
extern LPDIRECT3DTEXTURE8 GetElementTexture( LPCTSTR szSubSource, LPCTSTR szSourceLoc );
extern HRESULT	RenderSpecial( CLayoutElement *pElement );



class CLayoutElement
{
public:
	CLayoutElement();
	~CLayoutElement();
	CLayoutElement * GetPtr( void );
	HRESULT Render( LPDIRECT3DDEVICE8 pDevice );
	HRESULT RenderImage( LPDIRECT3DDEVICE8 pDevice );
	HRESULT RenderText( LPDIRECT3DDEVICE8 pDevice );
	HRESULT ProcessNode( CXMLNode * pNode );

	TMapStrings m_msAttributes;
	int			m_iSource;
	CStdString	m_sSubSource;
	CStdStringW m_sCacheString;
	LPDIRECT3DTEXTURE8 m_pCacheTexture;

	CGenericSkin *	m_pParentSkin;
	CStdString		m_sFont;

	int m_iType;

	int m_iPosX;			// Defaults to 0
	int m_iPosY;			// Defaults to 0
	int	m_iWidth;			// Defaults to -1
	int m_iHeight;			// Defaults to -1

	int	m_iJustifyX;		// 0=Left,	1=Center,	2=Right
	int	m_iJustifyY;		// 0=Top,	1=Center,	2=Bottom

	WORD	m_wJustify;		// XXYY 

	DWORD	m_dwaColor[4];	//	4 corners for boxes
							//	for Text: 0 - text color, 1 - shadow 2 - Glow
	DWORD m_dwColor;
	DWORD m_dwAltColor;
	CStdString	m_sSourceLoc;

	PVOID	m_pSpecialDataPtr;

};


typedef list<CLayoutElement> TLayoutElements;

class CLayout
{
	TLayoutElements	m_elements;
	TLayoutElements::iterator m_iterCurElement;
	CStdString		m_sName;
public:
	CStdString GetName( void ) { return m_sName; };
	void	SetName( LPCTSTR szName ) { m_sName = szName; };
	HRESULT Render( LPDIRECT3DDEVICE8 pDevice );
	HRESULT ProcessNode( CXMLNode * pNode );
	HRESULT ProcessLayoutElemNode( CXMLNode * pNode );
	void Activate( void );
	void Deactivate( void );
	void Initialize( void );
	CLayout();
	~CLayout();
	CLayoutElement * GetFirstElement( void );
	CLayoutElement * GetNextElement( void );
	CLayoutElement * GetLastElement( void );
	CLayoutElement * AddLayoutElement( void );
};


typedef map<CStdString, CXBFont> TFontItems;
typedef map<CStdString, CImageSource> TImageItems;
typedef map<CStdString, CLayout> TLayouts;



class CGenericSkin
{
	TImageItems		m_images;
	TFontItems		m_fonts;
	LPDIRECT3DDEVICE8 m_pd3dDevice;
	LPDIRECTSOUND8	m_pDSound;
	CLayout *		m_pCacheLayout;
	CStdString		m_sCacheLayoutName;
	CStdString		m_sBasePath;

public:

	HRESULT			ProcessMainNode( CXMLNode * pNode );
	HRESULT			ProcessInfoNode( CXMLNode * pNode );
	HRESULT			ProcessResourceNode( CXMLNode * pNode );
	HRESULT			ProcessLayoutNode( CXMLNode * pNode );
	CGenericSkin();
	~CGenericSkin();
	void			SetDevices( LPDIRECT3DDEVICE8 pd3dDevice = NULL, LPDIRECTSOUND8	pDSound = NULL );
	CStdString		m_sSkinName;
	CStdString		m_sVersion;
	CStdString		m_sAuthor;
	CStdString		m_sCopyright;
	TLayouts		m_layouts;
	TMapStrings		m_msInformation;

	HRESULT			Render( LPCTSTR szLayout );
	HRESULT			Render( CLayout * pLayout );

	CLayout *		AddLayout( LPCTSTR szLayout );
	CLayout *		GetLayout( LPCTSTR szLayout );

	HRESULT			LoadSkin( LPCTSTR szXMLFile );

	CXBFont *		GetFontResource( LPCTSTR szName );
	CXBFont *		AddFontResource( LPCTSTR szName, LPCTSTR szFilePath = NULL );
	CXBFont *		AddFontResource( LPCTSTR szName, const LPBYTE pDataBuffer, LPCTSTR szSection = NULL );

	CImageSource *	GetImgResource( LPCTSTR szName );
	CImageSource *	AddImgResource( LPCTSTR szName, LPCTSTR szFilePath = NULL );
	CImageSource *	AddImgResource( LPCTSTR szName, const LPBYTE pDataBuffer, DWORD dwSize, LPCTSTR szSection = NULL );

};


#endif //  __GENSKINS_H__