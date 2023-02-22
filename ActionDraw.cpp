

#include "StdAfx.h"
#include "ActionDraw.h"
#include "XBStateApp.h"




CActionDraw::CActionDraw(int iType) : m_iType(iType)
{
}

CActionDraw::CActionDraw(const CActionDraw &src )
{ 
	m_iType = src.m_iType;
}

CActionDraw::~CActionDraw()
{
}

CActionDraw * CActionDraw::GetPtr( void )
{
	return this;
}

void ClearActionDrawList( TActionDrawList & actionDrawList )
{
	EnterCriticalSection( &g_RenderCriticalSection );

    TActionDrawList::iterator iterDrawList;   

    iterDrawList = actionDrawList.begin();
    while( iterDrawList != actionDrawList.end() )
    {
        delete (*iterDrawList)->GetPtr();
        iterDrawList++;
    }
    actionDrawList.clear();
	LeaveCriticalSection( &g_RenderCriticalSection );
}


void DrawActionDrawList( TActionDrawList & actionDrawList )
{
    TActionDrawList::iterator iterDrawList;   

    iterDrawList = actionDrawList.begin();
	if ( actionDrawList.size() )
	{
		while( iterDrawList != actionDrawList.end() )
		{
			(*iterDrawList)->Draw();
			iterDrawList++;
		}
	}
}

void AddActionDrawList( TActionDrawList & actionDrawList, CActionDraw * pActionDraw, bool bCritical )
{

	if ( bCritical )
	{
		EnterCriticalSection( &g_RenderCriticalSection );
	}
	actionDrawList.push_back( pActionDraw );
	if ( bCritical )
	{
		LeaveCriticalSection( &g_RenderCriticalSection );
	}
}

CADImage::CADImage( void ) :
       CActionDraw( CADTYPE_IMAGE )
       ,m_pTexture(NULL)
       ,m_bTextureAttached(false)
       ,m_pImageSource(NULL)
       ,m_iX(0)
       ,m_iY(0)
       ,m_iW(0)
       ,m_iH(0)
{
}

void CADImage::SetImage( LPDIRECT3DTEXTURE8  pTexture, bool bAttach )
{
	ClearResources();
	m_pTexture = pTexture;
	if ( m_pTexture && bAttach )
	{
		m_pTexture->AddRef();
		m_bTextureAttached = true;
	}
	else
	{
		m_bTextureAttached = false;
	}
}


void CADImage::ClearResources( void )
{
	if ( m_pTexture )
	{
		if ( m_bTextureAttached )
		{
			m_pTexture->Release();
		}
		m_pTexture = NULL;
	}
	m_bTextureAttached = false;
	if ( m_pImageSource )
	{
		if ( m_bImageAttached )
		{
			delete m_pImageSource;
		}
		m_pImageSource = NULL;
	}
	m_bImageAttached = false;
}

void CADImage::SetImage( CImageSource * pImageSource )
{
	ClearResources();
	m_pImageSource = pImageSource;
//	if ( m_pImageSource && bAttach )
//	{
//		m_bImageAttached = true;
//	}
//	else
//	{
		m_bImageAttached = false;
//	}
}

CADImage::~CADImage()
{
	ClearResources();
}

CADImage::CADImage( const CADImage & src ) :
       CActionDraw( CADTYPE_IMAGE )
{
	if ( src.m_pTexture )
	{
		m_pTexture = src.m_pTexture;
		m_bTextureAttached = src.m_bTextureAttached;
		if ( m_bTextureAttached )
		{
			m_pTexture->AddRef();
		}
		m_pImageSource = NULL;
		m_bImageAttached = false;
	}
	else if ( src.m_pImageSource )
	{
		m_pImageSource = src.m_pImageSource;
		m_bImageAttached = false; // src.m_bImageAttached;
		m_pTexture = NULL;
		m_bTextureAttached = false;
	}
	m_iX = src.m_iX;
	m_iY = src.m_iY;
	m_iW = src.m_iW;
	m_iH = src.m_iH;
}

void CADImage::SetPos( int iX, int iY, int iW, int iH )
{
	m_iX = iX;
	m_iY = iY;
	m_iW = iW;
	m_iH = iH;
}

void CADImage::Draw( void )
{
	LPDIRECT3DTEXTURE8 pTexture = m_pTexture;

	if ( !pTexture && m_pImageSource )
	{
		pTexture = m_pImageSource->GetTexture();
	}
	if ( pTexture )
	{
		// Draw it...
		DrawSprite( GetStateApp()->Get3dDevice(), pTexture, m_iX, m_iY, m_iW, m_iH, TRUE, TRUE );
	}
}

CADMsgBox::CADMsgBox( void ) :
	CActionDraw(CADTYPE_MSGBOX)
{
}

CADMsgBox::CADMsgBox( const CADMsgBox & src ) :
	CActionDraw(CADTYPE_MSGBOX)
{
	m_sMessage = src.m_sMessage;
}

void CADMsgBox::Draw( void )
{
	GetStateApp()->DrawMessageBox( m_sMessage );
}

void CADMsgBox::SetMessage( LPCTSTR szMessage )
{
	m_sMessage = szMessage;
}

CADBox::CADBox( void ) :
	CActionDraw(CADTYPE_BOX)
{
}

CADBox::CADBox( const CADBox & src ) :
	CActionDraw(CADTYPE_BOX)
{
}

void CADBox::SetPos( int iX, int iY, int iW, int iH )
{
	m_iX = iX;
	m_iY = iY;
	m_iW = iW;
	m_iH = iH;
}

void CADBox::SetColor( DWORD dwColor, DWORD dwBorder  )
{
	m_dwColor = dwColor;
	m_dwBorderColor = dwBorder;
}

void CADBox::Draw( void )
{
	FLOAT fX, fY, fX2, fY2;

	fX = (FLOAT)m_iX;
	fX += 0.5;
	fX2 = (FLOAT)m_iX+m_iW;
	fX2 += 0.5;
	fY = (FLOAT)m_iY;
	fY += 0.5;
	fY2 = (FLOAT)m_iY+m_iH;
	fY2 += 0.5;

	if ( m_dwColor == 0 )
	{
		m_dwColor = GetStateApp()->MessageAttr.m_dwBoxColor;
	}
	if ( m_dwBorderColor == 0 )
	{
		m_dwBorderColor = GetStateApp()->MessageAttr.m_dwBorderColor;
	}

	// Render to current drawing screen....
	DrawBox( GetStateApp()->Get3dDevice(), fX, fY, fX2, fY2, m_dwColor, m_dwBorderColor );
}

CADText::CADText( void ) :
	CActionDraw(CADTYPE_TEXT)
{
}

CADText::CADText( const CADText & src ) :
	CActionDraw(CADTYPE_TEXT)
{
}

void CADText::SetPos( int iX, int iY, DWORD dwJustify )
{
	m_iX = iX;
	m_iY = iY;
	m_dwJustify = dwJustify;
}

void CADText::SetColor( DWORD dwColor )
{
	m_dwColor = dwColor;
}

void CADText::SetText( LPCTSTR szMessage )
{
	m_sMessage = szMessage;
}

void CADText::Draw( void )
{
	FLOAT fX, fY;

	fX = (FLOAT)m_iX;
	fX += 0.5;
	fY = (FLOAT)m_iY;
	fY += 0.5;

	if ( m_dwColor == 0 )
	{
		m_dwColor = GetStateApp()->MessageAttr.m_dwTextColor;
	}
	// Render to current drawing screen....
	GetStateApp()->m_Font16.DrawText( fX, fY, m_dwColor, m_sMessage, m_dwJustify );
}




