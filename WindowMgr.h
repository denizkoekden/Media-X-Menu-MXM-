// WindowMgr.h


#ifndef __WINDOWMGR_H__
#define __WINDOWMGR_H__

#include "StdString.h"


// Handle:
// Gamepad Input
// Keyboard Input
// IR Input
// Mouse
// Provide generic translation of input to function
class CXBGenericInput
{

public:
	CXBGenericInput();
	~CXBGenericInput();
};

class CXBWindowEvents
{
public:
//	CXBWindowEvents();	{};
//	~CXBWindowEvents();	{};
	virtual bool OnClose( DWORD dwCtx, int DialogID );
	virtual bool OnUserMessage( DWORD dwCtx, int DialogID, PVOID pParam );
	virtual bool OnHandleInput( DWORD dwCtx, int DialogID, int iInputType, int iInputIndex, DWORD dwInput );
};

class CXBWndCtrl
{
protected:
	RECT		m_rcWindowBox;
	CStdString	m_sName;
	CStdString	m_sText;
	bool		m_bDefault;
	int			m_iID;
	CXBFont	*	m_pCtrlFont;
public:
	void	SetName( LPCTSTR szName );
	void	SetText( LPCTSTR szText );
	void	SetPosition( int iX, int iY, int iW=-1, int iH=-1 );
	void	SetDefault( bool bDefault );
	void	SetID( int iID );
	void	SetFont( CXBFont * pFont );


	virtual void	Draw( RECT * pClientRect );
//	virtual void	HandleInput( 
	
	CStdString	GetName( void );
	CStdString	GetText( void );
	void	GetPosition( RECT *pRect );
	bool	GetDefault( void );
	int		GetID( void );
	CXBFont *GetFont( void );
};



class CXBWindow
{	

protected:
	CXBWindowEvents *		m_pHandler;
	RECT					m_rcWindowBox;

	virtual bool	InternalHandleInput( void );
	virtual bool	InternalDraw( void );
	virtual bool	DrawWindowBox( void );
	void			SetWindowSize( int iX, int iY );
	void			SetWindowPosition( int iX, int iY );
public:
	CXBWindow( void );
	~CXBWindow( void );
	void SetEventHandler( DWORD dwCtx, CXBWindowEvents * pHandler = NULL );
	void Draw( void );
	void HandleInput( void );
	bool HandleMessage( void );
	void CloseWindow( void );
	void Display( void );
};

class CXBMessageBox : public CXBWindow
{
	CStdString	m_sMessageText;
	bool		m_bRebootAfter;
	bool		m_bDismissable;
	
public:
	void SetMessage( LPCTSTR szMessage );
	void CanDismiss( bool bDismissable ) { m_bDismissable = bDismissable; };
	void RebootAfter( bool bRebootAfter ) { m_bRebootAfter = bRebootAfter; };
};










#endif //  __WINDOWMGR_H__