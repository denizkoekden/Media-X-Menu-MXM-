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

#include <xtl.h>
#include "AppMenu.h"
#include "StdString.h"
#define DEBUG_KEYBOARD
#include <xkbd.h>

#include "XBStateApp.h"


_TAppMenuItem g_SampleMainMenu[] =
{
  {    "Test Option", AMIT_SELECTION, 1, NULL, 0, 0, 0 },
  {    "Test YesNo", AMIT_BOOLEAN, 2, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {    "Return", AMIT_RETURN, -1, NULL, 0, 0, 0 },  
  {   NULL, 0, 0, NULL, CAppMenu::EndSet, 0, 0 }
};

CXBFont	* CAppMenu::s_pMenuFont = NULL;
CXBFont	* CAppMenu::s_pWidgetFont = NULL;


CAppMenu::CAppMenu( CAppMenuHandler * pHandler, _TAppMenuItem * pItemList, long lMenuID ) :
	m_pHandler(pHandler)
{
//	DEBUG_FORMAT( _T("CAppMenu::CAppMenu") );
	m_pCurrentState = new CAppMenuState(this, NULL, pItemList, lMenuID );
	
	PrepState();

	GetStateApp()->SetDelays( 400, 200 );
	MenuAttr.m_dwBoxColor = GetStateApp()->MenuAttr.m_dwBoxColor;
	MenuAttr.m_dwBorderColor = GetStateApp()->MenuAttr.m_dwBorderColor;
	MenuAttr.m_dwTextColor = GetStateApp()->MenuAttr.m_dwTextColor;
	MenuAttr.m_dwShadowColor = GetStateApp()->MenuAttr.m_dwShadowColor;
	MenuAttr.m_dwSelectColor = GetStateApp()->MenuAttr.m_dwSelectColor;
	MenuAttr.m_dwValueColor = GetStateApp()->MenuAttr.m_dwValueColor;
	MenuAttr.m_dwSelectValueColor = GetStateApp()->MenuAttr.m_dwSelectValueColor;
}


void CAppMenu::PrepItem( _TAppMenuItem  * pItem )
{
	int iIndex;

	if ( pItem->m_pParamPtr == NULL )
	{
		m_pHandler->PrepareData( m_lCurrentMenuID, pItem->m_lID, &(pItem->m_pParamPtr) );
	}
	if ( pItem->m_lMin == pItem->m_lMax )
	{
		switch( pItem->m_iType )
		{
			case AMIT_SELECTION:
			case AMIT_RETURN:
			case AMIT_SUBMENU:
				break;
			case AMIT_BOOLEAN:
				pItem->m_lMin = 0;
				pItem->m_lMax = 1;
				break;
			case AMIT_VALUE:
				pItem->m_lMin = m_pHandler->GetMinValue( m_lCurrentMenuID, pItem->m_lID );
				pItem->m_lMax = m_pHandler->GetMaxValue( m_lCurrentMenuID, pItem->m_lID );
				break;
			case AMIT_STRINGLIST:
				pItem->m_lMin = 0;
				pItem->m_lMax = 0;
				if ( pItem->m_pParamPtr )
				{
					LPCTSTR *strsList = (LPCTSTR *)pItem->m_pParamPtr;
					for( iIndex=0; strsList[iIndex]; iIndex++ );
					if ( iIndex )
					{
						pItem->m_lMax = (long)iIndex-1;
					}
				}
				break;
		}
	}
}

void CAppMenu::PrepState( void )
{
	int iIndex;

	for( iIndex=0; iIndex<m_pCurrentState->m_iNumItems; iIndex++ )
	{
		m_pCurrentState->m_iaCurrValue[iIndex] = m_pHandler->GetValue( m_pCurrentState->m_lMenuID, m_pCurrentState->m_pItemList[iIndex].m_lID );
		PrepItem( &(m_pCurrentState->m_pItemList[iIndex]) );
	}
}

CAppMenu::~CAppMenu()
{
	CAppMenuState *		pState;
	
//	DEBUG_FORMAT( _T("CAppMenu::~CAppMenu") );
	while( m_pCurrentState )
	{
		pState = m_pCurrentState;
		m_pCurrentState = pState->m_pPrevState;
		delete pState;
	}
}

void CAppMenu::CalcExtents(CAppMenuState * pCurrentState)
{
	FLOAT fWidth, fHeight;
	int iMaxItemWidth, iMaxSelectionWidth;

	iMaxItemWidth = 0;
	iMaxSelectionWidth = 0;

//	DEBUG_FORMAT( _T("CAppMenu::CalcExtents() CurrentState=0x%08x"), pCurrentState );

	if ( pCurrentState )
	{
		pCurrentState->m_iZeroWidth = 0;

		if ( s_pMenuFont )
		{
//			DEBUG_FORMAT( _T("CAppMenu::CalcExtents() Calculating ZeroWidth") );

			int iIndex;
			CStdString sNum;

			for( iIndex=0; iIndex<10; iIndex++ )
			{
				sNum.Format( _T("%d"), iIndex );
				fWidth = 0;
				fHeight = 0;
				s_pMenuFont->GetTextExtent(sNum, &fWidth, &fHeight );
				fWidth += 0.5;
				fHeight += 0.5;
				if ( pCurrentState->m_iZeroWidth < (int)fWidth )
				{
					pCurrentState->m_iZeroWidth = (int)fWidth;
				}
				if ( pCurrentState->m_iMaxItemHeight < (int)fHeight )
				{
					pCurrentState->m_iMaxItemHeight = (int)fHeight;
				}

			}
		}

//		DEBUG_FORMAT( _T("CAppMenu::CalcExtents() ZeroWidth=%d ItemHeight=%d"), pCurrentState->m_iZeroWidth, pCurrentState->m_iMaxItemHeight );

		int iIndex = 0;
		LPCTSTR * szaParamList = NULL;

		while(	(pCurrentState->m_pItemList) && 
				(pCurrentState->m_pItemList[iIndex].m_szTitle) && 
				iIndex<pCurrentState->m_iNumItems )
		{
			if ( s_pMenuFont )
			{
				fWidth = 0;
				fHeight = 0;
				s_pMenuFont->GetTextExtent(pCurrentState->m_pItemList[iIndex].m_szTitle, &fWidth, &fHeight );
				fWidth += 0.5;
				fHeight += 0.5;
				if ( iMaxItemWidth < (int)fWidth )
				{
					iMaxItemWidth = (int)fWidth;
				}
				if ( pCurrentState->m_iMaxItemHeight < (int)fHeight )
				{
					pCurrentState->m_iMaxItemHeight = (int)fHeight;
				}
				// Do we have selections associated with this?
				switch( pCurrentState->m_pItemList[iIndex].m_iType )
				{
					case AMIT_SELECTION:
					case AMIT_RETURN:
					case AMIT_SUBMENU:
						break;
					case AMIT_BOOLEAN:
						fWidth = 0.0;
						fHeight = 0.0;
						if ( pCurrentState->m_pItemList[iIndex].m_iParamArg == BooleanSetYesNo )
						{
							s_pMenuFont->GetTextExtent(_T(" Yes"), &fWidth, &fHeight );
						}
						else if ( pCurrentState->m_pItemList[iIndex].m_iParamArg == BooleanSetOnOff )
						{
							s_pMenuFont->GetTextExtent(_T(" Off"), &fWidth, &fHeight );
						}
						else if ( pCurrentState->m_pItemList[iIndex].m_iParamArg == BooleanSetTrueFalse )
						{
							s_pMenuFont->GetTextExtent(_T(" False"), &fWidth, &fHeight );
						}
						if ( iMaxSelectionWidth < (int)(fWidth+0.5) )
						{
							iMaxSelectionWidth = (int)(fWidth+0.5);
						}
						if ( pCurrentState->m_iMaxItemHeight < (int)(fHeight+0.5) )
						{
							pCurrentState->m_iMaxItemHeight = (int)(fHeight+0.5);
						}
						break;
					case AMIT_VALUE:
						{
							CStdString sValue;
							sValue.Format(_T("%d"),m_pHandler->GetMaxValue(pCurrentState->m_lMenuID, m_pCurrentState->m_pItemList[iIndex].m_lID) );
							
							if ( iMaxSelectionWidth < (int)(sValue.GetLength()*pCurrentState->m_iZeroWidth) )
							{
								iMaxSelectionWidth = (int)(sValue.GetLength()*pCurrentState->m_iZeroWidth);
							}
							sValue.Format(_T("%d"),m_pHandler->GetMinValue(pCurrentState->m_lMenuID, m_pCurrentState->m_pItemList[iIndex].m_lID) );
							
							if ( iMaxSelectionWidth < (int)(sValue.GetLength()*pCurrentState->m_iZeroWidth) )
							{
								iMaxSelectionWidth = (int)(sValue.GetLength()*pCurrentState->m_iZeroWidth);
							}
						}
						break;
					case AMIT_STRINGLIST:
						szaParamList = (LPCTSTR *)pCurrentState->m_pItemList[iIndex].m_pParamPtr;
						if ( szaParamList == NULL )
						{
							// Get Parameter from handler
							m_pHandler->PrepareData(pCurrentState->m_lMenuID, iIndex, NULL );
						}
						if ( szaParamList )
						{
							int iParamIndex = 0;

							while( szaParamList[iParamIndex] )
							{
								fWidth = 0;
								fHeight = 0;
								s_pMenuFont->GetTextExtent(szaParamList[iParamIndex], &fWidth, &fHeight );
								// s_pMenuFont->GetTextExtent(pCurrentState->m_pItemList[iIndex].m_szTitle, &fWidth, &fHeight );
								fWidth += 0.5;
								fHeight += 0.5;
								if ( iMaxSelectionWidth < (int)fWidth )
								{
									iMaxSelectionWidth = (int)fWidth;
								}
								if ( pCurrentState->m_iMaxItemHeight < (int)fHeight )
								{
									pCurrentState->m_iMaxItemHeight = (int)fHeight;
								}
								iParamIndex++;
							}
						}
						break;
				}

			}
			iIndex++;
		}
		if ( pCurrentState->m_iMaxItemHeight )
		{
			pCurrentState->m_iMenuWidth = iMaxSelectionWidth+iMaxItemWidth+pCurrentState->m_iZeroWidth;
			pCurrentState->m_iMenuHeight = pCurrentState->m_iMaxItemHeight*pCurrentState->m_iNumItems;
	//	iMaxItemWidth = 0;
	//	iMaxSelectionWidth = 0;
		}
	}
//	DEBUG_FORMAT( _T("CAppMenu::CalcExtents() - EXIT") );
}

HRESULT	CAppMenu::Render()
{
	HRESULT hr = S_OK;
	LPCTSTR * szaParamList = NULL;

	if ( m_pCurrentState && s_pMenuFont)
	{
		if ( m_pCurrentState->m_iMaxItemHeight == 0 )
		{
			CalcExtents(m_pCurrentState);
		}
		
		if ( m_pCurrentState->m_iMaxItemHeight )
		{
			FLOAT fLeft, fRight, fTop, fBottom;
			FLOAT fBoxLeft, fBoxRight, fBoxTop, fBoxBottom;
			int	iIndex;

			DWORD dwTextColor, dwValueColor;

			fLeft = (FLOAT)((640-m_pCurrentState->m_iMenuWidth)/2.0)+(FLOAT)0.5; 
			fRight = fLeft + ((FLOAT)m_pCurrentState->m_iMenuWidth)+(FLOAT)0.5;
			fTop = ((FLOAT)(480-m_pCurrentState->m_iMenuHeight)/(FLOAT)2.0)+(FLOAT)0.5;
			fBottom = fTop + ((FLOAT)m_pCurrentState->m_iMenuHeight)+(FLOAT)0.5;

			fBoxLeft = fLeft - (FLOAT)(m_pCurrentState->m_iZeroWidth/2);
			fBoxRight = fRight + (FLOAT)(m_pCurrentState->m_iZeroWidth/2);
			fBoxTop = fTop - (FLOAT)(m_pCurrentState->m_iMaxItemHeight/2);
			fBoxBottom = fBottom + (FLOAT)(m_pCurrentState->m_iMaxItemHeight/2);


			// Draw background box....
			GetStateApp()->DrawBox(	fBoxLeft, fBoxTop, 
									fBoxRight, fBoxBottom, 
									MenuAttr.m_dwBoxColor, MenuAttr.m_dwBorderColor );

			for( iIndex=0; iIndex<m_pCurrentState->m_iNumItems; iIndex++ )
			{
				if ( m_pCurrentState->m_iCurrPos == iIndex )
				{
					dwTextColor = MenuAttr.m_dwSelectColor;
					dwValueColor = MenuAttr.m_dwSelectValueColor;
				}
				else
				{
					dwTextColor = MenuAttr.m_dwTextColor;
					dwValueColor = MenuAttr.m_dwValueColor;
				}
				// Draw menu item first....
				s_pMenuFont->DrawText( fLeft, fTop, dwTextColor, m_pCurrentState->m_pItemList[iIndex].m_szTitle );

				// Draw option/values
				switch( m_pCurrentState->m_pItemList[iIndex].m_iType )
				{
					case AMIT_SELECTION:
					case AMIT_RETURN:
					case AMIT_SUBMENU:
						break;
					case AMIT_BOOLEAN:
						if ( m_pCurrentState->m_pItemList[iIndex].m_iParamArg == BooleanSetYesNo )
						{
							s_pMenuFont->DrawText( fRight, fTop, dwValueColor, m_pCurrentState->m_iaCurrValue[iIndex]?_T("Yes"):_T("No"), XBFONT_RIGHT );
						}
						else if ( m_pCurrentState->m_pItemList[iIndex].m_iParamArg == BooleanSetOnOff )
						{
							s_pMenuFont->DrawText( fRight, fTop, dwValueColor, m_pCurrentState->m_iaCurrValue[iIndex]?_T("On"):_T("Off"), XBFONT_RIGHT );
						}
						else if ( m_pCurrentState->m_pItemList[iIndex].m_iParamArg == BooleanSetTrueFalse )
						{
							s_pMenuFont->DrawText( fRight, fTop, dwValueColor, m_pCurrentState->m_iaCurrValue[iIndex]?_T("True"):_T("False"), XBFONT_RIGHT );
						}
						break;
					case AMIT_VALUE:
						{
							CStdString sValue;
							sValue.Format(_T("%d"),m_pCurrentState->m_iaCurrValue[iIndex] );
							s_pMenuFont->DrawText( fRight, fTop, dwValueColor, sValue, XBFONT_RIGHT );
						}
						break;
					case AMIT_STRINGLIST:
						szaParamList = (LPCTSTR *)m_pCurrentState->m_pItemList[iIndex].m_pParamPtr;
						if ( szaParamList == NULL )
						{
							// Get Parameter from handler
//							m_pHandler->PrepareData(m_pCurrentState->m_lMenuID, iIndex, NULL );
						}
						if ( szaParamList )
						{
							if( szaParamList[m_pCurrentState->m_iaCurrValue[iIndex]] )
							{
								s_pMenuFont->DrawText( fRight, fTop, dwValueColor, szaParamList[m_pCurrentState->m_iaCurrValue[iIndex]], XBFONT_RIGHT );
							}
						}
						break;
				}
				fTop += (FLOAT)(m_pCurrentState->m_iMaxItemHeight);
			}

		}
		else
		{
			// Couldn't get extents! Can't draw a menu!!
		}
	}
	return hr;
}

BOOL	CAppMenu::MenuFrameMove()
{
	BOOL bResult = TRUE;
	DWORD dwDir;
	WORD wState = 0;
	BYTE byVirtKey;


	if ( m_pCurrentState )
	{
		long lMenuID = m_pCurrentState->m_lMenuID;
		int iMenuPos = m_pCurrentState->m_iCurrPos;

		// pressed keys for other things...
		WORD wPressedButtons = GetStateApp()->GetDefaultGamepad()->wPressedButtons;
		WORD wRemotes = GetStateApp()->m_DefaultIR_Remote.wPressedButtons;

		dwDir = GetStateApp()->GetDir(DIR_IN_ALL, &wState );
//		if ( dwDir )
//		{
//			DEBUG_FORMAT( _T("GetDir=0x%08x 0x%04x"), dwDir, wState );
//		}
		dwDir &= (DWORD)wState;

		byVirtKey = GetStateApp()->GetVirtualKey();
		dwDir |= GetStateApp()->GetKeyDir( byVirtKey, &wState );
		if ( dwDir & DC_UP )
		{
			if ( m_pCurrentState->m_iCurrPos > 0 )
			{
				m_pCurrentState->m_iCurrPos--;
			}
			else
			{
				m_pCurrentState->m_iCurrPos = (m_pCurrentState->m_iNumItems-1);
				// m_pCurrentState->m_iCurrPos = 0;
			}
		}
		else if ( dwDir & DC_DOWN )
		{
			if ( m_pCurrentState->m_iCurrPos < (m_pCurrentState->m_iNumItems-1) )
			{
				m_pCurrentState->m_iCurrPos++;
			}
			else
			{
				// m_pCurrentState->m_iCurrPos = (m_pCurrentState->m_iNumItems-1);
				m_pCurrentState->m_iCurrPos = 0;
			}
		}
		else if ( dwDir & DC_LEFT )
		{
			// Change value....
			switch( m_pCurrentState->m_pItemList[iMenuPos].m_iType )
			{
				case AMIT_SELECTION:
				case AMIT_RETURN:
				case AMIT_SUBMENU:
					break;
				case AMIT_BOOLEAN:
					if( m_pCurrentState->m_iaCurrValue[iMenuPos] )
					{
						m_pCurrentState->m_iaCurrValue[iMenuPos] = (int)FALSE;
						if ( m_pHandler )
						{
							m_pHandler->SetValue( lMenuID, m_pCurrentState->m_pItemList[iMenuPos].m_lID, (int)FALSE );
						}
					}
					else
					{
						m_pCurrentState->m_iaCurrValue[iMenuPos] = (int)TRUE;
						if ( m_pHandler )
						{
							m_pHandler->SetValue( lMenuID, m_pCurrentState->m_pItemList[iMenuPos].m_lID, (int)TRUE );
						}
					}
					break;
				case AMIT_VALUE:
				case AMIT_STRINGLIST:
					{
						if ( m_pCurrentState->m_iaCurrValue[iMenuPos] > m_pCurrentState->m_pItemList[iMenuPos].m_lMin )
						{
							m_pCurrentState->m_iaCurrValue[iMenuPos]--;
						}
						else
						{
							m_pCurrentState->m_iaCurrValue[iMenuPos] = m_pCurrentState->m_pItemList[iMenuPos].m_lMin;
						}
					}
					if ( m_pHandler )
					{
						m_pHandler->SetValue( lMenuID, m_pCurrentState->m_pItemList[iMenuPos].m_lID, m_pCurrentState->m_iaCurrValue[iMenuPos] );
					}
					break;
			}
		}
		else if ( dwDir & DC_RIGHT )
		{
			// Change value....
			switch( m_pCurrentState->m_pItemList[iMenuPos].m_iType )
			{
				case AMIT_SELECTION:
				case AMIT_RETURN:
				case AMIT_SUBMENU:
					break;
				case AMIT_BOOLEAN:
					if( m_pCurrentState->m_iaCurrValue[iMenuPos] )
					{
						m_pCurrentState->m_iaCurrValue[iMenuPos] = (int)FALSE;
					}
					else
					{
						m_pCurrentState->m_iaCurrValue[iMenuPos] = (int)TRUE;
					}
					if ( m_pHandler )
					{
						m_pHandler->SetValue( lMenuID, m_pCurrentState->m_pItemList[iMenuPos].m_lID, m_pCurrentState->m_iaCurrValue[iMenuPos] );
					}
					break;
				case AMIT_VALUE:
				case AMIT_STRINGLIST:
					{
						if ( m_pCurrentState->m_iaCurrValue[iMenuPos] < m_pCurrentState->m_pItemList[iMenuPos].m_lMax )
						{
							m_pCurrentState->m_iaCurrValue[iMenuPos]++;
						}
						else
						{
							m_pCurrentState->m_iaCurrValue[iMenuPos] = m_pCurrentState->m_pItemList[iMenuPos].m_lMax;
						}
					}
					if ( m_pHandler )
					{
						m_pHandler->SetValue( lMenuID, m_pCurrentState->m_pItemList[iMenuPos].m_lID, m_pCurrentState->m_iaCurrValue[iMenuPos] );
					}
					break;
			}
		}
		else if ( ( wPressedButtons & XINPUT_GAMEPAD_START ) || (GetStateApp()->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_A]) || byVirtKey==VK_RETURN || wRemotes == XINPUT_IR_REMOTE_SELECT )
		{

			GetStateApp()->SetDebounce();

			CAppMenuState * pMenuState = m_pCurrentState;
			bool	bBackup = false;
			bool	bBackAll = false;
			// Actions: "Start" and "A"
			HRESULT hr = S_OK;

			hr = m_pHandler->SelectItem( lMenuID, m_pCurrentState->m_pItemList[iMenuPos].m_lID, m_pCurrentState->m_iaCurrValue[iMenuPos], &m_pCurrentState->m_pItemList[iMenuPos] );
			if ( hr == S_FALSE )
			{
				GetStateApp()->SetDebounce();
				bBackup = true;
				bBackAll = true;
			}
			else
			{
				if ( m_pCurrentState->m_pItemList[iMenuPos].m_iType == AMIT_SUBMENU )
				{
					// Special case. We notified the app, so now load the menu
					if ( m_pCurrentState->m_pItemList[iMenuPos].m_pParamPtr )
					{
						pMenuState = new CAppMenuState( this, m_pCurrentState, (_TAppMenuItem *)m_pCurrentState->m_pItemList[iMenuPos].m_pParamPtr, m_pCurrentState->m_pItemList[iMenuPos].m_lID );
					
						m_pCurrentState = pMenuState;

						PrepState();

					}

				}
				else
				{
					if ( m_pCurrentState->m_pItemList[iMenuPos].m_iType == AMIT_RETURN )
					{
						if ( hr != S_FALSE )
						{
							bBackup = true;
							if ( m_pCurrentState->m_pItemList[iMenuPos].m_lID == -2 )
							{
								// Back up out completely
								bBackAll = true;
							}
						}
					}
				}
			}
			if ( bBackup )
			{
				while ( pMenuState )
				{
					m_pCurrentState = pMenuState->m_pPrevState;
					delete pMenuState;
					pMenuState = m_pCurrentState;
					if ( !bBackAll )
					{
						break;
					}
				}
			}

		}
		else if ( (wPressedButtons & XINPUT_GAMEPAD_BACK )||(byVirtKey==VK_ESCAPE) || (wRemotes == XINPUT_IR_REMOTE_BACK) || (GetStateApp()->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_B]) )
		{
			GetStateApp()->SetDebounce();
//			DEBUG_FORMAT( _T("BACK pressed: 0x%08x -> 0x%08x"), m_pCurrentState, m_pCurrentState->m_pPrevState );

			if ( m_pCurrentState )
			{
				CAppMenuState * pMenuState = m_pCurrentState;

				m_pCurrentState = pMenuState->m_pPrevState;
				delete pMenuState;
			}
		}
	}

	if ( m_pCurrentState == NULL )
	{
//		DEBUG_FORMAT( _T("Leaving AppMenu FrameMove FALSE"));
		bResult = FALSE;
	}

	return bResult;
}

CAppMenuState::CAppMenuState(CAppMenu * pMenu, CAppMenuState * pPrevState, _TAppMenuItem * pItemList, long lMenuID ) :
	m_iMenuWidth(0),
	m_iMenuHeight(0),
	m_iNumItems(0),
	m_pMenu(pMenu),
	m_pPrevState(pPrevState),
	m_lMenuID(lMenuID),
	m_iMaxItemHeight(0)
{
	m_pItemList = pItemList;

	m_iCurrPos = 0;

//	DEBUG_FORMAT( _T("CAppMenuState::CAppMenuState()") );
	while(	(m_pItemList) && 
			(m_pItemList[m_iNumItems].m_szTitle) && 
			(m_iNumItems<AM_MAXITEMS))
	{
		m_iNumItems++;

	}

//	DEBUG_FORMAT( _T("CAppMenuState::CAppMenuState() - %d items"), m_iNumItems );

	if ( pMenu )
	{
		pMenu->CalcExtents(this);
	}

}

CAppMenuState::~CAppMenuState()
{
}




/*

  Rendering the menu... a few notes:

  Our AppMenu will not have to deal with scrolling. The menu will be fixed items.

  If they do not fit on the screen, then :::shrug::: the programmer should know
  better! They will just have to set up their own renderer in that case.

  When an item is "selected", there may be a dialog assosicated with that option.

  For Value dialogs:
   
	 User will be presented with a box displaying the menu option, a visual slider,
	 min and max, as well as the "new" value

  +----------------------------------------------+
  |             SomeValue                        |
  |                                              |
  | 0 *************..........................100 |
  |                     33                       |
  |   "A"/Start to select,  "Back" to cancel     |
  +----------------------------------------------+

  For Yes no Dialogs:
   
	 User will be able to switch between "Yes" or "No" 

  +-----------------------------+
  |      Activate FTP           |
  |     +-----+   +-----+       |
  |     | YES |   | NO  |       |
  |     +-----+   +-----+       |
  | "A" to select,"B" to cancel |
  +-----------------------------+

  For String Selection dialogs:

  +-----------------------------+
  |         Some Mode           |
  |                             |
  | <     A Value Mode      >   |
  |                             |
  | "A" to select,"B" to cancel |
  +-----------------------------+



   OR alternately, we simply display the value with the menu item


   We'll have to render one side of the menu items, another for 
   the values, if there are value items.

  +------------------------------------------------+
  |    Configuration Menu                          |
  |                                                |
  | YesNo Choice                               YES |
  | String Choice                        Some Mode |
  | Value Choice                               100 |
  | SubMenu...                                     |
  |                                                |
  | Return to Main                                 |
  +------------------------------------------------+

   So, we must find the maximum length menu item, by getting text extents of each
   menu option for max width and height, then do the same for each string choice
   Yes no choice, and numeric value possible (use "00000" as size of digits)

  So in preperation for our menu display, we need to run a function to precache 
  these values and set up our true size,


Using AppMenu:


  Create a handler object, to get/set values, maximums, etc...

  Call DisplayMenu() to begin menu sequence

   Creates CAppMenu object
     Creates CAppMenuState object (Parent)
	  Initializes entries with values, gets text extents


  FrameMove() is overridden for menu
  Render() is called, but menu and other items are rendered above that

  While an item is not selected, user can move up and down
   On some items, user can move left or right, to modify a value
   Other items can be selected, allowing for custom dialogs to be drawn
   by the handler object




  */
