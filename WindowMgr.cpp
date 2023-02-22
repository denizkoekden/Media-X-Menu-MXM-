// WindowMgr.cpp


#include <xtl.h>
#include "MXM.h"
#include "WindowMgr.h"





void CXBWndCtrl::SetName( LPCTSTR szName )
{
	m_sName = szName;
}

void	CXBWndCtrl::SetText( LPCTSTR szText )
{
	m_sText = szText;
}

void	CXBWndCtrl::SetPosition( int iX, int iY, int iW /*=-1*/, int iH /*=-1*/ )
{
	if ( iY >= 0 )
	{
		m_rcWindowBox.top = iY;
	}
	if ( iH >= 0 )
	{
		m_rcWindowBox.bottom = (iY+iH)-1;
	}
	if ( iX >= 0 )
	{
		m_rcWindowBox.left = iX;
	}
	if ( iW >= 0 )
	{
		m_rcWindowBox.right = (iX+iW)-1;
	}
}

void	CXBWndCtrl::SetDefault( bool bDefault )
{
	m_bDefault = bDefault;
}

void	CXBWndCtrl::SetID( int iID )
{
	m_iID = iID;
}

void	CXBWndCtrl::SetFont( CXBFont * pFont )
{
	m_pCtrlFont = pFont;
}

void	CXBWndCtrl::Draw( RECT * pClientRect )
{
}

CStdString	CXBWndCtrl::GetName( void )
{
	return m_sName;
}

CStdString	CXBWndCtrl::GetText( void )
{
	return m_sText;
}

void	CXBWndCtrl::GetPosition( RECT *pRect )
{
	if ( pRect )
	{
		pRect->top = m_rcWindowBox.top;
		pRect->bottom = m_rcWindowBox.bottom;
		pRect->left = m_rcWindowBox.left;
		pRect->right = m_rcWindowBox.right;
	}
}

bool	CXBWndCtrl::GetDefault( void )
{
	return m_bDefault;
}

int		CXBWndCtrl::GetID( void )
{
	return m_iID;
}

CXBFont *CXBWndCtrl::GetFont( void )
{
	return m_pCtrlFont;
}



bool CXBWindow::InternalHandleInput( void )
{
	bool bReturn = true;

	return bReturn;
}

bool CXBWindow::InternalDraw( void )
{
	bool bReturn = true;

	return bReturn;
}

bool CXBWindow::DrawWindowBox( void )
{
	bool bReturn = true;

	return bReturn;
}

void CXBWindow::SetWindowSize( int iX, int iY )
{
}

void CXBWindow::SetWindowPosition( int iX, int iY )
{
}

CXBWindow::CXBWindow( void )
{
}

CXBWindow::~CXBWindow( void )
{
}

void CXBWindow::SetEventHandler( DWORD dwCtx, CXBWindowEvents * pHandler /*= NULL*/ )
{
}

void CXBWindow::Draw( void )
{
}

void CXBWindow::HandleInput( void )
{
}

bool CXBWindow::HandleMessage( void )
{
	return true;
}

void CXBWindow::CloseWindow( void )
{
}

void CXBWindow::Display( void )
{
}



void CXBMessageBox::SetMessage( LPCTSTR szMessage )
{
	if ( szMessage )
	{
		m_sMessageText = szMessage;
	}
	else
	{
	}
		CloseWindow();
}

