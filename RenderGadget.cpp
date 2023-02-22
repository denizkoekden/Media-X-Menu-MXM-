

#include "RenderGadget.h"
#include "XBEasel.h"
#include "ImageSource.h"
#include "xbstateapp.h"
#include "FontCache.h"
#include "MenuInfo.h"
#include "ImgFileCache.h"

#include "CommDebug.h"

CXMLNode g_SkinNodes;
extern CFontCache g_FontCache;
CGadgetSkin g_GadgetSkin;


// Return value between -1.0 and 1.0
FLOAT GetPeriodValueFloat(DWORD dwTimeStamp, DWORD dwPeriod )
{
	float fReturn = 0.0;

	if ( dwPeriod )
	{
		dwTimeStamp %= dwPeriod;

		fReturn = (float)dwTimeStamp*2.0f*3.14159265359f;
		fReturn /= (float)dwPeriod;
	}
	fReturn = cosf(fReturn);
	return fReturn;
}


CModShifter::CModShifter() :
	m_bIncrement(true),
	m_lLength(0),
	m_dwTimeStamp(0),
	m_dwPeriod(1000),
	m_bWrap(false),
	m_bEnable(false)

{
}

CModShifter::~CModShifter()
{
}

void CModShifter::InternalConfigure( CXMLNode * pNode )
{
}


/*

<Modifier type="Shifter" target="x">
 <Period>3000</Period>
 <Wrap>True</Wrap>
</Modifier>

*/
void CModShifter::Configure( CXMLNode * pNode )
{
	if ( pNode && (_tcsicmp( pNode->m_sName, _T("modifier") ) == 0 ) )
	{
		m_bIncrement = (pNode->GetBool( NULL, _T("ShiftLeft"), TRUE )==TRUE);
		m_dwPeriod = pNode->GetLong( NULL, _T("period"), 3000 );
		m_bWrap = (pNode->GetBool( NULL, _T("wrap"), TRUE )==TRUE);
		m_bEnable = true;
		m_dwTimeStamp = GetTickCount();
		m_lLength = 480;
	}
}

long	CModShifter::GetOffset( DWORD dwTimeStamp )
{
	long lReturn = 0;

	if  ( m_bEnable && m_lLength && m_dwPeriod && m_dwTimeStamp )
	{
		DWORD dwDifference;
		long lOffset;

		dwDifference = dwTimeStamp-m_dwTimeStamp;
		dwDifference %= m_dwPeriod;
		lOffset = (long)dwDifference;
		lOffset *= m_lLength;
		lOffset /= (long)m_dwPeriod;
		if ( m_bIncrement )
		{
			lReturn = lOffset;
		}
		else
		{
			lReturn = 0-lOffset;
		}
	}
	return lReturn;
}


//Modifiers:
//Bounce
// Take Rectangle defined, and move it around the screen.
//  Vector: 1 or -1 for X and Y
//  Has bounding box
// <Modifier Type="Bounce" Target="Pos">
//  <Bounds T="10" L="10" W="620" H="460"/>
// </Modifier>

CColorModHandler::CColorModHandler()
{
	m_clrModFlicker.SetEnable(false);
	m_clrModThrob.SetEnable(false);
}

CColorModHandler::CColorModHandler(const CColorModHandler & src )
{
	m_clrModFlicker = src.m_clrModFlicker;
	m_clrModThrob = src.m_clrModThrob;
}


DWORD CColorModHandler::ModifyColor( DWORD dwARGB )
{
	DWORD dwReturn;

	dwReturn = m_clrModFlicker.ModifyColor(dwARGB);
	dwReturn = m_clrModThrob.ModifyColor(dwReturn);
	return dwReturn;
}

bool CColorModHandler::Configure( CXMLNode * pNode, LPCTSTR szTarget )
{
	bool bReturn = false;
	CStdString sTarget, sModifierType;

	if ( pNode && (_tcsicmp( pNode->m_sName, _T("modifier") ) == 0 ) && szTarget && _tcslen(szTarget) )
	{
		sModifierType = pNode->GetString( NULL, _T("type"), _T("") );
		sTarget = pNode->GetString( NULL, _T("target"), _T("") );
		if ( _tcsicmp( szTarget, sTarget ) == 0 )
		{
			if ( _tcsicmp( sModifierType, _T("throb") ) == 0 )
			{
				bReturn = m_clrModThrob.Configure(pNode);
				m_clrModThrob.SetEnable(true);
			}
			else if ( _tcsicmp( sModifierType, _T("flicker") ) == 0 )
			{
				bReturn = m_clrModFlicker.Configure(pNode);
				m_clrModFlicker.SetEnable(true);
			}
		}
	}
	return bReturn;
}

CModColor * CColorModHandler::GetColorMod( int iColorModID )
{
	CModColor *pReturn = NULL;
	switch( iColorModID )
	{
		case CM_FLICKER:
			pReturn = &m_clrModFlicker;
			break;
		case CM_THROB:
			pReturn = &m_clrModThrob;
			break;
	}
	return pReturn;
}

bool CColorModHandler::Enabled( int iColorModID )
{
	bool bReturn = false;
	CModColor *pColorMod = GetColorMod( iColorModID );

	if ( pColorMod )
	{
		bReturn = pColorMod->Enabled();
	}
	return bReturn;
}

void CColorModHandler::SetEnable( int iColorModID, bool bEnable )
{
	CModColor *pColorMod = GetColorMod( iColorModID );

	if ( pColorMod )
	{
		pColorMod->SetEnable(bEnable);
	}
}

void CColorModHandler::SetPeriod( int iColorModID, int iColorElement, DWORD dwPeriod )
{
	CModColor *pColorMod = GetColorMod( iColorModID );

	if ( pColorMod )
	{
		pColorMod->SetPeriod(iColorElement, dwPeriod);
	}
}

DWORD CColorModHandler::GetPeriod( int iColorModID, int iColorElement )
{
	DWORD dwReturn = 0;
	CModColor *pColorMod = GetColorMod( iColorModID );

	if ( pColorMod )
	{
		dwReturn = pColorMod->GetPeriod(iColorElement);
	}
	return dwReturn;
}

void CColorModHandler::SetColorEnable( int iColorModID, int iColorElement, BOOL bEnable )
{
	CModColor *pColorMod = GetColorMod( iColorModID );

	if ( pColorMod )
	{
		pColorMod->SetColorEnable(iColorElement, bEnable );
	}
}

BOOL CColorModHandler::GetColorEnable( int iColorModID, int iColorElement )
{
	BOOL bReturn = 0;
	CModColor *pColorMod = GetColorMod( iColorModID );

	if ( pColorMod )
	{
		bReturn = pColorMod->GetColorEnable(iColorElement);
	}
	return bReturn;
}

void CColorModHandler::SetRange( int iColorModID, int iColorElement, int iMin, int iMax )
{
	CModColor *pColorMod = GetColorMod( iColorModID );

	if ( pColorMod )
	{
		pColorMod->SetRange(iColorElement, iMin, iMax );
	}
}

void CColorModHandler::SetMin( int iColorModID, int iColorElement, int iMin )
{
	CModColor *pColorMod = GetColorMod( iColorModID );

	if ( pColorMod )
	{
		pColorMod->SetMin(iColorElement, iMin);
	}
}

void CColorModHandler::SetMax( int iColorModID, int iColorElement, int iMax )
{
	CModColor *pColorMod = GetColorMod( iColorModID );

	if ( pColorMod )
	{
		pColorMod->SetMax(iColorElement, iMax);
	}
}

int CColorModHandler::GetMin( int iColorModID, int iColorElement )
{
	int iReturn = 0;
	CModColor *pColorMod = GetColorMod( iColorModID );

	if ( pColorMod )
	{
		iReturn = pColorMod->GetMin(iColorElement);
	}
	return iReturn;
}

int CColorModHandler::GetMax( int iColorModID, int iColorElement )
{
	int iReturn = 0;
	CModColor *pColorMod = GetColorMod( iColorModID );

	if ( pColorMod )
	{
		iReturn = pColorMod->GetMax(iColorElement);
	}
	return iReturn;
}


CModColor::CModColor()
{
	int iIndex;

	m_bEnable = false;
	for( iIndex=0; iIndex<4; iIndex++ )
	{
		m_colorInfo[iIndex].m_dwPeriod = 3000;
		m_colorInfo[iIndex].m_iMin = 0;
		m_colorInfo[iIndex].m_iMax = 255;
		if ( iIndex == CIDX_ALPHA )
		{
			m_colorInfo[iIndex].m_bEnableColor = true;
		}
		else
		{
			m_colorInfo[iIndex].m_bEnableColor = false;
		}
	}
}

CModColor::CModColor( const CModColor & src )
{
	int iIndex;

	m_bEnable = src.m_bEnable;
	for( iIndex=0; iIndex<4; iIndex++ )
	{
		m_colorInfo[iIndex].m_dwPeriod = src.m_colorInfo[iIndex].m_dwPeriod;
		m_colorInfo[iIndex].m_iMin = src.m_colorInfo[iIndex].m_iMin;
		m_colorInfo[iIndex].m_iMax = src.m_colorInfo[iIndex].m_iMax;
		m_colorInfo[iIndex].m_bEnableColor = src.m_colorInfo[iIndex].m_bEnableColor;
	}
}


CModColor::~CModColor()
{
}

void CModColor::SetPeriod( int iColorElement, DWORD dwPeriod )
{
	switch( iColorElement)
	{
		case CIDX_ALPHA:
			m_colorInfo[CIDX_ALPHA].m_dwPeriod = dwPeriod;
			break;
		case CIDX_RED:
			m_colorInfo[CIDX_RED].m_dwPeriod = dwPeriod;
			break;
		case CIDX_GREEN:
			m_colorInfo[CIDX_GREEN].m_dwPeriod = dwPeriod;
			break;
		case CIDX_BLUE:
			m_colorInfo[CIDX_BLUE].m_dwPeriod = dwPeriod;
			break;
		case CIDX_ARGB:
			m_colorInfo[CIDX_ALPHA].m_dwPeriod = dwPeriod;
			m_colorInfo[CIDX_RED].m_dwPeriod = dwPeriod;
			m_colorInfo[CIDX_GREEN].m_dwPeriod = dwPeriod;
			m_colorInfo[CIDX_BLUE].m_dwPeriod = dwPeriod;
			break;
		case CIDX_RGB:
			m_colorInfo[CIDX_RED].m_dwPeriod = dwPeriod;
			m_colorInfo[CIDX_GREEN].m_dwPeriod = dwPeriod;
			m_colorInfo[CIDX_BLUE].m_dwPeriod = dwPeriod;
			break;
	}
}

DWORD CModColor::GetPeriod( int iColorElement )
{
	DWORD dwReturn = 0;
	switch( iColorElement)
	{
		case CIDX_ALPHA:
			dwReturn = m_colorInfo[CIDX_ALPHA].m_dwPeriod;
			break;
		case CIDX_RED:
			dwReturn = m_colorInfo[CIDX_RED].m_dwPeriod;
			break;
		case CIDX_GREEN:
			dwReturn = m_colorInfo[CIDX_GREEN].m_dwPeriod;
			break;
		case CIDX_BLUE:
			dwReturn = m_colorInfo[CIDX_BLUE].m_dwPeriod;
			break;
	}
	return dwReturn;
}

void CModColor::SetColorEnable( int iColorElement, BOOL bEnable )
{
	switch( iColorElement)
	{
		case CIDX_ALPHA:
			m_colorInfo[CIDX_ALPHA].m_bEnableColor = bEnable?true:false;
			break;
		case CIDX_RED:
			m_colorInfo[CIDX_RED].m_bEnableColor = bEnable?true:false;
			break;
		case CIDX_GREEN:
			m_colorInfo[CIDX_GREEN].m_bEnableColor = bEnable?true:false;
			break;
		case CIDX_BLUE:
			m_colorInfo[CIDX_BLUE].m_bEnableColor = bEnable?true:false;
			break;
		case CIDX_ARGB:
			m_colorInfo[CIDX_ALPHA].m_bEnableColor = bEnable?true:false;
			m_colorInfo[CIDX_RED].m_bEnableColor = bEnable?true:false;
			m_colorInfo[CIDX_GREEN].m_bEnableColor = bEnable?true:false;
			m_colorInfo[CIDX_BLUE].m_bEnableColor = bEnable?true:false;
			break;
		case CIDX_RGB:
			m_colorInfo[CIDX_RED].m_bEnableColor = bEnable?true:false;
			m_colorInfo[CIDX_GREEN].m_bEnableColor = bEnable?true:false;
			m_colorInfo[CIDX_BLUE].m_bEnableColor = bEnable?true:false;
			break;
	}
}

BOOL CModColor::GetColorEnable( int iColorElement )
{
	BOOL bReturn = FALSE;
	switch( iColorElement)
	{
		case CIDX_ALPHA:
			bReturn = m_colorInfo[CIDX_ALPHA].m_bEnableColor;
			break;
		case CIDX_RED:
			bReturn = m_colorInfo[CIDX_RED].m_bEnableColor;
			break;
		case CIDX_GREEN:
			bReturn = m_colorInfo[CIDX_GREEN].m_bEnableColor;
			break;
		case CIDX_BLUE:
			bReturn = m_colorInfo[CIDX_BLUE].m_bEnableColor;
			break;
	}
	return bReturn;
}

void CModColor::SetRange( int iColorElement, int iMin, int iMax )
{
	SetMin( iColorElement, iMin );
	SetMax( iColorElement, iMax );
}

void CModColor::SetMin( int iColorElement, int iMin )
{
	switch( iColorElement)
	{
		case CIDX_ALPHA:
			m_colorInfo[CIDX_ALPHA].m_iMin = iMin;
			break;
		case CIDX_RED:
			m_colorInfo[CIDX_RED].m_iMin = iMin;
			break;
		case CIDX_GREEN:
			m_colorInfo[CIDX_GREEN].m_iMin = iMin;
			break;
		case CIDX_BLUE:
			m_colorInfo[CIDX_BLUE].m_iMin = iMin;
			break;
		case CIDX_ARGB:
			m_colorInfo[CIDX_ALPHA].m_iMin = iMin;
			m_colorInfo[CIDX_RED].m_iMin = iMin;
			m_colorInfo[CIDX_GREEN].m_iMin = iMin;
			m_colorInfo[CIDX_BLUE].m_iMin = iMin;
			break;
		case CIDX_RGB:
			m_colorInfo[CIDX_RED].m_iMin = iMin;
			m_colorInfo[CIDX_GREEN].m_iMin = iMin;
			m_colorInfo[CIDX_BLUE].m_iMin = iMin;
			break;
	}
}

void CModColor::SetMax( int iColorElement, int iMax )
{
	switch( iColorElement)
	{
		case CIDX_ALPHA:
			m_colorInfo[CIDX_ALPHA].m_iMax = iMax;
			break;
		case CIDX_RED:
			m_colorInfo[CIDX_RED].m_iMax = iMax;
			break;
		case CIDX_GREEN:
			m_colorInfo[CIDX_GREEN].m_iMax = iMax;
			break;
		case CIDX_BLUE:
			m_colorInfo[CIDX_BLUE].m_iMax = iMax;
			break;
		case CIDX_ARGB:
			m_colorInfo[CIDX_ALPHA].m_iMax = iMax;
			m_colorInfo[CIDX_RED].m_iMax = iMax;
			m_colorInfo[CIDX_GREEN].m_iMax = iMax;
			m_colorInfo[CIDX_BLUE].m_iMax = iMax;
			break;
		case CIDX_RGB:
			m_colorInfo[CIDX_RED].m_iMax = iMax;
			m_colorInfo[CIDX_GREEN].m_iMax = iMax;
			m_colorInfo[CIDX_BLUE].m_iMax = iMax;
			break;
	}
}

int CModColor::GetMin( int iColorElement )
{
	BOOL iReturn = 0;
	switch( iColorElement)
	{
		case CIDX_ALPHA:
			iReturn = m_colorInfo[CIDX_ALPHA].m_iMin;
			break;
		case CIDX_RED:
			iReturn = m_colorInfo[CIDX_RED].m_iMin;
			break;
		case CIDX_GREEN:
			iReturn = m_colorInfo[CIDX_GREEN].m_iMin;
			break;
		case CIDX_BLUE:
			iReturn = m_colorInfo[CIDX_BLUE].m_iMin;
			break;
	}
	return iReturn;
}

int CModColor::GetMax( int iColorElement )
{
	BOOL iReturn = 0;
	switch( iColorElement)
	{
		case CIDX_ALPHA:
			iReturn = m_colorInfo[CIDX_ALPHA].m_iMax;
			break;
		case CIDX_RED:
			iReturn = m_colorInfo[CIDX_RED].m_iMax;
			break;
		case CIDX_GREEN:
			iReturn = m_colorInfo[CIDX_GREEN].m_iMax;
			break;
		case CIDX_BLUE:
			iReturn = m_colorInfo[CIDX_BLUE].m_iMax;
			break;
	}
	return iReturn;
}


/*
DEFAULT Alpha 0,255
<Modifier Type="Color" Target="ShadowColor">
 <Red Enable="True" Min="0" Max="255" />
 <Alpha Enable="True" Min="0" Max="255" />
</Modifier>
*/

bool CModColor::Configure( CXMLNode * pNode )
{
	bool bReturn = false;
	if ( m_colorInfo[CIDX_RED].m_bEnableColor = (pNode->GetBool( _T("red"), _T("enable"), FALSE )==TRUE) )
	{
		bReturn = true;
		// Default this back to off now...
		m_colorInfo[CIDX_ALPHA].m_bEnableColor = false;

		m_colorInfo[CIDX_RED].m_iMin = pNode->GetLong( _T("red"), _T("min"), 0 );
		m_colorInfo[CIDX_RED].m_iMax = pNode->GetLong( _T("red"), _T("max"), 255 );
	}
	if ( m_colorInfo[CIDX_GREEN].m_bEnableColor = (pNode->GetBool( _T("green"), _T("enable"), FALSE )==TRUE) )
	{
		bReturn = true;
		// Default this back to off now...
		m_colorInfo[CIDX_ALPHA].m_bEnableColor = false;

		m_colorInfo[CIDX_GREEN].m_iMin = pNode->GetLong( _T("green"), _T("min"), 0 );
		m_colorInfo[CIDX_GREEN].m_iMax = pNode->GetLong( _T("green"), _T("max"), 255 );
	}
	if ( m_colorInfo[CIDX_BLUE].m_bEnableColor = (pNode->GetBool( _T("blue"), _T("enable"), FALSE )==TRUE) )
	{
		bReturn = true;
		// Default this back to off now...
		m_colorInfo[CIDX_ALPHA].m_bEnableColor = false;

		m_colorInfo[CIDX_BLUE].m_iMin = pNode->GetLong( _T("blue"), _T("min"), 0 );
		m_colorInfo[CIDX_BLUE].m_iMax = pNode->GetLong( _T("blue"), _T("max"), 255 );
	}
	if ( m_colorInfo[CIDX_ALPHA].m_bEnableColor = (pNode->GetBool( _T("alpha"), _T("enable"), FALSE )==TRUE) )
	{
		bReturn = true;
		m_colorInfo[CIDX_ALPHA].m_iMin = pNode->GetLong( _T("alpha"), _T("min"), 0 );
		m_colorInfo[CIDX_ALPHA].m_iMax = pNode->GetLong( _T("alpha"), _T("max"), 255 );
	}
	InternalConfigure( pNode );
	return bReturn;
}




CModClrThrob::CModClrThrob()
{
}

CModClrThrob::~CModClrThrob()
{
}

DWORD CModClrThrob::ModifyColor( DWORD dwARGB )
{
	DWORD dwRed, dwGreen, dwBlue, dwAlpha;
	FLOAT fDelta;
	int iIndex;
	DWORD dwTimeStamp = GetTickCount();

	if ( m_bEnable )
	{
		dwAlpha = (dwARGB>>24)&0xff;
		dwRed = (dwARGB>>16)&0xff;
		dwGreen = (dwARGB>>8)&0xff;
		dwBlue = dwARGB&0xff;
		for( iIndex=0; iIndex<4; iIndex++ )
		{
			if ( m_colorInfo[iIndex].m_bEnableColor )
			{
				fDelta = (FLOAT)(m_colorInfo[iIndex].m_iMax-m_colorInfo[iIndex].m_iMin);
				fDelta /= 2.0f;
				fDelta *= (GetPeriodValueFloat( dwTimeStamp, m_colorInfo[iIndex].m_dwPeriod )+1.0f);
				fDelta += 0.5;
				switch( iIndex )
				{
					case CIDX_ALPHA:
						dwAlpha = m_colorInfo[iIndex].m_iMin+(int)fDelta;
						dwAlpha &= 0xff;
						break;
					case CIDX_RED:
						dwRed = m_colorInfo[iIndex].m_iMin+(int)fDelta;
						dwRed &= 0xff;
						break;
					case CIDX_GREEN:
						dwGreen = m_colorInfo[iIndex].m_iMin+(int)fDelta;
						dwGreen &= 0xff;
						break;
					case CIDX_BLUE:
						dwBlue = m_colorInfo[iIndex].m_iMin+(int)fDelta;
						dwBlue &= 0xff;
						break;
				}
			}
		}

		dwAlpha <<= 24;
		dwAlpha |= (dwRed<<16);
		dwAlpha |= (dwGreen<<8);
		dwAlpha |= dwBlue;
	}
	else
	{
		dwAlpha = dwARGB;
	}

	return dwAlpha;
}

void CModClrThrob::InternalConfigure( CXMLNode * pNode )
{
	m_colorInfo[CIDX_ALPHA].m_dwPeriod = pNode->GetLong( _T("alpha"), _T("period"), 3000 );
	m_colorInfo[CIDX_RED].m_dwPeriod = pNode->GetLong( _T("red"), _T("period"), 3000 );
	m_colorInfo[CIDX_GREEN].m_dwPeriod = pNode->GetLong( _T("green"), _T("period"), 3000 );
	m_colorInfo[CIDX_BLUE].m_dwPeriod = pNode->GetLong( _T("blue"), _T("period"), 3000 );
}


CModClrFlicker::CModClrFlicker()
{
}

CModClrFlicker::~CModClrFlicker()
{
}

DWORD CModClrFlicker::ModifyColor( DWORD dwARGB )
{
	DWORD dwRed, dwGreen, dwBlue, dwAlpha;
	FLOAT fDelta;
	int iIndex;
	int iRand;
	DWORD dwTimeStamp = GetTickCount();


	if ( m_bEnable )
	{
		dwAlpha = (dwARGB>>24)&0xff;
		dwRed = (dwARGB>>16)&0xff;
		dwGreen = (dwARGB>>8)&0xff;
		dwBlue = dwARGB&0xff;
		for( iIndex=0; iIndex<4; iIndex++ )
		{
			if ( m_colorInfo[iIndex].m_bEnableColor )
			{
				fDelta = (FLOAT)(m_colorInfo[iIndex].m_iMax-m_colorInfo[iIndex].m_iMin);
				iRand = (RandomNum()%1501);
				if ( iRand > 1000 )
				{
					iRand = 1000-(RandomNum()%250);
				}
				fDelta *= (FLOAT)iRand; // (rand()%1001);
				fDelta /= 1000.0f;
				fDelta += 0.5;
				switch( iIndex )
				{
					case CIDX_ALPHA:
						dwAlpha = m_colorInfo[iIndex].m_iMin+(int)fDelta;
						dwAlpha &= 0xff;
						break;
					case CIDX_RED:
						dwRed = m_colorInfo[iIndex].m_iMin+(int)fDelta;
						dwRed &= 0xff;
						break;
					case CIDX_GREEN:
						dwGreen = m_colorInfo[iIndex].m_iMin+(int)fDelta;
						dwGreen &= 0xff;
						break;
					case CIDX_BLUE:
						dwBlue = m_colorInfo[iIndex].m_iMin+(int)fDelta;
						dwBlue &= 0xff;
						break;
				}
			}
		}

		dwAlpha <<= 24;
		dwAlpha |= (dwRed<<16);
		dwAlpha |= (dwGreen<<8);
		dwAlpha |= dwBlue;
	}
	else
	{
		dwAlpha = dwARGB;
	}

	return dwAlpha;
}

void CModClrFlicker::InternalConfigure( CXMLNode * pNode )
{
}




CModPosition::CModPosition()
{
	m_bEnable = false;
}

CModPosition::~CModPosition()
{
}

void CModPosition::Configure( CXMLNode * pNode )
{
	InternalConfigure( pNode );
}


CModSizeSpaz::CModSizeSpaz()
{
	m_iRangeX = 0;
	m_iRangeY = 0;
	m_iOffsetX = 0;
	m_iOffsetY = 0;
}

CModSizeSpaz::~CModSizeSpaz()
{
}

bool CModSizeSpaz::ModifyPosRect( RECT & rectInit, RECT & rectResult )
{
	int iDeltaX;
	int iDeltaY;
	bool bResult = false;

	if ( m_bEnable )
	{
		iDeltaX = 3-(RandomNum()%6);
		iDeltaY = 3-(RandomNum()%6);

		m_iOffsetX += iDeltaX;
		m_iOffsetY += iDeltaY;
		
		if ( m_iOffsetX > m_iRangeX )
		{
			m_iOffsetX = m_iRangeX;
		}
		if ( m_iOffsetX < (0-m_iRangeX) )
		{
			m_iOffsetX = (0-m_iRangeX);
		}

		if ( m_iOffsetY > m_iRangeY )
		{
			m_iOffsetY = m_iRangeY;
		}
		if ( m_iOffsetY < (0-m_iRangeY) )
		{
			m_iOffsetY = (0-m_iRangeY);
		}
		memcpy( &rectResult, &rectInit, sizeof( RECT ) );
		rectResult.left += m_iOffsetX;
		rectResult.right -= m_iOffsetX;
		rectResult.top += m_iOffsetY;
		rectResult.bottom -= m_iOffsetY;
		bResult = true;
	}
	return bResult;
}

// Parameters for this one: <Range x="3" y="3"/>
void CModSizeSpaz::InternalConfigure( CXMLNode * pNode )
{
	m_iRangeX = pNode->GetLong( _T("range"), _T("x"), 2 );
	m_iRangeY = pNode->GetLong( _T("range"), _T("y"), 2 );
}

CModSizePulse::CModSizePulse()
{
	m_iRangeX = 0;
	m_iRangeY = 0;
	m_iOffsetX = 0;
	m_iOffsetY = 0;
}

CModSizePulse::~CModSizePulse()
{
}

bool CModSizePulse::ModifyPosRect( RECT & rectInit, RECT & rectResult )
{
	bool bResult = false;
	FLOAT fThrob;

	if ( m_bEnable )
	{
		fThrob = GetPeriodValueFloat( GetTickCount(), m_dwThrobPeriod );
		m_iOffsetX = (int)(fThrob*((FLOAT)m_iRangeX));
		m_iOffsetY = (int)(fThrob*((FLOAT)m_iRangeY));

		memcpy( &rectResult, &rectInit, sizeof( RECT ) );
		rectResult.left += m_iOffsetX;
		rectResult.right -= m_iOffsetX;
		rectResult.top += m_iOffsetY;
		rectResult.bottom -= m_iOffsetY;
		bResult = true;
	}
	return bResult;
}

// Parameters for this one: <Range x="3" y="3"/>
void CModSizePulse::InternalConfigure( CXMLNode * pNode )
{
	m_iRangeX = pNode->GetLong( _T("range"), _T("x"), 2 );
	m_iRangeY = pNode->GetLong( _T("range"), _T("y"), 2 );
	m_dwThrobPeriod = pNode->GetDWORD( NULL, _T("throbperiod"), 3000 );
}



CModPosTremor::CModPosTremor()
{
	m_iRangeX = 0;
	m_iRangeY = 0;
	m_iOffsetX = 0;
	m_iOffsetY = 0;
}

CModPosTremor::~CModPosTremor()
{
}

bool CModPosTremor::ModifyPosRect( RECT & rectInit, RECT & rectResult )
{
	int iDeltaX;
	int iDeltaY;
	bool bResult = false;

	if ( m_bEnable )
	{
		iDeltaX = 3-(RandomNum()%6);
		iDeltaY = 3-(RandomNum()%6);

		m_iOffsetX += iDeltaX;
		m_iOffsetY += iDeltaY;
		
		if ( m_iOffsetX > m_iRangeX )
		{
			m_iOffsetX = m_iRangeX;
		}
		if ( m_iOffsetX < (0-m_iRangeX) )
		{
			m_iOffsetX = (0-m_iRangeX);
		}

		if ( m_iOffsetY > m_iRangeY )
		{
			m_iOffsetY = m_iRangeY;
		}
		if ( m_iOffsetY < (0-m_iRangeY) )
		{
			m_iOffsetY = (0-m_iRangeY);
		}
		memcpy( &rectResult, &rectInit, sizeof( RECT ) );
		rectResult.left += m_iOffsetX;
		rectResult.right += m_iOffsetX;
		rectResult.top += m_iOffsetY;
		rectResult.bottom += m_iOffsetY;
		bResult = true;
	}
	return bResult;
}

// Parameters for this one: <Range x="3" y="3"/>
void CModPosTremor::InternalConfigure( CXMLNode * pNode )
{
	m_iRangeX = pNode->GetLong( _T("range"), _T("x"), 2 );
	m_iRangeY = pNode->GetLong( _T("range"), _T("y"), 2 );
}

CModPosBounce::CModPosBounce()
{
	bool bPosDir;

	if ( RandomNum()&0x01 )
	{
		bPosDir = true;
	}
	else
	{
		bPosDir = false;
	}
	m_iXVector = bPosDir?1:-1;
	if ( RandomNum()&0x01 )
	{
		bPosDir = true;
	}
	else
	{
		bPosDir = false;
	}
	m_iYVector = bPosDir?1:-1;
	m_rectBounds.left = 0;
	m_rectBounds.top = 0;
	m_rectBounds.right = 639;
	m_rectBounds.bottom = 479;
	m_bInitRect = true;
}

CModPosBounce::~CModPosBounce()
{
}

void CModPosBounce::SetSize( int iWidth, int iHeight )
{ 
	int iCurrWidth, iCurrHeight;
	// memcpy( &m_rectCurrent, &rectInit, sizeof(RECT) );
	if ( (m_rectBounds.bottom-m_rectBounds.top) < iHeight )
	{
		iHeight = (m_rectBounds.bottom-m_rectBounds.top)-1;
	}
	if ( (m_rectBounds.right-m_rectBounds.left) < iWidth )
	{
		iWidth = (m_rectBounds.right-m_rectBounds.left)-1;
	}
	iCurrWidth = m_rectCurrent.right-m_rectCurrent.left;
	iCurrHeight = m_rectCurrent.bottom-m_rectCurrent.top;
	if ( iCurrHeight != iHeight )
	{
		m_rectCurrent.top += ((iCurrHeight-iHeight)/2);
		m_rectCurrent.bottom = m_rectCurrent.top+(iHeight+1);
	}
	if ( iCurrWidth != iWidth )
	{
		m_rectCurrent.left += ((iCurrWidth-iWidth)/2);
		m_rectCurrent.right = m_rectCurrent.left+(iWidth+1);
	}
}


bool CModPosBounce::ModifyPosRect( RECT & rectInit, RECT & rectResult )
{
	bool bReturn = false;

	if ( m_bInitRect )
	{
		int iWidth, iHeight;
		m_bInitRect = false;
		memcpy( &m_rectCurrent, &rectInit, sizeof(RECT) );
		m_rectCurrent.left = rectInit.left;
		m_rectCurrent.right = rectInit.right;
		m_rectCurrent.top = rectInit.top;
		m_rectCurrent.bottom = rectInit.bottom;
		iWidth = m_rectCurrent.right-m_rectCurrent.left;
		iHeight = m_rectCurrent.bottom-m_rectCurrent.top;
		if ( (m_rectBounds.right-m_rectBounds.left) < (iWidth-1) )
		{
			m_rectBounds.right = m_rectBounds.left+(iWidth-1);
			if ( m_rectBounds.right > 639 )
			{
				m_rectBounds.right = 639;
				m_rectBounds.left = m_rectBounds.right-(iWidth-1);
			}
		}
		if ( (m_rectBounds.bottom-m_rectBounds.top) < (iHeight-1) )
		{
			m_rectBounds.bottom = m_rectBounds.top+(iHeight-1);
			if ( m_rectBounds.bottom > 639 )
			{
				m_rectBounds.bottom = 639;
				m_rectBounds.top = m_rectBounds.bottom-(iHeight-1);
			}
		}
	}
	if ( m_bEnable )
	{
		if ( m_iXVector == 1 )
		{
			// Move to right 1
			if ( m_rectCurrent.right < m_rectBounds.right )
			{
				m_rectCurrent.right++;
				m_rectCurrent.left++;
			}
			else
			{
				m_iXVector = -1;
				m_rectCurrent.right--;
				m_rectCurrent.left--;
			}
		}
		else
		{
			// Move to right 1
			if ( m_rectCurrent.left > m_rectBounds.left )
			{
				m_rectCurrent.right--;
				m_rectCurrent.left--;
			}
			else
			{
				m_iXVector = 1;
				m_rectCurrent.right++;
				m_rectCurrent.left++;
			}
		}
		if ( m_iYVector == 1 )
		{
			// Move to right 1
			if ( m_rectCurrent.bottom < m_rectBounds.bottom )
			{
				m_rectCurrent.bottom++;
				m_rectCurrent.top++;
			}
			else
			{
				m_iYVector = -1;
				m_rectCurrent.bottom--;
				m_rectCurrent.top--;
			}
		}
		else
		{
			// Move to right 1
			if ( m_rectCurrent.top > m_rectBounds.top )
			{
				m_rectCurrent.bottom--;
				m_rectCurrent.top--;
			}
			else
			{
				m_iYVector = 1;
				m_rectCurrent.bottom++;
				m_rectCurrent.top++;
			}
		}

		memcpy( &rectResult, &m_rectCurrent, sizeof(RECT) );

		bReturn = true;
	}
	return bReturn;
}

void CModPosBounce::InternalConfigure( CXMLNode * pNode )
{
	int iWidth, iHeight;

	m_bInitRect = true;

	m_rectBounds.left = pNode->GetLong( _T("bounds"), _T("l"), pNode->GetLong( _T("bounds"), _T("x"), 0 ));
	m_rectBounds.top = pNode->GetLong( _T("bounds"), _T("t"), pNode->GetLong( _T("bounds"), _T("y"), 0 ));

	iWidth = pNode->GetLong( _T("bounds"), _T("w"), -1 );
	if ( iWidth != -1 )
	{
		m_rectBounds.right = m_rectBounds.left+(iWidth-1);
	}
	else
	{
		m_rectBounds.right = pNode->GetLong( _T("bounds"), _T("r"), 639 );
	}

	iHeight = pNode->GetLong( _T("bounds"), _T("h"), -1 );
	if ( iHeight != -1 )
	{
		m_rectBounds.bottom = m_rectBounds.top+(iHeight-1);
	}
	else
	{
		m_rectBounds.bottom = pNode->GetLong( _T("bounds"), _T("b"), 639 );
	}
	if ( m_rectBounds.left < 0 )
	{
		m_rectBounds.left = 0;
	}
	if ( m_rectBounds.left > 638 )
	{
		m_rectBounds.left = 638;
	}
	if( m_rectBounds.right < 1 )
	{
		m_rectBounds.right = 1;
	}
	if( m_rectBounds.right > 639 )
	{
		m_rectBounds.right = 639;
	}
	if ( m_rectBounds.left >= m_rectBounds.right )
	{
		m_rectBounds.left = m_rectBounds.right-1;
	}

	if ( m_rectBounds.top < 0 )
	{
		m_rectBounds.top = 0;
	}
	if ( m_rectBounds.top > 478 )
	{
		m_rectBounds.top = 478;
	}
	if( m_rectBounds.bottom < 1 )
	{
		m_rectBounds.bottom = 1;
	}
	if( m_rectBounds.bottom > 479 )
	{
		m_rectBounds.bottom = 479;
	}
	if ( m_rectBounds.top >= m_rectBounds.bottom )
	{
		m_rectBounds.top = m_rectBounds.bottom-1;
	}
}


CRenderGadget::CRenderGadget() :
	m_pDevice(NULL),
	m_pParent(NULL),
	m_pRenderTexture(NULL),
	m_bInit(false),
	m_bProportional(false),
	m_iWidth(640),
	m_iHeight(480),
	m_iOriginX(0),
	m_iOriginY(0),
	m_iConditionID(-1),
	m_bNotCondition(false),
	m_dwClearAttribute(0),
	m_tmrDelay(0),
	m_tmrLifetime(-1),
	m_tmrCycleOn(-1),
	m_tmrCycleOff(0),
	m_bSoundSupport(false)
{
}

CRenderGadget::~CRenderGadget()
{
	Destroy();
}

void CRenderGadget::UpdateMusicStatus(void)
{
	if ( m_pParent )
	{
		m_pParent->UpdateMusicStatus();
	}
}

void CRenderGadget::_OnActivation( void )
{
	m_bInLifetime = false;
	m_bCycleOn = true;
	m_tmrDelay.Reset();
	m_tmrLifetime.Reset();
	m_tmrCycleOn.Reset();
	m_tmrCycleOff.Reset();
	m_tmrDelay.Start(m_pParent->GetRenderTimestamp());
	OnActivation();
}

void CRenderGadget::_OnDeactivation( void )
{
	OnDeactivation();
}


/*
<LayoutElement Type=Image Source=Cache>
<SrcLoc Val=Title />
<Pos T=0 H=480 L=0 W=640/>
<DefColor ARGB=0xff000000 Point=Bottom/>
<DefColor ARGB=0xff0000ff Point=Top/>
</LayoutElement>
  */

void CRenderGadget::MakeRects( int iGadgetWidth, int iGadgetHeight )
{
	int iDestMaxWidth, iDestMaxHeight;	// Bounding Box
	int iDestDeltaX, iDestDeltaY;
	bool bBoundWidth, bBoundHeight;		// If no bounding, these will be false.
	int iDestWidth, iDestHeight;		// Unbounded dest box (adjusted in last box step)
	int iSrcWidth, iSrcHeight;

	// Establish "actuals"
	if ( m_iWidth == -54321 )
	{
		bBoundWidth = false;
		if ( iGadgetWidth < 1 )
		{
			iGadgetWidth = 1;
		}
	}
	else
	{
		bBoundWidth = true;
		iDestMaxWidth = m_iWidth;
		if ( iGadgetWidth < 1 )
		{
			iGadgetWidth = iDestMaxWidth;
		}
	}
	// Establish "actuals"
	if ( m_iHeight == -54321 )
	{
		bBoundHeight = false;
		if ( iGadgetHeight < 1 )
		{
			iGadgetHeight = 1;
		}
	}
	else
	{
		bBoundHeight = true;
		iDestMaxHeight = m_iHeight;
		if ( iGadgetHeight < 1 )
		{
			iGadgetHeight = iDestMaxHeight;
		}
	}

	// Current source box
	iSrcWidth = iGadgetWidth;
	iSrcHeight = iGadgetHeight;

	// Cannot scale X if we have no bounds!
	if ( m_bScaleX && !bBoundWidth )
	{
		m_bScaleX = false;
	}

	// Cannot scale Y if we have no bounds!
	if ( m_bScaleY && !bBoundHeight )
	{
		m_bScaleY = false;
	}

	// Sanity check. If we aren't scaling anything, remove proportional!
	if ( m_bProportional )
	{
		if ( !m_bScaleX && !m_bScaleY )
		{
			m_bProportional = false;
		}
	}

	if ( m_bProportional )
	{
		if ( m_bScaleX )
		{
			iDestWidth = iDestMaxWidth;
			iDestHeight = (iGadgetHeight*iDestWidth)/iGadgetWidth;
			if ( m_bScaleY )
			{
				// Need to squeeze it, if height is bounded
				if ( bBoundHeight && ( iDestHeight > iDestMaxHeight ) )
				{
					iDestHeight = iDestMaxHeight;
					iDestWidth = (iGadgetWidth*iDestHeight)/iGadgetHeight;					
				}
			}
		}
		else
		{
			// MUST be scaling Y only, since we check before getting into this condition!
			iDestHeight = iDestMaxHeight;
			iDestWidth = (iGadgetWidth*iDestHeight)/iGadgetHeight;					
		}
	}
	else
	{
		if ( m_bScaleX )
		{
			iDestWidth = iDestMaxWidth;
		}
		else
		{
			iDestWidth = iGadgetWidth;
		}
		if ( m_bScaleY )
		{
			iDestHeight = iDestMaxHeight;
		}
		else
		{
			iDestHeight = iGadgetHeight;
		}
	}

	// Now hit it with the bounding limits...
	if ( bBoundWidth )
	{
		// iDestWidth represents the destination, in pixels, that 
		// represents 100% of our scaled item
		if ( iDestWidth > iDestMaxWidth )
		{
			// In paring it down, we also need to trim the source width
			iSrcWidth = (iSrcWidth*iDestMaxWidth)/iDestWidth;
			iDestWidth = iDestMaxWidth;
		}
	}
	if ( bBoundHeight )
	{
		// iDestWidth represents the destination, in pixels, that 
		// represents 100% of our scaled item
		if ( iDestHeight > iDestMaxHeight )
		{
			// In paring it down, we also need to trim the source width
			iSrcHeight = (iSrcHeight*iDestMaxHeight)/iDestHeight;
			iDestHeight = iDestMaxHeight;
		}
	}
	iDestDeltaX = iDestMaxWidth-iDestWidth;
	iDestDeltaY = iDestMaxHeight-iDestHeight;
	switch( m_iJustifyX )
	{
		case 0: // Centered...
			if ( m_iOriginX != -54321 )
			{
				m_rectDest.left = m_iOriginX-(iDestWidth/2);
				m_rectDest.right = m_rectDest.left+(iDestWidth-1);
			}
			else
			{
				// Centered and scaled. Justify must be ignore inthis case,
				// Since it doesn't have an origin point
				// Use Left in this case as "left"
				m_rectDest.left = m_iLeft+(iDestDeltaX/2);
				m_rectDest.right = m_rectDest.left+(iDestWidth-1);
			}
			m_rectSrc.left = (iGadgetWidth-iSrcWidth)/2;
			m_rectSrc.right = m_rectSrc.left+(iSrcWidth-1);
			break;
		case 1: // Right
			if ( m_iOriginX != -54321 )
			{
				m_rectDest.left = m_iOriginX-iDestWidth;
				m_rectDest.right = m_rectDest.left+(iDestWidth-1);
			}
			else
			{
				// Since it doesn't have an origin point
				// Use Left in this case as "left"
				m_rectDest.left = m_iLeft;
				m_rectDest.right = m_rectDest.left+(iDestWidth-1);
			}
			m_rectSrc.right = iGadgetWidth-1;
			m_rectSrc.left = (iGadgetWidth-iSrcWidth);
			break;
		case -1: // Left
		default:
			if ( m_iOriginX != -54321 )
			{
				m_rectDest.left = m_iOriginX;
				m_rectDest.right = m_rectDest.left+(iDestWidth-1);
			}
			else
			{
				// Centered and scaled. Justify must be ignore inthis case,
				// Since it doesn't have an origin point
				// Use Left in this case as "left"
				m_rectDest.left = m_iLeft;
				m_rectDest.right = m_rectDest.left+(iDestWidth-1);
			}
			m_rectSrc.left = 0;
			m_rectSrc.right = iSrcWidth-1;
			break;
	}

	switch( m_iJustifyY )
	{
		case 0: // Centered...
			if ( m_iOriginY != -54321 )
			{
				m_rectDest.top = m_iOriginY-(iDestHeight/2);
				m_rectDest.bottom = m_rectDest.top+(iDestHeight-1);
			}
			else
			{
				// Centered and scaled. Justify must be ignore inthis case,
				// Since it doesn't have an origin point
				// Use Left in this case as "left"
				m_rectDest.top = m_iTop+(iDestDeltaY/2);
				m_rectDest.bottom = m_rectDest.top+(iDestHeight-1);
			}
			m_rectSrc.top = (iGadgetHeight-iSrcHeight)/2;
			m_rectSrc.bottom = m_rectSrc.top+(iSrcHeight-1);
			break;
		case 1: // Bottom
			if ( m_iOriginY != -54321 )
			{
				m_rectDest.top = m_iOriginY-iDestHeight;
				m_rectDest.bottom = m_rectDest.top+(iDestHeight-1);
			}
			else
			{
				// Since it doesn't have an origin point
				// Use Left in this case as "left"
				m_rectDest.top = m_iTop;
				m_rectDest.bottom = m_rectDest.top+(iDestHeight-1);
			}
			m_rectSrc.bottom = iGadgetHeight-1;
			m_rectSrc.top = (iGadgetHeight-iSrcHeight);
			break;
		case -1: // Top
		default:
			if ( m_iOriginY != -54321 )
			{
				m_rectDest.top = m_iOriginY;
				m_rectDest.bottom = m_rectDest.top+(iDestHeight-1);
			}
			else
			{
				// Centered and scaled. Justify must be ignore inthis case,
				// Since it doesn't have an origin point
				// Use Left in this case as "left"
				m_rectDest.top = m_iTop;
				m_rectDest.bottom = m_rectDest.top+(iDestHeight-1);
			}
			m_rectSrc.top = 0;
			m_rectSrc.bottom = iSrcHeight-1;
			break;
	}


#if 0
	int iDestWidth, iDestHeight;
	int iDestWidthTarg, iDestHeightTarg;
	bool bScaleX = m_bScaleX, bScaleY = m_bScaleY;


//	int iDestPropWidth, iDestPropHeight;

	// Establish "actuals"
	if ( m_iWidth == -54321 )
	{
		// Now set width, so use the object width
		if ( iGadgetWidth > 0 )
		{
			iDestWidth = iGadgetWidth;
		}
		else
		{
			iDestWidth = 1;
		}
	}
	else
	{
		iDestWidth = m_iWidth;
	}
	// At this point, iDestWidth == MAXIMUM Destination Width

	if ( m_iHeight == -54321 )
	{
		// No set width, so use the object width
		if ( iGadgetHeight > 0 )
		{
			iDestHeight = iGadgetHeight;
		}
		else
		{
			iDestHeight = 1;
		}
	}
	else
	{
		iDestHeight = m_iHeight;
	}
	// At this point, iDestHeight == MAXIMUM Destination Height

	// We need to figure out some more details, if proportional
	if ( m_bProportional )
	{
		// matters if either one or both of scaling on
		if ( bScaleX || bScaleY )
		{
			// If both, we can merely adjust both dest width and height
			if ( bScaleX && bScaleY )
			{
				// Everything MUST fit inside Dest w & h
			}
			else
			{
				// Implies scaling of source, but beyond dest, if need be
				if ( bScaleX )
				{
				}
				else
				{
				}
			}
		}
	}
	else
	{
	}


	// Work on horizontal measurements first...

	// 
	if ( bScaleX )
	{
		// No justify or offsets here...
		// We must have a width, too... if -1, we use Gadget width
		if ( iGadgetWidth > 0 )
		{
			m_rectSrc.left = 0;
			m_rectSrc.right = iGadgetWidth;
		}
		else
		{
			// Ugh... bad deal. Just set and forget, but this is bad news.
			m_rectSrc.left = 0;
			m_rectSrc.right = 1;
			iGadgetWidth = 1;
		}
	}
	else
	{
		// Non-scaling.
		// Our source rect is simple here...
		if ( m_iWidth != -54321 && iGadgetWidth > 0 )
		{
			if ( m_iWidth > iGadgetWidth )
			{
				iDestWidth = iGadgetWidth;
			}
		}

		m_rectSrc.left = m_iOffsetX;
		if ( m_iWidth > 0 )
		{
			m_rectSrc.right = m_iOffsetX+m_iWidth;
		}
		else
		{
			m_rectSrc.left = m_iOffsetX;
			m_rectSrc.right = m_iOffsetX+iGadgetWidth;
		}
		if ( iGadgetWidth < 1 ) 
		{
			iGadgetWidth = 1;
		}
	}
	switch( m_iJustifyX )
	{
		case 0: // Centered...
			if ( m_iOriginX != -54321 )
			{
				m_rectDest.left = m_iOriginX-(iDestWidth/2);
				m_rectDest.right = m_rectDest.left+iDestWidth;
			}
			else
			{
				// Centered and scaled. Justify must be ignore inthis case,
				// Since it doesn't have an origin point
				// Use Left in this case as "left"
				m_rectDest.left = m_iLeft;
				m_rectDest.right = m_rectDest.left+iDestWidth;
			}
			break;
		case 1: // Right
			if ( m_iOriginX != -54321 )
			{
				m_rectDest.left = m_iOriginX-iDestWidth;
				m_rectDest.right = m_rectDest.left+iDestWidth;
			}
			else
			{
				// Centered and scaled. Justify must be ignore inthis case,
				// Since it doesn't have an origin point
				// Use Left in this case as "left"
				m_rectDest.left = m_iLeft;
				m_rectDest.right = m_rectDest.left+iDestWidth;
			}
			break;
		case -1: // Left
			if ( m_iOriginX != -54321 )
			{
				m_rectDest.left = m_iOriginX;
				m_rectDest.right = m_rectDest.left+iDestWidth;
			}
			else
			{
				// Centered and scaled. Justify must be ignore inthis case,
				// Since it doesn't have an origin point
				// Use Left in this case as "left"
				m_rectDest.left = m_iLeft;
				m_rectDest.right = m_rectDest.left+iDestWidth;
			}
			break;
	}
	if ( !bScaleX )
	{
		// Correct values...
		// If not scaled, we need to make adjustments for overlaps
		// If we are offset, make that adjustment first....
		if ( m_rectSrc.left < 0 )
		{
			m_rectDest.left += m_rectSrc.left;
			m_rectSrc.left = 0;
		}
		if ( m_rectSrc.right > iGadgetWidth )
		{
			m_rectSrc.right = iGadgetWidth-1;
		}
		// Destination should be the width of the source now
		m_rectDest.right = m_rectDest.left+(m_rectSrc.right-m_rectSrc.left)+1;

		// Now we ned to check against iDestWidth...
		// 
		if ( iDestWidth < (m_rectDest.right-m_rectDest.left) )
		{
			// Need to shrink even more...
			m_rectSrc.right = (m_rectSrc.left+iDestWidth)-1;
			m_rectDest.right = m_rectDest.left+(m_rectSrc.right-m_rectSrc.left)+1;
		}
	}

	// Now work on vertical measurements
	if ( bScaleY )
	{
		// Now justify or offsets here...
		// We must have a width, too... if -1, we use Gadget width
		m_rectSrc.top = 0;
		if ( iGadgetHeight > 0 )
		{
			m_rectSrc.bottom = iGadgetHeight;
		}
		else
		{
			// Ugh... bad deal. Just set and forget, but this is bad news.
			m_rectSrc.bottom = 1;
			iGadgetWidth = 1;
		}
	}
	else
	{
		// Non-scaling.
		// Our source rect is simple here...
		if ( m_iHeight != -54321 && iGadgetHeight > 0 )
		{
			if ( m_iHeight > iGadgetHeight )
			{
				iDestHeight = iGadgetHeight;
			}
		}

		m_rectSrc.top = m_iOffsetY;
		if ( m_iHeight > 0 )
		{
			m_rectSrc.bottom = m_iOffsetY+m_iHeight;
		}
		else
		{
			m_rectSrc.bottom = m_iOffsetX+iGadgetHeight;
		}
		if ( iGadgetHeight < 1 ) 
		{
			iGadgetHeight = 1;
		}
	}
	switch( m_iJustifyY )
	{
		case 0: // Centered...
			if ( m_iOriginY != -54321 )
			{
				m_rectDest.top = m_iOriginY-(iDestHeight/2);
				m_rectDest.bottom = m_rectDest.top+iDestHeight;
			}
			else
			{
				// Centered and scaled. Justify must be ignore inthis case,
				// Since it doesn't have an origin point
				m_rectDest.top = m_iTop;
				m_rectDest.bottom = m_rectDest.top+iDestHeight;
			}
			break;
		case 1: // bottom
			if ( m_iOriginY != -54321 )
			{
				m_rectDest.top = m_iOriginY-iDestHeight;
				m_rectDest.bottom = m_rectDest.top+iDestHeight;
			}
			else
			{
				// Centered and scaled. Justify must be ignore inthis case,
				// Since it doesn't have an origin point
				m_rectDest.top = m_iTop;
				m_rectDest.bottom = m_rectDest.top+iDestHeight;
			}
			break;
		case -1: // top
			if ( m_iOriginY != -54321 )
			{
				m_rectDest.top = m_iOriginY;
				m_rectDest.bottom = m_rectDest.top+iDestHeight;
			}
			else
			{
				// Centered and scaled. Justify must be ignore inthis case,
				// Since it doesn't have an origin point
				m_rectDest.top = m_iTop;
				m_rectDest.bottom = m_rectDest.top+iDestHeight;
			}
			break;
	}
	if ( !bScaleY )
	{
		// Correct values...
		// If not scaled, we need to make adjustments for overlaps
		// If we are offset, make that adjustment first....
		if ( m_rectSrc.top < 0 )
		{
			m_rectDest.top += m_rectSrc.top;
			m_rectSrc.top = 0;
		}
		if ( m_rectSrc.bottom > iGadgetHeight )
		{
			m_rectSrc.bottom = iGadgetHeight-1;
		}
		// Destination should be the width of the source now
		m_rectDest.bottom = m_rectDest.top+(m_rectSrc.bottom-m_rectSrc.top)+1;

		// Now we ned to check against iDestHeight...
		// 
		if ( iDestHeight < (m_rectDest.bottom-m_rectDest.top) )
		{
			// Need to shrink even more...
			m_rectSrc.bottom = (m_rectSrc.top+iDestHeight)-1;
			m_rectDest.bottom = m_rectDest.top+(m_rectSrc.bottom-m_rectSrc.top)+1;
		}
	}
#endif
}


void CRenderGadget::Configure( CXMLNode * pNode )
{
	CXMLNode * pSubNode = NULL;
	int iX = -1, iY = -1;
	CStdString sJustifyX, sJustifyY, sCondition;

	sCondition = pNode->GetString( NULL, _T("condition"), _T("") );
	sCondition.MakeLower();

	if ( sCondition.GetLength() )
	{
		m_iConditionID = GetConditionID(sCondition);
	}
	else
	{
		m_iConditionID = -1;
	}

	if ( pNode->GetBool( _T("condition"), _T("not"), FALSE ) )
	{
		m_bNotCondition = true;
	}
	else
	{
		m_bNotCondition = false;
	}

	m_iOriginX = pNode->GetLong( _T("pos"), _T("x"), pNode->GetLong( _T("position"), _T("x"), -54321 ) );
	m_iOriginY = pNode->GetLong( _T("pos"), _T("y"), pNode->GetLong( _T("position"), _T("y"), -54321 ) );
	m_iTop = pNode->GetLong( _T("pos"), _T("t"), pNode->GetLong( _T("position"), _T("t"), -54321 ) );
	m_iLeft = pNode->GetLong( _T("pos"), _T("l"), pNode->GetLong( _T("position"), _T("l"), -54321 ) );
	m_iWidth = pNode->GetLong( _T("pos"), _T("w"), pNode->GetLong( _T("position"), _T("w"), -54321 ) );
	m_iHeight = pNode->GetLong( _T("pos"), _T("h"), pNode->GetLong( _T("position"), _T("h"), -54321 ) );

	sJustifyX = pNode->GetString( _T("justify"), _T("x"), _T("left") );
	sJustifyY = pNode->GetString( _T("justify"), _T("y"), _T("top") );
	sJustifyX.MakeLower();
	sJustifyY.MakeLower();

	if ( sJustifyX.GetLength() )
	{
		if ( sJustifyX.Compare( _T("center") ) == 0 )
		{
			m_iJustifyX = 0;	
		}
		else if ( sJustifyX.Compare( _T("right") ) == 0 )
		{
			m_iJustifyX = 1;
		}
		else
		{
			m_iJustifyX = -1;
		}
	}
	if ( sJustifyY.GetLength() )
	{
		if ( sJustifyY.Compare( _T("center") ) == 0 )
		{
			m_iJustifyY = 0;	
		}
		else if ( sJustifyY.Compare( _T("bottom") ) == 0 )
		{
			m_iJustifyY = 1;
		}
		else
		{
			m_iJustifyY = -1;
		}
	}

	m_bScaleX = (pNode->GetBool( _T("Scale"), _T("x"), pNode->GetBool( _T("Scaling"), _T("x"), FALSE ) ) == TRUE);
	m_bScaleY = (pNode->GetBool( _T("Scale"), _T("y"), pNode->GetBool( _T("Scaling"), _T("y"), FALSE ) ) == TRUE);
	m_bProportional = (pNode->GetBool( NULL, _T("proportional"), pNode->GetBool( NULL, _T("prop"), FALSE ) ) == TRUE);

	m_iOffsetX = pNode->GetLong( _T("Offset"), _T("x"), 0 );
	m_iOffsetY = pNode->GetLong( _T("Offset"), _T("y"), 0 );

	long lTimerValue;
	CStdString sTimerType;
	CXMLElement * pElement;
	int iIndex = 0;

	while( pElement = pNode->GetElement( _T("timer"), iIndex++ ) )
	{
		sTimerType = pElement->GetBasicValue( _T("type") );
		if ( _tcsicmp( sTimerType, _T("delay") ) == 0 )
		{
			lTimerValue = pElement->GetLong( 0 );
			m_tmrDelay.SetDelay( lTimerValue );
		}
		else if ( _tcsicmp( sTimerType, _T("lifetime") ) == 0 )
		{
			lTimerValue = pElement->GetLong( -1 );
			m_tmrLifetime.SetDelay( lTimerValue );
		}
		else if ( _tcsicmp( sTimerType, _T("cycleon") ) == 0 )
		{
			lTimerValue = pElement->GetLong( -1 );
			m_tmrCycleOn.SetDelay( lTimerValue );
		}
		else if ( _tcsicmp( sTimerType, _T("cycleoff") ) == 0 )
		{
			lTimerValue = pElement->GetLong( 0 );
			m_tmrCycleOff.SetDelay( lTimerValue );
		}

	}

	InternalConfigure( pNode );
}

void CRenderGadget::InternalConfigure( CXMLNode * pNode )
{
}

void CRenderGadget::InternalInitialize( void )
{
}

void CRenderGadget::InternalDestroy( void )
{
}

void CRenderGadget::InternalRender( void )
{
}





void	CRenderGadget::SetClear( DWORD dwARGB )
{
	m_dwClearAttribute = dwARGB;
}

void	CRenderGadget::Initialize( LPDIRECT3DDEVICE8 pDevice, int iWidth, int iHeight, int iOriginX, int iOriginY )
{
	if ( (!m_bInit) && pDevice)
	{
		if ( iWidth != -1 )
		{
			m_iWidth=iWidth;
		}
		if ( iHeight != -1 )
		{
			m_iHeight=iHeight;
		}
		if ( iOriginX != -1 )
		{
			m_iOriginX=iOriginX;
		}
		if ( iOriginY != -1 )
		{
			m_iOriginY=iOriginY;
		}
		m_pDevice = pDevice;
		m_pDevice->AddRef();

		InternalInitialize();
		m_bInit = true;
	}
}

void	CRenderGadget::Destroy( void )
{
	if ( m_pRenderTexture )
	{
		m_pRenderTexture->Release();
		m_pRenderTexture = NULL;
	}
	if ( m_bInit )
	{
		m_pDevice->Release();
		m_pDevice = NULL;
		InternalDestroy();
	}
	m_bInit = false;
}

LPDIRECT3DTEXTURE8	CRenderGadget::Render( LPDIRECT3DDEVICE8 pDevice, bool bRenderToTexture )
{
	LPDIRECT3DTEXTURE8 pReturn = NULL;
	bool bRenderIt = true;

	if ( !m_bInit && pDevice )
	{
		Initialize( pDevice );
	}
//	m_bInLifetime = false;
//	m_bCycleOn = true;

	if ( !m_bInLifetime )
	{
		if (m_tmrDelay.IsElapsed(m_pParent->GetRenderTimestamp()) )
		{
			DWORD dwTimeStamp = m_tmrDelay.GetEndTime();
			m_bInLifetime = true;
			m_tmrLifetime.Start(dwTimeStamp);
			m_bCycleOn = true;
			m_tmrCycleOn.Start(dwTimeStamp);
		}
		else
		{
			bRenderIt = false;
		}
	}

	if ( m_bInLifetime )
	{
		if ( m_tmrLifetime.IsElapsed(m_pParent->GetRenderTimestamp()) )
		{
			bRenderIt = false;
		}
		else
		{
			if ( m_bCycleOn )
			{
				if ( m_tmrCycleOn.IsElapsed(m_pParent->GetRenderTimestamp()) )
				{
					DWORD dwTimeStamp = m_tmrCycleOn.GetEndTime();
					m_bCycleOn = false;
					m_tmrCycleOff.Start(dwTimeStamp);
					bRenderIt = false;
				}
				else
				{
					bRenderIt = true;
				}
			}
			else
			{
				if ( m_tmrCycleOff.IsElapsed(m_pParent->GetRenderTimestamp()) )
				{
					DWORD dwTimeStamp = m_tmrCycleOff.GetEndTime();
					m_bCycleOn = true;
					m_tmrCycleOn.Start(dwTimeStamp);
					bRenderIt = true;
				}
				else
				{
					bRenderIt = false;
				}
			}
		}
	}

	if ( bRenderIt && (( m_iConditionID == -1 ) || IsConditionMet(m_iConditionID, m_bNotCondition )) )
	{
		if ( m_bInit && m_pDevice )
		{
			if ( bRenderToTexture )
			{
				if ( !m_pRenderTexture )
				{
					// Attempt to create cache texture object
					CImageSource imgTemp( m_pDevice ); 
					imgTemp.MakeBlank( 640, 480 );
					m_pRenderTexture = imgTemp.GetTexture();
					if ( m_pRenderTexture )
					{
						m_pRenderTexture->AddRef();
					}
				}
				if ( m_pRenderTexture )
				{
					// Change viewport to cache texture
					CXBEasel easel( m_pDevice, m_pRenderTexture);

					// Render
					InternalRender();

					// Return viewport here
					pReturn = m_pRenderTexture;
					pReturn->AddRef();
				}
			}
			else
			{
				InternalRender();
			}
		}
	}
	return pReturn;
}


class CLineGadget : public CRenderGadget
{
protected:
	int					m_iX1;
	int					m_iY1;
	int					m_iX2;
	int					m_iY2;
	int					m_iThickness;
	DWORD				m_dwColor;
	CColorModHandler	m_colorHandler;
	virtual void		InternalInitialize( void );
	virtual void		InternalDestroy( void );
	virtual void		InternalRender( void );
	virtual void		InternalConfigure( CXMLNode * pNode );
public:
	CLineGadget();
	~CLineGadget();

	virtual	void OnActivation( void );
	virtual	void OnDeactivation( void );
};

CLineGadget::CLineGadget() :
	m_iX1(-54321),
	m_iY1(-54321),
	m_iX2(-54321),
	m_iY2(-54321),
	m_iThickness(1),
	m_dwColor(0)
{
}

CLineGadget::~CLineGadget()
{
}

void CLineGadget::OnActivation( void )
{
}

void CLineGadget::OnDeactivation( void )
{
}

void CLineGadget::InternalInitialize( void )
{
}

void CLineGadget::InternalDestroy( void )
{
}

void CLineGadget::InternalRender( void )
{
	if ( ( m_iX1 != -54321 ) && 
		( m_iX2 != -54321 ) && 
		( m_iY1 != -54321 ) && 
		( m_iY2 != -54321 ) )
	{
		GetStateApp()->DrawLine( (FLOAT)m_iX1, (FLOAT)m_iY1, (FLOAT)m_iX2, (FLOAT)m_iY2, m_colorHandler.ModifyColor(m_dwColor) );
	}
}

void CLineGadget::InternalConfigure( CXMLNode * pNode )
{
	m_iX1 = pNode->GetLong( _T("pos"), _T("x1"), -54321 );
	m_iY1 = pNode->GetLong( _T("pos"), _T("y1"), -54321 );
	m_iX2 = pNode->GetLong( _T("pos"), _T("x2"), -54321 );
	m_iY2 = pNode->GetLong( _T("pos"), _T("y2"), -54321 );

	// Ooops, width support complicates things.
//	m_iThickness = pNode->GetLong( NULL, _T("thickness"), -54321 );
	m_dwColor = pNode->GetDWORD( NULL, _T("color"), 0 );
	// Look for modifier nodes...
	CXMLNode * pModifierNode;
	int iIndex = 0;

	while( pModifierNode = pNode->GetNode( _T("modifier"), iIndex++ ) )
	{
		m_colorHandler.Configure( pModifierNode, _T("color") );
	}

}


class CPolyGadget : public CRenderGadget
{
	DWORD				m_dwBorderColor;
	DWORD				m_dwColor;
	D3DXVECTOR4		*	m_pVectors;
	int					m_iNumVectors;
	CColorModHandler	m_colorHandlerBorder;
	CColorModHandler	m_colorHandler;
//	CModClrThrob		m_modClrThrobBorder;
//	CModClrThrob		m_modClrThrob;
//	CModClrFlicker		m_modClrFlkrBorder;
//	CModClrFlicker		m_modClrFlkr;
protected:
	virtual void		InternalInitialize( void );
	virtual void		InternalDestroy( void );
	virtual void		InternalRender( void );
	virtual void		InternalConfigure( CXMLNode * pNode );
public:
	CPolyGadget();
	~CPolyGadget();

	virtual	void OnActivation( void );
	virtual	void OnDeactivation( void );
};

CPolyGadget::CPolyGadget() :
	m_pVectors(NULL),
	m_dwBorderColor(0),
	m_iNumVectors(0),
	m_dwColor(0)
{
}

CPolyGadget::~CPolyGadget()
{
}

void CPolyGadget::OnActivation( void )
{
}

void CPolyGadget::OnDeactivation( void )
{
}

void CPolyGadget::InternalInitialize( void )
{
}

void CPolyGadget::InternalDestroy( void )
{
	delete [] m_pVectors;
}

void CPolyGadget::InternalRender( void )
{
	DWORD dwColor, dwBorderColor;

	//dwBorderColor = m_modClrThrobBorder.ModifyColor(m_dwBorderColor);
	//dwColor = m_modClrThrob.ModifyColor(m_dwColor);
	//dwBorderColor = m_modClrFlkrBorder.ModifyColor(dwBorderColor);
	//dwColor = m_modClrFlkr.ModifyColor(dwColor);
	dwBorderColor = m_colorHandlerBorder.ModifyColor(m_dwBorderColor);
	dwColor = m_colorHandler.ModifyColor(m_dwColor);

	if ( m_iNumVectors > 2 && m_pVectors )
	{
		m_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
		m_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
		m_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
		m_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
		m_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
		m_pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
		m_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		m_pDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
		m_pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		m_pDevice->SetVertexShader( D3DFVF_XYZRHW );

		if ( m_iNumVectors == 3 )
		{
			// Render the box
			m_pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwColor );
			m_pDevice->DrawVerticesUP( D3DPT_TRIANGLELIST, m_iNumVectors, m_pVectors, sizeof(D3DXVECTOR4) );
		}
		else if ( m_iNumVectors == 4 )
		{
			// Render the box
			m_pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwColor );
			m_pDevice->DrawVerticesUP( D3DPT_QUADLIST, m_iNumVectors, m_pVectors, sizeof(D3DXVECTOR4) );
		}
		else
		{
			// Render the box
			m_pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwColor );
			m_pDevice->DrawVerticesUP( D3DPT_POLYGON, m_iNumVectors, m_pVectors, sizeof(D3DXVECTOR4) );
		}

		// Render the lines
		m_pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwBorderColor );
		m_pDevice->DrawVerticesUP( D3DPT_LINELOOP, m_iNumVectors, m_pVectors, sizeof(D3DXVECTOR4) );
	}
}

void CPolyGadget::InternalConfigure( CXMLNode * pNode )
{
	CStdString sValue;
//	int iNumMembers;

	// Normalize box
	if ( m_iHeight == -54321 )
	{
		m_iHeight = 480;
	}
	if ( m_iTop == -54321 )
	{
		if ( m_iOriginY != -54321 )
		{
			switch ( m_iJustifyY )
			{
				case -1: // Top
					m_iTop = m_iOriginY;
					break;
				case 0: // Center
					m_iTop = (m_iOriginY-m_iHeight)/2;
					break;
				case 1: // Bottom
					m_iTop = (m_iOriginY-m_iHeight)-1;
					break;
			}
		}
		else
		{
			m_iTop = 0;
		}
	}
	if ( m_iWidth == -54321 )
	{
		m_iWidth = 480;
	}
	if ( m_iLeft == -54321 )
	{
		if ( m_iOriginX != -54321 )
		{
			switch ( m_iJustifyX )
			{
				case -1: // Left
					m_iLeft = m_iOriginX;
					break;
				case 0: // Center
					m_iLeft = (m_iOriginX-m_iWidth)/2;
					break;
				case 1: // Right
					m_iLeft = (m_iOriginX-m_iWidth)-1;
					break;
			}
		}
		else
		{
			m_iLeft = 0;
		}
	}

	m_dwBorderColor = pNode->GetDWORD( NULL, _T("bordercolor"), 0 );
	m_dwColor = pNode->GetDWORD( NULL, _T("color"), 0 );

	sValue = pNode->GetString( NULL, _T("type"), _T("") );
	sValue.MakeLower();
	if ( sValue.Compare( _T("box") ) == 0 )
	{
		// 4 sides
		m_pVectors = new D3DXVECTOR4[4];
		m_iNumVectors = 4;
		// We use the box as our coordinates here.
		m_pVectors[0] = D3DXVECTOR4( (FLOAT)m_iLeft, (FLOAT)m_iTop, 0, 0 );
		m_pVectors[1] = D3DXVECTOR4( (FLOAT)((m_iLeft+m_iWidth)-1), (FLOAT)m_iTop, 0, 0 );
		m_pVectors[2] = D3DXVECTOR4( (FLOAT)((m_iLeft+m_iWidth)-1), (FLOAT)((m_iTop+m_iHeight)-1), 0, 0 );
		m_pVectors[3] = D3DXVECTOR4( (FLOAT)m_iLeft, (FLOAT)((m_iTop+m_iHeight)-1), 0, 0 );

	}
	else if ( sValue.Compare( _T("triangle") ) == 0 )
	{
		// 3 sides
		m_pVectors = new D3DXVECTOR4[3];
		m_iNumVectors = 3;
		// Left, Right, Up, or Down?
		sValue = pNode->GetString( NULL, _T("direction"), _T("up") );
		sValue.MakeLower();
		if ( sValue.Compare( _T("left") ) == 0 )
		{
			m_pVectors[0] = D3DXVECTOR4( (FLOAT)m_iLeft, (FLOAT)(m_iTop+(m_iHeight/2)), 0, 0 );
			m_pVectors[1] = D3DXVECTOR4( (FLOAT)((m_iLeft+m_iWidth)-1), (FLOAT)m_iTop, 0, 0 );
			m_pVectors[2] = D3DXVECTOR4( (FLOAT)((m_iLeft+m_iWidth)-1), (FLOAT)((m_iTop+m_iHeight)-1), 0, 0 );
		}
		else if ( sValue.Compare( _T("right") ) == 0 )
		{
			m_pVectors[0] = D3DXVECTOR4( (FLOAT)((m_iLeft+m_iWidth)-1), (FLOAT)(m_iTop+(m_iHeight/2)), 0, 0 );
			m_pVectors[1] = D3DXVECTOR4( (FLOAT)m_iLeft, (FLOAT)((m_iTop+m_iHeight)-1), 0, 0 );
			m_pVectors[2] = D3DXVECTOR4( (FLOAT)m_iLeft, (FLOAT)m_iTop, 0, 0 );
		}
		else if ( sValue.Compare( _T("down") ) == 0 )
		{
			m_pVectors[0] = D3DXVECTOR4( (FLOAT)(m_iLeft+(m_iWidth/2)), (FLOAT)((m_iTop+m_iHeight)-1), 0, 0 );
			m_pVectors[1] = D3DXVECTOR4( (FLOAT)m_iLeft, (FLOAT)m_iTop, 0, 0 );
			m_pVectors[2] = D3DXVECTOR4( (FLOAT)((m_iLeft+m_iWidth)-1), (FLOAT)m_iTop, 0, 0 );
		}
		else // up
		{
			m_pVectors[0] = D3DXVECTOR4( (FLOAT)(m_iLeft+(m_iWidth/2)), (FLOAT)m_iTop, 0, 0 );
			m_pVectors[1] = D3DXVECTOR4( (FLOAT)((m_iLeft+m_iWidth)-1), (FLOAT)((m_iTop+m_iHeight)-1), 0, 0 );
			m_pVectors[2] = D3DXVECTOR4( (FLOAT)m_iLeft, (FLOAT)((m_iTop+m_iHeight)-1), 0, 0 );
		}
	}

	CXMLNode * pModifierNode;
	int iIndex = 0;

	while( pModifierNode = pNode->GetNode( _T("modifier"), iIndex++ ) )
	{
		m_colorHandler.Configure( pModifierNode, _T("color") );
		m_colorHandlerBorder.Configure( pModifierNode, _T("bordercolor") );
	}

	//CXMLNode * pModifierNodes = NULL;
	//int iModNodeCount = 0;
	//CStdString sModifierType, sTarget;

	//while( pModifierNodes = pNode->GetNode( _T("modifier"), iModNodeCount++ ) )
	//{
	//	sModifierType = pModifierNodes->GetString( NULL, _T("type"), _T("") );
	//	sModifierType.MakeLower();
	//	if ( sModifierType.Compare( _T("throb") ) == 0 )
	//	{
	//		sTarget = pModifierNodes->GetString( NULL, _T("target"), _T("") );
	//		sTarget.MakeLower();
	//		if ( sTarget.Compare( _T("color") ) == 0 )
	//		{
	//			m_modClrThrob.Configure(pModifierNodes);
	//			m_modClrThrob.SetEnable(true);
	//		}
	//		else if ( sTarget.Compare( _T("bordercolor") ) == 0 )
	//		{
	//			m_modClrThrobBorder.Configure(pModifierNodes);
	//			m_modClrThrobBorder.SetEnable(true);
	//		}
	//	}
	//	else if ( sModifierType.Compare( _T("flicker") ) == 0 )
	//	{
	//		sTarget = pModifierNodes->GetString( NULL, _T("target"), _T("") );
	//		sTarget.MakeLower();
	//		if ( sTarget.Compare( _T("color") ) == 0 )
	//		{
	//			m_modClrFlkr.Configure(pModifierNodes);
	//			m_modClrFlkr.SetEnable(true);
	//		}
	//		else if ( sTarget.Compare( _T("bordercolor") ) == 0 )
	//		{
	//			m_modClrFlkrBorder.Configure(pModifierNodes);
	//			m_modClrFlkrBorder.SetEnable(true);
	//		}
	//	}
	//}


}


CRenderGadget * CreateBasicGadget( CXMLNode * pNode, CStdString & sGadgetName )
{
	CRenderGadget * pGadget = NULL;
	CStdString sValue;

	sValue = pNode->GetString( NULL, _T("type"), _T("") );
	sValue.MakeLower();
	if ( sValue.GetLength() )
	{
		if ( sValue.Compare( _T("line") ) == 0 )
		{
			pGadget = new CLineGadget();
		}
		else if ( sValue.Compare( _T("box") ) == 0 )
		{
			pGadget = new CPolyGadget();
		}
		else if ( sValue.Compare( _T("triangle") ) == 0 )
		{
			pGadget = new CPolyGadget();
		}
		else if ( sValue.Compare( _T("polygon") ) == 0 )
		{
			pGadget = new CPolyGadget();
		}
		else if ( sValue.Compare( _T("arrow") ) == 0 )
		{
			pGadget = new CPolyGadget();
		}
	}
	if ( pGadget )
	{
		pGadget->Configure( pNode );
		pGadget->Initialize( GetStateApp()->Get3dDevice() );
	}
	return pGadget;
}




CGadgetLayout::CGadgetLayout()
{
	m_dwRenderTimestamp = GetTickCount();
	m_bFadeIn= true;
	m_bMusicEnable = true;
	m_bMusicEnableCfg = true;
	m_pDevice = NULL;
	m_bTalliedMusicEnable = m_bMusicTally = true;
}

CGadgetLayout::~CGadgetLayout()
{
}

void CGadgetLayout::SetMusicEnable( bool bMusicEnable )
{
	if ( this )
	{
		if ( m_bMusicEnable != bMusicEnable )
		{
			m_bMusicEnable = bMusicEnable;
			UpdateMusicStatus();
		}
	}
}


void CGadgetLayout::Configure( CXMLNode * pNode )
{
	CRenderGadget * pGadget;
	CXMLNode * pLayoutNode;
	CStdString sName;

	m_dwRenderTimestamp = GetTickCount();
	if ( m_pDevice == NULL )
	{
		m_pDevice = GetStateApp()->Get3dDevice();
	}
	if ( m_pDevice )
	{
		int iIndex = 0;

		while( pLayoutNode = pNode->GetNode( _T("LayoutElement"), iIndex++) )
		{
			// Create a simple gadget
			pGadget = CreateBasicGadget( pLayoutNode, sName );

			// If that didn't make it, try the application specific gadgets
			if ( !pGadget )
			{
				pGadget = CreateGadget( pLayoutNode, sName );
			}
			if ( pGadget )
			{
				m_rgl.push_back(pGadget);
				pGadget->SetParent( this );
			}
		}
		m_bMusicEnableCfg = pNode->GetBool( NULL, _T("MusicEnable"), TRUE )?true:false;
		m_bFadeIn = pNode->GetBool( NULL, _T("FadeIn"), TRUE )?true:false;;

	}
}

void CGadgetLayout::UpdateMusicStatus( void )
{
	bool bReturn = true;

	TRenderGadgetList::iterator iterGadget;
	CRenderGadget * pGadget;	
	
	if ( m_bMusicEnable && m_bMusicEnableCfg )
	{
		iterGadget = m_rgl.begin();
		while( iterGadget != m_rgl.end() )
		{
			pGadget = (*iterGadget)->GetPtr();
			if ( pGadget )
			{
				if ( pGadget->AllowMusic() == false )
				{
					bReturn = false;
				}
			}
			iterGadget++;
		}
	}
	else
	{
		bReturn = false;
	}

	//CXBoxStateApplication * pApp = GetStateApp();
	//if ( pApp )
	//{
	//	if ( bReturn == false )
	//	{
	//		// Pause Playback
	//		pApp->MusicPause();
	//	}
	//	else
	//	{
	//		// Resume Playback
	//		pApp->MusicResume();
	//	}
	//}
}

bool CGadgetLayout::IsSequenceEnded( void )
{
	bool bReturn = false;

	TRenderGadgetList::iterator iterGadget;
	CRenderGadget * pGadget;	
	
	iterGadget = m_rgl.begin();
	while( iterGadget != m_rgl.end() )
	{
		pGadget = (*iterGadget)->GetPtr();
		if ( pGadget )
		{
			if ( pGadget->SequenceDone() )
			{
				bReturn = true;
			}
		}
		iterGadget++;
	}
	return bReturn;
}

bool CGadgetLayout::AllowMusic( bool bForceCheck /* = false */ )
{
	TRenderGadgetList::iterator iterGadget;
	CRenderGadget * pGadget;

	if ( bForceCheck )
	{
		m_bMusicTally = true;

		iterGadget = m_rgl.begin();
		while( iterGadget != m_rgl.end() )
		{
			pGadget = (*iterGadget)->GetPtr();
			if ( pGadget )
			{
				if ( pGadget->AllowMusic() == false )
				{
					m_bMusicTally = false;
				}
			}
			iterGadget++;
		}
		if ( m_bMusicEnableCfg == false )
		{
			m_bMusicTally = false;
			m_bTalliedMusicEnable = false;
		}
	}
	return m_bTalliedMusicEnable;
}

DWORD CGadgetLayout::GetRenderTimestamp( void )
{
	DWORD dwReturn = GetTickCount();
	if ( this )
	{
		dwReturn = m_dwRenderTimestamp;
	}
	return dwReturn;
}

void CGadgetLayout::Render( void )
{
	TRenderGadgetList::iterator iterGadget;
	CRenderGadget * pGadget;

	m_dwRenderTimestamp = GetTickCount();

	m_bMusicTally = true;

	iterGadget = m_rgl.begin();
	while( iterGadget != m_rgl.end() )
	{
		pGadget = (*iterGadget)->GetPtr();
		if ( pGadget )
		{
			pGadget->Render(m_pDevice);
			if ( pGadget->AllowMusic() == false )
			{
				m_bMusicTally = false;
			}
			//if ( SUCCEEDED( GetBackBufferTexture( m_pDevice, &pTexture ) ))
			//{
			//	pTexture->Release();
			//}
			
		}
		iterGadget++;
	}
	// m_bMusicTally now tells us what the status is!
	if ( m_bMusicEnableCfg == false )
	{
		m_bMusicTally = false;
		m_bTalliedMusicEnable = false;
	}
	else
	{
		m_bTalliedMusicEnable = m_bMusicTally;
	}
}

void CGadgetLayout::OnActivation( void )
{
	TRenderGadgetList::iterator iterGadget;
	CRenderGadget * pGadget;	
	
	m_dwRenderTimestamp = GetTickCount();

	iterGadget = m_rgl.begin();
	while( iterGadget != m_rgl.end() )
	{
		pGadget = (*iterGadget)->GetPtr();
		if ( pGadget )
		{
			pGadget->_OnActivation();
		}
		iterGadget++;
	}
}

void CGadgetLayout::OnDeactivation( void )
{
	TRenderGadgetList::iterator iterGadget;
	CRenderGadget * pGadget;	
	
	m_dwRenderTimestamp = GetTickCount();

	iterGadget = m_rgl.begin();
	while( iterGadget != m_rgl.end() )
	{
		pGadget = (*iterGadget)->GetPtr();
		if ( pGadget )
		{
			pGadget->_OnDeactivation();
		}
		iterGadget++;
	}
}

void	CGadgetLayout::DeleteLayout( void )
{
	TRenderGadgetList::iterator iterGadget;
	CRenderGadget * pGadget;	
	
	iterGadget = m_rgl.begin();
	while( iterGadget != m_rgl.end() )
	{
		pGadget = (*iterGadget)->GetPtr();
		if ( pGadget )
		{
			pGadget->Destroy();
			delete pGadget;
		}
		iterGadget++;
	}
	m_rgl.clear();
}


CGadgetSkin::CGadgetSkin() :
	m_iSoundVolume(10)
{
	m_bSkinLoaded = false;
	m_iMasterRandomIndex = (-1);
}

CGadgetSkin::~CGadgetSkin()
{
}

void CGadgetSkin::SetSoundVolume( int iVolume )
{
	
}


void CGadgetSkin::DeleteSkin( void )
{
	m_iMasterRandomIndex = (-1);
	Information.m_msValues.clear();
	Information.m_sAuthor = _T("");
	Information.m_sCopyright = _T("");
	Information.m_sName = _T("");
	Information.m_sSkinVersion = _T("");
	Music.m_sDirectory = _T("");
	// Sounds.m_sndMenuDown
	SoundSources.m_sMenuDown = _T("");
	SoundSources.m_sMenuLaunch = _T("");
	SoundSources.m_sMenuUp = _T("");
	SoundSources.m_sSaverBounce = _T("");
	SoundSources.m_sTitleClose = _T("");
	SoundSources.m_sTitleOpen = _T("");
	m_glTitle.DeleteLayout();
	m_glMenu.DeleteLayout();
	m_glHelp.DeleteLayout();
	m_glGameHelp.DeleteLayout();
	m_glLoad.DeleteLayout();
	m_glSaver.DeleteLayout();
	m_glInfo01.DeleteLayout();
	m_glInfo02.DeleteLayout();
	m_glInfo03.DeleteLayout();

}

void	CGadgetSkin::Configure( CXMLNode * pNode )
{
	CXMLNode * pSkinNode;
	int iIndex = 0;
	CStdString sNodeName;
	CStdString sSkinDirectory;
	CStdString sTempVal;
	
	if ( m_bSkinLoaded )
	{
		DeleteSkin();
	}

	Information.m_sName = pNode->GetString( NULL, _T("name"), _T("") );

	DEBUG_FORMAT( _T("Configuring Skin: %s"), Information.m_sName.c_str() );

	m_iMasterRandomIndex = pNode->GetLong( _T("resourcecontrol"), _T("GlobalRandom"), -1 );
	if ( m_iMasterRandomIndex > 1 )
	{
		m_iMasterRandomIndex = (RandomNum()%(m_iMasterRandomIndex+1));
	}
	else
	{
		m_iMasterRandomIndex = (-1);
	}

	g_MenuInfo.m_iMasterRandomIndex = m_iMasterRandomIndex;

	Delays.m_lLoadingDelay = pNode->GetLong( _T("delays"), _T("Load"), 2000 );
	// Launch Delays not implemented yet!
	g_MenuInfo.m_lLoadDelay = Delays.m_lLoadingDelay;
	Delays.m_lTitleDelay = pNode->GetLong( _T("delays"), _T("Title"), 0 );
	g_MenuInfo.m_lTitleDelay = Delays.m_lTitleDelay;
	Delays.m_lSaverDelay = pNode->GetLong( _T("delays"), _T("Saver"), 300 )*1000;
	g_MenuInfo.m_lSaverDelay = Delays.m_lSaverDelay;
	sSkinDirectory = pNode->GetString( NULL, _T("directory"), _T("") );

	Information.m_sAuthor = pNode->GetString( _T("information"), _T("author"), _T("") );
	Information.m_sCopyright = pNode->GetString( _T("information"), _T("copyright"), _T("") );
	Information.m_sSkinVersion = pNode->GetString( _T("information"), _T("skinversion"), _T("") );

	GetStateApp()->MessageAttr.m_dwBoxColor = pNode->GetDWORD( _T("messagebox"), _T("boxcolor"), 0x80000000 );
	GetStateApp()->MessageAttr.m_dwBorderColor = pNode->GetDWORD( _T("messagebox"), _T("bordercolor"), 0xffffff00 );
	GetStateApp()->MessageAttr.m_dwTextColor = pNode->GetDWORD( _T("messagebox"), _T("textcolor"), 0xffffffff );
	GetStateApp()->MessageAttr.m_dwShadowColor = pNode->GetDWORD( _T("messagebox"), _T("shadowcolor"), 0xff000000 );
	GetStateApp()->MenuAttr.m_dwBoxColor = pNode->GetDWORD( _T("systemmenu"), _T("boxcolor"), 0xc0000000 );
	GetStateApp()->MenuAttr.m_dwBorderColor = pNode->GetDWORD( _T("systemmenu"), _T("bordercolor"), 0xffffff00 );
	GetStateApp()->MenuAttr.m_dwTextColor = pNode->GetDWORD( _T("systemmenu"), _T("textcolor"), 0xff909090 );
	GetStateApp()->MenuAttr.m_dwShadowColor = pNode->GetDWORD( _T("systemmenu"), _T("shadowcolor"), 0xff000000 );
	GetStateApp()->MenuAttr.m_dwSelectColor = pNode->GetDWORD( _T("systemmenu"), _T("selectcolor"), 0xffffffff );
	GetStateApp()->MenuAttr.m_dwValueColor = pNode->GetDWORD( _T("systemmenu"), _T("valuecolor"), 0xff000090 );
	GetStateApp()->MenuAttr.m_dwSelectValueColor = pNode->GetDWORD( _T("systemmenu"), _T("selectvaluecolor"), 0xffc0c0ff );

	if ( sSkinDirectory.GetLength() )
	{
		g_MenuInfo.m_sSkinPath = sSkinDirectory;
	}
	pSkinNode = pNode->GetNode(iIndex);
	while( pSkinNode )
	{
		sNodeName = pSkinNode->m_sName;
		sNodeName.MakeLower();
		if ( sNodeName.Compare( _T("resource") ) == 0 )
		{
			CStdString sType, sName;
			sType = pSkinNode->GetString( NULL, _T("type"), _T("") );
			sName = pSkinNode->GetString( NULL, _T("name"), _T("") );
			if ( sName.GetLength() )
			{
				DEBUG_FORMAT( _T("Skin Resource: %s / %s"), sType.c_str(), sName.c_str() );
				sName.MakeLower();
				sType.MakeLower();
				if ( sType.Compare( _T("image") ) == 0 )
				{
					g_ImgResCache.AddEntry(pSkinNode, g_MenuInfo.m_sSkinPath);
				}
				else if ( sType.Compare( _T("font") ) == 0 )
				{
					CStdString sFilename;
					sFilename = pSkinNode->GetString( NULL,_T("file"), _T("") );
					sFilename = MakeFullFilePath( g_MenuInfo.m_sSkinPath, sFilename );
					if ( FileExists( sFilename ) )
					{
						// This also adds font to cache
						g_FontCache.GetFontPtrByFilename( sFilename, sName );
					}
				}
				else if ( sType.Compare( _T("sound") ) == 0 )
				{
					CStdString sFilename;
					sFilename = pSkinNode->GetString( NULL,_T("file"), _T("") );
					if ( sFilename.GetLength() )
					{
						sFilename = MakeFullFilePath( sSkinDirectory, sFilename );
						if ( FileExists( sFilename ) )
						{
							// This also adds sound to cache
							g_SndResCache.AddEntry( sName, sFilename );
						}
					}
				}
			}
		}
		else if ( sNodeName.Compare( _T("layout") ) == 0 )
		{
			CStdString sScreenName;
			sScreenName = pSkinNode->GetString( NULL, _T("screen"), _T("") );
			sScreenName.MakeLower();
			if ( sScreenName.Compare( _T("title") ) == 0 )
			{
				DEBUG_FORMAT( _T("Skin LAYOUT: %s"), sScreenName.c_str() );
				m_glTitle.Configure( pSkinNode );
			}
			else if ( sScreenName.Compare( _T("menu") ) == 0 )
			{
				DEBUG_FORMAT( _T("Skin LAYOUT: %s"), sScreenName.c_str() );
				m_glMenu.Configure( pSkinNode );
			}
			else if ( sScreenName.Compare( _T("load") ) == 0 )
			{
				DEBUG_FORMAT( _T("Skin LAYOUT: %s"), sScreenName.c_str() );
				m_glLoad.Configure( pSkinNode );
			}
			else if ( sScreenName.Compare( _T("help") ) == 0 )
			{
				DEBUG_FORMAT( _T("Skin LAYOUT: %s"), sScreenName.c_str() );
				m_glHelp.Configure( pSkinNode );
			}
			else if ( sScreenName.Compare( _T("gamehelp") ) == 0 )
			{
				DEBUG_FORMAT( _T("Skin LAYOUT: %s"), sScreenName.c_str() );
				m_glGameHelp.Configure( pSkinNode );
			}
			else if ( sScreenName.Compare( _T("saver") ) == 0 )
			{
				DEBUG_FORMAT( _T("Skin LAYOUT: %s"), sScreenName.c_str() );
				m_glSaver.Configure( pSkinNode );
			}
			else if ( sScreenName.Compare( _T("info01") ) == 0 )
			{
				DEBUG_FORMAT( _T("Skin LAYOUT: %s"), sScreenName.c_str() );
				m_glInfo01.Configure( pSkinNode );
			}
			else if ( sScreenName.Compare( _T("info02") ) == 0 )
			{
				DEBUG_FORMAT( _T("Skin LAYOUT: %s"), sScreenName.c_str() );
				m_glInfo02.Configure( pSkinNode );
			}
			else if ( sScreenName.Compare( _T("info03") ) == 0 )
			{
				DEBUG_FORMAT( _T("Skin LAYOUT: %s"), sScreenName.c_str() );
				m_glInfo03.Configure( pSkinNode );
			}
		}
		else if ( sNodeName.Compare( _T("music") ) == 0 )
		{
			CXMLNode * pSubNode = NULL;
			CXMLElement * pElement = NULL;
			int iInnerIndex;

			// g_MenuInfo.m_bUseMusicManager = true;
			g_MenuInfo.m_bGlobalMusic = (pSkinNode->GetBool(NULL, _T("PlayGlobal"), FALSE)==TRUE);
			if ( g_MenuInfo.Overrides.m_bForceGlobalMusic )
			{
				g_MenuInfo.m_bGlobalMusic = true;
			}
			else if ( g_MenuInfo.Overrides.m_bForceLocalMusic )
			{
				g_MenuInfo.m_bGlobalMusic = false;
			}
			g_MenuInfo.m_bRandomMusic = (pSkinNode->GetBool(NULL, _T("PlayRandom"), TRUE)==TRUE);
			if ( g_MenuInfo.Overrides.m_bForceRandomMusic )
			{
				g_MenuInfo.m_bRandomMusic = true;
			}



			iInnerIndex = 0;
			pElement = pSkinNode->GetElement(iInnerIndex++);
			while( pElement )
			{
				if ( ( pElement->m_sName.Compare( _T("dir") ) == 0 ) ||
					 ( pElement->m_sName.Compare( _T("directory") ) == 0 ) )
				{
					// Add value into directory list
					g_MenuInfo.m_saMusicDirectories.push_back( MakeFullDirPath( sSkinDirectory, pElement->m_sValue ) );
					// g_MenuInfo.m_sMusicDirectory = MakeFullDirPath( sSkinDirectory, pElement->m_sValue );
				}
				else if ( ( pElement->m_sName.Compare( _T("file") ) == 0 ) ||
					 ( pElement->m_sName.Compare( _T("track") ) == 0 ) )
				{
					// Add value into directory list
					g_MenuInfo.m_saMusicFiles.push_back( MakeFullFilePath( sSkinDirectory, pElement->m_sValue ) );
				}
				pElement = pSkinNode->GetElement(iInnerIndex++);
			}
			iInnerIndex = 0;
			pSubNode = pSkinNode->GetNode( iInnerIndex++ );
			while( pSubNode )
			{
				if ( ( pSubNode->m_sName.Compare( _T("dir") ) == 0 ) ||
					 ( pSubNode->m_sName.Compare( _T("directory") ) == 0 ) )
				{
					// Add value into directory list
					sTempVal = pSubNode->GetString( NULL, _T("val"), _T("") );
					sTempVal = pSubNode->GetString( NULL, _T("value"), sTempVal );
                    if ( sTempVal.GetLength() )
					{
						g_MenuInfo.m_saMusicDirectories.push_back( MakeFullDirPath( sSkinDirectory, sTempVal ) );
						// g_MenuInfo.m_sMusicDirectory = MakeFullDirPath( sSkinDirectory, sTempVal );
					}
				}
				else if ( ( pSubNode->m_sName.Compare( _T("file") ) == 0 ) ||
					 ( pSubNode->m_sName.Compare( _T("track") ) == 0 ) )
				{
					// Add value into directory list
					sTempVal = pSubNode->GetString( NULL, _T("val"), _T("") );
					sTempVal = pSubNode->GetString( NULL, _T("value"), sTempVal );
                    if ( sTempVal.GetLength() )
					{
						g_MenuInfo.m_saMusicFiles.push_back( MakeFullFilePath( sSkinDirectory, sTempVal ) );
					}
				}
				pSubNode = pSkinNode->GetNode( iInnerIndex++ );
			}
		}
		iIndex++;
		pSkinNode = pNode->GetNode(iIndex);
	}
	m_bSkinLoaded = true;
}

CGadgetLayout * CGadgetSkin::GetLayout( LPCTSTR szLayout )
{
	CGadgetLayout * pReturn = NULL;
	CStdString sLayout(szLayout);

	sLayout.MakeLower();
	if ( sLayout.Compare(_T("title")) == 0 )
	{
		pReturn = &m_glTitle;
	}
	else if ( sLayout.Compare(_T("menu")) == 0 )
	{
		pReturn = &m_glMenu;
	}
	else if ( sLayout.Compare(_T("help")) == 0 )
	{
		pReturn = &m_glHelp;
	}
	else if ( sLayout.Compare(_T("gamehelp")) == 0 )
	{
		pReturn = &m_glGameHelp;
	}
	else if ( sLayout.Compare(_T("load")) == 0 )
	{
		pReturn = &m_glLoad;
	}
	else if ( sLayout.Compare(_T("saver")) == 0 )
	{
		pReturn = &m_glSaver;
	}
	else if ( sLayout.Compare(_T("info01")) == 0 )
	{
		pReturn = &m_glInfo01;
	}
	else if ( sLayout.Compare(_T("info02")) == 0 )
	{
		pReturn = &m_glInfo02;
	}
	else if ( sLayout.Compare(_T("info03")) == 0 )
	{
		pReturn = &m_glInfo03;
	}
	return pReturn;
}

CTextAttributes::CTextAttributes()
{
	m_bDoGlow = false;
	m_dwBackColor = 0;
	m_dwBorderColor = 0;
	m_dwShadowColor = 0;
	m_dwTextColor = 0xFFFFFFFF;
	m_iShadowSize = 2;
	m_iJustify = -1;
}

CTextAttributes::CTextAttributes( const CTextAttributes & src )
{
	m_bDoGlow = src.m_bDoGlow;
	m_dwBackColor = src.m_dwBackColor;
	m_dwBorderColor = src.m_dwBorderColor;
	m_dwShadowColor = src.m_dwShadowColor;
	m_dwTextColor = src.m_dwTextColor;
	m_iShadowSize = src.m_iShadowSize;
	m_iJustify = src.m_iJustify;
	m_colorHandlerText = src.m_colorHandlerText;
	m_colorHandlerShadow = src.m_colorHandlerShadow;
	m_colorHandlerBack = src.m_colorHandlerBack;
	m_colorHandlerBorder = src.m_colorHandlerBorder;
}

void CTextAttributes::Configure( CXMLNode * pNode )
{
	m_bDoGlow = (pNode->GetBool( NULL, _T("DoGlow"), m_bDoGlow )==TRUE);
	m_dwBackColor = pNode->GetDWORD( NULL, _T("BackColor"), m_dwBackColor);
	m_dwBorderColor = pNode->GetDWORD( NULL, _T("BorderColor"), m_dwBorderColor);
	m_dwShadowColor = pNode->GetDWORD( NULL, _T("ShadowColor"), m_dwShadowColor);
	m_dwTextColor = pNode->GetDWORD( NULL, _T("TextColor"), m_dwTextColor);
	m_iShadowSize = pNode->GetLong( NULL, _T("ShadowSize"), m_iShadowSize);

	CXMLNode * pModifierNode;
	int iIndex = 0;

	while( pModifierNode = pNode->GetNode( _T("modifier"), iIndex++ ) )
	{
		m_colorHandlerText.Configure( pModifierNode, _T("textcolor") );
		m_colorHandlerShadow.Configure( pModifierNode, _T("shadowcolor") );
		m_colorHandlerBack.Configure( pModifierNode, _T("backcolor") );
		m_colorHandlerBorder.Configure( pModifierNode, _T("bordercolor") );
	}
}

CTripTimer::CTripTimer( long lDelay, bool bStart ) :
	m_bStarted(false),
	m_bElapsed(false),
	m_lDelay(lDelay)
{
	if ( bStart )
	{
		Start();
	}
}

void CTripTimer::Reset( void )
{
	m_bStarted = false;
}

bool CTripTimer::IsElapsed( DWORD dwTimestamp )
{ 
	if ( dwTimestamp == 0 )
	{
		dwTimestamp = GetTickCount();
	}
	if ( !m_bStarted )
	{
		m_bElapsed = false;
	}
	else
	{
		if ( !m_bElapsed )
		{
			// Never expires if -1
			if ( m_lDelay >= 0 )
			{
				DWORD dwNewTime = dwTimestamp;

				if ( (m_dwTimeStamp+(DWORD)m_lDelay) < dwNewTime )
				{
					m_bElapsed = true;
				}
			}
		}
	}
	return m_bElapsed;
}

void CTripTimer::Start( DWORD dwTimeStamp )
{
	if ( dwTimeStamp == 0 )
	{
		m_dwTimeStamp = GetTickCount();
	}
	else
	{
		m_dwTimeStamp = dwTimeStamp;
	}
	if ( m_lDelay == 0 )
	{
		m_bElapsed = true;
	}
	else
	{
		m_bElapsed = false;
	}
	m_bStarted = true;
}

void CTripTimer::SetDelay( long lDelay )
{
	if( lDelay == 0 )
	{
		m_bElapsed = true;
	}
	m_lDelay = lDelay;
}
