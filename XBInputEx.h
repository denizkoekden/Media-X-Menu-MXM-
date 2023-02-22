/*
 * XBoxMediaPlayer
 * Copyright (c) 2002 d7o3g4q and RUNTiME
 * Portions Copyright (c) by the authors of ffmpeg and xvid
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

// XBInputEx.h: interface for the XBInputEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XBINPUTEX_H__A3816A1D_6A04_4295_95C0_AF9708BA0D07__INCLUDED_)
#define AFX_XBINPUTEX_H__A3816A1D_6A04_4295_95C0_AF9708BA0D07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifdef __cplusplus
extern "C" {
#endif

#include "XBIRRemote.h"

#ifdef __cplusplus
}
#endif



typedef struct _XINPUT_STATEEX
{
    DWORD dwPacketNumber;
    XINPUT_IR_REMOTE IR_Remote;
} XINPUT_STATEEX, *PXINPUT_STATEEX;


//-----------------------------------------------------------------------------
// Name: struct XBGAMEPAD
// Desc: structure for holding Game pad data
//-----------------------------------------------------------------------------
struct XBIR_REMOTE : public XINPUT_IR_REMOTE
{
    // State of buttons tracked since last poll
    WORD       wLastButtons;
    WORD       wPressedButtons;


    // Device properties
    XINPUT_CAPABILITIES caps;
    HANDLE     hDevice;


	// Rumble properties
    XINPUT_RUMBLE   Remote_Feedback;
    XINPUT_FEEDBACK Feedback;

    // Flags for whether game pad was just inserted or removed
    BOOL       bInserted;
    BOOL       bRemoved;
};

//-----------------------------------------------------------------------------
// Global access to ir remote devices
//-----------------------------------------------------------------------------
extern XBIR_REMOTE g_IR_Remote[4];

//-----------------------------------------------------------------------------
// Name: XBInput_CreateIR_Remotes()
// Desc: Creates the ir remote devices
//-----------------------------------------------------------------------------
HRESULT XBInput_CreateIR_Remotes( XBIR_REMOTE** ppIR_Remote = NULL );

//-----------------------------------------------------------------------------
// Name: XBInput_GetInput()
// Desc: Processes input from the ir remote
//-----------------------------------------------------------------------------
VOID XBInput_GetInput( XBIR_REMOTE* pIR_Remote = NULL, FLOAT m_fTime = NULL);



#endif // !defined(AFX_XBINPUTEX_H__A3816A1D_6A04_4295_95C0_AF9708BA0D07__INCLUDED_)
