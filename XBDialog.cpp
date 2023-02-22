

#include <xtl.h>
#include "XBDialog.h"
#include "XBStateApp.h"

/*

<Dialogs>
 <name>
  <Position l="50" t="50" w="380" h=" />
  <control type="button" text="ok" value="ok">
  </control>
 </name>
</Dialogs>



Keys/Input:
OK     (Enter [except in multi-edit], GP Start)
Cancel (Esc, GP Back )
FocusNext (Tab, GP Black)
FocusPrev (Shift-Tab, Shift-GP Black)
VirtualKeyboard (GP White)
Mouse (GP Right Thumbstick)
MouseClick (GP Right Thumbstick Click)


Controls:
Numeric Spin Box
 Range
 Value
 -----Controls:
  Up
  Down
  ShiftUp
  ShiftDown
  CharIn (Numeric, Del, Backspace)
 -----D-Pad adjust Up/Down (Shift uses larger increments)


String Spin Box
 List
 Value
 -----Controls:
  Up
  Down
  ShiftUp
  ShiftDown
  CharIn (Selects next string starting with that character)

RadioGroupBox
 List
 Value

CheckBox
 Text
 Left/Right Alignment
 Checked

EditBox
 SingleLine/MultiLine
 Value
 CursorPosition
 ReadOnly


Button


GamePad Editing:
 Right Thumb: Mouse Pointer
 D-Pad: Select Control






*/



CXBControl::CXBControl()
{
}

CXBControl::~CXBControl()
{
}

HRESULT CXBControl::Render( void )
{
	HRESULT hr = S_OK;


	return hr;
}

HRESULT CXBControl::HandleInput( void )
{
	HRESULT hr = S_OK;


	return hr;
}

void	CXBControl::OnSetFocus( void )
{
}

void	CXBControl::OnKillFocus( void )
{
}

void CXBControl::Focus( BOOL bFocus )
{
	if ( bFocus )
	{
		if ( !m_bHasFocus )
		{
			m_bHasFocus = true;
			OnSetFocus();
		}
	}
	else
	{
		if ( m_bHasFocus )
		{
			m_bHasFocus = false;
			OnKillFocus();
		}
	}
}

void CXBControl::DeleteAttached( void )
{
	if ( m_bIsAttachedToList )
	{
		delete this;
	}
}

////////////////////////////////////////////////////////////////
// CXBDialog
////////////////////////////////////////////////////////////////

CXBDialog::CXBDialog()
{
}

CXBDialog::~CXBDialog()
{
	if ( m_pChild )
	{
		delete m_pChild;
	}
}




void	CXBDialog::CloseDialog( BOOL bOK )
{
	bool bCloseIt = true;

	// Force children to close gracefully
	if ( m_pChild )
	{
		m_pChild->CloseDialog( bOK );
	}
	// Only wrap up if our children closed properly!
	if ( m_pChild == NULL )
	{
		if ( bOK )
		{
			if ( OnOK() == FALSE )
			{
				bCloseIt = false;
			}
		}
		else
		{
			OnCancel();
		}
		if ( bCloseIt )
		{
			// Finally, kill us!
			if ( m_pParent )
			{
				m_pParent->RemoveChildDialog( this );
			}
			else
			{
				// Must be in stateapp
				GetStateApp()->RemoveDialog( this );
			}
		}
	}
}


void CXBDialog::DeleteAttachedControls( void )
{
	TCtrlList::iterator iterControls;
	CXBControl * pControl = NULL;

	iterControls = m_ctrlList.begin();
	while( iterControls != m_ctrlList.end() )
	{
		
		pControl = *iterControls;
		pControl->DeleteAttached();
		iterControls++;
	}
}



HRESULT CXBDialog::SetFocus( int iCtrlID )
{
	CXBControl * pControl = GetControlByID( iCtrlID );
	HRESULT hrResult = E_FAIL;

	if ( pControl != m_pControlFocus )
	{
		if ( m_pControlFocus )
		{
			m_pControlFocus->Focus(FALSE);
		}
		m_pControlFocus = pControl;
		if ( pControl )
		{
			m_pControlFocus->Focus(TRUE);
		}
		hrResult = S_OK;
	}
	return hrResult;
}

BYTE CXBDialog::GlobalAlphaShift( void )
{
	BYTE byReturn = 0;

	if ( m_pChild )
	{
		byReturn = m_pChild->GlobalAlphaShift()+1;
	}
	return byReturn;
}

CXBControl * CXBDialog::GetControlByID( int iID )
{
	CXBControl *pReturn = NULL;
	TCtrlList::iterator iterControls;
	CXBControl * pControl = NULL;

	iterControls = m_ctrlList.begin();
	while( iterControls != m_ctrlList.end() )
	{
		pControl = *iterControls;
		if ( pControl->GetID() == iID )
		{
			pReturn = pControl;
			break;
		}
		iterControls++;
	}
	return pReturn;
}

bool CXBDialog::AttachControl( CXBControl * pControl )
{
	bool bReturn = false;

	if ( pControl->HandlesInput() ) // Must have ID
	{
		if ( GetControlByID( pControl->GetID() ) == NULL )
		{
			m_ctrlList.push_back( pControl );
			bReturn = true;
		}
	}
	else
	{
		m_ctrlList.push_back( pControl );
		bReturn = true;
	}
	return bReturn;
}

void CXBDialog::RemoveChildDialog( CXBDialog * pDialog )
{
	if ( pDialog == NULL )
	{
		pDialog = m_pChild;
	}
	if ( m_pChild && m_pChild == pDialog )
	{
		delete m_pChild;
		m_pChild = NULL;
	}
}

void CXBDialog::AttachDialog( CXBDialog * pDialog )
{
	if ( m_pChild )
	{
		m_pChild->AttachDialog( pDialog );
	}
	else
	{
		m_pChild = pDialog;
		pDialog->m_pParent = this;
	}
}

HRESULT CXBDialog::Render( void )
{
	return S_OK;
}


//Keys/Input:
//OK     (Enter [except in multi-edit], GP Start)
//Cancel (Esc, GP Back )
//FocusNext (Tab, GP Black)
//FocusPrev (Shift-Tab, Shift-GP Black)
//Help (GP-White)
//VirtualKeyboard (Shift-GP White)
//Mouse (GP Right Thumbstick)
//MouseClick (GP Right Thumbstick Click)
HRESULT	CXBDialog::HandleInput( void )
{
	WORD wButtons = GetStateApp()->GetDefaultGamepad()->wButtons;
	WORD wRemotes = GetStateApp()->m_DefaultIR_Remote.wPressedButtons;
	BYTE byVirtKey = GetStateApp()->GetVirtualKey();

	if ( GetStateApp()->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] )
	{
		if ( (wButtons & XINPUT_GAMEPAD_BLACK) )
		{
			// Prev Focus
		}
		else if ( (wButtons & XINPUT_GAMEPAD_WHITE) )
		{
			// Virtual Keyboard
		}
	}
	else if ( GetStateApp()->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER] )
	{
	}
	else
	{
		// 
		if ( (wButtons & XINPUT_GAMEPAD_BACK ) )
		{
			OnCancel();
		}
		else if ( (wButtons & XINPUT_GAMEPAD_START) )
		{
			OnOK();
		}
		else if ( (wButtons & XINPUT_GAMEPAD_BLACK) )
		{
			OnOK();
		}
		else if ( (wButtons & XINPUT_GAMEPAD_BLACK) )
		{
			// Next Focus
		}
		else if ( (wButtons & XINPUT_GAMEPAD_WHITE) )
		{
			// Help
		}
		else
		{
			// pass on to current control with focus....
		}
	}
	return S_OK;
}

BOOL CXBDialog::OnOK( void )
{
	return TRUE;
}


void CXBDialog::OnCancel( void )
{
}

HRESULT	CXBDialog::OnInitDialog( void )
{
	return S_OK;
}

void CXBDialog::OnChange( int iCtrlID )
{
}
