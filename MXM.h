

#ifndef __MXM_H__
#define __MXM_H__

#include "FontCache.h"


#define NOXDEBUG
#define NOUSECOMMDBG


//void DumpNode( CXMLNode * pNode, int iLevel = 0 );

extern CRITICAL_SECTION g_RenderCriticalSection;

//#define GP_LEFTTHUMBSTICK  0x0040
//#define GP_RIGHTTHUMBSTICK 0x0080
//#define GP_BACKBUTTON      0x0020
//#define GP_STARTBUTTON     0x0010
//#define GP_DIRUP		   0x0001
//#define GP_DIRDOWN		   0x0002
//#define GP_DIRLEFT		   0x0004
//#define GP_DIRRIGHT		   0x0008

//#define GP_ANALOG_A			0
//#define GP_ANALOG_B			1
//#define GP_ANALOG_X			2
//#define GP_ANALOG_Y			3

#define GS_TITLE		1
#define GS_MENU			2
#define GS_SAVER		3
#define GS_LOAD			4
#define GS_PASS			5
#define GS_GHELP		6
#define GS_HELP			7
#define GS_THUMB		8
#define GS_INFO			9

// Subsets for within the gamepad mesh. This are specific to the gamepad mesh.
// For instance, subset 7 represents an internal XBMESH_SUBSET structure for
// rendering the geometry for the gamepad's x button.
#define CONTROL_LEFTTHUMBSTICK  0
#define CONTROL_RIGHTTHUMBSTICK 1
#define CONTROL_BODY            2
#define CONTROL_BACKBUTTON      3
#define CONTROL_STARTBUTTON     4
#define CONTROL_ABUTTON         5
#define CONTROL_BBUTTON         6
#define CONTROL_XBUTTON         7
#define CONTROL_YBUTTON         8
#define CONTROL_WHITEBUTTON     9
#define CONTROL_BLACKBUTTON    10
#define CONTROL_CORDSTUB       11
#define CONTROL_GASKETS        12
#define CONTROL_MEMCARDSLOT    13
#define CONTROL_LEFTTRIGGER    14
#define CONTROL_RIGHTTRIGGER   15
#define CONTROL_DPAD           16
#define CONTROL_JEWEL          17
#define NUM_CONTROLS           18


#define MAX_STATES    20

#define FVF_SPRITE (D3DFVF_XYZRHW | D3DFVF_TEX1)

extern CFontCache g_FontCache;

#define PSC_GUI			0x0001

HRESULT ProcessScriptCommand( LPCTSTR szArg, CStdString & sResponse, LPCTSTR szPath = NULL, DWORD dwFlags = 0, TMapStrings * pLocalEnv = NULL );
//HRESULT ProcessScriptCommand( LPCTSTR szArg, CStdString & sResponse );


#endif