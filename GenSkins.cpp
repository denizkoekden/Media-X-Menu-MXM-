/*********************************************************************************\
 * GenSkins.cpp
 * Generic Skin classes
 * (C) 2002 MasterMind
\*********************************************************************************/


#include <xtl.h>
#include <xgraphics.h>
#include <XbSound.h>
#include "MXM.h"
#include "XBXmlCfgAccess.h"
#include "utils.h"
#include "ImageSource.h"
#include <list>
#include "XBEasel.h"
#include "XmlStream.h"
#include "GenSkins.h"
#include "CommDebug.h"

#define PN_INFO			0
#define PN_RESOURCE		1
#define PN_LAYOUT		2

#define LE_UNKNOWN		-1
#define LE_TEXT			0
#define LE_IMAGE		1
#define LE_SPECIAL		2




class CSkinLoader : public XmlNotify
{
	CGenericSkin *	m_pSkin;
	int				m_iPrimaryNodeType;
	int				m_iLayoutType;
	bool			m_bInLayoutElement;
	TMapStrings		m_msCurrentResourceValues;
	CLayout	*		m_pCurrentLayout;
	CXMLNode	*   m_pLayoutElementNode;
	CXMLNode	*   m_pCurrentElementNode;
public:
	int m_iLevel;

	CSkinLoader(CGenericSkin * pSkin = NULL)
	{
		m_pLayoutElementNode = NULL;
		m_pCurrentLayout = NULL;
		m_pSkin = pSkin;
		m_iLevel = 0;
		m_iPrimaryNodeType = -1;
		m_bInLayoutElement = false;
	};

	virtual void foundNode		( string & name, string & attributes )
	{
		TMapStrings msAttributes;
		CStdString sName(name);
		CStdString sValue;
		CStdString sTemp;

		sName.MakeLower();

		DEBUG_FORMAT( _T("%*sfoundNode: %s (%s)"), m_iLevel, "", name.c_str(), attributes.c_str() );

		ParseXMLAttributes( attributes, msAttributes );

		sValue = msAttributes[_T("val")];
		if ( sValue.GetLength() == 0 )
		{
			sValue = msAttributes[_T("value")];
		}

		switch ( m_iLevel )
		{
			case 4:
				if ( m_bInLayoutElement )
				{
				}
				break;
			case 3:
				if ( m_bInLayoutElement )
				{
				}
				break;
			case 2:
				switch( m_iPrimaryNodeType )
				{
					case PN_RESOURCE:
						// Type
						// Name
						// File
						// m_msCurrentResourceValues = msAttributes;
						m_bInLayoutElement = false;
						if ( sValue.GetLength() )
						{
							m_msCurrentResourceValues[sName] = sValue;
						}
						break;
					case PN_LAYOUT:
						m_bInLayoutElement = false;
						if ( m_pLayoutElementNode )
						{
							delete m_pLayoutElementNode;
						}
						if ( m_pCurrentLayout )
						{
							if ( !sName.Compare( _T("layoutelement") ) )
							{
								// Set up layout element
								// MUST have type and source
								CStdString sType, sSource, sSubSource;
								sType = msAttributes[_T("type")];
								sType.MakeLower();
								sSource = msAttributes[_T("source")];
								sSource.MakeLower();
								sSubSource = msAttributes[_T("subsource")];
								sSubSource.MakeLower();
								if ( sType.GetLength() && sSource.GetLength() )
								{
									if ( !sType.Compare( _T("text") ) )
									{
										m_pLayoutElementNode = new CXMLNode;
										m_pLayoutElementNode->m_msAttributes = msAttributes;

										m_bInLayoutElement = true;
										m_iLayoutType = LE_TEXT;
										DEBUG_FORMAT(_T( "Layout Type: %s"), sType.c_str() );
									}
									else if ( !sType.Compare( _T("image") ) )
									{
										m_pLayoutElementNode = new CXMLNode;
										m_pLayoutElementNode->m_msAttributes = msAttributes;
										m_bInLayoutElement = true;
										m_iLayoutType = LE_IMAGE;
										DEBUG_FORMAT(_T( "Layout Type: %s"), sType.c_str() );
									}
									else if ( !sType.Compare( _T("special") ) )
									{
										m_pLayoutElementNode = new CXMLNode;
										m_pLayoutElementNode->m_msAttributes = msAttributes;
										m_bInLayoutElement = true;
										m_iLayoutType = LE_SPECIAL;
										DEBUG_FORMAT(_T( "Layout Type: %s"), sType.c_str() );
									}
									else
									{
										m_bInLayoutElement = false;
										m_iLayoutType = LE_UNKNOWN;
										DEBUG_FORMAT(_T( "Unknown Layout Type: %s"), sType.c_str() );
									}
								}
							}
						}
						break;
					case PN_INFO:
						m_bInLayoutElement = false;
						if ( !sName.Compare( _T("version") ) && sValue.GetLength() )
						{
							m_pSkin->m_sVersion = sValue;
						}
						else if ( !sName.Compare( _T("author") ) && sValue.GetLength() )
						{
							m_pSkin->m_sAuthor = sValue;
						}
						else if ( !sName.Compare( _T("copyright") ) && sValue.GetLength() )
						{
							m_pSkin->m_sCopyright = sValue;
						}
						else
						{
							m_pSkin->m_msInformation[sName] = sValue;
						}
						break;
				}
				break;
			case 1:
				m_bInLayoutElement = false;
				if ( !sName.Compare( _T("information") ) )
				{
					TMapStrings::iterator iterAttr;
					CStdString sAttrName, sAttrVal;

					m_iPrimaryNodeType = PN_INFO;
					
					iterAttr = msAttributes.begin();
					while( iterAttr != msAttributes.end() )
					{
						sAttrName = iterAttr->first;
						sAttrVal = iterAttr->second;
						if ( sAttrVal.GetLength() )
						{
							if ( !sAttrName.Compare(_T("author")) )
							{
								m_pSkin->m_sAuthor = sAttrVal;
							}
							else if ( !sAttrName.Compare(_T("copyright")) )
							{
								m_pSkin->m_sCopyright = sAttrVal;
							}
							else if ( !sAttrName.Compare(_T("version")) )
							{
								m_pSkin->m_sVersion = sAttrVal;
							}
							else
							{
								m_pSkin->m_msInformation[sAttrName] = sAttrVal;
							}
						}
						iterAttr++;
					}
				}
				else if ( !sName.Compare( _T("resource") ) )
				{
					m_iPrimaryNodeType = PN_RESOURCE;
					// Type
					// Name
					// File
					m_msCurrentResourceValues = msAttributes;
				}
				else if ( !sName.Compare( _T("layout") ) )
				{
					sValue = msAttributes[_T("id")];
					if ( sValue.GetLength() )
					{
						m_pCurrentLayout = m_pSkin->AddLayout( sValue );

						if ( m_pCurrentLayout )
						{
							m_iPrimaryNodeType = PN_LAYOUT;
						}
					}
				}
				else
				{
					// Unknown!! 
				}
				break;
			case 0:
				m_bInLayoutElement = false;
				// Must be "Skin" or it's not correct!
				if ( sName.Compare( _T("skin") ) == 0 )
				{
					m_pSkin->m_sSkinName = msAttributes[_T("name")];
				}
				else
				{
					// Bad format!
				}
				break;
		}

#if 0
		if ( m_iLevel == 0 )
		{
			sTemp = name;
			sTemp.MakeLower();
			if ( sTemp.Compare( _T("config") ) == 0 )
			{
				m_bInConfig = true;
			}
		}
		else if ( m_iLevel == 1 && m_bInConfig )
		{
			m_sCurrentSection = name;

//			sTemp.Format(_T("[%s]"), name.c_str() );
//			DEBUG_LINE( sTemp.c_str() );
		}
		else
		{
			// Invalid!! Can't have more nodes in ini file construct
		}
#endif
		m_iLevel++;
	};
	virtual void terminateNode  ( string & name, string & attributes )
	{
		CStdString sTemp, sName, sFile;

		DEBUG_FORMAT( _T("%*sterminateNode: %s (%s)"), m_iLevel, "", name.c_str(), attributes.c_str() );

		switch ( m_iLevel )
		{
			case 2:
				switch( m_iPrimaryNodeType )
				{
					case PN_RESOURCE:
						// Type
						// Name
						// File
						// m_msCurrentResourceValues = msAttributes;
						// Make new font or image resource from data....
						sTemp = m_msCurrentResourceValues[_T("type")];
						sTemp.MakeLower();
						sName = m_msCurrentResourceValues[_T("name")];
						sFile = m_msCurrentResourceValues[_T("file")];
						if ( !sTemp.Compare(_T("image")) )
						{
							m_pSkin->AddImgResource( sName, sFile );
						}
						else if ( !sTemp.Compare(_T("font")) )
						{
							m_pSkin->AddFontResource( sName, sFile );
						}
						break;
					case PN_LAYOUT:
						break;
					case PN_INFO:
						break;
				}
				break;
			case 1:
				break;
			case 0:
				// Must be "Skin" or it's not correct!
				break;
		}
		
		if ( m_iLevel )
		{
			m_iLevel--;
		}
#if 0
		if ( m_iLevel == 0 )
		{
			m_bInConfig = false;
		}
		m_sCurrentSection = _T("");
#endif
	};

	virtual void foundElement	( string & name, string & value, string & attributes )
	{
		TMapStrings msAttributes;

		DEBUG_FORMAT( _T("%*s-foundElement: %s (%s)"), m_iLevel, "", name.c_str(), attributes.c_str() );

		ParseXMLAttributes( attributes, msAttributes );
#if 0
		CStdString sKey(name), sValue(value);

		if ( m_iLevel && m_sCurrentSection.GetLength() )
		{
			if( sKey.GetLength() && sValue.GetLength() )
			{
				m_pCfgAccess->SetString( m_sCurrentSection, sKey, sValue );
			}
			else if ( sKey.GetLength() && (sValue.GetLength()==0) )
			{
				m_pCfgAccess->SetString( m_sCurrentSection, sKey, _T("") );
			}
		}
#endif
//		CStdString sTemp;

//		sTemp.Format(_T("%s=%s"), name.c_str(), value.c_str() );
//		DEBUG_LINE( sTemp.c_str() );
	};

	virtual void startElement	( string & name, string & value, string & attributes )
	{
	};

	virtual void endElement		( string & name, string & value, string & attributes )
	{
	};

};



CLayoutElement::CLayoutElement()
{
	m_pParentSkin = NULL;

	m_iType = -1;

	m_pSpecialDataPtr = NULL;

	m_pCacheTexture = NULL;
	m_sCacheString = L"";

	m_iPosX = -1;
	m_iPosY = -1;
	m_iWidth = -1;
	m_iHeight = -1;
	m_iSource = -1;
	m_dwColor = 0x0;
	m_dwAltColor = 0x0;
}

CLayoutElement::~CLayoutElement()
{
	if ( m_pCacheTexture )
	{
		m_pCacheTexture->Release();
	}
	if ( m_pSpecialDataPtr )
	{
		delete m_pSpecialDataPtr;
	}
}

CLayoutElement * CLayoutElement::GetPtr( void )
{
	return this;
}


HRESULT CLayoutElement::RenderImage( LPDIRECT3DDEVICE8 pDevice )
{
	CImageSource *pImage;
	LPDIRECT3DTEXTURE8 pTexture;

	HRESULT hr = E_FAIL;

	if ( (m_iWidth>0) && (m_iHeight>0) )
	{

		switch ( m_iSource )
		{
			case LES_CACHE:
				if ( m_pParentSkin )
				{
					pImage = m_pParentSkin->GetImgResource(m_sSourceLoc);
					if ( pImage && pImage->GetTexture() )
					{
						hr = DrawSprite(pDevice, pImage->GetTexture(), m_iPosX, m_iPosY, m_iWidth, m_iHeight, FALSE );
					}
				}
				break;
			case LES_DYNAMIC:
				pTexture = GetElementTexture( m_sSubSource, m_sSourceLoc );
				if ( pTexture )
				{
					hr = DrawSprite(pDevice, pTexture, m_iPosX, m_iPosY, m_iWidth, m_iHeight, FALSE );
				}
				break;
		}
	}
	return hr;
}


// Function to create new RECTs based on available settings and justification
// rcSrc must be set to (0,0)-(width,height)
// rcDest.left and top must be X and Y (will be transformed afterward)
bool RectSetup( RECT &rcSrc, RECT &rcDest, int iDestWidth, int iDestHeight, WORD wJustify )
{
	bool bReturn = true;
	int iSrcWidth, iSrcHeight;


	if ( (rcSrc.right && rcSrc.bottom) && !( rcSrc.top || rcSrc.left ) )
	{
		iSrcWidth = rcSrc.right;
		iSrcHeight = rcSrc.bottom;

		// First, let's look at the width and height constraints
		if ( iDestWidth < 1)
		{
			iDestWidth = rcSrc.right;
		}
		if ( iDestHeight < 1 )
		{
			iDestHeight = rcSrc.bottom;
		}

		if ( wJustify&(LEJ_STRETCH|LEJ_SHRINK) )
		{
			if ( wJustify&LEJ_PROPORTION )
			{
				int iTryWidth, iTryHeight;


				iTryHeight = iSrcHeight*iDestWidth;
				iTryHeight /= iSrcWidth;

				iTryWidth = iSrcWidth*iDestHeight;
				iTryWidth /= iSrcHeight;

				// Funky resizing stuff.
				// Whooopeee!!!! Time for more brain pain.
				switch ( wJustify&(LEJ_STRETCH|LEJ_SHRINK) )
				{
					case (LEJ_STRETCH|LEJ_SHRINK):
						// We can fill the space anyway it can fit
						if ( iTryWidth <= iDestWidth )
						{
							iDestWidth = iTryWidth;
						}
						else
						{
							// Use iTryHeight
							iDestHeight = iTryHeight;
						}
						break;
					case LEJ_STRETCH:
						// We can fill the space as long as we increase the image size
						if ( iDestWidth > iSrcWidth )
						{
							if ( iDestHeight > iSrcHeight )
							{
								// Stretch on best option
								if ( iTryWidth <= iDestWidth )
								{
									iDestWidth = iTryWidth;
								}
								else
								{
									// Use iTryHeight
									iDestHeight = iTryHeight;
								}
							}
							else
							{
								// Only stretch on width
								// Use iTryHeight
								iDestHeight = iTryHeight;
							}
						}
						else
						{
							if ( iDestHeight > iSrcHeight )
							{
								iDestWidth = iTryWidth;
							}
							else
							{
								// Just clip source
								iSrcWidth = iDestWidth;
								iSrcHeight = iDestHeight;
							}
						}
						break;
					case LEJ_SHRINK:
						// We can fill the space as long as we decrease the image size
						if ( iDestWidth < iSrcWidth )
						{
							if ( iDestHeight < iSrcHeight )
							{
								// Shrink on best option
								if ( iTryWidth <= iDestWidth )
								{
									iDestWidth = iTryWidth;
								}
								else
								{
									// Use iTryHeight
									iDestHeight = iTryHeight;
								}
							}
							else
							{
								// Only shrink on width
								// Use iTryHeight
								iDestHeight = iTryHeight;
							}
						}
						else
						{
							if ( iDestHeight < iSrcHeight )
							{
								iDestWidth = iTryWidth;
							}
							else
							{
								// Just clip source
								iSrcWidth = iDestWidth;
								iSrcHeight = iDestHeight;
							}
						}
						break;
				}

			}
			else
			{
				// No need for funky resizing... let's just figure out what 
				// gets clipped or not.
				if ( iSrcWidth>iDestWidth )
				{
					// If not shrink, clip it!
					if ( !(wJustify&LEJ_SHRINK) )
					{
						iSrcWidth = iDestWidth;
					}
				}
				else
				{
					// If not stretch, clip it!
					if ( !(wJustify&LEJ_STRETCH) )
					{
						iDestWidth = iSrcWidth;
					}
				}
				if ( iSrcHeight>iDestHeight )
				{
					// If not shrink, clip it!
					if ( !(wJustify&LEJ_SHRINK) )
					{
						iSrcHeight = iDestHeight;
					}
				}
				else
				{
					// If not stretch, clip it!
					if ( !(wJustify&LEJ_STRETCH) )
					{
						iDestHeight = iSrcHeight;
					}
				}

			}
		}
		else
		{
			// Proportional is irrelevant here
			// Simple clip.
			if ( iSrcWidth>iDestWidth )
			{
				iSrcWidth = iDestWidth;
			}
			else
			{
				iDestWidth = iSrcWidth;
			}
			if ( iSrcHeight>iDestHeight )
			{
				iSrcHeight = iDestHeight;
			}
			else
			{
				iDestHeight = iSrcHeight;
			}
		}

		// Position...
		// rcDest.top is PosY, rcDest.left is PosX
		if ( wJustify&LEJ_CENTERX )
		{
			rcDest.left -= (iDestWidth>>1);
			rcSrc.left = ((rcSrc.right-iSrcWidth)>>1);
		}
		else if ( wJustify&LEJ_RIGHTX )
		{
			rcDest.left -= (iDestWidth);
			rcSrc.left = (rcSrc.right-iSrcWidth);
		}
 		// else No adjustment required...
		rcDest.right = rcDest.left+(iDestWidth-1);
		rcSrc.right = rcSrc.left + (iSrcWidth-1);
		if ( wJustify&LEJ_CENTERY )
		{
			rcDest.top -= (iDestHeight>>1);
			rcSrc.top = ((rcSrc.bottom-iSrcHeight)>>1);
		}
		else if ( wJustify&LEJ_BOTTOMY )
		{
			rcDest.top -= (iDestHeight);
			rcSrc.top = (rcSrc.bottom-iSrcHeight);
		}
		// else No adjustment required...
		rcDest.bottom = rcDest.top+(iDestHeight-1);
		rcSrc.bottom = rcSrc.top+(iSrcHeight-1);
	}
	else
	{
		bReturn = false;
	}
	return bReturn;

}



HRESULT CLayoutElement::RenderText( LPDIRECT3DDEVICE8 pDevice )
{
	HRESULT hr = E_FAIL;
	CStdStringW sText;
	CXBFont * pFont;
	RECT rcPos;

	if ( m_pParentSkin )
	{
		pFont = m_pParentSkin->GetFontResource( m_sFont );
		if ( pFont )
		{
			switch ( m_iSource )
			{
				case LES_CACHE:
					sText = m_sSourceLoc;
					break;
				case LES_DYNAMIC:
					sText = GetElementText( m_sSubSource, m_sSourceLoc );
					break;
			}
			if ( sText.GetLength() )
			{
				if ( ( m_pCacheTexture == NULL ) || m_sCacheString.Compare(sText) )
				{
					if ( m_pCacheTexture )
					{
						m_pCacheTexture->Release();
						m_pCacheTexture = NULL;
					}
					m_pCacheTexture = pFont->CreateTexture( sText, 0, m_dwColor, D3DFMT_LIN_A8R8G8B8 );
				}
				if ( m_pCacheTexture )
				{
					D3DSURFACE_DESC descSurface;
					// m_iWidth = descSurface.Width;
					// m_iHeight = descSurface.Height;

					// Now make pTexture our texture.
					// First we need to get the params
					if ( SUCCEEDED( m_pCacheTexture->GetLevelDesc( 0, &descSurface ) ) )
					{
						RECT rcSource;
						RECT rcDest;

						// We can stretch or shrink the text, or simply clip it.
						if (m_wJustify&(LEJ_SHRINK|LEJ_STRETCH))
						{
							// We are going to resize, under certain circumstances

							
						}
						else
						{
							// Clip, no resizing at all
							if ( m_iWidth == -1 || ( m_iWidth>=(int)descSurface.Width) )
							{
								rcSource.left = 0;
								rcSource.right = descSurface.Width-1;
								if (m_wJustify&LEJ_CENTERX)
								{
									// m_iPosX is center
									rcDest.left = m_iPosX-(descSurface.Width>>1);
									rcDest.right = rcDest.left+(descSurface.Width-1);
								}
								else if (m_wJustify&LEJ_RIGHTX)
								{
									// m_iPosX is right
									rcDest.left = (m_iPosX-descSurface.Width)+1;
									rcDest.right = m_iPosX;
								}
								else
								{
									rcDest.left = m_iPosX;
									rcDest.right = rcDest.left+(descSurface.Width-1);
								}
								
							}
							else
							{
								// Must clip width
								if (m_wJustify&LEJ_CENTERX)
								{
									// m_iPosX is center
									rcDest.left = m_iPosX-(m_iWidth>>1);
									rcDest.right = rcDest.left+(m_iWidth-1);
									rcSource.left = ((descSurface.Width-m_iWidth)/2);
									rcSource.right = rcSource.left+(m_iWidth-1);
								}
								else if (m_wJustify&LEJ_RIGHTX)
								{
									// m_iPosX is right
									rcDest.left = (m_iPosX-m_iWidth)+1;
									rcDest.right = m_iPosX;
									rcSource.right = descSurface.Width-1;
									rcSource.left = rcSource.right-m_iWidth;
								}
								else
								{
									rcSource.left = 0;
									rcSource.right = m_iWidth-1;
									rcDest.left = m_iPosX;
									rcDest.right = rcDest.left+(m_iWidth-1);
								}

							}
							if ( m_iHeight == -1 || ( m_iHeight>=(int)descSurface.Height) )
							{
								rcSource.top = 0;
								rcSource.bottom = descSurface.Height-1;
								if (m_wJustify&LEJ_CENTERY)
								{
									// m_iPosY is center
									rcDest.top = m_iPosY-(descSurface.Height>>1);
									rcDest.bottom = rcDest.top+(descSurface.Height-1);
								}
								else if (m_wJustify&LEJ_BOTTOMY)
								{
									// m_iPosY is bottom
									rcDest.top = (m_iPosY-descSurface.Height)+1;
									rcDest.bottom = m_iPosY;
								}
								else
								{
									rcDest.top = m_iPosY;
									rcDest.bottom = rcDest.top+(descSurface.Height-1);
								}
								
							}
							else
							{
								// Must clip width
								if (m_wJustify&LEJ_CENTERY)
								{
									// m_iPosY is center
									rcDest.top = m_iPosX-(m_iHeight>>1);
									rcDest.bottom = rcDest.top+(m_iHeight-1);
									rcSource.top = ((descSurface.Height-m_iHeight)/2);
									rcSource.bottom = rcSource.top+(m_iHeight-1);
								}
								else if (m_wJustify&LEJ_BOTTOMY)
								{
									// m_iPosY is bottom
									rcDest.top = (m_iPosY-m_iHeight)+1;
									rcDest.bottom = m_iPosY;
									rcSource.bottom = descSurface.Height-1;
									rcSource.top = rcSource.bottom-m_iHeight;
								}
								else
								{
									rcSource.top = 0;
									rcSource.bottom = m_iHeight-1;
									rcDest.top = m_iPosY;
									rcDest.bottom = rcDest.top+(m_iHeight-1);
								}
							}
							// Pheeew! Finally have source and destination rects!
							// Let's convert these into offset coordinates!

						}

						if ( m_iWidth < (int)descSurface.Width )
						{
							if (m_wJustify&LEJ_CENTERX)
							{
							}
							else if (m_wJustify&LEJ_RIGHTX)
							{
							}
							else
							{
								rcPos.left = m_iPosX;
								if ( m_wJustify&LEJ_SHRINK )
								{
									rcPos.right = m_iPosX+(m_iWidth-1);
								}
								else
								{
								}
							}
						}
						else
						{
							// No need to resize...  unless we are directed to stretch
							if ( m_wJustify&LEJ_STRETCH )
							{
							}
						}
					}
				}
			}
			else
			{
				m_sCacheString = L"";
				if ( m_pCacheTexture )
				{
					m_pCacheTexture->Release();
					m_pCacheTexture = NULL;
				}
			}
		}
	}
	return hr;
}

HRESULT CLayoutElement::Render( LPDIRECT3DDEVICE8 pDevice )
{
	HRESULT hr = E_FAIL;

	if ( (m_iPosX>=0) && (m_iPosY>=0) )
	{
		// Call appropriate rendering routine
		switch( m_iType )
		{
			case LET_IMAGE:
				hr = RenderImage(pDevice);
				break;
			case LET_TEXT:
				hr = RenderText(pDevice);
				break;
			case LET_SPECIAL:
				break;
		}
	}
	return hr;
}



CLayout::CLayout()
{
	m_sName = _T("");
}

CLayout::~CLayout()
{
}

HRESULT CLayoutElement::ProcessNode( CXMLNode * pNode )
{
	HRESULT hr = E_FAIL;
	CStdString sType;
	CStdString sSource;
	CStdString sSubSource;
//	CXMLElement * pSubElement;
//	CXMLNode * pSubNode;
	CStdString sValue;


	if ( pNode )
	{
		sType = pNode->GetBasicValue( _T("type") );
		sType.MakeLower();
		sSource = pNode->GetBasicValue( _T("source") );
		sSubSource = pNode->GetBasicValue( _T("subsource") );
		if ( sType.Compare( _T("image") ) == 0 )
		{
			m_iType = LET_IMAGE;
		}
		else if ( sType.Compare( _T("text") ) == 0 )
		{
			m_iType = LET_TEXT;
		}
		else if ( sType.Compare( _T("special") ) == 0 )
		{
			m_iType = LET_SPECIAL;
		}

		// Get justification values...
		sValue = pNode->GetSubValue( _T("justify"), _T("x") );
		sValue.MakeLower();
		if ( sValue.Compare(_T("center")) == 0 )
		{
			m_iJustifyX = 1;		// 0=Left,	1=Center,	2=Right
		}
		else if ( sValue.Compare(_T("left")) == 0 )
		{
			m_iJustifyX = 0;		// 0=Left,	1=Center,	2=Right
		}
		else if ( sValue.Compare(_T("right")) == 0 )
		{
			m_iJustifyX = 2;		// 0=Left,	1=Center,	2=Right
		}
		else
		{
			m_iJustifyX = 0;		// 0=Left,	1=Center,	2=Right
		}

		sValue = pNode->GetSubValue( _T("justify"), _T("y") );
		sValue.MakeLower();
		if ( sValue.Compare(_T("center")) == 0 )
		{
			m_iJustifyY = 1;		// 0=Top,	1=Center,	2=Bottom
		}
		else if ( sValue.Compare(_T("top")) == 0 )
		{
			m_iJustifyY = 0;		// 0=Top,	1=Center,	2=Bottom
		}
		else if ( sValue.Compare(_T("bottom")) == 0 )
		{
			m_iJustifyY = 2;		// 0=Top,	1=Center,	2=Bottom
		}
		else
		{
			m_iJustifyY = 0;		// 0=Top,	1=Center,	2=Bottom
		}

		// Get position values
		// X,Y,H,W,T,L,B,R

		sValue = pNode->GetSubValue( _T("pos"), _T("x") );
		if ( sValue.GetLength() )
		{
		}

		// Get color values

		hr = S_OK;

	}
	return hr;
}

HRESULT CLayout::ProcessLayoutElemNode( CXMLNode * pNode )
{
	HRESULT hr = E_FAIL;
	if ( pNode )
	{
		CStdString sType;
		CLayoutElement * pLayoutElement = NULL;

		sType = pNode->GetBasicValue( _T("type") );
		sType.MakeLower();
		if ( sType.Compare( _T("image") ) == 0 )
		{
			pLayoutElement = AddLayoutElement();
			hr = pLayoutElement->ProcessNode( pNode );
		}
		else if ( sType.Compare( _T("text") ) == 0 )
		{
			pLayoutElement = AddLayoutElement();
			hr = pLayoutElement->ProcessNode( pNode );
		}
		else if ( sType.Compare( _T("special") ) == 0 )
		{
			pLayoutElement = AddLayoutElement();
			hr = pLayoutElement->ProcessNode( pNode );
		}
		else
		{
			DEBUG_FORMAT( _T("UNKNOWN LAYOUT ELEMENT TYPE: %s"), sType.c_str() );
			// Don't error out on this one!
			hr = S_OK;
		}
	}
	return hr;
}

HRESULT CLayout::ProcessNode( CXMLNode * pNode )
{
	HRESULT hr = E_FAIL;
	if ( pNode )
	{
		CXMLNode * pLENode = NULL;
		int iIndex = 0;
		// iterate through each layoutelement
		pLENode = pNode->GetNode( _T("layoutelement"), iIndex );

		hr = S_OK;

		while( pLENode && SUCCEEDED(hr) )
		{
			hr = ProcessLayoutElemNode( pLENode );

			// Get next layout element node
			iIndex++;
			pLENode = pNode->GetNode( _T("layoutelement"), iIndex );
		}
	}
	return hr;
}


CLayoutElement * CLayout::GetLastElement( void )
{
	CLayoutElement *pReturn = NULL;
	TLayoutElements::reverse_iterator iterCurElement;

	iterCurElement = m_elements.rend(); 

	if ( iterCurElement != m_elements.rbegin() )
	{
		pReturn = iterCurElement->GetPtr();
	}
	return pReturn;
}

CLayoutElement * CLayout::AddLayoutElement( void )
{
	m_elements.insert( m_elements.end(), CLayoutElement() );
	return GetLastElement();
}


CLayoutElement * CLayout::GetFirstElement( void )
{
	CLayoutElement *pReturn = NULL;

	m_iterCurElement = m_elements.begin();
	if ( m_iterCurElement != m_elements.end() )
	{
		pReturn = m_iterCurElement->GetPtr();
	}
	return pReturn;
}

CLayoutElement * CLayout::GetNextElement( void )
{
	CLayoutElement *pReturn = NULL;

	if ( m_iterCurElement != m_elements.end() )
	{
		m_iterCurElement++;
		if ( m_iterCurElement != m_elements.end() )
		{
			pReturn = m_iterCurElement->GetPtr();
		}
	}
	return pReturn;
}


HRESULT CLayout::Render( LPDIRECT3DDEVICE8 pDevice )
{
	HRESULT hr = S_OK;

	CLayoutElement * pElement;

	// Now, cycle through all the layout elements and render them....
	pElement = GetFirstElement();
	while( pElement )
	{
		// Render pElement
		pElement->Render(pDevice);
		pElement = GetNextElement();
	}
	return hr;
}

void CLayout::Activate( void )
{
}

void CLayout::Deactivate( void )
{
}

void CLayout::Initialize( void )
{
}



CGenericSkin::CGenericSkin()
{
	m_pCacheLayout = NULL;
	m_sCacheLayoutName = _T("");
}

CGenericSkin::~CGenericSkin()
{
}

void CGenericSkin::SetDevices( LPDIRECT3DDEVICE8 pd3dDevice, LPDIRECTSOUND8 pDSound )
{
	if ( pd3dDevice )
	{
		m_pd3dDevice = pd3dDevice;
	}
	if ( pDSound )
	{
		m_pDSound = pDSound;
	}
}	



HRESULT	CGenericSkin::ProcessInfoNode( CXMLNode * pNode )
{
	HRESULT hr = E_FAIL;

	if ( pNode )
	{
		hr = S_OK;
		m_sVersion = pNode->GetBasicValue(_T("version"));
		m_sAuthor = pNode->GetBasicValue(_T("author"));
		m_sCopyright = pNode->GetBasicValue(_T("copyright"));
	}

	return hr;
}

HRESULT	CGenericSkin::ProcessResourceNode( CXMLNode * pNode )
{
	HRESULT hr = E_FAIL;
	CStdString sType;
	CStdString sName;
	CStdString sFile;

	if ( pNode )
	{
		sType = pNode->GetBasicValue( _T("type") );
		sName = pNode->GetBasicValue( _T("name") );
		sFile = pNode->GetBasicValue( _T("file") );

		sType.MakeLower();
		if ( sType.GetLength() && sName.GetLength() && sFile.GetLength() )
		{
			if ( sType.Compare( _T("font") ) == 0 )
			{
				if ( AddFontResource( sName, sFile ) )
				{
					hr = S_OK;
				}
			}
			else if ( sType.Compare( _T("image") ) == 0 )
			{
				if ( AddImgResource( sName, sFile ) )
				{
					hr = S_OK;
				}
			}
		}
	}
	return hr;
}

HRESULT	CGenericSkin::ProcessLayoutNode( CXMLNode * pNode )
{
	HRESULT hr = E_FAIL;
	CStdString sID;
	CLayout * pLayout = NULL;

	if ( pNode )
	{
		sID = pNode->GetBasicValue( _T("id") );
		sID.MakeLower();

		DEBUG_FORMAT( _T("       Layout (%s)"), sID.c_str() );

		if ( sID.GetLength() )
		{
			// Create a new layout
			pLayout = AddLayout( sID );
			if ( pLayout )
			{
				hr = pLayout->ProcessNode( pNode );
			}
		}
	}
	return hr;
}

	
HRESULT	CGenericSkin::ProcessMainNode( CXMLNode * pMainNode )
{
	HRESULT hr = E_FAIL;
	CStdString sName;
	CStdString sValue;
	CXMLNode * pNode = NULL;
	CXMLElement * pElement = NULL;
	int iIndex;

	// Begin processing XML nodes
	if ( pMainNode && pMainNode->m_sName.Compare( _T("skin") ) == 0 )
	{
		// Get name and continue....
		sName = pMainNode->GetBasicValue( _T("name") ); // pNode->m_msAttributes[_T("name")];
		if ( sName.GetLength() )
		{
			m_sSkinName = sName;
			// Continue processing
			iIndex = 0;
			pNode = pMainNode->GetNode( iIndex );
			hr = S_OK;
			while( pNode && SUCCEEDED(hr) )
			{
				if ( pNode->m_sName.Compare( _T("information") )==0 )
				{
					hr = ProcessInfoNode( pNode );
				}
				else if ( pNode->m_sName.Compare( _T("info") )==0 )
				{
					hr = ProcessInfoNode( pNode );
				}
				else if ( pNode->m_sName.Compare( _T("resource") )==0 )
				{
					hr = ProcessResourceNode( pNode );
				}
				else if ( pNode->m_sName.Compare( _T("layout") )==0 )
				{
					DEBUG_FORMAT( _T("Processing Layout") );
					hr = ProcessLayoutNode( pNode );
				}
				// Get the next node....
				iIndex++;
				pNode = pMainNode->GetNode( iIndex );
			}
		}
	}
	return hr;
}


HRESULT	CGenericSkin::LoadSkin( LPCTSTR szXMLFile )
{
	HRESULT hrReturn = S_OK;
	DWORD dwSize = 0;
	PBYTE pBuffer = NULL;
	CXMLNode * pMainNode;

//	CSkinLoader ldrSkin(this);

	CXMLLoader xmlLoader;


	// Base path is made from this filename...
	m_sBasePath = StripFilenameFromPath( szXMLFile );

	DEBUG_FORMAT( _T("Loading Generic Skin: %s"), szXMLFile );

	pMainNode = xmlLoader.LoadXMLNodes( szXMLFile );

	// Now process the nodes.
	hrReturn = ProcessMainNode( pMainNode );

#if 0
	ParseFile(szXMLFile, ldrSkin );
#endif
	return hrReturn;
}

CXBFont * CGenericSkin::GetFontResource( LPCTSTR szName )
{
	CXBFont * pReturn = NULL;
	TFontItems::iterator iterFont;

	iterFont = m_fonts.begin();
	while( iterFont != m_fonts.end() )
	{
		if ( iterFont->first.Compare( szName ) == 0 )
		{
			pReturn = (CXBFont *)&iterFont->second;

			iterFont = m_fonts.end();
		}
		else
		{
			iterFont++;
		}
	}
	return pReturn;
}

CXBFont * CGenericSkin::AddFontResource( LPCTSTR szName, LPCTSTR szFilePath )
{
	CXBFont * pReturn = NULL;
	CXBFont font;
	CStdString sFilePath;

	sFilePath = ::MakeFullFilePath( m_sBasePath, szFilePath );
	m_fonts.insert( TFontItems::value_type( szName, font ) );
	pReturn = GetFontResource( szName );
	if ( pReturn )
	{
		if ( szFilePath )
		{
			HRESULT hr = pReturn->Create( m_pd3dDevice, sFilePath );
			DEBUG_FORMAT( _T("Font Loaded: %s 0x%08x"), sFilePath.c_str(), hr );
		}
	}
	return pReturn;
}

CXBFont * CGenericSkin::AddFontResource( LPCTSTR szName, const LPBYTE pDataBuffer, LPCTSTR szSection )
{
	CXBFont * pReturn = NULL;
	CXBFont font;

	m_fonts.insert( TFontItems::value_type( szName, font ) );
	pReturn = GetFontResource( szName );
	if ( pReturn )
	{
		pReturn->CreateFromMem( m_pd3dDevice, szSection, pDataBuffer );
	}
	return pReturn;
}

CImageSource * CGenericSkin::GetImgResource( LPCTSTR szName )
{
	CImageSource * pReturn = NULL;
	TImageItems::iterator iterImg;

	iterImg = m_images.begin();
	while( iterImg != m_images.end() )
	{
		if ( iterImg->first.Compare( szName ) == 0 )
		{
			pReturn = (CImageSource *)&iterImg->second;

			iterImg = m_images.end();
		}
		else
		{
			iterImg++;
		}
	}
	return pReturn;
}

CImageSource * CGenericSkin::AddImgResource( LPCTSTR szName, LPCTSTR szFilePath )
{
	CImageSource * pReturn = NULL;
	CImageSource image;
	CStdString sFilePath;

	sFilePath = ::MakeFullFilePath( m_sBasePath, szFilePath );
	m_images.insert( TImageItems::value_type( szName, image ) );
	pReturn = GetImgResource( szName );
	if ( pReturn )
	{
		pReturn->Initialize( m_pd3dDevice, m_pDSound );
		if ( szFilePath )
		{
			HRESULT hr = pReturn->LoadImage( sFilePath );
			DEBUG_FORMAT( _T("Image Loaded: %s 0x%08x"), sFilePath.c_str(), hr );
		}
	}

	return pReturn;
}

CImageSource * CGenericSkin::AddImgResource( LPCTSTR szName, const LPBYTE pDataBuffer, DWORD dwSize, LPCTSTR szSection )
{
	CImageSource * pReturn = NULL;
	CImageSource image;

	m_images.insert( TImageItems::value_type( szName, image ) );
	pReturn = GetImgResource( szName );
	if ( pReturn )
	{
		pReturn->Initialize( m_pd3dDevice, m_pDSound );
		pReturn->LoadImageMem( pDataBuffer, dwSize );
	}

	return pReturn;
}

CLayout *	CGenericSkin::AddLayout( LPCTSTR szLayout )
{
	CLayout * pReturn = NULL;
	m_layouts.insert( TLayouts::value_type( szLayout, CLayout() ) );
	pReturn = GetLayout( szLayout );
	if ( pReturn )
	{
		DEBUG_FORMAT( _T("Layout '%s' Added"), szLayout );
	}
	return pReturn;
}

CLayout * CGenericSkin::GetLayout( LPCTSTR szLayout )
{
	CLayout * pReturn = NULL;
	TLayouts::iterator iterLayout;

	iterLayout = m_layouts.begin();
	while( iterLayout != m_layouts.end() )
	{
		if ( iterLayout->first.Compare(szLayout)==0 )
		{
			pReturn = &iterLayout->second;
			iterLayout = m_layouts.end();
		}
		else
		{
			iterLayout++;
		}
	}
	return pReturn;
}

HRESULT CGenericSkin::Render( LPCTSTR szLayout )
{
	HRESULT hr =  E_FAIL;

	// Check to see if we need to clear a previously cached layout
	if ( m_pCacheLayout )
	{
		if ( (szLayout==NULL) || m_sCacheLayoutName.Compare( szLayout ))
		{
			// Different layout? If so, change it
			if ( m_sCacheLayoutName.Compare( szLayout ) )
			{
				m_pCacheLayout->Deactivate();
				m_pCacheLayout = NULL;
				m_sCacheLayoutName = _T("");
			}
		}
	}

	// Activate a new layout, if we have one
	if ( szLayout && (m_pCacheLayout==NULL) )
	{
		m_sCacheLayoutName = szLayout;
		m_pCacheLayout = GetLayout(szLayout);
		m_pCacheLayout->Activate();
	}
	
	// Render the layout, if we have one
	if ( m_pCacheLayout )
	{
		hr = Render(m_pCacheLayout);
	}

	return hr;
}

HRESULT CGenericSkin::Render( CLayout * pLayout )
{
	HRESULT hr =  E_FAIL;

	if ( m_pCacheLayout != pLayout )
	{
		// Remove cached layout stuff, deactivate 'old' layout
		if ( m_pCacheLayout )
		{
			m_pCacheLayout->Deactivate();
			m_pCacheLayout = NULL;
			m_sCacheLayoutName = _T("");
		}
		if ( pLayout )
		{
			pLayout = m_pCacheLayout;
			m_sCacheLayoutName = m_pCacheLayout->GetName();
			m_pCacheLayout->Activate();
		}
	}
	if ( pLayout )
	{
		pLayout->Render( m_pd3dDevice );
	}
	return hr;
}
