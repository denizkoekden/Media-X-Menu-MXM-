

#include "xk.h"
#include "XKUtils\XKUtils.h"



void XK_XBOXPowerOff(void)
{
	XKUtils::XBOXPowerOff();
}

void XK_XBOXPowerCycle( void )
{
	XKUtils::XBOXPowerCycle();
}

void XK_TrayEject( void )
{
	XKUtils::DVDEjectTray();
}

void XK_TrayClose( void )
{
	XKUtils::DVDLoadTray();
}