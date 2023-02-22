#pragma once


/*

<InputMap Define="ScreenShot">
<Device>GamePad</Device>
<Shift>RightTrigger</Shift>
<Press>X</Press>
</InputMap>

<InputMap Define="ScreenShot">
<Device>Keyboard</Device>
<Shift>Alt</Shift>
<Press>PrtScr</Press>
</InputMap>

<InputMap Define="ScreenShot">
<Device>Remote</Device>
<Press>Title</Press>
</InputMap>

Maps:
MenuUp
MenuDown
MenuRight
MenuLeft
MenuSelect
Back
MXMHelp
ItemHelp
SystemMenu
Dashboard
NextSong
PrevSong
ScreenShot
Reboot


Devices:
GamePad
SHIFTS:
-RightTrigger
-LeftTrigger
-Ignore
PRESSES:
-A
-B
-X
-Y
-BLACK
-WHITE
-START
-BACK
-RIGHTTHUMB
-LEFTTHUMB
-DPADUP
-DPADDOWN
-DPADLEFT
-DPADRIGHT
-RSTICKUP
-RSTICKDOWN
-RSTICKLEFT
-RSTICKRIGHT
-LSTICKUP
-LSTICKDOWN
-LSTICKLEFT
-LSTICKRIGHT

Keyboard
SHIFTS:
-ALT
-SHIFT
-CTRL
-Ignore
PRESSES


*/




class CXBInputMap
{
public:
	BYTE	m_byShifts;

	CXBInputMap(void);
	virtual ~CXBInputMap(void);
};
