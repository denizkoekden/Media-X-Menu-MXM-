//-----------------------------------------------------------------------------
// File: AppMenu.h
//
// Desc: 
//
// Hist: 
//      
//
// 
//-----------------------------------------------------------------------------

#ifndef __APPMENU_H__
#define __APPMENU_H__

#include <xtl.h>
#include "XBFont_mxm.h"


#define AM_MAXITEMS		20

class CAppMenu;

class CAppMenuState;



// HRESULT ValueCallback( PVOID pThis, long lMenuID, long lItemID, long &lValue, bool bSaveValue );
struct _TAppMenuItem
{
	LPCTSTR	m_szTitle;
	int		m_iType;
	long	m_lID;
	PVOID	m_pParamPtr;
	int		m_iParamArg;
	long	m_lMin;
	long	m_lMax;	
};


class CAppMenuHandler
{
public:
	virtual long GetValue( long lMenuId, long lItemID ) = 0;
	virtual long GetMinValue( long lMenuId, long lItemID ) = 0;
	virtual long GetMaxValue( long lMenuId, long lItemID ) = 0;
	virtual void SetValue( long lMenuId, long lItemID, long lValue ) = 0;
	virtual HRESULT SelectItem( long lMenuID, long lItemID, long lValue, _TAppMenuItem * pItem ) = 0;
	virtual HRESULT PrepareData( long lMenuID, long lItemID, PVOID *pDataPtr ) = 0;
	virtual HRESULT FrameMove( long lMenuID, long lItemID ) = 0;
	virtual HRESULT Render( long lMenuID, long lItemID ) = 0;
};

class CAppMenu
{
	CAppMenuState *		m_pCurrentState; 
	CAppMenuHandler *	m_pHandler;
	int					m_lCurrentMenuID;
	void				PrepState( void );
	void				PrepItem( _TAppMenuItem  * pItem );
	struct
	{
		DWORD m_dwBoxColor;
		DWORD m_dwBorderColor;
		DWORD m_dwTextColor;
		DWORD m_dwShadowColor;
		DWORD m_dwSelectColor;
		DWORD m_dwValueColor;
		DWORD m_dwSelectValueColor;
	} MenuAttr;
public:
	void				CalcExtents( CAppMenuState * pCurrentState );
	static CXBFont		* s_pMenuFont;
	static CXBFont		* s_pWidgetFont;
	CAppMenu( CAppMenuHandler * pHandler, _TAppMenuItem * pItemList, long lMenuID );
	~CAppMenu();
	virtual HRESULT	Render();
	virtual BOOL	MenuFrameMove();
//	virtual HRESULT SetValueCallback( 
	enum {	BooleanSetYesNo, BooleanSetOnOff, BooleanSetTrueFalse, EndSet };
};

class CAppMenuState
{
public:
	CAppMenuState(CAppMenu * pMenu, CAppMenuState * pPrevState, _TAppMenuItem * pItemList, long lMenuID );
	~CAppMenuState();

	CAppMenu * m_pMenu;
	_TAppMenuItem * m_pItemList;

	CAppMenuState *	m_pPrevState;
	int				m_iCurrPos;
	int				m_iNumItems;
	int				m_iaCurrValue[AM_MAXITEMS];

	int				m_iMenuWidth;
	int				m_iMenuHeight;

	int				m_iZeroWidth;

	int				m_iMaxItemHeight;
	long			m_lMenuID;
};


#define	AMIT_SELECTION	0
#define	AMIT_RETURN		1
#define	AMIT_BOOLEAN	2
#define	AMIT_VALUE		3
#define	AMIT_STRINGLIST	4
#define	AMIT_SUBMENU	5


/*
AMIT_SELECTION
  Does little except call app event handler
AMIT_RETURN
  Backs up one level, or exits top level
AMIT_BOOLEAN
  Select yes or no, on or off, true or false
AMIT_VALUE
  Integer value from min to max, with visual slider
AMIT_STRINGLIST
  Select one of several strings from a list
AMIT_SUBMENU
  Selects a new submenu to call up

Up/Down selects item in menu
Left/Right will change choice in selection (such as a value)
"A" or "Start" will select the item
"Back" will back out to the previous menu


  App sequence:

  LaunchMenu() will create a new menu object and launch it, attaching the previous
  to the new object.

  While a menu is active, FrameMove is bypassed, but render is done beneath the 
  menu screen.

  Default rendering displays a menu box using the Font16
  Element colors:
   BoxColor
   BorderColor
   UnSel Item Color
   Sel Item Color
   Slider Color

   ....:::::::|||||||| Slider is a triangular indicator with a line to indicate pos.




*/

#endif