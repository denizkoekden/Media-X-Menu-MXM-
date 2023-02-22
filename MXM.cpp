
//-----------------------------------------------------------------------------
// File: MXM.cpp
//
// Desc: MediaXMenu main source file
//
// Hist: 
//
// 
//-----------------------------------------------------------------------------
#include "XBApp_mxm.h"
#include "XBFont_mxm.h"
#include <XBMesh.h>
#include <XBUtil.h>
#include "XBResource_mxm.h"
#include "CommDebug.h"

#define DEBUG_KEYBOARD
#include <xkbd.h>
#include <xgraphics.h>
#include <XbSound.h>
#include <WinNT.h>
#include "ItemInfo.h"
#include "MenuInfo.h"
#include "Resource.h"
#include "StdString.h"
//#include "XBXmlCfgAccess.h"
#include "wmvplayer.h"
#include "musicmanager.h"
#include "xntdll.h"
#include "MXM.h"
#include "utils.h"
#include "keyboard.h"
#include "ImageSource.h"
#include "XBInputEx.h"
#include "Skin.h"
#include <list>
#include "XBEasel.h"
#include "ColorCycle.h"
#include "XBStateApp.h"
#include "BinData.h"
#include "DelayController.h"
#include "AppMenu.h"
// #include "GenSkins.h"
#include <direct.h>
#include "xmltree.h"
#include "RenderGadget.h"
#include "ImgFileCache.h"
#include "Helpers.h"
#include "xk.h"
//#include "FTPServer.h"
// #include "ftp_mxm.h"
#include "xbdirectorylister.h"
#include "xbftpserver.h"
// #include "ftpuser.h"
#include "xbmd5.h"
#include "XbeInfo.h"
#include "ActionDraw.h"
#include "ASXML.h"
#include "DNSCache.h"

#include "bintree.h"
#include "ActionScript.h"
#include "ASScheduler.h"

#include "URI.h"


#include "BinXmlInternal.h"

#include "NewsFeed.h"
#include "CherryPatch.h"


#include "XKUtils\XKEEPROM.h"

#include "XmlStream.h"

#include "XBSocket.h"


extern TMapStrings	g_msGlobalEnvironment;

CRITICAL_SECTION g_RenderCriticalSection;
CStdString		g_sPSCCurDir(_T(""));
//TMapStrings		g_msEnvironment;
//TMapStrings		g_msLocalEnvironment;
CStdString		g_sFormatResult;

void DrawAttributedText( CXBFont * pFont, CTextAttributes * pAttrSet, int iX, int iY, int iWidth, LPCTSTR szValue, CCaratInfo * pCaratInfo = NULL );


LPCTSTR pszList[] = { _T("One"),_T("Two"),_T("Three"),_T("Four"), NULL };

#define MSG_INIT_INIT		0
#define MSG_INIT_SKINS		1
#define MSG_INIT_MENUS		2
#define MSG_INIT_CONFIG		3
#define MSG_INIT_FTP		4
#define MSG_INIT_BIOS		5
#define MSG_INIT_SYSTEM		6
#define MSG_INIT_NETWORK	7
#define MSG_INIT_FINAL		8
#define MSG_INIT_INITSKIN	9

bool CheckTime( void );

LPCTSTR g_pszInitStatus[] = 
{ 
	_T("initializing"), 
	_T("loading skins"),
	_T("loading menus"), 
	_T("loading config"), 
	_T("init ftp"), 
	_T("loading bios table"), 
	_T("init system"), 
	_T("init network"), 
	_T("final initialization"), 
	_T("initializing skin"),
	NULL };



#define MNU_MAIN			0
#define MNU_TITLE			1
#define MNU_TEST			2
#define MNU_SUB_CFG			3
#define MNU_SUB_OTHER		4
#define MNU_DASHBOARD		5
#define MNU_HARDREBOOT		6
#define MNU_WARMREBOOT		13
#define MNU_GAMEINFO		7
#define MNU_ABOUT			8
#define MNU_POWEROFF		9
#define MNU_RSTCACHE		10
#define MNU_INFOSCREEN		11
#define MNU_CONFIG			12

#define MNU_CFG_MUSIC		14
#define MNU_CFG_AUTOCFG		15
#define MNU_CFG_LOCAL		16
#define MNU_CFG_DVD			17

#define MNU_SAFEMODE		18

#define MNU_CFG_MISC		19
#define MNU_CFG_NET			20

#define MNU_CTX_DVD			40
#define MNU_CTX_HD			41

#define ADIR_FLG_NODEMO		0x00000001
#define ADIR_FLG_DEFONLY	0x00000002

// #define VERSION_STRING	"0.9n.5 Beta WIP"
#define VERSION_STRING	"0.9n.6 Beta"

XKEEPROM g_eepromInfo;

CFontCache g_FontCache;
CStdString g_sLaunchDescr;
// LAUNCH_DATA g_ldInfo;
DWORD g_dwLaunchInfoResult = -54321;
LPCTSTR * g_pszSkinsList = NULL;

//struct TActionDrawState
//{
//	LPDIRECT3DTEXTURE8	m_pBackTexture;
//	bool				m_bBeginDraw;
//	IDirect3DDevice8*	m_pDevice;
//} g_ActionDrawState = { NULL, false, NULL };

class CMXMApp;

CMXMApp * GetMXMApp( void );
bool DoesScriptExist( LPCTSTR szName, bool bContext = false );
CStdString GetInternalScript( LPCTSTR szName, bool bContext = false );
bool SetActionVar( LPCTSTR szVarName, LPCTSTR szValue, TMapStrings * pMapLocal = NULL );
void LaunchActionScript( LPCTSTR szAction, bool bContext = false );
DWORD ActionScriptThreadHandlerProc( LPVOID pParam );


//void BuildXDISkin( void );
//bool FindMXM( LPCTSTR szPath );

LPSTR g_szPatchKeyword = "PATCHPOINT4321:02X00000";


_TAppMenuItem g_DVDContextSubMenu[12];
_TAppMenuItem g_HDContextSubMenu[12];

void BuildContextMenus( void )
{
	int iIndex = 0, iDVDIndex = 0, iHDIndex = 0;

	CXMLNode * pContextItemNode;
	CXMLNode * pContextNode;
	CStdString sMode, sName;
	LPCTSTR szTitle;

	for( iIndex=0; iIndex<12; iIndex++ )
	{
		g_DVDContextSubMenu[iIndex].m_szTitle = NULL;
		g_DVDContextSubMenu[iIndex].m_iType = 0;
		g_DVDContextSubMenu[iIndex].m_lID = 0;
		g_DVDContextSubMenu[iIndex].m_pParamPtr = NULL;
		g_DVDContextSubMenu[iIndex].m_iParamArg = 0;
		g_DVDContextSubMenu[iIndex].m_lMin = 0;
		g_DVDContextSubMenu[iIndex].m_lMax = 0;
		g_HDContextSubMenu[iIndex].m_szTitle = NULL;
		g_HDContextSubMenu[iIndex].m_iType = 0;
		g_HDContextSubMenu[iIndex].m_lID = 0;
		g_HDContextSubMenu[iIndex].m_pParamPtr = NULL;
		g_HDContextSubMenu[iIndex].m_iParamArg = 0;
		g_HDContextSubMenu[iIndex].m_lMin = 0;
		g_HDContextSubMenu[iIndex].m_lMax = 0;
	}

	iIndex = 0;

	pContextNode = g_MenuInfo.m_pInternalInfo->GetNode( _T("context") );
	while( pContextNode && (pContextItemNode = pContextNode->GetNode( _T("Item"), iIndex++ )) )
	{
		sName = pContextItemNode->GetString( NULL, _T("name"), _T(""), true );
		if ( sName.GetLength() )
		{
			szTitle = _tcsdup( sName );
			sMode = pContextItemNode->GetString( NULL, _T("mode"), _T(""), true );
			if ( iDVDIndex < 9 && _tcsicmp( sMode, _T("hd") ) )
			{
				g_DVDContextSubMenu[iDVDIndex].m_szTitle = szTitle;
				g_DVDContextSubMenu[iDVDIndex].m_iType = AMIT_SELECTION;
				g_DVDContextSubMenu[iDVDIndex].m_lID = iDVDIndex;
				g_DVDContextSubMenu[iDVDIndex].m_pParamPtr = pContextItemNode;
				iDVDIndex++;
			}
			if ( iHDIndex < 9 && _tcsicmp( sMode, _T("dvd") ) )
			{
				g_HDContextSubMenu[iHDIndex].m_szTitle = szTitle;
				g_HDContextSubMenu[iHDIndex].m_iType = AMIT_SELECTION;
				g_HDContextSubMenu[iHDIndex].m_lID = iHDIndex;
				g_HDContextSubMenu[iHDIndex].m_pParamPtr = pContextItemNode;
				iHDIndex++;
			}
		}
	}
	pContextNode = g_MenuInfo.m_pCfgInfo->GetNode( _T("context") );
	while( pContextNode && (pContextItemNode = pContextNode->GetNode( _T("Item"), iIndex++ )) )
	{
		sName = pContextItemNode->GetString( NULL, _T("name"), _T(""), true );
		if ( sName.GetLength() )
		{
			szTitle = _tcsdup( sName );
			sMode = pContextItemNode->GetString( NULL, _T("mode"), _T(""), true );
			if ( iDVDIndex < 9 && _tcsicmp( sMode, _T("hd") ) )
			{
				g_DVDContextSubMenu[iDVDIndex].m_szTitle = szTitle;
				g_DVDContextSubMenu[iDVDIndex].m_iType = AMIT_SELECTION;
				g_DVDContextSubMenu[iDVDIndex].m_lID = iDVDIndex;
				g_DVDContextSubMenu[iDVDIndex].m_pParamPtr = pContextItemNode;
				iDVDIndex++;
			}
			if ( iHDIndex < 9 && _tcsicmp( sMode, _T("dvd") ) )
			{
				g_HDContextSubMenu[iHDIndex].m_szTitle = szTitle;
				g_HDContextSubMenu[iHDIndex].m_iType = AMIT_SELECTION;
				g_HDContextSubMenu[iHDIndex].m_lID = iHDIndex;
				g_HDContextSubMenu[iHDIndex].m_pParamPtr = pContextItemNode;
				iHDIndex++;
			}
		}
	}
	g_DVDContextSubMenu[iDVDIndex].m_szTitle = _T("Return");;
	g_DVDContextSubMenu[iDVDIndex].m_iType = AMIT_RETURN;
	g_DVDContextSubMenu[iDVDIndex].m_lID = -1;
	g_HDContextSubMenu[iHDIndex].m_szTitle = _T("Return");
	g_HDContextSubMenu[iHDIndex].m_iType = AMIT_RETURN;
	g_HDContextSubMenu[iHDIndex].m_lID = -1;
}

LPCTSTR g_szPlayerChoices[] = { _T("Default"), _T("DVD-RegionX2"), _T("Microsoft"), NULL };

_TAppMenuItem g_DVDActionSubMenu[] =
{
  {		"Launch Movies", AMIT_BOOLEAN, 1, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {		"Preferred Player", AMIT_STRINGLIST, 5, g_szPlayerChoices, 0, 0, 0 },
  {		"Launch Games", AMIT_BOOLEAN, 2, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {		"AutoLoad Multigame", AMIT_BOOLEAN, 3, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {		"Deep Search Multigame", AMIT_BOOLEAN, 4, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {    "Return", AMIT_RETURN, -1, NULL, 0, 0, 0 },  
  {   NULL, 0, 0, NULL, CAppMenu::EndSet, 0, 0 }
};

LPCTSTR g_szRegionChoices[] = { _T("Default"), _T("Auto"), _T("USA"), _T("JAP"), _T("EUR"), NULL };
LPCTSTR g_szVideoChoices[] = { _T("Default"), _T("Auto"), _T("NTSC-M"), _T("NTSC-J"), _T("PAL50"), _T("PAL60"), NULL };
LPCTSTR g_szMonitorChoices[] = { _T("Default TV"), _T("Multi (PAL/NTSC)"), NULL };

LPCTSTR g_szTempChoices[] = { _T("Farenheit"), _T("Celsius"), NULL };
LPCTSTR g_szTriState[] = { _T("Default"), _T("Yes"), _T("No"), NULL };
LPCTSTR g_szMenuSounds[] = { _T("Skin Defined"), _T("Blips"), NULL };
LPCTSTR g_szConfigStyle[] = { _T("Apps & Games"), _T("By Drives"), NULL };
LPCTSTR g_szTimeFormat[] = { _T("HH:MM:SS 24hr"), _T("HH:MM:SSam/pm"),_T("HH:MM 24hr"), _T("HH:MMam/pm"), NULL };
LPCTSTR g_szDateFormat[] = { _T("MM/DD/YY"), _T("YY/MM/DD"),_T("DD/MM YY"), _T("DD/MM/YY"), NULL };
LPCTSTR g_szDateSeparator[] = { _T("/"), _T("-"), _T("."), _T("*"), _T("\\"), _T(" "), NULL };
LPCTSTR g_szTimeSeparator[] = { _T(":"), _T("."), _T(" "), _T("-"), _T("|"), NULL };

LPCTSTR g_szFTPPriority[] = { _T("Lowest"), _T("Low"), _T("Below Normal"), _T("Normal"), _T("Above Normal"), _T("High"), _T("Highest"), NULL };

	//if ( sTemp.Compare( _T("abovenormal") ) == 0 )
	//{
	//	g_MenuInfo.m_iFTPPriority = 9;
	//}
	//else if ( sTemp.Compare( _T("belownormal") ) == 0 )
	//{
	//	g_MenuInfo.m_iFTPPriority = 7;
	//}
	//else if ( sTemp.Compare( _T("high") ) == 0 )
	//{
	//	g_MenuInfo.m_iFTPPriority = 10;
	//}
	//else if ( sTemp.Compare( _T("highest") ) == 0 )
	//{
	//	g_MenuInfo.m_iFTPPriority = 11;
	//}
	//else if ( sTemp.Compare( _T("low") ) == 0 )
	//{
	//	g_MenuInfo.m_iFTPPriority = 6;
	//}
	//else if ( sTemp.Compare( _T("lowest") ) == 0 )
	//{
	//	g_MenuInfo.m_iFTPPriority = 5;
	//}
	//else
	//{
	//	g_MenuInfo.m_iFTPPriority = 8;
	//}

//Select Skin (reboots after)
//DVD Drive
// Launch Movies		Yes/No
// Lnch Single Games	Yes/No
// Autoload Multi		Yes/No
// Deep Search Multi	Yes/No
//Boot to Safe Mode
//

_TAppMenuItem g_LocalizationSubMenu[] =
{
  {		"Date Format", AMIT_STRINGLIST, 1, g_szDateFormat, 0, 0, 0 },
  {		"Date Separator", AMIT_STRINGLIST, 2, g_szDateSeparator, 0, 0, 0 },
  {		"Time Format", AMIT_STRINGLIST, 3, g_szTimeFormat, 0, 0, 0 },
  {		"Time Separator", AMIT_STRINGLIST, 4, g_szTimeSeparator, 0, 0, 0 },
  {		"Temperature", AMIT_STRINGLIST, 5, g_szTempChoices, 0, 0, 0 },
  {		"Euro Numbers", AMIT_BOOLEAN, 6, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {		"Region", AMIT_STRINGLIST, 10, g_szRegionChoices, 0, 0, 0 },
  {		"TV Type", AMIT_STRINGLIST, 8, g_szMonitorChoices, 0, 0, 0 },
  {		"Video Standard", AMIT_STRINGLIST, 7, g_szVideoChoices, 0, 0, 0 },
  {		"Force 60hz PAL", AMIT_BOOLEAN, 9, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {    "Return", AMIT_RETURN, -1, NULL, 0, 0, 0 },  
  {   NULL, 0, 0, NULL, CAppMenu::EndSet, 0, 0 }
};



_TAppMenuItem g_AutoConfigSubMenu[] =
{
  {		"AutoCfg Style", AMIT_STRINGLIST, 1, g_szConfigStyle, 0, 0, 0 },
  {		"AutoCfg Add Dashboards", AMIT_BOOLEAN, 2, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {		"AutoCfg Sort By Title", AMIT_BOOLEAN, 3, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {		"AutoCfg Default.xbe only", AMIT_BOOLEAN, 6, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {		"AutoCfg Reject 'Demos'", AMIT_BOOLEAN, 7, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {		"Remove Invalid Entries", AMIT_BOOLEAN, 4, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {		"Remove Duplicate Entries", AMIT_BOOLEAN, 5, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {		"Add Action Menu", AMIT_BOOLEAN, 8, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {    "Return", AMIT_RETURN, -1, NULL, 0, 0, 0 },  
  {   NULL, 0, 0, NULL, CAppMenu::EndSet, 0, 0 }
};

_TAppMenuItem g_MusicSoundSubMenu[] =
{
  {		"Enable Music", AMIT_BOOLEAN, 8, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {		"Music During Video", AMIT_STRINGLIST, 1, g_szTriState, 0, 0, 0 },
  {		"Random Music", AMIT_STRINGLIST, 2, g_szTriState, 0, 0, 0 },
  {		"Global Music", AMIT_STRINGLIST, 3, g_szTriState, 0, 0, 0 },
  {		"Preview Audio", AMIT_STRINGLIST, 4, g_szTriState, 0, 0, 0 },
  {		"Menu Sounds", AMIT_STRINGLIST, 5, g_szMenuSounds, 0, 0, 0 },
  {		"Music Volume", AMIT_VALUE, 6, NULL, 0, 0, 10 },
//  {		"Audio Volume", AMIT_VALUE, 7, NULL, 0, 0, 10 },
  {    "Return", AMIT_RETURN, -1, NULL, 0, 0, 0 },  
  {   NULL, 0, 0, NULL, CAppMenu::EndSet, 0, 0 }
};

_TAppMenuItem g_MiscPrefsSubMenu[] =
{
  {	   "Select Skin", AMIT_STRINGLIST, 2, NULL, 0, 0, 0 },
  {	   "Saver Delay (min, 0=OFF)", AMIT_VALUE, 1, NULL, 0, 0, 30 },
  {	   "Load Internal Skins", AMIT_BOOLEAN, 3, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {	   "Enable Menu Cacheing", AMIT_BOOLEAN, 4, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {    "Enable Partition Formatting", AMIT_BOOLEAN, 5, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {    "Return", AMIT_RETURN, -1, NULL, 0, 0, 0 },  
  {   NULL, 0, 0, NULL, CAppMenu::EndSet, 0, 0 }
};

_TAppMenuItem g_NetPrefsSubMenu[] =
{
  {	   "Enable Network", AMIT_BOOLEAN, 5, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {	   "Enable FTP Server", AMIT_BOOLEAN, 1, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {	   "FTP Server Priority", AMIT_STRINGLIST, 2, g_szFTPPriority, 0, 0, 0 },
  {	   "Allow Anon FTP Login", AMIT_BOOLEAN, 3, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {	   "Synchronize time using Server", AMIT_BOOLEAN, 4, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {	   "Time Synch Hour offset", AMIT_VALUE, 6, NULL, 0, -24, 24 },
  {    "Return", AMIT_RETURN, -1, NULL, 0, 0, 0 },  
  {   NULL, 0, 0, NULL, CAppMenu::EndSet, 0, 0 }
};


_TAppMenuItem g_PrefsSubMenu[] =
{
//  {		"Select Skin", AMIT_STRINGLIST, 1, NULL, 0, 0, 0 },
  {		"Skin & Misc Settings", AMIT_SUBMENU, MNU_CFG_MISC, g_MiscPrefsSubMenu, 0, 0, 0 },
  {		"Music & Sound", AMIT_SUBMENU, MNU_CFG_MUSIC, g_MusicSoundSubMenu, 0, 0, 0 },
  {		"AutoMenu", AMIT_SUBMENU, MNU_CFG_AUTOCFG, g_AutoConfigSubMenu, 0, 0, 0 },
  {		"DVD Drive", AMIT_SUBMENU, MNU_CFG_DVD, g_DVDActionSubMenu, 0, 0, 0 },
  {		"Localization", AMIT_SUBMENU, MNU_CFG_LOCAL, g_LocalizationSubMenu, 0, 0, 0 },
  {		"Network", AMIT_SUBMENU, MNU_CFG_NET, g_NetPrefsSubMenu, 0, 0, 0 },
  {    "Return", AMIT_RETURN, -1, NULL, 0, 0, 0 },  
  {   NULL, 0, 0, NULL, CAppMenu::EndSet, 0, 0 }
};

_TAppMenuItem g_TestAppSubMenu[] =
{
  {    "Test Option", AMIT_SELECTION, 1, NULL, 0, 0, 0 },
  {    "Test YesNo", AMIT_BOOLEAN, 2, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {    "Value", AMIT_VALUE, 3, NULL, 0, 0, 20 },
  {    "Test String", AMIT_STRINGLIST, 4, pszList, 0, 0, 0 },
  {	   "Reboot System", AMIT_SELECTION, 5, NULL, 0, 0, 0 },
  {    "Return", AMIT_RETURN, -1, NULL, 0, 0, 0 },  
  {    "Exit from menu", AMIT_RETURN, -2, NULL, 0, 0, 0 },  
  {   NULL, 0, 0, NULL, CAppMenu::EndSet, 0, 0 }
};

_TAppMenuItem g_ConfigMenu[] =
{
  {    "Skin", AMIT_STRINGLIST, 1, NULL, 0, 0, 0 },
  {    "Return", AMIT_RETURN, -1, NULL, 0, 0, 0 },  
  {   NULL, 0, 0, NULL, CAppMenu::EndSet, 0, 0 }
};

bool SelectSkinByName( LPCTSTR szName );
void MakeInfoScreens( void );



#if 1
_TAppMenuItem g_MainAppMenu[] =
{
  {    "Title Screen", AMIT_SELECTION, MNU_TITLE, NULL, 0, 0, 0 },
  {    "About", AMIT_SELECTION, MNU_ABOUT, NULL, 0, 0, 0 },
  {    "Game Info", AMIT_SELECTION, MNU_GAMEINFO, NULL, 0, 0, 0 },
  {	   "Game Options", AMIT_SELECTION, MNU_CTX_DVD, NULL, 0, 0, 0 },
  {    "System Info", AMIT_SELECTION, MNU_INFOSCREEN, NULL, 0, 0, 0 },
  {	   "Configure", AMIT_SUBMENU, MNU_CONFIG, g_PrefsSubMenu, 0, 0, 0 },
  {    "Load Dashboard", AMIT_SELECTION, MNU_DASHBOARD, NULL, 0, 0, 0 },
  {	   "Reset Menu Cache", AMIT_SELECTION, MNU_RSTCACHE, NULL, 0, 0, 0 },
  {	   "Reboot System", AMIT_SELECTION, MNU_WARMREBOOT, NULL, 0, 0, 0 },
  {	   "Power Cycle", AMIT_SELECTION, MNU_HARDREBOOT, NULL, 0, 0, 0 },
  {	   "Power Off", AMIT_SELECTION, MNU_POWEROFF, NULL, 0, 0, 0 },
  {	   "Reboot to Safe Mode", AMIT_SELECTION, MNU_SAFEMODE, NULL, 0, 0, 0 },
//  {    "Test", AMIT_SELECTION, MNU_TEST, NULL, 0, 0, 20 },
  {    "Return", AMIT_RETURN, -1, NULL, 0, 0, 0 },  
  {   NULL, 0, 0, NULL, CAppMenu::EndSet, 0, 0 }
};

_TAppMenuItem g_MainAppMenuDVD[] =
{
  {    "Title Screen", AMIT_SELECTION, MNU_TITLE, NULL, 0, 0, 0 },
  {    "About", AMIT_SELECTION, MNU_ABOUT, NULL, 0, 0, 0 },
  {    "Game Info", AMIT_SELECTION, MNU_GAMEINFO, NULL, 0, 0, 0 },
  {	   "Game Options", AMIT_SELECTION, MNU_CTX_DVD, NULL, 0, 0, 0 },
  {    "System Info", AMIT_SELECTION, MNU_INFOSCREEN, NULL, 0, 0, 0 },
  {    "Load Dashboard", AMIT_SELECTION, MNU_DASHBOARD, NULL, 0, 0, 0 },
  {	   "Reboot System", AMIT_SELECTION, MNU_HARDREBOOT, NULL, 0, 0, 0 },
  {	   "Power Off", AMIT_SELECTION, MNU_POWEROFF, NULL, 0, 0, 0 },
//  {    "Test", AMIT_SELECTION, MNU_TEST, NULL, 0, 0, 20 },
  {    "Return", AMIT_RETURN, -1, NULL, 0, 0, 0 },  
  {   NULL, 0, 0, NULL, CAppMenu::EndSet, 0, 0 }
};

#else
_TAppMenuItem g_MainAppMenu[] =
{
  {    "Test Option", AMIT_SELECTION, 1, NULL, 0, 0, 0 },
  {    "Test Submenu", AMIT_SUBMENU, 2, g_TestAppSubMenu, 0, 0, 0 },
  {    "Test YesNo", AMIT_BOOLEAN, 3, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
  {    "Value", AMIT_VALUE, 4, NULL, 0, 0, 20 },
  {    "Test String", AMIT_STRINGLIST, 5, pszList, 0, 0, 0 },
  {	   "Reboot System", AMIT_SELECTION, 6, NULL, 0, 0, 0 },
  {    "Return", AMIT_RETURN, -1, NULL, 0, 0, 0 },  
  {   NULL, 0, 0, NULL, CAppMenu::EndSet, 0, 0 }
};
#endif

CXMLNode * GetXDI( void );


// Configure:
// Skin:
// 

#ifdef XDEBUG
#pragma message ( "Compiling NETWORK DEBUG version" )
#endif


// The following IP is the target machine, 
// where the PC Debug client will be run
#define DEBUG_IP  "192.168.0.100"




// list<CStdString> g_saDirectories;
//typedef list<struct T_ItemInfo> TItemList;

CMenuInfo g_MenuInfo;

HRESULT DoActionScript( LPCTSTR szAction, bool bClearLocal = true, TMapStrings		* pmsLocalEnv = NULL, TActionDrawList * pactionDrawList = NULL, TXMLMap * pXMLMap = NULL );
// HRESULT DoActionScript( LPCTSTR szAction, bool bClearLocal = true );
int GetConditionID( LPCTSTR szName );
bool IsConditionMet( int iID, bool bNotCondition, bool bResetDrives );

void SetNTSC( bool bSetToNTSC );


bool IsNTSC( void )
{
	bool bReturn = false;
	if ( XKEEPROM::NTSC_M == g_eepromInfo.GetVideoStandardVal() )
	{
		bReturn = true;
	}
	return bReturn;
}

void SetNTSC( bool bSetToNTSC )
{
//	XKEEPROM eepromInfo;

	PatchCountryVideo( COUNTRY_NULL, bSetToNTSC?VIDEO_NTSCM:VIDEO_PAL50);
//	eepromInfo.SetVideoStandardVal( bSetToNTSC?XKEEPROM::NTSC_M:XKEEPROM::PAL_I );
//	XKUtils::WriteEEPROMToXBOX((LPBYTE)&g_eepromInfo.m_EEPROMData.VideoStandard, 0x58, 0x5B );
//	eepromInfo.WriteToXBOX();
//	g_eepromInfo.ReadFromXBOX();

}

F_COUNTRY GetXboxCountryCode_CP( LPCTSTR szFilename = NULL )
{
	F_COUNTRY cntryReturn = COUNTRY_NULL;

	if ( szFilename == NULL )
	{
		if ( g_eepromInfo.GetXBERegionVal() == XKEEPROM::EURO_AUSTRALIA )
		{
			cntryReturn = COUNTRY_EUR;
		}
		else if ( g_eepromInfo.GetXBERegionVal() == XKEEPROM::JAPAN )
		{
			cntryReturn = COUNTRY_JAP;
		}
		else if ( g_eepromInfo.GetXBERegionVal() == XKEEPROM::NORTH_AMERICA )
		{
			cntryReturn = COUNTRY_USA;
		}
	}
	else
	{
		CXBEInfo * pInfo = NULL;
		pInfo = _GetXBEInfo(szFilename);
		if ( pInfo )
		{
			if ( pInfo->m_dwRegions & XBEIMAGE_GAME_REGION_NA )
			{
				cntryReturn = COUNTRY_USA;
			}
			else if ( pInfo->m_dwRegions & XBEIMAGE_GAME_REGION_JAPAN )
			{
				cntryReturn = COUNTRY_JAP;
			}
			else if ( pInfo->m_dwRegions & XBEIMAGE_GAME_REGION_RESTOFWORLD )
			{
				cntryReturn = COUNTRY_EUR;
			}
			else
			{
				cntryReturn = COUNTRY_USA;
			}
		}
	}
	return cntryReturn;
}

F_VIDEO GetXboxVideoCode_CP( LPCTSTR szFilename = NULL )
{
	F_VIDEO vidReturn = VIDEO_NULL;
	if ( szFilename == NULL )
	{
		if ( XKEEPROM::NTSC_M == g_eepromInfo.GetVideoStandardVal() )
		{
			vidReturn = VIDEO_NTSCM;
		}
		else if ( XKEEPROM::PAL_I == g_eepromInfo.GetVideoStandardVal() )
		{
			vidReturn = VIDEO_PAL50;
		}
	}
	else
	{
		CXBEInfo * pInfo = NULL;
		pInfo = _GetXBEInfo(szFilename);
		if ( pInfo )
		{
			if ( pInfo->m_dwRegions & XBEIMAGE_GAME_REGION_NA )
			{
				vidReturn = VIDEO_NTSCM;
			}
			else if ( pInfo->m_dwRegions & XBEIMAGE_GAME_REGION_JAPAN )
			{
				vidReturn = VIDEO_NTSCJ;
			}
			else if ( pInfo->m_dwRegions & XBEIMAGE_GAME_REGION_RESTOFWORLD )
			{
				vidReturn = VIDEO_PAL50;
			}
			else
			{
				vidReturn = VIDEO_NTSCM;
			}
		}
	}
	return vidReturn;
}

void SearchForAutos( CItemInfo & pItem );

bool CheckDirectory( LPCTSTR szDirectory, CItemInfo & pItem, bool bDefaultOnly = false );

//bool CheckDirectory( LPCTSTR szDirectory, T_ItemInfo * pItem );
// void ScanDirectories( void );

class CMXMAppMenuHandler : public CAppMenuHandler
{
	int m_iTestValue;
public:
	CMXMAppMenuHandler();
	long GetValue( long lMenuId, long lItemID );
	long GetMinValue( long lMenuId, long lItemID );
	long GetMaxValue( long lMenuId, long lItemID );
	void SetValue( long lMenuId, long lItemID, long lValue );
	HRESULT SelectItem( long lMenuID, long lItemID, long lValue, _TAppMenuItem * pItem );
	HRESULT PrepareData( long lMenuID, long lItemID, PVOID *pDataPtr );
	HRESULT FrameMove( long lMenuID, long lItemID );
	HRESULT Render( long lMenuID, long lItemID );
} g_MenuHandler;

CMXMAppMenuHandler::CMXMAppMenuHandler()
{
	m_iTestValue = 0;
}



long CMXMAppMenuHandler::GetValue( long lMenuId, long lItemID )
{
	long lResult = 0;

	switch( lMenuId )
	{
		case MNU_CFG_NET:
			switch ( lItemID )
			{
				case 6:
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("network"), _T("timesynchoffset"), 0 );
					break;
				case 5:
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("network"), _T("enablenetwork"), TRUE );
					break;
				case 1: //	"Enable FTP Server", AMIT_BOOLEAN, 1, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("network"), _T("enableftp"), TRUE );
					break;
				case 2: //	"FTP Server Priority", AMIT_STRINGLIST, 2, NULL, 0, 0, 30 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("network"), _T("ftppriority"), g_MenuInfo.m_iFTPPriority );
					if ( lResult < 5 )
					{
						lResult = 5;
					}
					if ( lResult > 11 )
					{
						lResult = 11;
					}
					lResult -= 5;
					break;
				case 3: //	"Allow Anon FTP Login", AMIT_BOOLEAN, 3, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("network"), _T("ftpanon"), FALSE );
					break;
				case 4: //	"Synchronize time using Server", AMIT_BOOLEAN, 4, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("network"), _T("synchtime"), TRUE );
					break;
			}
			break;
		case MNU_CFG_MISC:
			switch ( lItemID )
			{
				case 2: // Select Skin
					lResult = g_MiscPrefsSubMenu[0].m_iParamArg;
					break;
				case 1: //	"Saver Delay (min, 0=OFF)", AMIT_VALUE, 1, NULL, 0, 0, 30 },
					lResult = g_MenuInfo.m_lSaverDelay / 60000;
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("autocfg"), _T("saverdelay"), lResult );
					break;
				case 3: // Load Internal Skins  LoadSkins
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("skins"), _T("useinternal"), TRUE );
					break;
				case 4: // Enable Menu Cache
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("menu"), _T("usecache"), TRUE );
					break;
				case 5: //   "Enable Partition Formatting" 
					lResult = g_MenuInfo.m_bAllowFormat;
					break;
			}
			break;
		case MNU_CFG_MUSIC:
			switch ( lItemID )
			{
				case 1: //  "Music During Video", AMIT_STRINGLIST, 1, g_szTriState, 0, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("music"), _T("OvMusDurVid"), 0 );
					if ( lResult > 2 || lResult < 0 )
					{
						lResult = 0;
					}
					break;
				case 2: //  "Random Music", AMIT_STRINGLIST, 2, g_szTriState, 0, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("music"), _T("OvMusRandom"), 0 );
					if ( lResult > 2 || lResult < 0 )
					{
						lResult = 0;
					}
					break;
				case 3: //  "Global Music", AMIT_STRINGLIST, 3, g_szTriState, 0, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("music"), _T("OvMusGlobal"), 0 );
					if ( lResult > 2 || lResult < 0 )
					{
						lResult = 0;
					}
					break;
				case 4: //  "Preview Audio", AMIT_STRINGLIST, 4, g_szTriState, 0, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("audio"), _T("OvPrevAudio"), 0 );
					if ( lResult > 2 || lResult < 0 )
					{
						lResult = 0;
					}
					break;
				case 5: //  "Menu Sounds", AMIT_STRINGLIST, 5, g_szMenuSounds, 0, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("audio"), _T("OvMenuSource"), 0 );
					if ( lResult > 1 || lResult < 0 )
					{
						lResult = 0;
					}
					break;
				case 6: //  "Music Volume", AMIT_VALUE, 6, NULL, 0, 0, 63 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("audio"), _T("musicvolume"), GetStateApp()->MusicGetVolume() );
					if ( lResult > 10 || lResult < 0 )
					{
						lResult = 10;
					}
					break;
				case 7: //  "Audio Volume", AMIT_VALUE, 7, NULL, 0, 0, 63 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("audio"), _T("audiovolume"), 10 );
					if ( lResult > 10 || lResult < 0 )
					{
						lResult = 10;
					}
					break;
				case 8: //  "Enable Music", AMIT_BOOLEAN, 8, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("audio"), _T("enablemusic"), TRUE );
					if ( lResult > 1 || lResult < 0 )
					{
						lResult = 0;
					}
					break;

			}
			break;
		case MNU_CFG_AUTOCFG:
			switch ( lItemID )
			{
				case 1: //	"Config Style", AMIT_STRINGLIST, 1, g_szConfigStyle, 0, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("autocfg"), _T("cfgstyle"), 0 );
					if ( lResult > 2 || lResult < 0 )
					{
						lResult = 0;
					}
					break;
				case 2: //	"Add Dashboards", AMIT_BOOLEAN, 2, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("adddash"), TRUE );
					if ( lResult > 2 || lResult < 0 )
					{
						lResult = 0;
					}
					break;
				case 3: //	"Sort By Title", AMIT_BOOLEAN, 3, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("sort"), TRUE );
					if ( lResult > 1 || lResult < 0 )
					{
						lResult = 0;
					}
					break;
				case 4: //	"Remove Invalid Entries", AMIT_BOOLEAN, 4, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("removeinvalid"), TRUE );
					if ( lResult > 1 || lResult < 0 )
					{
						lResult = 0;
					}
					break;
				case 5: //	"Remove Duplicate Entries", AMIT_BOOLEAN, 5, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("removeduplicates"), TRUE );
					if ( lResult > 1 || lResult < 0 )
					{
						lResult = 0;
					}
					break;
				case 6: //	"AutoCfg Default.xbe only", AMIT_BOOLEAN, 6, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("defaultonly"), FALSE );
					if ( lResult > 1 || lResult < 0 )
					{
						lResult = 0;
					}
					break;
				case 7: //	"AutoCfg Reject 'Demos'", AMIT_BOOLEAN, 7, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("nodemos"), FALSE );
					if ( lResult > 1 || lResult < 0 )
					{
						lResult = 0;
					}
					break;
				case 8: // m_bAutoActionMenu
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("actionmenu"), g_MenuInfo.m_bAutoActionMenu );
					if ( lResult > 1 || lResult < 0 )
					{
						lResult = 0;
					}
					break;
			}
			break;
		case MNU_CFG_LOCAL:
			switch ( lItemID )
			{
				case 1: //	"Date Format", AMIT_STRINGLIST, 1, g_szDateFormat, 0, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("dateformat"), 0 );
					break;
				case 2: //	"Date Separator", AMIT_STRINGLIST, 2, g_szDateSeparator, 0, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("datesep"), 0 );
					break;
				case 3: //	"Time Format", AMIT_STRINGLIST, 3, g_szTimeFormat, 0, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("timeformat"), 0 );
					break;
				case 4: //	"Time Separator", AMIT_STRINGLIST, 4, g_szTimeSeparator, 0, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("timesep"), 0 );
					break;
				case 5: //	"Temperature", AMIT_STRINGLIST, 5, g_szTempChoices, 0, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("temptype"), 0 );
					break;
				case 6: //	"Euro Numbers", AMIT_BOOLEAN, 6, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("local"), _T("euronumbers"), FALSE );
					break;
				case 7: // "Video Standard", AMIT_STRINGLIST, 7, g_szVideoChoices, 0, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("videomode"), 0 );
					break;
				case 8: //  "TV Type", AMIT_STRINGLIST, 8, g_szMonitorChoices, 0, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("tvtype"), 0 );
					break;
				case 9:
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("local"), _T("force60pal"), FALSE );
					break;
				case 10:
					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("region"), 0 );
					break;
			}
			break;
		case MNU_CFG_DVD:
			switch ( lItemID )
			{
				case 1: //	"Launch Movies", AMIT_BOOLEAN, 1, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("dvddrive"), _T("launchmovie"), FALSE );
					break;
				case 2: //	"Launch Games", AMIT_BOOLEAN, 2, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("dvddrive"), _T("launchgame"), FALSE );
					break;
				case 3: //	"AutoLoad Multigame", AMIT_BOOLEAN, 3, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("dvddrive"), _T("automulti"), TRUE );
					break;
				case 4: //	"Deep Search Multigame", AMIT_BOOLEAN, 4, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("dvddrive"), _T("deepsearch"), FALSE );
					break;
				case 5: // Preferred player...
					if ( _tcsicmp( g_MenuInfo.m_pPrefInfo->GetString( _T("dvddrive"), _T("PrefPlayer"), _T("") ), _T("M$") ) == 0 )
					{
						lResult = 2;
					}
					else if ( _tcsicmp( g_MenuInfo.m_pPrefInfo->GetString( _T("dvddrive"), _T("PrefPlayer"), _T("") ), _T("X2") ) == 0 )
					{
						lResult = 1;
					}
					else
					{
						lResult = 0;
					}
					break;
			}
			break;
		case MNU_CONFIG:
			switch( lItemID )
			{
				case 1: // Select Skin
					// lResult = g_PrefsSubMenu[0].m_iParamArg;
					break;
			}
			break;
		case MNU_MAIN:
			switch( lItemID )
			{
				case 0:
					break;
				case 2:
					lResult = m_iTestValue;
					break;
			}
			break;

	}
	return lResult;
}

void CMXMAppMenuHandler::SetValue( long lMenuId, long lItemID, long lValue )
{
	switch( lMenuId )
	{
		case MNU_CFG_NET:
			switch ( lItemID )
			{
				case 6:
					g_MenuInfo.m_pPrefInfo->SetLong( _T("network"), _T("timesynchoffset"), lValue );
					break;
				case 5:
					g_MenuInfo.m_pPrefInfo->SetBool( _T("network"), _T("enablenetwork"), (lValue) );
					break;
				case 1: //	"Enable FTP Server", AMIT_BOOLEAN, 1, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("network"), _T("enableftp"), (lValue) );
					break;
				case 2: //	"FTP Server Priority", AMIT_STRINGLIST, 2, NULL, 0, 0, 30 },
					lValue += 5;
					if ( lValue < 5 )
					{
						lValue = 5;
					}
					if ( lValue > 11 )
					{
						lValue = 11;
					}
					g_MenuInfo.m_pPrefInfo->SetLong( _T("network"), _T("ftppriority"), lValue );
					break;
				case 3: //	"Allow Anon FTP Login", AMIT_BOOLEAN, 3, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("network"), _T("ftpanon"), (lValue) );
					break;
				case 4: //	"Synchronize time using Server", AMIT_BOOLEAN, 4, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("network"), _T("synchtime"), (lValue) );
					break;
			}
			break;
		case MNU_CFG_MISC:
			switch ( lItemID )
			{
				case 1: //	"Saver Delay (min, 0=OFF)", AMIT_VALUE, 1, NULL, 0, 0, 30 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("autocfg"), _T("saverdelay"), lValue );
					g_MenuInfo.m_lSaverDelay = lValue * 60000;
					break;
				case 3: // Load Internal Skins  LoadSkins
					g_MenuInfo.m_pPrefInfo->SetBool( _T("skins"), _T("useinternal"), (lValue) );
					break;
				case 4: // Enable Menu Cache
					g_MenuInfo.m_pPrefInfo->SetBool( _T("menu"), _T("usecache"), (lValue) );
					break;
				case 5: //   "Enable Partition Formatting" 
					g_MenuInfo.m_bAllowFormat = (lValue);
					break;
			}
			break;
		case MNU_CFG_MUSIC:
			switch ( lItemID )
			{
				case 1: //  "Music During Video", AMIT_STRINGLIST, 1, g_szTriState, 0, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("music"), _T("OvMusDurVid"), lValue );
					if ( lValue )
					{
						if ( lValue == 1 ) // Yes 
						{
							g_MenuInfo.Overrides.m_iPreviewSuspendMusic = -1;
						}
						else
						{
							g_MenuInfo.Overrides.m_iPreviewSuspendMusic = 1;
						}
					}
					break;
				case 2: //  "Random Music", AMIT_STRINGLIST, 2, g_szTriState, 0, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("music"), _T("OvMusRandom"), lValue );
					if ( lValue )
					{
						if ( lValue == 1 )
						{
							g_MenuInfo.Overrides.m_bForceRandomMusic = true;
						}
						else
						{
							g_MenuInfo.Overrides.m_bForceRandomMusic = false;
						}
					}
					break;
				case 3: //  "Global Music", AMIT_STRINGLIST, 3, g_szTriState, 0, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("music"), _T("OvMusGlobal"), lValue );
					if ( lValue )
					{
						if ( lValue == 1 )
						{
							g_MenuInfo.Overrides.m_bForceGlobalMusic = true;
							g_MenuInfo.Overrides.m_bForceLocalMusic = false;
						}
						else
						{
							g_MenuInfo.Overrides.m_bForceLocalMusic = true;
							g_MenuInfo.Overrides.m_bForceGlobalMusic = false;
						}
					}
					break;
				case 4: //  "Preview Audio", AMIT_STRINGLIST, 4, g_szTriState, 0, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("audio"), _T("OvPrevAudio"), lValue );
					if ( lValue )
					{
						if ( lValue == 1 )
						{
							g_MenuInfo.Overrides.m_iPreviewEnableSound = 1;
						}
						else
						{
							g_MenuInfo.Overrides.m_iPreviewEnableSound = -1;
						}
					}
					break;
				case 5: //  "Menu Sounds", AMIT_STRINGLIST, 5, g_szMenuSounds, 0, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("audio"), _T("OvMenuSource"), lValue );
					break;
				case 6: //  "Music Volume", AMIT_VALUE, 6, NULL, 0, 0, 63 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("audio"), _T("musicvolume"), lValue );
					GetStateApp()->MusicSetVolume(lValue);
					break;
				case 7: //  "Audio Volume", AMIT_VALUE, 7, NULL, 0, 0, 63 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("audio"), _T("audiovolume"), lValue );
					break;
				case 8: //  "Enable Music", AMIT_BOOLEAN, 8, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("audio"), _T("enablemusic"), (lValue) );
					if ( lValue )
					{
						// Start music, if possible
					}
					else
					{
						// Stop music
					}
					break;
			}
			break;
		case MNU_CFG_AUTOCFG:
			switch ( lItemID )
			{
				case 1: //	"Config Style", AMIT_STRINGLIST, 1, g_szConfigStyle, 0, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("autocfg"), _T("cfgstyle"), lValue );
					break;
				case 2: //	"Add Dashboards", AMIT_BOOLEAN, 2, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("autocfg"), _T("adddash"), (lValue) );
					break;
				case 3: //	"Sort By Title", AMIT_BOOLEAN, 3, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("autocfg"), _T("sort"), (lValue) );
					break;
				case 4: //	"Remove Invalid Entries", AMIT_BOOLEAN, 4, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("autocfg"), _T("removeinvalid"), (lValue) );
					break;
				case 5: //	"Remove Duplicate Entries", AMIT_BOOLEAN, 5, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("autocfg"), _T("removeduplicates"), (lValue) );
					break;
				case 6: //	"AutoCfg Default.xbe only", AMIT_BOOLEAN, 6, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("autocfg"), _T("defaultonly"), (lValue) );
					break;
				case 7: //	"AutoCfg Reject 'Demos'", AMIT_BOOLEAN, 7, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("autocfg"), _T("nodemos"), (lValue) );
					break;
				case 8: // m_bAutoActionMenu
					g_MenuInfo.m_pPrefInfo->SetBool( _T("autocfg"), _T("actionmenu"), (lValue) );
					g_MenuInfo.m_bAutoActionMenu = (lValue);
					break;
			}
			break;
		case MNU_CFG_LOCAL:
			switch ( lItemID )
			{
				case 1: //	"Date Format", AMIT_STRINGLIST, 1, g_szDateFormat, 0, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("local"), _T("dateformat"), (lValue) );
					g_MenuInfo.m_iDateFormat = lValue;
					break;
				case 2: //	"Date Separator", AMIT_STRINGLIST, 2, g_szDateSeparator, 0, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("local"), _T("datesep"), (lValue) );
				    g_MenuInfo.m_tcDateSepChar = g_szDateSeparator[lValue][0];
					break;
				case 3: //	"Time Format", AMIT_STRINGLIST, 3, g_szTimeFormat, 0, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("local"), _T("timeformat"), (lValue) );
					g_MenuInfo.m_iTimeFormat = lValue;
					break;
				case 4: //	"Time Separator", AMIT_STRINGLIST, 4, g_szTimeSeparator, 0, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("local"), _T("timesep"), (lValue) );
				    g_MenuInfo.m_tcTimeSepChar = g_szTimeSeparator[lValue][0];
					break;
				case 5: //	"Temperature", AMIT_STRINGLIST, 5, g_szTempChoices, 0, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("local"), _T("temptype"), (lValue) );
					g_MenuInfo.m_bUseCelsius = (lValue)?true:false;
					break;
				case 6: //	"Euro Numbers", AMIT_BOOLEAN, 6, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("local"), _T("euronumbers"), (lValue) );
					g_MenuInfo.m_bUseEuroNumbers = (lValue)?true:false;
					break;
				case 7: // "Video Standard", AMIT_STRINGLIST, 7, g_szVideoChoices, 0, 0, 0 },
					if ( g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("tvtype"), 0 ) )
					{
						g_MenuInfo.m_pPrefInfo->SetLong( _T("local"), _T("videomode"), lValue );
					}
					break;
				case 8: //  "TV Type", AMIT_STRINGLIST, 8, g_szMonitorChoices, 0, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetLong( _T("local"), _T("tvtype"), lValue );
					if ( lValue )
					{
						GetStateApp()->MessageBox( _T("WARNING!\r\nSetting this to Multistandard enables video mode\r\nswitching, which may cause problems with your system!\r\n\r\nPlease consult the user's guide and heed all warnings!") );
					}
					break;
				case 9:
					g_MenuInfo.m_pPrefInfo->SetBool( _T("local"), _T("force60pal"), (lValue) );
					break;
				case 10:
					g_MenuInfo.m_pPrefInfo->SetLong( _T("local"), _T("region"), lValue );
					break;
			}
			break;
		case MNU_CFG_DVD:
			switch ( lItemID )
			{
				case 1: //	"Launch Movies", AMIT_BOOLEAN, 1, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("dvddrive"), _T("launchmovie"), (lValue) );
					g_MenuInfo.m_bAutoLaunchMovie = (lValue)?true:false;
					break;
				case 2: //	"Launch Games", AMIT_BOOLEAN, 2, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("dvddrive"), _T("launchgame"), (lValue) );
					g_MenuInfo.m_bAutoLaunchGame = (lValue)?true:false;
					break;
				case 3: //	"AutoLoad Multigame", AMIT_BOOLEAN, 3, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("dvddrive"), _T("automulti"), (lValue) );
					g_MenuInfo.m_bAutoLoadMulti = (lValue)?true:false;
					break;
				case 4: //	"Deep Search Multigame", AMIT_BOOLEAN, 4, NULL, CAppMenu::BooleanSetYesNo, 0, 0 },
					g_MenuInfo.m_pPrefInfo->SetBool( _T("dvddrive"), _T("deepsearch"), (lValue) );
					g_MenuInfo.m_bDeepMultiSearch = (lValue)?true:false;
					break;
				case 5: // Preferred player...
					if ( lValue == 1 )
					{
						g_MenuInfo.m_pPrefInfo->SetString( _T("dvddrive"), _T("PrefPlayer"), _T("X2") );
					}
					else if (lValue == 2 )
					{
						g_MenuInfo.m_pPrefInfo->SetString( _T("dvddrive"), _T("PrefPlayer"), _T("M$") );
					}
					else
					{
						g_MenuInfo.m_pPrefInfo->SetString( _T("dvddrive"), _T("PrefPlayer"), _T("ANY") );
					}
					break;
			}
			break;
		case MNU_MAIN:
			switch( lItemID )
			{
				case 0:
					break;
				case 2:
					m_iTestValue = lValue;
					break;
			}
			break;

	}
}


long CMXMAppMenuHandler::GetMinValue( long lMenuId, long lItemID )
{
	long lResult = 0;

	switch( lMenuId )
	{
		case MNU_CFG_MUSIC:
			switch ( lItemID )
			{
				case 1: //  "Music During Video", AMIT_STRINGLIST, 1, g_szTriState, 0, 0, 0 },
					lResult = 0;
					break;
				case 2: //  "Random Music", AMIT_STRINGLIST, 2, g_szTriState, 0, 0, 0 },
					lResult = 0;
					break;
				case 3: //  "Global Music", AMIT_STRINGLIST, 3, g_szTriState, 0, 0, 0 },
					lResult = 0;
					break;
				case 4: //  "Preview Audio", AMIT_STRINGLIST, 4, g_szTriState, 0, 0, 0 },
					lResult = 0;
					break;
				case 5: //  "Menu Sounds", AMIT_STRINGLIST, 5, g_szMenuSounds, 0, 0, 0 },
					lResult = 0;
					break;
			}
			break;
		case MNU_CFG_AUTOCFG:
			break;
		case MNU_CFG_LOCAL:
			break;
		case MNU_CFG_DVD:
			break;
		case MNU_MAIN:
			switch( lItemID )
			{
				case 0:
					break;
				case 2:
					lResult = -20;
					break;
			}
			break;

	}
	return lResult;
}

long CMXMAppMenuHandler::GetMaxValue( long lMenuId, long lItemID )
{
	long lResult = 0;

	switch( lMenuId )
	{
		case MNU_CFG_MUSIC:
			switch ( lItemID )
			{
				case 1: //  "Music During Video", AMIT_STRINGLIST, 1, g_szTriState, 0, 0, 0 },
					lResult = 2;
					break;
				case 2: //  "Random Music", AMIT_STRINGLIST, 2, g_szTriState, 0, 0, 0 },
					lResult = 2;
					break;
				case 3: //  "Global Music", AMIT_STRINGLIST, 3, g_szTriState, 0, 0, 0 },
					lResult = 2;
					break;
				case 4: //  "Preview Audio", AMIT_STRINGLIST, 4, g_szTriState, 0, 0, 0 },
					lResult = 2;
					break;
				case 5: //  "Menu Sounds", AMIT_STRINGLIST, 5, g_szMenuSounds, 0, 0, 0 },
					lResult = 1;
					break;
			}
			break;
		case MNU_CFG_AUTOCFG:
			break;
		case MNU_CFG_LOCAL:
			break;
		case MNU_CFG_DVD:
			break;
		case MNU_MAIN:
			switch( lItemID )
			{
				case 0:
					break;
				case 2:
					lResult = 1000;
					break;
			}
			break;

	}
	return lResult;
}


// MNU_MAIN
// MNU_TITLE
// MNU_TEST
// MNU_SUB_CFG
// MNU_SUB_OTHER
// MNU_DASHBOARD
// MNU_HARDREBOOT


HRESULT CMXMAppMenuHandler::SelectItem( long lMenuID, long lItemID, long lValue, _TAppMenuItem * pMenuItem )
{
	CStdString sTemp;
	CStdString sMode;
	CItemInfo * pItem = NULL;
	HRESULT hr = S_OK;	// S_FALSE will cause an exit of the menus
//	int iIndex;

	DEBUG_FORMAT( _T("SelectItem: MenuID=%d ItemID=%d"),lMenuID, lItemID );

	switch( lMenuID )
	{
		case MNU_CTX_DVD:
		case MNU_CTX_HD:
			{
				CXMLNode * pNode;
				CStdString sScript;
				if ( lMenuID == MNU_CTX_HD )
				{
					pNode = (CXMLNode *)g_HDContextSubMenu[lItemID].m_pParamPtr;
				}
				else
				{
					pNode = (CXMLNode *)g_DVDContextSubMenu[lItemID].m_pParamPtr;
				}
				if ( pNode && (pMenuItem->m_iType == AMIT_SELECTION) )
				{
					// Run the accompanying action...
					sScript = pNode->GetString( NULL, _T("script"), _T(""), true, true );
					if ( sScript.GetLength() )
					{
						sScript = GetInternalScript( sScript );
					}
					else
					{
						sScript = pNode->GetString( NULL, _T("action"), _T(""), true, true );
					}
					if ( sScript.GetLength() )
					{
						CXMLNode * pMenuNode;
						//g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->GetValue( _T("title") );
						//g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->GetValue( _T("desc") );
						//g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->GetValue( _T("dir") );
						//g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->GetValue( _T("exe") );
						//g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->GetValue( _T("thumbnail") );
						pMenuNode = g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->m_pMenuNode;
						//g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->GetValue( _T("thumbnail") );
						SetActionVar( _T("_GameTitle"), pMenuNode->GetString( NULL, _T("title"), _T(""), true ) );
						SetActionVar( _T("_GameDesc"), pMenuNode->GetString( NULL, _T("desc"), _T(""), true ) );
						SetActionVar( _T("_GameDir"), pMenuNode->GetString( NULL, _T("dir"), _T(""), true ) );
						SetActionVar( _T("_GameExe"), pMenuNode->GetString( NULL, _T("exe"), _T(""), true ) );
						SetActionVar( _T("_GameMedia"), pMenuNode->GetString( NULL, _T("media"), _T(""), true ) );
						SetActionVar( _T("_GameThumbnail"), pMenuNode->GetString( NULL, _T("thumbnail"), _T(""), true ) );
						SetActionVar( _T("_GameXBEID"), pMenuNode->GetString( NULL, _T("xbeid"), _T(""), true ) );
						SetActionVar( _T("_GameVerified"), pMenuNode->GetString( NULL, _T("verified"), _T(""), true ) );

						LaunchActionScript( sScript, true );

						SetActionVar( _T("_GameTitle"), NULL );
						SetActionVar( _T("_GameDesc"), NULL );
						SetActionVar( _T("_GameDir"), NULL  );
						SetActionVar( _T("_GameExe"), NULL  );
						SetActionVar( _T("_GameMedia"),  NULL );
						SetActionVar( _T("_GameThumbnail"),  NULL );
						SetActionVar( _T("_GameXBEID"),  NULL );
						SetActionVar( _T("_GameVerified"),  NULL );
					}
					else
					{
						GetStateApp()->MessageBox( _T("Script not found") );
					}
				}
			}
			break;
		case MNU_CFG_MISC:
			g_MenuInfo.SavePrefs();
			hr = S_OK;
			switch( lItemID )
			{
				case 2: // Select Skin
					if ( g_pszSkinsList )
					{
						CStdString sName;
						
						sName = g_pszSkinsList[lValue];

						if ( _tcsicmp( sName, _T("::Random::") ) == 0 )
						{
							sName = _T("$-=random=-$");
							g_MenuInfo.m_pPrefInfo->SetString( _T("UserSetting"), _T("Skin"), sName );
							g_MenuInfo.SavePrefs();
							// g_MenuInfo.m_pPrefInfo->SaveNode( _T("U:\\pref.xml") );
				
						}
						else
						{
							// Select this as our skin!
							g_ImgResCache.Clear();
							g_SndResCache.Clear();
							if ( SelectSkinByName(sName) )
							{
								g_MenuInfo.m_pPrefInfo->SetString( _T("UserSetting"), _T("Skin"), sName );
								g_MenuInfo.SavePrefs();
								// g_MenuInfo.m_pPrefInfo->SaveNode( _T("U:\\pref.xml") );
							}
							GetStateApp()->SetNewSkin();
							GetStateApp()->RebootMessageBox( _T("After selecting a new skin,\r\nyou must reboot") );
						}
					}
					break;
			}
			break;
		case MNU_CFG_MUSIC:
			g_MenuInfo.SavePrefs();
			hr = S_OK;
			break;
		case MNU_CFG_AUTOCFG:
			g_MenuInfo.SavePrefs();
			hr = S_OK;
			break;
		case MNU_CFG_LOCAL:
			g_MenuInfo.SavePrefs();
			hr = S_OK;
			break;
		case MNU_CFG_NET:
			g_MenuInfo.SavePrefs();
			hr = S_OK;
			break;
		case MNU_CFG_DVD:
			g_MenuInfo.SavePrefs();
			hr = S_OK;
			break;
		case MNU_CONFIG:
			switch( lItemID )
			{

				case MNU_CFG_MISC: // Select Skin
					if ( !g_pszSkinsList )
					{
						CStdString sCurrentName;
						int iCurrentSkin = 0;
						sCurrentName = g_MenuInfo.m_pSkinInfo->GetString( NULL, _T("name"), _T("--"), true );
						int iSkinCount = g_SkinNodes.GetNodeCount( _T("skin") );
						if ( iSkinCount )
						{
							g_pszSkinsList = new LPCTSTR[iSkinCount+2];
							if ( g_pszSkinsList )
							{
								CXMLNode * pNode;
								g_pszSkinsList[0] = _T("::Random::");
								int iIndex = 0;
								while( pNode = g_SkinNodes.GetNode( _T("skin"), iIndex ) )
								{
									g_pszSkinsList[iIndex+1] = _tcsdup( pNode->GetString( NULL, _T("name"), _T("unknown"), true ) );
									if ( _tcsicmp( g_pszSkinsList[iIndex], sCurrentName ) == 0 )
									{
										iCurrentSkin = iIndex;
									}
									iIndex++;
								}
								g_pszSkinsList[iIndex+1] = NULL;
							}
						}
						g_MiscPrefsSubMenu[0].m_iParamArg = iCurrentSkin;						
					}
					if ( g_pszSkinsList )
					{
						g_MiscPrefsSubMenu[0].m_pParamPtr = g_pszSkinsList;
					}
					else
					{
						g_MiscPrefsSubMenu[0].m_pParamPtr = NULL;
					}

					//if ( g_pszSkinsList )
					//{
					//	CStdString sName;
					//	
					//	sName = g_pszSkinsList[lValue];

					//	if ( _tcsicmp( sName, _T("::Random::") ) == 0 )
					//	{
					//		sName = _T("$-=random=-$");
					//		g_MenuInfo.m_pPrefInfo->SetString( _T("UserSetting"), _T("Skin"), sName );
					//		g_MenuInfo.SavePrefs();
					//		// g_MenuInfo.m_pPrefInfo->SaveNode( _T("U:\\pref.xml") );
				
					//	}
					//	else
					//	{
					//		// Select this as our skin!
					//		g_ImgResCache.Clear();
					//		g_SndResCache.Clear();
					//		if ( SelectSkinByName(sName) )
					//		{
					//			g_MenuInfo.m_pPrefInfo->SetString( _T("UserSetting"), _T("Skin"), sName );
					//			g_MenuInfo.SavePrefs();
					//			// g_MenuInfo.m_pPrefInfo->SaveNode( _T("U:\\pref.xml") );
					//		}
					//		GetStateApp()->SetNewSkin();
					//		GetStateApp()->RebootMessageBox( _T("After selecting a new skin,\r\nyou must reboot") );
					//	}
					//}
					//break;
			}
			break;
		case MNU_MAIN:
			switch( lItemID )
			{
				case MNU_CONFIG:
					//if ( g_pszSkinsList )
					//{
					//	int iIndex = 0;
					//	while( g_pszSkinsList[iIndex] )
					//	{
					//		delete g_pszSkinsList[iIndex];
					//		iIndex++;
					//	}
					//	delete [] g_pszSkinsList;
					//	g_pszSkinsList = NULL;
					//}
					//if ( !g_pszSkinsList )
					//{
					//	CStdString sCurrentName;
					//	int iCurrentSkin = 0;
					//	sCurrentName = g_MenuInfo.m_pSkinInfo->GetString( NULL, _T("name"), _T("--") );
					//	int iSkinCount = g_SkinNodes.GetNodeCount( _T("skin") );
					//	if ( iSkinCount )
					//	{
					//		g_pszSkinsList = new LPCTSTR[iSkinCount+2];
					//		if ( g_pszSkinsList )
					//		{
					//			CXMLNode * pNode;
					//			g_pszSkinsList[0] = _T("::Random::");
					//			int iIndex = 0;
					//			while( pNode = g_SkinNodes.GetNode( _T("skin"), iIndex ) )
					//			{
					//				g_pszSkinsList[iIndex+1] = _tcsdup( pNode->GetString( NULL, _T("name"), _T("unknown") ) );
					//				if ( _tcsicmp( g_pszSkinsList[iIndex], sCurrentName ) == 0 )
					//				{
					//					iCurrentSkin = iIndex;
					//				}
					//				iIndex++;
					//			}
					//			g_pszSkinsList[iIndex+1] = NULL;
					//		}
					//	}
					//	g_PrefsSubMenu[0].m_iParamArg = iCurrentSkin;						
					//}
					//if ( g_pszSkinsList )
					//{
					//	g_PrefsSubMenu[0].m_pParamPtr = g_pszSkinsList;
					//}
					//else
					//{
					//	g_PrefsSubMenu[0].m_pParamPtr = NULL;
					//}
	//				GetStateApp()->EnterMenu( &g_MenuHandler, g_PrefsSubMenu, MNU_CONFIG );
					break;
				case MNU_INFOSCREEN:
					g_MenuInfo.m_lTitleDelay = (DWORD)(-2);
					GetStateApp()->SetGameState( GS_INFO );
					hr = S_FALSE;
					break;
				case MNU_RSTCACHE:
					sTemp = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("cache_menu.xml") );
					if ( FileExists( sTemp ) )
					{
						DeleteFile( sTemp );
						GetStateApp()->MessageBox( _T("Must reboot to take full effect") );
					}
					break;
				case MNU_SAFEMODE:
					g_MenuInfo.SafeMode.m_bSetUserSafeMode = true;
					g_MenuInfo.UpdateSafeMode( _T("User Requested") );
					break;
				case MNU_CTX_DVD:
//   {	   "Game Options", AMIT_SUBMENU, MNU_CTX_DVD, g_DVDContextSubMenu, 0, 0, 0 },
					{
						CStdString sDir = g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->GetValue( _T("dir") );
						CStdString sExe = g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->GetValue( _T("exe") );
						sDir.MakeUpper();
						if ( sDir.GetLength() && sExe.GetLength() && !(g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->m_bIsMenu&&g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->m_bIsAction))
						{
							if ( sDir.Find( _T("D:\\") ) > -1 )
							{
								if ( g_DVDContextSubMenu[0].m_iType == AMIT_SELECTION )
								{
									pMenuItem->m_pParamPtr = (PVOID)g_DVDContextSubMenu;
									pMenuItem->m_lID = MNU_CTX_DVD;
									pMenuItem->m_iType = AMIT_SUBMENU;
								}
	//							GetStateApp()->EnterMenu( &g_MenuHandler, g_DVDContextSubMenu, MNU_CTX_DVD );
							}
							else
							{
								if ( g_HDContextSubMenu[0].m_iType == AMIT_SELECTION )
								{
									pMenuItem->m_pParamPtr = (PVOID)g_HDContextSubMenu;
									pMenuItem->m_lID = MNU_CTX_HD;
									pMenuItem->m_iType = AMIT_SUBMENU;
								}
	//							GetStateApp()->EnterMenu( &g_MenuHandler, g_HDContextSubMenu, MNU_CTX_HD );
							}
						}
					}
					break;
				case MNU_POWEROFF:
					XK_XBOXPowerOff();
					break;
				case MNU_DASHBOARD:
					{
						DEBUG_LINE("Leaving to dashboard");
						DEBUG_LINE("====================================================");
						Sleep(50);

						g_MenuInfo.m_bSpecialLaunch = true;
						g_MenuInfo.m_sSpecialLaunch = g_MenuInfo.m_sExitApp;
						g_MenuInfo.m_sSpecialLaunchData = g_MenuInfo.m_sExitAppParams;
						GetStateApp()->SetGameState( GS_LOAD );
						hr = S_FALSE;
					}
					break;
				case MNU_WARMREBOOT:
					GetStateApp()->Reboot();
					break;
				case MNU_HARDREBOOT:
					XK_XBOXPowerCycle();
					//GetStateApp()->Reboot();
					//GetStateApp()->MessageBox( _T("Reboot chosen") );
					break;
				case MNU_TITLE:
					g_MenuInfo.m_lTitleDelay = (DWORD)(-2);
					GetStateApp()->SetGameState( GS_TITLE );
					hr = S_FALSE;
					break;
				case MNU_ABOUT:

					if ( g_MenuInfo.m_bDashMode )
					{
						sMode = _T("Dashboard Mode");
					}
					else if ( g_MenuInfo.m_bXDIMode )
					{
						sMode = _T("DVDR XDI Mode");
					}
					else if ( g_MenuInfo.m_bDVDMode ) // g_MenuInfo.m_sActualPath.Find( _T("D:") ) >= 0 )
					{
						sMode = _T("DVDR Mode");
					}
					else
					{
						sMode = _T("Launcher Mode");
					}
					if ( g_MenuInfo.m_bSafeMode )
					{
						sMode += _T(" SAFE MODE");
					}

#ifdef XDEBUG
					sTemp = _T("Media X Menu\r\n Version " VERSION_STRING " (NDEBUG)\r\n" __DATE__ " " __TIME__);
//					GetStateApp()->MessageBox( _T("Media X Menu\r\n Version " VERSION_STRING " (NDEBUG)\r\n" __DATE__ " " __TIME__) );
#else
					sTemp = _T("Media X Menu\r\n Version " VERSION_STRING "\r\n" __DATE__ " " __TIME__);
#endif
					GetStateApp()->MessageBox( _T("%s\r\n%s\r\n%s"), sTemp.c_str(), sMode.c_str(), g_MenuInfo.m_sActualPath.c_str() ); // _T("Media X Menu\r\n Version " VERSION_STRING "\r\n" __DATE__ " " __TIME__) );
					break;
				case MNU_GAMEINFO:
					pItem = g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem);
					if ( pItem )
					{
						if ( pItem->m_bIsMenu )
						{
							GetStateApp()->MessageBox( _T("Current Menu Info:\r\nMedia=%s"),
								pItem->GetValue( szII_Media ).c_str() );
						}
						else if ( pItem->m_bIsAction )
						{
							GetStateApp()->MessageBox( _T("Current Action Info:\r\n%s"),
								pItem->m_sAction.c_str() );
						}
						else
						{
							CStdString sPath(pItem->GetValue( szII_Exe ));
							CStdString sMessage;

							if ( sPath )
							{
								CXBEInfo * pxbeInfo;

								pxbeInfo = _GetXBEInfo( sPath );
								if ( pxbeInfo )
								{
									CStdString sVideoMode;
									CStdString sContentSize;
									CStdString sDirPath;
									sDirPath = StripFilenameFromPath(sPath);
									//XBE Title
									//XBE ID
									//Region Code
									//VideMode setting
									//MD5 Hash
									//Built-in Description and Publisher
									//Path to game
									//Game Directory Size
									//FATX Compatible naming
									if ( pxbeInfo->m_dwRegions & (XBEIMAGE_GAME_REGION_NA|XBEIMAGE_GAME_REGION_JAPAN) )
									{
										sVideoMode = _T("NTSC");
									}
									else if ( pxbeInfo->m_dwRegions & XBEIMAGE_GAME_REGION_RESTOFWORLD )
									{
										sVideoMode = _T("PAL");
									}
									else
									{
										sVideoMode = _T("UNKNOWN");
									}
									if ( sDirPath.GetLength() > 3 )
									{
										ULONGLONG ulValue = (ULONGLONG)DirectorySpace(sDirPath);
										sContentSize = MakeNiceNumber( ulValue, _T(','), false, true );
									}
									else
									{
										sContentSize = _T("Root Directory Item");
									}
									sMessage.Format( _T("Current Game: %s\r\nPath: %s\r\nID: 0x%08x\r\nImage Title: %s\r\nXBE Size: %d\r\nContent Size: %s bytes\r\nVideoMode: %s\r\nMD5 Hash: %s"), pItem->GetValue( szII_Title ).c_str(),
										sPath.c_str(), pxbeInfo->m_dwID, pxbeInfo->m_sTitle.c_str(), pxbeInfo->m_dwSize, sContentSize.c_str(), sVideoMode.c_str(), GetXbeMD5(sPath).c_str() );

								}
								else
								{
									sMessage.Format(_T("Current Game Info:\r\nExe=%s\r\nDir=%s\r\nMedia=%s"),
										pItem->GetValue( szII_Exe ).c_str(), pItem->GetValue( szII_Dir ).c_str(), 
										pItem->GetValue( szII_Media ).c_str() );
									
									// GetStateApp()->MessageBox( 
								}
								GetStateApp()->MessageBox( sMessage );
							}
						}
					}
											
					break;
				case MNU_TEST:
					GetStateApp()->MessageBox( _T("Launch Info:\n\r%s"), g_sLaunchDescr.c_str() );
					break;
			}
			break;
		case MNU_SUB_CFG:
		case MNU_SUB_OTHER:
			switch( lItemID )
			{
				case 0:
					break;
				case 5:
					break;
				case 1:
					GetStateApp()->MessageBox( _T("Selection submenu chosen") );
					break;
			}
			break;
	}

	return hr;
}

HRESULT CMXMAppMenuHandler::PrepareData( long lMenuID, long lItemID, PVOID * pDataPtr )
{
	HRESULT hr = S_OK;

	if ( pDataPtr && (*pDataPtr == NULL) )
	{
	}
	return hr;
}

HRESULT CMXMAppMenuHandler::FrameMove( long lMenuID, long lItemID )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT CMXMAppMenuHandler::Render( long lMenuID, long lItemID )
{
	HRESULT hr = S_OK;

	return hr;
}

int GetGadgetStringID( LPCTSTR szToken );
bool GetGadgetString( int iID, CStdString & sValue, LPCTSTR szSection = NULL, LPCTSTR szKey  = NULL );


// LPSTR g_szLogoName = "MEDIA X MENU";
LPSTR g_szLogoName = "media x menu";


#define GSTR_DISCTITLE				0
#define GSTR_DISCSUBTITLE			1
#define GSTR_CG_TITLE				2
#define GSTR_CG_DESCR				3
#define GSTR_DRV_C_SIZE				4
#define GSTR_DRV_E_SIZE				5
#define GSTR_DRV_F_SIZE				6
#define GSTR_DRV_C_FREE				7
#define GSTR_DRV_E_FREE				8
#define GSTR_DRV_F_FREE				9
#define GSTR_ACTUAL_PATH			10
#define GSTR_IP						11
#define GSTR_TIME					12 
#define GSTR_MXMVERSION				13
#define GSTR_KERNELVERSION			14
#define GSTR_BIOSVERSION			15
#define GSTR_RDNAME					16
#define GSTR_NAME					17
#define GSTR_TEMPSYS				18
#define GSTR_TEMPCPU				19
#define GSTR_DATE					20
#define GSTR_DATETIME				21
#define GSTR_CONFIGVAL				22
#define GSTR_SKINVAL				23
#define GSTR_CG_ENTRYVAL			24
#define GSTR_MAC					25
#define GSTR_XBOXVERS				26
#define GSTR_HDDKEY					27
#define GSTR_SERIALNUM				28
#define GSTR_VIDSTANDARD			29
#define GSTR_SHDRV_C_SIZE			30
#define GSTR_SHDRV_E_SIZE			31
#define GSTR_SHDRV_F_SIZE			32
#define GSTR_SHDRV_C_FREE			33
#define GSTR_SHDRV_E_FREE			34
#define GSTR_SHDRV_F_FREE			35
#define GSTR_TEMPSYSF				36
#define GSTR_TEMPCPUF				37
#define GSTR_TEMPSYSC				38
#define GSTR_TEMPCPUC				39
#define GSTR_BIOSMD5				40
#define GSTR_BIOSNAME				41
#define GSTR_CURRENTSUBMENU			42
#define GSTR_TRAYSTATE				43
#define GSTR_DVDTYPE				44
#define GSTR_CURRENTSONG			45
#define GSTR_CURRENTSOUNDTRACK		46
#define GSTR_NUMFTPUSERS			47
#define GSTR_SUBNETMASK				48
#define GSTR_DEFAULTGATEWAY			49
#define GSTR_DNS1					50
#define GSTR_DNS2					51
#define GSTR_FLASHID				52
#define GSTR_NEWSFEED				53
#define GSTR_LAUNCHMODE				54
#define GSTR_CG_ENTRYVALDW			55
#define GSTR_CG_ENTRYMODE			56
#define GSTR_SHDRV_G_SIZE			57
#define GSTR_SHDRV_G_FREE			58
#define GSTR_DRV_G_SIZE				59
#define GSTR_DRV_G_FREE				60
#define GSTR_FANSPEED				61
#define GSTR_ENV					62

LPCTSTR g_szTokens[] =
{
	_T("disctitle"),
	_T("discsubtitle"),
	_T("cgtitle"),
	_T("cgdescr"),
	_T("drivecsize"),
	_T("driveesize"),
	_T("drivefsize"),
	_T("drivecfree"),
	_T("driveefree"),
	_T("driveffree"),
	_T("actualpath"),
	_T("ip"),
	_T("time"),
	_T("mxmversion"),
	_T("kernelversion"),
	_T("biosversion"),
	_T("rdname"),
	_T("name"),
	_T("tempsys"),
	_T("tempcpu"),
	_T("date"),
	_T("datetime"),
	_T("configvalue"),
	_T("skinvalue"),
	_T("cgentryvalue"),
	_T("mac"),
	_T("xboxversion"),
	_T("hddkey"),
	_T("serialnumber"),
	_T("videostandard"),
	_T("shortdrivecsize"),
	_T("shortdriveesize"),
	_T("shortdrivefsize"),
	_T("shortdrivecfree"),
	_T("shortdriveefree"),
	_T("shortdriveffree"),
	_T("tempsysf"),
	_T("tempcpuf"),
	_T("tempsysc"),
	_T("tempcpuc"),
	_T("biosmd5"),
	_T("biosname"),
	_T("currentsubmenu"),
	_T("traystate"),
	_T("dvdtype"),
	_T("currentsong"),
	_T("currentsoundtrack"),
	_T("numftpusers"),
	_T("subnetmask"),
	_T("defaultgateway"),
	_T("dns1"),
	_T("dns2"),
	_T("flashid"),
	_T("newsfeed"),
	_T("launchmode"),
	_T("cgentryvaluedword"),
	_T("cgentrymode"),
	_T("shortdrivegsize"),
	_T("shortdrivegfree"),
	_T("drivegsize"),
	_T("drivegfree"),
	_T("fanspeed"),
	_T("env"),
	NULL
};

#if 0
CStdString TranslateActionVars( LPCTSTR szArg, TMapStrings * pMapLocal = NULL )
{
	CStdString sReturn;
	if ( pMapLocal == NULL )
	{
		pMapLocal = &g_msLocalEnvironment;
	}

	if ( szArg && _tcslen( szArg ) )
	{
		int iPos = 0, iStart = 0;
		while( szArg[iPos] )
		{
			if ( szArg[iPos] == _T('%') )
			{
				if ( szArg[iPos+1] == _T('%') )
				{
					if ( iStart < iPos )
					{
						sReturn += _T('%');
					}
					iPos += 2;
					iStart = iPos;
				}
				else if ( szArg[iPos+1] == 0 )
				{
					// Just finish...
					iPos++;
				}
				else
				{
					// Add previous portion to return string
					if ( iStart < iPos )
					{
						sReturn += CStdString( &szArg[iStart], (iPos-iStart) );
					}
					iStart = iPos+1;
					iPos++;
					// Now read until next '%'
					while ( szArg[iPos] && szArg[iPos] != _T('%') )
					{
						iPos++;
					}
					if ( szArg[iPos] == _T('%') )
					{
						CStdString sVarName( &szArg[iStart], (iPos-iStart) );
						CStdString sValue;
						sVarName.MakeLower();
						if ( sVarName[0] == _T('_') )
						{
							sVarName = sVarName.Mid(1);
							if ( g_msEnvironment.size() && g_msEnvironment.find( sVarName ) != g_msEnvironment.end() )
							{
								sValue = g_msEnvironment[sVarName];
							}
						}
						else
						{
							if ( pMapLocal->size() && pMapLocal->find( sVarName ) != pMapLocal->end() )
							{
								sValue = (*pMapLocal)[sVarName];
							}
						}
						sReturn += sValue;
						iPos++;
						iStart = iPos;
					}
				}
			}
			else if ( szArg[iPos] == _T('$') )
			{
				if ( szArg[iPos+1] == _T('$') )
				{
					if ( iStart < iPos )
					{
						sReturn += _T('$');
					}
					iPos += 2;
					iStart = iPos;
				}
				else if ( szArg[iPos+1] == 0 )
				{
					// Just finish...
					iPos++;
				}
				else
				{
					// Add previous portion to return string
					if ( iStart < iPos )
					{
						sReturn += CStdString( &szArg[iStart], (iPos-iStart) );
					}
					iStart = iPos+1;
					iPos++;
					// Now read until next '$'
					while ( szArg[iPos] && szArg[iPos] != _T('$') )
					{
						iPos++;
					}
					if ( szArg[iPos] == _T('$') )
					{
						CStdString sVarName( &szArg[iStart], (iPos-iStart) );
						CStdString sValue;
						sVarName.MakeLower();

						if ( _tcscmp( sVarName, _T("eol") ) == 0 )
						{
							sValue = _T("\r\n");
						}
						if ( _tcscmp( sVarName, _T("formatresult") ) == 0 )
						{
							sValue = g_sFormatResult;
						}						
						if ( _tcscmp( sVarName, _T("lasterror") ) == 0 )
						{
							sValue.Format( _T("ERROR CODE: 0x%08X"), GetLastError() );
						}
						if ( _tcscmp( sVarName, _T("dvd_mode") ) == 0 )
						{
							if ( g_MenuInfo.m_bDVDMode )
							{
								sValue = _T("1");
							}
							else
							{
								sValue = _T("0");
							}
						}
						else if ( _tcscmp( sVarName, _T("gt") ) == 0 )
						{
							sValue = _T(">");
						}
						else if ( _tcscmp( sVarName, _T("lt") ) == 0 )
						{
							sValue = _T("<");
						}
						else if ( _tcscmp( sVarName, _T("tab") ) == 0 )
						{
							sValue = _T("\x08");
						}
						else
						{
							int iGstrID = GetGadgetStringID( sVarName );
							if ( iGstrID >= 0 )
							{
								GetGadgetString( iGstrID, sValue );
							}
						}
						sReturn += sValue;
						iPos++;
						iStart = iPos;
					}
				}
			}
			else
			{
				iPos++;
			}
		}
		if ( iStart < iPos && szArg[iStart] )
		{
			sReturn += CStdString( &szArg[iStart], (iPos-iStart) );
		}

	}
	return sReturn;
}
#endif

void AddInternalScript( LPCTSTR szName, LPCTSTR szScript, LPCTSTR szSource, LPCTSTR szPath )
{
	CStdString sReturn;
	CStdString sName( szName );
	CXMLNode * pScriptsNode;
	bool bFoundIt = false;

	sName.MakeLower();
	if ( sName.GetLength() )
	{
		pScriptsNode = g_MenuInfo.m_pInternalInfo->GetNode( _T("scripts") );
		if ( pScriptsNode )
		{
			CXMLElement * pElement;
			int iIndex = 0;

			while( pElement = pScriptsNode->GetElement( _T("actionscript"), iIndex++ ) )
			{
				if ( pElement->GetString( _T(""), _T("name"), true, true ).Compare(sName) == 0 )
				{
					pElement->SetString( szScript );
					bFoundIt = true;
					break;
				}
			}
			if ( !bFoundIt )
			{
				pElement = pScriptsNode->AddElement( _T("actionscript"), szScript );
				if ( pElement )
				{
					pElement->SetString( sName, _T("name") );
				}
			}
		}
	}
}


bool DoesScriptExist( LPCTSTR szName, bool bContext )
{
	bool bReturn = false;
	CStdString sName( szName );
	CXMLNode * pScriptsNode = NULL;

	sName.MakeLower();
	if ( sName.GetLength() )
	{
		if ( bContext )
		{
			CItemInfo * pItem = g_MenuInfo.Entry( g_MenuInfo.m_iCurrentItem );

			if ( pItem )
			{
				pScriptsNode = pItem->m_pMenuNode->GetNode( _T("scripts") );
			}
		}
		else
		{
			pScriptsNode = g_MenuInfo.m_pInternalInfo->GetNode( _T("scripts") );
		}
		if ( pScriptsNode )
		{
			CXMLElement * pElement;
			int iIndex = 0;
			CStdString sScriptName;

			while( pElement = pScriptsNode->GetElement( _T("actionscript"), iIndex++ ) )
			{
				sScriptName = pElement->GetString( _T(""), _T("name"), true, true );
				sScriptName.MakeLower();
				if ( sScriptName.Compare(sName) == 0 )
				{
					bReturn = true;
					break;
				}
			}
		}
	}
	return bReturn;
}

CStdString GetInternalScript( LPCTSTR szName, bool bContext )
{
	CStdString sReturn;
	CStdString sName( szName );
	CXMLNode * pScriptsNode = NULL;

	sName.MakeLower();
	if ( sName.GetLength() )
	{
		if ( bContext )
		{
			CItemInfo * pItem = g_MenuInfo.Entry( g_MenuInfo.m_iCurrentItem );

			if ( pItem )
			{
				pScriptsNode = pItem->m_pMenuNode->GetNode( _T("scripts") );
			}
		}
		else
		{
			pScriptsNode = g_MenuInfo.m_pInternalInfo->GetNode( _T("scripts") );
		}
		if ( pScriptsNode )
		{
			CXMLElement * pElement;
			int iIndex = 0;
			CStdString sScriptName;

			while( pElement = pScriptsNode->GetElement( _T("actionscript"), iIndex++ ) )
			{
				sScriptName = pElement->GetString( _T(""), _T("name"), true, true );
				sScriptName.MakeLower();
				if ( sScriptName.Compare(sName) == 0 )
				{
					sReturn = pElement->GetString( _T("") );
					break;
				}
			}
		}
	}
	return sReturn;
}

#if 0
bool SetActionVar( LPCTSTR szVarName, LPCTSTR szValue, TMapStrings * pMapLocal )
{
	bool bReturn = false;
	bool bLocal = true;
	CStdString sVarName;
	if ( pMapLocal == NULL )
	{
		pMapLocal = &g_msLocalEnvironment;
	}

	if ( szVarName && _tcslen( szVarName ) )
	{
		if ( szVarName[0] == _T('_') )
		{
			bLocal = false;
			szVarName++;
		}
		if ( _tcslen( szVarName ) )
		{
			int iCheck = 0;

			while( szVarName[iCheck] && ( _istalnum( szVarName[iCheck] )|| ( _T('_') == szVarName[iCheck]) ) )
			{
				iCheck++;
			}
			if ( szVarName[iCheck] == 0 ) // Passed check
			{
				sVarName = szVarName;
				sVarName.MakeLower();
				if ( szValue && _tcslen( szValue ) )
				{
					if ( bLocal )
					{
						(*pMapLocal)[sVarName] = szValue;
					}
					else
					{
						g_msEnvironment[sVarName] = szValue;
					}
				}
				else
				{
					// Reset variable - remove from list!
					if ( bLocal )
					{
						(*pMapLocal)[sVarName] = _T("");
					}
					else
					{
						g_msEnvironment[sVarName] = _T("");
					}
				}
				bReturn = true;
			}
		}
	}
	return bReturn;
}

#endif



void SyncInput( void )
{
	CXBoxStateApplication * pApp = GetStateApp();

	pApp->m_bGotInput = false;

	int iRetry = 0;
	
	while( ( pApp->m_bGotInput == false ) && ( iRetry++ < 5 ) )
	{
		Sleep( 25 );
	}
	
}

bool HasInput(XBGAMEPAD* pGamePad )
{
	bool bReturn = false;
	
	SyncInput();

	if ( pGamePad->wPressedButtons )
	{
		bReturn = true;
	}
	else if ( pGamePad->bPressedAnalogButtons[XINPUT_GAMEPAD_A] || pGamePad->bPressedAnalogButtons[XINPUT_GAMEPAD_B] )
	{
		bReturn = true;
	}
	else if ( pGamePad->bPressedAnalogButtons[XINPUT_GAMEPAD_X] || pGamePad->bPressedAnalogButtons[XINPUT_GAMEPAD_Y] )
	{
		bReturn = true;
	}
	else if ( pGamePad->bPressedAnalogButtons[XINPUT_GAMEPAD_BLACK] || pGamePad->bPressedAnalogButtons[XINPUT_GAMEPAD_WHITE] )
	{
		bReturn = true;
	}
	// We ignore triggers....
	return bReturn;
}

void ReadInputToEnv( XBGAMEPAD* pGamePad )
{
	SetActionVar( _T("_GP_A"), NULL );
	SetActionVar( _T("_GP_B"), NULL );
	SetActionVar( _T("_GP_X"), NULL );
	SetActionVar( _T("_GP_Y"), NULL );
	SetActionVar( _T("_GP_BLACK"), NULL );
	SetActionVar( _T("_GP_WHITE"), NULL );
	SetActionVar( _T("_GP_START"), NULL );
	SetActionVar( _T("_GP_BACK"), NULL );
	SetActionVar( _T("_GP_D_UP"), NULL );
	SetActionVar( _T("_GP_D_DN"), NULL );
	SetActionVar( _T("_GP_D_RT"), NULL );
	SetActionVar( _T("_GP_D_LF"), NULL );
	SetActionVar( _T("_GP_TRG_RT"), NULL );
	SetActionVar( _T("_GP_TRG_LT"), NULL );
	SetActionVar( _T("_GP_THUMB_LF"), NULL );
	SetActionVar( _T("_GP_THUMB_RT"), NULL );
	SetActionVar( _T("_GP_ANYBUTTON"), NULL );

	bool bAnyButton = false;

	if ( pGamePad->wPressedButtons )
	{
		if ( pGamePad->wPressedButtons&XINPUT_GAMEPAD_DPAD_UP )
		{
			SetActionVar( _T("_GP_D_UP"), _T("1") );
			bAnyButton = true;
		}
		if ( pGamePad->wPressedButtons&XINPUT_GAMEPAD_DPAD_DOWN )
		{
			SetActionVar( _T("_GP_D_DN"), _T("1") );
			bAnyButton = true;
		}
		if ( pGamePad->wPressedButtons&XINPUT_GAMEPAD_DPAD_LEFT )
		{
			SetActionVar( _T("_GP_D_LF"), _T("1") );
			bAnyButton = true;
		}
		if ( pGamePad->wPressedButtons&XINPUT_GAMEPAD_DPAD_RIGHT )
		{
			SetActionVar( _T("_GP_D_RT"), _T("1") );
			bAnyButton = true;
		}
		if ( pGamePad->wPressedButtons&XINPUT_GAMEPAD_START )
		{
			SetActionVar( _T("_GP_START"), _T("1") );
			bAnyButton = true;
		}
		if ( pGamePad->wPressedButtons&XINPUT_GAMEPAD_BACK )
		{
			SetActionVar( _T("_GP_BACK"), _T("1") );
			bAnyButton = true;
		}
		if ( pGamePad->wPressedButtons&XINPUT_GAMEPAD_LEFT_THUMB )
		{
			SetActionVar( _T("_GP_THUMB_LF"), _T("1") );
			bAnyButton = true;
		}
		if ( pGamePad->wPressedButtons&XINPUT_GAMEPAD_RIGHT_THUMB )
		{
			SetActionVar( _T("_GP_THUMB_RT"), _T("1") );
			bAnyButton = true;
		}
	}
    if ( pGamePad->bPressedAnalogButtons[XINPUT_GAMEPAD_A] )
	{
		SetActionVar( _T("_GP_A"), _T("1") );
		bAnyButton = true;
	}
	if ( pGamePad->bPressedAnalogButtons[XINPUT_GAMEPAD_B] )
	{
		SetActionVar( _T("_GP_B"), _T("1") );
		bAnyButton = true;
	}
	if ( pGamePad->bPressedAnalogButtons[XINPUT_GAMEPAD_X] )
	{
		SetActionVar( _T("_GP_X"), _T("1") );
		bAnyButton = true;
	}
	if ( pGamePad->bPressedAnalogButtons[XINPUT_GAMEPAD_Y] )
	{
		SetActionVar( _T("_GP_Y"), _T("1") );
		bAnyButton = true;
	}
	if ( pGamePad->bPressedAnalogButtons[XINPUT_GAMEPAD_BLACK] )
	{
		SetActionVar( _T("_GP_BLACK"), _T("1") );
		bAnyButton = true;
	}
	if ( pGamePad->bPressedAnalogButtons[XINPUT_GAMEPAD_WHITE] )
	{
		SetActionVar( _T("_GP_WHITE"), _T("1") );
		bAnyButton = true;
	}
	if ( pGamePad->bLastAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER] )
	{
		SetActionVar( _T("_GP_TRG_LF"), _T("1") );
	}
	if ( pGamePad->bLastAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] )
	{
		SetActionVar( _T("_GP_TRG_RT"), _T("1") );
	}
	if ( bAnyButton )
	{
		SetActionVar( _T("_GP_ANYBUTTON"), _T("1") );
	}
	SyncInput();
}



class CMXMActionScript : public CActionScript
{
protected:
	virtual CStdString	HandleFunction( LPCTSTR szFunc, int iFirstArg, int * piLastArg );
	virtual int		ExecuteLine( LPCTSTR szCommand );
	virtual CActionScript * CreateActionScript( LPCTSTR szScript );
	virtual	void	ContextInit( void );
	virtual	void	ContextExit( void );
public:
	CMXMActionScript( LPCTSTR szActionScript, CActionScript * pParent, bool bIsContext = false ) :
	  CActionScript( szActionScript, pParent, bIsContext ) {};
};


// new CActionScript( szScript, NULL, bContext );
CActionScript * GetNewAppActionScript( LPCTSTR szActionScript, CActionScript * pParent, bool bIsContext )
{
	return new CMXMActionScript( szActionScript, pParent, bIsContext );
}


struct TActionLaunchData
{
	CStdString m_sActionProg;
	bool m_bContext;
};

void LaunchActionScript( LPCTSTR szAction, bool bContext )
{
//	HANDLE hThread;
	ClearActionDrawList( g_MenuInfo.m_drawList );
	GetStateApp()->m_pdrawList = &g_MenuInfo.m_drawList;

#if 1
	if ( !g_bSchedulerInit )
	{
		InitializeActionScriptScheduler();
	}
	ScheduleScript( szAction, 0, 0, false, bContext );
#else
	TActionLaunchData * plaunchData = new TActionLaunchData;


	if ( !g_MenuInfo.m_bInActionScript )
	{
		if ( plaunchData )
		{
			ClearActionDrawList( g_MenuInfo.m_drawList );
			plaunchData->m_sActionProg = szAction;
			plaunchData->m_bContext = bContext;
			hThread = CreateThread( NULL, (1024*96), (LPTHREAD_START_ROUTINE)ActionScriptThreadHandlerProc, plaunchData, 0, NULL );
			if ( hThread != INVALID_HANDLE_VALUE )
			{
				
			}
		}
	}
#endif
}


CActionScript * CMXMActionScript::CreateActionScript( LPCTSTR szScript )
{
	CActionScript * pReturn;

	pReturn = new CMXMActionScript( szScript, this );
	return pReturn;
}


void	CMXMActionScript::ContextInit( void )
{
	CXMLNode * pMenuNode = NULL;
	CItemInfo * pItemInfo = g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem);

	if ( pItemInfo )
	{
		pMenuNode = pItemInfo->m_pMenuNode;
	}
	if ( pMenuNode )
	{
		SetActionVar( _T("_GameTitle"), pMenuNode->GetString( NULL, _T("title"), _T(""), true ) );
		SetActionVar( _T("_GameDesc"), pMenuNode->GetString( NULL, _T("desc"), _T(""), true ) );
		SetActionVar( _T("_GameDir"), pMenuNode->GetString( NULL, _T("dir"), _T(""), true ) );
		SetActionVar( _T("_GameExe"), pMenuNode->GetString( NULL, _T("exe"), _T(""), true ) );
		SetActionVar( _T("_GameMedia"), pMenuNode->GetString( NULL, _T("media"), _T(""), true ) );
		SetActionVar( _T("_GameThumbnail"), pMenuNode->GetString( NULL, _T("thumbnail"), _T(""), true ) );
		SetActionVar( _T("_GameXBEID"), pMenuNode->GetString( NULL, _T("xbeid"), _T(""), true ) );
		SetActionVar( _T("_GameVerified"), pMenuNode->GetString( NULL, _T("verified"), _T(""), true ) );
	}
}


void	CMXMActionScript::ContextExit( void )
{
	SetActionVar( _T("_GameTitle"), NULL );
	SetActionVar( _T("_GameDesc"), NULL );
	SetActionVar( _T("_GameDir"), NULL  );
	SetActionVar( _T("_GameExe"), NULL  );
	SetActionVar( _T("_GameMedia"),  NULL );
	SetActionVar( _T("_GameThumbnail"),  NULL );
	SetActionVar( _T("_GameXBEID"),  NULL );
	SetActionVar( _T("_GameVerified"),  NULL );
}

int		CMXMActionScript::ExecuteLine( LPCTSTR szCommand )
{
	int iResult = ASRC_NOTIMPL;
	CStdString sResponse;
	HRESULT hr;

	if ( _tcsicmp( szCommand, _T("loadcurrent") ) == 0 )
	{
		g_MenuInfo.m_bSpecialLaunch = false;
		GetStateApp()->SetGameState( GS_LOAD );
		iResult = ASRC_OK;
	}
	else
	{
		hr = ProcessScriptCommand( m_sCurrLine, sResponse, NULL, PSC_GUI, &m_msLocalEnvironment );
		if ( SUCCEEDED( hr ) )
		{
			iResult = ASRC_OK;
		}
		else
		{
			if ( hr == E_NOTIMPL )
			{
				SetError( _T("Command not implemented") );
			}
			else
			{
				SetError( sResponse );
			}
		}
	}

	return iResult;
}

CStdString CMXMActionScript::HandleFunction(LPCTSTR szFunc, int iFirstArg, int * piLastArg )
{
	CStdString sReturn;

	if ( ( _tcsicmp( szFunc, _T("condition") ) == 0 ) || ( _tcsicmp( szFunc, _T("cond") ) == 0 ) )
	{
		CStdString sCond = GetArgument( iFirstArg++ );
		int iCond = GetConditionID(sCond);

		if ( IsConditionMet(iCond, false ) )
		{
			sReturn = _T("1");
		}
		else
		{
			sReturn = _T("0");
		}
	}
	else if ( ( _tcsicmp( szFunc, _T("!condition") ) == 0 ) || ( _tcsicmp( szFunc, _T("!cond") ) == 0 ) )
	{
		CStdString sCond = GetArgument( iFirstArg++ );
		sCond.MakeLower();
		int iCond = GetConditionID(sCond);

		if ( IsConditionMet(iCond, true ) )
		{
			sReturn = _T("1");
		}
		else
		{
			sReturn = _T("0");
		}
	}
	else
	{
		sReturn = CActionScript::HandleFunction( szFunc, iFirstArg, piLastArg );
	}
	return sReturn;
}


DWORD ActionScriptThreadHandlerProc( LPVOID pParam )
{
	TActionLaunchData * plaunchData = (TActionLaunchData *)pParam;
	DWORD dwReturn = S_OK;

	if ( plaunchData )
	{
		CMXMActionScript * pActionScript = new CMXMActionScript( plaunchData->m_sActionProg, NULL, plaunchData->m_bContext );

		if ( pActionScript )
		{
			g_MenuInfo.m_bInActionScript = true;
			GetStateApp()->m_pdrawList = &g_MenuInfo.m_drawList;
			GetStateApp()->m_bInActionScript = true;
			pActionScript->ExecuteScript();
	//		dwReturn = DoActionScript( plaunchData->m_sActionProg, true, NULL, &g_MenuInfo.m_drawList );
			GetStateApp()->m_bInActionScript = false;
			g_MenuInfo.m_bInActionScript = false;
			delete pActionScript;
		}
		delete plaunchData;
	}
	return dwReturn;
}

#if 0
class CActionScriptx
{
public:
	CActionScriptx( LPCTSTR szProgram );
	~CActionScriptx();
	HRESULT DoActionScript( void );
	TListStrings	m_lsProgram;
	TMapLabels		m_mlLabels;
	TMapStrings		m_msLocalEnvironment;
	TMapFileData	m_fileData;
	TXMLMap *		m_pXMLMap;
	TActionDrawList * m_pactionDrawList;
	CStdString		m_sResponse;
};

CStdString ScanProgram( LPCTSTR szAction, TMapLabels & mlLabels, TListStrings	& lsProgram, TMapStrings * pmsLocalEnv )
{
	CStdString sAction(szAction);
	CStdString sLine, sLabel;
	int iPos = 0, iLen = 0, iLine = 0;
	bool bContinue = true;
	CStdString sError;

	lsProgram.clear();

	sAction.Trim();
	if ( szAction && sAction.GetLength() )
	{
		sAction.Replace( _T("\t"), _T(" ") );
		sAction.Replace( _T("\r\n"), _T("\x01") );
		sAction.Replace( _T("\n\r"), _T("\x01") );
		sAction.Replace( _T("\n"), _T("\x01") );
		sAction.Replace( _T("\r"), _T("\x01") );
		sAction.Replace( _T("\x01"), _T("\r") );
		sAction.Trim();
		iPos = 0;
		iLen = sAction.Find( _T('\r'), 0 );
		if ( iLen == -1 )
		{
			iLen = sAction.GetLength();
		}
		iLen -= iPos;
		while( (iPos < sAction.GetLength()) )
		{
			if ( iLen )
			{
				sLine = sAction.Mid( iPos, iLen );
				sLine.Trim();
				if( sLine.GetLength() )
				{
					// Look for conditionals or skip to label
					if ( sLine[0] == _T(':') ) 
					{
						// Label! Mark it and continue
						sLabel = TranslateActionVars(GetArg( sLine.Mid(1), 0 ), pmsLocalEnv );
						sLabel.MakeLower();
						// Save off the position to the next program line in the program list
						// and associate it with the label.
						if ( mlLabels.find(sLabel) == mlLabels.end() )
						{
							mlLabels[sLabel] = (int)(lsProgram.size());
						}
						else
						{
							// Label already exists!!
							bContinue = false;
							sError.Format( _T("ERROR: (%d) Label \'%s\' already exists"), iLine, sLabel.c_str() );
						}
					}
					else if ( ( sLine[0] == _T(';') ) || ( sLine[0] == _T('#') ) || ( sLine[0] == _T('\'') )  )
					{
						// Comment. Ignore this line!
					}
					else if ( ( _tcsnicmp( sLine, _T("rem "), 4 ) == 0 ) || ( _tcsnicmp( sLine, _T("// "), 3 ) == 0 )  )
					{
						// Comment. Ignore this line!
					}
					else
					{
						lsProgram.push_back( sLine );
					}
				}
			}
			iPos += iLen;
			iPos++;
			if ( iPos < sAction.GetLength() )
			{
				iLen = sAction.Find( _T('\r'), iPos );
				if ( iLen == -1 )
				{
					iLen = sAction.GetLength();
				}
				iLen -= iPos;
			}
		}
		iLine++;
	}
	return sError;
}

HRESULT DoActionScript( LPCTSTR szAction, bool bClearLocal, TMapStrings	* pmsLocalEnv, TActionDrawList * pactionDrawList, TXMLMap * pXMLMap )
{
	// Parse each line and execute it...
	CStdString sLine, sResponse, sLabel, sError, sSearchPath;
	TListStrings lsProgram;
	TListStrings::iterator iterProgram;
	HRESULT hr = S_OK;
	TMapLabels mlLabels;
	int iLine = 0;
	HANDLE hFind;
	WIN32_FIND_DATA ffData;
	CStdString sWildcard;
	bool m_bBeginDraw = false;
//	TActionDrawState actionDrawState;
	TMapStrings		* pmsLocalEnvironment;
	TMapFileData	fileData;
	bool bDeleteDrawList = false;
	bool bDeleteXMLMap = false;

	if ( pactionDrawList == NULL )
	{
		bDeleteDrawList = true;
		pactionDrawList = new TActionDrawList;
	}
	if ( pXMLMap == NULL )
	{
		pXMLMap = new TXMLMap;
		bDeleteXMLMap = true;
	}
	

	hFind = NULL;
	int iPos, iLen;
	bool bContinue = true;

	if ( g_sPSCCurDir.GetLength() == 0 )
	{
		g_sPSCCurDir = g_MenuInfo.m_sMXMPath;
	}

	// msLocalEnvironment.clear();
	pmsLocalEnvironment = new TMapStrings;
	pmsLocalEnvironment->clear();

	if ( bClearLocal )
	{
//		if ( g_ActionDrawState.m_pBackTexture )
//		{
//			g_ActionDrawState.m_pBackTexture->Release();
//			g_ActionDrawState.m_pBackTexture = NULL;
//		}
	}
	else
	{
		// Copy previous environment!!
		if ( pmsLocalEnv )
		{
			*pmsLocalEnvironment = (*pmsLocalEnv);
		}
	}

	pmsLocalEnv = pmsLocalEnvironment;

	// g_sPSCCurDir

	sError = ScanProgram( szAction, mlLabels, lsProgram, pmsLocalEnv );

	if ( sError.GetLength() )
	{
		bContinue = false;
	}

	iterProgram = lsProgram.begin();

	while( iterProgram != lsProgram.end() && bContinue )
	{
		sLine = iterProgram->c_str();
		if( sLine.GetLength() )
		{
			// Look for conditionals or skip to label
			if ( _tcsnicmp( sLine, _T("goto "), 5 ) == 0 )
			{
				// GOTO - find label and proceed to it
				CStdString sArg;
				TMapLabels::iterator iterLabel;

				sArg = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				sArg.MakeLower();
				if ( sArg[0] == _T(':') )
				{
					sArg = sArg.Mid(1);
				}
				if ( sArg.GetLength() && ((iterLabel = mlLabels.find( sArg ) ) != mlLabels.end()) )
				{
					iLine = mlLabels[sArg];
					// Found it
					iterProgram = lsProgram.begin();
					while( iLine && (iterProgram != lsProgram.end()))
					{
						iterProgram++;
						iLine--;
					}
				}
				else
				{
					// Program error.... could not find label
					// Automatic "quit"
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Label \'%s\' not found"), sLabel.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("openread "), 9 ) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sFilename = MakeFullFilePath( g_sPSCCurDir, TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv ) );


				CMXMFileData * pFileData = CreateFileData( fileData, sHandle );
				if ( pFileData )
				{
					pFileData->m_hFileHandle = CreateFile( sFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
					if ( pFileData->m_hFileHandle && pFileData->m_hFileHandle != INVALID_HANDLE_VALUE )
					{
						pFileData->m_bRead = true;
						iterProgram++;
					}
					else
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: (OpenRead) Error opening file") );
					}
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: (OpenRead) Error creating handle") );
				}
			}
			else if ( _tcsnicmp( sLine, _T("openwrite "), 10 ) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sFilename = MakeFullFilePath( g_sPSCCurDir, TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv ) );


				CMXMFileData * pFileData = CreateFileData( fileData, sHandle );
				if ( pFileData )
				{
					pFileData->m_hFileHandle = CreateFile( sFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
					if ( pFileData->m_hFileHandle && pFileData->m_hFileHandle != INVALID_HANDLE_VALUE )
					{
						pFileData->m_bRead = false;
						iterProgram++;
					}
					else
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: (OpenWrite) Error opening file") );
					}
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: (OpenWrite) Error creating handle") );
				}
			}
			else if ( _tcsnicmp( sLine, _T("readfileln "), 11 ) == 0 )
			{
				CStdString sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				CStdString sArg2 = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );
				CMXMFileData * pFileData = GetFileData( fileData, sArg1 );

				if ( pFileData )
				{
					if ( pFileData->m_hFileHandle && pFileData->m_bRead )
					{
//int ReadFileLine( HANDLE hFile, char * szBuffer, int iMaxChars );
//int WriteFileLine( HANDLE hFile, LPCSTR szString );
						CStdString sValue = ReadFileLine( pFileData->m_hFileHandle );
						
						if ( !SetActionVar( sArg2, sValue, pmsLocalEnv ) )
						{
							iterProgram = lsProgram.end();
							bContinue = false;
							sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sArg1.c_str(), sArg2.c_str() );
						}
						else
						{
							iterProgram++;
						}
					}
					else
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: (ReadFile) Unable to read file (%s)"), sArg1.c_str() );
					}
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: (ReadFile) Handle does not exist (%s)"), sArg1.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("writefile"), 9 ) == 0 )
			{
				bool bWriteEOL = false;
				if ( _toupper(sLine[9]) == _T('L') )
				{
					bWriteEOL = true;
				}
				CStdString sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				CStdString sArg2 = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );
				CMXMFileData * pFileData = GetFileData( fileData, sArg1 );

				if ( pFileData )
				{
					if ( pFileData->m_hFileHandle && !pFileData->m_bRead )
					{
						WriteFileLine( pFileData->m_hFileHandle, sArg2, true );
						iterProgram++;
					}
					else
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: (WriteFile) Unable to write file (%s)"), sArg1.c_str() );
					}
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: (WriteFile) Handle does not exist (%s)"), sArg1.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("closefile "), 10 ) == 0 )
			{
				CStdString sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );

				if ( DeleteFileData( fileData, sArg1 ) )
				{
					iterProgram++;
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: (CloseFile) Unable to delete handle (%s)"), sArg1.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("filestat "), 9 ) == 0 )
			{
				CStdString sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				CStdString sVarName = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );
				CStdString sValue;
				CMXMFileData * pFileData = GetFileData( fileData, sArg1 );

				if ( pFileData )
				{
					if ( pFileData->m_hFileHandle )
					{
						DWORD dwValueLow = 0, dwValueHigh = 0;
						if ( ((dwValueLow=SetFilePointer( pFileData->m_hFileHandle, 0, (PLONG)&dwValueHigh, FILE_CURRENT )) == INVALID_SET_FILE_POINTER) && (GetLastError()!=NO_ERROR) )
						{
							// Error!
							sValue = _T("");
						}
						else
						{
							if ( pFileData->m_bRead ) 
							{
								DWORD dwSizeLow = 0, dwSizeHigh = 0;
								if ( ( (dwSizeLow = GetFileSize( pFileData->m_hFileHandle, &dwSizeHigh )) == dwValueLow ) && ( dwSizeHigh == dwValueHigh ))
								{
									// End of file
									sValue = _T("0");
								}
								else
								{
									// In file...
									sValue = _T("1");
									// Have to reset file pointer!!
									SetFilePointer( pFileData->m_hFileHandle, dwValueLow, (PLONG)&dwValueHigh, FILE_BEGIN );
								}
							}
							else
							{
								// In file...
								sValue = _T("1");
							}
						}
					}
					else
					{
						// Doesn't exist. Return "NULL"
						sValue = _T("");
					}
				}
				else
				{
					// Doesn't exist. Return "NULL"
					sValue = _T("");
				}
				if ( !SetActionVar( sVarName, sValue, pmsLocalEnv ) )
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sVarName.c_str(), sValue.c_str() );
				}
				else
				{
					iterProgram++;
				}
			}
			else if ( _tcsnicmp( sLine, _T("set "), 4 ) == 0 )
			{
				CStdString sArg1, sArg2;

				sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				sArg2 = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );
				if ( !SetActionVar( sArg1, sArg2, pmsLocalEnv ) )
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sArg1.c_str(), sArg2.c_str() );
				}
				else
				{
					iterProgram++;
				}
			}
			else if ( _tcsnicmp( sLine, _T("setfunc "), 8 ) == 0 )
			{
				CStdString sArg1, sArg2, sValue;

				sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				// Function name...
				sArg2 = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );

				if ( _tcsicmp( sArg2, _T("fileexists") ) == 0 )
				{
					CStdString sFilename;

					sFilename = MakeFullFilePath( g_sPSCCurDir, TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv ) );
					if ( FileExists( sFilename ) )
					{
						sValue = _T("1");
					}
					else
					{
						sValue = _T("0");
					}
				}
				else if ( _tcsicmp( sArg2, _T("filesize") ) == 0 )
				{
					CStdString sFilename;

					sFilename = MakeFullFilePath( g_sPSCCurDir, TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv ) );
					if ( FileExists( sFilename ) )
					{						
						DWORD dwFileSize = FileLength( sFilename );
						if ( dwFileSize )
						{
							sValue.Format( _T("%d"), dwFileSize );
						}
						else
						{
							ULONGLONG u64DirSize;

							u64DirSize = DirectorySpace( sFilename );
							sValue.Format( _T("%I64d"), u64DirSize );
						}
					}
					else
					{
						sValue = _T("0");
					}
				}
				else if ( _tcsicmp( sArg2, _T("pathempty") ) == 0 )
				{
					CStdString sPath;
					bool bHasFiles;
					HANDLE hFindPath;
					WIN32_FIND_DATA ffDataPath;


					sPath = PathSlasher( MakeFullDirPath( g_sPSCCurDir, TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv ) ), true );
					sPath += _T("*");

					hFindPath = FindFirstFile( sPath, &ffDataPath );
					if ( INVALID_HANDLE_VALUE != hFindPath )
					{
						// Don't care WHAT we found... just that we found it.
						FindClose( hFindPath );
						sValue = _T("0");
					}
					else
					{
						sValue = _T("1");
					}
				}
				else if ( _tcsicmp( sArg2, _T("xbeid") ) == 0 )
				{
					CStdString sFilename;

					sFilename = MakeFullFilePath( g_sPSCCurDir, TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv ) );
					if ( FileExists( sFilename ) )
					{						
						sValue.Format( _T("%d"), GetXbeID( sFilename ) );
					}
					else
					{
						sValue = _T("0");
					}
				}
				else if ( _tcsicmp( sArg2, _T("left") ) == 0 )
				{
					CStdString sCount = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );
					CStdString sString = TranslateActionVars(GetArg( sLine, 4 ), pmsLocalEnv );
					int iCount = _tcstoul( sCount, NULL, 10 );

					if ( sCount.GetLength() )
					{
						sValue = sString.Left(iCount);
					}
					else
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: Syntax error on SETFUNC LEFT") );
					}
				}
				// int GetConditionID( LPCTSTR szName );
				// bool IsConditionMet( int iID, bool bNotCondition );
				else if ( ( _tcsicmp( sArg2, _T("condition") ) == 0 ) || ( _tcsicmp( sArg2, _T("cond") ) == 0 ) )
				{
					CStdString sCond = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );
					int iCond = GetConditionID(sCond);

					if ( IsConditionMet(iCond, false ) )
					{
						sValue = _T("1");
					}
					else
					{
						sValue = _T("0");
					}
				}
				else if ( ( _tcsicmp( sArg2, _T("!condition") ) == 0 ) || ( _tcsicmp( sArg2, _T("!cond") ) == 0 ) )
				{
					CStdString sCond = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );
					sCond.MakeLower();
					int iCond = GetConditionID(sCond);

					if ( IsConditionMet(iCond, true ) )
					{
						sValue = _T("1");
					}
					else
					{
						sValue = _T("0");
					}
				}
				else if ( _tcsicmp( sArg2, _T("right") ) == 0 )
				{
					CStdString sCount = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );
					CStdString sString = TranslateActionVars(GetArg( sLine, 4 ), pmsLocalEnv );
					int iCount = _tcstoul( sCount, NULL, 10 );

					if ( sCount.GetLength() )
					{
						sValue = sString.Right(iCount);
					}
					else
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: Syntax error on SETFUNC RIGHT") );
					}
				}
				else if ( _tcsicmp( sArg2, _T("mid") ) == 0 )
				{
					CStdString sCount = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );
					CStdString sLength = TranslateActionVars(GetArg( sLine, 4 ), pmsLocalEnv );
					CStdString sString = TranslateActionVars(GetArg( sLine, 5 ), pmsLocalEnv );
					int iCount = _tcstoul( sCount, NULL, 10 );
					int iLength = _tcstol( sLength, NULL, 10 );

					if ( sCount.GetLength() && sLength.GetLength() )
					{
						sValue = sString.Mid( iCount, iLength );
					}
					else
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: Syntax error on SETFUNC MID") );
					}
				}
				else if ( _tcsicmp( sArg2, _T("space") ) == 0 )
				{
					CStdString sCount = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );
					int iCount = _tcstoul( sCount, NULL, 10 );

					if ( iCount )
					{
						sValue.Format( _T("%*s"), iCount, _T("") );
					}
					else
					{
						sValue = _T("");
					}
				}
				else if ( _tcsicmp( sArg2, _T("length") ) == 0 )
				{
					CStdString sText = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );

					sValue.Format( _T("%d"), sText.GetLength() );
				}
				else if ( _tcsicmp( sArg2, _T("trim") ) == 0 )
				{
					CStdString sText = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );

					sText.Trim();
					sValue = sText;
				}
				else if ( _tcsicmp( sArg2, _T("trimleft") ) == 0 )
				{
					CStdString sText = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );

					sText.TrimLeft();
					sValue = sText;
				}
				else if ( _tcsicmp( sArg2, _T("trimright") ) == 0 )
				{
					CStdString sText = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );

					sText.TrimRight();
					sValue = sText;
				}
				else if ( _tcsicmp( sArg2, _T("lower") ) == 0 )
				{
					CStdString sText = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );

					sText.MakeLower();
					sValue = sText;
				}
				else if ( _tcsicmp( sArg2, _T("upper") ) == 0 )
				{
					CStdString sText = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );

					sText.MakeUpper();
					sValue = sText;
				}
				else if ( _tcsicmp( sArg2, _T("tempfile") ) == 0 )
				{
					CStdString sPath = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );
					CStdString sPrefix = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );

					sValue = GetTempFileName( sPath, sPrefix );
				}
				else if ( _tcsicmp( sArg2, _T("fatxify") ) == 0 )
				{
					CStdString sText = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );
					CStdString sReplace = TranslateActionVars(GetArg( sLine, 4 ), pmsLocalEnv );

					// sText.MakeUpper();
					sValue = FatXify( sText, false, sReplace.GetLength()?sReplace[0]:0 );
				}
				else if ( _tcsicmp( sArg2, _T("repeat") ) == 0 )
				{
					CStdString sCount = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );
					CStdString sRepeat = TranslateActionVars(GetArg( sLine, 4 ), pmsLocalEnv );
					int iCount = _tcstoul( sCount, NULL, 10 );

					sValue = _T("");
					if ( iCount )
					{
						while( iCount-- )
						{
							sValue += sRepeat;
						}
					}
				}
				else if ( _tcsicmp( sArg2, _T("fnmerge") ) == 0 )
				{
					CStdString sPath, sBasename, sExt;
					CStdString sFilename;

					sPath =  MakeFullDirPath( g_sPSCCurDir, TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv ) );
					sFilename = TranslateActionVars(GetArg( sLine, 4 ), pmsLocalEnv );
					sExt = TranslateActionVars(GetArg( sLine, 5 ), pmsLocalEnv );
					if ( sExt.GetLength() )
					{
						sFilename = ReplaceFilenameExtension( sFilename, sExt );
					}
					sValue = MakeFullFilePath( sPath, sFilename );
				}
				else if ( _tcsicmp( sArg2, _T("fnpath") ) == 0 )
				{
					CStdString sFilename;

					sFilename = MakeFullFilePath( g_sPSCCurDir, TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv ) );
					sValue = StripFilenameFromPath( sFilename );
				}
				else if ( _tcsicmp( sArg2, _T("fnbasename") ) == 0 )
				{
					CStdString sFilename;

					sFilename = MakeFullFilePath( g_sPSCCurDir, TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv ) );
					sFilename = ExtractFilenameFromPath( sFilename, false );
					sValue = GetFilenameBase( sFilename );
				}
				else if ( _tcsicmp( sArg2, _T("fnextension") ) == 0 )
				{
					CStdString sFilename;

					sFilename = MakeFullFilePath( g_sPSCCurDir, TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv ) );
					sFilename = ExtractFilenameFromPath( sFilename, false );
					sValue = GetFilenameExtension( sFilename );
				}
				else if ( _tcsicmp( sArg2, _T("xbetitle") ) == 0 )
				{
					CStdString sFilename;

					sFilename = MakeFullFilePath( g_sPSCCurDir, TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv ) );
					if ( FileExists( sFilename ) )
					{						
						sValue = GetXbeTitle( sFilename );
					}
					else
					{
						sValue = _T("");
					}
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: (setfunc) Unknown function %s"), sArg2.c_str() );
				}
				if ( bContinue )
				{
					if ( !SetActionVar( sArg1, sValue, pmsLocalEnv ) )
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sArg1.c_str(), sArg2.c_str() );
					}
					else
					{
						iterProgram++;
					}
				}
			}
			else if ( _tcsnicmp( sLine, _T("add "), 4 ) == 0 )
			{
				CStdString sArg1, sArg2,sArg3;
				int iValue, iAdd;

				sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				sArg2 = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );

				// Get value...
				sArg3.Format( _T("%%%s%%"), sArg1.c_str() );
				sArg3 = TranslateActionVars(sArg3, pmsLocalEnv );

				iValue = _tcstol( sArg3, NULL, 10 );
				iAdd = _tcstol( sArg2, NULL, 10 );
				
				iValue += iAdd;
				sArg2.Format( _T("%d"), iValue );

				if ( !SetActionVar( sArg1, sArg2, pmsLocalEnv ) )
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sArg1.c_str(), sArg2.c_str() );
				}
				else
				{
					iterProgram++;
				}
			}
			else if ( _tcsnicmp( sLine, _T("sub "), 4 ) == 0 )
			{
				CStdString sArg1, sArg2,sArg3;
				int iValue, iSub;

				sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				sArg2 = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );

				// Get value...
				sArg3.Format( _T("%%%s%%"), sArg1.c_str() );
				sArg3 = TranslateActionVars(sArg3, pmsLocalEnv );

				iValue = _tcstol( sArg3, NULL, 10 );
				iSub = _tcstol( sArg2, NULL, 10 );
				
				iValue -= iSub;
				sArg2.Format( _T("%d"), iValue );

				if ( !SetActionVar( sArg1, sArg2, pmsLocalEnv ) )
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sArg1.c_str(), sArg2.c_str() );
				}
				else
				{
					iterProgram++;
				}
			}
			else if ( _tcsnicmp( sLine, _T("div "), 4 ) == 0 )
			{
				CStdString sArg1, sArg2,sArg3;
				int iValue, iDiv;

				sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				sArg2 = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );

				// Get value...
				sArg3.Format( _T("%%%s%%"), sArg1.c_str() );
				sArg3 = TranslateActionVars(sArg3, pmsLocalEnv );

				iValue = _tcstol( sArg3, NULL, 10 );
				iDiv = _tcstol( sArg2, NULL, 10 );
				
				if ( iDiv )
				{
					iValue /= iDiv;
				}
				else
				{
					iValue = 0;
				}
				sArg2.Format( _T("%d"), iValue );

				if ( !SetActionVar( sArg1, sArg2, pmsLocalEnv ) )
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sArg1.c_str(), sArg2.c_str() );
				}
				else
				{
					iterProgram++;
				}
			}
			else if ( _tcsnicmp( sLine, _T("mult "), 5 ) == 0 )
			{
				CStdString sArg1, sArg2,sArg3;
				int iValue, iMult;

				sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				sArg2 = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );

				// Get value...
				sArg3.Format( _T("%%%s%%"), sArg1.c_str() );
				sArg3 = TranslateActionVars(sArg3, pmsLocalEnv );

				iValue = _tcstol( sArg3, NULL, 10 );
				iMult = _tcstol( sArg2, NULL, 10 );
				
				iValue *= iMult;
				sArg2.Format( _T("%d"), iValue );

				if ( !SetActionVar( sArg1, sArg2, pmsLocalEnv ) )
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sArg1.c_str(), sArg2.c_str() );
				}
				else
				{
					iterProgram++;
				}
			}
			else if ( ( _tcsnicmp( sLine, _T("if "), 3 ) == 0 ) || ( _tcsnicmp( sLine, _T("if# "), 4 ) == 0 ) )
			{
				CStdString sArg1, sOp, sArg2, sGoto, sArg3;
				bool bOK = true;
				int iGotoLine = -1;
				bool bNumeric = false;
				bool bCondition = false;
				int iArg1 = 0, iArg2 = 0;

				if ( _tcsnicmp( sLine, _T("if# "), 4 ) == 0 )
				{
					bNumeric = true;
				}

				sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				sOp = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );
				sArg2 = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );
				sGoto = TranslateActionVars(GetArg( sLine, 4 ), pmsLocalEnv );
				sArg3 = TranslateActionVars(GetArg( sLine, 5 ), pmsLocalEnv );

				if ( bNumeric )
				{
					iArg1 = _tcstol( sArg1, NULL, 10 );
					iArg2 = _tcstol( sArg2, NULL, 10 );
				}

				if ( _tcsicmp( sGoto, _T("goto") ) == 0 )
				{
					if ( sArg3.GetLength() == 0 )
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: Bad Syntax for IF") );
						bOK = false;
					}
				}
				else
				{
					if ( sGoto.GetLength() && (sArg3.GetLength() == 0) )
					{
						sArg3 = sGoto;
					}
					else
					{
						// Bad news... bad label!
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: Bad Syntax for IF") );
						bOK = false;
					}
				}

				if ( bOK )
				{
					TMapLabels::iterator iterLabel;

					sArg3.MakeLower();
					if ( sArg3.GetLength() && ((iterLabel = mlLabels.find( sArg3 ) ) != mlLabels.end()) )
					{
						iGotoLine = mlLabels[sArg3];
//						// Found it
//						iterProgram = lsProgram.begin();
//						while( iLine && (iterProgram != lsProgram.end()))
//						{
//							iterProgram++;
//							iLine--;
//						}
					}
					else
					{
						iGotoLine = -1;
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: Invalid Label for IF") );
						bOK = false;
					}
					if ( bOK )
					{
						int iEval = 0;
						if ( bNumeric )
						{
							if ( iArg1 > iArg2 )
							{
								iEval = 1;
							}
							else if ( iArg1 < iArg2 )
							{
								iEval = -1;
							}
						}
						else
						{
							iEval = _tcsicmp( sArg1, sArg2 );
						}
						if ( _tcscmp( sOp, _T("==") ) == 0 )
						{
							if ( iEval == 0 )
							{
								bCondition = true;
							}
						}
						else if ( _tcscmp( sOp, _T("<") ) == 0 )
						{
							if ( iEval < 0 )
							{
								bCondition = true;
							}
						}
						else if ( _tcscmp( sOp, _T(">") ) == 0 )
						{
							if ( iEval > 0 )
							{
								bCondition = true;
							}
						}
						else if ( _tcscmp( sOp, _T("<=") ) == 0 )
						{
							if ( iEval <= 0 )
							{
								bCondition = true;
							}
						}
						else if ( _tcscmp( sOp, _T(">=") ) == 0 )
						{
							if ( iEval >= 0 )
							{
								bCondition = true;
							}
						}
						else if ( ( _tcscmp( sOp, _T("<>") ) == 0 ) || (_tcscmp( sOp, _T("!=") ) == 0 ) )
						{
							if ( iEval )
							{
								bCondition = true;
							}
						}
						else if ( _tcscmp( sOp, _T("##") ) == 0  )
						{
							if ( bNumeric )
							{
								// Bad news... bad label!
								iterProgram = lsProgram.end();
								bContinue = false;
								sError.Format( _T("ERROR: Not a numeric conditional") );
								bOK = false;
							}
							else
							{
								if ( sArg1.Find( sArg2 ) >= 0 )
								{
									bCondition = true;
								}
							}
						}
						else if ( _tcscmp( sOp, _T("#^") ) == 0  )
						{
							if ( bNumeric )
							{
								// Bad news... bad label!
								iterProgram = lsProgram.end();
								bContinue = false;
								sError.Format( _T("ERROR: Not a numeric conditional") );
								bOK = false;
							}
							else
							{
								sArg1.MakeLower();
								sArg2.MakeLower();
								if ( sArg1.Find( sArg2 ) >= 0 )
								{
									bCondition = true;
								}
							}
						}
						else if ( _tcscmp( sOp, _T("!##") ) == 0  )
						{
							if ( bNumeric )
							{
								// Bad news... bad label!
								iterProgram = lsProgram.end();
								bContinue = false;
								sError.Format( _T("ERROR: Not a numeric conditional") );
								bOK = false;
							}
							else
							{
								if ( sArg1.Find( sArg2 ) < 0 )
								{
									bCondition = true;
								}
							}
						}
						else if ( _tcscmp( sOp, _T("!#^") ) == 0  )
						{
							if ( bNumeric )
							{
								// Bad news... bad label!
								iterProgram = lsProgram.end();
								bContinue = false;
								sError.Format( _T("ERROR: Not a numeric conditional") );
								bOK = false;
							}
							else
							{
								sArg1.MakeLower();
								sArg2.MakeLower();
								if ( sArg1.Find( sArg2 ) < 0 )
								{
									bCondition = true;
								}
							}
						}
						if ( bCondition )
						{
							// Found it
							iLine = iGotoLine;
							iterProgram = lsProgram.begin();
							while( iLine && (iterProgram != lsProgram.end()))
							{
								iterProgram++;
								iLine--;
							}
						}
						else
						{
							iterProgram++;
						}
					}
				}

			}
			else if ( ( _tcsnicmp( sLine, _T("quit "), 5 ) == 0 ) || ( _tcsicmp( sLine, _T("quit") ) == 0 ) )
			{
				iterProgram = lsProgram.end();
				bContinue = false;
			}
			//else if ( _tcsnicmp( sLine, _T("mediapatch "), 11 ) == 0 )
			//{
			//	CStdString sFilename = MakeFullFilePath( g_sPSCCurDir, TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv ) );

			//	if ( MediaPatch( sFilename ) )
			//	{
			//		iterProgram++;
			//	}
			//	else
			//	{
			//		iterProgram = lsProgram.end();
			//		bContinue = false;
			//		sError.Format( _T("ERROR: (MediaPatch) Erorr applying media patch") );
			//	}
			//}
			else if ( _tcsnicmp( sLine, _T("callfile "), 9 ) == 0 ) 
			{
				CStdString sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );

				sArg1 = MakeFullFilePath( g_sPSCCurDir, sArg1 );

				if ( FileExists( sArg1 ) )
				{
					TMapStrings		msLocalEnvironmentSave;
					CStdString sVarName, sFilename(sArg1);

					msLocalEnvironmentSave = (*pmsLocalEnv);
						
					// We'll call it here... after copying arguments into local environment
					SetActionVar( _T("ActionScriptFile"), sFilename, &msLocalEnvironmentSave );
					SetActionVar( _T("ActionScriptName"), _T(""), &msLocalEnvironmentSave );
					SetActionVar( _T("0"), sFilename, &msLocalEnvironmentSave );
					sArg1 = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );
					int iIndex = 0;
					while( sArg1.GetLength() )
					{
						iIndex++;
						sVarName.Format( _T("%d"), iIndex );
						SetActionVar( sVarName, sArg1, &msLocalEnvironmentSave );
						sArg1 = TranslateActionVars(GetArg( sLine, 2+iIndex ), pmsLocalEnv );
					}
					sVarName.Format( _T("%d"), iIndex );
					SetActionVar( _T("ArgCount"), sVarName, &msLocalEnvironmentSave );

					// Now load the file and run it...
					sVarName = LoadStringFromFile( sFilename );
					if ( SUCCEEDED( DoActionScript( sVarName, false, &msLocalEnvironmentSave, pactionDrawList, pXMLMap ) ) )
					{
						iterProgram++;
					}
					else
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: ActionScript file failed execution") );
					}
					// g_msLocalEnvironment = msLocalEnvironmentSave;
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: ActionScript file does not exist") );
				}
			}
			else if ( _tcsnicmp( sLine, _T("callscript "), 11 ) == 0 ) 
			{
				CStdString sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				CStdString sScript;

				sScript = GetInternalScript( sArg1 );

				if ( sScript.GetLength() )
				{
					TMapStrings		msLocalEnvironmentSave;
					CStdString sVarName;

					msLocalEnvironmentSave = (*pmsLocalEnv);
						
					// We'll call it here... after copying arguments into local environment
					SetActionVar( _T("ActionScriptFile"), _T(""), &msLocalEnvironmentSave );
					SetActionVar( _T("ActionScriptName"), sArg1, &msLocalEnvironmentSave );
					SetActionVar( _T("0"), sArg1, &msLocalEnvironmentSave );
					sArg1 = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );
					int iIndex = 0;
					while( sArg1.GetLength() )
					{
						iIndex++;
						sVarName.Format( _T("%d"), iIndex );
						SetActionVar( sVarName, sArg1, &msLocalEnvironmentSave );
						sArg1 = TranslateActionVars(GetArg( sLine, 2+iIndex ), pmsLocalEnv );
					}
					sVarName.Format( _T("%d"), iIndex );
					SetActionVar( _T("ArgCount"), sVarName, &msLocalEnvironmentSave );

					if ( SUCCEEDED( DoActionScript( sScript, false, &msLocalEnvironmentSave, pactionDrawList, pXMLMap ) ) )
					{
						iterProgram++;
					}
					else
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: ActionScript failed execution") );
					}
					// g_msLocalEnvironment = msLocalEnvironmentSave;
				}
				else
				{
					iterProgram++;
				}
			}
			else if ( _tcsnicmp( sLine, _T("begindraw"), 9 ) == 0 )
			{
				if ( !m_bBeginDraw )
				{
					CStdString sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
					sArg1.MakeLower();
//					if ( g_ActionDrawState.m_pDevice == NULL )
//					{
//						g_ActionDrawState.m_pDevice = GetStateApp()->Get3dDevice();
//					}
//					if ( g_ActionDrawState.m_pDevice )
//					{
						if ( pactionDrawList )
						{
							ClearActionDrawList( *pactionDrawList );
						}

						m_bBeginDraw = true;
						//if ( g_ActionDrawState.m_pBackTexture == NULL )
						//{
						//	GetBackBufferTexture( g_ActionDrawState.m_pDevice, &g_ActionDrawState.m_pBackTexture );
						//}
						// g_ActionDrawState.m_pDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0L );
						if ( _tcsicmp( sArg1, _T("usecurrent") ) != 0  ) // && g_ActionDrawState.m_pBackTexture )
						{
							CADBox * pBox = new CADBox;

							if ( pBox )
							{
								pBox->SetPos( 0, 0, 640, 480 );
								pBox->SetColor( 0xFF000000, 0xFF000000 );
								if ( pactionDrawList )
								{
									AddActionDrawList( *pactionDrawList, pBox );
								}
								else
								{
									delete pBox;
								}
							}
						}
						//else
						//{
						//	CADImage * pNewImage = new CADImage;

						//	if ( pNewImage )
						//	{
						//		pNewImage->SetImage(g_ActionDrawState.m_pBackTexture, true );
						//		pNewImage->SetPos( 0, 0, 640, 480 );
						//		if ( pactionDrawList )
						//		{
						//			AddActionDrawList( *pactionDrawList, pNewImage );
						//		}
						//		else
						//		{
						//			delete pNewImage;
						//		}
						//	}
						//	//GetStateApp()->RenderQuad( g_ActionDrawState.m_pBackTexture, 640, 480, 1.0, 0.0 );
						//}
//					}
				}
				iterProgram++;
			}
			else if ( _tcsnicmp( sLine, _T("messagebox "), 11 ) == 0 )
			{
				CStdString sArg1;
				sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				if ( m_bBeginDraw )
				{
					// Render to current drawing screen....
					CADMsgBox * pMsgBox = new CADMsgBox;

					if ( pMsgBox )
					{
						pMsgBox->SetMessage(sArg1);
						if ( pactionDrawList )
						{
							AddActionDrawList( *pactionDrawList, pMsgBox );
						}
						else
						{
							delete pMsgBox;
						}
					}

					// GetStateApp()->DrawMessageBox( sArg1 );
				}
				else
				{
					GetStateApp()->MessageBox( _T("%s"), sArg1.c_str() );
				}
				iterProgram++;
			}
			else if ( _tcsnicmp( sLine, _T("box "), 4 ) == 0 )
			{
				CStdString sArgX, sArgY, sArgW, sArgH, sArgCol,sArgColBorder;
				DWORD dwColor, dwBorderColor;
//				FLOAT fX, fY, fX2, fY2;
				DWORD dwX, dwY, dwW, dwH;
				sArgX = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				sArgY = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );
				sArgW = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );
				sArgH = TranslateActionVars(GetArg( sLine, 4 ), pmsLocalEnv );
				sArgCol = TranslateActionVars(GetArg( sLine, 5 ), pmsLocalEnv );
				sArgColBorder = TranslateActionVars(GetArg( sLine, 6 ), pmsLocalEnv );

				dwX = _tcstoul( sArgX, NULL, 10 ); 
//				fX = (FLOAT)dwX;
//				fX += 0.5;
				dwW = _tcstoul( sArgW, NULL, 10 ); 
//				fX2 = (FLOAT)dwX;
//				fX2 += 0.5;
				dwY = _tcstoul( sArgY, NULL, 10 ); 
//				fY = (FLOAT)dwY;
//				fY += 0.5;
				dwH = _tcstoul( sArgH, NULL, 10 ); 
//				fY2 = (FLOAT)dwY;
//				fY2 += 0.5;

				dwColor = ConvertColorName( sArgCol );
//				if ( dwColor == 0 )
//				{
//					dwColor = GetStateApp()->MessageAttr.m_dwBoxColor;
//				}
				dwBorderColor = ConvertColorName( sArgColBorder );
//				if ( dwBorderColor == 0 )
//				{
//					dwBorderColor = GetStateApp()->MessageAttr.m_dwBorderColor;
//				}
				if ( m_bBeginDraw )
				{
					CADBox * pBox = new CADBox;

					if ( pBox )
					{
						pBox->SetPos( dwX, dwY, dwW, dwH );
						pBox->SetColor( dwColor, dwBorderColor );
						if ( pactionDrawList )
						{
							AddActionDrawList( *pactionDrawList, pBox );
						}
						else
						{
							delete pBox;
						}
					}

					// Render to current drawing screen....
//					DrawBox( g_ActionDrawState.m_pDevice, fX, fY, fX2, fY2, dwColor, dwBorderColor );
				}
				iterProgram++;
			}
			else if ( _tcsnicmp( sLine, _T("text "), 5 ) == 0 )
			{
				CStdString sArgX, sArgY, sArgJ, sArgText, sArgCol;
				DWORD dwColor, dwFlags;
//				FLOAT fX, fY;
				DWORD dwX, dwY;
				sArgX = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				sArgY = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );
				sArgJ = TranslateActionVars(GetArg( sLine, 3 ), pmsLocalEnv );
				sArgText = TranslateActionVars(GetArg( sLine, 4 ), pmsLocalEnv );
				sArgCol = TranslateActionVars(GetArg( sLine, 5 ), pmsLocalEnv );
				// sArgColBorder = TranslateActionVars(GetArg( sLine, 5 ));

				dwFlags = XBFONT_LEFT;
				dwX = _tcstoul( sArgX, NULL, 10 ); 
//				fX = (FLOAT)dwX;
//				fX += 0.5;
				dwY = _tcstoul( sArgY, NULL, 10 ); 
//				fY = (FLOAT)dwY;
//				fY += 0.5;

				dwColor = ConvertColorName( sArgCol );
//				if ( dwColor == 0 )
//				{
//					dwColor = GetStateApp()->MessageAttr.m_dwTextColor;
//				}

				if ( _tcsicmp( sArgJ, _T("center") ) == 0 )
				{
					dwFlags = XBFONT_CENTER_X;
				}
				if ( _tcsicmp( sArgJ, _T("right") ) == 0 )
				{
					dwFlags = XBFONT_RIGHT;
				}

				if ( m_bBeginDraw )
				{
					CADText * pText = new CADText;

					if ( pText )
					{
						pText->SetPos( dwX, dwY, dwFlags );
						pText->SetColor( dwColor );
						pText->SetText( sArgText );
						if ( pactionDrawList )
						{
							AddActionDrawList( *pactionDrawList, pText );
						}
						else
						{
							delete pText;
						}
					}

					// Render to current drawing screen....
//					DrawBox( g_ActionDrawState.m_pDevice, fX, fY, fX2, fY2, dwColor, dwBorderColor );
				}

//				if ( g_ActionDrawState.m_bBeginDraw )
//				{
//					// Render to current drawing screen....
//					GetStateApp()->m_Font16.DrawText( fX, fY, dwColor, sArgText, dwFlags );
//					// DrawBox( g_ActionDrawState.m_pDevice, fX, fY, fX2, fY2, dwColor, dwBorderColor );
//				}
				iterProgram++;
			}
			else if ( _tcsnicmp( sLine, _T("input"), 5 ) == 0 )
			{
				CStdString sArg1 = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );
				bool bWait = true;

				if ( _tcsicmp( sArg1, _T("nowait") ) == 0 )
				{
					bWait = false;
				}
				// If currently drawing, wrap it up and display it!
//				if ( g_ActionDrawState.m_bBeginDraw )
//				{
//					g_ActionDrawState.m_bBeginDraw = false;
					// Present the scene
//					GetStateApp()->Get3dDevice()->Present( NULL, NULL, NULL, NULL );
//				}
				// Now Query Input and wait....
//				GetStateApp()->QueryInput();
				XBGAMEPAD* pGamePad = NULL;
				

				
				pGamePad = GetStateApp()->GetDefaultGamepad();
				while( !HasInput(pGamePad) && bWait )
				{
//					GetStateApp()->QueryInput();
					pGamePad = GetStateApp()->GetDefaultGamepad();
				}
				ReadInputToEnv(pGamePad);
				iterProgram++;
			}
			else if ( _tcsnicmp( sLine, _T("enddraw"), 7 ) == 0 )
			{
				if ( m_bBeginDraw )
				{
					GetStateApp()->m_pdrawList = pactionDrawList;
					m_bBeginDraw = false;
//					if ( pactionDrawList )
//					{
//						DrawActionDrawList( *pactionDrawList );
//					}
					// Present the scene
//					GetStateApp()->Get3dDevice()->Present( NULL, NULL, NULL, NULL );
				}
				iterProgram++;
			}
			else if ( _tcsnicmp( sLine, _T("xmlopen "), 8 ) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sFilename = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );

				ASXML_Open( sHandle, sFilename, g_sPSCCurDir, *pXMLMap );
				iterProgram++;
			}
			else if ( _tcsnicmp( sLine, _T("xmlclose "), 9 ) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));

				ASXML_Close( sHandle, *pXMLMap );
				iterProgram++;
			}
			else if ( _tcsnicmp( sLine, _T("xmlsave "), 8 ) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sFilename = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );
				
				ASXML_Save( sHandle, sFilename, g_sPSCCurDir, *pXMLMap );
				iterProgram++;
			}
			else if ( _tcsnicmp( sLine, _T("xmlsavenode "), 12 ) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sFilename = TranslateActionVars(GetArg( sLine, 2 ), pmsLocalEnv );
				
				ASXML_SaveNode( sHandle, sFilename, g_sPSCCurDir, *pXMLMap );
				iterProgram++;
			}
			else if ( _tcsnicmp( sLine, _T("xmlsetnodeptr "), 14 ) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sLocation = TranslateActionVars(GetArg( sLine, 2 ));
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					pXML->SetNodePtr(sLocation);
				}
				iterProgram++;
			}
			else if ( _tcsnicmp( sLine, _T("xmlgetnodeptr "), 14 ) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sVarName = TranslateActionVars(GetArg( sLine, 2 ));
				CStdString sValue;
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					sValue = pXML->GetCurrentNodeLocation();
					if ( !SetActionVar( sVarName, sValue, pmsLocalEnv ) )
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sVarName.c_str(), sValue.c_str() );
					}
					else
					{
						iterProgram++;
					}
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to get XML object (%s)"), sHandle.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("xmlcreatenode "), 14 ) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sLocation = TranslateActionVars(GetArg( sLine, 2 ));
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					pXML->CreateNode( sLocation );
					iterProgram++;
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to get XML object (%s)"), sHandle.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("xmlcreateelement "), 17 ) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sLocation = TranslateActionVars(GetArg( sLine, 2 ));
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					pXML->CreateElement( sLocation );
					iterProgram++;
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to get XML object (%s)"), sHandle.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("xmlsetelementptr "), 17 ) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sLocation = TranslateActionVars(GetArg( sLine, 2 ));
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					pXML->SetElementPtr( sLocation );
					iterProgram++;
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to get XML object (%s)"), sHandle.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("xmlgetelementptr "), 17 ) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sVarName = TranslateActionVars(GetArg( sLine, 2 ));
				CStdString sValue;
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					sValue = pXML->GetCurrentElementLocation();
					if ( !SetActionVar( sVarName, sValue, pmsLocalEnv ) )
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sVarName.c_str(), sValue.c_str() );
					}
					else
					{
						iterProgram++;
					}
				}
			}
			else if ( _tcsnicmp( sLine, _T("xmlsetelementvalue "), 19 ) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sValue = TranslateActionVars(GetArg( sLine, 2 ));
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					pXML->SetElementValue( NULL, sValue );
					iterProgram++;
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to get XML object (%s)"), sHandle.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("xmlsetelementattr "), 18) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					iterProgram++;
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to get XML object (%s)"), sHandle.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("xmlsetnodeattr "), 15) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					iterProgram++;
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to get XML object (%s)"), sHandle.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("xmlsetvalue "), 12) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sLocation = TranslateActionVars(GetArg( sLine, 2 ));
				CStdString sValue = TranslateActionVars(GetArg( sLine, 3 ));
				CStdString sPrefAttr = TranslateActionVars(GetArg( sLine, 4 ));
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					pXML->SetValue( sLocation, sValue, sPrefAttr.GetLength()?true:false );
					iterProgram++;
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to get XML object (%s)"), sHandle.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("xmlgetvalue "), 12) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sVarName = TranslateActionVars(GetArg( sLine, 2 ));
				CStdString sLocation = TranslateActionVars(GetArg( sLine, 3 ));
				CStdString sDefault = TranslateActionVars(GetArg( sLine, 4 ));
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					CStdString sValue = pXML->GetValue( sLocation, sDefault);
					if ( !SetActionVar( sVarName, sValue, pmsLocalEnv ) )
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sVarName.c_str(), sValue.c_str() );
					}
					else
					{
						iterProgram++;
					}
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to get XML object (%s)"), sHandle.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("xmlgetnodecount "), 16) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sVarName = TranslateActionVars(GetArg( sLine, 2 ));
				CStdString sName = TranslateActionVars(GetArg( sLine, 3 ));
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					int iCount = pXML->GetNodeCount( sName );
					CStdString sValue;

					sValue.Format( _T("%d"), iCount );
					if ( !SetActionVar( sVarName, sValue, pmsLocalEnv ) )
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sVarName.c_str(), sValue.c_str() );
					}
					else
					{
						iterProgram++;
					}
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to get XML object (%s)"), sHandle.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("xmlgetelementcount "), 19) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sVarName = TranslateActionVars(GetArg( sLine, 2 ));
				CStdString sName = TranslateActionVars(GetArg( sLine, 3 ));
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					int iCount = pXML->GetElementCount( sName );
					CStdString sValue;

					sValue.Format( _T("%d"), iCount );
					if ( !SetActionVar( sVarName, sValue, pmsLocalEnv ) )
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sVarName.c_str(), sValue.c_str() );
					}
					else
					{
						iterProgram++;
					}
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to get XML object (%s)"), sHandle.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("xmlgetnodeattrcount "), 20) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sVarName = TranslateActionVars(GetArg( sLine, 2 ));
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					int iCount = pXML->GetNodeAttrCount();
					CStdString sValue;

					sValue.Format( _T("%d"), iCount );
					if ( !SetActionVar( sVarName, sValue, pmsLocalEnv ) )
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sVarName.c_str(), sValue.c_str() );
					}
					else
					{
						iterProgram++;
					}
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to get XML object (%s)"), sHandle.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("xmlgetelementattrcount "), 23) == 0 )
			{
				CStdString sHandle = TranslateActionVars(GetArg( sLine, 1 ));
				CStdString sVarName = TranslateActionVars(GetArg( sLine, 2 ));
				CASXmlInfo * pXML = ASXML_GetInfo( sHandle, *pXMLMap );

				if ( pXML )
				{
					int iCount = pXML->GetElementAttrCount();
					CStdString sValue;

					sValue.Format( _T("%d"), iCount );
					if ( !SetActionVar( sVarName, sValue, pmsLocalEnv ) )
					{
						iterProgram = lsProgram.end();
						bContinue = false;
						sError.Format( _T("ERROR: Unable to set variable (%s)=(%s)"), sVarName.c_str(), sValue.c_str() );
					}
					else
					{
						iterProgram++;
					}
				}
				else
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: Unable to get XML object (%s)"), sHandle.c_str() );
				}
			}
			else if ( _tcsnicmp( sLine, _T("searchitem "), 11 ) == 0 )
			{
				CStdString sVarName, sValue;
				sVarName = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );

				//SEARCHITEM <VarName>   (Load VarName with current file/pathname)
				if ( hFind )
				{
					sValue = MakeFullFilePath( sSearchPath, ffData.cFileName );
				}
				if ( !SetActionVar( sVarName, sValue, pmsLocalEnv ) )
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: (SearchItem) Unable to set variable (%s)=(%s)"), sVarName.c_str(), sValue.c_str() );
				}
				else
				{
					iterProgram++;
				}
			}
			else if ( _tcsnicmp( sLine, _T("searchsize "), 11 ) == 0 )
			{
				CStdString sVarName, sValue;
				sVarName = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );

			    //SEARCHSIZE <VarName>   (Load VarName with current file size)
				if ( hFind )
				{
					ULARGE_INTEGER ulXlate;

					ulXlate.LowPart = ffData.nFileSizeLow;
					ulXlate.HighPart = ffData.nFileSizeHigh;

					sValue.Format( _T("%I64d"), ulXlate.QuadPart );
				}
				if ( !SetActionVar( sVarName, sValue, pmsLocalEnv ) )
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: (SearchSize) Unable to set variable (%s)=(%s)"), sVarName.c_str(), sValue.c_str() );
				}
				else
				{
					iterProgram++;
				}
			}
			else if ( _tcsnicmp( sLine, _T("searchtype "), 11 ) == 0 )
			{
				CStdString sVarName, sValue;
				sVarName = TranslateActionVars(GetArg( sLine, 1 ), pmsLocalEnv );

			    //SEARCHTYPE <VarName>   (Load VarName with current type for entry - "file" or "dir")
				if ( hFind )
				{
					if ( ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
					{
						sValue = _T("dir");
					}
					else
					{
						sValue = _T("file");
					}
				}
				if ( !SetActionVar( sVarName, sValue, pmsLocalEnv ) )
				{
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: (Searchtype) Unable to set variable (%s)=(%s)"), sVarName.c_str(), sValue.c_str() );
				}
				else
				{
					iterProgram++;
				}
			}
			else if ( _tcsnicmp( sLine, _T("searchnext"), 10 ) == 0 )
			{
				//SEARCHNEXT
				if ( hFind )
				{
					if ( !FindNextFile( hFind, &ffData ) )
					{
						FindClose( hFind );
						hFind = NULL;
						SetActionVar( _T("search_active"), NULL, pmsLocalEnv );
						SetActionVar( _T("search_name"), NULL, pmsLocalEnv );
						SetActionVar( _T("search_type"), NULL, pmsLocalEnv );
						SetActionVar( _T("search_size"), NULL, pmsLocalEnv );
						SetActionVar( _T("search_path"), NULL, pmsLocalEnv );
						SetActionVar( _T("search_arg"), NULL, pmsLocalEnv );
					}
					else
					{
						SetActionVar( _T("search_name"), ffData.cFileName, pmsLocalEnv );
						if ( ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
						{
							SetActionVar( _T("search_type"), _T("dir"), pmsLocalEnv );
						}
						else
						{
							SetActionVar( _T("search_type"), _T("file"), pmsLocalEnv );
						}
						{
							CStdString sTemp;
							ULARGE_INTEGER ulXlate;

							ulXlate.LowPart = ffData.nFileSizeLow;
							ulXlate.HighPart = ffData.nFileSizeHigh;

							sTemp.Format( _T("%I64d"), ulXlate.QuadPart );
							SetActionVar( _T("search_size"), sTemp, pmsLocalEnv );
						}
					}
				}
				iterProgram++;
			}
			else if ( _tcsnicmp( sLine, _T("endsearch"), 9 ) == 0 )
			{
			    //ENDSEARCH
				if ( hFind )
				{
					FindClose( hFind );
					hFind = NULL;
					SetActionVar( _T("search_active"), NULL, pmsLocalEnv );
					SetActionVar( _T("search_name"), NULL, pmsLocalEnv );
					SetActionVar( _T("search_type"), NULL, pmsLocalEnv );
					SetActionVar( _T("search_size"), NULL, pmsLocalEnv );
					SetActionVar( _T("search_path"), NULL, pmsLocalEnv );
					SetActionVar( _T("search_arg"), NULL, pmsLocalEnv );
				}
				iterProgram++;
			}
			else if ( _tcsnicmp( sLine, _T("beginsearch "), 12 ) == 0 )
			{
			    //BEGINSEARCH <WildCardPath>
				if ( hFind )
				{
					// Error?
					iterProgram = lsProgram.end();
					bContinue = false;
					sError.Format( _T("ERROR: (BeginSearch) Search already in progress") );
				}
				else
				{
					CStdString sPath;

					sWildcard = MakeFullFilePath( g_sPSCCurDir, TranslateActionVars( GetArg( sLine, 1 ), pmsLocalEnv ) );
					sSearchPath = StripFilenameFromPath( sWildcard );
					SetActionVar( _T("search_path"), sSearchPath, pmsLocalEnv );
					SetActionVar( _T("search_arg"), sWildcard, pmsLocalEnv );
					// sPath = PathSlasher( sSearchPath, true );
					// sPath += _T("*");

					hFind = FindFirstFile( sWildcard, &ffData );
					if( INVALID_HANDLE_VALUE == hFind )
					{
						// Search didn't find anything
						hFind = NULL;
						SetActionVar( _T("search_active"), NULL, pmsLocalEnv );
						SetActionVar( _T("search_name"), NULL, pmsLocalEnv );
						SetActionVar( _T("search_type"), NULL, pmsLocalEnv );
						SetActionVar( _T("search_size"), NULL, pmsLocalEnv );
						SetActionVar( _T("search_path"), NULL, pmsLocalEnv );
						SetActionVar( _T("search_arg"), NULL, pmsLocalEnv );
					}
					else
					{
						SetActionVar( _T("search_active"), _T("1"), pmsLocalEnv );
						SetActionVar( _T("search_name"), ffData.cFileName, pmsLocalEnv );
						if ( ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
						{
							SetActionVar( _T("search_type"), _T("dir"), pmsLocalEnv );
						}
						else
						{
							SetActionVar( _T("search_type"), _T("file"), pmsLocalEnv );
						}
						{
							CStdString sTemp;
							ULARGE_INTEGER ulXlate;

							ulXlate.LowPart = ffData.nFileSizeLow;
							ulXlate.HighPart = ffData.nFileSizeHigh;

							sTemp.Format( _T("%I64d"), ulXlate.QuadPart );
							SetActionVar( _T("search_size"), sTemp, pmsLocalEnv );
						}
					}
					iterProgram++;
				}
			}
			else
			{
				hr = ProcessScriptCommand( sLine, sResponse, NULL, PSC_GUI, pmsLocalEnv );
				iterProgram++;
			}
		}
	}

	if ( hFind )
	{
		FindClose( hFind );
		hFind = NULL;
	}

	if ( sError.GetLength() )
	{
		GetStateApp()->MessageBox( _T("%s"), sError.c_str() );
		hr = E_FAIL;
	}
	else
	{
		hr = S_OK;
	}
	if ( bClearLocal )
	{
//		if ( g_ActionDrawState.m_pBackTexture )
//		{
//			g_ActionDrawState.m_pBackTexture->Release();
//			g_ActionDrawState.m_pBackTexture = NULL;
//		}
	}
	if ( bDeleteXMLMap && pXMLMap  )
	{
		ASXML_ClearList( *pXMLMap );
		delete pXMLMap;
	}
	if ( bDeleteDrawList && pactionDrawList )
	{
		ClearActionDrawList( *pactionDrawList );
		delete pactionDrawList;
	}
	if ( pmsLocalEnvironment )
	{
		delete pmsLocalEnvironment;
	}
	CloseFileData( fileData );
	return hr;
}
#endif


int GetGadgetStringID( LPCTSTR szToken )
{
	CStdString sToken(szToken);
	int iReturn = -1;
	int iIndex;

	

	sToken.MakeLower();
	iIndex = 0;
	while( g_szTokens[iIndex] && ( iReturn == -1 ))
	{
		if( sToken.Compare(g_szTokens[iIndex]) == 0 )
		{
			iReturn = iIndex;
		}
		iIndex++;
	}
	return iReturn;
}

CStdString GetXboxVersion( void )
{
	CStdString sVersion;
	BYTE byVersionString[5];

	ZeroMemory( byVersionString, 5 );
	int iRetry = 0;
	HRESULT hr = S_OK;

	while( SUCCEEDED(hr) && (iRetry < 4) && (byVersionString[0]!='P') && (byVersionString[0]!='D') )
	{
		hr = HalReadSMBusValue(0x20, 0x01, 0, (LPBYTE)&byVersionString[0]);
		iRetry++;
	}
	if ( SUCCEEDED(hr) && ((byVersionString[0]=='P') || (byVersionString[0]=='D')) )
	{
		if ( SUCCEEDED( HalReadSMBusValue(0x20, 0x01, 0, (LPBYTE)&byVersionString[1]) ) )
		{
			if ( SUCCEEDED( HalReadSMBusValue(0x20, 0x01, 0, (LPBYTE)&byVersionString[2]) ) )
			{
				sVersion = (LPSTR)byVersionString;
				if ( sVersion.Compare( _T("DBX") ) == 0 )
				{
					sVersion = _T("Debug");
				}
				else if ( sVersion.Compare( _T("P01") ) == 0 )
				{
					sVersion = _T("1.0");
				}
				else if ( sVersion.Compare( _T("P05") ) == 0 )
				{
					sVersion = _T("1.1");
				}
			}
		}
	}

	return sVersion;
}

bool GetTemperature( bool bCPU, int & iTemp, bool bCelsius )
{
	WORD byTemp;
	bool bReturn = false;

	if ( SUCCEEDED( HalReadSMBusValue(0x20, bCPU?0x09:0x0a, 0, (LPBYTE)&byTemp) ) )
	{
		iTemp = (int)byTemp;
		if ( !bCelsius )
		{
			float fConvert = (float)iTemp;
			fConvert *= 1.8f;
			fConvert += 32.5f;
			iTemp = (int)fConvert;
		}
		bReturn = true;
	}
	return bReturn;
}

struct XBOX_KRNL_VERSION
{
	WORD	Major;
	WORD	Minor;
	WORD	Build;
};


bool IsUserStringDefined( int iUserStringID )
{
	bool bReturn = false;

	return bReturn;
}

bool GetUserString( int iUserStringID, CStdString & sValue )
{
	bool bReturn = true;

	if ( bReturn = IsUserStringDefined(iUserStringID) )
	{
		sValue.Format( _T("User String #%d"), iUserStringID );
	}
	else
	{
		sValue.Format( _T("User String #%d"), iUserStringID );
	}
	return bReturn;
}

bool GetGadgetString( int iID, CStdString & sValue, LPCTSTR szSection, LPCTSTR szKey )
{
	bool bReturn = false;
	CItemInfo * pEntry;
	SYSTEMTIME sysTime;
	CStdString sTemp1, sTemp2;
	char szTemp[256];
	DWORD dwLength = 255;
//	DWORD dwBootFlags;
	int iTemp;
	DWORD dwTemp;
	struct XBOX_KRNL_VERSION * pKernel = (struct XBOX_KRNL_VERSION *)XboxKrnlVersion;
	

	ZeroMemory( szTemp, 256 );
	if ( szSection && ( _tcslen( szSection ) == 0 ) )
	{
		szSection = NULL;
	}

	if ( szKey && ( _tcslen( szKey ) == 0 ) )
	{
		szKey = NULL;
	}

	switch( iID )
	{
		case GSTR_ENV:
			// Grab string from global environment
			if ( szKey && _tcslen( szKey ) )
			{
				CStdString sKey(szKey);
				sKey.Trim();
				// Remove leading scores...
				while ( sKey.GetLength() && sKey[0] == _T('_') )
				{
					sKey = sKey.Mid(1);
				}
				sKey.MakeLower();
				if ( sKey.GetLength() && ( g_msGlobalEnvironment.find( sKey ) != g_msGlobalEnvironment.end() ) )
				{
					sValue = g_msGlobalEnvironment[sKey];
				}
			}
			break;
		case GSTR_LAUNCHMODE:
			if ( g_MenuInfo.m_bSafeMode )
			{
				sValue = _T("SAFEMODE - ");
			}
			else
			{
				sValue = _T("");
			}
			if ( g_MenuInfo.m_bDVDMode )
			{
				if ( g_MenuInfo.m_bXDIMode )
				{
					sValue += _T("MenuX Compatibility (XDI)");
				}
				else
				{
					sValue += _T("DVD-R Menu");
				}
			}
			else
			{
				if ( g_MenuInfo.m_bDashMode )
				{
					sValue += _T("Dashboard");
				}
				else
				{
					sValue += _T("Apps Launcher");
				}
			}
			break;
		case GSTR_NEWSFEED:
			// Section is URL, Key is Index...
			sValue = g_NewsFeedManager.GetNewsItemString(_T("title"));
			bReturn = true;
			break;
		case GSTR_DISCTITLE:
			sValue = g_MenuInfo.m_sTitle;
			break;
		case GSTR_DISCSUBTITLE:
			sValue = g_MenuInfo.m_sSubTitle;
			break;
		case GSTR_NUMFTPUSERS:
			sValue.Format( _T("%d FTP Users online"), g_MenuInfo.Xbox.m_iNumFTPUsers);
			bReturn = true;
			break;
		case GSTR_CURRENTSONG:
			sValue = GetStateApp()->MusicGetCurrentSong();
			bReturn = true;
			break;
		case GSTR_FLASHID:
			dwTemp = GetFlashID();
			sValue = GetFlashString(dwTemp,true);
			break;
		case GSTR_CURRENTSOUNDTRACK:
			sValue = GetStateApp()->MusicGetCurrentSoundtrack();
			bReturn = true;
			break;
		case GSTR_CG_TITLE:
			if ( g_MenuInfo.m_bSpecialLaunch )
			{
				sValue = _T("");
			}
			else
			{
				pEntry = g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem);
				if ( pEntry )
				{
					sValue = pEntry->GetValue(szII_Title);
				}
				else
				{
					sValue = _T("UNKNOWN");
				}
			}
			bReturn = true;
			break;
		case GSTR_CG_DESCR:
			if ( g_MenuInfo.m_bSpecialLaunch )
			{
				sValue = _T("");
			}
			else
			{
				pEntry = g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem);
				if ( pEntry )
				{
					sValue = pEntry->GetValue(szII_Descr);
				}
				else
				{
					sValue = _T("UNKNOWN");
				}
				bReturn = true;
			}
			break;
		case GSTR_DRV_C_SIZE:
			sValue.Format( _T("C: %s bytes total"), GetDiskTotal('c', 1 ).c_str() );
			break;
		case GSTR_DRV_E_SIZE:
			sValue.Format( _T("E: %s bytes total"), GetDiskTotal('e', 1 ).c_str() );
			break;
		case GSTR_DRV_F_SIZE:
			sValue.Format( _T("F: %s bytes total"), GetDiskTotal('f', 1 ).c_str() );
			break;
		case GSTR_DRV_G_SIZE:
			sValue.Format( _T("G: %s bytes total"), GetDiskTotal('g', 1 ).c_str() );
			break;
		case GSTR_DRV_C_FREE:
			sValue.Format( _T("C: %s bytes free"), GetDiskFree('c', 1 ).c_str() );
			bReturn = true;
			break;
		case GSTR_DRV_E_FREE:
			sValue.Format( _T("E: %s bytes free"), GetDiskFree('e', 1 ).c_str() );
			bReturn = true;
			break;
		case GSTR_DRV_F_FREE:
			sValue.Format( _T("F: %s bytes free"), GetDiskFree('f', 1 ).c_str() );
			bReturn = true;
			break;
		case GSTR_DRV_G_FREE:
			sValue.Format( _T("G: %s bytes free"), GetDiskFree('g', 1 ).c_str() );
			bReturn = true;
			break;
		case GSTR_ACTUAL_PATH:
			sValue = g_MenuInfo.m_sActualPath;
			break;
		case GSTR_IP:
			if ( g_MenuInfo.m_pPrefInfo == NULL )
			{
				sValue = GetIPString();
				// Dynamic if the IP hasn't been set properly yet.
				if ( !g_MenuInfo.m_bXnaValid )
				{
					bReturn = true;
				}
			}
			else if (  g_MenuInfo.m_pPrefInfo->GetBool( _T("network"), _T("enablenetwork"), TRUE ) && 
				       !(g_MenuInfo.SafeMode.m_dwFlags&SM_NONETWORK) )
			{
				sValue = GetIPString();
				// Dynamic if the IP hasn't been set properly yet.
				if ( !g_MenuInfo.m_bXnaValid )
				{
					bReturn = true;
				}
			}
			else
			{
				sValue = _T("0.0.0.0");
				bReturn = false;
			}
			break;
		case GSTR_SUBNETMASK:
			if ( g_MenuInfo.CurrentNetParams.m_sSubnetmask.GetLength() )
			{
				sValue = g_MenuInfo.CurrentNetParams.m_sSubnetmask;
			}
			else
			{
				sValue = _T("");
				bReturn = true;
			}
			break;
		case GSTR_DEFAULTGATEWAY:
			if ( g_MenuInfo.CurrentNetParams.m_sDefaultgateway.GetLength() )
			{
				sValue = g_MenuInfo.CurrentNetParams.m_sDefaultgateway;
			}
			else
			{
				sValue = _T("");
				bReturn = true;
			}
			break;
		case GSTR_DNS1:
			if ( g_MenuInfo.CurrentNetParams.m_sDNS1.GetLength() )
			{
				sValue = g_MenuInfo.CurrentNetParams.m_sDNS1;
			}
			else
			{
				sValue = _T("");
				bReturn = true;
			}
			break;
		case GSTR_DNS2:
			if ( g_MenuInfo.CurrentNetParams.m_sDNS2.GetLength() )
			{
				sValue = g_MenuInfo.CurrentNetParams.m_sDNS2;
			}
			else
			{
				sValue = _T("");
				bReturn = true;
			}
			break;
		case GSTR_BIOSMD5:
			sValue = g_MenuInfo.Xbox.m_sBIOSMD5;
			break;
		case GSTR_CURRENTSUBMENU:
			sValue = g_MenuInfo.m_sMenuName;
			bReturn = true;
			break;
		case GSTR_TIME:
			GetLocalTime(&sysTime);
			switch( g_MenuInfo.m_iTimeFormat )
			{
				case 3: // AM/PM, no seconds
					if	( sysTime.wHour < 12 )
					{
						if ( sysTime.wHour == 0 )
						{
							sValue.Format( _T("12%c%02d%am"), g_MenuInfo.m_tcTimeSepChar, sysTime.wMinute );
						}
						else
						{
							sValue.Format( _T("%2d%c%02dam"), sysTime.wHour, g_MenuInfo.m_tcTimeSepChar, sysTime.wMinute );
						}
					}
					else
					{
						if ( sysTime.wHour == 12 )
						{
							sValue.Format( _T("12%c%02dam"), g_MenuInfo.m_tcTimeSepChar, sysTime.wMinute );
						}
						else
						{
							sValue.Format( _T("%2d%c%02dpm"), sysTime.wHour-12, g_MenuInfo.m_tcTimeSepChar, sysTime.wMinute );
						}
					}
					break;
				case 2: // 24-hour, no seconds
					sValue.Format( _T("%2d%c%02d"), sysTime.wHour, g_MenuInfo.m_tcTimeSepChar, sysTime.wMinute );
					break;
				case 1: // AM/PM
					if	( sysTime.wHour < 12 )
					{
						if ( sysTime.wHour == 0 )
						{
							sValue.Format( _T("12%c%02d%c%02dam"), g_MenuInfo.m_tcTimeSepChar, sysTime.wMinute, g_MenuInfo.m_tcTimeSepChar, sysTime.wSecond );
						}
						else
						{
							sValue.Format( _T("%2d%c%02d%c%02d"), sysTime.wHour, g_MenuInfo.m_tcTimeSepChar, sysTime.wMinute, g_MenuInfo.m_tcTimeSepChar, sysTime.wSecond );
						}
					}
					else
					{
						if ( sysTime.wHour == 12 )
						{
							sValue.Format( _T("12%c%02d%c%02dam"), g_MenuInfo.m_tcTimeSepChar, sysTime.wMinute, g_MenuInfo.m_tcTimeSepChar, sysTime.wSecond );
						}
						else
						{
							sValue.Format( _T("%2d%c%02d%c%02dpm"), sysTime.wHour-12, g_MenuInfo.m_tcTimeSepChar, sysTime.wMinute, g_MenuInfo.m_tcTimeSepChar, sysTime.wSecond );
						}
					}
					break;
				case 0: // 24-hour
				default:
					sValue.Format( _T("%2d%c%02d%c%02d"), sysTime.wHour, g_MenuInfo.m_tcTimeSepChar, sysTime.wMinute, g_MenuInfo.m_tcTimeSepChar, sysTime.wSecond );
					break;
			}
			bReturn = true;
			break;
		case GSTR_MXMVERSION:
			sValue = _T(VERSION_STRING);
			break;
		case GSTR_KERNELVERSION:
			// sValue = (LPCSTR)XeImageFileName;
			// dwBootFlags = (DWORD)XboxBootFlags;
			// struct _STRING
			// {
			//     WORD Length;
			//     WORD MaximumLength;
			//     char * Buffer;
			// };
			// sValue = (LPCSTR)LaunchDataPage;
			sValue.Format( _T("%d.%d.%d"), pKernel->Major, pKernel->Minor, pKernel->Build );
			// sValue = (LPCSTR)XboxKrnlVersion;
			break;
		case GSTR_BIOSNAME:
			sValue = g_MenuInfo.Xbox.m_sBIOSName;
			break;
		case GSTR_BIOSVERSION:
			sValue = _T("");
			break;
		case GSTR_RDNAME:
			sValue = _T("");
			break;
		case GSTR_NAME:
			if ( g_MenuInfo.Xbox.m_sNickName.GetLength() )
			{
				sValue = g_MenuInfo.Xbox.m_sNickName;
			}
			else
			{
				sValue = _T("<UNKNOWN>");
			}
			// sValue = _T("");
			break;
		case GSTR_TEMPSYS:
			bReturn = GetGadgetString( g_MenuInfo.m_bUseCelsius?GSTR_TEMPSYSC:GSTR_TEMPSYSF, sValue );
			break;
		case GSTR_TEMPCPU:
			bReturn = GetGadgetString( g_MenuInfo.m_bUseCelsius?GSTR_TEMPCPUC:GSTR_TEMPCPUF, sValue );
			break;
		case GSTR_TEMPSYSF:
			if ( GetTemperature(false, iTemp, false ) )
			{
				sValue.Format( _T("M/B: %d deg F"), iTemp );
			}
			else
			{
				sValue = _T("");
			}
			bReturn = true;
			break;
		case GSTR_FANSPEED:
			sValue.Format( _T("%d"), GetFanSpeed() );
			bReturn = true;
			break;
		case GSTR_TEMPCPUF:
			if ( GetTemperature(true, iTemp, false ) )
			{
				sValue.Format( _T("CPU: %d deg F"), iTemp );
			}
			else
			{
				sValue = _T("");
			}
			bReturn = true;
			break;
		case GSTR_TEMPSYSC:
			if ( GetTemperature(false, iTemp, true ) )
			{
				sValue.Format( _T("M/B: %d deg C"), iTemp );
			}
			else
			{
				sValue = _T("");
			}
			bReturn = true;
			break;
		case GSTR_TEMPCPUC:
			if ( GetTemperature(true, iTemp, true ) )
			{
				sValue.Format( _T("CPU: %d deg C"), iTemp );
			}
			else
			{
				sValue = _T("");
			}
			bReturn = true;
			break;
		case GSTR_TRAYSTATE:
			if ( g_MenuInfo.Xbox.m_bTrayOpen )
			{
				sValue = _T("Tray Open");
			}
			else if ( g_MenuInfo.Xbox.m_bTrayClosed )
			{
				sValue = _T("Tray Closed");
			}
			else
			{
				sValue = _T("Tray Init");
			}
			bReturn = true;
			break;
		case GSTR_DVDTYPE:
			if ( !g_MenuInfo.Xbox.m_bTrayClosed || g_MenuInfo.Xbox.m_bTrayEmpty )
			{
				sValue = _T("Empty");
			}
			else
			{
				switch( g_MenuInfo.Xbox.m_iDVDType )
				{
					case XBI_DVD_EMPTY:
						sValue = _T("Empty");
						break;
					case XBI_DVD_UNKNOWN:
						sValue = _T("Unknown");
						break;
					case XBI_DVD_XBOX:
						sValue = _T("Game Disc");
						break;
					case XBI_DVD_MOVIE:
						sValue = _T("Movie Disc");
						break;
					case XBI_DVD_DATA:
						sValue = _T("Data Disc");
						break;
					case XBI_DVD_MUSIC:
						sValue = _T("CD Audio Disc");
						break;
				}
			}
			bReturn = true;
			break;
		case GSTR_DATE:
			GetLocalTime(&sysTime);
			switch( g_MenuInfo.m_iDateFormat )
			{
				case 3: // Euro3 style
					sValue.Format( _T("%d%c%d%c%02d"), sysTime.wDay, g_MenuInfo.m_tcDateSepChar, sysTime.wMonth, g_MenuInfo.m_tcDateSepChar, sysTime.wYear%100 );
					break;
				case 2: // Euro2 style
					sValue.Format( _T("%d%c%d %02d"), sysTime.wDay, g_MenuInfo.m_tcDateSepChar, sysTime.wMonth, sysTime.wYear%100 );
					break;
				case 1: // Euro style
					sValue.Format( _T("%02d%c%d%c%d"), sysTime.wYear%100, g_MenuInfo.m_tcDateSepChar, sysTime.wMonth, g_MenuInfo.m_tcDateSepChar, sysTime.wDay );
					break;
				case 0:
				default:
					sValue.Format( _T("%d%c%d%c%02d"), sysTime.wMonth, g_MenuInfo.m_tcDateSepChar, sysTime.wDay, g_MenuInfo.m_tcDateSepChar, sysTime.wYear%100 );
					break;
			}
			bReturn = true;
			break;
		case GSTR_DATETIME:
			GetGadgetString(GSTR_DATE, sTemp1 );
			GetGadgetString(GSTR_TIME, sTemp2 );
			sValue.Format( _T("%s %s"), sTemp1.c_str(), sTemp2.c_str() );
			bReturn = true;
			break;
		case GSTR_CONFIGVAL:
			if ( g_MenuInfo.m_pCfgInfo && szKey && _tcslen(szKey ) )
			{
				sValue = g_MenuInfo.m_pCfgInfo->GetString( szSection, szKey, _T(""), true );
			}
			break;
		case GSTR_SKINVAL:
			if ( g_MenuInfo.m_pSkinInfo && szKey && _tcslen(szKey ) )
			{
				sValue = g_MenuInfo.m_pSkinInfo->GetString( szSection, szKey, _T(""), true );
			}
			break;
		case GSTR_CG_ENTRYVALDW:
			GetGadgetString(GSTR_CG_ENTRYVAL, sTemp1, szSection, szKey );
			if ( sTemp1.GetLength() )
			{
				DWORD dwValue = atol(sTemp1);
				sValue.Format( _T("0x%08x"), dwValue );
			}
			else
			{
				sValue = _T("");
			}
			bReturn = true;
			break;
		case GSTR_CG_ENTRYMODE:
			pEntry = g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem);
			if ( pEntry )
			{
				switch( pEntry->m_iVideoMode )
				{
					case -1: // PAL
						sValue = _T("PAL");
						break;
					case 1: // NTSC
						sValue = _T("NTSC");
						break;
					case 0: // Auto
						sValue = _T("AUTO");
						break;
				}
			}
			bReturn = true;
			break;
		case GSTR_CG_ENTRYVAL:
			if ( szKey && _tcslen(szKey ) )
			{
				if ( g_MenuInfo.m_bSpecialLaunch )
				{
					sValue = _T("");
				}
				else
				{
					pEntry = g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem);
					if ( pEntry )
					{
						sValue = pEntry->GetValue(szKey);
					}
					else
					{
						sValue = _T("UNKNOWN");
					}
					bReturn = true;
				}
			}
			bReturn = true;
			break;
		case GSTR_MAC:
			g_eepromInfo.GetMACAddressString( szTemp, &dwLength );
			sValue = szTemp;
			break;
		case GSTR_XBOXVERS:
			switch ( g_eepromInfo.GetXBOXVersion() )
			{
				case XKEEPROM::V1_0:
					sValue = _T("Xbox Ver 1.0");
					break;
				case XKEEPROM::V1_1:
					sValue = _T("Xbox Ver 1.1/1.2");
					break;
				default:
					sValue = _T("Xbox Ver ");
					sValue += GetXboxVersion();
					// sValue = _T("Xbox Ver unknown");
					break;
			}
			break;
		case GSTR_HDDKEY:
			g_eepromInfo.GetHDDKeyString( szTemp, &dwLength );
			sValue = szTemp;
			break;
		case GSTR_SERIALNUM:
			g_eepromInfo.GetSerialNumberString( szTemp, &dwLength );
			sValue = szTemp;
			sValue.Replace( _T("  "), _T(" ") );
			sValue.Replace( _T("  "), _T(" ") );
			sValue.Replace( _T("  "), _T(" ") );
			sValue.Replace( _T("  "), _T(" ") );
			break;
		case GSTR_VIDSTANDARD:
			if ( IsNTSC() )
			{
				sValue = _T("NTSC");
			}
			else
			{
				sValue = _T("PAL");
			}
			break;
		case GSTR_SHDRV_C_SIZE:
			sValue.Format( _T("C: %s total"), GetDiskTotal('c', 1, true ).c_str() );
			break;
		case GSTR_SHDRV_E_SIZE:
			sValue.Format( _T("E: %s total"), GetDiskTotal('e', 1, true ).c_str() );
			break;
		case GSTR_SHDRV_F_SIZE:
			sValue.Format( _T("F: %s total"), GetDiskTotal('f', 1, true ).c_str() );
			break;
		case GSTR_SHDRV_G_SIZE:
			sValue.Format( _T("G: %s total"), GetDiskTotal('g', 1, true ).c_str() );
			break;
		case GSTR_SHDRV_C_FREE:
			sValue.Format( _T("C: %s free"), GetDiskFree('c', 1, true ).c_str() );
			bReturn = true;
			break;
		case GSTR_SHDRV_E_FREE:
			sValue.Format( _T("E: %s free"), GetDiskFree('e', 1, true ).c_str() );
			bReturn = true;
			break;
		case GSTR_SHDRV_F_FREE:
			sValue.Format( _T("F: %s free"), GetDiskFree('f', 1, true ).c_str() );
			bReturn = true;
			break;
		case GSTR_SHDRV_G_FREE:
			sValue.Format( _T("G: %s free"), GetDiskFree('g', 1, true ).c_str() );
			bReturn = true;
			break;
	}
	return bReturn;
}

class CTextItem
{
public:
	int					m_iUserStringIndex;
	DWORD				m_dwUserStringCheck;
	CStdString			m_sSection;
	CStdString			m_sKey;
	CStdString			m_sString;
	int					m_iStringID;
	CTextItem *			GetPtr() { return this; };
	CTextItem( bool bIsDynamic, LPCTSTR szString, LPCTSTR szSection = NULL, LPCTSTR szKey = NULL, int iUserStringIndex = -1 );
	CTextItem( int iStringID, LPCTSTR szSection = NULL, LPCTSTR szKey = NULL, int iUserStringIndex = -1 );
	bool				GetString( CStdString & sValue );
	CStdString			GetString(void);
};


CTextItem::CTextItem ( int iStringID, LPCTSTR szSection, LPCTSTR szKey, int iUserStringIndex ) :
	m_iUserStringIndex(iUserStringIndex),
	m_dwUserStringCheck(0),
	m_sSection(szSection),
	m_sKey(szKey),
	m_sString(_T("")),
	m_iStringID(iStringID)
{
}

CTextItem::CTextItem ( bool bIsDynamic, LPCTSTR szString, LPCTSTR szSection, LPCTSTR szKey, int iUserStringIndex ) :
	m_iUserStringIndex(iUserStringIndex),
	m_dwUserStringCheck(0),
	m_sSection(szSection),
	m_sKey(szKey),
	m_sString(_T("")),
	m_iStringID(-1)
{
	if ( bIsDynamic )
	{
		if ( szString && _tcslen( szString ) )
		{
			m_iStringID = GetGadgetStringID( szString );
		}
	}
	else
	{
		// It's a constant
		m_sString = szString;
	}
}

CStdString CTextItem::GetString( void )
{
	if ( IsUserStringDefined(m_iUserStringIndex) )
	{
		GetUserString( m_iUserStringIndex, m_sString );
	}
	else if ( m_iStringID > -1 )
	{
		GetGadgetString( m_iStringID, m_sString, m_sSection, m_sKey );
	}
	return m_sString;
}

bool CTextItem::GetString( CStdString & sValue )
{
	bool bReturn = false;

	if ( IsUserStringDefined(m_iUserStringIndex) )
	{
		GetUserString( m_iUserStringIndex, sValue );
	}
	else if ( m_iStringID > -1 )
	{
		bReturn = GetGadgetString( m_iStringID, sValue, m_sSection, m_sKey );

		// If it's a fixed value, don't bother checking any more...
		if ( bReturn == false )
		{
			m_iStringID = -1;
		}
	}
	else
	{
		sValue = m_sString;
	}
	if ( m_sString.Compare( sValue ) )
	{
		m_sString = sValue;
		bReturn = true;
	}
	else
	{
		// No change from last time.
		bReturn = false;
	}
	return bReturn;
}

typedef list<CTextItem *>	TListTextItems;

class CTextGroup
{
	TListTextItems		m_tList;
public:
	CStdString			m_sString;
	bool				m_bDynamic;
	void				Configure( CXMLNode * pNode );
	CStdString			GetString( void );
	void				ClearTextItems( void );
	CTextGroup();
	~CTextGroup();
};

void CTextGroup::ClearTextItems( void )
{
	CTextItem * pTextItem;
	TListTextItems::iterator iterList;

	iterList = m_tList.begin();
	while( iterList != m_tList.end() )
	{
		pTextItem = (*iterList);
		iterList = m_tList.erase( iterList );
		if ( pTextItem )
		{
			delete pTextItem;
		}
	}
}

CTextGroup::~CTextGroup()
{
	ClearTextItems();
}

CTextGroup::CTextGroup() :
	m_bDynamic(false)
{
}

void CTextGroup::Configure( CXMLNode * pNode )
{
	int iIndex = 0;
	CXMLElement * pTextElement;
	CStdString sConstantString;
	bool bIsDynamic;
	bool bIsConstant;
	CStdString sTempVal, sTempName, sKey, sSection;
	int	iStringID;
	int iUserStringID;
//	CTextItem * pTextItem;

	m_bDynamic = false;
	ClearTextItems();
	// <Text Source="NumFTPUsers" />
	while( pTextElement = pNode->GetElement( _T("text"), iIndex++ ) )
	{
		iStringID = -1;
		iUserStringID = pTextElement->GetLong( -1, _T("userstringid") );
		sTempName = pTextElement->GetString( _T(""), _T("source"), true, true );
		if ( sTempName.GetLength() )
		{
			iStringID = GetGadgetStringID( sTempName );
		}
		if ( iStringID == -1 )
		{
			iStringID = pTextElement->GetLong( -1, _T("sourceid") );
		}
		// Must be a constant!
		if ( (iStringID == -1) && (iUserStringID == -1 ))
		{
			sTempVal = pTextElement->GetString( _T(""), _T("constant"), false, false );
			if ( sTempVal.GetLength() == 0 )
			{
				sTempVal = pTextElement->GetString( _T(""), _T("const"), false, false );
				if ( sTempVal.GetLength() == 0 )
				{
					sTempVal = pTextElement->GetString( _T(""), NULL, false, false );
				}
			}
			if ( sTempVal.GetLength() )
			{
				sConstantString += sTempVal;
				bIsConstant = true;
				bIsDynamic= false;
			}
		}
		else
		{
            sSection = pTextElement->GetString( _T(""), _T("section"), true, true );
            sKey = pTextElement->GetString( _T(""), _T("key"), true, true );
			if ( GetGadgetString( iStringID, sTempVal, sSection, sKey ) || (iUserStringID>(-1)) )
			{
				if ( sConstantString.GetLength() )
				{
					// Add Constant to list
					m_tList.push_back(new CTextItem( false, sConstantString, NULL, NULL, -1 ));
				}
				// Have to assume dynamic response on user strings
				m_bDynamic = true;
				sConstantString = _T("");
				bIsDynamic = true;
				// Add Dynamic string to list
				m_tList.push_back(new CTextItem( iStringID, sSection, sKey, iUserStringID ));
				bIsConstant = false;
			}
			else
			{
				sConstantString += sTempVal;
			}
		}
	}
	if ( sConstantString.GetLength() )
	{
		m_tList.push_back(new CTextItem( false, sConstantString, NULL, NULL, -1 ));
		if ( m_tList.size() == 1 )
		{
			m_sString = sConstantString;
		}
	}
}

CStdString			CTextGroup::GetString( void )
{
	CTextItem * pTextItem;
	TListTextItems::iterator iterList;

	if ( m_bDynamic )
	{
		m_sString = _T("");
		iterList = m_tList.begin();
		while( iterList != m_tList.end() )
		{
			pTextItem = (*iterList);
			if ( pTextItem )
			{
				m_sString += pTextItem->GetString();
			}
			iterList++;
		}
	}
	return m_sString;
}

class CRenderText : public CRenderGadget
{
protected:
	//CModClrThrob		m_modClrThrobShadow;
	//CModClrThrob		m_modClrThrobText;
	//CModClrFlicker		m_modClrFlkrShadow;
	//CModClrFlicker		m_modClrFlkrText;

	CCaratInfo			m_caratInfo;

	CColorModHandler	m_colorHandlerShadow;
	CColorModHandler	m_colorHandler;

	CModShifter			m_shifterX;
	CModShifter			m_shifterY;

	CStdString			m_sSection;
	CStdString			m_sKey;
	int					m_iUserStringID;
	DWORD				m_dwUserStringCheck;



	bool				m_bWordWrapEnable;
	int					m_iWordWrapWidth;
	DWORD				m_dwShadowColor;
	DWORD				m_dwTextColor;
	RECT				m_rectModBounce;
	RECT				m_rectModTremor;
	CModPosBounce		m_modBouncePos;
	CModPosTremor		m_modTremorPos;
	CModSizePulse		m_modPulseSize;
	CModSizeSpaz		m_modSpazSize;
	bool				m_bGlow;
	CStdString			m_sString;
	CStdString			m_sFont;
	int					m_iStringID;
	int					m_iShadowSize;
	bool				m_bDynamic;
	bool				m_bDrawIt;
	bool				m_bCached;
	bool				m_bNeedsSetup;
	CXBFont			*	m_pfnt;
	CImageSource		m_imgBackTexture;
	bool				m_bIsMulti;
	CTextGroup			m_tGroup;
	int					m_iXOffset;
	int					m_iYOffset;
	bool				m_bWrapX;
	bool				m_bWrapY;

	bool				m_bIsMenu;
	bool				m_bHorizontalMenu;

	void	SetupValues( void );
//	void	CleanInvalidChars(void);

	virtual void		InternalInitialize( void );
	virtual void		InternalDestroy( void );
	virtual void		InternalRender( void );
	virtual void		InternalConfigure( CXMLNode * pNode );
public:
	virtual	void OnActivation( void );
	virtual	void OnDeactivation( void );
	CRenderText();
	~CRenderText();
};


//void CRenderText::CleanInvalidChars(void)
//{
//	WCHAR * pwszString;
//
//	pwszString = new WCHAR[m_sString.GetLength()+4];
//	if ( pwszString )
//	{
//		CStdStringW sString(m_sString);
//		wcscpy( pwszString, sString );
//		if ( m_pfnt )
//		{
//			m_pfnt->ReplaceInvalidChars(pwszString, L' ' );
//			m_sString = pwszString;
//		}
//	}
//}

void CRenderText::InternalInitialize( void )
{
	m_iXOffset = 0;
	m_iYOffset = 0;
	m_caratInfo.m_iCaratPos = -1;
//	m_bWrap = false;
	if ( m_sFont.GetLength() == 0  )
	{
		m_sFont = _T("default");
	}
	CXBFont * pFont = g_FontCache.GetFontPtrByResname( m_sFont );
	if ( pFont )
	{
		m_pfnt = pFont;
	}
	else
	{
		// Last resort, if font cache fails
		m_pfnt = &GetStateApp()->m_Font16;
	}
	m_imgBackTexture.Initialize( m_pDevice, GetStateApp()->GetSoundDevice() );
	if ( m_bNeedsSetup )
	{
		SetupValues();
	}
}

void CRenderText::InternalDestroy( void )
{
}

void CRenderText::SetupValues( void )
{
	FLOAT fWidth, fHeight;
	// int iLeft, iRight;
	int iWidth, iHeight;
	int iTexWidth = 0, iTexHeight = 0;
	LPDIRECT3DTEXTURE8 pTexture;

	CleanInvalidChars( m_pfnt, m_sString );

	if ( m_sString.GetLength() )
	{
		m_bCached = true;
		m_bDrawIt = true;

		m_pfnt->GetTextExtent( m_sString, &fWidth, &fHeight );
		iWidth = (int)(fWidth+0.5);
		iHeight = (int)(fHeight+0.5);

		if ( m_bWordWrapEnable )
		{
			if ( m_iWordWrapWidth < 1 )
			{
				m_iWordWrapWidth = m_iWidth;
				if ( m_iWordWrapWidth != -1 )
				{
					iWidth = m_iWordWrapWidth;
					if ( m_iHeight != -1 )
					{
						iHeight = m_iHeight;
					}
					m_pfnt->WrapString( m_sString, iWidth );
				}
			}
			else
			{
				iWidth = m_iWordWrapWidth;
				if ( m_iHeight < 2 )
				{
					m_iHeight = 10;
				}
				iHeight = m_iHeight*iWidth;
				iHeight /= m_iWidth;
				m_pfnt->WrapString( m_sString, iWidth );
			}
		}
		if ( m_dwShadowColor )
		{
			iWidth += m_iShadowSize;
			iHeight += m_iShadowSize;
			if ( m_bGlow )
			{
				iWidth += m_iShadowSize;
				iHeight += m_iShadowSize;
			}
		}
		//if ( iWidth < m_iWidth && !m_bScaleX )
		//{
		//	iWidth = m_iWidth;
		//}
		//if ( iHeight < m_iHeight && !m_bScaleY )
		//{
		//	iHeight = m_iHeight;
		//}
		MakeRects( iWidth, iHeight );
		pTexture = m_imgBackTexture.GetTexture();
		if ( pTexture )
		{
			GetTextureSize( pTexture, iTexWidth, iTexHeight );
		}
		if ( ( iTexHeight < iHeight ) ||
			 ( iTexWidth < iWidth ) )
		{
			m_imgBackTexture.MakeBlank( iWidth, iHeight );
		}
		if ( iWidth < m_iWidth )
		{
			iWidth = m_iWidth;
		}
		if( iHeight < m_iHeight )
		{
			iHeight = m_iHeight;
		}
		m_shifterX.SetLength( iWidth+10 );
		m_shifterY.SetLength( iHeight );
	}
	else
	{
		m_bDrawIt = false;
	}
	m_bNeedsSetup = false;
}

void DrawAttributedText( CXBFont * pFont, CTextAttributes * pAttrSet, int iX, int iY, int iWidth, LPCTSTR szValue, CCaratInfo * pCaratInfo /* = NULL */ )
{
	FLOAT fOffsetX = (FLOAT)iX;
	FLOAT fOffsetY = (FLOAT)iY;
	FLOAT fOffset = 0.0;

	FLOAT fWidth, fHeight;
	FLOAT fLeftPad = 0.0f;
	FLOAT fRightPad = 0.0f;


	if ( pFont && pAttrSet && szValue && _tcslen( szValue ) )
	{
		if ( pAttrSet->m_dwShadowColor & 0xff000000 )
		{
			fRightPad = (FLOAT)pAttrSet->m_iShadowSize;
			if ( pAttrSet->m_bDoGlow )
			{
				fLeftPad = (FLOAT)pAttrSet->m_iShadowSize;
			}
		}

		pFont->GetTextExtent( szValue, &fWidth, &fHeight );
		fWidth += (fLeftPad+fRightPad);
		switch ( pAttrSet->m_iJustify )
		{
			case 0: // Center
				fOffsetX = (((FLOAT)iWidth-fWidth)/2.0f)+fLeftPad;
				fOffsetX += (FLOAT)iX;
				break;
			case 1: // Right
				fOffsetX = ((FLOAT)iWidth-fWidth)+fLeftPad;
				fOffsetX += (FLOAT)iX;
				break;
		}
		if ( pAttrSet->m_dwShadowColor & 0xff000000 )
		{
			if ( pAttrSet->m_bDoGlow )
			{
				DWORD dwShadowColor = pAttrSet->GetShadowColor();
				//DWORD dwShadowColor = 192+(rand()%64);
				//dwShadowColor <<= 24;
				//dwShadowColor |= (pAttrSet->m_dwShadowColor&0x00ffffff);

				FLOAT fRand = (FLOAT)(RandomNum()%(((pAttrSet->m_iShadowSize-1)*100)+1));
				fOffset = (FLOAT)pAttrSet->m_iShadowSize;
							
				fRand = (FLOAT)pAttrSet->m_iShadowSize;

				pFont->DrawText( fOffsetX+fOffset+fRand, fOffsetY+fOffset+fRand, dwShadowColor, szValue );
				pFont->DrawText( fOffsetX+fOffset+fRand, fOffsetY+fOffset-fRand, dwShadowColor, szValue );
				pFont->DrawText( fOffsetX+fOffset-fRand, fOffsetY+fOffset-fRand, dwShadowColor, szValue );
				pFont->DrawText( fOffsetX+fOffset-fRand, fOffsetY+fOffset+fRand, dwShadowColor, szValue );
				pFont->DrawText( fOffsetX+fOffset, fOffsetY+fOffset+fRand, dwShadowColor, szValue );
				pFont->DrawText( fOffsetX+fOffset+fRand, fOffsetY+fOffset, dwShadowColor, szValue );
				pFont->DrawText( fOffsetX+fOffset, fOffsetY+fOffset-fRand, dwShadowColor, szValue );
				pFont->DrawText( fOffsetX+fOffset-fRand, fOffsetY+fOffset, dwShadowColor, szValue );
			}
			else
			{
				// pFont->DrawText( fOffsetX+(FLOAT)pAttrSet->m_iShadowSize, fOffsetY+(FLOAT)pAttrSet->m_iShadowSize, pAttrSet->m_dwShadowColor, szValue );
				pFont->DrawText( fOffsetX+(FLOAT)pAttrSet->m_iShadowSize, fOffsetY+(FLOAT)pAttrSet->m_iShadowSize, pAttrSet->GetShadowColor(), szValue );
			}
		}
		pFont->DrawText( fOffsetX+fOffset, fOffsetY+fOffset, pAttrSet->GetTextColor(), szValue, 0, pCaratInfo );
		// pFont->DrawText( fOffsetX+fOffset, fOffsetY+fOffset, pAttrSet->m_dwTextColor, szValue );
	}
}

void CRenderText::InternalRender( void )
{
	CStdString sText;
	DWORD dwShadow, dwText;

	if ( m_bNeedsSetup == true )
	{
		SetupValues();
	}

	if ( m_bIsMenu )
	{
		if ( g_MenuInfo.m_iCurrentItem > 0 )
		{
			g_MenuInfo.m_bItemPrev = true;
		}
		else
		{
			g_MenuInfo.m_bItemPrev = false;
		}
		if ( (g_MenuInfo.m_iCurrentItem+1) < (int)g_MenuInfo.GetItemCount() )
		{
			g_MenuInfo.m_bItemNext = true;
		}
		else
		{
			g_MenuInfo.m_bItemNext = false;
		}
	}

	if ( m_iStringID >= -1 )
	{
		sText = m_sString;
		if ( m_bDynamic )
		{
			if ( m_bIsMulti )
			{
				m_sString = m_tGroup.GetString();
			}
			else
			{
				if ( m_iUserStringID > (-1) )
				{
					if ( IsUserStringDefined(m_iUserStringID) )
					{
						GetUserString( m_iUserStringID, m_sString );
					}
					else
					{
						if ( m_iStringID > (-1) )
						{
							GetGadgetString( m_iStringID, m_sString, m_sSection, m_sKey );
						}
					}
				}
				else
				{
					GetGadgetString( m_iStringID, m_sString, m_sSection, m_sKey );
				}
			}
			if ( m_sString.Compare( sText ) )
			{
				sText = m_sString;
				SetupValues();
				m_modBouncePos.SetSize(m_rectDest.right-m_rectDest.left,m_rectDest.bottom-m_rectDest.top);
			}
		}

		if ( m_bDrawIt )
		{
			if ( m_imgBackTexture.GetTexture() )
			{
				{
					dwShadow = m_colorHandlerShadow.ModifyColor(m_dwShadowColor);
					dwText = m_colorHandler.ModifyColor(m_dwTextColor);
					//dwShadow = m_modClrThrobShadow.ModifyColor(m_dwShadowColor);
					//dwText = m_modClrThrobText.ModifyColor(m_dwTextColor);
					//dwShadow = m_modClrFlkrShadow.ModifyColor(dwShadow);
					//dwText = m_modClrFlkrText.ModifyColor(dwText);

					CXBEasel easel(m_pDevice, m_imgBackTexture.GetTexture());

					CTextAttributes attrSet;

					attrSet.m_dwTextColor = dwText;
					attrSet.m_dwShadowColor = dwShadow;
					attrSet.m_dwBackColor = 0;
					attrSet.m_dwBorderColor = 0;
					attrSet.m_bDoGlow = m_bGlow;
					attrSet.m_iShadowSize = m_iShadowSize;
					attrSet.m_iJustify = -1; // m_iJustifyX; // -1;
					DrawAttributedText(m_pfnt, &attrSet, 0, 0, m_iWidth, m_sString, &m_caratInfo );

					// If there's a position marked, then draw it
//					if ( m_caratInfo.m_iCaratPos >= 0 )
//					{
//						DrawCarat( m_pDevice, &(m_caratInfo.m_rcCaratRect),
  //                          0, 0xffffffff, 0 );
	//				}
					// DrawAttributedText(m_pfnt, &attrSet, m_iXOffset, m_iYOffset, m_iWidth, m_sString );
					//if ( m_iXOffset && m_shifterX.GetWrap() )
					//{
					//	if ( m_iXOffset < 0 )
					//	{
					//		DrawAttributedText(m_pfnt, &attrSet, m_iWidth+m_iXOffset, m_iYOffset, m_iWidth, m_sString );
					//	}
					//	else
					//	{
					//		DrawAttributedText(m_pfnt, &attrSet, m_iXOffset-m_iWidth, m_iYOffset, m_iWidth, m_sString );
					//	}
					//}
					//if ( m_iYOffset && m_shifterY.GetWrap() )
					//{
					//	if ( m_iYOffset < 0 )
					//	{
					//		DrawAttributedText(m_pfnt, &attrSet, m_iXOffset, m_iHeight+m_iYOffset, m_iWidth, m_sString );
					//	}
					//	else
					//	{
					//		DrawAttributedText(m_pfnt, &attrSet, m_iXOffset, m_iYOffset-m_iHeight, m_iWidth, m_sString );
					//	}
					//}
				}

				RECT rectNewDest;

				memcpy( &rectNewDest, &m_rectDest, sizeof(RECT) );
				if ( m_modBouncePos.ModifyPosRect( rectNewDest, m_rectModBounce) )
				{
					memcpy(&rectNewDest, &m_rectModBounce, sizeof(RECT) );
				}
				if ( m_modTremorPos.ModifyPosRect( rectNewDest, m_rectModTremor) )
				{
					memcpy(&rectNewDest, &m_rectModTremor, sizeof(RECT) );
				}
				if ( m_modPulseSize.ModifyPosRect( rectNewDest, m_rectModTremor) )
				{
					memcpy(&rectNewDest, &m_rectModTremor, sizeof(RECT) );
				}
				if ( m_modSpazSize.ModifyPosRect( rectNewDest, m_rectModTremor) )
				{
					memcpy(&rectNewDest, &m_rectModTremor, sizeof(RECT) );
				}
				RECT rc1, rc2, rc3, rc4;


				m_iXOffset = m_shifterX.GetOffset( GetTickCount() );
				m_iYOffset = m_shifterY.GetOffset( GetTickCount() );
				int iXOffset2, iYOffset2;

				iXOffset2 = m_iXOffset;
				iYOffset2 = m_iYOffset;
				if ( m_iXOffset < 0 )
				{
					iXOffset2 += (m_rectSrc.right-m_rectSrc.left);
				}
				else
				{
					iXOffset2 -= (m_rectSrc.right-m_rectSrc.left);
				}
				if ( m_iYOffset < 0 )
				{
					iYOffset2 += (m_rectSrc.bottom-m_rectSrc.top);
				}
				else
				{
					iYOffset2 -= (m_rectSrc.bottom-m_rectSrc.top);
				}
				if ( m_iXOffset )
				{
					if ( m_iYOffset )
					{
						// Possibly all four?
						if ( m_shifterY.GetWrap() )
						{
							if ( m_shifterX.GetWrap() )
							{
								// All four!
								rc1.bottom = m_rectSrc.bottom+m_iYOffset;
								rc2.bottom = m_rectSrc.bottom+m_iYOffset;
								rc1.top = m_rectSrc.top+m_iYOffset;
								rc2.top = m_rectSrc.top+m_iYOffset;
								rc1.left = m_rectSrc.left+m_iXOffset;
								rc2.left = m_rectSrc.left+iXOffset2;
								rc1.right = m_rectSrc.right+m_iXOffset;
								rc2.right = m_rectSrc.right+iXOffset2;
								rc3.bottom = m_rectSrc.bottom+iYOffset2;
								rc4.bottom = m_rectSrc.bottom+iYOffset2;
								rc3.top = m_rectSrc.top+iYOffset2;
								rc4.top = m_rectSrc.top+iYOffset2;
								rc3.left = m_rectSrc.left+m_iXOffset;
								rc4.left = m_rectSrc.left+iXOffset2;
								rc3.right = m_rectSrc.right+m_iXOffset;
								rc4.right = m_rectSrc.right+iXOffset2;
								DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc1, -1, -1, TRUE, TRUE );
								DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc2, -1, -1, TRUE, TRUE  );
								DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc3, -1, -1, TRUE, TRUE  );
								DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc4, -1, -1, TRUE, TRUE  );
							}
							else
							{
								// Only two for Y
								rc1.bottom = m_rectSrc.bottom+m_iYOffset;
								rc1.top = m_rectSrc.top+m_iYOffset;
								rc1.left = m_rectSrc.left+m_iXOffset;
								rc1.right = m_rectSrc.right+m_iXOffset;
								rc3.bottom = m_rectSrc.bottom+iYOffset2;
								rc3.top = m_rectSrc.top+iYOffset2;
								rc3.left = m_rectSrc.left+m_iXOffset;
								rc3.right = m_rectSrc.right+m_iXOffset;
								DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc1, -1, -1, TRUE, TRUE  );
								DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc3, -1, -1, TRUE, TRUE  );
							}
						}
						else
						{
							if ( m_shifterX.GetWrap() )
							{
								rc1.bottom = m_rectSrc.bottom+m_iYOffset;
								rc2.bottom = m_rectSrc.bottom+m_iYOffset;
								rc1.top = m_rectSrc.top+m_iYOffset;
								rc2.top = m_rectSrc.top+m_iYOffset;
								rc1.left = m_rectSrc.left+m_iXOffset;
								rc2.left = m_rectSrc.left+iXOffset2;
								rc1.right = m_rectSrc.right+m_iXOffset;
								rc2.right = m_rectSrc.right+iXOffset2;
								DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc1, -1, -1, TRUE, TRUE  );
								DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc2, -1, -1, TRUE, TRUE  );
							}
							else
							{
								rc1.bottom = m_rectSrc.bottom+m_iYOffset;
								rc1.top = m_rectSrc.top+m_iYOffset;
								rc1.left = m_rectSrc.left+m_iXOffset;
								rc1.right = m_rectSrc.right+m_iXOffset;
								DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc1, -1, -1, TRUE, TRUE  );
							}
						}
					}
					else
					{
						// Possibly  two?
						if ( m_shifterX.GetWrap() )
						{
							rc1.bottom = m_rectSrc.bottom+m_iYOffset;
							rc2.bottom = m_rectSrc.bottom+m_iYOffset;
							rc1.top = m_rectSrc.top+m_iYOffset;
							rc2.top = m_rectSrc.top+m_iYOffset;
							rc1.left = m_rectSrc.left+m_iXOffset;
							rc2.left = m_rectSrc.left+iXOffset2;
							rc1.right = m_rectSrc.right+m_iXOffset;
							rc2.right = m_rectSrc.right+iXOffset2;
							DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc1, -1, -1, TRUE, TRUE  );
							DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc2, -1, -1, TRUE, TRUE  );
						}
						else
						{
							rc1.bottom = m_rectSrc.bottom+m_iYOffset;
							rc1.top = m_rectSrc.top+m_iYOffset;
							rc1.left = m_rectSrc.left+m_iXOffset;
							rc1.right = m_rectSrc.right+m_iXOffset;
							DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc1, -1, -1, TRUE, TRUE  );
						}
					}
				}
				else
				{
					if ( m_iYOffset )
					{
						if ( m_shifterY.GetWrap() )
						{
							rc1.bottom = m_rectSrc.bottom+m_iYOffset;
							rc1.top = m_rectSrc.top+m_iYOffset;
							rc1.left = m_rectSrc.left+m_iXOffset;
							rc1.right = m_rectSrc.right+m_iXOffset;
							rc3.bottom = m_rectSrc.bottom+iYOffset2;
							rc3.top = m_rectSrc.top+iYOffset2;
							rc3.left = m_rectSrc.left+m_iXOffset;
							rc3.right = m_rectSrc.right+m_iXOffset;
							DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc1, -1, -1, TRUE, TRUE  );
							DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc3, -1, -1, TRUE, TRUE  );
						}
						else
						{
							rc1.bottom = m_rectSrc.bottom+m_iYOffset;
							rc1.top = m_rectSrc.top+m_iYOffset;
							rc1.left = m_rectSrc.left+m_iXOffset;
							rc1.right = m_rectSrc.right+m_iXOffset;
							DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, rc1, -1, -1, TRUE, TRUE  );
						}
					}
					else
					{
						DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, m_rectSrc, -1, -1, TRUE, TRUE  );
					}
				}
				// DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, m_rectSrc, -1, -1, true, false );
			}
		}
	}
}

void CRenderText::InternalConfigure( CXMLNode * pNode )
{
	CStdString sToken;
	CXMLNode * pMultiNode;

	m_bIsMulti = false;

	m_dwShadowColor = pNode->GetDWORD( NULL, _T("ShadowColor"), 0 );
	m_iShadowSize = pNode->GetLong( NULL, _T("ShadowSize"), 2 );
	if ( m_iShadowSize < 1 )
	{
		m_iShadowSize = 1;
	}
	m_dwTextColor =  pNode->GetDWORD( NULL, _T("TextColor"), 0xFFFFFFFF );
	m_bGlow = (pNode->GetBool( NULL, _T("doglow"), FALSE ) == TRUE);

	m_sFont = pNode->GetString( NULL, _T("font"), _T("default"), true );

	m_bIsMenu = (pNode->GetBool( NULL, _T("IsMenu"), FALSE) == TRUE );
	m_bHorizontalMenu = (pNode->GetBool( _T("IsMenu"), _T("Horizontal"), TRUE ) == TRUE );

	m_bWordWrapEnable  = (pNode->GetBool( NULL, _T("WordWrapEnable"), FALSE) == TRUE );
	if ( m_bWordWrapEnable )
	{
		m_iWordWrapWidth = pNode->GetLong( NULL, _T("WordWrapWidth"), 2 );
	}

	if ( pMultiNode = pNode->GetNode( _T("multi") ) )
	{
		m_bIsMulti = true;
		m_tGroup.Configure( pMultiNode );
		m_bDynamic = m_tGroup.m_bDynamic;
		if ( m_bDynamic == false )
		{
			m_sString = m_tGroup.GetString();
		}
	}
	else
	{
//		int iUserStringID;

		sToken = pNode->GetString( NULL, _T("source"), _T(""), true );

		m_iUserStringID = pNode->GetLong( NULL, _T("userstringid"), -1 );

		m_sSection = pNode->GetString( NULL, _T("section"), _T(""), true );
		m_sKey = pNode->GetString( NULL, _T("key"), _T(""), true );


		if ( sToken.GetLength() )
		{
			m_iStringID = GetGadgetStringID( sToken );
		}
		else
		{
			m_iStringID = pNode->GetLong( NULL, _T("sourceid"), -1 );
		}
		if ( m_iStringID >= 0 )
		{
			m_bDynamic = GetGadgetString( m_iStringID, m_sString, m_sSection, m_sKey );
		}
		else
		{
			m_bDynamic = false;
			m_sString = pNode->GetString( NULL, _T("constant"), _T(""), true );
			if ( m_sString.GetLength() )
			{
				m_iStringID = -1;
			}
			else
			{
				// Do not display this gadget... we have no string.
				m_iStringID = -2;
			}
		}
		if ( m_iUserStringID > (-1) )
		{
			m_bDynamic = true;
		}
	}

	CXMLNode * pModifierNodes = NULL;
	int iModNodeCount = 0;
	CStdString sModifierType, sTarget;

	m_modBouncePos.SetEnable(false);
	m_modTremorPos.SetEnable(false);

	while( pModifierNodes = pNode->GetNode( _T("modifier"), iModNodeCount++ ) )
	{
		sModifierType = pModifierNodes->GetString( NULL, _T("type"), _T(""), true );
		sModifierType.MakeLower();
		if ( sModifierType.Compare( _T("bounce") ) == 0 )
		{
			m_modBouncePos.Configure(pModifierNodes);
			m_modBouncePos.SetEnable(true);
		}
		else if ( sModifierType.Compare( _T("tremor") ) == 0 )
		{
			m_modTremorPos.Configure(pModifierNodes);
			m_modTremorPos.SetEnable(true);
		}
		else if ( sModifierType.Compare( _T("spaz") ) == 0 )
		{
			m_modSpazSize.Configure(pModifierNodes);
			m_modSpazSize.SetEnable(true);
		}
		else if ( sModifierType.Compare( _T("pulse") ) == 0 )
		{
			m_modPulseSize.Configure(pModifierNodes);
			m_modPulseSize.SetEnable(true);
		}
		else if ( sModifierType.Compare( _T("shifter") ) == 0 )
		{
			sTarget = pModifierNodes->GetString( NULL, _T("target"), _T("x"), true );
			if ( _tcsicmp( sTarget, _T("y") ) == 0 )
			{
				m_shifterY.Configure(pModifierNodes);
				m_shifterY.SetEnable(true);
			}
			else
			{
				m_shifterX.Configure(pModifierNodes);
				m_shifterX.SetEnable(true);
			}
		}
		// m_shifterX
		else
		{
			m_colorHandler.Configure( pModifierNodes, _T("textcolor") );
			m_colorHandlerShadow.Configure( pModifierNodes, _T("shadowcolor") );
		}

		//else if ( sModifierType.Compare( _T("throb") ) == 0 )
		//{
		//	sTarget = pModifierNodes->GetString( NULL, _T("target"), _T("") );
		//	sTarget.MakeLower();
		//	if ( sTarget.Compare( _T("textcolor") ) == 0 )
		//	{
		//		m_modClrThrobText.Configure(pModifierNodes);
		//		m_modClrThrobText.SetEnable(true);
		//	}
		//	else if ( sTarget.Compare( _T("shadowcolor") ) == 0 )
		//	{
		//		m_modClrThrobShadow.Configure(pModifierNodes);
		//		m_modClrThrobShadow.SetEnable(true);
		//	}
		//}
		//else if ( sModifierType.Compare( _T("flicker") ) == 0 )
		//{
		//	sTarget = pModifierNodes->GetString( NULL, _T("target"), _T("") );
		//	sTarget.MakeLower();
		//	if ( sTarget.Compare( _T("textcolor") ) == 0 )
		//	{
		//		m_modClrFlkrText.Configure(pModifierNodes);
		//		m_modClrFlkrText.SetEnable(true);
		//	}
		//	else if ( sTarget.Compare( _T("shadowcolor") ) == 0 )
		//	{
		//		m_modClrFlkrShadow.Configure(pModifierNodes);
		//		m_modClrFlkrShadow.SetEnable(true);
		//	}
		//}
	}

	m_bNeedsSetup = true;
}

void CRenderText::OnActivation( void )
{
	DEBUG_FORMAT( _T("Activating Text Gadget 0x%08x"), this );
	if ( m_bIsMenu )
	{
		if ( m_bHorizontalMenu )
		{
			g_MenuInfo.m_bMenuOrientedUpDown = false;
		}
		else
		{
			g_MenuInfo.m_bMenuOrientedUpDown = true;
		}
	}
}

void CRenderText::OnDeactivation( void )
{
}

CRenderText::CRenderText() :
	m_pfnt(NULL),
	m_bHorizontalMenu(true),
	m_bIsMenu(false),
	m_bNeedsSetup(false),
	m_bCached(false),
	m_dwShadowColor(0),
	m_dwTextColor(0),
	m_bGlow(false),
	m_iStringID(0),
	m_iShadowSize(0),
	m_bDynamic(false),
	m_bDrawIt(false),
	m_iUserStringID(-1),
	m_dwUserStringCheck(0),
	m_bWordWrapEnable(false),
	m_iWordWrapWidth(-1)
{
}

CRenderText::~CRenderText()
{
}


class CRGHorzMenu : public CRenderGadget
{
protected:
	CColorModHandler	m_colorHandlerShadow;
	CColorModHandler	m_colorHandler;

	DWORD				m_dwShadowColor;
	DWORD				m_dwTextColor;
	RECT				m_rectModBounce;
	RECT				m_rectModTremor;
	CModPosBounce		m_modBouncePos;
	CModPosTremor		m_modTremorPos;
	CModSizePulse		m_modPulseSize;
	CModSizeSpaz		m_modSpazSize;
	bool				m_bGlow;
	CStdString			m_sString;
	CStdString			m_sFont;
	int					m_iStringID;
	int					m_iShadowSize;
	bool				m_bDynamic;
	bool				m_bDrawIt;
	bool				m_bCached;
	bool				m_bNeedsSetup;
	CXBFont			*	m_pfnt;
	CImageSource		m_imgBackTexture;

	void	SetupValues( void );
//	void	CleanInvalidChars(void);

	virtual void		InternalInitialize( void );
	virtual void		InternalDestroy( void );
	virtual void		InternalRender( void );
	virtual void		InternalConfigure( CXMLNode * pNode );
public:
	virtual	void OnActivation( void );
	virtual	void OnDeactivation( void );
	CRGHorzMenu();
	~CRGHorzMenu();
};


void CRGHorzMenu::InternalInitialize( void )
{
	if ( m_sFont.GetLength() == 0  )
	{
		m_sFont = _T("default");
	}
	CXBFont * pFont = g_FontCache.GetFontPtrByResname( m_sFont );
	if ( pFont )
	{
		m_pfnt = pFont;
	}
	else
	{
		// Last resort, if font cache fails
		m_pfnt = &GetStateApp()->m_Font16;
	}
	m_imgBackTexture.Initialize( m_pDevice, GetStateApp()->GetSoundDevice() );
	if ( m_bNeedsSetup )
	{
		SetupValues();
	}
}

void CRGHorzMenu::InternalDestroy( void )
{
}

void CRGHorzMenu::SetupValues( void )
{
	FLOAT fWidth, fHeight;
	// int iLeft, iRight;
	int iWidth, iHeight;
	int iTexWidth = 0, iTexHeight = 0;
	LPDIRECT3DTEXTURE8 pTexture;

	CleanInvalidChars( m_pfnt, m_sString );

	if ( m_sString.GetLength() )
	{
		m_bCached = true;
		m_bDrawIt = true;
		m_pfnt->GetTextExtent( m_sString, &fWidth, &fHeight );

		iWidth = (int)(fWidth+0.5);
		iHeight = (int)(fHeight+0.5);
		if ( m_dwShadowColor )
		{
			iWidth += m_iShadowSize;
			iHeight += m_iShadowSize;
			if ( m_bGlow )
			{
				iWidth += m_iShadowSize;
				iHeight += m_iShadowSize;
			}
		}
		MakeRects( iWidth, iHeight );
		pTexture = m_imgBackTexture.GetTexture();
		if ( pTexture )
		{
			GetTextureSize( pTexture, iTexWidth, iTexHeight );
		}
		if ( ( iTexHeight < iHeight ) ||
			 ( iTexWidth < iWidth ) )
		{
			m_imgBackTexture.MakeBlank( iWidth, iHeight );
		}
	}
	else
	{
		m_bDrawIt = false;
	}
	m_bNeedsSetup = false;
}


void CRGHorzMenu::InternalRender( void )
{
	CStdString sText;
	DWORD dwShadow, dwText;

	if ( m_bNeedsSetup == true )
	{
		SetupValues();
	}
	if ( m_iStringID >= -1 )
	{
		sText = m_sString;
		if ( m_bDynamic )
		{
			GetGadgetString( m_iStringID, m_sString ); // , m_sSection, m_sKey );
			if ( m_sString.Compare( sText ) )
			{
				sText = m_sString;
				SetupValues();
			}
		}

		if ( m_bDrawIt )
		{
			if ( m_imgBackTexture.GetTexture() )
			{
				{
					dwShadow = m_colorHandlerShadow.ModifyColor(m_dwShadowColor);
					dwText = m_colorHandler.ModifyColor(m_dwTextColor);
					//dwShadow = m_modClrThrobShadow.ModifyColor(m_dwShadowColor);
					//dwText = m_modClrThrobText.ModifyColor(m_dwTextColor);
					//dwShadow = m_modClrFlkrShadow.ModifyColor(dwShadow);
					//dwText = m_modClrFlkrText.ModifyColor(dwText);
					FLOAT fOffset = 0.0f;
					CXBEasel easel(m_pDevice, m_imgBackTexture.GetTexture());
					// if ( m_dwShadowColor & 0xff000000 )
					if ( dwShadow & 0xff000000 )
					{
						if ( m_bGlow )
						{
//							DWORD dwShadowColor = 192+(rand()%64);
//							dwShadowColor <<= 24;
//							dwShadowColor |= (m_dwShadowColor&0x00ffffff);
//
							FLOAT fRand = (FLOAT)(RandomNum()%(((m_iShadowSize-1)*100)+1));
							fOffset = (FLOAT)m_iShadowSize;
							
							fRand = (FLOAT)m_iShadowSize;

							m_pfnt->DrawText( fOffset+fRand, fOffset+fRand, dwShadow, m_sString );
							m_pfnt->DrawText( fOffset+fRand, fOffset-fRand, dwShadow, m_sString );
							m_pfnt->DrawText( fOffset-fRand, fOffset-fRand, dwShadow, m_sString );
							m_pfnt->DrawText( fOffset-fRand, fOffset+fRand, dwShadow, m_sString );
							m_pfnt->DrawText( fOffset, fOffset+fRand, dwShadow, m_sString );
							m_pfnt->DrawText( fOffset+fRand, fOffset, dwShadow, m_sString );
							m_pfnt->DrawText( fOffset, fOffset-fRand, dwShadow, m_sString );
							m_pfnt->DrawText( fOffset-fRand, fOffset, dwShadow, m_sString );
						}
						else
						{
							m_pfnt->DrawText( (FLOAT)m_iShadowSize, (FLOAT)m_iShadowSize, dwShadow, m_sString );
						}
					}
					m_pfnt->DrawText( fOffset, fOffset, dwText, m_sString );
				}

				RECT rectNewDest;

				memcpy( &rectNewDest, &m_rectDest, sizeof(RECT) );
				if ( m_modBouncePos.ModifyPosRect( rectNewDest, m_rectModBounce) )
				{
					memcpy(&rectNewDest, &m_rectModBounce, sizeof(RECT) );
				}
				if ( m_modTremorPos.ModifyPosRect( rectNewDest, m_rectModTremor) )
				{
					memcpy(&rectNewDest, &m_rectModTremor, sizeof(RECT) );
				}
				if ( m_modPulseSize.ModifyPosRect( rectNewDest, m_rectModTremor) )
				{
					memcpy(&rectNewDest, &m_rectModTremor, sizeof(RECT) );
				}
				if ( m_modSpazSize.ModifyPosRect( rectNewDest, m_rectModTremor) )
				{
					memcpy(&rectNewDest, &m_rectModTremor, sizeof(RECT) );
				}
				DrawSpriteRects( m_pDevice, m_imgBackTexture.GetTexture(), rectNewDest, m_rectSrc, -1, -1, true, false );
			}
		}
	}
}

void CRGHorzMenu::InternalConfigure( CXMLNode * pNode )
{
	CStdString sToken;
	m_dwShadowColor = pNode->GetDWORD( NULL, _T("ShadowColor"), 0 );
	m_iShadowSize = pNode->GetLong( NULL, _T("ShadowSize"), 2 );
	if ( m_iShadowSize < 1 )
	{
		m_iShadowSize = 1;
	}
	m_dwTextColor =  pNode->GetDWORD( NULL, _T("TextColor"), 0xFFFFFFFF );
	m_bGlow = (pNode->GetBool( NULL, _T("doglow"), FALSE ) == TRUE);

	sToken = pNode->GetString( NULL, _T("source"), _T(""), true );

	m_sFont = pNode->GetString( NULL, _T("font"), _T("default"), true );

	if ( sToken.GetLength() )
	{
		m_iStringID = GetGadgetStringID( sToken );
	}
	else
	{
		m_iStringID = pNode->GetLong( NULL, _T("sourceid"), -1 );
	}
	if ( m_iStringID >= 0 )
	{
		m_bDynamic = GetGadgetString( m_iStringID, m_sString );
	}
	else
	{
		m_bDynamic = false;
		m_sString = pNode->GetString( NULL, _T("constant"), _T(""), true );
		if ( m_sString.GetLength() )
		{
			m_iStringID = -1;
		}
		else
		{
			// Do not display this gadget... we have no string.
			m_iStringID = -2;
		}
	}

	CXMLNode * pModifierNodes = NULL;
	int iModNodeCount = 0;
	CStdString sModifierType, sTarget;

	m_modBouncePos.SetEnable(false);
	m_modTremorPos.SetEnable(false);

	while( pModifierNodes = pNode->GetNode( _T("modifier"), iModNodeCount++ ) )
	{
		sModifierType = pModifierNodes->GetString( NULL, _T("type"), _T(""), true );
		sModifierType.MakeLower();
		if ( sModifierType.Compare( _T("bounce") ) == 0 )
		{
			m_modBouncePos.Configure(pModifierNodes);
			m_modBouncePos.SetEnable(true);
		}
		else if ( sModifierType.Compare( _T("tremor") ) == 0 )
		{
			m_modTremorPos.Configure(pModifierNodes);
			m_modTremorPos.SetEnable(true);
		}
		else if ( sModifierType.Compare( _T("spaz") ) == 0 )
		{
			m_modSpazSize.Configure(pModifierNodes);
			m_modSpazSize.SetEnable(true);
		}
		else if ( sModifierType.Compare( _T("pulse") ) == 0 )
		{
			m_modPulseSize.Configure(pModifierNodes);
			m_modPulseSize.SetEnable(true);
		}
		else
		{
			m_colorHandler.Configure( pModifierNodes, _T("textcolor") );
			m_colorHandlerShadow.Configure( pModifierNodes, _T("shadowcolor") );
		}

		//else if ( sModifierType.Compare( _T("throb") ) == 0 )
		//{
		//	sTarget = pModifierNodes->GetString( NULL, _T("target"), _T("") );
		//	sTarget.MakeLower();
		//	if ( sTarget.Compare( _T("textcolor") ) == 0 )
		//	{
		//		m_modClrThrobText.Configure(pModifierNodes);
		//		m_modClrThrobText.SetEnable(true);
		//	}
		//	else if ( sTarget.Compare( _T("shadowcolor") ) == 0 )
		//	{
		//		m_modClrThrobShadow.Configure(pModifierNodes);
		//		m_modClrThrobShadow.SetEnable(true);
		//	}
		//}
		//else if ( sModifierType.Compare( _T("flicker") ) == 0 )
		//{
		//	sTarget = pModifierNodes->GetString( NULL, _T("target"), _T("") );
		//	sTarget.MakeLower();
		//	if ( sTarget.Compare( _T("textcolor") ) == 0 )
		//	{
		//		m_modClrFlkrText.Configure(pModifierNodes);
		//		m_modClrFlkrText.SetEnable(true);
		//	}
		//	else if ( sTarget.Compare( _T("shadowcolor") ) == 0 )
		//	{
		//		m_modClrFlkrShadow.Configure(pModifierNodes);
		//		m_modClrFlkrShadow.SetEnable(true);
		//	}
		//}
	}

	m_bNeedsSetup = true;
}

void CRGHorzMenu::OnActivation( void )
{
}

void CRGHorzMenu::OnDeactivation( void )
{
}

CRGHorzMenu::CRGHorzMenu() :
	m_pfnt(NULL),
	m_bNeedsSetup(false),
	m_bCached(false)
{
}

CRGHorzMenu::~CRGHorzMenu()
{
}




class CRGImage : public CRenderGadget
{
protected:
	bool				m_bDeleteEntry;
	CImageResourceEntry * m_pEntry;
	CImageSource		m_img;
	RECT				m_rectModBounce;
	RECT				m_rectModTremor;
	bool				m_bEntryDynamic;
	bool				m_bIndexDynamic;
	int					m_iCurEntry;
	int					m_iScrIndex;
	DWORD				m_dwMenuHash;
	bool				m_bLocalDefined;
	CStdString			m_sFilename;
	bool				m_bLoad;
	bool				m_bEnableSound;
	bool				m_bSuspendMusic;
	bool				m_bMusicPaused;
	int					m_iGadgetWidth;
	int					m_iGadgetHeight;
	int					m_iLoopControl;
	bool				m_bEndAfterSequence;
	CModSizePulse		m_modPulseSize;
	CModSizeSpaz		m_modSpazSize;
	CModPosBounce		m_modBouncePos;
	CModPosTremor		m_modTremorPos;
//	LPDIRECT3DDEVICE8	m_pDevice;

	virtual void		InternalInitialize( void );
	virtual void		InternalDestroy( void );
	virtual void		InternalRender( void );
	virtual void		InternalConfigure( CXMLNode * pNode );
public:
	virtual bool AllowMusic( void );
	virtual bool SequenceDone( void );
	virtual	void OnActivation( void );
	virtual	void OnDeactivation( void );
	CRGImage();
	~CRGImage();
};

bool CRGImage::AllowMusic( void )
{
	bool bReturn = true;

	if ( m_bEnableSound && m_img.HasAudio() )
	{
		if ( m_bEntryDynamic )
		{
			switch( g_MenuInfo.Overrides.m_iPreviewSuspendMusic )
			{
				case -1: // 
					bReturn = true;
					break;
				case 1:
					bReturn = false;
					break;
				default:
					if ( m_bSuspendMusic )
					{
						bReturn = false;
					}
					break;
			}
		}
		else
		{
			if ( m_bSuspendMusic )
			{
				bReturn = false;
			}
		}
	}
	return bReturn;
}

void CRGImage::InternalInitialize( void )
{
	m_img.Initialize( GetStateApp()->Get3dDevice(), GetStateApp()->GetSoundDevice() );
}

void CRGImage::InternalDestroy( void )
{
	if ( m_bDeleteEntry && m_pEntry )
	{
		delete m_pEntry;
		m_pEntry = NULL;
	}
}

bool CRGImage::SequenceDone( void )
{
	bool bReturn = false;

	if ( m_bEndAfterSequence )
	{
		if ( m_img.IsVideoTexture() && m_img.IsLoopingEnded() )
		{
			bReturn = true;
		}
	}
	return bReturn;
}

void CRGImage::InternalRender( void )
{
	bool bNewImage = false;
	CStdString sFilename;
	LPDIRECT3DTEXTURE8 pTexture;

	if( m_pEntry )
	{
		if ( m_bEntryDynamic )
		{
			if ( m_dwMenuHash != g_MenuInfo.GetMenuHash() )
			{
				m_iCurEntry = -1;
				m_iScrIndex = -1;
				m_dwMenuHash = g_MenuInfo.GetMenuHash();
			}

			if ( m_iCurEntry != g_MenuInfo.m_iCurrentItem )
			{
				m_iCurEntry = g_MenuInfo.m_iCurrentItem;
				sFilename = m_pEntry->GetFilename();
				if ( m_sFilename.Compare(sFilename) )
				{
					bNewImage = true;
					m_sFilename = sFilename;
				}
			}
		}
		if ( m_bIndexDynamic )
		{
			if ( m_iScrIndex != g_MenuInfo.m_iScreenIndex )
			{
				m_iScrIndex = g_MenuInfo.m_iScreenIndex;
				sFilename = m_pEntry->GetFilename();
				if ( m_sFilename.Compare(sFilename) )
				{
					bNewImage = true;
					m_sFilename = sFilename;
					m_bLoad = true;
				}
			}
		}
	}
	if ( m_bLoad || bNewImage )
	{
		m_img.LoadImage( m_sFilename );
		m_img.SetVideoSound(m_bEnableSound);
		m_img.SetLoopControl( m_iLoopControl );

		// Suspend music??
		if ( m_img.HasAudio() && m_bEnableSound && m_bSuspendMusic )
		{
			// Disable music.
			// Still some stuff to figure out here...
			// To consider that we might have multiple video sources!!

		}
		else
		{
			// Enable music, if paused!
		}
		UpdateMusicStatus();
		m_bLoad = false;
	}
	pTexture = m_img.GetTexture();
	if ( pTexture )
	{
		// Display image now...
		m_iGadgetWidth = m_img.GetWidth();
		m_iGadgetHeight = m_img.GetHeight();
		MakeRects( m_iGadgetWidth, m_iGadgetHeight );

		if ( m_modBouncePos.ModifyPosRect( m_rectDest, m_rectModBounce) )
		{
			memcpy(&m_rectDest, &m_rectModBounce, sizeof(RECT) );
		}
		if ( m_modTremorPos.ModifyPosRect( m_rectDest, m_rectModTremor) )
		{
			memcpy(&m_rectDest, &m_rectModTremor, sizeof(RECT) );
		}
		if ( m_modPulseSize.ModifyPosRect( m_rectDest, m_rectModTremor) )
		{
			memcpy(&m_rectDest, &m_rectModTremor, sizeof(RECT) );
		}
		if ( m_modSpazSize.ModifyPosRect( m_rectDest, m_rectModTremor) )
		{
			memcpy(&m_rectDest, &m_rectModTremor, sizeof(RECT) );
		}
		DrawSpriteRects(m_pDevice, pTexture, m_rectDest, m_rectSrc, m_iGadgetWidth, m_iGadgetHeight, TRUE, TRUE );
	}
}

void CRGImage::InternalConfigure( CXMLNode * pNode )
{
	CStdString sValue;

	sValue = pNode->GetString( NULL, _T("source"), _T("default"), true );
	m_bDeleteEntry = false;
	m_pEntry = g_ImgResCache.GetEntry(sValue);
	if ( m_pEntry == NULL )
	{
		// Have to look for local definition node
		sValue = pNode->GetString( NULL, _T("file"), _T(""), true );
		if ( sValue.GetLength() )
		{
			m_sFilename = MakeFullFilePath( g_MenuInfo.m_sMXMPath, sValue );
			m_bLocalDefined = true;
		}
		else
		{
			// Do we have a resource node?
			CXMLNode * pResNode;
			pResNode = pNode->GetNode( _T("resource") );
			if ( pResNode )
			{
				sValue = pResNode->GetString( NULL, _T("name"), _T(""), true );
				if ( sValue.GetLength() )
				{
					g_ImgResCache.AddEntry(pResNode);
					m_pEntry = g_ImgResCache.GetEntry(sValue);
					if ( !m_pEntry )
					{
						m_bDeleteEntry = true;
						// Last gasp!
						m_pEntry = new CImageResourceEntry(pResNode);
					}
				}
				else
				{
					m_bDeleteEntry = true;
					m_pEntry = new CImageResourceEntry(pResNode);
				}
				if ( !m_pEntry )
				{
					// No img. 
					m_sFilename = _T("");
					m_bLocalDefined = true;
				}
			}
			else
			{
				// No img. 
				m_sFilename = _T("");
				m_bLocalDefined = true;
			}
		}
	}
	// OK, either we're locally defined or we proceed...
	if ( m_pEntry )
	{
		m_bEntryDynamic = m_pEntry->IsEntryDynamic();
		m_bIndexDynamic = m_pEntry->IsScreenDynamic();
		m_sFilename = m_pEntry->GetFilename();
	}
	if ( m_sFilename.GetLength() )
	{
		m_img.Initialize( GetStateApp()->Get3dDevice(), GetStateApp()->GetSoundDevice() );
		g_FileManager.AddFileEntry( m_sFilename );
		m_bLoad = true;
	}

	CXMLNode * pModifierNodes = NULL;
	int iModNodeCount = 0;
	CStdString sModifierType;

	m_modBouncePos.SetEnable(false);
	m_modTremorPos.SetEnable(false);

	while( pModifierNodes = pNode->GetNode( _T("modifier"), iModNodeCount++ ) )
	{
		sModifierType = pModifierNodes->GetString( NULL, _T("type"), _T(""), true );
		sModifierType.MakeLower();
		if ( sModifierType.Compare( _T("bounce") ) == 0 )
		{
			m_modBouncePos.Configure(pModifierNodes);
			m_modBouncePos.SetEnable(true);
		}
		else if ( sModifierType.Compare( _T("tremor") ) == 0 )
		{
			m_modTremorPos.Configure(pModifierNodes);
			m_modTremorPos.SetEnable(true);
		}
		else if ( sModifierType.Compare( _T("spaz") ) == 0 )
		{
			m_modSpazSize.Configure(pModifierNodes);
			m_modSpazSize.SetEnable(true);
		}
		else if ( sModifierType.Compare( _T("pulse") ) == 0 )
		{
			m_modPulseSize.Configure(pModifierNodes);
			m_modPulseSize.SetEnable(true);
		}
	}

	m_iLoopControl = pNode->GetLong( NULL, _T("loop"), -1 );

	m_bEnableSound = (pNode->GetBool( _T("sound"), _T("enable"), FALSE )==TRUE);
	if ( m_bEntryDynamic )
	{
		switch( g_MenuInfo.Overrides.m_iPreviewEnableSound )
		{
			case 0: // no change
				break;
			case 1: // "True"
				m_bEnableSound = true;
				break;
			case -1: // "False"
				m_bEnableSound = false;
				break;
		}
	}
	m_bSuspendMusic = (pNode->GetBool( _T("sound"), _T("suspendmusic"), TRUE )==TRUE);
	m_bEndAfterSequence = (pNode->GetBool( NULL, _T("EndAfterSequence"), FALSE )==TRUE);
	
//	m_bEntryDynamic(false),
//	m_bIndexDynamic(false),

}

void CRGImage::OnActivation( void )
{
	DEBUG_FORMAT( _T("Activating RGImage Gadget 0x%08x"), this );
	m_iCurEntry = -1;
	m_iScrIndex = -1;
	m_img.ReSync();
	DEBUG_FORMAT( _T("Done Activating RGImage Gadget 0x%08x"), this );
}

void CRGImage::OnDeactivation( void )
{
	
}

CRGImage::CRGImage() :
	m_bDeleteEntry(false),
	m_pEntry(NULL),
	m_bEntryDynamic(false),
	m_bIndexDynamic(false),
	m_iCurEntry(-1),
	m_iScrIndex(-1),
	m_dwMenuHash(0),
	m_bEndAfterSequence(false),
	m_bLocalDefined(false)
{
}

CRGImage::~CRGImage()
{
}



class CRGNewWaveMenu : public CRenderGadget
{
protected:
	CImageSource		m_imgBackTexture;
	CImageSource		m_imgSelTexture;
	int					m_iNumItems;
	int					m_iSelPos;
	int					m_iGadgetWidth;
	int					m_iGadgetHeight;
	bool				m_bIsTitleMenu;
	int					m_iCurEntry;
	int					m_iMinHeight;
	bool				m_bThumbnails;
	bool				m_bLeft;
	CImageResourceEntry *	m_pFolderEntry;
	CImageResourceEntry *	m_pPassEntry;
	CImageSource		m_imgFolder;
	bool				m_bFolderLoad;
	bool				m_bFolderEntryDynamic;
	CStdString			m_sFolderFilename;
	CImageSource		m_imgPass;
	bool				m_bPassLoad;
	bool				m_bPassEntryDynamic;
	CStdString			m_sPassFilename;
	
	CTextAttributes		m_taSelPrimary;
	CTextAttributes		m_taSelSecondary;

	CTextAttributes		m_taUnselPrimary;
	CTextAttributes		m_taUnselSecondary;

	CStdString			m_sPrimarySource;
	CStdString			m_sSecondarySource;

	CStdString			m_sPrimaryFont;
	CStdString			m_sSecondaryFont;

	CXBFont *			m_pMenuFont;
	CXBFont *			m_pSecondaryFont;
	int					m_iPrimaryOffsetY;
	int					m_iSecondaryOffsetY;
	bool				m_bDrawSecondary;

	bool				m_bSmoothScroll;

	int					m_iTopSquash;
	int					m_iBottomSquash;
	int					m_iSubSource;
	FLOAT				m_fPixelWidth;
	FLOAT				m_fPixelHeight;
	FLOAT				m_fSelPixelHeight;
	FLOAT				m_fSelPixelWidth;
	int					m_iSelTop;
	int					m_iSelHeight;
	int					m_iSecondaryItemHeight;
	int					m_iItemHeight;
	int					m_iItemWidth;
	int					m_iScrollOffset;
	int					m_iScrollFactor;
	int					m_iLastPos;
	DWORD				m_dwMenuHash;

	TListStrings		m_slItems;
	TListStrings		m_slSecondaryItems;

	void				LoadItems( void );
	void				PrepMenuBack(void);
	virtual void		InternalInitialize( void );
	virtual void		InternalDestroy( void );
	virtual void		InternalRender( void );
	virtual void		InternalConfigure( CXMLNode * pNode );
public:
	virtual	void OnActivation( void );
	virtual	void OnDeactivation( void );
	CRGNewWaveMenu();
	~CRGNewWaveMenu();
};

void CRGNewWaveMenu::LoadItems( void )
{
	int iNumItems = g_MenuInfo.GetItemCount();
	int iIndex;
	FLOAT fWidth, fHeight;
	CStdString sValue;
	
	int iSecondaryItemHeight = -1;
	int iItemHeight = -1;
	int iItemWidth = -1;

	m_iLastPos = g_MenuInfo.m_iCurrentItem;

	m_slItems.clear();
	m_slSecondaryItems.clear();

	if ( m_pMenuFont && SUCCEEDED(m_pMenuFont->GetTextExtent( _T("Ag"), &fWidth, &fHeight )) )
	{
		iItemHeight = (int)(fWidth+0.5);
	}
	if ( m_bDrawSecondary )
	{
		if ( m_pSecondaryFont && SUCCEEDED(m_pSecondaryFont->GetTextExtent( _T("Ag"), &fWidth, &fHeight )) )
		{
			iSecondaryItemHeight = (int)(fWidth+0.5);
		}
	}
	for( iIndex=0; iIndex<iNumItems; iIndex++ )
	{
		CItemInfo * pItem;
		pItem = g_MenuInfo.Entry(iIndex);

		if ( pItem )
		{
			sValue = pItem->GetValue(m_sPrimarySource);
			if ( sValue.GetLength() )
			{
				m_slItems.push_back( sValue );
			}
			else
			{
				m_slItems.push_back( _T(" ") );
			}
			if ( m_pMenuFont && SUCCEEDED(m_pMenuFont->GetTextExtent( sValue, &fWidth, &fHeight )) )
			{
				fWidth += 0.5f;
				fHeight += 0.5f;
				if ( iItemHeight < (int)fHeight )
				{
					iItemHeight = (int)fHeight;
				}
				if ( iItemWidth < (int)fWidth )
				{
					iItemWidth = (int)fWidth;
				}
			}
			if ( m_bDrawSecondary )
			{
				sValue = pItem->GetValue(m_sSecondarySource);
				if ( sValue.GetLength() )
				{
					m_slSecondaryItems.push_back( sValue );
				}
				else
				{
					m_slSecondaryItems.push_back( _T(" ") );
				}
				if ( m_pSecondaryFont && SUCCEEDED(m_pSecondaryFont->GetTextExtent( sValue, &fWidth, &fHeight )) )
				{
					fWidth += 0.5f;
					fHeight += 0.5f;
					if ( iSecondaryItemHeight < (int)fHeight )
					{
						iSecondaryItemHeight = (int)fHeight;
					}
					if ( iItemWidth < (int)fWidth )
					{
						iItemWidth = (int)fWidth;
					}
				}
			}
		}
	}
	if ( iItemWidth != -1 )
	{
		m_iItemWidth = iItemWidth;
	}
	if ( iItemHeight != -1 )
	{
		m_iItemHeight = iItemHeight;
	}
	if ( iSecondaryItemHeight != -1 )
	{
		m_iSecondaryItemHeight = iSecondaryItemHeight;
	}
	else
	{
		m_iSecondaryItemHeight = 0;
	}
}

void CRGNewWaveMenu::PrepMenuBack(void)
{
	int iItemHeight;
	LoadItems();


	iItemHeight = m_iItemHeight+m_iSecondaryItemHeight;
	if ( iItemHeight < m_iMinHeight )
	{
		iItemHeight = m_iMinHeight;
	}

	m_fPixelWidth = 1.0;
	m_fPixelHeight = 1.0;

	// Make background texture...
	if ( m_bScaleX )
	{
		m_iGadgetWidth = m_iItemWidth;
	}
	else
	{
		if ( m_iWidth > 0 )
		{
			m_iGadgetWidth = m_iWidth;
		}
		else
		{
			// No set width, so...
			m_iGadgetWidth = m_iItemWidth;
		}
	}
	if ( m_bScaleY )
	{
		// Based on number of items displayed OR proportional adjustment
		if ( m_iNumItems < 1 )
		{
			// Proportional adjustment needed... num items determined by this.
			if ( m_bScaleX )
			{
				int iTempHeight;

				if ( m_iHeight > 0 )
				{
					// Fitting 20hX130w in 300hX200w
					iTempHeight = iItemHeight*m_iHeight;
					iTempHeight /= m_iItemWidth;
					// Now we have our proportional item height...
					m_iNumItems = m_iHeight/iTempHeight;
				}
				else
				{
					// No height, no num items. Yuck.
					m_iNumItems = 1;
				}
			}
			else
			{
				if ( m_iHeight > 0 )
				{
					m_iNumItems = m_iHeight/iItemHeight;
				}
			}
		}
	}
	else
	{
		if ( m_iHeight > 0 )
		{
			// Adjust num items to match height
			m_iNumItems = m_iHeight/iItemHeight;
		}
	}
	if ( m_iNumItems < 1 )
	{
		m_iNumItems = 1;
	}
	if ( iItemHeight < 2 )
	{
		iItemHeight = 20;
	}
	m_iGadgetHeight = (iItemHeight*m_iNumItems);


	if ( ( m_iGadgetWidth > 0 ) && (m_iGadgetHeight>0) )
	{
		m_imgBackTexture.MakeBlank( m_iGadgetWidth, m_iGadgetHeight );
		m_imgBackTexture.GetCorrectedTC( m_fPixelWidth, m_fPixelHeight );
		//if ( m_imgBackTexture.GetWidth() )
		//{
		//	m_fPixelWidth = (FLOAT)m_iGadgetWidth;
		//	m_fPixelWidth /= (FLOAT)m_imgBackTexture.GetWidth();
		//}
		//if ( m_imgBackTexture.GetHeight() )
		//{
		//	m_fPixelHeight = (FLOAT)m_iGadgetHeight;
		//	m_fPixelHeight /= (FLOAT)m_imgBackTexture.GetHeight();
		//}
		// Now make Select Back

		m_imgSelTexture.MakeBlank( m_iGadgetWidth, iItemHeight );
		m_imgSelTexture.GetCorrectedTC( m_fSelPixelWidth, m_fSelPixelHeight );
		//if ( m_imgSelTexture.GetHeight() )
		//{
		//	m_fSelPixelHeight = (FLOAT)iItemHeight;
		//	m_fSelPixelHeight /= (FLOAT)m_imgSelTexture.GetHeight();
		//}
	}
	m_iSelTop = m_iTop+(((iItemHeight*m_iSelPos)*m_iHeight)/m_iGadgetHeight);
	m_iSelHeight = (iItemHeight*m_iHeight)/m_iGadgetHeight;
}


void CRGNewWaveMenu::InternalInitialize( void )
{
	m_imgBackTexture.Initialize( m_pDevice, GetStateApp()->GetSoundDevice() );
	m_imgSelTexture.Initialize( m_pDevice, GetStateApp()->GetSoundDevice() );
	m_imgBackTexture.MakeBlank( 40, 40 );
	m_imgSelTexture.MakeBlank( 40, 40 );

	//m_pMenuFont = &GetStateApp()->m_Font16;
	//m_pSecondaryFont = &GetStateApp()->m_Font16;
}

void CRGNewWaveMenu::InternalDestroy( void )
{
}


void CRGNewWaveMenu::InternalConfigure( CXMLNode * pNode )
{
	CStdString sValue;
	int iPrimaryJustify = -1;
	int iSecondaryJustify = -1;

	{
		CXMLNode * pPrimaryNode = pNode->GetNode( _T("primary") );
		if ( pPrimaryNode )
		{
			m_sPrimarySource = pPrimaryNode->GetString( NULL, _T("source"), _T("title"), true );
			m_sPrimarySource.MakeLower();
			if ( m_sPrimarySource.Compare( _T("title") ) == 0 )
			{
				m_bIsTitleMenu = true;
			}
			else
			{
				m_bIsTitleMenu = false;
			}
			sValue = pPrimaryNode->GetString( NULL, _T("font"), _T("default"), true );
			// Handle loading font resource
			m_pMenuFont = g_FontCache.GetFontPtrByResname(sValue);
			if ( m_pMenuFont == NULL )
			{
				m_pMenuFont = &GetStateApp()->m_Font16;
			}
			m_iPrimaryOffsetY = pPrimaryNode->GetLong( NULL, _T("offsety"), 0 );
			sValue = pPrimaryNode->GetString( NULL, _T("justifyx"), _T("left"), true );
			sValue.MakeLower();
			if ( sValue.Compare(_T("right")) == 0 )
			{
				iPrimaryJustify = 1;
			}
			else if ( sValue.Compare(_T("center")) == 0 )
			{
				iPrimaryJustify = 0;
			}
		}
		else
		{
			// Set font...
			m_sPrimarySource = _T("title");
			m_iPrimaryOffsetY = 0;
			m_pMenuFont = &GetStateApp()->m_Font16;
		}
	}
	iSecondaryJustify = iPrimaryJustify;
	{
		CXMLNode * pSecondaryNode = pNode->GetNode( _T("secondary") );
		if ( pSecondaryNode )
		{
			m_sSecondarySource = pSecondaryNode->GetString( NULL, _T("source"), _T("descr"), true );
			sValue = pSecondaryNode->GetString( NULL, _T("font"), _T("default"), true );
			// Handle loading font resource
			m_pSecondaryFont = g_FontCache.GetFontPtrByResname(sValue);
			if ( m_pSecondaryFont == NULL )
			{
				m_pSecondaryFont = &GetStateApp()->m_Font16;
			}


			m_iSecondaryOffsetY = pSecondaryNode->GetLong( NULL, _T("offsety"), 0 );
			switch ( iSecondaryJustify )
			{
				case 1:
					sValue = pSecondaryNode->GetString( NULL, _T("justifyx"), _T("right"), true );
					break;
				case 0:
					sValue = pSecondaryNode->GetString( NULL, _T("justifyx"), _T("center"), true );
					break;
				default:
					sValue = pSecondaryNode->GetString( NULL, _T("justifyx"), _T("left"), true );
					break;
			}
			sValue.MakeLower();
			if ( sValue.Compare(_T("right")) == 0 )
			{
				iSecondaryJustify = 1;
			}
			else if ( sValue.Compare(_T("center")) == 0 )
			{
				iSecondaryJustify = 0;
			}
		}
		else
		{
			// Set font...
			m_sSecondarySource = _T("descr");
			m_iSecondaryOffsetY = 0;
			m_pSecondaryFont = &GetStateApp()->m_Font16;
		}
	}


	// Handle Selection Colors
	{
		CXMLNode * pSelectionNode = pNode->GetNode( _T("selection") );

		if ( pSelectionNode )
		{
			m_taSelPrimary.Configure( pSelectionNode );
			//m_taSelPrimary.m_bDoGlow = (pSelectionNode->GetBool( NULL, _T("DoGlow"), false )==TRUE);
			//m_taSelPrimary.m_dwBackColor = pSelectionNode->GetDWORD( NULL, _T("BackColor"), 0);
			//m_taSelPrimary.m_dwBorderColor = pSelectionNode->GetDWORD( NULL, _T("BorderColor"), 0);
			//m_taSelPrimary.m_dwShadowColor = pSelectionNode->GetDWORD( NULL, _T("ShadowColor"), 0);
			//m_taSelPrimary.m_dwTextColor = pSelectionNode->GetDWORD( NULL, _T("TextColor"), 0xFFFFFFFF);
			//m_taSelPrimary.m_iShadowSize = pSelectionNode->GetLong( NULL, _T("ShadowSize"), 2);
			m_taSelSecondary = m_taSelPrimary;
			pSelectionNode = pSelectionNode->GetNode( _T("secondary") );
			if ( pSelectionNode )
			{
				m_taSelSecondary.Configure(pSelectionNode);
				//m_taSelSecondary.m_bDoGlow = (pSelectionNode->GetBool( NULL, _T("DoGlow"), m_taSelPrimary.m_bDoGlow )==TRUE);
				//m_taSelSecondary.m_dwShadowColor = pSelectionNode->GetDWORD( NULL, _T("ShadowColor"), m_taSelPrimary.m_dwShadowColor);
				//m_taSelSecondary.m_dwTextColor = pSelectionNode->GetDWORD( NULL, _T("TextColor"), m_taSelPrimary.m_dwTextColor);
				//m_taSelSecondary.m_iShadowSize = pSelectionNode->GetLong( NULL, _T("ShadowSize"), m_taSelPrimary.m_iShadowSize );
			}
			//else
			//{
			//	m_taSelSecondary.m_bDoGlow = m_taSelPrimary.m_bDoGlow;
			//	m_taSelSecondary.m_dwShadowColor = m_taSelPrimary.m_dwShadowColor;
			//	m_taSelSecondary.m_dwTextColor = m_taSelPrimary.m_dwTextColor;
			//	m_taSelSecondary.m_iShadowSize = m_taSelPrimary.m_iShadowSize;
			//}
		}
		else
		{
			m_taSelPrimary.m_bDoGlow = false;
			m_taSelPrimary.m_dwBackColor = 0;
			m_taSelPrimary.m_dwBorderColor = 0;
			m_taSelPrimary.m_dwShadowColor = 0;
			m_taSelPrimary.m_dwTextColor = 0xFFFFFFFF;
			m_taSelPrimary.m_iShadowSize = 2;
			m_taSelSecondary = m_taSelPrimary;

			//m_taSelSecondary.m_bDoGlow = m_taSelPrimary.m_bDoGlow;
			//m_taSelSecondary.m_dwShadowColor = m_taSelPrimary.m_dwShadowColor;
			//m_taSelSecondary.m_dwTextColor = m_taSelPrimary.m_dwTextColor;
			//m_taSelSecondary.m_iShadowSize = m_taSelPrimary.m_iShadowSize;
		}
	}
	{
		m_taUnselPrimary.m_dwTextColor = 0xFF808080;
		CXMLNode * pSelectionNode = pNode->GetNode( _T("unselitem") );
		if ( pSelectionNode )
		{
			m_taUnselPrimary.Configure( pSelectionNode );

			//m_taUnselPrimary.m_bDoGlow = (pSelectionNode->GetBool( NULL, _T("DoGlow"), false )==TRUE);
			//m_taUnselPrimary.m_dwBackColor = pSelectionNode->GetDWORD( NULL, _T("BackColor"), 0);
			//m_taUnselPrimary.m_dwBorderColor = pSelectionNode->GetDWORD( NULL, _T("BorderColor"), 0);
			//m_taUnselPrimary.m_dwShadowColor = pSelectionNode->GetDWORD( NULL, _T("ShadowColor"), 0);
			//m_taUnselPrimary.m_dwTextColor = pSelectionNode->GetDWORD( NULL, _T("TextColor"), 0xFF808080);
			//m_taUnselPrimary.m_iShadowSize = pSelectionNode->GetLong( NULL, _T("ShadowSize"), 2);

			m_taUnselSecondary = m_taUnselPrimary;
			pSelectionNode = pSelectionNode->GetNode( _T("secondary") );
			if ( pSelectionNode )
			{
				m_taUnselSecondary.Configure(pSelectionNode);
				//m_taUnselSecondary.m_bDoGlow = (pSelectionNode->GetBool( NULL, _T("DoGlow"), m_taUnselPrimary.m_bDoGlow )==TRUE);
				//m_taUnselSecondary.m_dwShadowColor = pSelectionNode->GetDWORD( NULL, _T("ShadowColor"), m_taUnselPrimary.m_dwShadowColor);
				//m_taUnselSecondary.m_dwTextColor = pSelectionNode->GetDWORD( NULL, _T("TextColor"), m_taUnselPrimary.m_dwTextColor);
				//m_taUnselSecondary.m_iShadowSize = pSelectionNode->GetLong( NULL, _T("ShadowSize"), m_taUnselPrimary.m_iShadowSize );
			}
			//else
			//{
			//	m_taUnselSecondary.m_bDoGlow = m_taUnselPrimary.m_bDoGlow;
			//	m_taUnselSecondary.m_dwShadowColor = m_taUnselPrimary.m_dwShadowColor;
			//	m_taUnselSecondary.m_dwTextColor = m_taUnselPrimary.m_dwTextColor;
			//	m_taUnselSecondary.m_iShadowSize = m_taUnselPrimary.m_iShadowSize;
			//}
		}
		else
		{
			m_taUnselPrimary.m_bDoGlow = false;
			m_taUnselPrimary.m_dwBackColor = 0;
			m_taUnselPrimary.m_dwBorderColor = 0;
			m_taUnselPrimary.m_dwShadowColor = 0;
			m_taUnselPrimary.m_dwTextColor = 0xFF808080;
			m_taUnselPrimary.m_iShadowSize = 2;
			m_taUnselSecondary = m_taUnselPrimary;
			//m_taUnselSecondary.m_bDoGlow = m_taUnselPrimary.m_bDoGlow;
			//m_taUnselSecondary.m_dwShadowColor = m_taUnselPrimary.m_dwShadowColor;
			//m_taUnselSecondary.m_dwTextColor = m_taUnselPrimary.m_dwTextColor;
			//m_taUnselSecondary.m_iShadowSize = m_taUnselPrimary.m_iShadowSize;
		}
	}

	m_taSelPrimary.m_iJustify = iPrimaryJustify;
	m_taSelSecondary.m_iJustify = iSecondaryJustify;
	m_taUnselPrimary.m_iJustify = iPrimaryJustify;
	m_taUnselSecondary.m_iJustify = iSecondaryJustify;

	m_iMinHeight = pNode->GetLong( NULL, _T("MinimumHeight"), 1 );
	m_bThumbnails = (pNode->GetBool( NULL, _T("thumbnails"), FALSE ) == TRUE);

	sValue = pNode->GetString( _T("thumbnails"), _T("align"), _T("left"), true );
	if ( _tcsicmp( sValue, _T("left") ) == 0 )
	{
		m_bLeft = true;
	}
	else
	{
		m_bLeft = false;
	}
	sValue = pNode->GetString( _T("thumbnails"), _T("FolderImg"), _T(""), true );
	m_pFolderEntry = g_ImgResCache.GetEntry(sValue);
	// OK, either we're locally defined or we proceed...
	if ( m_pFolderEntry )
	{
		m_bFolderEntryDynamic = m_pFolderEntry->IsEntryDynamic();
		m_sFolderFilename = m_pFolderEntry->GetFilename();
	}
	if ( m_sFolderFilename.GetLength() )
	{
		m_imgFolder.Initialize( GetStateApp()->Get3dDevice(), GetStateApp()->GetSoundDevice() );
		g_FileManager.AddFileEntry( m_sFolderFilename );
		m_bFolderLoad = true;
	}

	sValue = pNode->GetString( _T("thumbnails"), _T("PassImg"), _T(""), true );
	m_pPassEntry = g_ImgResCache.GetEntry(sValue);

	// OK, either we're locally defined or we proceed...
	if ( m_pPassEntry )
	{
		m_bPassEntryDynamic = m_pPassEntry->IsEntryDynamic();
		m_sPassFilename = m_pPassEntry->GetFilename();
	}
	if ( m_sPassFilename.GetLength() )
	{
		m_imgPass.Initialize( GetStateApp()->Get3dDevice(), GetStateApp()->GetSoundDevice() );
		g_FileManager.AddFileEntry( m_sPassFilename );
		m_bPassLoad = true;
	}

	m_iScrollFactor = pNode->GetLong( NULL, _T("ScrollFactor"), 3 );
	if ( m_iScrollFactor < 1 )
	{
		m_iScrollFactor = 1;
	}
	m_iNumItems = pNode->GetLong( NULL, _T("NumItems"), 5 );
	m_iSelPos = pNode->GetLong( NULL, _T("SelPos"), 2 );
	m_bSmoothScroll = (pNode->GetBool( NULL, _T("SmoothScroll"), TRUE ) == TRUE);

	m_bDrawSecondary = (pNode->GetBool( NULL, _T("DrawSecondary"), FALSE ) == TRUE);

//	m_dwSelTextColor = pNode->GetDWORD( NULL, _T("SelTextColor"), 0xFFFFFFFF );
//	m_dwSelBackColor = pNode->GetDWORD( NULL, _T("SelBackColor"), 0x80000000 );
//	m_dwSelBorderColor = pNode->GetDWORD( NULL, _T("SelBorderColor"), 0 );
//	m_dwSelShadowColor = pNode->GetDWORD( NULL, _T("SelShadowColor"), 0xFF000000 );
//	m_dwTextColor = pNode->GetDWORD( NULL, _T("TextColor"), 0xFFFFFF00 );
//	m_dwBackColor = pNode->GetDWORD( NULL, _T("BackColor"), 0x80000000 );
//	m_dwBorderColor = pNode->GetDWORD( NULL, _T("BorderColor"), 0 );
//	m_dwShadowColor = pNode->GetDWORD( NULL, _T("ShadowColor"), 0xFF000000 );
	
	m_iTopSquash = pNode->GetLong( NULL, _T("TopSquash"), -2 );
	m_iBottomSquash = pNode->GetLong( NULL, _T("BottomSquash"), -2 );

	sValue = pNode->GetString( NULL, _T("SubSource"), _T("Title"), true );
	sValue.MakeLower();
	if ( sValue.Compare( _T("descr") ) == 0 )
	{
		m_iSubSource = 1;
	}
	else
	{
		m_iSubSource = 0;
	}
//	DumpNode( pNode );

	// Conditionals NOT allowed for menus
	m_iConditionID = -1;
}

void CRGNewWaveMenu::InternalRender( void )
{
	TListStrings::iterator iterSecondaryItems;
	TListStrings::iterator iterItems;

	int iIndex = 0;
	int iTop, iBottom, iWidth;
	int iSelTop, iSelBottom;
	int	iCombinedHeight;

	FLOAT fWidth, fHeight;
	int iBaseXOffset;
	LPDIRECT3DTEXTURE8 pThumbTexture;
	LPDIRECT3DTEXTURE8 pFolderTexture;
	LPDIRECT3DTEXTURE8 pPassTexture;
	bool bFolderNewImage = false, bPassNewImage = false;
	CItemInfo * pItemEntry;
	D3DXIMAGE_INFO * pimgInfo;
	RECT rectSrc;
	RECT rectDest;
	CStdString sFilename;

	iBaseXOffset = 0;


	// m_taSelPrimary.m_dwBackColor
	// m_taSelPrimary.m_dwBorderColor
	if( m_pFolderEntry )
	{
		if ( m_bFolderEntryDynamic )
		{
			if ( m_dwMenuHash != g_MenuInfo.GetMenuHash() )
			{
				m_iCurEntry = -1;
			}

			if ( m_iCurEntry != g_MenuInfo.m_iCurrentItem )
			{
				m_iCurEntry = g_MenuInfo.m_iCurrentItem;
				sFilename = m_pFolderEntry->GetFilename();
				if ( m_sFolderFilename.Compare(sFilename) )
				{
					bFolderNewImage = true;
					m_sFolderFilename = sFilename;
				}
			}
		}
	}
	if( m_pPassEntry )
	{
		if ( m_bPassEntryDynamic )
		{
			if ( m_dwMenuHash != g_MenuInfo.GetMenuHash() )
			{
				m_iCurEntry = -1;
			}

			if ( m_iCurEntry != g_MenuInfo.m_iCurrentItem )
			{
				m_iCurEntry = g_MenuInfo.m_iCurrentItem;
				sFilename = m_pPassEntry->GetFilename();
				if ( m_sPassFilename.Compare(sFilename) )
				{
					bPassNewImage = true;
					m_sPassFilename = sFilename;
				}
			}
		}
	}
	if ( m_bFolderLoad || bFolderNewImage )
	{
		m_imgFolder.LoadImage( m_sFolderFilename );
		m_imgFolder.SetVideoSound(false);
		m_imgFolder.SetLoopControl( -1 );
		m_bFolderLoad = false;
	}
	if ( m_bPassLoad || bPassNewImage )
	{
		m_imgPass.LoadImage( m_sPassFilename );
		m_imgPass.SetVideoSound(false);
		m_imgPass.SetLoopControl( -1 );
		m_bPassLoad = false;
	}

	pFolderTexture = m_imgFolder.GetTexture();
	pPassTexture = m_imgPass.GetTexture();

	if ( m_dwMenuHash != g_MenuInfo.GetMenuHash() )
	{
		PrepMenuBack();
		m_dwMenuHash = g_MenuInfo.GetMenuHash();
	}

	iCombinedHeight = (m_iItemHeight+m_iSecondaryItemHeight);


	if ( iCombinedHeight < m_iMinHeight )
	{
		iCombinedHeight = m_iMinHeight;
	}

	if ( m_bThumbnails )
	{
		if ( m_bLeft )
		{
			iBaseXOffset = iCombinedHeight;
		}
		else
		{
			iBaseXOffset = 0;
		}
	}

	if ( m_iLastPos != g_MenuInfo.m_iCurrentItem )
	{
		if ( m_iLastPos < g_MenuInfo.m_iCurrentItem )
		{
			m_iScrollOffset = iCombinedHeight;
		}
		else
		{
			m_iScrollOffset = 0-iCombinedHeight;
		}
		m_iLastPos = g_MenuInfo.m_iCurrentItem;
	}

	{
		CXBEasel easel( m_pDevice, m_imgBackTexture.GetTexture() );
		// Top position depends on:
		// m_iSelPos and CurrentSelectedItem
		// (m_iSelPos*m_iItemHeight) is top of selected item...

		m_pDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L );
		
		// GetStateApp()->DrawBox( 0.0f, 0.0f, (FLOAT)(m_iGadgetWidth-1), (FLOAT)(m_iGadgetHeight-1), m_taUnselPrimary.m_dwBackColor, m_taUnselPrimary.m_dwBorderColor );
		GetStateApp()->DrawBox( 0.0f, 0.0f, (FLOAT)(m_iGadgetWidth-1), (FLOAT)(m_iGadgetHeight-1), m_taUnselPrimary.GetBackColor(), m_taUnselPrimary.GetBorderColor() );

		int iMenuItemsDrawn = 0;

		iTop = ((m_iSelPos*iCombinedHeight)-(g_MenuInfo.m_iCurrentItem*iCombinedHeight))+m_iScrollOffset;
		iIndex = 0;
		iterSecondaryItems = m_slSecondaryItems.begin();
		iterItems = m_slItems.begin();
		while( iterItems != m_slItems.end() )
		{
			iBottom = (iTop+iCombinedHeight)-1;
			iWidth = 0;
			if ( m_pMenuFont && SUCCEEDED(m_pMenuFont->GetTextExtent( iterItems->c_str(), &fWidth, &fHeight )) )
			{
				iWidth = (int)(fWidth+0.5);
//				if ( g_MenuInfo.m_iCurrentItem != iIndex )
				{
					// Draw it now...
					if ( ( iBottom > 0 ) &&
						 ( iTop < m_iGadgetHeight ) )
					{
//						iMenuItemsDrawn++;

						pItemEntry = g_MenuInfo.Entry(iIndex);

						if ( m_bLeft )
						{
							DrawAttributedText( m_pMenuFont, &m_taUnselPrimary, iBaseXOffset, iTop, m_iGadgetWidth-iBaseXOffset, iterItems->c_str() );
						}
						else
						{
							DrawAttributedText( m_pMenuFont, &m_taUnselPrimary, 0, iTop, m_iGadgetWidth-iBaseXOffset, iterItems->c_str() );
						}
						if ( m_bDrawSecondary )
						{
							if ( m_bLeft )
							{
								DrawAttributedText( m_pSecondaryFont, &m_taUnselSecondary, iBaseXOffset, iTop+m_iItemHeight, m_iGadgetWidth-iBaseXOffset, iterSecondaryItems->c_str() );
							}
							else
							{
								DrawAttributedText( m_pSecondaryFont, &m_taUnselSecondary, 0, iTop+m_iItemHeight, m_iGadgetWidth-iBaseXOffset, iterSecondaryItems->c_str() );
							}
						}
						// Draw Thumbnail here
						if ( m_bThumbnails ) // pItemEntry->m_pThumbEntry )
						{
							int iImgWidth, iImgHeight;

							if ( pPassTexture && !pItemEntry->HasPassed() )
							{
								pThumbTexture = pPassTexture;
								iImgWidth = m_imgPass.GetWidth();
								iImgHeight = m_imgPass.GetHeight();
							}
							else if ( pItemEntry->m_pThumbEntry && SUCCEEDED(pItemEntry->m_pThumbEntry->GetTexture(&pThumbTexture, &pimgInfo) ) )
							{
								iImgWidth = pimgInfo->Width;
								iImgHeight = pimgInfo->Height;
							}
							else if ( pFolderTexture && pItemEntry->m_bIsMenu )
							{
								pThumbTexture = pFolderTexture;
								iImgWidth = m_imgFolder.GetWidth();
								iImgHeight = m_imgFolder.GetHeight();
							}
							else
							{
								pThumbTexture = NULL;
							}
							// Draw it... pItemEntry->m_pThumbEntry
							if ( pThumbTexture )
							{
								int iWidth, iHeight, iXOff, iYOff;

								iXOff = 0;
								iYOff = 0;
								rectSrc.left = 0;
								rectSrc.top = 0;
								rectSrc.right = iImgWidth-1;
								rectSrc.bottom = iImgHeight-1;

								if ( iImgWidth && iImgHeight )
								{
									if ( iImgWidth == iImgHeight )
									{
										iWidth = iCombinedHeight;
										iHeight = iCombinedHeight;
									}
									else if ( iImgWidth > iImgHeight )
									{
										iWidth = iCombinedHeight;
										iHeight = (iImgHeight*iCombinedHeight)/iImgWidth;
										iYOff = (iCombinedHeight-iHeight)>>1;
									}
									else
									{
										iWidth = (iImgWidth*iCombinedHeight)/iImgHeight;
										iHeight = iCombinedHeight;
										iXOff = (iCombinedHeight-iWidth)>>1;
									}
								}
								else
								{
									iWidth = iCombinedHeight;
									iHeight = iCombinedHeight;
								}

								if ( m_bLeft )
								{
									rectDest.left = iXOff;
									rectDest.right = iXOff+(iWidth-1);
								}
								else
								{
									rectDest.left = m_iGadgetWidth-(iWidth+iXOff);
									rectDest.right = m_iGadgetWidth-iXOff;
								}
								rectDest.top = iTop+iYOff;
								rectDest.bottom = iTop+iYOff+(iHeight-1);

								DrawSpriteRects(m_pDevice, pThumbTexture, rectDest, rectSrc, iImgWidth, iImgHeight, TRUE );
							}
						}
					}
				}

			}
			iTop += iCombinedHeight;
			iIndex++;
			iterItems++;
			iterSecondaryItems++;
		}

		iMenuItemsDrawn = m_iNumItems;

		if ( m_bIsTitleMenu )
		{
			if ( g_MenuInfo.m_iCurrentItem>m_iSelPos )
			{
				g_MenuInfo.m_bItemPrev = true;
			}
			else
			{
				g_MenuInfo.m_bItemPrev = false;
			}
			iMenuItemsDrawn -= (m_iSelPos);
			iMenuItemsDrawn += g_MenuInfo.m_iCurrentItem;

			if ( iMenuItemsDrawn < (int)m_slItems.size() )
			{
				g_MenuInfo.m_bItemNext = true;
			}
			else
			{
				g_MenuInfo.m_bItemNext = false;
			}
		}

		D3DRECT rect[2];

		// Clear out selection area from this texture....
		iTop = (m_iSelPos*iCombinedHeight);
		iBottom = (iTop+iCombinedHeight)-1;
		rect[0].x1 = 0;
		rect[0].x2 = m_iGadgetWidth-1;
		rect[0].y1 = iTop;
		rect[0].y2 = iBottom;

// BJ - Removed. For whatever reason, it isn't working with 4927
//		m_pDevice->Clear( 1L, rect, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0L );
	}
	{
		CXBEasel easel( m_pDevice, m_imgSelTexture.GetTexture() );
		// Top position depends on:
		// m_iSelPos and CurrentSelectedItem
		// (m_iSelPos*m_iItemHeight) is top of selected item...

		m_pDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L );
		
		GetStateApp()->DrawBox( 0.0f, 0.0f, (FLOAT)(m_iGadgetWidth-1), (FLOAT)(iCombinedHeight-1), m_taSelPrimary.GetBackColor(), m_taSelPrimary.GetBorderColor() );
		// GetStateApp()->DrawBox( 0.0f, 0.0f, (FLOAT)(m_iGadgetWidth-1), (FLOAT)(iCombinedHeight-1), m_taSelPrimary.m_dwBackColor, m_taSelPrimary.m_dwBorderColor );

		iIndex = 0;

		iTop = ((m_iSelPos*iCombinedHeight)-(g_MenuInfo.m_iCurrentItem*iCombinedHeight)); // +m_iScrollOffset;
		iSelTop = iTop-(m_iSelPos*iCombinedHeight)+m_iScrollOffset;
		iterSecondaryItems = m_slSecondaryItems.begin();
		iterItems = m_slItems.begin();
		while( iterItems != m_slItems.end() )
		{
			iSelBottom = (iSelTop+(m_iItemHeight+m_iSecondaryItemHeight))-1;
			iBottom = (iTop+(m_iItemHeight+m_iSecondaryItemHeight))-1;
			iWidth = 0;
			if ( m_pMenuFont && SUCCEEDED(m_pMenuFont->GetTextExtent( iterItems->c_str(), &fWidth, &fHeight )) )
			{
				iWidth = (int)(fWidth+0.5);
//				if ( g_MenuInfo.m_iCurrentItem != iIndex )
				{
					// Draw it now...
					if ( ( iSelBottom > 0 ) &&
						 ( iSelTop < iCombinedHeight ) )
					{
						pItemEntry = g_MenuInfo.Entry(iIndex);
						if ( m_bLeft )
						{
							DrawAttributedText( m_pMenuFont, &m_taSelPrimary, iBaseXOffset, iSelTop, m_iGadgetWidth-iBaseXOffset, iterItems->c_str() );
						}
						else
						{
							DrawAttributedText( m_pMenuFont, &m_taSelPrimary, 0, iSelTop, m_iGadgetWidth-iBaseXOffset, iterItems->c_str() );
						}
						if ( m_bDrawSecondary )
						{
							if ( m_bLeft )
							{
								DrawAttributedText( m_pSecondaryFont, &m_taSelSecondary, iBaseXOffset, iSelTop+m_iItemHeight, m_iGadgetWidth-iBaseXOffset, iterSecondaryItems->c_str() );
							}
							else
							{
								DrawAttributedText( m_pSecondaryFont, &m_taSelSecondary, 0, iSelTop+m_iItemHeight, m_iGadgetWidth-iBaseXOffset, iterSecondaryItems->c_str() );
							}
						}
						if ( m_bThumbnails ) // pItemEntry->m_pThumbEntry )
						{
							int iImgWidth, iImgHeight;

							if ( pPassTexture && !pItemEntry->HasPassed() )
							{
								pThumbTexture = pPassTexture;
								iImgWidth = m_imgPass.GetWidth();
								iImgHeight = m_imgPass.GetHeight();
							}
							else if ( pItemEntry->m_pThumbEntry && SUCCEEDED(pItemEntry->m_pThumbEntry->GetTexture(&pThumbTexture, &pimgInfo) ) )
							{
								iImgWidth = pimgInfo->Width;
								iImgHeight = pimgInfo->Height;
							}
							else if ( pFolderTexture && pItemEntry->m_bIsMenu )
							{
								pThumbTexture = pFolderTexture;
								iImgWidth = m_imgFolder.GetWidth();
								iImgHeight = m_imgFolder.GetHeight();
							}
							else
							{
								pThumbTexture = NULL;
							}
							// Draw it... pItemEntry->m_pThumbEntry
							if ( pThumbTexture )
							{
								int iWidth, iHeight, iXOff, iYOff;

								iXOff = 0;
								iYOff = 0;
								rectSrc.left = 0;
								rectSrc.top = 0;
								rectSrc.right = iImgWidth-1;
								rectSrc.bottom = iImgHeight-1;

								if ( iImgWidth && iImgHeight )
								{
									if ( iImgWidth == iImgHeight )
									{
										iWidth = iCombinedHeight;
										iHeight = iCombinedHeight;
									}
									else if ( iImgWidth > iImgHeight )
									{
										iWidth = iCombinedHeight;
										iHeight = (iImgHeight*iCombinedHeight)/iImgWidth;
										iYOff = (iCombinedHeight-iHeight)>>1;
									}
									else
									{
										iWidth = (iImgWidth*iCombinedHeight)/iImgHeight;
										iHeight = iCombinedHeight;
										iXOff = (iCombinedHeight-iWidth)>>1;
									}
								}
								else
								{
									iWidth = iCombinedHeight;
									iHeight = iCombinedHeight;
								}

								if ( m_bLeft )
								{
									rectDest.left = iXOff;
									rectDest.right = iXOff+(iWidth-1);
								}
								else
								{
									rectDest.left = m_iGadgetWidth-(iWidth+iXOff);
									rectDest.right = m_iGadgetWidth-iXOff;
								}
								rectDest.top = iSelTop+iYOff;
								rectDest.bottom = iSelTop+iYOff+(iHeight-1);
								DrawSpriteRects(m_pDevice, pThumbTexture, rectDest, rectSrc, iImgWidth, iImgHeight, TRUE );
							}

						}

					}
				}

			}
			iTop += iCombinedHeight;
			iSelTop += iCombinedHeight;
			iIndex++;
			iterItems++;
			iterSecondaryItems++;
		}
	}
	DrawSpriteOffset(m_pDevice, m_imgBackTexture.GetTexture(), m_iLeft, m_iTop, m_iWidth, m_iHeight, 0.0, 0.0, m_fPixelWidth, m_fPixelHeight, false, true, -1 );
	DrawSpriteOffset(m_pDevice, m_imgSelTexture.GetTexture(), m_iLeft, m_iSelTop, m_iWidth, m_iSelHeight, 0.0, 0.0, m_fSelPixelWidth, m_fSelPixelHeight, false, true, -1 );
	if( m_iScrollOffset )
	{
		if( m_iScrollOffset < 0 )
		{
			m_iScrollOffset+=m_iScrollFactor;
			if ( m_iScrollOffset > 0 )
			{
				m_iScrollOffset = 0;
			}
		}
		else
		{
			m_iScrollOffset-=m_iScrollFactor;
			if ( m_iScrollOffset < 0 )
			{
				m_iScrollOffset = 0;
			}
		}
	}
	
}


void CRGNewWaveMenu::OnActivation( void )
{
	DEBUG_FORMAT( _T("Activating NewWaveMenu Gadget 0x%08x"), this );
}

void CRGNewWaveMenu::OnDeactivation( void )
{
}

CRGNewWaveMenu::CRGNewWaveMenu() :
	m_iNumItems(5),
	m_iLastPos(0),
	m_iMinHeight(1),
	m_bThumbnails(false),
	m_bLeft(true),
	m_iSelPos(2),
	m_iScrollOffset(0),
	m_bSmoothScroll(true),
	m_pMenuFont(NULL),
//	m_dwSelTextColor(0),
//	m_dwSelBackColor(0),
//	m_dwSelBorderColor(0),
//	m_dwSelShadowColor(0),
//	m_dwTextColor(0),
//	m_dwBackColor(0),
//	m_dwBorderColor(0),
//  m_dwShadowColor(0),
	m_bFolderLoad(false),
	m_bFolderEntryDynamic(false),
	m_bPassLoad(false),
	m_bPassEntryDynamic(false),
	m_iCurEntry(-1),
	m_iTopSquash(-2),
	m_iBottomSquash(-2),
	m_iItemHeight(-1),
	m_iItemWidth(-1),
	m_iScrollFactor(3),
	m_dwMenuHash(0)
{
}

CRGNewWaveMenu::~CRGNewWaveMenu()
{
}


class CRGOldSchoolMenu : public CRenderGadget
{
protected:
	CXBFont 			m_fntXDITitle;
	CXBFont 			m_fntXDIDescr;
	CXBFont *			m_pTitleFont;
	CXBFont *			m_pDescrFont;

	int					m_iCurEntry;
	int					m_iMinHeight;
	bool				m_bThumbnails;
	bool				m_bLeft;
	CImageResourceEntry *	m_pFolderEntry;
	CImageResourceEntry *	m_pPassEntry;
	CImageSource		m_imgFolder;
	bool				m_bFolderLoad;
	bool				m_bFolderEntryDynamic;
	CStdString			m_sFolderFilename;
	CImageSource		m_imgPass;
	bool				m_bPassLoad;
	bool				m_bPassEntryDynamic;
	CStdString			m_sPassFilename;
	int					m_iTFHeight;
	int					m_iDFHeight;
	int					m_iTextOffset;
	int					m_iImgWidth;
	int					m_iImgHeight;
	DWORD				m_dwMenuHash;
	CImageSource		m_imgBackTexture;
	FLOAT				m_fPixelWidth;
	FLOAT				m_fPixelHeight;
	CStdString			m_sTitleFont;
	CStdString			m_sDescrFont;
	int					m_iScreenOffset;
	int					m_iDisplayCount;
	int					m_iTitleSquash;
	int					m_iDescrSquash;
	int					m_iTitleXOffset;
	int					m_iDescrXOffset;
	bool				m_bDrawDescr;
	DWORD				m_dwThrobPeriod;
	DWORD				m_dwMenuColor;
	DWORD				m_dwSelColor;
	DWORD				m_dwCurrentAlpha;
	DWORD				m_dwArrowColor;
	void				PrepMenuBack(void);
	virtual void		InternalInitialize( void );
	virtual void		InternalDestroy( void );
	virtual void		InternalRender( void );
	virtual void		InternalConfigure( CXMLNode * pNode );
public:
	virtual	void OnActivation( void );
	virtual	void OnDeactivation( void );
	CRGOldSchoolMenu();
	~CRGOldSchoolMenu();
};

/*
<LayoutElement Type=Image Source=Cache>
<SrcLoc Val=Title />
<Pos T=0 H=480 L=0 W=640/>
<DefColor ARGB=0xff000000 Point=Bottom/>
<DefColor ARGB=0xff0000ff Point=Top/>
</LayoutElement>
  */



/*

Throbber....

  pi*2 is our 100% cycle....


  */

CRGOldSchoolMenu::CRGOldSchoolMenu() :
	m_dwMenuHash(0),
	m_iTextOffset(0),
	m_iScreenOffset(0),
	m_iDisplayCount(4),
	m_iTitleSquash(-4),
	m_iDescrSquash(-7),
	m_iTitleXOffset(6),
	m_pFolderEntry(NULL),
	m_iDescrXOffset(16),
	m_iMinHeight(1),
	m_bThumbnails(false),
	m_bLeft(true),
	m_dwMenuColor(0xFFFFFF00),
	m_dwArrowColor( 0xFF0000FF),
	m_dwSelColor(0xFF000000),
	m_dwCurrentAlpha(0xFF000000),
	m_dwThrobPeriod(9000),
	m_bFolderLoad(false),
	m_bFolderEntryDynamic(false),
	m_bPassLoad(false),
	m_bPassEntryDynamic(false),
	m_iCurEntry(-1),
	m_bDrawDescr(true)
{
	m_pTitleFont = &GetStateApp()->m_Font16;
	m_pDescrFont = &GetStateApp()->m_Font16;
}

CRGOldSchoolMenu::~CRGOldSchoolMenu()
{
}



/*

  Rendering "original flava" menu:

  4 items displayed, title, description.
  Start at top. Move up when not at absolute botton
  Selection tends to not go to ends, unless last choice in that direction (first or last)


  Render arrows at top and bottom, if needed



  */

void CRGOldSchoolMenu::InternalRender( void )
{
	LPDIRECT3DTEXTURE8 pTexture;
	int iCurrPos, iTotalItems;
	bool bUpArrow, bDownArrow;
	int iActualDisplayCount;
	int iItemTotalHeight, iBaseXOffset;
	int iTempY;
	LPDIRECT3DTEXTURE8 pThumbTexture;
	LPDIRECT3DTEXTURE8 pFolderTexture;
	LPDIRECT3DTEXTURE8 pPassTexture;
	bool bFolderNewImage = false, bPassNewImage = false;
	CItemInfo * pItemEntry;
	D3DXIMAGE_INFO * pimgInfo;
	RECT rectSrc;
	RECT rectDest;
	CStdString sFilename;

	iBaseXOffset = 0;

	// Update iBaseXOffset if we have a thumbnail to display on the left
	iItemTotalHeight = (m_iTFHeight+m_iTitleSquash);
	if ( m_bDrawDescr )
	{
		iItemTotalHeight += (m_iDFHeight+m_iDescrSquash);
	}
	if ( iItemTotalHeight < m_iMinHeight )
	{
		iItemTotalHeight = m_iMinHeight;
	}
	if ( m_bThumbnails )
	{
		if ( m_bLeft )
		{
			iBaseXOffset = iItemTotalHeight;
		}
		else
		{
			iBaseXOffset = 0;
		}
	}

	// Determine current display offset m_sScreenOffset
	iTotalItems = g_MenuInfo.GetItemCount();
	iCurrPos = g_MenuInfo.m_iCurrentItem;
	if ( iCurrPos < 0 )
	{
		iCurrPos = 0;
		g_MenuInfo.m_iCurrentItem = iCurrPos;
	}
	else if ( iCurrPos >= iTotalItems )
	{
		iCurrPos = (iTotalItems-1);
		g_MenuInfo.m_iCurrentItem = iCurrPos;
	}
	if ( iCurrPos == 0 )
	{
		m_iScreenOffset = 0;
	}
	else if ( iTotalItems == m_iDisplayCount )
	{
		// If we display only total we have, no need to change the offset!
		m_iScreenOffset = 0;
	}
	else if ( iCurrPos == ( iTotalItems-1) )
	{
		if ( iCurrPos >= m_iDisplayCount )
		{
			m_iScreenOffset = iCurrPos-(m_iDisplayCount-1);
		}
		else
		{
			m_iScreenOffset = 0;
		}
	}
	else if ( iCurrPos == m_iScreenOffset )
	{
		m_iScreenOffset--;
	}
	else if ( iCurrPos == (m_iScreenOffset+(m_iDisplayCount-1)) )
	{
		m_iScreenOffset++;
	}
	else
	{
		if ( iCurrPos < (m_iScreenOffset+(m_iDisplayCount-1)) )
		{
//			m_iScreenOffset = iCurrPos-(m_iDisplayCount-2);
		}
		else if ( iCurrPos >= (m_iDisplayCount-1) )
		{
			m_iScreenOffset = iCurrPos-(m_iDisplayCount-2);
		}
		else
		{
			m_iScreenOffset = 0;
		}
	}
	if ( m_iScreenOffset )
	{
		g_MenuInfo.m_bItemPrev = true;
		bUpArrow = true;
	}
	else
	{
		g_MenuInfo.m_bItemPrev = false;
		bUpArrow = false;
	}
	if ( m_iScreenOffset < (iTotalItems-m_iDisplayCount) )
	{
		g_MenuInfo.m_bItemNext = true;
		bDownArrow = true;
	}
	else
	{
		g_MenuInfo.m_bItemNext = false;
		bDownArrow = false;
	}

	// Render the thing....
	if ( m_pDevice )
	{

		if( m_pFolderEntry )
		{
			if ( m_bFolderEntryDynamic )
			{
				if ( m_dwMenuHash != g_MenuInfo.GetMenuHash() )
				{
					m_iCurEntry = -1;
				}

				if ( m_iCurEntry != g_MenuInfo.m_iCurrentItem )
				{
					m_iCurEntry = g_MenuInfo.m_iCurrentItem;
					sFilename = m_pFolderEntry->GetFilename();
					if ( m_sFolderFilename.Compare(sFilename) )
					{
						bFolderNewImage = true;
						m_sFolderFilename = sFilename;
					}
				}
			}
		}
		if( m_pPassEntry )
		{
			if ( m_bPassEntryDynamic )
			{
				if ( m_dwMenuHash != g_MenuInfo.GetMenuHash() )
				{
					m_iCurEntry = -1;
				}

				if ( m_iCurEntry != g_MenuInfo.m_iCurrentItem )
				{
					m_iCurEntry = g_MenuInfo.m_iCurrentItem;
					sFilename = m_pPassEntry->GetFilename();
					if ( m_sPassFilename.Compare(sFilename) )
					{
						bPassNewImage = true;
						m_sPassFilename = sFilename;
					}
				}
			}
		}
		if ( m_bFolderLoad || bFolderNewImage )
		{
			m_imgFolder.LoadImage( m_sFolderFilename );
			m_imgFolder.SetVideoSound(false);
			m_imgFolder.SetLoopControl( -1 );
			m_bFolderLoad = false;
		}
		if ( m_bPassLoad || bPassNewImage )
		{
			m_imgPass.LoadImage( m_sPassFilename );
			m_imgPass.SetVideoSound(false);
			m_imgPass.SetLoopControl( -1 );
			m_bPassLoad = false;
		}

		pFolderTexture = m_imgFolder.GetTexture();
		pPassTexture = m_imgPass.GetTexture();

		if ( m_dwMenuHash != g_MenuInfo.GetMenuHash() )
		{
			PrepMenuBack();
			m_dwMenuHash = g_MenuInfo.GetMenuHash();
		}
		pTexture = m_imgBackTexture.GetTexture();
		if ( pTexture )
		{
			{
				CXBEasel easel( m_pDevice, m_imgBackTexture.GetTexture() );
			
				m_pDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L );
				
				// Start and end of displayed menu
				int iMenuDispStart = 0;
				int iMenuDispEnd = 4;
				int iIndex;
				int iPosY;

				if ( g_MenuInfo.m_iCurrentItem > iMenuDispEnd )
				{
					iMenuDispEnd = g_MenuInfo.m_iCurrentItem;
					iMenuDispStart = iMenuDispEnd-4;
				}
				if ( iMenuDispEnd >= g_MenuInfo.GetItemCount() )
				{
					iMenuDispEnd = g_MenuInfo.GetItemCount()-1;
				}
				
				iPosY = m_iTextOffset;
				DWORD dwMenuColor;
				DWORD dwMenuAlpha;
				DWORD dwTimeStamp = GetTickCount();

				if ( m_iDisplayCount > iTotalItems )
				{
					iActualDisplayCount = iTotalItems; 
				}
				else
				{
					iActualDisplayCount = m_iDisplayCount;
				}
				for(iIndex=0; iIndex<iActualDisplayCount; iIndex++ )
				{
					pItemEntry = g_MenuInfo.Entry(m_iScreenOffset+iIndex);

					if ( pItemEntry )
					{
						if ( m_dwThrobPeriod )				
						{
							dwMenuAlpha = GetPeriodValue(dwTimeStamp, m_dwThrobPeriod );
						}
						else
						{
							dwMenuAlpha = 0xFF000000;
						}
						dwMenuColor = (m_dwMenuColor&0x00ffffff)|dwMenuAlpha;
						dwTimeStamp += (m_dwThrobPeriod/m_iDisplayCount);
						if ( (m_iScreenOffset+iIndex) == iCurrPos )
						{
							GetStateApp()->DrawBox( (FLOAT)0, (FLOAT)iPosY, (FLOAT)m_iWidth, (FLOAT)(iPosY+iItemTotalHeight), dwMenuColor, 0 );
//							GetStateApp()->DrawBox( (FLOAT)0, (FLOAT)iPosY, (FLOAT)m_iWidth, (FLOAT)(iPosY+(m_iTFHeight+m_iTitleSquash)), dwMenuColor, 0 );
							m_pTitleFont->DrawText( (FLOAT)(m_iTitleXOffset+iBaseXOffset), (FLOAT)(iPosY+m_iTitleSquash), (m_dwSelColor&0x00ffffff)|dwMenuAlpha, pItemEntry->GetValue(szII_Title) );
						}
						else
						{
							m_pTitleFont->DrawText( (FLOAT)(m_iTitleXOffset+iBaseXOffset), (FLOAT)(iPosY+m_iTitleSquash), dwMenuColor, pItemEntry->GetValue(szII_Title) );
						}
						iTempY = iPosY;
						iPosY += (m_iTFHeight+m_iTitleSquash);
						if ( m_bDrawDescr )
						{
							if ( (m_iScreenOffset+iIndex) == iCurrPos )
							{
//								GetStateApp()->DrawBox( (FLOAT)0, (FLOAT)iPosY, (FLOAT)m_iWidth, (FLOAT)(iPosY+(m_iDFHeight+m_iDescrSquash)), dwMenuColor, 0 );
								m_pDescrFont->DrawText( (FLOAT)(m_iDescrXOffset+iBaseXOffset), (FLOAT)(iPosY+m_iDescrSquash), (m_dwSelColor&0x00ffffff)|dwMenuAlpha, pItemEntry->GetValue(szII_Descr) );
							}
							else
							{
								m_pDescrFont->DrawText( (FLOAT)(m_iDescrXOffset+iBaseXOffset), (FLOAT)(iPosY+m_iDescrSquash), dwMenuColor, pItemEntry->GetValue(szII_Descr) );
							}
							iPosY += (m_iDFHeight+m_iDescrSquash);
						}

						if ( m_bThumbnails ) // pItemEntry->m_pThumbEntry )
						{
							int iImgWidth, iImgHeight;

							if ( pPassTexture && !pItemEntry->HasPassed() )
							{
								pThumbTexture = pPassTexture;
								iImgWidth = m_imgPass.GetWidth();
								iImgHeight = m_imgPass.GetHeight();
							}
							else if ( pItemEntry->m_pThumbEntry && SUCCEEDED(pItemEntry->m_pThumbEntry->GetTexture(&pThumbTexture, &pimgInfo) ) )
							{
								iImgWidth = pimgInfo->Width;
								iImgHeight = pimgInfo->Height;
							}
							else if ( pFolderTexture && pItemEntry->m_bIsMenu )
							{
								pThumbTexture = pFolderTexture;
								iImgWidth = m_imgFolder.GetWidth();
								iImgHeight = m_imgFolder.GetHeight();
							}
							else
							{
								pThumbTexture = NULL;
							}
							// Draw it... pItemEntry->m_pThumbEntry

//							if ( SUCCEEDED(pItemEntry->m_pThumbEntry->GetTexture(&pThumbTexture, &pimgInfo) ) )
							if ( pThumbTexture )
							{
								int iWidth, iHeight, iXOff, iYOff;

								iXOff = 0;
								iYOff = 0;
								rectSrc.left = 0;
								rectSrc.top = 0;
								rectSrc.right = iImgWidth-1;
								rectSrc.bottom = iImgHeight-1;

								if ( iImgWidth && iImgHeight )
								{
									if ( iImgWidth == iImgHeight )
									{
										iWidth = iItemTotalHeight;
										iHeight = iItemTotalHeight;
									}
									else if ( iImgWidth > iImgHeight )
									{
										iWidth = iItemTotalHeight;
										iHeight = (iImgHeight*iItemTotalHeight)/iImgWidth;
										iYOff = (iItemTotalHeight-iHeight)>>1;
									}
									else
									{
										iWidth = (iImgWidth*iItemTotalHeight)/iImgHeight;
										iHeight = iItemTotalHeight;
										iXOff = (iItemTotalHeight-iWidth)>>1;
									}
								}
								else
								{
									iWidth = iItemTotalHeight;
									iHeight = iItemTotalHeight;
								}

								if ( m_bLeft )
								{
									rectDest.left = iXOff;
									rectDest.right = iXOff+(iWidth-1);
								}
								else
								{
									rectDest.left = m_iWidth-(iWidth+iXOff);
									rectDest.right = m_iWidth-iXOff;
								}
								rectDest.top = iTempY+iYOff;
								rectDest.bottom = iTempY+iYOff+(iHeight-1);

								DrawSpriteRects(m_pDevice, pThumbTexture, rectDest, rectSrc, iImgWidth, iImgHeight, TRUE );
							}
						}
						if ( (iPosY-iTempY) < m_iMinHeight )
						{
							iPosY = (iTempY+m_iMinHeight);
						}
					}

				}
				// Draw arrows now....
				if ( bUpArrow )
				{
					GetStateApp()->DrawTriangle( (FLOAT)(m_iWidth/3), (FLOAT)0.0, (FLOAT)m_iWidth-(m_iWidth/3), (FLOAT)m_iTextOffset, m_dwArrowColor, 0, DTRI_UP );
				}
				if ( bDownArrow )
				{
					GetStateApp()->DrawTriangle( (FLOAT)(m_iWidth/3), (FLOAT)(m_iHeight-m_iTextOffset), (FLOAT)m_iWidth-(m_iWidth/3), (FLOAT)m_iHeight, m_dwArrowColor, 0, DTRI_DN );
				}
			}
			// DrawSprite(m_pDevice, m_imgBackTexture.GetTexture(), 100, 100, m_imgBackTexture.GetWidth(), m_imgBackTexture.GetHeight(), false, true, -1 );
			DrawSpriteOffset(m_pDevice, m_imgBackTexture.GetTexture(), m_iLeft, m_iTop, m_iWidth, m_iHeight, 0.0, 0.0, m_fPixelWidth, m_fPixelHeight, false, true, -1 );
			// DrawSpriteOffset(m_pDevice, m_imgBackTexture.GetTexture(), m_iLeft, m_iTop, m_iWidth, m_iHeight, 0.0, 0.0, 1.0, 1.0, false, true, -1 );

		}
//		GetStateApp()->DrawBox( (FLOAT)m_iTop, (FLOAT)m_iLeft, (FLOAT)(m_iTop+m_iHeight), (FLOAT)(m_iLeft+m_iWidth), 0xff00ff00, 0xffffffff );
	}
}

void CRGOldSchoolMenu::InternalConfigure( CXMLNode * pNode )
{
	CStdString sValue;
	// Configure the elements available for this....
	/*
	CXBFont 			m_fntXDITitle;
	CXBFont 			m_fntXDIDescr;
	CXBFont *			m_pTitleFont;
	CXBFont *			m_pDescrFont;
	DWORD				m_dwArrowColor;
	*/

	m_sTitleFont = pNode->GetString( NULL, _T("titlefont"), _T("default"), true );
	m_sDescrFont = pNode->GetString( NULL, _T("descrfont"), _T("default"), true );
	m_iDisplayCount = pNode->GetLong( NULL, _T("DisplayCount"), 4 );
	m_iTitleSquash = pNode->GetLong( NULL, _T("TitleSquash"), -4 );
	m_iDescrSquash = pNode->GetLong( NULL, _T("DescrSquash"), -7 );
	m_bDrawDescr = (pNode->GetBool( NULL, _T("DrawDesc"), TRUE ) == TRUE);
	m_dwThrobPeriod = pNode->GetLong( NULL, _T("ThrobPeriod"), 8000 );
	m_dwMenuColor = pNode->GetDWORD( NULL, _T("MenuColor"), 0xFFFFFF00 );
	m_dwSelColor =  pNode->GetDWORD( NULL, _T("SelColor"), 0xFF000000 );
	m_dwArrowColor =  pNode->GetDWORD( NULL, _T("ArrowColor"), 0xFF0000FF );


	m_iMinHeight = pNode->GetLong( NULL, _T("MinimumHeight"), 1 );
	m_bThumbnails = (pNode->GetBool( NULL, _T("thumbnails"), FALSE ) == TRUE);

	sValue = pNode->GetString( _T("thumbnails"), _T("align"), _T("left"), true );
	if ( _tcsicmp( sValue, _T("left") ) == 0 )
	{
		m_bLeft = true;
	}
	else
	{
		m_bLeft = false;
	}
	sValue = pNode->GetString( _T("thumbnails"), _T("FolderImg"), _T(""), true );
	m_pFolderEntry = g_ImgResCache.GetEntry(sValue);
	// OK, either we're locally defined or we proceed...
	if ( m_pFolderEntry )
	{
		m_bFolderEntryDynamic = m_pFolderEntry->IsEntryDynamic();
		m_sFolderFilename = m_pFolderEntry->GetFilename();
	}
	if ( m_sFolderFilename.GetLength() )
	{
		m_imgFolder.Initialize( GetStateApp()->Get3dDevice(), GetStateApp()->GetSoundDevice() );
		g_FileManager.AddFileEntry( m_sFolderFilename );
		m_bFolderLoad = true;
	}

	sValue = pNode->GetString( _T("thumbnails"), _T("PassImg"), _T(""), true );
	m_pPassEntry = g_ImgResCache.GetEntry(sValue);

	// OK, either we're locally defined or we proceed...
	if ( m_pPassEntry )
	{
		m_bPassEntryDynamic = m_pPassEntry->IsEntryDynamic();
		m_sPassFilename = m_pPassEntry->GetFilename();
	}
	if ( m_sPassFilename.GetLength() )
	{
		m_imgPass.Initialize( GetStateApp()->Get3dDevice(), GetStateApp()->GetSoundDevice() );
		g_FileManager.AddFileEntry( m_sPassFilename );
		m_bPassLoad = true;
	}

	// FolderImg="Folder" PassImg="Lock"

//	DumpNode( pNode );

	// Conditionals NOT allowed for menus
	m_iConditionID = -1;
}


void CRGOldSchoolMenu::InternalInitialize( void )
{
	int iIndex;
	FLOAT fWidth, fHeight;

	m_imgBackTexture.Initialize( m_pDevice, GetStateApp()->GetSoundDevice() );
	m_imgBackTexture.MakeBlank( 640, 480 );

	m_pTitleFont = g_FontCache.GetFontPtrByResname(m_sTitleFont);
	if ( !m_pTitleFont )
	{
		if ( g_MenuInfo.m_bXDIMode )
		{
			m_pTitleFont = g_FontCache.GetFontPtrByFilename(_T("D:\\MenuX\\media\\fontb.xpr"), _T("fontb") );
		}
		if ( !m_pTitleFont )
		{
			m_pTitleFont = &GetStateApp()->m_Font16;
		}
	}

	m_pDescrFont = g_FontCache.GetFontPtrByResname(m_sDescrFont);
	if ( !m_pDescrFont )
	{
		if ( g_MenuInfo.m_bXDIMode )
		{
			m_pDescrFont = g_FontCache.GetFontPtrByFilename(_T("D:\\MenuX\\media\\fontn.xpr"), _T("fontn") );
		}
		if ( !m_pDescrFont )
		{
			m_pDescrFont = &GetStateApp()->m_Font16;
		}
	}

	m_pTitleFont->GetTextExtent( _T("Ag"), &fWidth, &fHeight );
	m_iTFHeight = (int)(fHeight+0.5);

	m_pDescrFont->GetTextExtent( _T("Ag"), &fWidth, &fHeight );
	m_iDFHeight = (int)(fHeight+0.5);

	for( iIndex=0; iIndex<g_MenuInfo.GetItemCount(); iIndex++ )
	{
		m_pTitleFont->GetTextExtent( g_MenuInfo.Entry(m_iScreenOffset+iIndex)->GetValue(szII_Title), &fWidth, &fHeight );
		if ( m_iTFHeight < (int)(fHeight+0.5) )
		{
			m_iTFHeight = (int)(fHeight+0.5);
		}
		m_pDescrFont->GetTextExtent( g_MenuInfo.Entry(m_iScreenOffset+iIndex)->GetValue(szII_Descr), &fWidth, &fHeight );
		if ( m_iDFHeight < (int)(fHeight+0.5) )
		{
			m_iDFHeight = (int)(fHeight+0.5);
		}
	}
	// Calc out how many entries we can have, and include a little bit for the arrows...
	int iItemHeight;

	if ( m_bDrawDescr )
	{
		iItemHeight = m_iTFHeight+m_iDFHeight+m_iTitleSquash+m_iDescrSquash+1;
	}
	else
	{
		iItemHeight = m_iTFHeight+m_iTitleSquash+1;
	}
	if ( iItemHeight < m_iMinHeight )
	{
		iItemHeight = m_iMinHeight;
	}
	m_iDisplayCount = (m_iHeight-8)/iItemHeight;
	if ( m_iDisplayCount < 1 )
	{
		m_iDisplayCount=1;
	}
	m_iTextOffset = (m_iHeight-(iItemHeight*m_iDisplayCount))/2;
}

void CRGOldSchoolMenu::InternalDestroy( void )
{
}


void CRGOldSchoolMenu::PrepMenuBack(void)
{
	m_fPixelWidth = 1.0;
	m_fPixelHeight = 1.0;
	// Make background texture...
	if ( ( m_iWidth > 0 ) && (m_iHeight>0) )
	{
//		if ( m_imgBackTexture.GetHeight() != m_iHeight ||
//			 m_imgBackTexture.GetWidth() != m_iWidth )
//		{
			m_imgBackTexture.MakeBlank( m_iWidth, m_iHeight );
			m_imgBackTexture.GetCorrectedTC( m_fPixelWidth, m_fPixelHeight );
/*
			if ( m_imgBackTexture.GetWidth() )
			{
				m_fPixelWidth = (FLOAT)m_iWidth;
				m_fPixelWidth /= (FLOAT)m_imgBackTexture.GetWidth();
			}
			if ( m_imgBackTexture.GetHeight() )
			{
				m_fPixelHeight = (FLOAT)m_iHeight;
				m_fPixelHeight /= (FLOAT)m_imgBackTexture.GetHeight();
			}
			*/
//		}
	}
}

void CRGOldSchoolMenu::OnActivation( void )
{
	DEBUG_FORMAT( _T("Activating OldSchoolMenu Gadget 0x%08x"), this );
}

void CRGOldSchoolMenu::OnDeactivation( void )
{
}

static LPCTSTR szConditionals[] =
{
	_T("itemprev"),		// 0
	_T("itemnext"),		// 1
	_T("itementry"),	// 2
	_T("itemsubmenu"),	// 3
	_T("itempasscode"), // 4
	_T("itemnopasscode"), // 5
	_T("submenu"), // 6
	_T("mainmenu"), // 7
	_T("itemaction"), // 8
	_T("cdrivepresent"), // 9
	_T("edrivepresent"), // 10
	_T("ddrivepresent"), // 11
	_T("fdrivepresent"), // 12
	_T("gdrivepresent"), // 13
	NULL
};

int GetConditionID( LPCTSTR szName )
{
	int iReturn = -1;
	int iIndex = 0;
	CStdString sName(szName);

	sName.MakeLower();
	if ( szName && sName.GetLength() )
	{
		while( szConditionals[iIndex] )
		{
			if ( sName.Compare(szConditionals[iIndex]) == 0 )
			{
				iReturn = iIndex;
				break;
			}
			iIndex++;
		}
	}
	return iReturn;
}

bool IsConditionMet( int iID, bool bNotCondition, bool bResetDrives )
{
	bool bReturn = false;
	static int iCDriveStatus = 0;
	static int iEDriveStatus = 0;
	static int iFDriveStatus = 0;
	static int iGDriveStatus = 0;

	if ( bResetDrives )
	{
		iCDriveStatus = 0;
		iEDriveStatus = 0;
		iFDriveStatus = 0;
		iGDriveStatus = 0;
	}
	switch( iID )
	{
		case 0: // itemprev
			bReturn = g_MenuInfo.m_bItemPrev;
			break;
		case 1: // itemnext
			bReturn = g_MenuInfo.m_bItemNext;
			break;
		case 2: // itementry
			CItemInfo * pEntry;
			pEntry = g_MenuInfo.Entry( g_MenuInfo.m_iCurrentItem );
			if ( pEntry )
			{
				if ( !pEntry->m_bIsMenu && !pEntry->m_bIsAction )
				{
					bReturn = true;
				}
			}
			break;
		case 8: // itemaction
			// CItemInfo * pEntry;
			pEntry = g_MenuInfo.Entry( g_MenuInfo.m_iCurrentItem );
			if ( pEntry )
			{
				if ( pEntry->m_bIsAction )
				{
					bReturn = true;
				}
			}
			break;
		case 3: // itemsubmenu
			pEntry = g_MenuInfo.Entry( g_MenuInfo.m_iCurrentItem );
			if ( pEntry )
			{
				if ( pEntry->m_bIsMenu )
				{
					bReturn = true;
				}
			}
			break;
		case 4: // _T("itempasscode"), // 4
			pEntry = g_MenuInfo.Entry( g_MenuInfo.m_iCurrentItem );
			if ( pEntry )
			{
				if ( pEntry->HasPassed() )
				{
					bReturn = false;
				}
				else
				{
					bReturn = true;
				}
			}
			break;
		case 5: // _T("itemnopasscode"), // 5
			pEntry = g_MenuInfo.Entry( g_MenuInfo.m_iCurrentItem );
			if ( pEntry )
			{
				if ( pEntry->HasPassed() )
				{
					bReturn = true;
				}
				else
				{
					bReturn = false;
				}
			}
			break;
		case 7: // mainmenu
			if ( g_MenuInfo.m_pMenuNode == g_MenuInfo.m_pMenuCurrNode )
			{
				bReturn = true;
			}
			else
			{
				bReturn = false;
			}
			break;
		case 6: // submenu
			if ( g_MenuInfo.m_pMenuNode == g_MenuInfo.m_pMenuCurrNode )
			{
				bReturn = false;
			}
			else
			{
				bReturn = true;
			}
			break;
		case 9: // 	_T("cdrivepresent"), 
			if ( iCDriveStatus == 0 )
			{
				iCDriveStatus = -1;
				if ( IsDrivePresent('C') )
				{
					iCDriveStatus = 1;
				}
			}
			bReturn = ( iCDriveStatus > 0 );
			break;
		case 10: // 	_T("edrivepresent"),
			if ( iEDriveStatus == 0 )
			{
				iEDriveStatus = -1;
				if ( IsDrivePresent('E') )
				{
					iEDriveStatus = 1;
				}
			}
			bReturn = ( iEDriveStatus > 0 );
			break;
		case 11: // _T("ddrivepresent"),
			break;
		case 12: // 	_T("fdrivepresent")
			if ( iFDriveStatus == 0 )
			{
				iFDriveStatus = -1;
				if ( IsDrivePresent('F') )
				{
					iFDriveStatus = 1;
				}
			}
			bReturn = ( iFDriveStatus > 0 );
			break;
		case 13: // 	_T("gdrivepresent")
			if ( iGDriveStatus == 0 )
			{
				iGDriveStatus = -1;
				if ( IsDrivePresent('G') )
				{
					iGDriveStatus = 1;
				}
			}
			bReturn = ( iGDriveStatus > 0 );
			break;
	}
	if ( bNotCondition )
	{
		bReturn = bReturn?false:true;
	}
	return bReturn;
}





class CMatrixEffect
{
	CXBFont			m_fntMatrix;
	bool			m_bDoEffect;
	char			m_mtrxGrid[40][30];
	int				m_iMatrixPos[40];
	int				m_iMatrixSpeed[40];
	DWORD			m_dwFrameCount;
	CImageSource	m_imgMatrixBack;
//	CImageSource	m_imgMatrixEasel;
	bool			m_bInit;
	bool			m_bShowMXM;
	CStdString		m_sMessage1;
	CStdString		m_sMessage2;
	DWORD			m_dwColor;
	bool			m_bUseBack;
public:
	void		SetStatusIdx( DWORD iIndex );
	void		SetStatusArray( LPCTSTR * pszStatusArray ) { m_pszStatusArray = pszStatusArray; };
	LPCTSTR		*	m_pszStatusArray;
	DWORD			m_dwStatusIndex;
	void		SetColor( DWORD dwColor ) { m_dwColor = dwColor; };
	DWORD		GetColor( void ) { return m_dwColor; };
	CMatrixEffect( void );
	~CMatrixEffect( void );
	void		Initialize( LPDIRECT3DDEVICE8 pDevice, bool bShowMXM = false, DWORD dwColor = 0xFF00FF00, bool bDoEffect = true );
	void		Destroy( void );
	void		Render( LPDIRECT3DDEVICE8 pDevice, bool bFade );
};

	
void		CMatrixEffect::SetStatusIdx( DWORD iIndex )
{
	// Make sure it's valid....
	DWORD iCheck;

	if ( m_pszStatusArray )
	{
		for( iCheck=0; iCheck <= iIndex; iCheck++ )
		{
			if ( m_pszStatusArray[iCheck] == NULL )
			{
				break;
			}
		}
		if ( iCheck > iIndex )
		{
			InterlockedExchange( (LPLONG)&m_dwStatusIndex, iIndex );
		}
	}
}



CMatrixEffect::CMatrixEffect( void )
{
	m_pszStatusArray = NULL;
	m_dwStatusIndex = 0;
	m_bUseBack = true;
	m_dwColor = 0xFF00FF00;
	m_bInit = false;
	m_bShowMXM = false;
	m_bDoEffect = true;
}

CMatrixEffect::~CMatrixEffect()
{
	if ( m_bInit )
	{
		Destroy();
	}
}

void CMatrixEffect::Initialize( LPDIRECT3DDEVICE8 pDevice, bool bShowMXM, DWORD dwColor, bool bDoEffect )
{
	int iX, iY;

	if ( !m_bInit )
	{
		m_bDoEffect = bDoEffect;
		m_dwColor = dwColor;
		m_dwFrameCount=0;
		m_bShowMXM = bShowMXM;
		// Initialize things...
//		m_imgMatrixEasel.Initialize( pDevice, GetStateApp()->GetSoundDevice() );
		m_imgMatrixBack.Initialize( pDevice, GetStateApp()->GetSoundDevice() );
		// bool CImageSource::DuplicateTexture( LPDIRECT3DTEXTURE8 pTexture )
		// m_imgMatrixEasel.MakeBlank( 640, 480 );
		// m_imgMatrixBack.MakeBlank( 640, 480 );

		LPDIRECT3DTEXTURE8 pTexture;
		if ( SUCCEEDED( GetBackBufferTexture( pDevice, &pTexture ) ))
		{
			//m_imgMatrixEasel.DuplicateTexture( pTexture );
		//	m_imgMatrixBack.DuplicateTexture( pTexture );
		//	m_imgMatrixBack.Attach( pTexture );
			pTexture->Release();
		}


		m_fntMatrix.CreateFromMem( pDevice, _T("LogoData"), (PBYTE)g_pbMatrixFont );
		for( iX = 0; iX<40; iX++ )
		{
			for( iY = 0; iY<30; iY++ )
			{
				// 0x61-0x6d (97-109)
				// m_mtrxGrid[iX][iY] = 97+(rand()%13);
				m_mtrxGrid[iX][iY] = 'A'+(char)(RandomNum()%13);
			}
			// Pos will be (0-120)/4
			// Speed will depend on how many "pos" it will skip on the way through
			m_iMatrixPos[iX] = 0-((RandomNum()+iX)%240);
			m_iMatrixSpeed[iX] = 1+((RandomNum()+iX)%4);
		}
		if ( m_bShowMXM )
		{
			for ( iX=0; g_szLogoName[iX]; iX++ )
			{
				if ( g_szLogoName[iX] != ' ' )
				{
					m_mtrxGrid[14+iX][14] = g_szLogoName[iX];
				}
			}
		}
		m_bInit = true;
	}
}

void CMatrixEffect::Destroy( void )
{
	if ( !m_bInit )
	{
		m_fntMatrix.Destroy();
		//if ( m_imgMatrixEasel.GetTexture() )
		//{
		//	m_imgMatrixEasel.ReleaseResources();
		//}
		if( m_imgMatrixBack.GetTexture() )
		{
			m_imgMatrixBack.ReleaseResources();
		}
		m_bInit = false;
	}
}

void CMatrixEffect::Render( LPDIRECT3DDEVICE8 pDevice, bool bFade )
{
	char chText[2];
	int iPos, iCol, iX, iY;
//	FLOAT fU, fV;
	DWORD dwGreen;
	DWORD dwLeadColor;
	RECT rect;
//	LPDIRECT3DTEXTURE8 pEaselTexture;
//	LPDIRECT3DTEXTURE8 pBackTexture;

	rect.left = 0;
	rect.top = 0;
	rect.right = 640;
	rect.bottom = 480;

	if ( !m_bInit )
	{
		Initialize(pDevice,false,m_dwColor);
	}

	iPos = (int)(m_dwFrameCount%32)*8;

	if ( iPos >= 128 )
	{
		dwGreen = iPos;
	}
	else
	{
		dwGreen = 255-iPos;
	}
	//dwGreen <<= 8;
	//dwGreen |= 0xff000000;
	dwGreen |= dwGreen<<8;
	dwGreen |= dwGreen<<8;
	dwGreen |= 0xff000000;
	dwGreen &= m_dwColor;
	dwLeadColor = 0xFFFFFFFF;

	//if ( m_dwFrameCount%2 )
	//{
	//	pEaselTexture = m_imgMatrixBack.GetTexture();
	//	pBackTexture = m_imgMatrixEasel.GetTexture();
	//	m_imgMatrixEasel.GetCorrectedTC( fU, fV );
	//}
	//else
	//{
	//	pEaselTexture = m_imgMatrixEasel.GetTexture();
	//	pBackTexture = m_imgMatrixBack.GetTexture();
	//	m_imgMatrixBack.GetCorrectedTC( fU, fV );
	//}


	{
//		CXBEasel easel(pDevice, pEaselTexture);
		// Render the PREVIOUS screen, which displays fading symbols
		// as new symbol is drawn below it. The essence of the Matrix Effect

//		if( pBackTexture && m_dwFrameCount > 0 )
		if( m_imgMatrixBack.GetTexture() )
		{
			// DrawSpriteOffset(pDevice, pBackTexture, 0, 0, 640, 480, 0, 0, fU, fV, false, false, -1 );
			GetStateApp()->RenderQuad(m_imgMatrixBack.GetTexture(), 640, 480, (FLOAT)0.90 );
			//GetStateApp()->RenderQuad(pBackTexture, 640, 480, (FLOAT)0.94 );
		}
		// GetStateApp()->DrawBox( 0, 0, 640, 480, 0x10000000, 0 );
			
		chText[1] = 0;

		if ( m_bDoEffect )
		{
			// Render falling symbols, one per column...
			for( iCol = 0; iCol<40; iCol++ )
			{
				m_iMatrixPos[iCol] += m_iMatrixSpeed[iCol];

				// Reset only if we are not fading out...
				if ( (m_iMatrixPos[iCol] > 239) && (!bFade) )
				{
					m_iMatrixPos[iCol] = 0-(RandomNum()%240);
				}

				// Get our grid Y coord
				iPos = m_iMatrixPos[iCol]/8;

				// Draw character only if the pos is visible
				if ( (iPos >= 0) && (iPos<30) )
				{
					iX = iCol*16;
					iY = iPos*16;
					chText[0] = m_mtrxGrid[iCol][iPos];
					m_fntMatrix.DrawText( (FLOAT)iX, (FLOAT)iY, m_dwColor, chText );
					if ( iPos<29 )
					{
						iX = iCol*16;
						iY = (iPos+1)*16;
						chText[0] = m_mtrxGrid[iCol][iPos+1];
						m_fntMatrix.DrawText( (FLOAT)iX, (FLOAT)iY, dwLeadColor, chText );
					}
				}
			}
		}

		if ( m_bShowMXM )
		{
			// Draw MEDIA X MENU text as throbbing text
			for ( iX=0; g_szLogoName[iX]; iX++ )
			{
				chText[0] = g_szLogoName[iX];
				if ( g_szLogoName[iX] != ' ' )
				{
					m_fntMatrix.DrawText( (FLOAT)((iX+14)*16), (FLOAT)(14*16), dwGreen, chText );
				}
			}
		}

		if ( m_pszStatusArray )
		{
			LPCTSTR szString = m_pszStatusArray[m_dwStatusIndex];
			if ( szString && _tcslen( szString ) )
			{
				int iLength = _tcslen( szString );

				iLength = (40-iLength)/2;

				for ( iX=0; szString[iX]; iX++ )
				{
					chText[0] = szString[iX];
					if ( szString[iX] != ' ' )
					{
						m_fntMatrix.DrawText( (FLOAT)((iX+iLength)*16), (FLOAT)(20*16), dwGreen, chText );
					}
				}
			}
		}
		if ( m_sMessage1.GetLength() )
		{
			int iLength = m_sMessage1.GetLength();
			iLength = (35-iLength);
			for ( iX=0; m_sMessage1[iX]; iX++ )
			{
				chText[0] = m_sMessage1[iX];
				if ( m_sMessage1[iX] != ' ' )
				{
					m_fntMatrix.DrawText( (FLOAT)((iX+iLength)*16), (FLOAT)(25*16), dwGreen, chText );
				}
			}
		}
		if ( m_sMessage2.GetLength() )
		{
			int iLength = m_sMessage2.GetLength();
			iLength = (35-iLength);
			for ( iX=0; m_sMessage2[iX]; iX++ )
			{
				chText[0] = m_sMessage2[iX];
				if ( m_sMessage2[iX] != ' ' )
				{
					m_fntMatrix.DrawText( (FLOAT)((iX+iLength)*16), (FLOAT)(26*16), dwGreen, chText );
				}
			}
		}

	}
	// GetStateApp()->RenderQuad(pEaselTexture, 640, 480, (FLOAT)1.0 );

	//LPDIRECT3DTEXTURE8 pTexture;
	//if ( SUCCEEDED( GetBackBufferTexture( pDevice, &pTexture ) ))
	//{
	//	// m_imgMatrixBack.Attach( pTexture );
	//	{
	//		CXBEasel easel( pDevice, m_imgMatrixBack.GetTexture() );
	//		GetStateApp()->RenderQuad(pTexture, 640, 480, (FLOAT)1.0 );
	//	}
	//	pTexture->Release();
	//}

	// DrawSpriteRects( pDevice, pEaselTexture, rect, rect, -1, -1, true, false );
	// GetStateApp()->RenderQuad(m_imgMatrixEasel.GetTexture(), 640, 480, (FLOAT)1.0 );



	// Copy surface from m_imgMatrixEasel to m_imgMatrixBack
	//if (1)
	//{
	//	LPDIRECT3DTEXTURE8 pSrcTexture = m_imgMatrixEasel.GetTexture();
	//	LPDIRECT3DTEXTURE8 pDestTexture = m_imgMatrixBack.GetTexture();

	//	if ( pSrcTexture && pDestTexture )
	//	{
	//		LPDIRECT3DSURFACE8 pSrcSurface;
	//		LPDIRECT3DSURFACE8 pDestSurface;

	//		if ( SUCCEEDED(pSrcTexture->GetSurfaceLevel( 0, &pSrcSurface) ) )
	//		{
	//			if( SUCCEEDED( pDestTexture->GetSurfaceLevel( 0, &pDestSurface ) ) )
	//			{
	//				D3DXLoadSurfaceFromSurface( pDestSurface, NULL, NULL, 
	//					pSrcSurface, NULL, NULL,
	//					D3DX_DEFAULT, D3DCOLOR( 0 ) );
	//				pDestSurface->Release();
	//			}
	//			pSrcSurface->Release();
	//		}
	//	}
	//}
	//else
	//{
	//	CXBEasel easel(pDevice, m_imgMatrixBack.GetTexture());
	//	DrawSpriteRects( pDevice, m_imgMatrixEasel.GetTexture(), rect, rect, -1, -1, true, false );
	//}

	// Grab copy of screen...
	LPDIRECT3DTEXTURE8 pTexture;
	if ( SUCCEEDED( GetBackBufferTexture( pDevice, &pTexture ) ))
	{
		m_imgMatrixBack.Attach( pTexture );
		pTexture->Release();
	}

	m_dwFrameCount++;
}



class CRGMatrix : public CRenderGadget
{
protected:
	CColorModHandler	m_colorHandler;
	DWORD				m_dwColor;
	CMatrixEffect		m_matrixRenderer;
	virtual void		InternalInitialize( void );
	virtual void		InternalDestroy( void );
	virtual void		InternalRender( void );
	virtual void		InternalConfigure( CXMLNode * pNode );
public:
	virtual	void OnActivation( void );
	virtual	void OnDeactivation( void );
	CRGMatrix();
	~CRGMatrix();
};

void CRGMatrix::InternalInitialize( void )
{
	m_iWidth = 640;
	m_iHeight = 480;
	m_matrixRenderer.Initialize( m_pDevice, false, m_dwColor );
}

void CRGMatrix::InternalDestroy( void )
{
	m_matrixRenderer.Destroy();
}

void CRGMatrix::InternalRender( void )
{
	m_matrixRenderer.SetColor( m_colorHandler.ModifyColor(m_dwColor) );
	m_matrixRenderer.Render( m_pDevice, false );
}

void CRGMatrix::InternalConfigure( CXMLNode * pNode )
{
	m_iWidth = 640;
	m_iHeight = 480;
	m_dwColor = pNode->GetDWORD( NULL, _T("color"), 0xFF00FF00 );
	m_matrixRenderer.SetColor(m_dwColor);

	CXMLNode * pModifierNode;
	int iIndex = 0;

	while(pModifierNode=pNode->GetNode( _T("modifier"), iIndex++ ))
	{
		m_colorHandler.Configure( pModifierNode,_T("color") );
	}
}

void CRGMatrix::OnActivation( void )
{
	DEBUG_FORMAT( _T("Activating MatrixEffect Gadget 0x%08x"), this );
}

void CRGMatrix::OnDeactivation( void )
{
}

CRGMatrix::CRGMatrix() :
	m_dwColor(0xff00ff00)
{
}

CRGMatrix::~CRGMatrix()
{
}


class CRGColorShiftBox : public CRenderGadget
{
protected:
	CColorModHandler	m_colorHandlerUL;
	CColorModHandler	m_colorHandlerLL;
	CColorModHandler	m_colorHandlerUR;
	CColorModHandler	m_colorHandlerLR;
	DWORD				m_dwColor[4];
	virtual void		InternalInitialize( void );
	virtual void		InternalDestroy( void );
	virtual void		InternalRender( void );
	virtual void		InternalConfigure( CXMLNode * pNode );
public:
	virtual	void OnActivation( void );
	virtual	void OnDeactivation( void );
	CRGColorShiftBox();
	~CRGColorShiftBox();
};

void CRGColorShiftBox::InternalInitialize( void )
{
//	m_iWidth = 640;
//	m_iHeight = 480;
}

void CRGColorShiftBox::InternalDestroy( void )
{
//	m_matrixRenderer.Destroy();
}

void CRGColorShiftBox::InternalRender( void )
{
	MakeRects( m_iWidth, m_iHeight );
	GetStateApp()->RenderQuadGradientBackground(
		m_colorHandlerUL.ModifyColor( m_dwColor[0]), 
		m_colorHandlerUR.ModifyColor( m_dwColor[1]), 
		m_colorHandlerLL.ModifyColor( m_dwColor[2]), 
		m_colorHandlerLR.ModifyColor( m_dwColor[3]), &m_rectDest );

//	m_matrixRenderer.SetColor( m_colorHandler.ModifyColor(m_dwColor) );
//	m_matrixRenderer.Render( m_pDevice, false );
}

void CRGColorShiftBox::InternalConfigure( CXMLNode * pNode )
{
//	m_iWidth = 640;
//	m_iHeight = 480;

	m_dwColor[0] = pNode->GetDWORD( NULL, _T("colorul"), m_dwColor[0] );
	m_dwColor[1] = pNode->GetDWORD( NULL, _T("colorur"), m_dwColor[1] );
	m_dwColor[2] = pNode->GetDWORD( NULL, _T("colorll"), m_dwColor[2] );
	m_dwColor[3] = pNode->GetDWORD( NULL, _T("colorlr"), m_dwColor[3] );
//	m_matrixRenderer.SetColor(m_dwColor);

	CXMLNode * pModifierNode;
	int iIndex = 0;
	bool bResults[4];

	bResults[0] = false; bResults[1] = false; 
	bResults[2] = false; bResults[3] = false;

	while(pModifierNode=pNode->GetNode( _T("modifier"), iIndex++ ))
	{
		
		if ( m_colorHandlerUL.Configure( pModifierNode,_T("colorul") ) )
		{
			bResults[0] = true;
		}
		if ( m_colorHandlerUR.Configure( pModifierNode,_T("colorur") ) )
		{
			bResults[1] = true;
		}
		if ( m_colorHandlerLL.Configure( pModifierNode,_T("colorll") ) )
		{
			bResults[2] = true;
		}
		if ( m_colorHandlerLR.Configure( pModifierNode,_T("colorlr") ) )
		{
			bResults[3] = true;
		}
	}
	if ( bResults[0] == false )
	{
		for( iIndex=0; iIndex<3; iIndex++ )
		{
			m_colorHandlerUL.SetColorEnable( CM_THROB, CIDX_RED+iIndex, TRUE );
			m_colorHandlerUL.SetRange( CM_THROB, CIDX_RED+iIndex, 0, 255 );
			m_colorHandlerUL.SetPeriod(CM_THROB, CIDX_RED+iIndex, (RandomNum()%100)*100+1000 );
		}
	}
	if ( bResults[1] == false )
	{
		for( iIndex=0; iIndex<3; iIndex++ )
		{
			m_colorHandlerUR.SetColorEnable( CM_THROB, CIDX_RED+iIndex, TRUE );
			m_colorHandlerUR.SetRange( CM_THROB, CIDX_RED+iIndex, 0, 255 );
			m_colorHandlerUR.SetPeriod(CM_THROB, CIDX_RED+iIndex, (RandomNum()%100)*100+1000 );
		}
	}
	if ( bResults[2] == false )
	{
		for( iIndex=0; iIndex<3; iIndex++ )
		{
			m_colorHandlerLL.SetColorEnable( CM_THROB, CIDX_RED+iIndex, TRUE );
			m_colorHandlerLL.SetRange( CM_THROB, CIDX_RED+iIndex, 0, 255 );
			m_colorHandlerLL.SetPeriod(CM_THROB, CIDX_RED+iIndex, (RandomNum()%100)*100+1000 );
		}
	}
	if ( bResults[3] == false )
	{
		for( iIndex=0; iIndex<3; iIndex++ )
		{
			m_colorHandlerLR.SetColorEnable( CM_THROB, CIDX_RED+iIndex, TRUE );
			m_colorHandlerLR.SetRange( CM_THROB, CIDX_RED+iIndex, 0, 255 );
			m_colorHandlerLR.SetPeriod(CM_THROB, CIDX_RED+iIndex, (RandomNum()%100)*100+1000 );
		}
	}
	
}

void CRGColorShiftBox::OnActivation( void )
{
	DEBUG_FORMAT( _T("Activating ColorShift Gadget 0x%08x"), this );
}

void CRGColorShiftBox::OnDeactivation( void )
{
}

CRGColorShiftBox::CRGColorShiftBox()
{
	int iIndex;

	for( iIndex=0; iIndex<4; iIndex++ )
	{
		m_dwColor[iIndex] = (RandomNum() % 0xffff);
		m_dwColor[iIndex] <<= 2;
		m_dwColor[iIndex] ^= (RandomNum() % 0xffff);
		m_dwColor[iIndex] <<= 3;
		m_dwColor[iIndex] ^= (RandomNum() % 0xffff);
		m_dwColor[iIndex] <<= 4;
		m_dwColor[iIndex] ^= (RandomNum() % 0xffff);
		m_dwColor[iIndex] <<= 5;
		m_dwColor[iIndex] ^= (RandomNum() % 0xffff);
		// Full on alpha...
		m_dwColor[iIndex] |= 0xff000000;
	}
}

CRGColorShiftBox::~CRGColorShiftBox()
{
}







CRenderGadget * CreateGadget( CXMLNode * pNode, CStdString & sGadgetName )
{
	CRenderGadget * pGadget = NULL;
	CStdString sValue, sName;

	sValue = pNode->GetString( NULL, _T("type"), _T(""), true );
	sValue.MakeLower();
	if ( sValue.GetLength() )
	{
		sName = pNode->GetString( NULL, _T("name"), _T(""), true );
		sName.MakeLower();
		if ( sValue.Compare( _T("menu") ) == 0 )
		{
			sValue = pNode->GetString( NULL, _T("source"), _T("MXM"), true );
			sValue.MakeLower();
			if ( sValue.Compare( _T("menux") ) == 0 )
			{
				pGadget = new CRGOldSchoolMenu();
				DEBUG_FORMAT( _T("Creating RenderGadget Menu::MXM") );
			}
			else
			{
				// Assume it's MXM in the absence of source
				pGadget = new CRGNewWaveMenu();
				DEBUG_FORMAT( _T("Creating RenderGadget Menu::MenuX") );
			}
		}
		else if ( sValue.Compare( _T("text") ) == 0 )
		{
			pGadget = new CRenderText();
			DEBUG_FORMAT( _T("Creating RenderGadget String") );
		}
		else if ( sValue.Compare( _T("string") ) == 0 )
		{
			pGadget = new CRenderText();
			DEBUG_FORMAT( _T("Creating RenderGadget String") );
		}
		else if ( sValue.Compare( _T("image") ) == 0 )
		{
			pGadget = new CRGImage();
			DEBUG_FORMAT( _T("Creating RenderGadget Image") );
		}
		else if ( sValue.Compare( _T("imagesource") ) == 0 )
		{
			pGadget = new CRGImage();
			DEBUG_FORMAT( _T("Creating RenderGadget Image") );
		}
		else if ( sValue.Compare( _T("matrixeffect") ) == 0 )
		{
			pGadget = new CRGMatrix();
			DEBUG_FORMAT( _T("Creating RenderGadget MatrixEffect") );
		}
		else if ( sValue.Compare( _T("colorshiftbox") ) == 0 )
		{
			pGadget = new CRGColorShiftBox();
			DEBUG_FORMAT( _T("Creating RenderGadget ColorShiftBox") );
		}
		
	}
	if ( pGadget )
	{
		sGadgetName = sName;
		pGadget->Configure( pNode );
		pGadget->Initialize( GetStateApp()->Get3dDevice() );
	}
	return pGadget;
}







#ifdef USE_XBFILEZILLA

XFSTATUS CriticalOperationCallbackMXM(SXFCriticalOperation Operation) 
{ 
	return GetStateApp()->CriticalOperationCallback(Operation); 
}

#endif

class CMXMApp : public CXBoxStateApplication, public CXBFTPServerEvents
{
    // Thread/Synchronization
    HANDLE                  m_hThread;                          // Worker thread
    HANDLE                  m_hShutdownEvent;                   // Shutdown event
    HANDLE                  m_hFadeEvent;						// Fade event
    HANDLE                  m_hThreadDoneEvent;                   // Shutdown event

	HANDLE					m_hFTPThread;
	bool					m_bFinishInitThread;
	bool					m_bFinishInitThread2;



	bool					m_bPauseInitScreen;
	CMatrixEffect			m_matrix;

	DWORD					m_dwFrameCount;

	
	void					RenderInitScreen(DWORD dwFrameCount, bool bFade );
	void					FinishIntialization();

	void					RenderInitStyle( DWORD dwFrameCount, bool bFade );

	CStdString				CheckBIOS( void );

public:

	CXBFTPServer	* m_pFTPPatchesServer;

//	CFTPServer	*	m_pFTPPatchesServer;


	DWORD		Prompt( LPCTSTR szMsg, DrawingAttributes * pAttributes = NULL );
	void	InitializeFTP( void );
	bool			m_bNetInitialized;

	CImageSource	m_imgPersistImage;
	CImageSource	m_imgLogo;
	CImageSource	m_imgTile;
	CMXMApp();
//	virtual void	MusicPause( void );
//	virtual void	MusicResume( void );
    virtual HRESULT Initialize();
	//void MakeInfoScreens( CXMLNode * pSkinNode );
	void LoadSkins( void );
	void LoadEntries( void );
	int LoadAutoCfg( void );
	int ValidateEntries( bool & bChangesMade, CXMLNode * pNode = NULL );
	bool ValidateEntry( bool & bChangeMade, CXMLNode * pNode );
	int LoadMenuFromDir( CXMLNode * pMainNode, LPCTSTR szDir, DWORD dwFlags, bool bRecurse = true, bool bFlatten = false, bool bCheckThisDir = true, bool bKeepD = false );
	int LoadMenuFromNode( CXMLNode * pMainNode, CXMLNode * pFileNode );
	int LoadMenuFromFile( CXMLNode * pMainNode, LPCTSTR szFile, bool bCheckOldStyle = false );
	int LoadMenuFromDVD( CXMLNode * pMainNode, bool bIsAutoLoad );
	virtual HRESULT LoadLocalSoundtrack( void );
	CXMLNode * GetMultiDVDNode( void );
	bool	RemoveMultiDVDMenu( void );
	virtual void OnMusicManagerFail( void );
	virtual void	FirstRender( void );
	virtual void	OnRunLoopTop();

	bool InitializeXDIConfig(void);
	bool InitializeDVDRConfig(void);
	bool InitializeHDConfig(void);
	void ConfigureFromMXM( void );
	void ConfigureFromPrefs( void );


    static DWORD WINAPI InitializeThreadProc(LPVOID lpParameter );
	void		InitializeThread( void );

    static DWORD WINAPI FTPThreadProc(LPVOID lpParameter );
	void		FTPThread( void );

	virtual DWORD	CMD( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szCmd, LPCTSTR szArgs, CStdString & sResult );
	virtual DWORD	EXEC( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szFilename, LPCTSTR szArgs );
	virtual HRESULT	SITE( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szArgs, CStdString & sResult );

};

#define DS_ROWHEIGHT	28

CXMLNode * AddBoxNode( CXMLNode * pNode, DWORD dwColor = 0xff0000ff, DWORD dwBorder = 0, int iX = 0, int iY = 0, int iW = 640, int iH = 480 )
{
	CXMLNode * pBoxNode = NULL;

	pBoxNode = pNode->AddNode( _T("LayoutElement") );
	if ( pBoxNode )
	{
		pBoxNode->SetString( NULL, _T("type"), _T("box"), true );
		SetPositionInNode( pBoxNode, iX, iY, iW, iH );
		pBoxNode->SetDWORD( NULL, _T("color"), dwColor );
		pBoxNode->SetDWORD( NULL, _T("bordercolor"), dwBorder );
	}
	return pBoxNode;
}

CXMLNode * AddDefaultString( CXMLNode * pNode, LPCTSTR szSource, int iRow, bool bLeftSide = true )
{
	bool bCenter = false;
	CXMLNode * pTextNode = NULL;
	int iX = 40;
	int iY = (iRow*(DS_ROWHEIGHT+2))+40;

	if ( iRow == 0 )
	{
		bCenter = true;
		iX = 320;
	}
	else if ( !bLeftSide )
	{
		iX = 600;
	}
	pTextNode = pNode->AddNode( _T("LayoutElement") );
	if ( pTextNode )
	{
		pTextNode->SetString( NULL, _T("type"), _T("text"), true );
		if ( szSource && _tcslen(szSource) )
		{
			pTextNode->SetString( NULL, _T("source"), szSource, true );
		}
		SetPositionInNode( pTextNode, -1, -1, -1, 24, iX, iY );
		SetScalingInNode( pTextNode, true, true );
		pTextNode->SetBool( NULL, _T("proportional"), TRUE );

		pTextNode->SetBool( NULL, _T("doglow"), bCenter?TRUE:FALSE );
		pTextNode->SetString( _T("justify"), _T("y"), _T("top"), true );
		if ( bCenter )
		{
			pTextNode->SetString( _T("justify"), _T("x"), _T("center"), true );
		}
		else
		{
			pTextNode->SetString( _T("justify"), _T("x"), bLeftSide?_T("left"):_T("right"), true );
		}
		pTextNode->SetString( NULL, _T("font"), _T("default") );
		pTextNode->SetDWORD( NULL, _T("textcolor"), 0xffffffff );
		pTextNode->SetDWORD( NULL, _T("shadowcolor"), 0xff000000 );
	}
	return pTextNode;
}

void MakeInfoScreens( void )
{
	if ( g_MenuInfo.m_pInternalInfo )
	{
		CXMLNode * pInfoScreens = g_MenuInfo.m_pInternalInfo->GetNode( _T("infoscreens") );
		if ( pInfoScreens )
		{
			CXMLNode * pInfoScreen;
			int iIndex = 0;

			while( pInfoScreen = pInfoScreens->GetNode( _T("layout"), iIndex++ ) )
			{
				CStdString sName = pInfoScreen->GetString( NULL, _T("screen"), _T(""), true, true );

				if ( _tcsnicmp( sName, _T("info"), 4 ) == 0 )
				{
					g_MenuInfo.m_pSkinInfo->CopyInNode( *pInfoScreen );
				}
			}
		}
	}
}

bool SelectSkinByName( LPCTSTR szName )
{
	bool bReturn = false;
	CStdString sSelectedName(szName);
	CXMLNode * pSkinNode = NULL;

	sSelectedName.MakeLower();

	if ( szName && sSelectedName.GetLength() )
	{
		// g_MenuInfo.m_sSelectedSkin
		// Search skins to locate selected skin node
		int iNodeCount = 0;

		// CStdString sSelectedName(g_MenuInfo.m_sSelectedSkin);
		CStdString sValue;
		CStdString sSkinDir;
		CStdString sNamedDir;
		int iSelectedSkin = -1;

		sNamedDir = MakeFullFilePath( g_MenuInfo.m_sSkinsPath, sSelectedName );

		pSkinNode = g_SkinNodes.GetNode( _T("skin"), iNodeCount );
		while( pSkinNode && (iSelectedSkin == -1))
		{
			sValue = pSkinNode->GetString( NULL, _T("name"), _T(""), true );
			sValue.MakeLower();
			if( sValue.Compare( sSelectedName ) == 0 )
			{
				iSelectedSkin = iNodeCount;
				break;
			}
			else
			{
				sSkinDir = pSkinNode->GetString( NULL, _T("Directory"), _T(""), true );
				if ( _tcsicmp( sNamedDir, sSkinDir ) == 0 )
				{
					iSelectedSkin = iNodeCount;
					break;
				}
				else
				{
					iNodeCount++;
					pSkinNode = g_SkinNodes.GetNode( _T("skin"), iNodeCount );
				}
			}
		}
		if ( pSkinNode )
		{
			g_MenuInfo.m_sSelectedSkin = sSelectedName;
			g_MenuInfo.m_pSkinInfo = pSkinNode;
			bReturn = true;
			MakeInfoScreens();
			g_GadgetSkin.Configure(pSkinNode);
		}
	}
	return bReturn;
}


DWORD	CMXMApp::EXEC( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szFilename, LPCTSTR szArgs )
{
	CStdString sFilename;

	sFilename = MakeFullFilePath( szPath, szFilename );
	if ( FileExists( sFilename ) )
	{
		g_MenuInfo.m_bSpecialLaunch = true;
		g_MenuInfo.m_sSpecialLaunch = sFilename;
		g_MenuInfo.m_sSpecialLaunchData = szArgs;
		GetStateApp()->SetGameState( GS_LOAD );
	}
	return 200;
}

DWORD	CMXMApp::CMD( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szCMD, LPCTSTR szArgs, CStdString & sResult )
{
	return 500;
}

HRESULT	CMXMApp::SITE( CXBFTPSession * pThis, LPCTSTR szPath, LPCTSTR szArgs, CStdString & sResult )
{
	HRESULT hr;
	CStdString sLine(szArgs);

	hr = ProcessScriptCommand( sLine, sResult, szPath );
	return hr;
}

#if 0
void CMXMApp::MakeInfoScreens( CXMLNode * pSkinNode )
{
	CXMLNode * pLayoutNode = NULL;
	// Create information screens
	// Network Configuration
	// System Information
	//  Drive Size and Free (C, E, F)
	//  Temperatures
	//  BIOS / Kernel
	pLayoutNode = pSkinNode->AddNode( _T("Layout") );
	if ( pLayoutNode )
	{
		CXMLNode * pLayoutElement;
		pLayoutNode->SetString( NULL, _T("screen"), _T("info01"), true );
		AddBoxNode( pLayoutNode );
		pLayoutElement = AddDefaultString( pLayoutNode, NULL, 0 );
		if ( pLayoutElement )
		{
			pLayoutElement->SetString( NULL, _T("constant"), _T("SYSTEM INFORMATION") );
		}
		pLayoutElement = AddDefaultString( pLayoutNode, _T("BIOSName"), 1 );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("KernelVersion"), 1, false );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("XboxVersion"), 2 );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("SerialNumber"), 2, false );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("TempCPU"), 3 );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("TempSys"), 3, false );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("DriveCSize"), 4 );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("DriveCFree"), 4, false );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("DriveESize"), 5 );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("DriveEFree"), 5, false );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("DriveFSize"), 6 );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("DriveFFree"), 6, false );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("DateTime"), 7 );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("Name"), 7, false );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("IP"), 9 );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("MAC"), 9, false );
		pLayoutElement = AddDefaultString( pLayoutNode, _T("DefaultGateway"), 10 );
		if ( pLayoutElement )
		{
			CXMLNode * pCompound = pLayoutElement->AddNode( _T("multi") );
			if ( pCompound )
			{
				pCompound->AddElement(_T("text"))->SetString( _T("Gateway:"), _T("constant") );
				pCompound->AddElement(_T("text"))->SetString( _T("defaultgateway"), _T("source") );
			}
		}
		pLayoutElement = AddDefaultString( pLayoutNode, _T("SubnetMask"), 10, false );
		if ( pLayoutElement )
		{
			CXMLNode * pCompound = pLayoutElement->AddNode( _T("multi") );
			if ( pCompound )
			{
				pCompound->AddElement(_T("text"))->SetString( _T("Subnet Mask:"), _T("constant") );
				pCompound->AddElement(_T("text"))->SetString( _T("subnetmask"), _T("source") );
			}
		}
		pLayoutElement = AddDefaultString( pLayoutNode, _T("DNS1"), 11 );
		if ( pLayoutElement )
		{
			CXMLNode * pCompound = pLayoutElement->AddNode( _T("multi") );
			if ( pCompound )
			{
				pCompound->AddElement(_T("text"))->SetString( _T("DNS1:"), _T("constant") );
				pCompound->AddElement(_T("text"))->SetString( _T("dns1"), _T("source") );
			}
		}
		pLayoutElement = AddDefaultString( pLayoutNode, _T("dns2"), 11, false );
		if ( pLayoutElement )
		{
			CXMLNode * pCompound = pLayoutElement->AddNode( _T("multi") );
			if ( pCompound )
			{
				pCompound->AddElement(_T("text"))->SetString( _T("DNS2:"), _T("constant") );
				pCompound->AddElement(_T("text"))->SetString( _T("dns2"), _T("source") );
			}
		}
		// CXMLNode * AddDefaultString( CXMLNode * pNode, LPCTSTR szSource, int iRow, bool bLeftSide = true )
	}
}
#endif

bool CMXMApp::RemoveMultiDVDMenu( void )
{
	CXMLNode *pNode = GetMultiDVDNode();
	bool bReturn = false;

	if ( pNode )
	{
		if ( g_MenuInfo.m_pMenuCurrNode == pNode )
		{
			g_MenuInfo.LoadMenu( g_MenuInfo.m_pMenuNode );
		}
		g_MenuInfo.m_pMenuNode->DeleteNode( pNode );
		if ( g_MenuInfo.m_pMenuCurrNode == g_MenuInfo.m_pMenuNode )
		{
			g_MenuInfo.LoadMenu( g_MenuInfo.m_pMenuNode );
		}
		bReturn = true;
	}
	return bReturn;
}

CXMLNode * CMXMApp::GetMultiDVDNode( void )
{
	CXMLNode *pReturnNode = NULL;
	CXMLNode *pNode;
	int iIndex = 0;

	while( (pReturnNode == NULL) && (pNode = g_MenuInfo.m_pMenuNode->GetNode( _T("submenu"), iIndex++ )) )
	{
		if ( _tcsicmp( pNode->GetString( NULL, _T("title"), _T(""), true ), _T("Multigame DVD...") ) == 0 )
		{
			pReturnNode = pNode;
		}
		else if ( _tcsicmp( pNode->GetString( NULL, _T("title"), _T(""), true ), _T("Single Game DVD...") ) == 0 )
		{
			pReturnNode = pNode;
		}
	}
	return pReturnNode;
}

int CMXMApp::LoadMenuFromDVD( CXMLNode * pMainNode, bool bIsAutoLoad )
{
	int iReturn = 0;
	CXMLNode nodeMenu;

	if ( FileExists( _T("D:\\mxm.xml") ) )
	{
		// First scan nodes for "Item_00" entries...
#if 0
		sNodeName.Format(_T("Item_%d"), iItemNum++ );
		pSubNode = pMainNode->GetNode( sNodeName );
		while( pSubNode )
		{
			// Transfer the contents of one node to another!
			pItemNode = g_MenuInfo.m_pMenuNode->AddNode( _T("item") );
			if ( pItemNode )
			{
				pItemNode->m_msAttributes = pSubNode->m_msAttributes;
				pItemNode->m_msElements = pSubNode->m_msElements;
				iItemCount++;
			}
			sNodeName.Format(_T("Item_%d"), iItemNum++ );
			pSubNode = pMainNode->GetNode( sNodeName );
		}

		pMainNode->SetString( NULL, _T("filedirectory"), sDir );
		iItemCount += LoadMenuFromNode( g_MenuInfo.m_pMenuNode, pMainNode );
#endif
	}
	if ( iReturn == 0 && FileExists( _T("D:\\menu.xml")) )
	{
	}
	if ( iReturn == 0 && FileExists( _T("D:\\MenuX\\menu.xdi")) )
	{
	}
	if ( iReturn == 0 && FileExists( _T("D:\\MenuX\\menu.xdi")) )
	{
	}
	return iReturn;
}

void	CMXMApp::OnRunLoopTop()
{
	bool bChanged = g_MenuInfo.Xbox.CheckState();

	if ( GetStateApp()->GetGameState() != GS_LOAD )
	{
		if ( bChanged )
		//if ( ( g_MenuInfo.Xbox.m_iPrevDVDType != g_MenuInfo.Xbox.m_iDVDType ) ||
		//	 ( g_MenuInfo.Xbox.m_bTrayWasClosed && !g_MenuInfo.Xbox.m_bTrayClosed ) ||
		//	 ( !g_MenuInfo.Xbox.m_bTrayWasClosed && g_MenuInfo.Xbox.m_bTrayClosed ) )
		{
			if ( !g_MenuInfo.m_bDVDMode )
			{
				if ( g_MenuInfo.Xbox.m_iDVDType != XBI_DVD_XBOX )
				{

	//				CXMLNode * pNode = GetMultiDVDNode();
	//				if ( pNode )
	//				{
						RemoveMultiDVDMenu();
	//				}
				}
				if ( ( g_MenuInfo.Xbox.m_iDVDType == XBI_DVD_XBOX ) && 
					( g_MenuInfo.m_bAutoLaunchGame || g_MenuInfo.m_bAutoLoadMulti )&& 
					( g_MenuInfo.Xbox.m_iNumFTPUsers == 0 ))
				{
					DWORD dwXBEID;

					dwXBEID = GetXbeID( _T("D:\\default.xbe") );
					if ( dwXBEID == 0x00004321 ) // MXM - we should also handle MenuX discs
					{
						int iNumItems;
						CXMLNode nodeSubMenuD;

						nodeSubMenuD.m_sName = _T("submenu");
						nodeSubMenuD.SetString( NULL, _T("title") , _T("Multigame DVD..."));
						nodeSubMenuD.SetString( NULL, szII_Descr , _T("SubMenu"));

						if ( FileExists( _T("D:\\mxm.xml") ) )
						{
							iNumItems = LoadMenuFromFile( &nodeSubMenuD, _T("D:\\mxm.xml"), true );
							if ( ( iNumItems == 0 ) && ( FileExists( _T("D:\\menu.xml") ) ) )
							{
								iNumItems = LoadMenuFromFile( &nodeSubMenuD, _T("D:\\menu.xml") );
							}
						}
						else if ( FileExists( _T("D:\\menu.xml") ) )
						{
							iNumItems = LoadMenuFromFile( &nodeSubMenuD, _T("D:\\menu.xml") );
						}
						if ( iNumItems == 0 )
						{
							iNumItems = LoadMenuFromDir( &nodeSubMenuD, _T("D:\\"), 0, g_MenuInfo.m_bDeepMultiSearch, true, true, true );
						}
						if ( iNumItems )
						{
							CXMLNode * pDVDNode = NULL;
							RemoveMultiDVDMenu();
							g_MenuInfo.m_pMenuNode->CopyInNode(nodeSubMenuD);
							pDVDNode = GetMultiDVDNode();
							if ( pDVDNode )
							{
								g_MenuInfo.LoadMenu( pDVDNode );
							}
						}
					}
					else if ( dwXBEID == 0x4d415800 )
					{
						// XDI mode!
						CXMLNode * pXDINode = NULL;
						int iNumItems = 0;				
						
						pXDINode = GetXDI();
						if ( pXDINode )
						{
							CXMLNode nodeSubMenuD;
							
							nodeSubMenuD.m_sName = _T("submenu");
							nodeSubMenuD.SetString( NULL, _T("title") , _T("Multigame DVD..."));
							nodeSubMenuD.SetString( NULL, szII_Descr , _T("SubMenu"));
							iNumItems = LoadMenuFromNode( &nodeSubMenuD, pXDINode );
							if ( iNumItems )
							{
								CXMLNode * pDVDNode = NULL;
								RemoveMultiDVDMenu();
								g_MenuInfo.m_pMenuNode->CopyInNode(nodeSubMenuD);
								pDVDNode = GetMultiDVDNode();
								if ( pDVDNode )
								{
									g_MenuInfo.LoadMenu( pDVDNode );
								}
							}
							delete pXDINode;
						}
						// Else, try standard load!
						if ( iNumItems == 0 )
						{
							CXMLNode nodeSubMenuD;
							
							int iNumItems;
							nodeSubMenuD.m_sName = _T("submenu");
							nodeSubMenuD.SetString( NULL, _T("title") , _T("Multigame DVD..."));
							nodeSubMenuD.SetString( NULL, szII_Descr , _T("SubMenu"));
							iNumItems = LoadMenuFromDir( &nodeSubMenuD, _T("D:\\"), 0, g_MenuInfo.m_bDeepMultiSearch, true, true, true );
							if ( iNumItems )
							{
								CXMLNode * pDVDNode = NULL;
								RemoveMultiDVDMenu();
								g_MenuInfo.m_pMenuNode->CopyInNode(nodeSubMenuD);
								pDVDNode = GetMultiDVDNode();
								if ( pDVDNode )
								{
									g_MenuInfo.LoadMenu( pDVDNode );
								}
							}
						}
					}
					else if ( g_MenuInfo.Xbox.m_bTrayClosed && FileExists( _T("D:\\default.xbe") ) ) // Anything else, launch the disc...
					{
						if ( g_MenuInfo.m_bAutoLaunchGame )
						{
							g_MenuInfo.m_bSpecialLaunch = true;
							g_MenuInfo.m_sSpecialLaunch = _T("D:\\default.xbe");
							g_MenuInfo.m_sSpecialLaunchData = _T("");
							GetStateApp()->SetGameState( GS_LOAD );
						}
						else
						{
							// Load single game disc into menu
							CXMLNode nodeSubMenuD;
							
							int iNumItems;
							nodeSubMenuD.m_sName = _T("submenu");
							nodeSubMenuD.SetString( NULL, _T("title") , _T("Single Game DVD..."));
							nodeSubMenuD.SetString( NULL, szII_Descr , _T("SubMenu"));
							iNumItems = LoadMenuFromDir( &nodeSubMenuD, _T("D:\\"), 0, false, true, true, true );
							if ( iNumItems )
							{
								CXMLNode * pDVDNode = NULL;
								RemoveMultiDVDMenu();
								g_MenuInfo.m_pMenuNode->CopyInNode(nodeSubMenuD);
								pDVDNode = GetMultiDVDNode();
								if ( pDVDNode )
								{
									g_MenuInfo.LoadMenu( pDVDNode );
								}
							}
						}
					}
				}
				if ( ( g_MenuInfo.Xbox.m_iDVDType == XBI_DVD_MOVIE ) && 
					g_MenuInfo.m_bAutoLaunchMovie &&
					( g_MenuInfo.Xbox.m_iNumFTPUsers == 0 ) &&
					g_MenuInfo.m_sDVDPlayer.GetLength() )
				{
					// 0xffadf32f is the code for the DVD player
					g_MenuInfo.m_bSpecialLaunch = true;
					g_MenuInfo.m_sSpecialLaunch = g_MenuInfo.m_sDVDPlayer;
					g_MenuInfo.m_sSpecialLaunchData = g_MenuInfo.m_sDVDPlayerCmdLine;
					GetStateApp()->SetGameState( GS_LOAD );
				}
			}
			else
			{
				if ( !g_MenuInfo.Xbox.m_bTrayWasClosed && g_MenuInfo.Xbox.m_bTrayClosed )
				{
					GetStateApp()->Reboot();
				}
			}
		}
		g_MenuInfo.Xbox.m_bTrayWasClosed = g_MenuInfo.Xbox.m_bTrayClosed;
		g_MenuInfo.Xbox.m_iPrevDVDType = g_MenuInfo.Xbox.m_iDVDType;
	}
}

CMXMApp * GetMXMApp( void )
{
	return (CMXMApp *)GetStateApp();
}

void	CMXMApp::InitializeFTP( void )
{

#ifdef USE_XBFILEZILLA

	if ( !m_bNetInitialized )
	{
		if (InitializeNetwork())
		{
			m_bNetInitialized = true;
		}
	}
	if (m_bNetInitialized)
	{
		m_pXBFileZilla = new CXBFileZilla(_T("D:\\"));
		// make sure to call SetConfigurationPath() before Start() or to specify path in constructor
		//gXBFileZilla.SetConfigurationPath(_T("C:\\"));
		m_pXBFileZilla->AddNotificationClient(this);
		m_pXBFileZilla->SetCriticalOperationCallback(::CriticalOperationCallbackMXM);
  
		if (m_pXBFileZilla->Start())
		{
			m_pXBFileZilla->mSettings.SetServerPort(1021);
			m_pXBFileZilla->mSettings.SetWelcomeMessage("Welcome to Media X Menu\n");
			m_pXBFileZilla->mSettings.SetAdminPass("MXMOwner");
			m_pXBFileZilla->mSettings.SetAdminIPBindings("");
			m_pXBFileZilla->mSettings.SetAdminIPAddresses("192.168.0.*");
	    
			m_pXBFileZilla->mSettings.SetSfvEnabled(true); // will use sfv file for checking crc of files
			m_pXBFileZilla->mSettings.SetCrcEnabled(true); // will display crc of file at prompt, though it's a bit unnnecessary if sfv is also enabled

			

			m_pXBFileZilla->mSettings.SetFreeSpace(_T("C:"), true);
			m_pXBFileZilla->mSettings.SetFreeSpace(_T("E:\\"), true);
			m_pXBFileZilla->mSettings.SetFreeSpace(_T("F:\\"), true);
		}

	}

#else

	m_pFTPPatchesServer = new CXBFTPServer();
	if ( m_pFTPPatchesServer )
	{
		m_pFTPPatchesServer->Configure( g_MenuInfo.m_pCfgInfo->GetNode( _T("ftpserver") ) );
		m_pFTPPatchesServer->SetXferPriority( g_MenuInfo.m_iFTPPriority );
		m_pFTPPatchesServer->SetFTPEventHandler(this);
//		m_pFTPPatchesServer->m_bAllowAnon = (g_MenuInfo.m_pPrefInfo->GetBool( _T("network"), _T("ftpanon"), (m_pFTPPatchesServer->m_bAllowAnon)?TRUE:FALSE ))?true:false;

//	WORD				m_wPort;
//	CStdString			m_sAnonRoot;
//	bool				m_bAllowAnon;
		
	}
//	m_pFTPPatchesServer = new CXBFTPServer();


#endif

}

#ifdef USE_XBFILEZILLA

XFSTATUS CMXMApp::CriticalOperationCallback(SXFCriticalOperation Operation)
{
	XFSTATUS xfsReturn = XFS_NOT_IMPLEMENTED;
	if (Operation.mOperation == SXFCriticalOperation::Reboot)
	{
		// persist settings
//		gXBFileZilla.Stop();
//		OutputDebugString(_T("CriticalOperationCallbackExample(): Reboot\n"));
//		XLaunchNewImage( NULL, NULL); 
	}
	else if (Operation.mOperation == SXFCriticalOperation::Shutdown)
	{
		// persist settings
//		gXBFileZilla.Stop();
//		OutputDebugString(_T("CriticalOperationCallbackExample(): Shutdown\n"));
//		HalInitiateShutdown();
	}
	else if (Operation.mOperation == SXFCriticalOperation::LaunchXBE)
	{
		// persist settings
//		OutputDebugString(_T("CriticalOperationCallbackExample(): LaunchXBE\n"));
//		return XFS_NOT_IMPLEMENTED;
	}
	return xfsReturn;
}

#endif


void CMXMApp::RenderInitStyle( DWORD dwFrameCount, bool bFade )
{
	int iX, iY; // , iCol;
	DWORD dwFramePct;
	float fOffset;
	static DWORD dwTrueFrameCount = 0;


	dwFramePct = dwTrueFrameCount%100;
	fOffset = (float)dwFramePct;
	fOffset /= 50;


	switch ( g_MenuInfo.m_iIntroOption )
	{
		case 0: // Matrix intro screen
			m_matrix.SetStatusArray( g_pszInitStatus );
			if ( dwTrueFrameCount == 0 )
			{
				DWORD dwColor = 0xff00ff00;

				if ( g_MenuInfo.m_iIntroColor == 0 )
				{
					g_MenuInfo.m_iIntroColor = (RandomNum()%6)+1;
				}

				switch( g_MenuInfo.m_iIntroColor )
				{
					case 1:
						dwColor = 0xffff0000;
						break;
					case 2:
						dwColor = 0xff00ff00;
						break;
					case 3:
						dwColor = 0xff0000ff;
						break;
					case 4:
						dwColor = 0xffffff00;
						break;
					case 5:
						dwColor = 0xff00ffff;
						break;
					case 6:
						dwColor = 0xffff00ff;
						break;
				}
				m_matrix.Initialize( m_pd3dDevice, true, dwColor );
			}
			else
			{
				m_matrix.Render( m_pd3dDevice, bFade );
			}
			break;
		case 1: // Sky and logo screen
			DrawBox( 0.0, 0.0, 640.5, 480.5, 0xff000000, 0xff000000 );

			if ( m_imgTile.GetTexture() )
			{
				DrawSpriteOffset(m_pd3dDevice, m_imgTile.GetTexture(), 0, 0, 640, 480, (FLOAT)fOffset, (FLOAT)fOffset, (FLOAT)(fOffset+1.5), (FLOAT)(fOffset+1.5), FALSE, FALSE, -1, D3DTADDRESS_MIRROR );
				//RenderTile( m_imgTile.GetTexture(), 640, 480 );
			}
			if ( m_imgLogo.GetTexture() )
			{
				iX = 640-m_imgLogo.GetWidth();
				iX >>= 1;
				iY = 480-m_imgLogo.GetHeight();
				iY >>= 1;
				DrawSprite(m_pd3dDevice, m_imgLogo.GetTexture(), iX, iY, m_imgLogo.GetWidth(), m_imgLogo.GetHeight(), TRUE, TRUE );
			}
			break;
		case 2: // Sky and logo (small) screen
			DrawBox( 0.0, 0.0, 640.5, 480.5, 0xff000000, 0xff000000 );

			if ( m_imgTile.GetTexture() )
			{
				DrawSpriteOffset(m_pd3dDevice, m_imgTile.GetTexture(), 0, 0, 640, 480, (FLOAT)fOffset, (FLOAT)fOffset, (FLOAT)(fOffset+1.5), (FLOAT)(fOffset+1.5), FALSE, FALSE, -1, D3DTADDRESS_MIRROR );
				//RenderTile( m_imgTile.GetTexture(), 640, 480 );
			}
			if ( m_imgLogo.GetTexture() )
			{
				iX = 640-(m_imgLogo.GetWidth()>>1);
				iX >>= 1;
				iY = 480-(m_imgLogo.GetHeight()>>1);
				iY >>= 1;
				DrawSprite(m_pd3dDevice, m_imgLogo.GetTexture(), iX, iY, m_imgLogo.GetWidth()>>1, m_imgLogo.GetHeight()>>1, TRUE, TRUE );
			}
			break;
		case 3: // Simple style (Matrix without effect)
			m_matrix.SetStatusArray( g_pszInitStatus );
			if ( dwTrueFrameCount == 0 )
			{
				DWORD dwColor = 0xff00ff00;

				if ( g_MenuInfo.m_iIntroColor == 0 )
				{
					g_MenuInfo.m_iIntroColor = (RandomNum()%6)+1;
				}

				switch( g_MenuInfo.m_iIntroColor )
				{
					case 1: // Red
						dwColor = 0xffff0000;
						break;
					case 2: // Green
						dwColor = 0xff00ff00;
						break;
					case 3: // Blue
						dwColor = 0xff0000ff;
						break;
					case 4: // Yellow
						dwColor = 0xffffff00;
						break;
					case 5: // Cyan
						dwColor = 0xff00ffff;
						break;
					case 6: // Magenta
						dwColor = 0xffff00ff;
						break;
				}
				m_matrix.Initialize( m_pd3dDevice, true, dwColor, false );
			}
			else
			{
				m_matrix.Render( m_pd3dDevice, bFade );
			}
			break;
	}

	dwTrueFrameCount++;
}

// g_MenuInfo.m_iIntroOption=0
void CMXMApp::RenderInitScreen(DWORD dwFrameCount, bool bFade )
{
	int iBlendAmount;
	float fAlpha;
//	int iX, iY;
	DWORD dwFramePct;
	float fOffset;


	EnterCriticalSection( &g_RenderCriticalSection );
	QueryInput();
	// Handle input....
	if ( GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_BLACK] )
	{
	}
	else if ( GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_WHITE] )
	{
	}
	else if ( GetDefaultGamepad()->wPressedButtons & XINPUT_GAMEPAD_START )
	{
		// Trigger safe mode.
		g_MenuInfo.SafeMode.m_bSetUserSafeMode = true;
	}

	dwFramePct = m_dwFrameCount%100;
	fOffset = (float)dwFramePct;
	fOffset /= 50;

	try
	{
		m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0L );
	}
	catch(...)
	{
	}

	if ( !bFade )
	{

		DrawBox( 0.0, 0.0, 640.5, 480.5, 0xff000000, 0xff000000 );

		RenderInitStyle( dwFrameCount, bFade );
		dwFrameCount*=4;
		if ( dwFrameCount < 255 )
		{
		
			iBlendAmount = 255-dwFrameCount;
			fAlpha = (float)iBlendAmount;
			fAlpha /= 255;
			//Display the Persist image,if there
			if( m_imgPersistImage.GetTexture() )
			{
				RenderQuad(m_imgPersistImage.GetTexture(), 640, 480, fAlpha );
			}

		}
		// DrawBox( 0.0, 0.0, 100.0, 100.0, 0x80000000, 0xffffff00 );
	}
	else
	{
		// Fade out...

		DrawBox( 0.0, 0.0, 640.5, 480.5, 0xff000000, 0xff000000 );

		if ( dwFrameCount < 64 )
		{

			RenderInitStyle( dwFrameCount, bFade );
			dwFrameCount*=4;
		}
		else
		{
			dwFrameCount = 255;
		}
		DrawBox( 0.0, 0.0, 640.5, 480.5, (dwFrameCount<<24), (dwFrameCount<<24) );
	}
    m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	LeaveCriticalSection( &g_RenderCriticalSection );
}

DWORD CMXMApp::InitializeThreadProc(LPVOID lpParameter )
{
	DWORD dwReturn = 0;
	CMXMApp * pThis = (CMXMApp*)lpParameter;

	pThis->InitializeThread();

	return dwReturn;
}

DWORD CMXMApp::FTPThreadProc(LPVOID lpParameter )
{
	DWORD dwReturn = 0;
	CMXMApp * pThis = (CMXMApp*)lpParameter;

	pThis->FTPThread();

	return dwReturn;
}

void CMXMApp::FTPThread( void )
{
	m_pFTPPatchesServer->Start();
}

void CMXMApp::InitializeThread( void )
{
	DWORD dwResult;
	DWORD dwFrameCount=0;
    //
    // Our thread procedure basically just loops indefinitely,
    // processing packets and switching between songs as 
    // appropriate, until we're signalled to shutdown.
    //
    do
    {
		// OK, here we render the "initializing" screen....
		RenderInitScreen(dwFrameCount++, false);
		m_dwFrameCount++;
        dwResult = WaitForSingleObject( m_hFadeEvent, 1000 / 60 );
		if ( dwResult == WAIT_TIMEOUT )
		{
	        dwResult = WaitForSingleObject( m_hShutdownEvent, 1000 / 60 );
		}
		while ( m_bPauseInitScreen )
		{
			Sleep( 1000/60 );
		}

		// Temp, just for testing....

//		dwResult = WAIT_TIMEOUT;
    } while( dwResult == WAIT_TIMEOUT && !m_bFinishInitThread );
	dwFrameCount=0;
    do
    {
		// OK, here we render the "initializing" screen....
		RenderInitScreen(dwFrameCount++, true);
		m_dwFrameCount++;
        dwResult = WaitForSingleObject( m_hShutdownEvent, 1000 / 60 );

		// Temp, just for testing....

//		dwResult = WAIT_TIMEOUT;
    } while( dwResult == WAIT_TIMEOUT && !m_bFinishInitThread2 );

	m_imgPersistImage.ReleaseResources();
	SetEvent( m_hThreadDoneEvent );

}


DWORD CMXMApp::Prompt( LPCTSTR szMsg, DrawingAttributes * pAttributes )
{
	DrawingAttributes daTemp;

	if ( pAttributes == NULL )
	{
		pAttributes = &daTemp;
		daTemp.m_dwBorderColor = 0xffffffff;
		daTemp.m_dwBoxColor = 0xff000000;
		daTemp.m_dwTextColor = 0xffffffff;
		daTemp.m_dwShadowColor = 0xff000000;
	}
	DWORD dwReturn = 0;
	if ( szMsg && _tcslen( szMsg ) )
	{
		m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0L );
		::DrawTextBox( m_pd3dDevice, szMsg, &m_Font16, pAttributes );
	    m_pd3dDevice->Present( NULL, NULL, NULL, NULL );

		while( dwReturn == 0 )
		{
			QueryInput();
			Sleep( 10 );
			dwReturn = 0;
			for( int iIndex=0; iIndex<8; iIndex++ )
			{
				if ( m_DefaultGamepad.bPressedAnalogButtons[iIndex] )
				{
					dwReturn |= (0x00010000<<iIndex);
				}
			}
			dwReturn |= m_DefaultGamepad.wPressedButtons;
		}
	}
	return dwReturn;
}

HRESULT CMXMApp::Initialize()
{
	HRESULT  hr = E_FAIL;
    VOID *dwLoc = NULL;

	m_bFinishInitThread = false;
	m_bFinishInitThread2 = false;
	PerfGetPerformance( _T("MXMApp Initialize"), FALSE );
	g_MenuInfo.UpdateSafeMode( _T("005: 3D Init") );
	if ( SUCCEEDED( hr = Init3DWorld() ) )
	{
		LPDIRECT3DTEXTURE8 pTexture = NULL;
		hr = GetPersistentTexture( Get3dDevice(), &pTexture );
		PerfGetPerformance( _T("MXMApp PersistScreen"), FALSE );

		m_imgPersistImage.Initialize( Get3dDevice(), GetSoundDevice() );
		m_imgPersistImage.Attach( pTexture );

		PerfGetPerformance( _T("MXMApp InitLogoAndTile Objects"), FALSE );

		m_imgLogo.Initialize( Get3dDevice(), GetSoundDevice() );

		m_imgTile.Initialize( Get3dDevice(), GetSoundDevice() );

		dwLoc = XLoadSection(_T("LogoData"));

		PerfGetPerformance( _T("MXMApp Load LogoSection"), FALSE );
		if(dwLoc!=NULL)
		{
			m_imgLogo.LoadImageMem( g_pbLargeLogoData, g_dwLargeLogoLength);
			m_imgTile.LoadImageMem(g_pbSkyTileData,g_dwSkyTileLength);
		}
		XFreeSection(_T("LogoData"));

		PerfGetPerformance( _T("MXMApp Create Render Threads"), FALSE );

		// Create an event to signal the worker thread to fade to black
		m_hFadeEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
		if( NULL == m_hFadeEvent )
			return E_FAIL;
		// Create an event to signal the worker thread to shutdown
		m_hShutdownEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
		if( NULL == m_hShutdownEvent )
			return E_FAIL;
		m_hThreadDoneEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
		if( NULL == m_hThreadDoneEvent )
			return E_FAIL;

		// HAve to do this initially...
		if ( FAILED( InitInputDevices()) )
		{
			return E_FAIL;
		}
		// Create the worker thread
		m_hThread = CreateThread( NULL, WORKER_THREAD_STACK_SIZE, InitializeThreadProc, this, 0, NULL );
		if( NULL == m_hThread )
		{
			return E_FAIL;
		}
		if ( pTexture )
		{
			pTexture->Release();
		}

	}
	else
	{
		g_MenuInfo.UpdateSafeMode( _T("005: FAILED 3D Init") );
	}

	g_MenuInfo.UpdateSafeMode( _T("006: State App Init") );

	PerfGetPerformance( _T("MXMApp StateApp Init"), FALSE );

	hr = CXBoxStateApplication::Initialize();

	PerfGetPerformance( _T("MXMApp StateApp Init Return (adding fonts)"), FALSE );

	g_MenuInfo.UpdateSafeMode( _T("007: Internal Font Init") );
	g_FontCache.SetDevice( m_pd3dDevice );
	g_FontCache.AddMemFont( _T("FontData"), (const LPBYTE)pbFontData, _T("default") );

	g_FontCache.AddMemFont( _T("LogoData"), (const LPBYTE)g_pbMatrixFont, _T("def_matrix") );

	CFontEntry * pEntry = g_FontCache.GetFontEntryByResname( _T("default") );

	if ( pEntry )
	{
		pEntry->AddName( _T("font16" ) );
	}


	// Assigns fonts for items driven from the system menu class
	CAppMenu::s_pMenuFont = &m_Font16;
	CAppMenu::s_pWidgetFont = &m_Font16;


	//CStdString sTestWrap( _T("This is a test of the word wrap function of the font that I've just created") );
	//CStdString sTestWrap2( _T("\nThis\nis\na test of the word wrap function of the font that I've just created abcdefghijklmnopqrstuvwxyzsuperduperlongstringthatshouldgetchoppedup!") );

	//m_Font16.WrapString( sTestWrap, 100 );
	//m_Font16.WrapString( sTestWrap2, 100 );


	// g_MenuInfo.SafeMode.m_bActive = true;
	if ( g_MenuInfo.SafeMode.m_bActive )
	{
		// PResent "Safe Mode" menu here...
		m_bPauseInitScreen = true;
		EnterCriticalSection( &g_RenderCriticalSection );

		g_MenuInfo.SafeMode.m_dwFlags = 0;

//#define SM_NONETWORK		0x00000001
//#define SM_NOVIDEO			0x00000002
//#define SM_BASICSKIN		0x00000004
//#define SM_SAFEMENU			0x00000008
//#define SM_NOFTP			0x00000010

		CStdString sSafeModeMenu;

		sSafeModeMenu.Format( _T("****Safe Mode Options****\r\n(%s)\r\n\r\n"), g_MenuInfo.SafeMode.m_sSafeMsg.c_str() );

		sSafeModeMenu += _T("A: Continue normal start\r\nB: No Network\r\nX: Disable Video Support\r\nY: Basic Skin\r\nSTART: AutoMenu\r\nBACK: Disable Music");

		int iResponse = Prompt( sSafeModeMenu );
		LeaveCriticalSection( &g_RenderCriticalSection );
		m_bPauseInitScreen = false;
		switch( iResponse )
		{
			case XINPUT_GAMEPAD_DPAD_UP:  
				break;
			case XINPUT_GAMEPAD_DPAD_DOWN:
				break;
			case XINPUT_GAMEPAD_DPAD_LEFT:
				break;
			case XINPUT_GAMEPAD_DPAD_RIGHT:
				break;
			case XINPUT_GAMEPAD_START:
				g_MenuInfo.SafeMode.m_dwFlags = SM_SAFEMENU;
				break;
			case XINPUT_GAMEPAD_BACK:
				g_MenuInfo.SafeMode.m_dwFlags = SM_NOMUSIC;
				g_MenuInfo.m_bUseMusicManager = FALSE;
				m_bInitMusicManager = true;
				break;
			case XINPUT_GAMEPAD_LEFT_THUMB:
				break;
			case XINPUT_GAMEPAD_RIGHT_THUMB:
				break;
			case (0x00010000<<XINPUT_GAMEPAD_A):
				g_MenuInfo.SafeMode.m_dwFlags = 0;
				break;
			case (0x00010000<<XINPUT_GAMEPAD_B):
				g_MenuInfo.SafeMode.m_dwFlags = SM_NONETWORK;
				break;
			case (0x00010000<<XINPUT_GAMEPAD_X):
				g_MenuInfo.SafeMode.m_dwFlags = SM_NOVIDEO;
				// Kill the video support!
				CImageSource::m_bAllowVideo = false;
				break;
			case (0x00010000<<XINPUT_GAMEPAD_Y):
				g_MenuInfo.SafeMode.m_dwFlags = SM_BASICSKIN;
				break;
			case (0x00010000<<XINPUT_GAMEPAD_BLACK):
				break;
			case (0x00010000<<XINPUT_GAMEPAD_WHITE):
				break;
			case (0x00010000<<XINPUT_GAMEPAD_LEFT_TRIGGER):
				break;
			case (0x00010000<<XINPUT_GAMEPAD_RIGHT_TRIGGER):
				break;
		}
	}

	PerfGetPerformance( _T("MXMApp Finishing init"), FALSE );
	

	g_MenuInfo.UpdateSafeMode( _T("008: Final Init") );

	FinishIntialization();

	if ( !( g_MenuInfo.SafeMode.m_dwFlags & SM_NONETWORK ) 
			&& g_MenuInfo.m_pPrefInfo->GetBool( _T("network"), _T("enablenetwork"), TRUE )
			&& g_MenuInfo.m_pPrefInfo->GetBool( _T("network"), _T("enableftp"), TRUE ) )
	{
		m_matrix.SetStatusIdx(MSG_INIT_FTP);

		g_MenuInfo.UpdateSafeMode( _T("009: FTP Server Intialization") );
		InitializeFTP();

		// Create the worker thread
		m_hFTPThread = CreateThread( NULL, WORKER_THREAD_STACK_SIZE, FTPThreadProc, this, 0, NULL );
		if( NULL == m_hFTPThread )
		{
			return E_FAIL;
		}
	}

	g_MenuInfo.UpdateSafeMode( _T("010: Init Screen Fadeout") );

//	Sleep( 1000 );
	SetEvent( m_hFadeEvent );

	Sleep(500);

	SetEvent( m_hShutdownEvent );
	m_bFinishInitThread = true;

	SetDelays( 700, 200 );


    if ( WaitForSingleObject( m_hThreadDoneEvent, 3000 ) == WAIT_TIMEOUT )
	{
		m_bFinishInitThread2 = true;
		WaitForSingleObject( m_hThreadDoneEvent, 1000 );
	}

	PerfGetPerformance( _T("Done with MXMApp Initialization"), FALSE );
	if ( m_pCurrentGameState )
	{
		m_pCurrentGameState->OnDeactivation();
		// Just trigger activation now... to reset timers and such.
		m_pCurrentGameState->OnActivation();
	}

	// Run AutoExec scripts here...
	{
		CStdString sAction, sAutoExec;

		sAction = GetInternalScript( _T("_AutoExec") );
		if ( sAction.GetLength() )
		{
			LaunchActionScript( sAction );
		}

		sAction = GetInternalScript( _T("AutoExec") );
		if ( sAction.GetLength() )
		{
			LaunchActionScript( sAction );
		}

		sAutoExec = MakeFullFilePath(g_MenuInfo.m_sMXMPath, _T("autoexec.xas") );

		if ( FileExists( sAutoExec ) )
		{
			sAction = LoadStringFromFile( sAutoExec );
			if ( sAction.GetLength() )
			{
				LaunchActionScript( sAction );
			}
		}
	}


	return hr;
}

void CMXMApp::FirstRender( void )
{
//	SetBlend();
	g_MenuInfo.m_skin.MakeResources( Get3dDevice(), GetSoundDevice() );
}


void CMXMApp::OnMusicManagerFail( void )
{
	g_MenuInfo.m_bUseMusicManager = FALSE;
}

//void	CMXMApp::MusicPause( void )
//{
//	if ( m_pMusicManager && g_MenuInfo.m_bUseMusicManager )
//	{
//		m_pMusicManager->Pause();
//	}
//}

//void	CMXMApp::MusicResume( void )
//{
//	if ( m_pMusicManager && g_MenuInfo.m_bUseMusicManager  )
//	{
//		if ( m_pMusicManager->GetStatus() != MM_PLAYING )
//		{
//			m_pMusicManager->Play();
//		}
//	}
//}


HRESULT CMXMApp::LoadLocalSoundtrack( void )
{
	HRESULT hr = S_OK;
	CSoundtrack * pST = NULL;
	CStdString sTrack, sDir;
	CItemInfo itemInfo;
	int iLocalSongCount = 0;
	
	if ( m_pMusicManager )
	{
		pST = m_pMusicManager->GetGameSoundtrack(); 
		if ( pST )
		{
			
			// Load tracks here....
			if ( g_MenuInfo.m_saMusicFiles.size() )
			{
				list<CStdString>::iterator iterFiles;

				iterFiles = g_MenuInfo.m_saMusicFiles.begin();
				while( iterFiles != g_MenuInfo.m_saMusicFiles.end() )
				{
					sTrack = MakeFullFilePath( g_MenuInfo.m_sMXMPath, iterFiles->c_str(), FALSE );
					DEBUG_FORMAT( _T("Adding file (%s) to local soundtrack"), sTrack.c_str() );
					if ( pST->AddSong( _T("Ambient"), sTrack, 0, TRUE ) )
					{
						iLocalSongCount++;
					}
					iterFiles++;
				}
			}

			if ( g_MenuInfo.m_saMusicDirectories.size() )
			{
				list<CStdString>::iterator iterDirs;

				iterDirs = g_MenuInfo.m_saMusicDirectories.begin();
				while( iterDirs != g_MenuInfo.m_saMusicDirectories.end() )
				{
					sDir = MakeFullDirPath( g_MenuInfo.m_sMXMPath, iterDirs->c_str(), FALSE );
					DEBUG_FORMAT( _T("Adding dir (%s) to local soundtrack"), sDir.c_str() );
					iLocalSongCount += pST->LoadSongs( sDir.c_str() );
					iterDirs++;
				}
			}

			//if ( g_MenuInfo.m_sMusicDirectory.GetLength() )
			//{
			//	sDir = MakeFullDirPath( g_MenuInfo.m_sMXMPath, g_MenuInfo.m_sMusicDirectory.c_str(), FALSE );
			//	// sDir = MakeFullDirPath( _T("D:\\"), g_MenuInfo.m_sMusicDirectory.c_str(), FALSE );
			//	DEBUG_LINE( sDir.c_str() );
			//	pST->LoadSongs( sDir.c_str() );
			//}
		}

		if ( iLocalSongCount == 0 )
		{
			g_MenuInfo.m_bGlobalMusic = TRUE;
		}

		// check to insure there's music. If not, don't use the soundtrack feature at all!!!
		if ( m_pMusicManager->GetSongCount() )
		{
			// Hmmmm.... no
			MusicSetRandom( (g_MenuInfo.m_bRandomMusic==TRUE) );
			MusicSetGlobal( (g_MenuInfo.m_bGlobalMusic==TRUE) );
			MusicPause();
			MusicInitSongSelection();
		}
		else
		{
			// Ugh. No songs! Disable!!
			g_MenuInfo.SafeMode.m_dwFlags = SM_NOMUSIC;
			g_MenuInfo.m_bUseMusicManager = FALSE;
			delete m_pMusicManager;
			m_pMusicManager = NULL;
		}
	}
	else
	{
		hr = E_FAIL;
	}
	return hr;
}


int CMXMApp::LoadMenuFromDir( CXMLNode * pMainNode, LPCTSTR szDir, DWORD dwFlags, bool bRecurse, bool bFlatten, bool bCheckThisDir, bool bKeepD )
{
	int iReturn = 0;
	CStdString sWildCard;
	HANDLE hFind;
	WIN32_FIND_DATA ffData;
	CStdString sFilename;
	CStdString sNewPath;
	CItemInfo itemInfo;
	bool bPassCheckList;
	bool bNoDemo;
	bool bDefaultXbeOnly;

	bNoDemo = ((dwFlags&ADIR_FLG_NODEMO) == ADIR_FLG_NODEMO);
	bDefaultXbeOnly = ((dwFlags&ADIR_FLG_DEFONLY) == ADIR_FLG_DEFONLY);

	DEBUG_FORMAT( _T("LoadMenuFromDir: %s"), szDir );
	
	if ( szDir && _tcslen(szDir) )
	{
		if ( bKeepD )
		{
			bPassCheckList = true;
		}
		else
		{
			bPassCheckList = (CheckList( g_MenuInfo.m_slMenuDirList, szDir, true, true ) == false);
		}
		if ( bPassCheckList )
		{
			
			sWildCard = MakeFullFilePath( szDir, _T("*") );

			hFind = FindFirstFile( sWildCard, &ffData );
			if( INVALID_HANDLE_VALUE != hFind )
			{
				do
				{
					if ( ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
					{
						CStdString sDirName(ffData.cFileName);
						sDirName.MakeUpper();
						bool bNotADemo  = true;

						if ( bNoDemo && ( sDirName.Find( _T("DEMO") ) != -1 ) )
						{
							bNotADemo = false;
						}

						// Ignore if it's a data directory
						// We won't recurse on that one.
						if ( sDirName.Compare( _T("UDATA") ) &&
							 sDirName.Compare( _T("TDATA") ) &&
							 bNotADemo )
						{
							sNewPath = MakeFullFilePath( szDir, ffData.cFileName );

							if ( bRecurse )
							{
								if ( bFlatten )
								{
									iReturn += LoadMenuFromDir( pMainNode, sNewPath, dwFlags, true, true, false, bKeepD );
								}
								else
								{
									// Make subnode...
									CXMLNode nodeSubMenu;
									CStdString sTitle;
									int iNumEntries;

									sTitle.Format( _T("%s..."),ffData.cFileName);
									nodeSubMenu.m_sName = _T("submenu");
									nodeSubMenu.SetString( NULL, _T("title"), sTitle, false );
									nodeSubMenu.SetString( NULL, _T("description"), _T("SubMenu"), false );
									iNumEntries = LoadMenuFromDir( &nodeSubMenu, sNewPath, dwFlags, true, false, false, bKeepD );
									if ( iNumEntries )
									{
										pMainNode->CopyInNode( nodeSubMenu, _T("submenu") );
									}
									iReturn += iNumEntries;
								}
							}
							//else
							//{
							//	if ( CheckDirectory( sNewPath, itemInfo ) )
							//	{
							//		itemInfo.BuildNode( pMainNode );
							//		iReturn++;
							//	}
							//}
						}
						// OK.... check for valid MXM_Entry.xml file
						// OR.... Menu.xml file (load menu file in that case
						// OR.... *.xbe
					}
				} while( FindNextFile( hFind, &ffData ) );
				// Close the find handle.
				FindClose( hFind );
			}

			hFind = FindFirstFile( sWildCard, &ffData );
			if( INVALID_HANDLE_VALUE != hFind )
			{
				do
				{
					if ( ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
					{
						CStdString sDirName(ffData.cFileName);
						sDirName.MakeUpper();
						bool bNotADemo  = true;

						if ( bNoDemo && ( sDirName.Find( _T("DEMO") ) != -1 ) )
						{
							bNotADemo = false;
						}

						// Ignore if it's a data directory
						// We won't recurse on that one.
						if ( sDirName.Compare( _T("UDATA") ) &&
							 sDirName.Compare( _T("TDATA") ) &&
							 bNotADemo )
						{
							sNewPath = MakeFullFilePath( szDir, ffData.cFileName );
//							sFilename = MakeFullFilePath( sNewPath, _T("MXM_Entry.xml") );
//							if ( FileExists( sFilename ) )
//							{
//								iReturn += LoadMenuFromFile( pMainNode, sFilename );
//								DEBUG_FORMAT( _T("Adding %s to menu"), sFilename.c_str() );
//							}
//							else
							{
								// Now check THIS directory....
								if ( CheckDirectory( sNewPath, itemInfo, bKeepD, bDefaultXbeOnly ) )
								{
									itemInfo.BuildNode( pMainNode );
									iReturn++;
								}
							}
						}
						// OK.... check for valid MXM_Entry.xml file
						// OR.... Menu.xml file (load menu file in that case
						// OR.... *.xbe
					}
				} while( FindNextFile( hFind, &ffData ) );
				// Close the find handle.
				FindClose( hFind );
			}
		}
		// Now check THIS directory....
		if ( bCheckThisDir )
		{
			if ( CheckDirectory( szDir, itemInfo, bKeepD, bDefaultXbeOnly ) )
			{
				itemInfo.BuildNode( pMainNode );
				iReturn++;
			}
		}
	}
	
	return iReturn;
}



int CMXMApp::LoadMenuFromFile( CXMLNode * pMainNode, LPCTSTR szFile, bool bCheckOldStyle )
{
	int iReturn = 0;
	CXMLNode * pFileNode;
	CXMLLoader xmlLoader(NULL,true);

	DEBUG_FORMAT( _T("Loading Menu from file: (%s)================\r\n"), szFile );
	if ( szFile && _tcslen( szFile ) && FileExists( szFile ) )
	{
		if ( !CheckList( g_MenuInfo.m_slMenuFileList, szFile, true, true ) )
		{
			pFileNode = xmlLoader.LoadXMLNodes( szFile );
			pFileNode->SetString( NULL, _T("filedirectory"), StripFilenameFromPath(szFile) );

			if ( bCheckOldStyle )
			{
				CStdString sNodeName;
				int iItemNum = 1;
				CXMLNode * pSubNode;
				CXMLNode * pItemNode;

				sNodeName.Format(_T("Item_%d"), iItemNum++ );
				pSubNode = pFileNode->GetNode( sNodeName );
				while( pSubNode )
				{
					// Transfer the contents of one node to another!
					pItemNode = pMainNode->AddNode( _T("item") );
					if ( pItemNode )
					{
						pItemNode->m_msAttributes = pSubNode->m_msAttributes;
						pItemNode->m_msElements = pSubNode->m_msElements;
						iReturn++;
					}
					sNodeName.Format(_T("Item_%d"), iItemNum++ );
					pSubNode = pFileNode->GetNode( sNodeName );
				}
			}
			iReturn += LoadMenuFromNode( pMainNode, pFileNode );
		}
	}
	return iReturn;
}


int CMXMApp::LoadMenuFromNode( CXMLNode * pMainNode, CXMLNode * pFileNode )
{
	int iItemCount = 0;
	int iNodeCount = 0;
	CXMLNode * pSubNode;
	CStdString sNodeName;
	CStdString sValue;
	CStdString sDir;

	sDir = pFileNode->GetString( NULL, _T("filedirectory"), _T(""), true );
	// Now we look over ALL nodes contained and also handle submenus and other goodies...
	pSubNode = pFileNode->GetNode( iNodeCount );
	while( pSubNode )
	{
		sNodeName = pSubNode->m_sName;
		sNodeName.MakeLower();
		if ( sNodeName.Compare( _T("item") ) == 0 )
		{
			// Need to look for MenuFile or AutoDir
			sValue = pSubNode->GetString( NULL, _T("menufile"), _T(""), true );
			if ( sValue.GetLength() )
			{
				// This mans we need to load a new file and insert the items here at this level
				iItemCount += LoadMenuFromFile( pMainNode, sValue );
			}
			else
			{
				bool bFlatten, bRecurse;
				sValue = pSubNode->GetString( NULL, _T("autodir"), _T(""), true );
				if ( sValue.GetLength() )
				{
					DWORD dwFlags = 0;
					//#define ADIR_FLG_NODEMO	0x00000001
					//#define ADIR_FLG_DEFONLY	0x00000002
					bFlatten = (pSubNode->GetBool( _T("autodir"), _T("flatten"), FALSE ) == TRUE);
					bRecurse = (pSubNode->GetBool( _T("autodir"), _T("recurse"), TRUE ) == TRUE);
					if ( (pSubNode->GetBool( _T("autodir"), _T("nodemo"), FALSE ) ) )
					{
						dwFlags |= ADIR_FLG_NODEMO;
					}
					if ( (pSubNode->GetBool( _T("autodir"), _T("defaultonly"), FALSE ) ) )
					{
                        dwFlags |= ADIR_FLG_DEFONLY;
					}
					// In this case, we perform an auto-dir on the aforementioned level
					int iNextNode = pMainNode->GetNodeCount();

					iItemCount += LoadMenuFromDir( pMainNode, sValue, dwFlags, bRecurse, bFlatten, true, false );
					if  ( iNextNode != pMainNode->GetNodeCount() )
					{
						CXMLNode * pNewNode = pMainNode->GetNode( iNextNode );
						if ( pNewNode )
						{
							int iIndex = 0;
							CXMLElement * pElement;
							while( pElement = pSubNode->GetElement( _T("sortfield"), iIndex++ ) )
							{
								pNewNode->AddElement( _T("sortfield"), pElement->GetBasicValue() );
							}
						}
					}
				}
				else
				{
					// Just copy the item info over.
					// Make sure we have: Title and Exe
//					CXMLNode * pItemNode;
//					pItemNode = pMainNode->AddNode( _T("item") );
					CStdString sTemp = pSubNode->GetString( NULL, _T("dir"), _T(""), true );
					
					if ( sTemp.GetLength() == 0 )
					{
						pSubNode->SetString( NULL, _T("dir"), sDir );
					}
					pMainNode->CopyInNode( (*pSubNode), _T("item") );
					DumpNode( pSubNode, 0 );
//					pItemNode->m_msAttributes = pSubNode->m_msAttributes;
//					pItemNode->m_msElements = pSubNode->m_msElements;
					iItemCount++;
				}
			}
		}
		else if ( sNodeName.Compare( _T("submenu") ) == 0 )
		{
			CXMLNode	nodeSubMenu;
			int			iNumItems;
		//	CXMLNode * pSubmenuNode;

			nodeSubMenu.m_sName = _T("submenu");
			nodeSubMenu.m_msAttributes = pSubNode->m_msAttributes;
			nodeSubMenu.m_msElements = pSubNode->m_msElements;

			pSubNode->SetString( NULL, _T("filedirectory"), sDir );

			iNumItems = LoadMenuFromNode( &nodeSubMenu, pSubNode );
			if ( iNumItems )
			{
				CStdString sTitle = nodeSubMenu.GetString( NULL, _T("title"), _T(""), true );
				if ( sTitle.GetLength() == 0 )
				{
					nodeSubMenu.SetString( NULL, _T("title"), _T("SubMenu") );
				}
				iItemCount += iNumItems;
				pMainNode->CopyInNode( nodeSubMenu, _T("submenu") );
			}
			
//			pSubmenuNode = pMainNode->AddNode( _T("submenu") );
//			pSubmenuNode->m_msAttributes = pSubNode->m_msAttributes;
//			pSubmenuNode->m_msElements = pSubNode->m_msElements;
//			if ( pSubmenuNode )
//			{
//				iItemCount += LoadMenuFromNode( pSubmenuNode, pSubNode );
//			}
		}
		pSubNode = pFileNode->GetNode( ++iNodeCount );
	}
	return iItemCount;
}




// Offsets for building matrices which are used to animate the gamepad controls.
//D3DXVECTOR3 g_vLeftTriggerAxis    = D3DXVECTOR3( 1.0f, -0.13f, 0.0f );
//D3DXVECTOR3 g_vLeftTriggerOffset  = D3DXVECTOR3( 0.0f, 12.0f, 38.0f );
//D3DXVECTOR3 g_vRightTriggerAxis   = D3DXVECTOR3( 1.0f, 0.13f, 0.0f );
//D3DXVECTOR3 g_vRightTriggerOffset = D3DXVECTOR3( 0.0f, 12.0f, 38.0f );
//D3DXVECTOR3 g_vDPadOffset         = D3DXVECTOR3( -36.70f, -26.27f, 12.0f );
//D3DXVECTOR3 g_vLeftStickOffset    = D3DXVECTOR3( -48.54f,   8.72f, 18.0f );
//D3DXVECTOR3 g_vRightStickOffset   = D3DXVECTOR3(  36.70f, -26.27f, 18.0f );


// Structures for animating, highlighting, and texturing the gamepad controls.
//BOOL               g_ControlActive[NUM_CONTROLS];
//D3DXMATRIX         g_ControlMatrix[NUM_CONTROLS];
//LPDIRECT3DTEXTURE8 g_ControlTexture[NUM_CONTROLS];


// Global world matrix, so the app can orient the gamepad
// D3DXMATRIX g_matWorld( 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 );

class CXBoxGameHelpState : public CXBoxGameState
{
	CXBXmlCfgAccess m_iniFile;
//	DWORD	m_dwLoadTimer;
	CImageSource	m_imgHelpScreen;
	int				m_iCurrentScreen;
	int				m_iLastScreen;
	BOOL			m_bGlobalHelp;
	int				m_iLastGameHelp;
	int				m_iNumScreens;
	list<CStdString> m_saHelp;
	CGadgetLayout *		m_pHelpLayout;
	CGadgetLayout *		m_pGameHelpLayout;
	bool				IsHelpLayoutValid( void );
	bool				IsGameHelpLayoutValid( void );
	virtual void OnNewSkin( void );


	virtual bool MusicIsSuspended( void );
	CDelayController	m_ctrStick;
	CDelayController	m_ctrDpad;
	CDelayController	m_ctrIR;
	CXBSound *			m_pSndMenuLaunch; // g_SndResCache
	void	PlaySndLaunch( void )
	{
		if ( m_pSndMenuLaunch == NULL )
		{
			m_pSndMenuLaunch = g_SndResCache.GetSound( _T("MenuLaunch") );
		}
		if ( m_pSndMenuLaunch  )
		{
			m_pSndMenuLaunch->Play();
		}
	};
public:
	CStdString			GetHelpScreen(int iIndex );
	CXBoxGameHelpState(CXBoxStateApplication * pbxApplication );
    virtual HRESULT Initialize();
    virtual HRESULT FrameMove();
    virtual HRESULT Render();
    virtual HRESULT Cleanup() { return S_OK; };
	HRESULT OnActivation( void );
	HRESULT OnDeactivation( void );
};


void CXBoxGameHelpState::OnNewSkin( void )
{
	m_pSndMenuLaunch = NULL;
	m_pHelpLayout = NULL;
	m_pGameHelpLayout = NULL;
}

bool CXBoxGameHelpState::MusicIsSuspended( void )
{ 
	// When we go to layouts here, we will query the 
	// layout for this determination
	return false; 
}

bool CXBoxGameHelpState::IsHelpLayoutValid( void )
{
	bool bReturn = false;
	if ( m_pHelpLayout )
	{
		bReturn = true;
	}
	else
	{
		m_pHelpLayout = g_GadgetSkin.GetLayout( _T("help") );
		if ( m_pHelpLayout )
		{
			bReturn = true;
		}
	}
	return bReturn;
}

bool CXBoxGameHelpState::IsGameHelpLayoutValid( void )
{
	bool bReturn = false;
	if ( m_pGameHelpLayout )
	{
		bReturn = true;
	}
	else
	{
		m_pGameHelpLayout = g_GadgetSkin.GetLayout( _T("gamehelp") );
		if ( m_pGameHelpLayout )
		{
			bReturn = true;
		}
	}
	return bReturn;
}

CXBoxGameHelpState::CXBoxGameHelpState(CXBoxStateApplication * pbxApplication ) : 
	CXBoxGameState ( pbxApplication ) ,
	m_pSndMenuLaunch(NULL),
	m_pGameHelpLayout(NULL),
	m_pHelpLayout(NULL),
	m_ctrStick(g_MenuInfo.m_dwMoveDelay, g_MenuInfo.m_dwRepeatDelay),
	m_ctrDpad(g_MenuInfo.m_dwMoveDelay, g_MenuInfo.m_dwRepeatDelay),
	m_ctrIR(g_MenuInfo.m_dwMoveDelay, g_MenuInfo.m_dwRepeatDelay)
{
	m_iLastScreen = -1;
}


CStdString CXBoxGameHelpState::GetHelpScreen(int iIndex )
{
	list <CStdString>::iterator iterStr;
	CStdString sReturn(_T(""));

	iterStr = m_saHelp.begin();

	while( iIndex-- )
	{
		iterStr++;
	}
	if ( iterStr != m_saHelp.end() )
	{
		sReturn = *iterStr;
	}
	return sReturn;
}
	
HRESULT CXBoxGameHelpState::Initialize() 
{ 
	m_imgHelpScreen.Initialize( m_pbxApplication->Get3dDevice(), m_pbxApplication->GetSoundDevice() );
	return S_OK; 
}

HRESULT CXBoxGameHelpState::FrameMove( void )
{
	WORD wButtons = this->m_pbxApplication->GetDefaultGamepad()->wButtons;
	WORD wRemotes = this->m_pbxApplication->m_DefaultIR_Remote.wPressedButtons;
	BYTE byVirtKey = m_pbxApplication->GetVirtualKey();

	WORD wDpad = wButtons&(XINPUT_GAMEPAD_DPAD_UP|XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_LEFT|XINPUT_GAMEPAD_DPAD_RIGHT);
	int iStickX,iStickY;

	m_pbxApplication->GetStickValues( iStickX, iStickY );


	CStdString sTemp;

	BOOL bUp = false;
	BOOL bDown = false;
	BOOL bRight = false;
	BOOL bLeft = false;

	WORD wDir = m_ctrDpad.DpadInput(wDpad);
	wDir |= m_ctrIR.IRInput(wRemotes);
	wDir |= m_ctrStick.StickInput(iStickX, iStickY);

	if ( m_iNumScreens )
	{
		if ( wDir & DC_LEFT )
		{
			m_iCurrentScreen--;
			if ( m_iCurrentScreen < 0 )
			{
				m_iCurrentScreen = m_iNumScreens-1;
			}
		}
		else if ( wDir & DC_RIGHT )
		{
			m_iCurrentScreen++;
			if ( m_iCurrentScreen >= m_iNumScreens )
			{
				m_iCurrentScreen = 0;
			}
		}
	}
	else
	{
		m_iCurrentScreen = 0;
	}

	if ( (wButtons & XINPUT_GAMEPAD_BACK )||(byVirtKey==VK_ESCAPE) || (wRemotes == XINPUT_IR_REMOTE_BACK) || m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_B])
	{
		m_pbxApplication->SetGameState( GS_MENU );
	}
	else if ( !m_bGlobalHelp &&
		(( wButtons & XINPUT_GAMEPAD_START ) || (m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_A]) || byVirtKey==VK_RETURN || wRemotes == XINPUT_IR_REMOTE_SELECT ))
	{
		PlaySndLaunch();
		// g_MenuInfo.m_skin.Sounds.m_sndMenuLaunch.Play();
//		ResetTimer();
		CItemInfo * pEntry;
		
		pEntry = g_MenuInfo.Entry( g_MenuInfo.m_iCurrentItem );
		if ( pEntry )
		{
			if ( pEntry->HasPassed() )
			{
				if ( pEntry->m_bIsMenu )
				{
					if ( pEntry->m_pMenuNode )
					{
						g_MenuInfo.LoadMenu( pEntry->m_pMenuNode, true );
					}
				}
				else if ( pEntry->m_bIsAction )
				{
					LaunchActionScript( pEntry->m_sAction );
				}
				else
				{
					m_pbxApplication->SetGameState( GS_LOAD );
				}
			}
			else
			{
				m_pbxApplication->SetGameState( GS_PASS, false, true );
			}
		}

		m_iniFile.Load( _T("T:\\MXM_State.xml") );
		m_iniFile.SetLong( _T("Selection"), _T("Current"), g_MenuInfo.m_iCurrentItem );
		m_iniFile.Save(_T("T:\\MXM_State.xml"));

		// Proceed to next state
	}
	if ( m_iLastScreen != m_iCurrentScreen )
	{
		m_pbxApplication->SetBlend();
	}
	
	return S_OK;
}

HRESULT CXBoxGameHelpState::OnDeactivation( void )
{ 	
	//if ( IsLayoutValid() )
	//{
	//	m_pLayout->OnDeactivation();
	//}
	return S_OK;
}
HRESULT CXBoxGameHelpState::OnActivation( void )
{ 	
	bool bLoadHelpScreens = false;
	bool bReleaseTexture = false;

	//if ( IsLayoutValid() )
	//{
	//	m_pLayout->OnActivation();
	//}

	// Proceed to next state	
	DEBUG_LINE( _T("Entering HELP state") );
	if ( m_iLastGameHelp == -1 )
	{
		if ( g_MenuInfo.m_bGlobalHelp )
		{
			// Cool, but let's check screen, too
			if ( m_iCurrentScreen )
			{
				bReleaseTexture = true;
			}
			m_iLastGameHelp = -1;
		}
		else
		{
			m_iLastGameHelp = g_MenuInfo.m_iCurrentItem;
			bReleaseTexture = true;
			bLoadHelpScreens = true;
		}
	}
	else
	{
		if ( g_MenuInfo.m_bGlobalHelp )
		{
			bReleaseTexture = true;
			bLoadHelpScreens = true;
			m_iLastGameHelp = -1;
		}
		else
		{
			if ( m_iLastGameHelp != g_MenuInfo.m_iCurrentItem )
			{
				bReleaseTexture = true;
				m_iLastGameHelp = g_MenuInfo.m_iCurrentItem;
				bLoadHelpScreens = true;
			}
			else
			{
				// Cool, but let's check screen, too
				if ( m_iCurrentScreen )
				{
					bReleaseTexture = true;
				}
			}
		}
	}
	if ( m_imgHelpScreen.GetTexture() && bReleaseTexture )
	{
		DEBUG_LINE( _T("Releasing Texture") );
		m_imgHelpScreen.ReleaseResources();
	}
	if ( bLoadHelpScreens )
	{
		m_iNumScreens = 0;
		m_saHelp.clear();
		if ( g_MenuInfo.m_bGlobalHelp )
		{
			if ( m_iNumScreens = g_MenuInfo.m_saMainHelp.size() )
			{
				DEBUG_LINE(_T("HELP: Using overrides"));
				m_saHelp = g_MenuInfo.m_saMainHelp;
			}
			else
			{
				m_iNumScreens = 3;

				m_saHelp.push_back( _T("HELPGC") );
				m_saHelp.push_back( _T("HELPIR") );
				m_saHelp.push_back( _T("HELPKB") );
			}
		}
		else
		{
			CItemInfo *pItem;
			int iHelpKey = 1;
			
			CStdString sKey, sValue;

			pItem = g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem);
			if ( pItem )
			{
				do
				{
					sKey.Format( _T("HelpScr_%02d"), iHelpKey++ );
					sValue = pItem->GetValue(sKey);
					if ( sValue.GetLength() && FileExists( sValue ) )
					{
						m_saHelp.push_back( sValue );
						m_iNumScreens++;
					}
					else
					{
						sValue = c_szEmpty;
					}
				} while( sValue.GetLength() );
			}
		}
		DEBUG_LINE( _T("Reloading help screens") );
	}
	else
	{
		m_imgHelpScreen.ReSync();
		DEBUG_LINE( _T("Not reloading help screens") );
	}
	m_iCurrentScreen = m_iLastScreen = 0;
	m_bGlobalHelp = g_MenuInfo.m_bGlobalHelp;

	return S_OK; 
}


HRESULT CXBoxGameHelpState::Render()
{
	HRESULT hr = S_OK;


	if ( m_iNumScreens )
	{
		if ( m_iLastScreen != m_iCurrentScreen )
		{
			m_imgHelpScreen.ReleaseResources();
			m_iLastScreen = m_iCurrentScreen;
		}

		if ( m_imgHelpScreen.GetTexture() == NULL )
		{
			CStdString sHelpScreen = GetHelpScreen(m_iCurrentScreen);
			if ( sHelpScreen )
			{
				if ( sHelpScreen.Compare( _T("HELPGC") ) == 0 )
				{
					m_imgHelpScreen.LoadImageMem( g_pbHelpGC, g_dwHelpGCLength, c_szHelpGCSection );
				}
				else if ( sHelpScreen.Compare( _T("HELPIR") ) == 0 )
				{
					m_imgHelpScreen.LoadImageMem( g_pbHelpIR, g_dwHelpIRLength, c_szHelpIRSection );
				}
				else if ( sHelpScreen.Compare( _T("HELPKB") ) == 0 )
				{
					m_imgHelpScreen.LoadImageMem( g_pbHelpKB, g_dwHelpKBLength, c_szHelpKBSection );
				}
				else
				{
					if ( FileExists( sHelpScreen ) )
					{
						m_imgHelpScreen.LoadImage( sHelpScreen );
					}
				}
			}
		}
		
		// Draw "Please Wait" Load Screen
		m_pbxApplication->RenderGradientBackground( 0xff0000ff, 0xff000000 );
		if ( m_imgHelpScreen.GetTexture() )
		{
			DrawSprite(m_pbxApplication->Get3dDevice(), m_imgHelpScreen.GetTexture(), 0, 0, 640, 480, FALSE );
		}
		else
		{
			CStdStringW sTemp;

			sTemp = L"Help not available\r\nInvalid image file";
			m_pbxApplication->m_Font16.DrawText( (FLOAT)(322), (FLOAT)(242), 0xff000000, sTemp, XBFONT_CENTER_X|XBFONT_CENTER_Y );
			m_pbxApplication->m_Font16.DrawText( (FLOAT)(320), (FLOAT)(240), 0xffffffff, sTemp, XBFONT_CENTER_X|XBFONT_CENTER_Y );
		}
	}
	else
	{
		CStdStringW sTemp;

		sTemp = L"Help not available";
		m_pbxApplication->RenderGradientBackground( 0xff0000ff, 0xff000000 );
		m_pbxApplication->m_Font16.DrawText( (FLOAT)(322), (FLOAT)(242), 0xff000000, sTemp, XBFONT_CENTER_X|XBFONT_CENTER_Y );
		m_pbxApplication->m_Font16.DrawText( (FLOAT)(320), (FLOAT)(240), 0xffffffff, sTemp, XBFONT_CENTER_X|XBFONT_CENTER_Y );

	}

	return hr;
}

class CXBoxGameLoadState : public CXBoxGameState
{
	DWORD	m_dwLoadTimer;
	CGadgetLayout *		m_pLayout;
	bool				IsLayoutValid( void );
	virtual bool MusicIsSuspended( void )   { return true; };

public:
	CXBoxGameLoadState(CXBoxStateApplication * pbxApplication ) : CXBoxGameState ( pbxApplication ), m_pLayout(NULL) {};
    virtual HRESULT Initialize() { return S_OK; };
    virtual HRESULT FrameMove() { return S_OK; };
    virtual HRESULT Render();
	virtual void OnNewSkin( void );

    virtual HRESULT Cleanup() { return S_OK; };
	void	SelectItem( void );
	HRESULT OnActivation( void );
	HRESULT OnDeactivation( void );
};


void CXBoxGameLoadState::OnNewSkin( void )
{
	m_pLayout = NULL;
}

bool CXBoxGameLoadState::IsLayoutValid( void )
{
	bool bReturn = false;
	if ( m_pLayout )
	{
		bReturn = true;
	}
	else
	{
		m_pLayout = g_GadgetSkin.GetLayout( _T("load") );
		if ( m_pLayout )
		{
			bReturn = true;
		}
	}
	return bReturn;
}

HRESULT CXBoxGameLoadState::OnDeactivation( void )
{ 	
	if ( IsLayoutValid() )
	{
		m_pLayout->OnDeactivation();
	}
	g_MenuInfo.m_bSpecialLaunch = false; // reset this
	return S_OK; 
}

HRESULT CXBoxGameLoadState::OnActivation( void )
{ 	
	if ( IsLayoutValid() )
	{
		m_pLayout->SetMusicEnable(false);
		m_pLayout->OnActivation();
		GetStateApp()->SetBlend( m_pLayout->IsFadeIn() );
	}
	// Proceed to next state	
// LAL 030502
//	if ( g_MenuInfo.m_bUseMusicManager )
//	{
//		if ( m_pbxApplication->m_pMusicManager )
//		{
//			m_pbxApplication->m_pMusicManager->Stop();
//		}
//	}
	DEBUG_LINE( _T("Entering LOAD state") );
//	m_pbxApplication->SetBlend(); 
	if ( g_MenuInfo.m_lLoadDelay >= 0 )
	{
		m_dwLoadTimer = GetTickCount()+g_MenuInfo.m_lLoadDelay;
	}
	else
	{
		m_dwLoadTimer = GetTickCount()+60000;
	}
	return S_OK; 
}

void CXBoxGameLoadState::SelectItem( void )
{
	CStdString sPath(g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->GetValue(szII_Exe));
	CStdString sXbe, sRealPath, sDevice;
	CStdString sMessage;
	CStdString sCommandLine(g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->GetValue(_T("CommandLine") ));
	DWORD dwStatus = 0;
	LAUNCH_DATA launchData;
	CStdString sPatchFile(g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->GetValue(_T("patchxbe")));

	DEBUG_LINE( _T("Launching application"));

	memset( &launchData, 0, sizeof(LAUNCH_DATA));

	if ( g_MenuInfo.m_bSpecialLaunch )
	{
		if ( g_MenuInfo.m_sSpecialLaunch.GetLength() )
		{
			if ( g_MenuInfo.m_bSpecialMXM )
			{
				if ( g_MenuInfo.m_sSpecialLaunchData.GetLength() )
				{
					memcpy( launchData.Data, g_MenuInfo.m_sSpecialLaunchData.c_str(), 
										g_MenuInfo.m_sSpecialLaunchData.GetLength()+1 );
				}
			}
			else
			{
				sPath = g_MenuInfo.m_sSpecialLaunch;				
			}
		}
		else
		{
			// If no length, we do a reboot.
			m_pbxApplication->Reboot();
		}
	}
	else
	{
		
		if ( sCommandLine.GetLength() )
		{
			memcpy( launchData.Data, sCommandLine.c_str(), 
								sCommandLine.GetLength()+1 );
		}
	}


	sXbe = ExtractFilenameFromPath( sPath.c_str() );
	sRealPath = StripFilenameFromPath( sPath.c_str() );
	sDevice = GetDeviceLocation( sPath.c_str() );

	// Should it fail, we'll get this message...
	sMessage.Format( _T("Launching: %s\nXBE: %s\nPath: %s\nDev: %s"), sPath.c_str(), sXbe.c_str(), sRealPath.c_str(), sDevice.c_str() );
	DEBUG_LINE( sMessage );

	if ( sPatchFile.GetLength() )
	{
		sPath = sPatchFile;
	}

	if ( FileExists( sPath ) )
	{
		F_COUNTRY ctyCode = COUNTRY_NULL;
		F_VIDEO vidCode = VIDEO_NULL;

		// Set region code...
		switch( g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->m_iCountryAction )
		{
			case 0: // Default Use main settings here...
				{
					switch( g_MenuInfo.m_pPrefInfo->GetLong(_T("local"), _T("region"), 0 ) )
					{
						case 0: // Default
							break;
						case 1: // Auto
							ctyCode = GetXboxCountryCode_CP(sPath);
							break;
						case 2: // USA
							ctyCode = COUNTRY_USA;
							break;
						case 3: // JAP
							ctyCode = COUNTRY_JAP;
							break;
						case 4: // EUR
							ctyCode = COUNTRY_EUR;
							break;
					}
				}
				break;
			case 1: // Auto
				ctyCode = GetXboxCountryCode_CP(sPath);
				break;
			case 2: // USA
				ctyCode = COUNTRY_USA;
				break;
			case 3: // JAP
				ctyCode = COUNTRY_JAP;
				break;
			case 4: // EUR
				ctyCode = COUNTRY_EUR;
				break;
		}

		if ( g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("tvtype"), 0 ) )
		{

			int iMode = g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->m_iVideoMode;
			switch( iMode )
			{
				case 0: // Default... Use main settings....
					{
						switch( g_MenuInfo.m_pPrefInfo->GetLong(_T("local"), _T("videomode"), 0 ) )
						{
							case 0: // Default
								break;
							case 1: // Auto
								vidCode = GetXboxVideoCode_CP(sPath);
								break;
							case 2: // NTSCM
								vidCode = VIDEO_NTSCM;
								break;
							case 3: // NTSCJ
								vidCode = VIDEO_NTSCJ;
								break;
							case 4: // PAL50
								vidCode = VIDEO_PAL50;
								break;
							case 5: // PAL60
								vidCode = VIDEO_PAL60;
								break;
						}
					}
					break;
				case 1: // Automatic
					vidCode = GetXboxVideoCode_CP(sPath);
					break;
				case 2: // NTSC-M
					vidCode = VIDEO_NTSCM;
					break;
				case 3: // NTSC-J
					vidCode = VIDEO_NTSCJ;
					break;
				case 4: // PAL50
					vidCode = VIDEO_PAL50;
					break;
				case 5: // PAL60
					vidCode = VIDEO_PAL60;
					break;
				//case 1: // NTSC Forced
				//	if ( !IsNTSC() )
				//	{
				//		SetNTSC(true);
				//	}
				//	break;
				//case -1: // PAL Forced
				//	if ( IsNTSC() )
				//	{
				//		SetNTSC(false);
				//	}
				//	break;
				//case 0: // Auto
				//	// Get video mode and set appropriately!
				//	if ( g_MenuInfo.m_pPrefInfo->GetBool( _T("local"), _T("videoautomode"), FALSE ) )
				//	{
				//		CXBEInfo * pInfo = NULL;
				//		pInfo = _GetXBEInfo(sPath);
				//		if ( pInfo )
				//		{
				//			if ( pInfo->m_dwRegions & (XBEIMAGE_GAME_REGION_NA|XBEIMAGE_GAME_REGION_JAPAN) )
				//			{
				//				if ( !IsNTSC() )
				//				{
				//					SetNTSC(true);
				//				}
				//			}
				//			else if ( pInfo->m_dwRegions & XBEIMAGE_GAME_REGION_RESTOFWORLD )
				//			{
				//				if ( IsNTSC() )
				//				{
				//					SetNTSC(false);
				//				}
				//			}
				//		}
				//	}
				//	break;
			}
			// Does item need video mode switch?
			//if ( g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->m_bNTSCMode != IsNTSC() )
			//{
			//	SetNTSC(g_MenuInfo.Entry(g_MenuInfo.m_iCurrentItem)->m_bNTSCMode);
			//}
		}

		if ( g_MenuInfo.m_pPrefInfo->GetBool( _T("local"), _T("force60pal"), FALSE ) )
		{
			if ( (vidCode == VIDEO_PAL50) || (vidCode == VIDEO_NULL) )
			{
				vidCode = VIDEO_PAL60;
			}
		}

		if ( ctyCode != COUNTRY_NULL || vidCode != VIDEO_NULL )
		{
			// We need to set our codes here...
			if ( ctyCode == COUNTRY_NULL )
			{
				// Need to default it to the Xbox's region...
				ctyCode = GetXboxCountryCode_CP();
			}
			if ( vidCode == VIDEO_NULL )
			{
				// Need to default it to the Xbox's region...
				vidCode = GetXboxVideoCode_CP();
				if ( g_MenuInfo.m_pPrefInfo->GetBool( _T("local"), _T("force60pal"), FALSE ) )
				{
					if ( vidCode == VIDEO_PAL50 )
					{
						vidCode = VIDEO_PAL60;
					}
				}
			}
			// OK, set the patch here.... if we are NOT running a debug BIOS
			// Ala TATX, VGA, or CPX....
			CStdString sBIOS;

			GetGadgetString( GSTR_BIOSNAME, sBIOS );
			sBIOS.MakeLower();
			// Only if BIOS is not a debug BIOS
			if ( sBIOS.Find( _T("debug") ) < 0 )
			{
				PatchCountryVideo( ctyCode, vidCode);
			}
		}

		// If patchfile exists....
		if ( sPatchFile.GetLength() )
		{
			dwStatus = LaunchTitlePatchFile(sDevice, sPatchFile );
		}
		else
		{
			if ( sCommandLine.GetLength() )
			{
				dwStatus = LaunchTitle( sDevice.c_str(), sXbe.c_str(), &launchData );
			}
			else
			{
				dwStatus = LaunchTitle( sDevice.c_str(), sXbe.c_str() );
			}
		}

		// Should it fail, we'll get this message...
		sMessage.Format( _T("Launch(0x%08lx): %s\nXBE: %s\nPath: %s\nDev: %s"), dwStatus, sPath.c_str(), sXbe.c_str(), sRealPath.c_str(), sDevice.c_str() );

		m_pbxApplication->MessageBox( sMessage.c_str() ); // _T("Selected\nThe game was SELECTED"));
	}
	else
	{
		sMessage.Format( _T("Program does not exist!\n(%s)\nXBE: %s\nPath: %s\nDev: %s"), sPath.c_str(), sXbe.c_str(), sRealPath.c_str(), sDevice.c_str() );
		m_pbxApplication->MessageBox( sMessage.c_str() ); // _T("Selected\nThe game was SELECTED"));
	}
	m_pbxApplication->SetGameState( GS_MENU );

}


HRESULT CXBoxGameLoadState::Render()
{
	HRESULT hr = S_OK;
	bool bDisplayDone = false;

#if 0
	// Draw "Please Wait" Load Screen
	m_pbxApplication->RenderGradientBackground( 0xff0000ff, 0xff000000 );
	if ( g_MenuInfo.m_skin.Textures.m_imgLoading.GetTexture() )
	{
		DrawSprite(m_pbxApplication->Get3dDevice(), g_MenuInfo.m_skin.Textures.m_imgLoading.GetTexture(), 0, 0, 640, 480, FALSE );
	}
#endif
	if ( IsLayoutValid() )
	{
		m_pLayout->Render();
	}

	if ( IsLayoutValid() )
	{
		if ( m_pLayout->IsSequenceEnded() )
		{
			bDisplayDone = true;
		}
	}

	if ( m_dwLoadTimer < GetTickCount() )
	{
		bDisplayDone = true;
	}

	if ( bDisplayDone )
	{
		m_pbxApplication->Get3dDevice()->Present( NULL, NULL, NULL, NULL );
		// Launch application
		m_pbxApplication->Get3dDevice()->PersistDisplay();
		SelectItem();
	}

	return hr;
}



class CXBoxGamePasscodeState : public CXBoxGameState
{
	int m_iCurrentBox;
	int m_iMaxCharWidth;
	int m_iMaxCharHeight;
	CStdStringW m_sChars;
	int		m_iEntered[8];
	RECT	m_rectCharBox[8];
	WCHAR	m_wchEntered[8];
	CGadgetLayout *		m_pLayout;
	bool				IsLayoutValid( void );
	CDelayController	m_ctrStick;
	CDelayController	m_ctrDpad;
	CDelayController	m_ctrIR;
	virtual bool MusicIsSuspended( void )   { return false; };
public:
	CXBoxGamePasscodeState(CXBoxStateApplication * pbxApplication );
    virtual HRESULT Initialize();
    virtual HRESULT FrameMove();
	virtual void OnNewSkin( void );
    virtual HRESULT Render();
    virtual HRESULT Cleanup() { return S_OK; };
	HRESULT OnActivation( void );
	HRESULT OnDeactivation( void );
	void ResetPasscodeEntry( void );
};

void CXBoxGamePasscodeState::OnNewSkin( void )
{
	m_pLayout = NULL;
}



bool CXBoxGamePasscodeState::IsLayoutValid( void )
{
	bool bReturn = false;
	if ( m_pLayout )
	{
		bReturn = true;
	}
	else
	{
		m_pLayout = g_GadgetSkin.GetLayout( _T("passcode") );
		if ( m_pLayout )
		{
			bReturn = true;
		}
	}
	return bReturn;
}


const WCHAR c_wszPassSet[] = L" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

CXBoxGamePasscodeState::CXBoxGamePasscodeState(CXBoxStateApplication * pbxApplication ) : 
	CXBoxGameState ( pbxApplication ) ,
	m_pLayout(NULL),
	m_ctrStick(g_MenuInfo.m_dwMoveDelay, g_MenuInfo.m_dwRepeatDelay),
	m_ctrDpad(g_MenuInfo.m_dwMoveDelay, g_MenuInfo.m_dwRepeatDelay),
	m_ctrIR(g_MenuInfo.m_dwMoveDelay, g_MenuInfo.m_dwRepeatDelay)
{
}

HRESULT CXBoxGamePasscodeState::OnDeactivation( void )
{ 
	if ( IsLayoutValid() )
	{
		m_pLayout->OnDeactivation();
	}
	return S_OK; 
}


HRESULT CXBoxGamePasscodeState::OnActivation( void )
{ 
	if ( IsLayoutValid() )
	{
		m_pLayout->OnActivation();
		GetStateApp()->SetBlend( m_pLayout->IsFadeIn() );
	}
	ResetPasscodeEntry(); 
	return S_OK; 
}

void CXBoxGamePasscodeState::ResetPasscodeEntry( void )
{
	int iIndex;

	m_iCurrentBox = 0;

	for( iIndex=0; iIndex<5; iIndex++ )
	{
		m_iEntered[iIndex] = 0;
	}
}

HRESULT CXBoxGamePasscodeState::Initialize() 
{
	int iIndex;
	FLOAT fWidth, fHeight;
	int iTop, iLeft;
	CStdString sTemp;
	WCHAR wszLetter[2];


	wszLetter[0] = L' ';
	wszLetter[1] = 0;
	m_iCurrentBox = 0;
	m_iMaxCharWidth = 0;
	m_iMaxCharHeight = 0;
	memset( m_wchEntered, 0, sizeof(WCHAR)*6 );

	m_sChars = c_wszPassSet;

	for( iIndex=0; iIndex<m_sChars.GetLength(); iIndex++ )
	{
		wszLetter[0] = m_sChars[iIndex];
		if ( !iswalnum(wszLetter[0]) )
		{
			wszLetter[0] = L' ';
		}
		m_pbxApplication->m_Font16.GetTextExtent( wszLetter, &fWidth, &fHeight, FALSE );
		if ( m_iMaxCharWidth < (int)(fWidth+0.5) )
		{
			m_iMaxCharWidth = (int)(fWidth+0.5);
		}
		if ( m_iMaxCharHeight < (int)(fHeight+0.5) )
		{
			m_iMaxCharHeight = (int)(fHeight+0.5);
		}
	}

	// Make it the box size
	m_iMaxCharHeight += 8;
	m_iMaxCharWidth += 8;
	

	sTemp.Format( _T("PASSCODE BOXES: (%d,%d)"), m_iMaxCharWidth, m_iMaxCharHeight );
	DEBUG_LINE( sTemp );
	// OK, formula for m_rectCharBox calculation....
	// 
	iTop = 240-(m_iMaxCharHeight/2);
	iLeft = (640-(m_iMaxCharWidth*7))/2;
	
	sTemp.Format( _T("  TOPLEFT: (%d,%d)"), iLeft, iTop );
	DEBUG_LINE( sTemp );

	for( iIndex=0; iIndex<5; iIndex++ )
	{
		m_iEntered[iIndex] = 0;

		m_rectCharBox[iIndex].left = iLeft;
		m_rectCharBox[iIndex].top = iTop;
		m_rectCharBox[iIndex].right = iLeft+m_iMaxCharWidth;
		m_rectCharBox[iIndex].bottom = iTop+m_iMaxCharHeight;

		iLeft += (m_iMaxCharWidth+(m_iMaxCharWidth/2));
		sTemp.Format( _T("  TOPLEFT: (%d,%d)"), iLeft, iTop );
		DEBUG_LINE( sTemp );
	}
	
	return S_OK; 
}

HRESULT CXBoxGamePasscodeState::FrameMove()
{
	int iIndex, iPos;
	CStdStringW sPassW;
	CStdString sPass;
	WORD wButtons = this->m_pbxApplication->GetDefaultGamepad()->wButtons;
	WORD wRemotes = this->m_pbxApplication->m_DefaultIR_Remote.wPressedButtons;
	BYTE byVirtKey = m_pbxApplication->GetVirtualKey();

	WORD wDpad = wButtons&(XINPUT_GAMEPAD_DPAD_UP|XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_LEFT|XINPUT_GAMEPAD_DPAD_RIGHT);
	int iStickX,iStickY;

	m_pbxApplication->GetStickValues( iStickX, iStickY );


	BOOL bUp = false;
	BOOL bDown = false;
	BOOL bRight = false;
	BOOL bLeft = false;

	WORD wDir = m_ctrDpad.DpadInput(wDpad);
	wDir |= m_ctrIR.IRInput(wRemotes);
	wDir |= m_ctrStick.StickInput(iStickX, iStickY);

	if ( byVirtKey == VK_UP )
	{
		bUp = true;
	} 
	else if ( byVirtKey == VK_DOWN )
	{
		bDown = true;
	}
	else if ( byVirtKey == VK_LEFT )
	{
		bLeft = true;
	}
	else if ( byVirtKey == VK_RIGHT )
	{
		bRight = true;
	}
	if ( wDir & DC_UP )
	{
		bUp = true;
	}
	else if (wDir & DC_DOWN )
	{
		bDown = true;
	}
	else if (wDir & DC_LEFT )
	{
		bLeft = true;
	}
	else if (wDir & DC_RIGHT )
	{
		bRight = true;
	}
	if ( bRight )
	{
		m_iCurrentBox++;
		if ( m_iCurrentBox > 4 )
		{
			m_iCurrentBox = 0;
		}
	}
	else if ( bLeft )
	{
		m_iCurrentBox--;
		if ( m_iCurrentBox < 0 )
		{
			m_iCurrentBox = 4;
		}
	}
	else if ( bUp )
	{
		m_iEntered[m_iCurrentBox]++;
		if ( m_iEntered[m_iCurrentBox] >= m_sChars.GetLength() )
		{
			m_iEntered[m_iCurrentBox] = 0;
		}
	}
	else if ( bDown )
	{
		m_iEntered[m_iCurrentBox]--;
		if ( m_iEntered[m_iCurrentBox] < 0  )
		{
			m_iEntered[m_iCurrentBox] = m_sChars.GetLength()-1;
		}
	}
	else if ( isalnum(byVirtKey) )
	{
		int iNewPos;

		if ( isdigit(byVirtKey) )
		{
			iNewPos = (int)(byVirtKey-'0');
			iNewPos += 1;
		}
		else
		{
			iNewPos = (int)(toupper(byVirtKey)-'A');
			iNewPos += 11;

		}
		m_iEntered[m_iCurrentBox] = iNewPos;
		m_iCurrentBox++;
		if (m_iCurrentBox>4)
		{
			m_iCurrentBox = 4;
		}
	}
	else if ( byVirtKey == VK_BACK )
	{
		m_iEntered[m_iCurrentBox] = 0;
		m_iCurrentBox--;
		if ( m_iCurrentBox < 0 )
		{
			m_iCurrentBox = 0;
		}
	}
	else if ( byVirtKey == VK_ESCAPE )
	{
		m_pbxApplication->SetGameState( GS_MENU );
	}

	if (( m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_A] )||( byVirtKey == VK_RETURN )||(wButtons & XINPUT_GAMEPAD_START))
	{
		iPos = 0;
		
		// Create passcode currently on screen
		for( iIndex=0; iIndex<5; iIndex++ )
		{
			if ( m_iEntered[iIndex] )
			{
				m_wchEntered[iPos++] = m_sChars[m_iEntered[iIndex]];
			}
			m_wchEntered[iPos] = 0;
		}
		sPassW = m_wchEntered;
		sPass = sPassW;
		DEBUG_LINE( _T("Passcode Entered") );
		DEBUG_LINE( sPass );
		CItemInfo * pEntry;
		
		pEntry = g_MenuInfo.Entry( g_MenuInfo.m_iCurrentItem );
		if ( pEntry )
		{
			pEntry->TryPasscode(sPass);
			if ( pEntry->HasPassed() )
			{
				if ( pEntry->m_bIsMenu  )
				{
					m_pbxApplication->SetGameState( GS_MENU, false );
					m_pbxApplication->PumpRender();
					m_pbxApplication->PumpRender();
					m_pbxApplication->PumpRender();
					if ( pEntry->m_pMenuNode )
					{
						g_MenuInfo.LoadMenu( pEntry->m_pMenuNode, true );
					}
				}
				else if ( pEntry->m_bIsAction )
				{
					m_pbxApplication->SetGameState( GS_MENU, false );
					m_pbxApplication->PumpRender();
					m_pbxApplication->PumpRender();
					m_pbxApplication->PumpRender();
					LaunchActionScript( pEntry->m_sAction );
				}
				else
				{
					m_pbxApplication->SetGameState( GS_MENU, false );
					m_pbxApplication->PumpRender();
					m_pbxApplication->PumpRender();
					m_pbxApplication->PumpRender();
					m_pbxApplication->SetGameState( GS_LOAD );
				}
			}
			else
			{
				m_pbxApplication->SetGameState( GS_MENU );
			}
		}
	}
	return S_OK;
}

HRESULT CXBoxGamePasscodeState::Render()
{
	int iIndex;
	DWORD dwBorderColor, dwBoxColor, dwTextColor;
	WCHAR wszLetter[4];

	if ( m_pbxApplication->m_imgPersistImage.GetTexture() )
	{
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pbxApplication->m_imgPersistImage.GetTexture(), 0, 0, 640, 480, FALSE );
	}

	m_pbxApplication->DrawBox( (FLOAT)0.0, (FLOAT)0.0, 
			(FLOAT)640.0, (FLOAT)480.0, 0x80ff0000, 0x80ff0000 );
	
	// m_pbxApplication->RenderGradientBackground( 0x800000ff, 0x80000000 );

	m_pbxApplication->m_Font16.DrawText( (FLOAT)320.0, (FLOAT)m_rectCharBox[0].top-(m_iMaxCharHeight*2), 0xffffffff, L"Enter Passcode", XBFONT_CENTER_Y|XBFONT_CENTER_X );

	wszLetter[0] = L' ';
	wszLetter[1] = 0;
	for( iIndex=0; iIndex<5; iIndex++ )
	{
		if ( iIndex == m_iCurrentBox )
		{
			dwTextColor = 0xff000000;
			dwBorderColor = 0xff000000;
			dwBoxColor = 0xffffff00;
		}
		else
		{
			dwTextColor = 0xff808080;
			dwBoxColor = 0xff000000;
			dwBorderColor = 0xffffff00;
		}
		wszLetter[0] = m_sChars[m_iEntered[iIndex]];
		if ( !iswalnum(wszLetter[0]) )
		{
			wszLetter[0] = L' ';
		}
		m_pbxApplication->DrawBox( (FLOAT)m_rectCharBox[iIndex].left, (FLOAT)m_rectCharBox[iIndex].top, 
			(FLOAT)m_rectCharBox[iIndex].right, (FLOAT)m_rectCharBox[iIndex].bottom, dwBoxColor, dwBorderColor );

		if ( wszLetter[0] != L' ' ) // m_iEntered[iIndex] )
		{
			m_pbxApplication->m_Font16.DrawText( (FLOAT)m_rectCharBox[iIndex].left+(m_iMaxCharWidth/2), (FLOAT)m_rectCharBox[iIndex].top+(m_iMaxCharHeight/2), dwTextColor, wszLetter, XBFONT_CENTER_Y|XBFONT_CENTER_X );
		}

//		m_rectCharBox[iIndex].right = iLeft+m_iMaxCharWidth;
//		m_rectCharBox[iIndex].top = iTop+m_iMaxCharHeight;
//		iLeft += (m_iMaxCharWidth+(m_iMaxCharWidth/2))
	}


//	m_pbxApplication->RenderQuadGradientBackground( m_ccTopLeft.GetColor(), m_ccTopRight.GetColor(), m_ccBottomLeft.GetColor(), m_ccBottomRight.GetColor() );
//	if ( g_MenuInfo.m_skin.Textures.m_imgSaverLogo.GetTexture() )
//	{
//
//		DrawSprite(m_pbxApplication->Get3dDevice(), g_MenuInfo.m_skin.Textures.m_imgSaverLogo.GetTexture(), m_iPosX, m_iPosY, m_iSizeX, m_iSizeY, TRUE, TRUE );
//	}
	return S_OK;
}

class CXBoxInfoState : public CXBoxGameState
{
	CGadgetLayout *		m_pLayout;
	bool				IsLayoutValid( void );
	virtual bool MusicIsSuspended( void );

public:
	CXBoxInfoState(CXBoxStateApplication * pbxApplication ) : CXBoxGameState ( pbxApplication ), m_pLayout(NULL) 	{};
    virtual HRESULT Initialize();
    virtual HRESULT FrameMove();
	virtual void OnNewSkin( void );
    virtual HRESULT Render();
    virtual HRESULT Cleanup();
	HRESULT OnActivation( void );
	HRESULT OnDeactivation( void );
};

void CXBoxInfoState::OnNewSkin( void )
{
	m_pLayout = NULL;
}



bool CXBoxInfoState::MusicIsSuspended( void )
{ 
	bool bReturn = false;
	return bReturn; 
}

bool CXBoxInfoState::IsLayoutValid( void )
{
	bool bReturn = false;
	if ( m_pLayout )
	{
		bReturn = true;
	}
	else
	{
		m_pLayout = g_GadgetSkin.GetLayout( _T("info01") );
		if ( m_pLayout )
		{
			bReturn = true;
		}
	}
	return bReturn;
}

HRESULT CXBoxInfoState::OnDeactivation( void )	
{ 
	if ( IsLayoutValid() )
	{
		m_pLayout->OnDeactivation();
	}
	return S_OK; 
};

HRESULT CXBoxInfoState::OnActivation( void )	
{ 
	if ( IsLayoutValid() )
	{
		m_pLayout->OnActivation();
		GetStateApp()->SetBlend( m_pLayout->IsFadeIn() );
	}
	return S_OK; 
};

HRESULT CXBoxInfoState::Initialize()
{
	return S_OK;
}

HRESULT CXBoxInfoState::FrameMove()
{
	WORD wButtons = this->m_pbxApplication->GetDefaultGamepad()->wButtons;
	WORD wRemotes = this->m_pbxApplication->m_DefaultIR_Remote.wPressedButtons;
	char byVirtKey = m_pbxApplication->GetKeyChar();
	
	int iStickX,iStickY;
	FLOAT fX, fY;

	fX = m_pbxApplication->GetDefaultGamepad()->fX1+m_pbxApplication->GetDefaultGamepad()->fX2;
	fY = m_pbxApplication->GetDefaultGamepad()->fY1+m_pbxApplication->GetDefaultGamepad()->fY2;

	iStickX=0; iStickY=0;
	if ( fX < 0.0 )
	{
		iStickX = (int)(fX-0.5);
	}
	if ( fX > 0.0 )
	{
		iStickX = (int)(fX+0.5);
	}
	if ( fY < 0.0 )
	{
		iStickY = (int)(fY-0.5);
	}
	if ( fY > 0.0 )
	{
		iStickY = (int)(fY+0.5);
	}
	// If moved, switch screens...

	// If BACK, return to menu
	if ( wButtons  || m_pbxApplication->AnalogButtonsPressed() || byVirtKey || wRemotes || iStickX || iStickY )
	{
		// Proceed to next state
		m_pbxApplication->SetGameState( GS_MENU );
	}
	return S_OK;
}

HRESULT CXBoxInfoState::Render()
{
	if ( IsLayoutValid() )
	{
		m_pLayout->Render();
	}

	return S_OK;
}

HRESULT CXBoxInfoState::Cleanup()
{
	return S_OK;
}


class CXBoxSaverState : public CXBoxGameState
{
	CGadgetLayout *		m_pLayout;
	bool				IsLayoutValid( void );
	virtual bool MusicIsSuspended( void );

public:
	CXBoxSaverState(CXBoxStateApplication * pbxApplication ) : CXBoxGameState ( pbxApplication ), m_pLayout(NULL) 	{};
    virtual HRESULT Initialize();
    virtual HRESULT FrameMove();
	virtual void OnNewSkin( void );
    virtual HRESULT Render();
    virtual HRESULT Cleanup();
	HRESULT OnActivation( void );
	HRESULT OnDeactivation( void );
};

void CXBoxSaverState::OnNewSkin( void )
{
	m_pLayout = NULL;
}



bool CXBoxSaverState::MusicIsSuspended( void )
{ 
	bool bReturn = false;
	if ( !g_MenuInfo.m_bAllowSaverMusic )
	{
		bReturn = true;
	}
	else
	{
		if ( m_pLayout )
		{
			if ( !m_pLayout->AllowMusic() )
			{
				bReturn = true;
			}
		}
	}
	return bReturn; 
}

bool CXBoxSaverState::IsLayoutValid( void )
{
	bool bReturn = false;
	if ( m_pLayout )
	{
		bReturn = true;
	}
	else
	{
		m_pLayout = g_GadgetSkin.GetLayout( _T("saver") );
		if ( m_pLayout )
		{
			bReturn = true;
		}
	}
	return bReturn;
}

HRESULT CXBoxSaverState::OnDeactivation( void )	
{ 
	if ( IsLayoutValid() )
	{
		m_pLayout->OnDeactivation();
	}
	return S_OK; 
};

HRESULT CXBoxSaverState::OnActivation( void )	
{ 
	if ( IsLayoutValid() )
	{
		m_pLayout->OnActivation();
		GetStateApp()->SetBlend( m_pLayout->IsFadeIn() );
	}
//	g_MenuInfo.m_skin.Textures.m_imgSaverLogo.ReSync();
//	if ( g_MenuInfo.m_bUseMusicManager )
//	{
//		if ( m_pbxApplication->m_pMusicManager && !g_MenuInfo.m_bAllowSaverMusic )
//		{
//			m_pbxApplication->m_pMusicManager->Pause();
//		}
//	}
//	m_dwTimeStamp = GetTickCount()+10; 
//	m_pbxApplication->SetBlend();
	return S_OK; 
};

HRESULT CXBoxSaverState::Initialize()
{
	//m_iPosX = 0;
	//m_iPosY = 0;
	//m_iDirX = 1;
	//m_iDirY = 1;
	//m_iSizeX = 128;
	//m_iSizeY = 96;

	// Initialize random seed
	srand(::GetTickCount());

	//m_dwTopColor = 0xffff0000;
	//m_dwBottomColor = 0xff000000;

	//m_iRed = 255;
	//m_iGreen = 0;
	//m_iBlue = 0;
	//m_iRedDir = -1;
	//m_iGreenDir = 0;
	//m_iBlueDir = 0;

//	m_pLayout->SetMusicEnable(!g_MenuInfo.m_bAllowSaverMusic);

//	m_pTitleTexture = NULL;

	//if ( g_MenuInfo.m_skin.Textures.m_imgSaverLogo.GetTexture() )
	//{
	//	m_iSizeX = g_MenuInfo.m_skin.Textures.m_imgSaverLogo.GetWidth();
	//	m_iSizeY = g_MenuInfo.m_skin.Textures.m_imgSaverLogo.GetHeight();
	//	if ( m_iSizeX > 160 )
	//	{
	//		// Make it 160x whatever....
	//		m_iSizeY *= 160;
	//		m_iSizeY /= m_iSizeX;
	//		m_iSizeX = 160;
	//	}
	//	if ( m_iSizeY > 120 )
	//	{
	//		// Make it 160x whatever....
	//		m_iSizeX *= 120;
	//		m_iSizeX /= m_iSizeY;
	//		m_iSizeY = 120;
	//	}

	//}	
	return S_OK;
}

HRESULT CXBoxSaverState::FrameMove()
{
	WORD wButtons = this->m_pbxApplication->GetDefaultGamepad()->wButtons;
	WORD wRemotes = this->m_pbxApplication->m_DefaultIR_Remote.wPressedButtons;
	char byVirtKey = m_pbxApplication->GetKeyChar();
	
	int iStickX,iStickY;
	FLOAT fX, fY;

	fX = m_pbxApplication->GetDefaultGamepad()->fX1+m_pbxApplication->GetDefaultGamepad()->fX2;
	fY = m_pbxApplication->GetDefaultGamepad()->fY1+m_pbxApplication->GetDefaultGamepad()->fY2;

	iStickX=0; iStickY=0;
	if ( fX < 0.0 )
	{
		iStickX = (int)(fX-0.5);
	}
	if ( fX > 0.0 )
	{
		iStickX = (int)(fX+0.5);
	}
	if ( fY < 0.0 )
	{
		iStickY = (int)(fY-0.5);
	}
	if ( fY > 0.0 )
	{
		iStickY = (int)(fY+0.5);
	}
	if ( wButtons  || m_pbxApplication->AnalogButtonsPressed(-2) || byVirtKey || wRemotes || iStickX || iStickY )
	{
		// Proceed to next state
//		if ( g_MenuInfo.m_bUseMusicManager )
//		{
//			if ( m_pbxApplication->m_pMusicManager && !g_MenuInfo.m_bAllowSaverMusic )
//			{
//				if ( g_MenuInfo.m_bRandomMusic )
//				{
//					m_pbxApplication->m_pMusicManager->RandomSong(g_MenuInfo.m_bGlobalMusic);
//				}
//				m_pbxApplication->m_pMusicManager->Play();
//			}
//		}
		// Don't skip town until capture is done....
		if ( m_pbxApplication->GetScreenCaptureState() == false )
		{
			m_pbxApplication->SetGameState( GS_MENU );
		}
	}
	return S_OK;
}

HRESULT CXBoxSaverState::Render()
{
	if ( IsLayoutValid() )
	{
		m_pLayout->Render();
	}

	//else
	//{
	//	this->m_pbxApplication->RenderQuadGradientBackground( m_ccTopLeft.GetColor(), m_ccTopRight.GetColor(), m_ccBottomLeft.GetColor(), m_ccBottomRight.GetColor() );
	//	if ( g_MenuInfo.m_iSaverOption == 0 )
	//	{
	//		m_matrix.Render( m_pbxApplication->Get3dDevice(), false );
	//	}
	//	if ( g_MenuInfo.m_skin.Textures.m_imgSaverLogo.GetTexture() )
	//	{

	//		DrawSprite(m_pbxApplication->Get3dDevice(), g_MenuInfo.m_skin.Textures.m_imgSaverLogo.GetTexture(), m_iPosX, m_iPosY, m_iSizeX, m_iSizeY, TRUE, TRUE );
	//	}
	//}
	return S_OK;
}

HRESULT CXBoxSaverState::Cleanup()
{
	return S_OK;
}



class CXBoxMenuState : public CXBoxGameState
{
	CXBXmlCfgAccess m_iniFile;

	int		m_iLastSelection;
	DWORD m_dwTimeStamp;
	bool	m_bLaunchTitle;
	bool	m_bAcceptInput;

	LPDIRECT3DTEXTURE8	m_pMenuTexture1;
	LPDIRECT3DTEXTURE8	m_pMenuTexture2;
	DWORD		m_dwMenuSizeX;
	DWORD		m_dwMenuSizeY;
	DWORD		m_dwMenuItemSizeY;
	D3DFORMAT	m_Format;
	int					m_iScrollOffset;

	LPDIRECT3DTEXTURE8	m_pDescrTexture;
	DWORD				m_dwDescrSizeX;
	DWORD				m_dwDescrSizeY;
	DWORD				m_dwDescrItemSizeY;

	CXBSound *			m_pSndMenuLaunch; // g_SndResCache
	CXBSound *			m_pSndMenuUp; // g_SndResCache
	CXBSound *			m_pSndMenuDn; // g_SndResCache
	void	PlaySndLaunch( void )
	{
		if ( m_pSndMenuLaunch == NULL )
		{
			m_pSndMenuLaunch = g_SndResCache.GetSound( _T("MenuLaunch") );
		}
		if ( m_pSndMenuLaunch  )
		{
			m_pSndMenuLaunch->Play();
		}
	};
	void	PlaySndUp( void )
	{
		if ( m_pSndMenuUp == NULL )
		{
			m_pSndMenuUp = g_SndResCache.GetSound( _T("MenuUp") );
		}
		if ( m_pSndMenuUp  )
		{
			m_pSndMenuUp->Play();
		}
	};
	void	PlaySndDown( void )
	{
		if ( m_pSndMenuDn == NULL )
		{
			m_pSndMenuDn = g_SndResCache.GetSound( _T("MenuDown") );
		}
		if ( m_pSndMenuDn  )
		{
			m_pSndMenuDn->Play();
		}
	};

	CDelayController	m_ctrStick;
	CDelayController	m_ctrDpad;
	CDelayController	m_ctrIR;

	int					m_iLastStickY;
	DWORD				m_dwStickTimer;
	int					m_iStickCount;
	CGadgetLayout *		m_pLayout;
	bool				IsLayoutValid( void );

	WORD				m_wLastDpad;
	DWORD				m_dwDpadTimer;
	int					m_iDpadCount;

	bool				m_bReSync;

	bool		m_bDisplayWait;

	BOOL  m_nHoldControllerDown1;

	HRESULT RenderSimpleTheme( void );

	virtual bool MusicIsSuspended( void );

	int			m_iCurrentItemShot;
	DWORD		m_dwShotTimer;
	int			m_iFlipFlop;
	void		PrepareResources( void );
	bool		m_bOneShot;

public:
	CXBoxMenuState(CXBoxStateApplication * pbxApplication );
    virtual HRESULT Initialize();
    virtual HRESULT FrameMove();
	virtual void OnNewSkin( void );
    virtual HRESULT Render();
    virtual HRESULT Cleanup();
	void	SelectItem( void );
	void	ResetTimer( void ) { m_dwTimeStamp = GetTickCount()+g_MenuInfo.m_lSaverDelay; };
	void	ResetShotTimer( void ) { m_dwShotTimer = GetTickCount()+550; };
	void	SyncShotTimer( void ) { m_dwShotTimer = GetTickCount()-5; };
	HRESULT OnActivation( void );
	HRESULT OnDeactivation( void );
};

void CXBoxMenuState::OnNewSkin( void )
{
	m_pSndMenuLaunch = NULL; // g_SndResCache
	m_pSndMenuUp = NULL; // g_SndResCache
	m_pSndMenuDn = NULL; // g_SndResCache
	m_pLayout = NULL;
}



HRESULT CXBoxMenuState::OnDeactivation( void )
{
	if ( IsLayoutValid() )
	{
		m_pLayout->OnDeactivation();
	}
	return S_OK;
}


bool CXBoxMenuState::MusicIsSuspended( void )
{
	bool bReturn = false;

	if ( m_pLayout )
	{
		if ( !m_pLayout->AllowMusic() )
		{
			bReturn = true;
		}
	}
	return bReturn;
}

CXBoxMenuState::CXBoxMenuState(CXBoxStateApplication * pbxApplication ) : 
	CXBoxGameState ( pbxApplication ),
	m_pLayout(NULL),
	m_bOneShot(false),
	m_bLaunchTitle(false),
	m_bDisplayWait(false),
	m_pSndMenuUp(NULL),
	m_pSndMenuDn(NULL),
	m_pSndMenuLaunch(NULL),
	m_pMenuTexture1(NULL),
	m_pMenuTexture2(NULL),
	m_pDescrTexture(NULL),
	m_dwDescrItemSizeY(0),
	m_wLastDpad(0),
	m_dwDpadTimer(0),
	m_iDpadCount(0),
	m_iLastStickY(0),
	m_dwDescrSizeY(0),
	m_dwDescrSizeX(0),
	m_dwMenuSizeX(0),
	m_dwMenuSizeY(0),
	m_iScrollOffset(0),
	m_iCurrentItemShot(-1),
	m_dwMenuItemSizeY(0),
	m_iFlipFlop(0),
	m_nHoldControllerDown1(0),
	m_ctrStick(g_MenuInfo.m_dwMoveDelay, g_MenuInfo.m_dwRepeatDelay),
	m_ctrDpad(g_MenuInfo.m_dwMoveDelay, g_MenuInfo.m_dwRepeatDelay),
	m_ctrIR(g_MenuInfo.m_dwMoveDelay, g_MenuInfo.m_dwRepeatDelay)
{
	m_bReSync = false;
	m_dwShotTimer = GetTickCount()-1;
}


void CXBoxMenuState::PrepareResources( void )
{
}

HRESULT CXBoxMenuState::OnActivation( void )	
{  
	DEBUG_FORMAT( _T("Activate MenuState 0x%08x"), this );

	m_dwShotTimer = GetTickCount()-1;
	m_bReSync = true;
	m_bAcceptInput = false;
//	CStdString sTemp, sSTName, sSngName;
//	WCHAR szSoundTrack[256];
//	WCHAR szSong[256];

//	m_pbxApplication->m_pMusicManager->GetCurrentInfo(szSoundTrack,szSong, NULL);
//	sSTName = szSoundTrack;
//	sSngName = szSong;

//	sTemp.Format( _T("Music Info: Global=%s Random=%s ST#=%ul (%s) (%s)"),
//		m_pbxApplication->m_pMusicManager->GetGlobal()?_T("ON"):_T("OFF"),
//		m_pbxApplication->m_pMusicManager->GetRandom()?_T("ON"):_T("OFF"),
//		m_pbxApplication->m_pMusicManager->GetSoundtrack(),
//		sSTName.c_str(), sSngName.c_str() );

//	DEBUG_LINE( sTemp.c_str() );


	PrepareResources();

	if ( IsLayoutValid() )
	{
		m_pLayout->OnActivation();
		GetStateApp()->SetBlend( m_pLayout->IsFadeIn() );
		DEBUG_FORMAT( _T("Layout Activated") );
	}
	
//	m_pbxApplication->SetBlend();

	ResetTimer();

	DEBUG_FORMAT( _T("Timer Reset") );

	m_bOneShot = true;

	return S_OK; 
}

void	CXBoxMenuState::SelectItem( void )
{
}

HRESULT CXBoxMenuState::Initialize()
{

	DEBUG_LINE( _T("Initialize MenuState") );

// LAL 030502
//	if ( g_MenuInfo.m_bUseMusicManager )
//	{
//		if ( m_pbxApplication->m_pMusicManager )
//		{
//			m_pbxApplication->m_pMusicManager->SetRandom(g_MenuInfo.m_bRandomMusic);
//			m_pbxApplication->m_pMusicManager->SetGlobal(g_MenuInfo.m_bGlobalMusic);
//
//			m_pbxApplication->m_pMusicManager->Pause();
//			m_pbxApplication->m_pMusicManager->SelectSoundtrack(0);
//			m_pbxApplication->m_pMusicManager->SelectSong(0);
//			m_pbxApplication->m_pMusicManager->SetVolume(DSBVOLUME_MAX);
//			if ( g_MenuInfo.m_bRandomMusic )
//			{
//				m_pbxApplication->m_pMusicManager->RandomSong( m_pbxApplication->m_pMusicManager->GetGlobal() );
//			}
//		}
//	}


	m_iniFile.Load( _T("T:\\MXM_State.xml") );
	g_MenuInfo.m_iCurrentItem = 0; // m_iniFile.GetLong( _T("Selection"), _T("Current"), 0 );
	DEBUG_FORMAT( _T("Getting MXMState Current Item: %d"), g_MenuInfo.m_iCurrentItem);

	if ( g_MenuInfo.m_iCurrentItem >= (int)g_MenuInfo.GetItemCount() )
	{
		g_MenuInfo.m_iCurrentItem = g_MenuInfo.GetItemCount()-1;
	}
	m_iLastSelection = g_MenuInfo.m_iCurrentItem;

	return S_OK;
}




HRESULT CXBoxMenuState::FrameMove()
{
	WORD wButtons = this->m_pbxApplication->GetDefaultGamepad()->wButtons;
	WORD wRemotes = this->m_pbxApplication->m_DefaultIR_Remote.wPressedButtons;
	BYTE byVirtKey = m_pbxApplication->GetVirtualKey();

	WORD wDpad = wButtons&(XINPUT_GAMEPAD_DPAD_UP|XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_LEFT|XINPUT_GAMEPAD_DPAD_RIGHT);
	int iStickX,iStickY;

	if ( m_bOneShot )
	{
		DEBUG_LINE( _T("Entering MenuState::FrameMove") );
	}

	m_pbxApplication->GetStickValues( iStickX, iStickY );
	if ( m_bAcceptInput )
	{
		// Removed, as per ITM's speedup
		//	WORD wButtons = m_pbxApplication->GetDefaultGamepad()->wPressedButtons;

		if ( g_MenuInfo.m_lSaverDelay && m_dwTimeStamp && m_dwTimeStamp < GetTickCount() )
		{
			// Proceed to next state
			m_pbxApplication->SetGameState( GS_SAVER );
		}
		if (1)
		{
			if ( ((m_pbxApplication->GetDefaultGamepad()->wButtons & ( XINPUT_GAMEPAD_LEFT_THUMB|XINPUT_GAMEPAD_RIGHT_THUMB)) == ( XINPUT_GAMEPAD_LEFT_THUMB|XINPUT_GAMEPAD_RIGHT_THUMB )) || (byVirtKey == VK_PRINT) || (wRemotes == XINPUT_IR_REMOTE_TITLE) )
			{
				DNS_SaveCache(true);
				DEBUG_LINE("Leaving to dashboard");
				DEBUG_LINE("====================================================");
				Sleep(50);

				g_MenuInfo.m_bSpecialLaunch = true;
				g_MenuInfo.m_sSpecialLaunch = g_MenuInfo.m_sExitApp;
				g_MenuInfo.m_sSpecialLaunchData = g_MenuInfo.m_sExitAppParams;
#if 1
				m_pbxApplication->SetGameState( GS_LOAD );
#else
				m_bLaunchTitle = true;
				m_bDisplayWait = true;
#endif

//				m_pbxApplication->Reboot();
			}
			else if ( (m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_WHITE]) || (byVirtKey==VK_F1) || (wRemotes == XINPUT_IR_REMOTE_MENU) )
			{
				DNS_SaveCache(true);
				g_MenuInfo.m_bGlobalHelp = true;
				m_pbxApplication->SetGameState( GS_HELP );
			}
			else if ( (m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_Y]) || (byVirtKey==VK_TAB) || (wRemotes == XINPUT_IR_REMOTE_DISPLAY) )
			{
				DNS_SaveCache(true);
				if ( !g_MenuInfo.m_bHideSystemMenu )
				{
					if ( g_MenuInfo.m_bDVDMode )
					{
						m_pbxApplication->EnterMenu( &g_MenuHandler, g_MainAppMenuDVD, 0 );
					}
					else
					{
						m_pbxApplication->EnterMenu( &g_MenuHandler, g_MainAppMenu, 0 );
					}
				}
/*
#ifdef XDEBUG
					GetStateApp()->MessageBox( _T("Media X Menu\r\n Version " VERSION_STRING " (NDEBUG)\r\n" __DATE__ " " __TIME__) );
#else
					GetStateApp()->MessageBox( _T("Media X Menu\r\n Version " VERSION_STRING "\r\n" __DATE__ " " __TIME__) );
#endif
*/
			}
			else if ( m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER] ) // (wButtons & XINPUT_GAMEPAD_BACK )||(byVirtKey==VK_ESCAPE) || (wRemotes == XINPUT_IR_REMOTE_BACK)||m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_B])
			{
			}
			else if ( m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] ) // (wButtons & XINPUT_GAMEPAD_BACK )||(byVirtKey==VK_ESCAPE) || (wRemotes == XINPUT_IR_REMOTE_BACK)||m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_B])
			{
			}
			else if ( m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_X] ) // (wButtons & XINPUT_GAMEPAD_BACK )||(byVirtKey==VK_ESCAPE) || (wRemotes == XINPUT_IR_REMOTE_BACK)||m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_B])
			{
				DNS_SaveCache(true);
				GetStateApp()->MusicNextSong();
			}
			else if ( (wButtons & XINPUT_GAMEPAD_BACK )||(byVirtKey==VK_ESCAPE) || (wRemotes == XINPUT_IR_REMOTE_BACK)||m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_B])
			{
				DNS_SaveCache(true);
				PlaySndLaunch();
				// Load previous menu (go up a level)
				if (	g_MenuInfo.m_pMenuCurrNode && 
						g_MenuInfo.m_pMenuNode && 
						(g_MenuInfo.m_pMenuCurrNode != g_MenuInfo.m_pMenuNode) &&
						g_MenuInfo.m_pMenuCurrNode->GetParent() )
				{
					g_MenuInfo.LoadMenu( g_MenuInfo.m_pMenuCurrNode->GetParent(), true );
					m_bAcceptInput = false;
				}
//				else
//				{
//					// Can't do anything? Just bring up the system menu...
//					m_pbxApplication->EnterMenu( &g_MenuHandler, g_MainAppMenu, 0 );
//					m_bAcceptInput = false;
//				}

			}
			else if ( (m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_BLACK])||(byVirtKey==VK_F2) || (wRemotes == XINPUT_IR_REMOTE_INFO))
			{
				DNS_SaveCache(true);
				g_MenuInfo.m_bGlobalHelp = false;
				m_pbxApplication->SetGameState( GS_HELP );
			}
			else if ( ( wButtons & XINPUT_GAMEPAD_START ) || (m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_A]) || byVirtKey==VK_RETURN || wRemotes == XINPUT_IR_REMOTE_SELECT )
			{
				DNS_SaveCache(true);
				PlaySndLaunch();
				// g_MenuInfo.m_skin.Sounds.m_sndMenuLaunch.Play();
				ResetTimer();

//				m_iniFile.Load( _T("T:\\MXM_State.xml") );
				m_iniFile.SetLong( _T("Selection"), _T("Current"), g_MenuInfo.m_iCurrentItem );
				m_iniFile.Save(_T("T:\\MXM_State.xml"));
				DEBUG_FORMAT( _T("Setting MXMState current item to: %d"), g_MenuInfo.m_iCurrentItem );

				CItemInfo * pEntry;
				
				pEntry = g_MenuInfo.Entry( g_MenuInfo.m_iCurrentItem );
				if ( pEntry )
				{
					if ( pEntry->HasPassed() )
					{
						if ( pEntry->m_bIsMenu  )
						{
							if ( pEntry->m_pMenuNode )
							{
								g_MenuInfo.LoadMenu( pEntry->m_pMenuNode, true );
							}
						}
						else if ( pEntry->m_bIsAction )
						{
							LaunchActionScript( pEntry->m_sAction );
						}
						else
						{
							CStdString sAction, sResult;

							sAction = pEntry->GetValue( _T("action") );
							if ( sAction.GetLength() )
							{
								HRESULT hr;
								sResult = m_pbxApplication->PerformActions( sAction, hr );
								sResult.Trim();
								if ( sResult.GetLength() )
								{
									m_pbxApplication->MessageBox( sResult );
								}
							}
							else
							{
								m_pbxApplication->SetGameState( GS_LOAD );
							}
						}
					}
					else
					{
						m_pbxApplication->SetGameState( GS_PASS );
					}
				}

				// Proceed to next state
			}
			else
			{



				BOOL bUp = false;
				BOOL bDown = false;

				WORD wDir = m_ctrDpad.DpadInput(wDpad);
				wDir |= m_ctrIR.IRInput(wRemotes);
				wDir |= m_ctrStick.StickInput(iStickX, iStickY);
				if ( g_MenuInfo.m_bMenuOrientedUpDown )
				{
					if ( wDir & DC_UP )
					{
						bUp = true;
					}
					if ( wDir & DC_DOWN )
					{
						bDown = true;
					}
				}
				else
				{
					if ( wDir & DC_LEFT )
					{
						bUp = true;
					}
					if ( wDir & DC_RIGHT )
					{
						bDown = true;
					}
				}
				if ( wDir == 0 )
				{
					// Synch screenshot NOW, if possible
					SyncShotTimer();
				}
				if ( byVirtKey==VK_UP )
				{
					bUp = true;
				}
				if ( byVirtKey==VK_DOWN )
				{
					bDown = true;
				}
				if ( wRemotes == XINPUT_IR_REMOTE_UP )
				{
					bUp = true;
				}
				if ( wRemotes == XINPUT_IR_REMOTE_DOWN )
				{
					bDown = true;
				}

				if (bUp)
				{
					ResetTimer();

					if ( g_MenuInfo.m_iCurrentItem > 0 )
					{
						g_MenuInfo.m_iCurrentItem--;
						m_iScrollOffset = 0-m_dwMenuItemSizeY;
						PlaySndUp();
						// g_MenuInfo.m_skin.Sounds.m_sndMenuUp.Play();
						m_nHoldControllerDown1=0;
						ResetShotTimer();
					}
					else
					{
						g_MenuInfo.m_iCurrentItem = 0;
						m_nHoldControllerDown1=0;
					}
				}

				if (bDown)
				{
					ResetTimer();

					if ( g_MenuInfo.m_iCurrentItem < (int)(g_MenuInfo.GetItemCount()-1) )
					{
						PlaySndDown();
						// g_MenuInfo.m_skin.Sounds.m_sndMenuDown.Play();
						g_MenuInfo.m_iCurrentItem++;
						m_iScrollOffset = m_dwMenuItemSizeY;
						m_nHoldControllerDown1=0;
						ResetShotTimer();
					}
					else
					{
// m_bCircularMenus is not implimented, because there are deeper issues.
// For one thing, our smoothscrolling is thrown loopy by this because of 
//	our pre-drawn items.
//						if ( g_MenuInfo.m_bCircularMenus )
//						{
//							g_MenuInfo.m_skin.Sounds.m_sndMenuDown.Play();
//							g_MenuInfo.m_iCurrentItem=0;
//							m_iScrollOffset = m_dwMenuItemSizeY;
//							m_nHoldControllerDown1=0;
//							ResetShotTimer();
//						}
//						else
//						{
							g_MenuInfo.m_iCurrentItem = (g_MenuInfo.GetItemCount()-1);
							m_nHoldControllerDown1=0;
//						}
					}
				}
			}
		}
		if ( m_iLastSelection != g_MenuInfo.m_iCurrentItem )
		{
			m_iLastSelection = g_MenuInfo.m_iCurrentItem;
		}
	}
	else
	{
		if ( (wButtons == 0) && (wRemotes == 0) && (byVirtKey == 0 ) && (iStickY==0) && (iStickX==0))
		{
			m_bAcceptInput = true;
		}
	}
	if ( m_bOneShot )
	{
		DEBUG_LINE( _T("Leaving MenuState::FrameMove") );
	}
	return S_OK;
}


HRESULT CXBoxMenuState::RenderSimpleTheme( void )
{
	return S_OK;
}

bool CXBoxMenuState::IsLayoutValid( void )
{
	bool bReturn = false;
	if ( m_pLayout )
	{
		bReturn = true;
	}
	else
	{
		m_pLayout = g_GadgetSkin.GetLayout( _T("menu") );
		if ( m_pLayout )
		{
			bReturn = true;
		}
	}
	return bReturn;
}


HRESULT CXBoxMenuState::Render()
{

	if ( m_bOneShot )
	{
		DEBUG_LINE( _T("Entering MenuState::Render") );
	}
	if ( IsLayoutValid() )
	{
		m_pLayout->Render();
	}
	if ( m_bOneShot )
	{
		m_bOneShot = false;
		DEBUG_LINE( _T("Leaving MenuState::Render") );
	}

	// Don't state the music until here...
//	if ( g_MenuInfo.m_bUseMusicManager && m_pbxApplication->m_pMusicManager )
//	{
//		if ( m_pbxApplication->m_pMusicManager->GetStatus() != MM_PLAYING )
//		{
//			m_pbxApplication->m_pMusicManager->Play();
//		}
//	}
	return S_OK;
}

HRESULT CXBoxMenuState::Cleanup()
{
    // Signal our worker thread to shutdown, then wait for it
    // before releasing objects
	return S_OK;
}



class CXBoxTitleState : public CXBoxGameState
{
	CGadgetLayout *		m_pLayout;
	bool				IsLayoutValid( void );
	DWORD m_dwTimeStamp;
	virtual bool MusicIsSuspended( void );
//	CImageSource m_imgTitle;
//	LPDIRECT3DTEXTURE8  m_pTitleTexture;
public:
	CXBoxTitleState(CXBoxStateApplication * pbxApplication ) : CXBoxGameState ( pbxApplication ), m_pLayout(NULL) {};
    virtual HRESULT Initialize();
    virtual HRESULT FrameMove();
    virtual HRESULT Render();
	virtual void OnNewSkin( void );
    virtual HRESULT Cleanup();
	HRESULT OnActivation( void );
	HRESULT OnDeactivation( void );
};

void CXBoxTitleState::OnNewSkin( void )
{
	m_pLayout = NULL;
}


bool CXBoxTitleState::MusicIsSuspended( void )
{
	bool bReturn = false;

	if ( m_pLayout )
	{
		if ( !m_pLayout->AllowMusic() )
		{
			bReturn = true;
		}
	}
	return bReturn;
}


bool CXBoxTitleState::IsLayoutValid( void )
{
	bool bReturn = false;
	if ( m_pLayout )
	{
		bReturn = true;
	}
	else
	{
		m_pLayout = g_GadgetSkin.GetLayout( _T("title") );
		if ( m_pLayout )
		{
			bReturn = true;
		}
	}
	return bReturn;
}



HRESULT CXBoxTitleState::OnActivation( void )	
{ 
	if ( IsLayoutValid() )
	{
		m_pLayout->OnActivation();
		m_pLayout->SetMusicEnable( true );
		GetStateApp()->SetBlend( m_pLayout->IsFadeIn() );
	}
//	g_MenuInfo.m_skin.MakeResources( m_pbxApplication->Get3dDevice(), m_pbxApplication->GetSoundDevice() );
	g_MenuInfo.m_skin.Textures.m_imgTitle.ReSync();
	if ( g_MenuInfo.m_lTitleDelay > (-1) )
	{
		m_dwTimeStamp = GetTickCount()+(DWORD)(g_MenuInfo.m_lTitleDelay);
	}
	else
	{
		m_dwTimeStamp = GetTickCount()+90000; 
	}
//	m_pbxApplication->SetBlend();
	return S_OK; 
}

HRESULT CXBoxTitleState::OnDeactivation( void )	
{ 
	if ( IsLayoutValid() )
	{
		m_pLayout->OnDeactivation();
	}
	return S_OK; 
}

HRESULT CXBoxTitleState::Initialize()            
{
	
#if 1
//	m_imgTitle.Initialize( m_pbxApplication->Get3dDevice(), m_pbxApplication->GetSoundDevice(), g_MenuInfo.m_sTitleScreen.c_str());
	
//	m_pTitleTexture = NULL;

//	D3DXCreateTextureFromFileEx( 
//			m_pbxApplication->Get3dDevice(), 
//			 g_MenuInfo.m_sTitleScreen.c_str(), 
//			// "D:\\title.bmp", 
//			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
//			0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
//			0xff000000, NULL, NULL,
//			(LPDIRECT3DTEXTURE8*)&m_pTitleTexture
//			);
#endif
	return S_OK; 
}

HRESULT CXBoxTitleState::FrameMove()             
{
//	WORD wButtons = this->m_pbxApplication->GetDefaultGamepad()->wButtons;
//	BYTE byVirtKey = m_pbxApplication->GetVirtualKey();
	char byVirtKey = m_pbxApplication->GetKeyChar();
	WORD wButtons = m_pbxApplication->GetDefaultGamepad()->wPressedButtons;
	WORD wRemotes = m_pbxApplication->m_DefaultIR_Remote.wPressedButtons;
	int iStickX,iStickY;
	FLOAT fX, fY;

	fX = m_pbxApplication->GetDefaultGamepad()->fX1+m_pbxApplication->GetDefaultGamepad()->fX2;
	fY = m_pbxApplication->GetDefaultGamepad()->fY1+m_pbxApplication->GetDefaultGamepad()->fY2;

	iStickX=0; iStickY=0;
	if ( fX < 0.0 )
	{
		iStickX = (int)(fX-0.5);
	}
	if ( fX > 0.0 )
	{
		iStickX = (int)(fX+0.5);
	}
	if ( fY < 0.0 )
	{
		iStickY = (int)(fY-0.5);
	}
	if ( fY > 0.0 )
	{
		iStickY = (int)(fY+0.5);
	}

	if ( IsLayoutValid() )
	{
		if ( g_MenuInfo.m_lTitleDelay != (-2) )
		{
			if ( m_pLayout->IsSequenceEnded() )
			{
				DEBUG_FORMAT( _T("TITLE STATE ENDED: SEQUENCE ENDED") );
				m_pbxApplication->SetGameState( GS_MENU );
			}
		}
	}
	
	if ( g_MenuInfo.m_lTitleDelay > (-1) )
	{
		if ( m_dwTimeStamp && m_dwTimeStamp < GetTickCount() )
		{
			DEBUG_FORMAT( _T("TITLE STATE ENDED: TIMEOUT (%ld)"),g_MenuInfo.m_lTitleDelay);
			// Proceed to next state
			m_pbxApplication->SetGameState( GS_MENU );
		}
	}
	
	if ( wButtons || m_pbxApplication->AnalogButtonsPressed(-2) || m_pbxApplication->GetVirtualKey() || byVirtKey || wRemotes || iStickX || iStickY )
	{
		DEBUG_FORMAT( _T("TITLE STATE ENDED: USER INPUT") );
		// Proceed to next state
		if ( m_pbxApplication->GetScreenCaptureState() == false )
		{
			m_pbxApplication->SetGameState( GS_MENU );
		}
	}
	return S_OK; 
}


HRESULT CXBoxTitleState::Render()                
{ 
	// Don't state the music until here...
//	if ( g_MenuInfo.m_bUseMusicManager && m_pbxApplication->m_pMusicManager )
//	{
//		if ( m_pbxApplication->m_pMusicManager->GetStatus() != MM_PLAYING )
//		{
//			m_pbxApplication->m_pMusicManager->Play();
//		}
//	}

	if ( IsLayoutValid() )
	{
		m_pLayout->Render();
	}
	return S_OK; 
}

HRESULT CXBoxTitleState::Cleanup()               
{ 
	return S_OK; 
}

static LPCTSTR s_szDashNames[] =
{ _T("EvoXDash"),
  _T("BoXplorer"),
  _T("Avalaunch"),
  _T("neXgen"),
  _T("MS Dashboard"),
  _T("MXM"),
  _T("Chihirox"),
  _T("XboxMediaPlayer"),
  _T("X-Selector"),
  NULL 
};

static DWORD s_dwDashXbeIDs[] =
{
	0x586f7645,
	0xffff051f,
	0,
	0x7867656e,
	0xfffe0000,
	0x00004321,
	0,
	0x0face007,
	0x01010001,
	0xffffffff
};

static LPCTSTR s_szDashXbeNames[] =
{
	_T("remotex"),
	_T("BoXplorer"),
	_T("Avalaunch"),
	_T("neXgen"),
	_T("Xbox Dashboard"),
	_T("MediaXMenu"),
	_T("Chihirox"),
	_T("XboxMediaPlayer"),
	_T("X-Selector"),
	NULL
};

CStdString GetDashboardName( TXBEINFO * pxbeInfo )
{
	CStdString sReturn;

	int iIndex = 0;

	while( s_dwDashXbeIDs[iIndex] != 0xffffffff && s_szDashXbeNames[iIndex] )
	{
		if ( s_dwDashXbeIDs[iIndex] )
		{
			if ( pxbeInfo->m_dwID == s_dwDashXbeIDs[iIndex] )
			{
				if ( pxbeInfo->m_dwID == 0xfffe0000 )
				{
					if ( _tcsicmp( s_szDashXbeNames[iIndex], pxbeInfo->m_sTitle ) )
					{
						sReturn = _T("XDK Launcher");
					}
					else
					{
						sReturn = s_szDashNames[iIndex];
					}
				}
				else
				{
					sReturn = s_szDashNames[iIndex];
				}
				break;
			}
		}
		else
		{
			if ( s_szDashXbeNames[iIndex] && ( _tcsicmp( s_szDashXbeNames[iIndex], pxbeInfo->m_sTitle ) == 0 ) )
			{
				sReturn = s_szDashNames[iIndex];
				break;
			}
		}
		iIndex++;
	}
	return sReturn;
}

static LPCTSTR s_szDashPaths[] =
{
	_T("C:\\"),
	_T("E:\\"),
	_T("F:\\"),
	_T("G:\\"),
	_T("E:\\dashboards"),
	_T("F:\\dashboards"),
	_T("G:\\dashboards"),
	_T("E:\\dash"),
	_T("F:\\dash"),
	_T("G:\\dash"),
	NULL
};

void LoadDashboards( void )
{
	if ( g_MenuInfo.m_bAutoCfgAddDash )
	{
		CXMLNode nodeSubMenuF;

		nodeSubMenuF.m_sName = _T("submenu");
		nodeSubMenuF.SetString( NULL, _T("title") , _T("Dashboards..."));
		nodeSubMenuF.SetString( NULL, szII_Descr , _T("SubMenu"));
//		nodeSubMenuF.SetString( NULL, _T("passcode") , _T("11"));
		// nodeSubMenuF.SetString( NULL, _T("sortfield"), _T("title"));
		CItemInfo itemInfo;

		int iDashDirIndex = 0;

		while( s_szDashPaths[iDashDirIndex] )
		{
			CStdString sWildCard;
			HANDLE hFind;
			WIN32_FIND_DATA ffData;

			sWildCard = MakeFullFilePath( s_szDashPaths[iDashDirIndex], _T("*.xbe") );
			hFind = FindFirstFile( sWildCard, &ffData );

			CStdString sNewPath;
//			DWORD dwID;
			CStdString sTitle;
			TXBEINFO xbeInfo;
			int iPos;
//			HRESULT hr;

			if( INVALID_HANDLE_VALUE != hFind )
			{
				do
				{
					if ( !(ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) )
					{
						// Copy file over, unless ZERO byte file?
						if ( ffData.nFileSizeLow || ffData.nFileSizeHigh )
						{
							sNewPath = MakeFullFilePath( s_szDashPaths[iDashDirIndex], ffData.cFileName );
							GetXbeInfo( sNewPath, xbeInfo );
							// Scan to see if it's valid...
							sTitle = GetDashboardName( &xbeInfo );
							if ( sTitle.GetLength() == 0 )
							{
								if ( xbeInfo.m_sTitle.GetLength() )
								{
									sTitle = xbeInfo.m_sTitle;
								}
								else
								{
									sTitle = ffData.cFileName;
									iPos = sTitle.Find( _T(".xbe") );
									if ( iPos > 0 )
									{
										sTitle = sTitle.Left( iPos );
									}
								}
							}

							if ( sTitle.GetLength() )
							{
								itemInfo.Clear();
								itemInfo.SetValue( szII_Title, sTitle );
								itemInfo.SetValue( szII_Dir, s_szDashPaths[iDashDirIndex] );
								itemInfo.SetValue( szII_Media, sNewPath );
								itemInfo.SetValue( szII_Exe, sNewPath );
								sNewPath.Format( _T("Dashboard (%s)"), s_szDashPaths[iDashDirIndex] );
								itemInfo.SetValue( szII_Descr, sNewPath );
								itemInfo.BuildNode( &nodeSubMenuF );
							}
						}
					}
				} while( FindNextFile( hFind, &ffData ) );
				// Close the find handle.
				FindClose( hFind );
			}
			iDashDirIndex++;
		}

#if 0		
		itemInfo.Clear();
		itemInfo.SetValue( szII_Title, _T("XboxDash") );
		itemInfo.SetValue( szII_Dir, _T("C:\\") );
		itemInfo.SetValue( szII_Descr, _T("Dashboard File") );
		itemInfo.SetValue( szII_Media, _T("C:\\xboxdash.xbe") );
		itemInfo.SetValue( szII_Exe, _T("C:\\xboxdash.xbe") );
		itemInfo.BuildNode( &nodeSubMenuF );

		itemInfo.Clear();
		itemInfo.SetValue( szII_Title, _T("EvoXDash") );
		itemInfo.SetValue( szII_Dir, _T("C:\\") );
		itemInfo.SetValue( szII_Descr, _T("Dashboard File") );
		itemInfo.SetValue( szII_Media, _T("C:\\evoxdash.xbe") );
		itemInfo.SetValue( szII_Exe, _T("C:\\evoxdash.xbe") );
		itemInfo.BuildNode( &nodeSubMenuF );

		itemInfo.Clear();
		itemInfo.SetValue( szII_Title, _T("neXgen") );
		itemInfo.SetValue( szII_Dir, _T("C:\\") );
		itemInfo.SetValue( szII_Descr, _T("Dashboard File") );
		itemInfo.SetValue( szII_Media, _T("C:\\nexgen.xbe") );
		itemInfo.SetValue( szII_Exe, _T("C:\\nexgen.xbe") );
		itemInfo.BuildNode( &nodeSubMenuF );

		itemInfo.Clear();
		itemInfo.SetValue( szII_Title, _T("MS Dashboard") );
		itemInfo.SetValue( szII_Dir, _T("C:\\") );
		itemInfo.SetValue( szII_Descr, _T("Dashboard File") );
		itemInfo.SetValue( szII_Media, _T("C:\\msdash.xbe") );
		itemInfo.SetValue( szII_Exe, _T("C:\\msdash.xbe") );
		itemInfo.BuildNode( &nodeSubMenuF );

		itemInfo.Clear();
		itemInfo.SetValue( szII_Title, _T("YBoxDash") );
		itemInfo.SetValue( szII_Dir, _T("C:\\") );
		itemInfo.SetValue( szII_Descr, _T("Dashboard File") );
		itemInfo.SetValue( szII_Media, _T("C:\\yboxdash.xbe") );
		itemInfo.SetValue( szII_Exe, _T("C:\\yboxdash.xbe") );
		itemInfo.BuildNode( &nodeSubMenuF );

		itemInfo.Clear();
		itemInfo.SetValue( szII_Title, _T("EvoX") );
		itemInfo.SetValue( szII_Dir, _T("C:\\") );
		itemInfo.SetValue( szII_Descr, _T("Dashboard File") );
		itemInfo.SetValue( szII_Media, _T("C:\\evox.xbe") );
		itemInfo.SetValue( szII_Exe, _T("C:\\evox.xbe") );
		itemInfo.BuildNode( &nodeSubMenuF );


		itemInfo.Clear();
		itemInfo.SetValue( szII_Title, _T("AvaLaunch") );
		itemInfo.SetValue( szII_Dir, _T("C:\\") );
		itemInfo.SetValue( szII_Descr, _T("Dashboard File") );
		itemInfo.SetValue( szII_Media, _T("C:\\avalaunch.xbe") );
		itemInfo.SetValue( szII_Exe, _T("C:\\avalaunch.xbe") );
		itemInfo.BuildNode( &nodeSubMenuF );

		itemInfo.Clear();
		itemInfo.SetValue( szII_Title, _T("MS Xbox Dash") );
		itemInfo.SetValue( szII_Dir, _T("C:\\") );
		itemInfo.SetValue( szII_Descr, _T("Dashboard File") );
		itemInfo.SetValue( szII_Media, _T("C:\\msxboxdash.xbe") );
		itemInfo.SetValue( szII_Exe, _T("C:\\msxboxdash.xbe") );
		itemInfo.BuildNode( &nodeSubMenuF );
#endif
		CXMLNode * pDashNode = g_MenuInfo.m_pMenuNode->CopyInNode(nodeSubMenuF);

		if ( g_MenuInfo.m_bAutoCfgSort && pDashNode->GetNodeCount() )
		{
			pDashNode = pDashNode->GetNode( 0 );
			if ( pDashNode )
			{
				pDashNode->SetString( NULL, _T("sortfield"), _T("title"));
			}
		}
	}

}


// Loading Entries:
// In XDI Mode
//		We get the entries from an XDI file
// Not XDI
//		If MXM.xml exists
//			We use MXM.xml to load entries
//		Else If Menu.xml exists (or no entries in MXM.xml file)
//			We use Menu.xml to load entries
//		Else
//			If In Dashboard Mode
//				We Auto-Load from roots of E:\ and F:\ through all subdirectories
//			Else
//				We Auto-Load from D:\ on down


int CMXMApp::LoadAutoCfg( void )
{
	int iItemCount = 0;

	// bool bDefaultOnly, bNoDemos;
	DWORD dwFlags = 0;


//	bDefaultOnly = ( g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("defaultonly"), FALSE ) );
//	bNoDemos = ( g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("nodemos"), FALSE ) );
	if ( g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("nodemos"), FALSE ) )
	{
		dwFlags |= ADIR_FLG_NODEMO;
	}
	if ( g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("defaultonly"), FALSE ) )
	{
        dwFlags |= ADIR_FLG_DEFONLY;
	}

	switch( g_MenuInfo.m_iAutoCfgStyle )
	{
		case 0:	// Apps and Games
			{
				CXMLNode nodeSubMenuGames;
				int iNumItems;
				nodeSubMenuGames.m_sName = _T("submenu");
				nodeSubMenuGames.SetString( NULL, _T("title") , _T("Games..."));
				nodeSubMenuGames.SetString( NULL, szII_Descr , _T("SubMenu"));
				if ( g_MenuInfo.m_bAutoCfgSort )
				{
					nodeSubMenuGames.SetString( NULL, _T("sortfield"), _T("title"));
				}
				iNumItems = LoadMenuFromDir( &nodeSubMenuGames, _T("E:\\GAMES"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuGames, _T("E:\\HDDLoader"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuGames, _T("F:\\GAMES"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuGames, _T("F:\\HDDLoader"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuGames, _T("G:\\GAMES"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuGames, _T("G:\\HDDLoader"), dwFlags, true, true );
				if ( iNumItems )
				{
					g_MenuInfo.m_pMenuNode->CopyInNode(nodeSubMenuGames);
					iItemCount += iNumItems;
				}
			}

			{
				CXMLNode nodeSubMenuApps;
				int iNumItems;
				nodeSubMenuApps.m_sName = _T("submenu");
				nodeSubMenuApps.SetString( NULL, _T("title") , _T("Apps..."));
				nodeSubMenuApps.SetString( NULL, szII_Descr , _T("SubMenu"));
				if ( g_MenuInfo.m_bAutoCfgSort )
				{
					nodeSubMenuApps.SetString( NULL, _T("sortfield"), _T("title"));
				}
				iNumItems = LoadMenuFromDir( &nodeSubMenuApps, _T("E:\\APPS"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuApps, _T("F:\\APPS"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuApps, _T("G:\\APPS"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuApps, _T("E:\\APP"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuApps, _T("F:\\APP"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuApps, _T("G:\\APP"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuApps, _T("E:\\UTIL"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuApps, _T("F:\\UTIL"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuApps, _T("G:\\UTIL"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuApps, _T("E:\\UTILS"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuApps, _T("F:\\UTILS"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuApps, _T("G:\\UTILS"), dwFlags, true, true );
				if ( iNumItems )
				{
					g_MenuInfo.m_pMenuNode->CopyInNode(nodeSubMenuApps);
					iItemCount += iNumItems;
				}
			}
			{
				CXMLNode nodeSubMenuGames;
				int iNumItems;
				nodeSubMenuGames.m_sName = _T("submenu");
				nodeSubMenuGames.SetString( NULL, _T("title") , _T("Emulators..."));
				nodeSubMenuGames.SetString( NULL, szII_Descr , _T("SubMenu"));
				if ( g_MenuInfo.m_bAutoCfgSort )
				{
					nodeSubMenuGames.SetString( NULL, _T("sortfield"), _T("title"));
				}
				iNumItems = LoadMenuFromDir( &nodeSubMenuGames, _T("E:\\EMUS"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuGames, _T("E:\\emulators"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuGames, _T("G:\\emulators"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuGames, _T("F:\\EMUS"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuGames, _T("F:\\emulators"), dwFlags, true, true );
				iNumItems += LoadMenuFromDir( &nodeSubMenuGames, _T("G:\\emulators"), dwFlags, true, true );
				if ( iNumItems )
				{
					g_MenuInfo.m_pMenuNode->CopyInNode(nodeSubMenuGames);
					iItemCount += iNumItems;
				}
			}
			{
				CXMLNode nodeSubMenuMisc;
				int iNumItems;
				nodeSubMenuMisc.m_sName = _T("submenu");
				nodeSubMenuMisc.SetString( NULL, _T("title") , _T("Misc..."));
				nodeSubMenuMisc.SetString( NULL, szII_Descr , _T("SubMenu"));
				if ( g_MenuInfo.m_bAutoCfgSort )
				{
					nodeSubMenuMisc.SetString( NULL, _T("sortfield"), _T("title"));
				}
				iNumItems = LoadMenuFromDir( &nodeSubMenuMisc, _T("E:\\"), 0, true, false );
				iNumItems += LoadMenuFromDir( &nodeSubMenuMisc, _T("F:\\"), 0, true, false );
				iNumItems += LoadMenuFromDir( &nodeSubMenuMisc, _T("G:\\"), 0, true, false );
				if ( iNumItems )
				{
					g_MenuInfo.m_pMenuNode->CopyInNode(nodeSubMenuMisc);
					iItemCount += iNumItems;
				}
			}
			break;
		case 1:	// By Drives
		default:
			{
				CXMLNode nodeSubMenuE;
				int iNumItems;
				nodeSubMenuE.m_sName = _T("submenu");
				nodeSubMenuE.SetString( NULL, _T("title") , _T("E Drive..."));
				nodeSubMenuE.SetString( NULL, szII_Descr , _T("SubMenu"));
				if ( g_MenuInfo.m_bAutoCfgSort )
				{
					nodeSubMenuE.SetString( NULL, _T("sortfield"), _T("title"));
				}
				iNumItems = LoadMenuFromDir( &nodeSubMenuE, _T("E:\\"), dwFlags, true, false );
				if ( iNumItems )
				{
					g_MenuInfo.m_pMenuNode->CopyInNode(nodeSubMenuE);
					iItemCount += iNumItems;
				}
			}
			{
				CXMLNode nodeSubMenuF;
				int iNumItems;
				nodeSubMenuF.m_sName = _T("submenu");
				nodeSubMenuF.SetString( NULL, _T("title") , _T("F Drive..."));
				nodeSubMenuF.SetString( NULL, szII_Descr , _T("SubMenu"));
				iNumItems = LoadMenuFromDir( &nodeSubMenuF, _T("F:\\"), dwFlags, true, false );
				if ( iNumItems )
				{
					g_MenuInfo.m_pMenuNode->CopyInNode(nodeSubMenuF);
					iItemCount += iNumItems;
				}
			}
			{
				CXMLNode nodeSubMenuG;
				int iNumItems;
				nodeSubMenuG.m_sName = _T("submenu");
				nodeSubMenuG.SetString( NULL, _T("title") , _T("G Drive..."));
				nodeSubMenuG.SetString( NULL, szII_Descr , _T("SubMenu"));
				iNumItems = LoadMenuFromDir( &nodeSubMenuG, _T("G:\\"), dwFlags, true, false );
				if ( iNumItems )
				{
					g_MenuInfo.m_pMenuNode->CopyInNode(nodeSubMenuG);
					iItemCount += iNumItems;
				}
			}
			break;
	}
	return iItemCount;
}


void CMXMApp::LoadEntries( void )
{
	int iItemCount = 0;
	bool bUseCache = true;
	bool bSaveCache = false;
//	WIN32_FILE_ATTRIBUTE_DATA fdCache;
//	WIN32_FILE_ATTRIBUTE_DATA fdConfig;
//	WIN32_FILE_ATTRIBUTE_DATA fdMenu;

	CStdString sConfigFile = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("mxm.xml") );
	CStdString sCacheFile = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("cache_menu.xml") );

	g_MenuInfo.m_bAllowAutoConfig = g_MenuInfo.m_pCfgInfo->GetBool( _T("Main"), _T("AllowAutoConfig"), FALSE )?true:false;
	g_MenuInfo.m_bAllowXDISkin = g_MenuInfo.m_pCfgInfo->GetBool( _T("Main"), _T("AllowXDISkin"), TRUE );

	g_MenuInfo.m_bAutoActionMenu = g_MenuInfo.m_pCfgInfo->GetBool( _T("autocfg"), _T("actionmenu"), FALSE );
	g_MenuInfo.m_bAutoActionMenu = g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("actionmenu"), g_MenuInfo.m_bAutoActionMenu );

#if 0
	if ( !g_MenuInfo.m_bDVDMode )
	{
		if ( !(g_MenuInfo.SafeMode.m_dwFlags & SM_SAFEMENU ))
		{

			CStdString sMnuFile = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("menu.xml") );

			if ( GetFileInfo( sCacheFile, &fdCache ) )
			{
				if ( GetFileInfo( sConfigFile, &fdConfig ) )
				{
					if ( GetFileInfo( sMnuFile, &fdMenu ) )
					{
						if ( CompareFileTime(&fdConfig.ftLastWriteTime, &fdCache.ftLastWriteTime) < 0 )
						{
							// Might still use cache...
							if ( CompareFileTime(&fdMenu.ftLastWriteTime, &fdCache.ftLastWriteTime) < 0 )
							{
								// WILL use cache
								bUseCache = true;
							}
						}
						// ELSE not  using cache, nreed to reset it!
					}
					else
					{
						// No menu.xml file.
						if ( CompareFileTime(&fdConfig.ftLastWriteTime, &fdCache.ftLastWriteTime) < 0 )
						{
							bUseCache = true;
						}
					}
				}
				else
				{
					if ( GetFileInfo( sMnuFile, &fdMenu ) )
					{
						if ( CompareFileTime(&fdMenu.ftLastWriteTime, &fdCache.ftLastWriteTime) < 0 )
						{
							// WILL use cache
							bUseCache = true;
						}
					}
					else
					{
						bUseCache = true;
					}
				}
				
			}
		}
	}
#else

	if ( g_MenuInfo.m_bDVDMode || ( g_MenuInfo.SafeMode.m_dwFlags & SM_SAFEMENU ) )
	{
		bUseCache = false;
	}

#endif

	g_MenuInfo.m_slMenuFileList.clear();
	g_MenuInfo.m_slMenuDirList.clear();
	if ( g_MenuInfo.m_pMenuNode )
	{
		delete g_MenuInfo.m_pMenuNode;
		g_MenuInfo.m_pMenuNode = NULL;
	}

	g_MenuInfo.m_pMenuNode = new CXMLNode( NULL, _T("Menu") );

	if ( g_MenuInfo.m_bXDIMode )
	{
		//////////////////////////////////////////////////////////
		// In XDI Mode
		//		We get the entries from an XDI file
		//////////////////////////////////////////////////////////
		CXMLNode * pXDINode = GetXDI();
		if ( pXDINode && pXDINode->GetNodeCount(_T("Item")) )
		{
//			bDoAuto = false;

			iItemCount += LoadMenuFromNode( g_MenuInfo.m_pMenuNode, pXDINode );
			delete pXDINode;
		}
		else
		{
			// Doh! What happened???
		}

	}
	else
	{
		//////////////////////////////////////////////////////////
		// Not XDI
		//		If MXM.xml exists
		//			We use MXM.xml to load entries
		//		Else If Menu.xml exists (or no entries in MXM.xml file)
		//			We use Menu.xml to load entries
		//		Else
		//			If In Dashboard Mode
		//				We Auto-Load from roots of E:\ and F:\ through all subdirectories
		//			Else
		//				We Auto-Load from D:\ on down
		//////////////////////////////////////////////////////////

		// New logic:
		// If cache_menu.cxml exists, check against mxm.xml and/or menu.xml

		if ( !g_MenuInfo.m_bDVDMode )
		{
			if ( g_MenuInfo.m_pPrefInfo->GetBool( _T("menu"), _T("usecache"), g_MenuInfo.m_pCfgInfo->GetBool( _T("Main"), _T("UseMenuCache"), TRUE )) == FALSE )
			{
				bUseCache = false;
			}
			else
			{
				bUseCache = true;
			}
		}
		else
		{
			bUseCache = false;
		}

		if ( (g_MenuInfo.SafeMode.m_dwFlags & SM_SAFEMENU ))
		{
			bUseCache = false;
		}

		if ( bUseCache )
		{
			if ( FileExists( sCacheFile ) )
			{
				iItemCount += LoadMenuFromFile( g_MenuInfo.m_pMenuNode, sCacheFile );
				DEBUG_FORMAT( _T("USING CACHE_MENU.XML: %d Items found"), iItemCount );
				if ( iItemCount == 0 )
				{
					// Reset, nothing found in cached menu!!
					// bUseCache = false;
				}
			}
		}

		if ( (iItemCount == 0) && FileExists( sConfigFile ) )
		{
//			CXMLNode * pMainNode;
//			CXMLLoader xmlLoader(NULL,true);
			CXMLNode * pSubNode;
			CXMLNode * pItemNode;
			CStdString sNodeName;
			int iItemNum = 1;
			CStdString sDir;
	
			sDir = StripFilenameFromPath( sConfigFile );
			// pMainNode will be deleted when XMLLoader goes out of scope
				// C:
			//pMainNode = xmlLoader.LoadXMLNodes( sConfigFile );


			if ( !bUseCache  || !FileExists( sCacheFile ))
			{
				// First, we look for the Item_X entries
				sNodeName.Format(_T("Item_%d"), iItemNum++ );
				pSubNode = g_MenuInfo.m_pCfgInfo->GetNode( sNodeName );
				while( pSubNode )
				{
					// Transfer the contents of one node to another!
					pItemNode = g_MenuInfo.m_pMenuNode->AddNode( _T("item") );
					if ( pItemNode )
					{
						pItemNode->m_msAttributes = pSubNode->m_msAttributes;
						pItemNode->m_msElements = pSubNode->m_msElements;
						iItemCount++;
					}
					sNodeName.Format(_T("Item_%d"), iItemNum++ );
					pSubNode = g_MenuInfo.m_pCfgInfo->GetNode( sNodeName );
				}

				//if ( iItemCount == 0 )
				//{
				//	pSubNode = pMainNode->GetNode( _T("item") );
				//	while( pSubNode )
				//	{
				//		// Transfer the contents of one node to another!
				//		pItemNode = g_MenuInfo.m_pMenuNode->AddNode( _T("item") );
				//		if ( pItemNode )
				//		{
				//			pItemNode->m_msAttributes = pSubNode->m_msAttributes;
				//			pItemNode->m_msElements = pSubNode->m_msElements;
				//			iItemCount++;
				//		}
				//		pSubNode = pMainNode->GetNode( _T("item") );
				//	}
				//}

				g_MenuInfo.m_pCfgInfo->SetString( NULL, _T("filedirectory"), sDir );
				iItemCount += LoadMenuFromNode( g_MenuInfo.m_pMenuNode, g_MenuInfo.m_pCfgInfo );

				if ( iItemCount == 0 )
				{
					// Look for "Menu" node
					CXMLNode * pMXMMenuNode = g_MenuInfo.m_pCfgInfo->GetNode( _T("menu") );
					if ( pMXMMenuNode )
					{
						pMXMMenuNode->SetString( NULL, _T("filedirectory"), sDir );
						iItemCount += LoadMenuFromNode( g_MenuInfo.m_pMenuNode, pMXMMenuNode );
					}
				}
			}
		}
	}


	if ( iItemCount==0 )
	{
		CStdString sMenuFile;

		if ( !(g_MenuInfo.SafeMode.m_dwFlags & SM_SAFEMENU ))
		{
			// Look for Menu.xml at this point...
			sMenuFile = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("menu.xml") );
			if ( FileExists( sMenuFile ) )
			{
				iItemCount += LoadMenuFromFile( g_MenuInfo.m_pMenuNode, sMenuFile );
			}
		}
	}

	if ( g_MenuInfo.m_bDashMode && !g_MenuInfo.m_bDVDMode )
	{
		if ( iItemCount == 0 )
		{
			iItemCount += LoadAutoCfg();
		}
	}
	else
	{
		if ( !bUseCache )
		{
			if ( (g_MenuInfo.m_bAllowAutoConfig || (iItemCount==0)) && ( g_MenuInfo.m_sActualPath.Find( _T("D:") ) > -1 ) )
			{
				// At this point, we don't worry about anything but immediate subdirectories
				// In this case, we perform an auto-dir on the aforementioned level
				// Are we running on the hard drive, or on the DVD?
				iItemCount += LoadMenuFromDir( g_MenuInfo.m_pMenuNode, g_MenuInfo.m_sMXMPath, 0, false, true ); // _T("D:\\") );
			}
		}
		if ( g_MenuInfo.m_bDVDMode )
		{
			if ( iItemCount == 0 )
			{
				iItemCount += LoadMenuFromDir( g_MenuInfo.m_pMenuNode, g_MenuInfo.m_sMXMPath, 0, false, true ); // _T("D:\\") );
				// Bad news if we don't see entries by now....
			}
			if ( iItemCount == 0 )
			{
				iItemCount += LoadMenuFromDir( g_MenuInfo.m_pMenuNode, _T("D:\\"), 0, false, true ); // _T("D:\\") );
			}
		}
		if ( iItemCount == 0 )
		{
			iItemCount += LoadAutoCfg();
			// LoadDashboards();
		}
	}


	// Validate menus here...
	iItemCount = ValidateEntries(bSaveCache);

	if ( iItemCount == 0 )
	{
		// Sigh.... NOTHING!!! Load up dashboards into menu list
		iItemCount += LoadAutoCfg();
		// LoadDashboards();
		iItemCount = ValidateEntries(bSaveCache);
	}

	if ( !g_MenuInfo.m_bDVDMode )
	{
		g_MenuInfo.FindDVDPlayer();
		CStdString sCacheMenu = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("cache_menu.xml") );

		if ( bUseCache && (!FileExists( sCacheMenu ) || bSaveCache) )
		{
			g_MenuInfo.m_pMenuNode->SaveNode( sCacheMenu );
		}
		if ( iItemCount == 0 )
		{
			// Force the issue.
			g_MenuInfo.m_bAutoCfgAddDash = true;
		}
		LoadDashboards();
		// iItemCount = ValidateEntries(bSaveCache);
	}
	else
	{
		if ( iItemCount == 0 )
		{
			g_MenuInfo.m_bAutoCfgAddDash = true;
			LoadDashboards();
		}
	}
	// Check for sorting here?

	// Add Action Menu
	if ( g_MenuInfo.m_bAutoActionMenu )
	{
		CXMLNode nodeSubMenuSys;
		nodeSubMenuSys.m_sName = _T("submenu");
		nodeSubMenuSys.SetString( NULL, _T("title") , _T("Action..."));
		nodeSubMenuSys.SetString( NULL, _T("media") , _T("::ICO_ACTION"));
		nodeSubMenuSys.SetString( NULL, _T("thumbnail") , _T("::ICO_ACTION"));
		nodeSubMenuSys.SetString( NULL, szII_Descr , _T("SubMenu"));
		{
			CXMLNode nodeAction;
			CStdString sActionScript;
			nodeAction.m_sName = _T("item");
			nodeAction.SetString( NULL, _T("sortfield") , _T("nothing"));
			nodeAction.SetString( NULL, _T("title") , _T("Run Test Script"));
//			nodeAction.SetString( NULL, _T("passcode") , _T("11"));
			nodeAction.SetString( NULL, szII_Descr , _T(""));
			sActionScript = _T("CallScript _TestScript\r\n");
			nodeAction.SetString( NULL, _T("action") , sActionScript );
			nodeAction.SetString( NULL, _T("media") , _T("::ICO_ACTION"));
			nodeAction.SetString( NULL, _T("thumbnail") , _T("::ICO_ACTION"));
			nodeSubMenuSys.CopyInNode(nodeAction);
		}
#if 1
		{
			CXMLNode nodeAction;
			CStdString sActionScript;
			nodeAction.m_sName = _T("item");
			nodeAction.SetString( NULL, _T("sortfield") , _T("nothing"));
			nodeAction.SetString( NULL, _T("title") , _T("Input Test"));
//			nodeAction.SetString( NULL, _T("passcode") , _T("11"));
			nodeAction.SetString( NULL, szII_Descr , _T(""));
			sActionScript = _T("debug \"First Line $time$\"\r\n");
			sActionScript += _T("SETFUNC TDataExists FileExists E:\\TDATA\r\n");
			sActionScript += _T("IF %TDataExists% == 1 GOTO TDATACHECK\r\n");
			sActionScript += _T("CallScript _DisplayMessageBox \"Did not find E:\\TDATA\"\r\n");
			sActionScript += _T("GOTO TDATAEND\r\n");
			sActionScript += _T(":TDATACHECK\r\n");
			sActionScript += _T("CallScript _DisplayMessageBox \"Found E:\\\\TDATA\"\r\n");
			sActionScript += _T(":TDATAEND\r\n");
			sActionScript += _T("CallFile TestScript.xas\r\n");
			sActionScript += _T("CallScript _DisplayMessageBox \"Testing this call function out!\"\r\n");
			sActionScript += _T("BeginDraw UseCurrent\r\n");
			sActionScript += _T("MessageBox \"This is a message$eol$Second Line\"\r\n");
			sActionScript += _T("Box 20 20 100 100 Red White\r\n");
			sActionScript += _T("Text 25 25 LEFT \"Test Text\" BLUE\r\n");

			sActionScript += _T("EndDraw\r\n");
			sActionScript += _T("Input\r\n");
			sActionScript += _T("If %_GP_A% == \"1\" GOTO APRESSED\r\n");
			sActionScript += _T("If %_GP_B% == \"1\" GOTO BPRESSED\r\n");
			sActionScript += _T("If %_GP_X% == \"1\" GOTO XPRESSED\r\n");
			sActionScript += _T("If %_GP_Y% == \"1\" GOTO YPRESSED\r\n");
			sActionScript += _T("QUIT\r\n");
			sActionScript += _T(":APRESSED\r\n");
			sActionScript += _T("MessageBox \"A Was Pressed!$eol$Second Line$eol$Third $lt$line$gt$\"\r\n");
			sActionScript += _T("QUIT\r\n");
			sActionScript += _T(":BPRESSED\r\n");
			sActionScript += _T("MessageBox \"B Was Pressed!\"\r\n");
			sActionScript += _T("QUIT\r\n");
			sActionScript += _T(":XPRESSED\r\n");
			sActionScript += _T("MessageBox \"X Was Pressed!\"\r\n");
			sActionScript += _T("QUIT\r\n");
			sActionScript += _T(":YPRESSED\r\n");
			sActionScript += _T("MessageBox \"Y Was Pressed!\"\r\n");
			nodeAction.SetString( NULL, _T("action") , sActionScript );
			nodeAction.SetString( NULL, _T("media") , _T("::ICO_ACTION"));
			nodeAction.SetString( NULL, _T("thumbnail") , _T("::ICO_ACTION"));
			nodeSubMenuSys.CopyInNode(nodeAction);
		}
		{

			CXMLNode nodeAction;
			CStdString sActionScript;
			nodeAction.m_sName = _T("item");
			nodeAction.SetString( NULL, _T("sortfield") , _T("nothing"));
			nodeAction.SetString( NULL, _T("title") , _T("Multi-Line Test"));
			nodeAction.SetString( NULL, szII_Descr , _T(""));
			sActionScript = _T("debug \"First Line $time$\"\r\n");
			sActionScript += _T("set testvar %testvar%TestIt\r\nset _testglobal %_testglobal%TestGlobal!\r\n");
			sActionScript += _T("set numvar 1\r\n");
			sActionScript += _T("debug \"Set numvar to %numvar% \"\r\n");
			sActionScript += _T("add numvar 1\r\n");
			sActionScript += _T("debug \"Added 1 to numvar = %numvar% \"\r\n");
			sActionScript += _T("add numvar 20\r\n");
			sActionScript += _T("debug \"Added 20 to numvar = %numvar% \"\r\n");
			sActionScript += _T("if# %numvar% > 20 GOTO TESTONE\r\n");
			sActionScript += _T("debug \"This line shouldn't be displayed from numerical test!\"\r\n");
			sActionScript += _T(":TESTONE\r\n");
			sActionScript += _T("if %numvar% > 100 GOTO TESTTWO\r\n");
			sActionScript += _T("debug \"This line shouldn't be displayed from string test!\"\r\n");
			sActionScript += _T(":TESTTWO\r\n");
			sActionScript += _T("debug \"Second Line %testvar% and %_testglobal% \"\r\n");
			sActionScript += _T("GOTO SKIP\r\n");
			sActionScript += _T("debug \"Skip This Line\"\r\n");
			sActionScript += _T(":SKIP\r\n");
			sActionScript += _T("debug \"Last Line\"\r\n");
			sActionScript += _T("begindraw usecurrent\r\n");
			sActionScript += _T("messagebox \"This is a message box$eol$Fanspeed: $fanspeed$\"");
			sActionScript += _T("enddraw\r\n");
			sActionScript += _T("input\r\n");
			nodeAction.SetString( NULL, _T("action") , sActionScript );
			nodeAction.SetString( NULL, _T("media") , _T("::ICO_ACTION"));
			nodeAction.SetString( NULL, _T("thumbnail") , _T("::ICO_ACTION"));
			nodeSubMenuSys.CopyInNode(nodeAction);
		}
#endif

		{
			CXMLNode nodeAction;
			nodeAction.m_sName = _T("item");
			nodeAction.SetString( NULL, _T("title") , _T("System Menu"));
			nodeAction.SetString( NULL, _T("sortfield") , _T("nothing"));
			nodeAction.SetString( NULL, szII_Descr , _T(""));
			nodeAction.SetString( NULL, _T("action") , _T("systemmenu activatefull"));
			nodeAction.SetString( NULL, _T("media") , _T("::ICO_ACTION"));
			nodeAction.SetString( NULL, _T("thumbnail") , _T("::ICO_ACTION"));
			nodeSubMenuSys.CopyInNode(nodeAction);
		}
		{
			CXMLNode nodeAction;
			nodeAction.m_sName = _T("item");
			nodeAction.SetString( NULL, _T("title") , _T("Hide System Menu"));
			nodeAction.SetString( NULL, szII_Descr , _T(""));
			nodeAction.SetString( NULL, _T("action") , _T("systemmenu hide\r\nmessagebox \"System Menu Hidden\""));
			nodeAction.SetString( NULL, _T("media") , _T("::ICO_ACTION"));
			nodeAction.SetString( NULL, _T("thumbnail") , _T("::ICO_ACTION"));
			nodeSubMenuSys.CopyInNode(nodeAction);
		}
		{
			CXMLNode nodeAction;
			nodeAction.m_sName = _T("item");
			nodeAction.SetString( NULL, _T("title") , _T("Unhide System Menu"));
			nodeAction.SetString( NULL, szII_Descr , _T(""));
			nodeAction.SetString( NULL, _T("action") , _T("systemmenu unhide\r\nmessagebox \"System Menu Now Available\""));
			nodeAction.SetString( NULL, _T("media") , _T("::ICO_ACTION"));
			nodeAction.SetString( NULL, _T("thumbnail") , _T("::ICO_ACTION"));
			nodeSubMenuSys.CopyInNode(nodeAction);
		}
		{
			CXMLNode nodeAction;
			nodeAction.m_sName = _T("item");
			nodeAction.SetString( NULL, _T("title") , _T("Eject Drive Tray"));
			nodeAction.SetString( NULL, _T("sortfield") , _T("nothing"));
			nodeAction.SetString( NULL, szII_Descr , _T(""));
			nodeAction.SetString( NULL, _T("action") , _T("trayopen"));
			nodeAction.SetString( NULL, _T("media") , _T("::ICO_ACTION"));
			nodeAction.SetString( NULL, _T("thumbnail") , _T("::ICO_ACTION"));
			nodeSubMenuSys.CopyInNode(nodeAction);
		}
		{
			CXMLNode nodeAction;
			nodeAction.m_sName = _T("item");
			nodeAction.SetString( NULL, _T("title") , _T("Close Drive Tray"));
			nodeAction.SetString( NULL, szII_Descr , _T(""));
			nodeAction.SetString( NULL, _T("action") , _T("trayclose"));
			nodeAction.SetString( NULL, _T("media") , _T("::ICO_ACTION"));
			nodeAction.SetString( NULL, _T("thumbnail") , _T("::ICO_ACTION"));
			nodeSubMenuSys.CopyInNode(nodeAction);
		}
		// reboot, shutdown, powercycle
		{
			CXMLNode nodeAction;
			nodeAction.m_sName = _T("item");
			nodeAction.SetString( NULL, _T("title") , _T("Reboot"));
			nodeAction.SetString( NULL, szII_Descr , _T(""));
			nodeAction.SetString( NULL, _T("action") , _T("reboot"));
			nodeAction.SetString( NULL, _T("media") , _T("::ICO_ACTION"));
			nodeAction.SetString( NULL, _T("thumbnail") , _T("::ICO_ACTION"));
			nodeSubMenuSys.CopyInNode(nodeAction);
		}
		{
			CXMLNode nodeAction;
			nodeAction.m_sName = _T("item");
			nodeAction.SetString( NULL, _T("title") , _T("Shutdown"));
			nodeAction.SetString( NULL, szII_Descr , _T(""));
			nodeAction.SetString( NULL, _T("action") , _T("shutdown"));
			nodeAction.SetString( NULL, _T("media") , _T("::ICO_ACTION"));
			nodeAction.SetString( NULL, _T("thumbnail") , _T("::ICO_ACTION"));
			nodeSubMenuSys.CopyInNode(nodeAction);
		}
		{
			CXMLNode nodeAction;
			nodeAction.m_sName = _T("item");
			nodeAction.SetString( NULL, _T("title") , _T("Power Cycle"));
			nodeAction.SetString( NULL, szII_Descr , _T(""));
			nodeAction.SetString( NULL, _T("action") , _T("powercycle"));
			nodeAction.SetString( NULL, _T("media") , _T("::ICO_ACTION"));
			nodeAction.SetString( NULL, _T("thumbnail") , _T("::ICO_ACTION"));
			nodeSubMenuSys.CopyInNode(nodeAction);
		}
		{
			CXMLNode nodeAction;
			nodeAction.m_sName = _T("item");
			nodeAction.SetString( NULL, _T("title") , _T("EEPROM Backup"));
			nodeAction.SetString( NULL, szII_Descr , _T(""));
			nodeAction.SetString( NULL, _T("action") , _T("eeprombackup C:\\eeprom_mxm.bin"));
			nodeAction.SetString( NULL, _T("media") , _T("::ICO_ACTION"));
			nodeAction.SetString( NULL, _T("thumbnail") , _T("::ICO_ACTION"));
			nodeSubMenuSys.CopyInNode(nodeAction);
		}
		{
			CXMLNode nodeAction;
			nodeAction.m_sName = _T("item");
			nodeAction.SetString( NULL, _T("title") , _T("EEPROM Restore"));
			nodeAction.SetString( NULL, szII_Descr , _T(""));
			nodeAction.SetString( NULL, _T("action") , _T("eepromrestore C:\\eeprom_mxm.bin"));
			nodeAction.SetString( NULL, _T("media") , _T("::ICO_ACTION"));
			nodeAction.SetString( NULL, _T("thumbnail") , _T("::ICO_ACTION"));
			nodeSubMenuSys.CopyInNode(nodeAction);
		}
		g_MenuInfo.m_pMenuNode->CopyInNode(nodeSubMenuSys);
	}

	g_MenuInfo.LoadMenu( g_MenuInfo.m_pMenuNode, true);
}



bool CheckForNotPrior( LPCTSTR szXbeFile, bool bAllowDuplicate = false )
{
	bool bReturn = true;
	static TListStrings slXBEs;

	if ( g_MenuInfo.m_bRemoveDuplicates && !bAllowDuplicate )
	{
		if ( szXbeFile && _tcslen( szXbeFile ) )
		{
			if ( _tcscmp( szXbeFile, _T("RESET") ) == 0 )
			{
				slXBEs.clear();
			}
			else
			{
				if ( HasString( slXBEs, szXbeFile, true ) )
				{
					bReturn = false;
				}
				else
				{
					slXBEs.push_back( szXbeFile );
				}
			}
		}
	}
	return bReturn;
}

bool CMXMApp::ValidateEntry( bool & bChangeMade, CXMLNode * pNode )
{
	bool bReturn = false;
//Each Xbe Entry will perform the following checks:
//- File Must Exist (Unless "Debug" is on)
//- If No Title, one is grabbed from the Xbe
//- If No Description, One will be generated (Unless disabled via config)
//- If No Media Entry:
//  Searches for MXM_SS.xxx files. WMV first, followed by JPG, BMP, TGA, PNG, and GIF
//  Uses TitleImage.xbx, if available
//- Creates a "Thumbnail" Entry for TitleImage.xbx file (uses Media if not available)  
	CStdString sXbe, sExe, sDir, sDescr, sDescription, sMedia, sThumbnail, sTitle, sAction;

	// DWORD dwReturn;
	TXBEINFO xbeInfo;
	bool bAllowDupe;

	bChangeMade = false;

	sDir = pNode->GetString( NULL, szII_Dir, _T(""), true );
	sExe = pNode->GetString( NULL, szII_Exe, _T(""), true );
	bAllowDupe = (pNode->GetBool( NULL, _T("DuplicateOK"), FALSE ) == TRUE);
	sAction = pNode->GetString( NULL, _T("action"), _T("") );

	if ( sExe.GetLength() == 0 )
	{
		sExe = pNode->GetString( NULL, _T("xbe"), _T(""), true );
	}
	if ( sExe.GetLength() == 0 )
	{
		sExe = _T("default.xbe");
	}

	
	if ( (pNode->GetBool( NULL, _T("Verified"), FALSE ) == FALSE ) && (sAction.GetLength()==0) )
	{
		if ( sDir.GetLength() )
		{
			sXbe = MakeFullFilePath( sDir, sExe );
		}
		else
		{
			sXbe = sExe;
			sDir = StripFilenameFromPath( sXbe );
		}

		if ( sDir.GetLength() && sXbe.GetLength() && FileExists( sXbe ) && CheckForNotPrior( sXbe, bAllowDupe ) )
		{

			bReturn = true;
			
			xbeInfo.m_dwID = 0;

			GetXbeInfo( sXbe, xbeInfo );

			sTitle = pNode->GetString( NULL, szII_Title, _T(""), true );
			sTitle.TrimLeft();
			sTitle.Trim();

			if ( sTitle.GetLength() == 0 )
			{
				int iPos;
				CStdString sName;

				sName = ExtractFilenameFromPath( sXbe );
				iPos = sName.ReverseFind( _T(".") );
				if ( iPos >= 0 )
				{
					sTitle = sName.Left( iPos );
				}
				if ( _tcsicmp( sTitle, _T("default") ) == 0 )
				{
					// Use the directory name...
					sTitle = ExtractFilenameFromPath( sDir, false );
				}
			}

			sDescr = pNode->GetString( NULL, szII_Descr, _T(""), true );
			sDescription = pNode->GetString( NULL, _T("description"), _T(""), true );

			if ( sDescr.GetLength() == 0 )
			{
				if ( sDescription.GetLength() )
				{
					sDescr = sDescription;
				}
				else
				{
					// Get Description from XBE
					if ( xbeInfo.m_sDescription.GetLength() )
					{
						sDescr = xbeInfo.m_sDescription;
					}
					else
					{
						sDescr = xbeInfo.m_sPublisher;
					}
				}
			}

			if ( sDescription.GetLength() == 0 )
			{
				sDescription = sDescr;
			}

			if ( sDir.GetLength() )
			{
				sMedia = MakeFullFilePath( sDir, pNode->GetString( NULL, szII_Media, _T(""), true ) );
			}
			else
			{
				sMedia = pNode->GetString( NULL, szII_Media, _T(""), true );
			}

			if ( sDir.GetLength() )
			{
				sThumbnail = MakeFullFilePath( sDir, pNode->GetString( NULL, szII_Thumbnail, _T(""), true ) );
			}
			else
			{
				sThumbnail = pNode->GetString( NULL, szII_Thumbnail, _T(""), true );
			}


			if ( !(sMedia.GetLength() && FileExists( sMedia ) ) )
			{
				// Find Media
				//- If No Media Entry:
				//  Searches for MXM_SS.xxx files. WMV first, followed by JPG, BMP, TGA, PNG, and GIF
				//  Uses TitleImage.xbx, if available
				sMedia = FindMedia( sDir );
				if ( sMedia.GetLength() == 0 )
				{
					// Still no go... use XBX, then.
					sMedia.Format( _T("E:\\UDATA\\%08x\\TitleImage.xbx"), xbeInfo.m_dwID );
					if ( !FileExists( sMedia ) )
					{
						// Hmmmph. Still no go, eh?
						// Last ditch effort to locate something we can use
						sMedia = FindFile( sDir, _T("TitleImage.xbx"), true );
						if ( sMedia.GetLength() == 0 && FileExists( sThumbnail) )
						{
							sMedia = sThumbnail;
						}
					}
				}
			}

			if ( !(sThumbnail.GetLength() && FileExists( sThumbnail ) ) )
			{
				// Locate a thumbnail file
				sThumbnail = FindMedia( sDir, false );
				if ( sThumbnail.GetLength() == 0 )
				{
					// Still no go... use XBX, then.
					sThumbnail.Format( _T("E:\\UDATA\\%08x\\TitleImage.xbx"), xbeInfo.m_dwID );
					if ( !FileExists( sThumbnail ) )
					{
						// Hmmmph. Still no go, eh?
						// Last ditch effort to locate something we can use
						sThumbnail = FindFile( sDir, _T("TitleImage.xbx"), true );
						if ( sThumbnail.GetLength() == 0 && FileExists( sMedia) )
						{
							sThumbnail = sMedia;
						}
					}
				}
			}
			

			if ( sTitle.GetLength() )
			{
				pNode->SetString( NULL, szII_Title, sTitle );
			}
			if ( sMedia.GetLength() )
			{
				pNode->SetString( NULL, szII_Media, sMedia );
			}
			if ( sThumbnail.GetLength() )
			{
				pNode->SetString( NULL, szII_Thumbnail, sThumbnail );
			}
			if ( sXbe.GetLength() )
			{
				pNode->SetString( NULL, szII_Exe, sXbe );
			}
			if ( sDescr.GetLength() )
			{
				pNode->SetString( NULL, szII_Descr, sDescr );
			}
			if ( sDescription.GetLength() )
			{
				pNode->SetString( NULL, _T("Description"), sDescription );
			}
			pNode->SetLong( NULL, _T("XbeID"), xbeInfo.m_dwID );
			bChangeMade = true;
		}
		if ( g_MenuInfo.m_bDebugMenu )
		{
			if ( bReturn == false )
			{
				pNode->SetString( NULL, _T("error"), _T("failed to find xbe") );
				bChangeMade = true;
			}
			bReturn = true;
		}
		if ( bReturn == true )
		{
			pNode->SetBool( NULL, _T("Verified"), TRUE );
			bChangeMade = true;
		}
	}
	else
	{
		bReturn = true;
	}
	return bReturn;
}

int CMXMApp::ValidateEntries( bool & bChangesMade, CXMLNode * pNode )
{
	int iReturn = 0;
	int iIndex;
	int iNumItems;
	int iCheck;
	CXMLNode * pSubNode;
	CStdString sName;
	bool bChangeMade = false;
	bool bSubChangesMade = false;

	bChangesMade = false;
	if ( pNode == NULL )
	{
		pNode = g_MenuInfo.m_pMenuNode;
	}
	// OK, parse through each entry. Determine if it's valid or not.
	if ( g_MenuInfo.m_bRemoveInvalid && pNode && pNode->GetNodeCount() )
	{
		iIndex = 0;
		while( pNode->GetNodeCount() && (pSubNode = pNode->GetNode( iIndex )) )
		{
			sName = pSubNode->m_sName;
			sName.MakeLower();
			if ( sName.Compare( _T("submenu") ) == 0 )
			{
				iNumItems = ValidateEntries( bSubChangesMade, pSubNode );
				if ( iNumItems == 0 )
				{
					iCheck = pNode->GetNodeCount();
					pNode->DeleteNode( pSubNode );
					iCheck = pNode->GetNodeCount();
				}
				else
				{
					iIndex++;
					iReturn += iNumItems;
				}
				if ( bSubChangesMade )
				{
					bChangesMade = true;
				}
			}
			else if ( sName.Compare( _T("item") ) == 0 )
			{
				if ( ValidateEntry( bChangeMade, pSubNode ) )
				{
					iIndex++;
					iReturn++;
				}
				else
				{
					iCheck = pNode->GetNodeCount();
					pNode->DeleteNode( pSubNode );
					iCheck = pNode->GetNodeCount();
				}
				if ( bChangeMade )
				{
					bChangesMade = true;
				}
			}
			else
			{
				iIndex++;
			}
		}
	}
	return iReturn;
}


void CMXMApp::LoadSkins( void )
{
	list<CStdString> saLocations;
	list<CStdString>::iterator iterDirs;
	CStdString sSkinName, sSkinDir;
	CXMLNode * pMXMXML = NULL;
	CXMLNode * pSkinXML = NULL;
	CXMLLoader xmlLoader(NULL,true);
	CStdString sConfigureFile;
	CStdString sNamedDir;
	CStdString sValue;

	sConfigureFile = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("mxm.xml") );
	// pMXMXML = xmlLoader.LoadXMLNodes( sConfigureFile ); // _T("D:\\mxm.xml") );
	bool bNoPref = false;
	bool bLocalOnly = false;
	bool bSetSkin = false;
	bool bLoadInternal = true;
	CStdString sPrefSkin;


	// Logic in loading desired skins:
	// Where to get skins path? In the near future, you'll be able to specify it in the preferences.
	// I'll add a multi-disk override as well so....
    // SkinsPath defaults to MXM executable path
	// 

	sSkinDir = g_MenuInfo.m_sMXMPath;
	if ( g_MenuInfo.m_bDVDMode || g_MenuInfo.m_bXDIMode )
	{
		sValue = g_MenuInfo.m_pCfgInfo->GetString( _T("Main"), _T("skinspath"), _T(""), true );
		if ( sValue.GetLength() == 0 )
		{
			if ( g_MenuInfo.m_pPrefInfo->GetBool( _T("skins"), _T("dvdusesprefs"), FALSE ) )
			{
				sSkinDir = g_MenuInfo.m_pPrefInfo->GetString( _T("skins"), _T("path"), sSkinDir, true );
			}
		}
		else
		{
			// Path specifically set... Relative to MXM path
			sSkinDir = MakeFullDirPath( g_MenuInfo.m_sMXMPath, sValue );
		}
		// Now, do we use internal skins?
		if ( g_MenuInfo.m_pCfgInfo->GetBool( _T("main"), _T("useprefskin"), FALSE ) == FALSE )
		{
			bNoPref = true;
			if ( g_MenuInfo.m_pCfgInfo->GetBool( _T("main"), _T("randomskins"), FALSE ) == FALSE )
			{
				bSetSkin = true;
				if ( g_MenuInfo.m_bXDIMode )
				{
					sPrefSkin = _T("XDI");
				}
			}

			if ( g_MenuInfo.m_pCfgInfo->GetBool( _T("main"), _T("dvdskinsonly"), TRUE ) == TRUE )
			{
				bLoadInternal = false;
				bLocalOnly=true;
				g_MenuInfo.m_sSkinsPath = _T("D:\\");
			}
		}
		sPrefSkin = g_MenuInfo.m_pCfgInfo->GetString( _T("main"), _T("loadskin"), _T(""), true );
	}
	else
	{
		sValue = g_MenuInfo.m_pCfgInfo->GetString( _T("Main"), _T("skinspath"), sSkinDir, true );
		sSkinDir = g_MenuInfo.m_pPrefInfo->GetString( _T("skins"), _T("path"), sValue, true );
		if ( g_MenuInfo.m_pPrefInfo->GetBool( _T("skins"), _T("useinternal"), TRUE ) )
		{
			bLoadInternal = true;
		}
		else
		{
			bLoadInternal = false;
		}
	}

	g_MenuInfo.m_sSkinsPath = sSkinDir;


	// g_SkinNodes
	// Load up internal skins...
	if ( g_MenuInfo.m_pInternalInfo && !bLocalOnly && bLoadInternal )
	{
		CXMLNode * pSkinsNode = g_MenuInfo.m_pInternalInfo->GetNode( _T("skins") );
		if ( pSkinsNode )
		{
			int iIndex = 0;

			while ( pSkinXML = pSkinsNode->GetNode( _T("skin"), iIndex++ ) )
			{
				if ( pSkinXML->GetNodeCount( _T("layout") ) )
				{
					g_SkinNodes.CopyInNode( *pSkinXML );
				}
			}
		}
	}

	if ( !(g_MenuInfo.SafeMode.m_dwFlags & SM_BASICSKIN ))
	{
		FindSkins ( saLocations, g_MenuInfo.m_sSkinsPath ); //  _T("D:\\") );

		iterDirs = saLocations.begin();
		while( iterDirs != saLocations.end() )
		{
			{
				CXMLLoader xmlSkinLoader(NULL,true);

				sSkinDir = StripFilenameFromPath( iterDirs->c_str() );
				sSkinName = ExtractFilenameFromPath(sSkinDir, false);

				pSkinXML = xmlSkinLoader.LoadXMLNodes( iterDirs->c_str() );

				// Valid "new" skins will have Layout Nodes.
				// This serves a dual purpose... skins can have both old and new definitions!
				// If we have a newer skin file, 
				if ( pSkinXML->GetNodeCount( _T("layout") ) )
				{
					CXMLNode * pSkinNode;
					CStdString sFoundSkinName;

					DumpNode( pSkinXML, 0 );
					pSkinNode = g_SkinNodes.CopyInNode( *pSkinXML, _T("skin"));
					if ( pSkinNode )
					{
						pSkinNode->SetString( NULL, _T("directory"), sSkinDir );
						DumpNode( pSkinNode, 0 );
						sFoundSkinName = pSkinNode->GetString( NULL, _T("name"), _T(""), true );
						if ( sFoundSkinName.GetLength() == 0 )
						{
							pSkinNode->SetString( NULL, _T("name"), sSkinName, true );
						}
					}
				}
	//			else
	//			{
	//				// BuildOldSkins( pMXMXML, pSkinXML, sSkinName, sSkinDir );
	//			}
			}
			iterDirs++;
		}
	}
	else
	{
		// We would load the internal "Basic Skin" here...
		CXMLNode * pSkinNode = g_MenuInfo.m_pInternalInfo->GetNode( _T("basicskin") );

		if ( pSkinNode )
		{
			if ( pSkinNode->GetNodeCount( _T("layout") ) )
			{
				g_SkinNodes.CopyInNode( *pSkinNode );
			}
		}
	}

	// If no skins were found, we MUST load the internal skins at this point...
	if( g_SkinNodes.GetNodeCount( _T("skin") ) == 0 )
	{
		CXMLNode * pSkinsNode = g_MenuInfo.m_pInternalInfo->GetNode( _T("skins") );
		if ( pSkinsNode )
		{
			int iIndex = 0;

			while ( pSkinXML = pSkinsNode->GetNode( _T("skin"), iIndex++ ) )
			{
				if ( pSkinXML->GetNodeCount( _T("layout") ) )
				{
					g_SkinNodes.CopyInNode( *pSkinXML );
				}
			}
		}
	}



	// OK, select our skin and mode on....
	{
		int iCount;
		int iSelectedSkin = -1;
		CXMLNode * pSkinNode = NULL;

		if ( !bNoPref )
		{
			sPrefSkin = g_MenuInfo.m_pPrefInfo->GetString( _T("UserSetting"), _T("Skin"), _T(""), true );

			// Force Random!
			if( _tcsicmp( sPrefSkin, _T("$-=random=-$") ) == 0 )
			{
				g_MenuInfo.m_sSelectedSkin = _T("");
				sPrefSkin = _T("");
			}
		}

		// If no skins at this point, build our internal one...
		//if( g_SkinNodes.GetNodeCount( _T("skin") ) == 0 )
		//{
		//	// At this point, build the internals, since we are missing ANY skins
		//	if ( g_MenuInfo.m_pInternalInfo && bLocalOnly )
		//	{
		//		CXMLNode * pSkinsNode = g_MenuInfo.m_pInternalInfo->GetNode( _T("skins") );
		//		if ( pSkinsNode )
		//		{
		//			int iIndex = 0;

		//			while ( pSkinXML = pSkinsNode->GetNode( _T("skin"), iIndex++ ) )
		//			{
		//				if ( pSkinXML->GetNodeCount( _T("layout") ) )
		//				{
		//					g_SkinNodes.CopyInNode( *pSkinXML );
		//				}
		//			}
		//		}
		//	}
		//	// Next condition should never happen!
		//	if( g_SkinNodes.GetNodeCount( _T("skin") ) == 0 )
		//	{
		//		BuildDefaultSkin();
		//	}
		//}

		iCount = g_SkinNodes.GetNodeCount( _T("skin") );

		sNamedDir = MakeFullFilePath( g_MenuInfo.m_sSkinsPath, g_MenuInfo.m_sSelectedSkin );

		m_matrix.SetStatusIdx(MSG_INIT_INITSKIN);

		if ( sPrefSkin.GetLength() )
		{
			if ( !SelectSkinByName( sPrefSkin ) )
			{
				bNoPref = true;
			}
		}
		else
		{
			bNoPref = true;
		}
		if ( bNoPref && !SelectSkinByName(g_MenuInfo.m_sSelectedSkin) )
		{
			// Select a random skin!
			if ( iCount )
			{
				iSelectedSkin = (RandomNum()%iCount);
				pSkinNode = g_SkinNodes.GetNode(_T("skin"), iSelectedSkin );
			}
			// Save off our skin node!
			g_MenuInfo.m_pSkinInfo = pSkinNode;
			// g_MenuInfo.m_pSkinInfo AddInternalScript(	, _T("MXM"), g_MenuInfo.m_sMXMPath
			MakeInfoScreens();
			g_GadgetSkin.Configure(pSkinNode);
		}




		// MakeInfoScreens( pSkinNode );

		// g_MenuInfo.m_pSkinInfo->SaveNode( _T("Z:\\SkinNode.xml") );


	}
	// Reset preference overrides...

	if ( g_MenuInfo.m_pSkinInfo )
	{
		CXMLNode * pNode = NULL;
		int iValue = 0;
		CStdString sSkinDir = g_MenuInfo.m_pSkinInfo->GetString( NULL, _T("directory"), g_MenuInfo.m_sMXMPath );

		if( pNode = g_MenuInfo.m_pSkinInfo->GetNode( _T("scripts") ) )
		{
			CXMLElement * pElement = NULL;
			CStdString sName, sScript;

			while( pElement = pNode->GetElement( _T("actionscript"), iValue++ ) )
			{
				sName = pElement->GetString( _T(""), _T("name"), true, true );
				sScript = pElement->GetString( _T("") );
				if( sScript.GetLength() && sName.GetLength() )
				{
					AddInternalScript( sName, sScript, _T("Skin"), sSkinDir );
				}
			}
			iValue = 0;
			CStdString sFilename;
			CStdString sScriptPath;
			while( pElement = pNode->GetElement( _T("actionscriptload"), iValue++ ) )
			{
				sName = pElement->GetString( _T(""), _T("name"), true, true );
				sFilename = MakeFullFilePath( sSkinDir, pElement->GetString( _T(""), NULL, true ) );
				sScriptPath = StripFilenameFromPath( sFilename );
				if( sName.GetLength() && FileExists(sFilename) )
				{
					sScript = LoadStringFromFile( sFilename );
					AddInternalScript( sName, sScript, _T("Skin"), sScriptPath );
				}
			}
		}
	}

}


CStdString CMXMApp::CheckBIOS( void )
{
	CStdString sReturn;
	CXBMD5 biosMD5;

	if ( biosMD5.Calculate( (LPBYTE)0xff000000, 0x100000 ) )
	{
		CXMLNode * pBIOSNode;
		CStdString sBIOSFile;

		if ( FileExists( _T("U:\\BIOSMD5.xml") ) )
		{
			sBIOSFile = _T("U:\\BIOSMD5.xml");
		}
		else
		{
			sBIOSFile = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("BIOSMD5.xml") );
		}

		g_MenuInfo.Xbox.m_sBIOSMD5.Format( _T("0x%s"), biosMD5.GetMD5String());
		g_MenuInfo.Xbox.m_sBIOSName = _T("Unknown");
		// Now figure out BIOS string
		CXMLLoader xmlBiosLoader(NULL,true);
		
		pBIOSNode = xmlBiosLoader.LoadXMLNodes( sBIOSFile );
		if ( pBIOSNode )
		{
			// scan elements to find match for our ROM
			int iIndex = 0;
			CXMLElement * pElement;

			while( pElement = pBIOSNode->GetElement( _T("rom"), iIndex++ ) )
			{
				if ( _tcsicmp( pElement->m_sValue, g_MenuInfo.Xbox.m_sBIOSMD5 ) == 0 )
				{
					sReturn = pElement->GetString( _T(""), _T("descr"), true, false );
					break;
				}
			}
			// AUTODELETED by loader delete pBIOSNode;
		}
	}
	else
	{
		sReturn = _T("[MD5 Calc Error]");
	}
	return sReturn;
}

bool CMXMApp::InitializeXDIConfig(void)
{
	bool bReturn = false;

	g_MenuInfo.UpdateSafeMode( _T("013: Initiating XDI Mode") );

	// First, make damn sure the XDI will load up properly!
	// If there are no items, why bother?
	CXMLNode * pXDINode = NULL;
	pXDINode = GetXDI();
	if ( pXDINode && pXDINode->GetNodeCount(_T("Item")) )
	{
		// XDI mode!
		g_MenuInfo.m_bXDIMode = true;

		// Assume D:\ is root directory, we are running from DVD, 
		// not hard drive in this mode!
		g_MenuInfo.m_sMXMPath = _T("D:\\");

		g_MenuInfo.m_sTitle = _T("XDI Mode!"); // c_szEmpty;
		g_MenuInfo.m_sSubTitle = _T("Running in MenuX compatibility Mode"); // c_szEmpty;
		g_MenuInfo.m_sExitApp = c_szEmpty;
		g_MenuInfo.m_sExitAppParams = c_szEmpty;

		BuildXDISkin();

		g_MenuInfo.m_sSelectedSkin = c_szEmpty;

		g_MenuInfo.m_sResult = _T("SUCCEEDED");

		g_MenuInfo.m_iStyle = -1;
		g_MenuInfo.m_bTestMode = FALSE;

		g_MenuInfo.m_bAllowVideoSound = TRUE;

		g_MenuInfo.m_lLoadDelay = 1500;
		g_MenuInfo.m_lTitleDelay = 0;				// No title sacreen in XDI mode
		g_MenuInfo.m_lSaverDelay = 300 * 1000;		// 5 Minute delay until screen saver

		g_MenuInfo.m_dwMoveDelay = 220;
		g_MenuInfo.m_dwRepeatDelay = 220;

		g_MenuInfo.m_bCircularMenus = FALSE;

		g_MenuInfo.m_bAllowSaverMusic = TRUE;
		g_MenuInfo.m_bAllowSaverSound = TRUE;
		g_MenuInfo.m_bMenuOrientedUpDown = TRUE;

		if ( !( g_MenuInfo.SafeMode.m_dwFlags & SM_NOMUSIC ) )
		{
//			if ( g_MenuInfo.m_bUseMusicManager )

//			g_MenuInfo.m_bUseMusicManager = TRUE;
		}
		else
		{
			g_MenuInfo.m_bUseMusicManager = FALSE;
		}

		g_MenuInfo.m_bGlobalMusic = FALSE;

		g_MenuInfo.m_bRandomMusic = FALSE;
		// g_MenuInfo.m_sMusicDirectory = c_szEmpty; // iniFile.GetString( _T("Music"), _T("Directory"), _T("MUSIC") )
		g_MenuInfo.m_saMusicDirectories.clear();
		g_MenuInfo.m_saMusicFiles.push_back(_T("D:\\menux\\media\\ambient.wma"));
		// g_MenuInfo.m_sAmbientTrack =  _T("D:\\menux\\media\\ambient.wma");

		// Save it off...
		g_MenuInfo.m_pCfgInfo = pXDINode;
		// delete pXDINode;
	}
	else
	{
		// Doh! What happened???
		g_MenuInfo.m_bXDIMode = false;
	}

	return g_MenuInfo.m_bXDIMode;
}


void CMXMApp::ConfigureFromMXM( void )
{
	CStdString sConfigure;
	CXMLLoader xmlCfgLoader(NULL,true);
	CStdString sTemp;
	int iValue;


	// Create full path name
	sConfigure = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("mxm.xml") );
	
	// Load XML tree from file
	g_MenuInfo.m_pCfgInfo = xmlCfgLoader.LoadXMLNodes( sConfigure );

	// Detach, so we aren't invalid as soon as the loader object is destroyed
	xmlCfgLoader.DetachMainNode();


	{
		// Set up internal scripts from MXM.xml
		iValue = 0;

		CXMLNode * pNode = NULL;
		if( pNode = g_MenuInfo.m_pCfgInfo->GetNode( _T("scripts") ) )
		{
			CXMLElement * pElement = NULL;
			CStdString sName, sScript;

			while( pElement = pNode->GetElement( _T("actionscript"), iValue++ ) )
			{
				sName = pElement->GetString( _T(""), _T("name"), true, true );
				sScript = pElement->GetString( _T("") );
				if( sScript.GetLength() && sName.GetLength() )
				{
					AddInternalScript( sName, sScript, _T("MXM"), g_MenuInfo.m_sMXMPath );
				}
			}
			iValue = 0;
			CStdString sFilename;
			CStdString sScriptPath;
			while( pElement = pNode->GetElement( _T("actionscriptload"), iValue++ ) )
			{
				sName = pElement->GetString( _T(""), _T("name"), true, true );
				sFilename = MakeFullFilePath( g_MenuInfo.m_sMXMPath, pElement->GetString( _T(""), NULL, true ) );
				sScriptPath = StripFilenameFromPath( sFilename );
				if( sName.GetLength() && FileExists(sFilename) )
				{
					sScript = LoadStringFromFile( sFilename );
					AddInternalScript( sName, sScript, _T("MXM"), sScriptPath );
				}
			}
		}
	}




	// Load network settings from MXM.xml
	if ( g_MenuInfo.m_pCfgInfo->GetString( _T("network"), _T("usedhcp"), _T(""), true ).GetLength() )
	{
		if ( g_MenuInfo.m_pCfgInfo->GetBool( _T("network"), _T("usedhcp"), TRUE ) )
		{
			g_MenuInfo.NetParams.m_iUseDHCP = 1;
		}
		else
		{
			g_MenuInfo.NetParams.m_iUseDHCP = 0;
		}
	}
	else
	{
		g_MenuInfo.NetParams.m_iUseDHCP = -1;
	}
	g_MenuInfo.NetParams.m_sIP = g_MenuInfo.m_pCfgInfo->GetString( _T("network"), _T("ip"), _T(""), true );
	g_MenuInfo.NetParams.m_sSubnetmask = g_MenuInfo.m_pCfgInfo->GetString( _T("network"), _T("subnetmask"), _T(""), true );
	g_MenuInfo.NetParams.m_sDefaultgateway = g_MenuInfo.m_pCfgInfo->GetString( _T("network"), _T("defaultgateway"), _T(""), true );
	g_MenuInfo.NetParams.m_sDNS1 = g_MenuInfo.m_pCfgInfo->GetString( _T("network"), _T("dns1"), _T(""), true );
	g_MenuInfo.NetParams.m_sDNS2 = g_MenuInfo.m_pCfgInfo->GetString( _T("network"), _T("dns2"), _T(""), true );

	g_MenuInfo.m_sTitle = g_MenuInfo.m_pCfgInfo->GetString( _T("Main"), _T("DiscTitle"), _T("MediaX Menu"), true );
	g_MenuInfo.m_sSubTitle = g_MenuInfo.m_pCfgInfo->GetString( _T("Main"), _T("DiscSubTitle"), _T(" "), true );
//	g_MenuInfo.m_sSubTitle = _T(" ");

	// Set FTPServer thread priority here.
	sTemp =  g_MenuInfo.m_pCfgInfo->GetString( _T("preferences"), _T("ftppriority"), _T("abovenormal"), true );
	sTemp.MakeLower();
	if ( sTemp.Compare( _T("abovenormal") ) == 0 )
	{
		iValue = 9;
	}
	else if ( sTemp.Compare( _T("belownormal") ) == 0 )
	{
		iValue = 7;
	}
	else if ( sTemp.Compare( _T("high") ) == 0 )
	{
		iValue = 10;
	}
	else if ( sTemp.Compare( _T("highest") ) == 0 )
	{
		iValue = 11;
	}
	else if ( sTemp.Compare( _T("low") ) == 0 )
	{
		iValue = 6;
	}
	else if ( sTemp.Compare( _T("lowest") ) == 0 )
	{
		iValue = 5;
	}
	else
	{
		iValue = 8;
	}
	g_MenuInfo.m_iFTPPriority = iValue;

	// These settings pertain to loading DVD/CDs when the tray is closed
	g_MenuInfo.m_bDeepMultiSearch = (g_MenuInfo.m_pCfgInfo->GetBool( _T("Preferences"), _T("DeepMultiSearch"), FALSE ) == TRUE );
	g_MenuInfo.m_bAutoLoadMulti = (g_MenuInfo.m_pCfgInfo->GetBool( _T("Preferences"), _T("AutoLoadMulti"), TRUE ) == TRUE);
	g_MenuInfo.m_bAutoLaunchGame = (g_MenuInfo.m_pCfgInfo->GetBool( _T("Preferences"), _T("AutoLaunchGame"), FALSE ) == TRUE);
	g_MenuInfo.m_bAutoLaunchMovie = (g_MenuInfo.m_pCfgInfo->GetBool( _T("Preferences"), _T("AutoLaunchMovie"), FALSE ) == TRUE);

	// Set up the default player (will auto search one, though, if none identified)
	g_MenuInfo.m_sDVDPlayer = g_MenuInfo.m_pCfgInfo->GetString( _T("Preferences"), _T("DVDPlayer"), _T(""), true );
	g_MenuInfo.m_sDVDPlayerCmdLine = g_MenuInfo.m_pCfgInfo->GetString( _T("Preferences"), _T("DVDPlayerCmdLine"), _T(""), true );

	// Set the Time Server IP
	g_MenuInfo.m_sTimeServerIP = g_MenuInfo.m_pCfgInfo->GetString( _T("Preferences"), _T("TimeServerIP"), _T(""), true );
	if ( g_MenuInfo.m_sTimeServerIP.GetLength() )
	{
		g_MenuInfo.m_bDoTimeServer = (g_MenuInfo.m_pCfgInfo->GetBool( _T("Preferences"), _T("SynchronizeClock"), TRUE ) == TRUE);
	}
	else
	{
		g_MenuInfo.m_bDoTimeServer = false;
	}

	// The user can set their nickname here...
	sTemp = g_MenuInfo.m_pCfgInfo->GetString( _T("Preferences"), _T("Nickname"), c_szEmpty, true );
	{
		WCHAR wszNickname[MAX_NICKNAME+1];
		HANDLE hHandle;
		wszNickname[MAX_NICKNAME] = 0;
		CStdStringW sTempW;

		if ( sTemp.GetLength() )
		{
			if ( sTemp.GetLength() > MAX_NICKNAME )
			{
				sTemp = sTemp.Left(MAX_NICKNAME);
			}
			sTempW = sTemp;
			g_MenuInfo.UpdateSafeMode( _T("014: Setting Nickame") );
			XSetNickname( sTempW, TRUE );
		}

		wszNickname[MAX_NICKNAME] = 0;

		hHandle = XFindFirstNickname( FALSE, wszNickname, MAX_NICKNAME );
		if ( hHandle != INVALID_HANDLE_VALUE )
		{
			g_MenuInfo.Xbox.m_sNickName = wszNickname;
		}
		CloseHandle( hHandle );
	}

	// MXM.xml sound overrides.
	// Don't know how many of these I want to keep yet.
	// Prefs can come in and override these.
	g_MenuInfo.Overrides.m_bForceGlobalMusic = (g_MenuInfo.m_pCfgInfo->GetBool( _T("overrides"), _T("ForceGlobalMusic"), FALSE ) == TRUE);
	g_MenuInfo.Overrides.m_bForceLocalMusic = (g_MenuInfo.m_pCfgInfo->GetBool( _T("overrides"), _T("ForceLocalMusic"), FALSE ) == TRUE);
	g_MenuInfo.Overrides.m_bForceRandomMusic = (g_MenuInfo.m_pCfgInfo->GetBool( _T("overrides"), _T("ForceRandomMusic"), FALSE ) == TRUE);

	// These settings determine how the menu is built.
	if ( !g_MenuInfo.m_pPrefInfo->HasValue( _T("autocfg"), _T("removeinvalid") ) )
	{
		g_MenuInfo.m_bRemoveInvalid = g_MenuInfo.m_pCfgInfo->GetBool( _T("Main"), _T("RemoveInvalid"), g_MenuInfo.m_bRemoveInvalid );
	}
	if ( !g_MenuInfo.m_pPrefInfo->HasValue( _T("autocfg"), _T("removeduplicates") ) )
	{
		g_MenuInfo.m_bRemoveDuplicates = g_MenuInfo.m_pCfgInfo->GetBool( _T("Main"), _T("RemoveDuplicates"), g_MenuInfo.m_bRemoveDuplicates );
	}
		
	// "Exit Application"
	g_MenuInfo.m_sExitApp = g_MenuInfo.m_pCfgInfo->GetString( _T("Main"), _T("ExitApp"), c_szEmpty, true );
	g_MenuInfo.m_sExitAppParams = g_MenuInfo.m_pCfgInfo->GetString( _T("Main"), _T("ExitAppParams"), c_szEmpty, true );

	// Path to locate skins
	g_MenuInfo.m_sSkinsPath = g_MenuInfo.m_pCfgInfo->GetString( _T("Main"), _T("SkinsPath"), g_MenuInfo.m_sMXMPath, true );
	g_MenuInfo.m_sSkinsPath = MakeFullDirPath( g_MenuInfo.m_sMXMPath, g_MenuInfo.m_sSkinsPath );

	// Selected skin from MXM.xml (prefs can override this)
	g_MenuInfo.m_sSelectedSkin = g_MenuInfo.m_pCfgInfo->GetString( _T("Main"), _T("LoadSkin"), c_szEmpty, true );

	// Were we actually loaded form an MXM.xml file?
	g_MenuInfo.m_sResult = g_MenuInfo.m_pCfgInfo->GetString( _T("LOAD"), _T("RESULT"), _T("SUCCEEDED"), true );

	// ?????
	g_MenuInfo.m_iStyle = g_MenuInfo.m_pCfgInfo->GetLong( _T("Main"), _T("Style"), -1 );
	g_MenuInfo.m_bTestMode = g_MenuInfo.m_pCfgInfo->GetBool( _T("Main"), _T("TestMode"), FALSE );

	// Is this used still?
	g_MenuInfo.m_bAllowVideoSound = g_MenuInfo.m_pCfgInfo->GetBool( _T("Main"), _T("AllowVideoSound"), TRUE );

	// Set delays.... skin and prefs can override these
	g_MenuInfo.m_lLoadDelay = 1500;
	g_MenuInfo.m_lTitleDelay = g_MenuInfo.m_pCfgInfo->GetLong( _T("Main"), _T("TitleDelay"), 20 ) * 1000;
	g_MenuInfo.m_lSaverDelay = g_MenuInfo.m_pCfgInfo->GetLong( _T("Main"), _T("SaverDelay"), 300 ) * 1000;
	if ( g_MenuInfo.m_lSaverDelay && g_MenuInfo.m_lSaverDelay<30000 )
	{
		g_MenuInfo.m_lSaverDelay = 30000;
	}

	// Input sensitivity settings
	g_MenuInfo.m_dwMoveDelay = g_MenuInfo.m_pCfgInfo->GetLong( _T("Main"), _T("MoveDelay"), 220 );
	g_MenuInfo.m_dwRepeatDelay = g_MenuInfo.m_pCfgInfo->GetLong( _T("Main"), _T("RepeatDelay"), 220 );

	// Skin setting. Removed
	// g_MenuInfo.m_bCircularMenus = g_MenuInfo.m_pCfgInfo->GetBool( _T("Menu"), _T("CircularSelection"), FALSE );

	// Skin settings.
	// Removed
	//g_MenuInfo.m_bAllowSaverMusic = g_MenuInfo.m_pCfgInfo->GetBool( _T("Menu"), _T("AllowSaverMusic"), TRUE );
	//g_MenuInfo.m_bAllowSaverSound = g_MenuInfo.m_pCfgInfo->GetBool( _T("Menu"), _T("AllowSaverSound"), TRUE );
	g_MenuInfo.m_bAllowSaverMusic = TRUE;
	g_MenuInfo.m_bAllowSaverSound = TRUE;

	// Skin setting. Removed
	// g_MenuInfo.m_bMenuOrientedUpDown = g_MenuInfo.m_pCfgInfo->GetBool( _T("Menu"), _T("VerticalSelection"), TRUE );
	g_MenuInfo.m_bMenuOrientedUpDown = TRUE;

	if ( !( g_MenuInfo.SafeMode.m_dwFlags & SM_NOMUSIC ) )
	{
		g_MenuInfo.m_bUseMusicManager = g_MenuInfo.m_pCfgInfo->GetBool( _T("Music"), _T("UseMusic"), g_MenuInfo.m_bUseMusicManager );
	}
	else
	{
		g_MenuInfo.m_bUseMusicManager = FALSE;
	}

	g_MenuInfo.m_bGlobalMusic = g_MenuInfo.m_pCfgInfo->GetBool( _T("Music"), _T("Global"), TRUE );
//	sValue.Format( _T("Global Music: %s"), g_MenuInfo.m_bGlobalMusic?_T("ON"):_T("OFF") );
//	DEBUG_LINE( sValue.c_str() );
	g_MenuInfo.m_bRandomMusic = g_MenuInfo.m_pCfgInfo->GetBool( _T("Music"), _T("Random"), FALSE );
//	sValue.Format( _T("Random Music: %s"), g_MenuInfo.m_bRandomMusic?_T("ON"):_T("OFF") );
//	DEBUG_LINE( sValue.c_str() );
	// g_MenuInfo.m_sMusicDirectory = g_MenuInfo.m_pCfgInfo->GetString( _T("Music"), _T("Directory"), _T("MUSIC") );

	// Should loop through these possible elements/nodes to get values here.
	sTemp = g_MenuInfo.m_pCfgInfo->GetString( _T("Music"), _T("Directory"), _T("MUSIC"), true );
	g_MenuInfo.m_saMusicDirectories.push_back( MakeFullDirPath( g_MenuInfo.m_sMXMPath, sTemp ) );

	sTemp = g_MenuInfo.m_pCfgInfo->GetString( _T("Music"), _T("Background"), c_szEmpty, true );
	g_MenuInfo.m_saMusicFiles.push_back( MakeFullFilePath( g_MenuInfo.m_sMXMPath, sTemp ) );
		
	// iItemCount = g_MenuInfo.m_pCfgInfo->GetLong( _T("Main"), _T("ItemCount"), -1 );

	{
		CStdString sKey;
		int iHelpScreen = 1;
		sKey.Format( _T("Screen_%02d"), iHelpScreen );
		sTemp = g_MenuInfo.m_pCfgInfo->GetString( _T("Help"), sKey, c_szEmpty, true );
		while( sTemp.GetLength() )
		{
//			sTempT.Format( _T("Adding Main Help: %s %d"),sValue.c_str(), iHelpScreen );
//			DEBUG_LINE( sTempT );
			iHelpScreen++;
			sKey.Format( _T("Screen_%02d"), iHelpScreen );

			// "Specials"
			if (( sTemp.Compare( _T("HELPGC") ) == 0 ) ||
				( sTemp.Compare( _T("HELPIR") ) == 0 ) ||
				( sTemp.Compare( _T("HELPKB") ) == 0 ) )
			{
				g_MenuInfo.m_saMainHelp.push_back( sTemp );
				sTemp = g_MenuInfo.m_pCfgInfo->GetString( _T("Help"), sKey, c_szEmpty, true );
			}
			else
			{
				// Make full path...
				sTemp = MakeFullFilePath( g_MenuInfo.m_sMXMPath, sTemp ); // _T("D:\\"), sValue );
				if ( FileExists(sTemp) )
				{
					g_MenuInfo.m_saMainHelp.push_back( sTemp );
					sTemp = g_MenuInfo.m_pCfgInfo->GetString( _T("Help"), sKey, c_szEmpty, true );
				}
				else
				{
					sTemp = c_szEmpty;
					DEBUG_LINE( _T("FAILED HELP LOAD") );
				}
			}
		}
	}

	PerfGetPerformance( _T("MXM.xml"), FALSE );

	g_MenuInfo.m_bAllowAutoConfig = g_MenuInfo.m_pCfgInfo->GetBool( _T("Main"), _T("AllowAutoConfig"), TRUE );
	g_MenuInfo.m_bAllowAutoTitle = g_MenuInfo.m_pCfgInfo->GetBool( _T("Main"), _T("AllowAutoTitle"), TRUE );
	g_MenuInfo.m_bAllowAutoDescr = g_MenuInfo.m_pCfgInfo->GetBool( _T("Main"), _T("AllowAutoDescr"), TRUE );


//	sValue.Format( _T("Number of MXM.xml items: %d"), iItemCount );
//	DEBUG_LINE( sValue.c_str() );
//	g_MenuInfo.m_iNumItems = g_MenuInfo.GetItemCount(); // iItemCount;

}

void CMXMApp::ConfigureFromPrefs( void )
{
	int iValue;
	CStdString sTemp;

	g_MenuInfo.LoadPrefs();

	if ( !( g_MenuInfo.SafeMode.m_dwFlags & SM_NOMUSIC ) )
	{
		g_MenuInfo.m_bUseMusicManager = g_MenuInfo.m_pPrefInfo->GetBool( _T("audio"), _T("enablemusic"), g_MenuInfo.m_bUseMusicManager );
	}
	else
	{
		g_MenuInfo.m_bUseMusicManager = FALSE;
	}


	g_MenuInfo.m_bRemoveInvalid = g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("removeinvalid"), TRUE );
	g_MenuInfo.m_bRemoveDuplicates = g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("removeduplicates"), TRUE );

	g_MenuInfo.m_bUseCelsius = (g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("temptype"), (g_MenuInfo.m_bUseCelsius?1:0) ) == 1);
	g_MenuInfo.m_bUseEuroNumbers = ( g_MenuInfo.m_pPrefInfo->GetBool( _T("local"), _T("euronumbers"), g_MenuInfo.m_bUseEuroNumbers?TRUE:FALSE ) == TRUE);

	g_MenuInfo.m_iDateFormat = g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("dateformat"), g_MenuInfo.m_iDateFormat );
	g_MenuInfo.m_iTimeFormat = g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("timeformat"), g_MenuInfo.m_iDateFormat );

	if ( g_MenuInfo.m_pPrefInfo->GetString( _T("local"), _T("datesep"), _T(""), true ).GetLength() )
	{
		sTemp  = g_szDateSeparator[g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("datesep"), 0 )];
		sTemp.Trim();
		if ( sTemp.GetLength() )
		{
			g_MenuInfo.m_tcDateSepChar = sTemp[0];
		}
	}
	else
	{
		g_MenuInfo.m_tcDateSepChar = _T('/');
//		sTempT = pMainNode->GetString( _T("Preferences"), _T("DateSepChar"), _T("/") );
//		sTempT.Trim();
//		if ( sTempT.GetLength() )
//		{
//			g_MenuInfo.m_tcDateSepChar = sTempT[0];
//		}
	}

	g_MenuInfo.m_bDoTimeServer = (g_MenuInfo.m_pPrefInfo->GetBool( _T("network"), _T("synchtime"), g_MenuInfo.m_bDoTimeServer?TRUE:FALSE ) == TRUE);
	g_MenuInfo.m_lTimeSynchOffset = g_MenuInfo.m_pPrefInfo->GetLong( _T("network"), _T("timesynchoffset"), 0 );


	if ( g_MenuInfo.m_pPrefInfo->GetString( _T("local"), _T("timesep"), _T(""), true ).GetLength() )
	{
		sTemp  = g_szTimeSeparator[g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("timesep"), 0 )];
		sTemp.Trim();
		if ( sTemp.GetLength() )
		{
			g_MenuInfo.m_tcTimeSepChar = sTemp[0];
		}
	}
	else
	{
		g_MenuInfo.m_tcTimeSepChar = _T(':');
//		sTempT = pMainNode->GetString( _T("Preferences"), _T("TimeSepChar"), _T(":") );
//		sTempT.Trim();
//		if ( sTempT.GetLength() )
//		{
//			g_MenuInfo.m_tcTimeSepChar = sTempT[0];
//		}
	}
	
	g_MenuInfo.m_iFTPPriority = g_MenuInfo.m_pPrefInfo->GetLong( _T("network"), _T("ftppriority"), g_MenuInfo.m_iFTPPriority );

	g_MenuInfo.m_bDeepMultiSearch = (g_MenuInfo.m_pPrefInfo->GetBool( _T("dvddrive"), _T("deepsearch"), g_MenuInfo.m_bDeepMultiSearch?TRUE:FALSE ) == TRUE );
	g_MenuInfo.m_bAutoLoadMulti = (g_MenuInfo.m_pPrefInfo->GetBool( _T("dvddrive"), _T("automulti"), g_MenuInfo.m_bAutoLoadMulti?TRUE:FALSE ) == TRUE);
	g_MenuInfo.m_bAutoLaunchGame = (g_MenuInfo.m_pPrefInfo->GetBool( _T("dvddrive"), _T("launchgame"), g_MenuInfo.m_bAutoLaunchGame?TRUE:FALSE ) == TRUE);
	g_MenuInfo.m_bAutoLaunchMovie = (g_MenuInfo.m_pPrefInfo->GetBool( _T("dvddrive"), _T("launchmovie"), g_MenuInfo.m_bAutoLaunchMovie?TRUE:FALSE ) == TRUE);

	iValue = g_MenuInfo.m_pPrefInfo->GetLong( _T("music"), _T("OvMusDurVid"), 0 );
	if ( iValue )
	{
		if ( iValue == 1 ) // Yes 
		{
			g_MenuInfo.Overrides.m_iPreviewSuspendMusic = -1;
		}
		else
		{
			g_MenuInfo.Overrides.m_iPreviewSuspendMusic = 1;
		}
	}
	else
	{
		g_MenuInfo.Overrides.m_iPreviewSuspendMusic = 0;
	}


	iValue = g_MenuInfo.m_pPrefInfo->GetLong( _T("audio"), _T("OvPrevAudio"), 0 );
	if ( iValue )
	{
		if ( iValue == 1 )
		{
			g_MenuInfo.Overrides.m_iPreviewEnableSound = 1;
		}
		else
		{
			g_MenuInfo.Overrides.m_iPreviewEnableSound = -1;
		}
	}
	else
	{
		g_MenuInfo.Overrides.m_iPreviewEnableSound = 0;
	}
	g_MenuInfo.m_bUseSkinMenuSounds = (g_MenuInfo.m_pPrefInfo->GetLong( _T("audio"), _T("OvMenuSource"), 0 ) == 0 );

	g_MenuInfo.m_iAutoCfgStyle = g_MenuInfo.m_pPrefInfo->GetLong( _T("autocfg"), _T("cfgstyle"), 0 );
	g_MenuInfo.m_bAutoCfgAddDash = ( g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("adddash"), TRUE ) == TRUE );
	g_MenuInfo.m_bAutoCfgSort = ( g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("sort"), TRUE ) == TRUE );

	iValue = g_MenuInfo.m_pPrefInfo->GetLong( _T("music"), _T("OvMusGlobal"), 0 );
	if ( iValue )
	{
		if ( iValue == 1 )
		{
			g_MenuInfo.Overrides.m_bForceGlobalMusic = true;
			g_MenuInfo.Overrides.m_bForceLocalMusic = false;
		}
		else
		{
			g_MenuInfo.Overrides.m_bForceLocalMusic = true;
			g_MenuInfo.Overrides.m_bForceGlobalMusic = false;
		}
	}

	iValue = g_MenuInfo.m_pPrefInfo->GetLong( _T("music"), _T("OvMusRandom"), 0 );

	if ( iValue )
	{
		if ( iValue == 1 )
		{
			g_MenuInfo.Overrides.m_bForceRandomMusic = true;
		}
		else
		{
			g_MenuInfo.Overrides.m_bForceRandomMusic = false;
		}
	}

}

bool CMXMApp::InitializeHDConfig(void)
{
	bool bReturn = false;

	// Initialize with config
	g_MenuInfo.UpdateSafeMode( _T("013: Initiating DVD Launcher") );

	ConfigureFromMXM();
	ConfigureFromPrefs();
	bReturn = true;

	return bReturn;
}

bool CMXMApp::InitializeDVDRConfig(void)
{
	bool bReturn = false;

	// Initialize with config
	g_MenuInfo.UpdateSafeMode( _T("013: Initiating HD Launcher/Dash") );

	ConfigureFromMXM();
	ConfigureFromPrefs();

	bReturn = true;

	return bReturn;
}


void CMXMApp::FinishIntialization()
{
	CStdString sDebug;
	CStdStringW sTemp;
	CStdString sTempT;
	CStdString sValue;
	CStdString sEntryDir;
	CStdString sEntryExe;
	CStdString sConfigure;
//	int iItemCount;

	m_matrix.SetStatusIdx(MSG_INIT_BIOS);

	g_MenuInfo.Xbox.m_sBIOSName = CheckBIOS();

	m_matrix.SetStatusIdx(MSG_INIT_SYSTEM);

	g_MenuInfo.m_bSpecialMXM = false;
	g_MenuInfo.m_bSpecialLaunch = false;
	g_MenuInfo.m_sSpecialLaunch = c_szEmpty;
	g_MenuInfo.m_sSpecialLaunchData = c_szEmpty;
	g_MenuInfo.m_bGlobalHelp = true;

	// Relink the hard drive partitions, 
	// in case our launcher failed to do this properly.
	DEBUG_LINE( _T("\r\n=============Begin MXM Initialization========") );

	// Special. "Q" is our "D" until we decide otherwise
	RebuildSymbolicLink( _T(DriveQ), _T(CdRom) );

	// Make sure these are linked in
	RebuildSymbolicLink( _T(DriveS), _T(SDrive) );


	RebuildSymbolicLink( _T(DriveT), _T(TDrive) );
	RebuildSymbolicLink( _T(DriveU), _T(UDrive) );

	RebuildSymbolicLink( _T(DriveV), _T(VDrive) );

//	TCHAR szVolumeName[256];
//	DWORD dwMaxCompLen, dwFileSys;
//	DWORD dwVolSerialC;
//	DWORD dwVolSerialE;
//	DWORD dwVolSerialF;
//	TCHAR szFileSysName[64];
//	BOOL bResult;
	TCHAR tcDrive = _T('D');
	CStdString sRealPath;

//	bResult = GetVolumeInformation( _T("C:\\"), szVolumeName, 256, &dwVolSerialC, &dwMaxCompLen, &dwFileSys, szFileSysName, 64 );
//	bResult = GetVolumeInformation( _T("E:\\"), szVolumeName, 256, &dwVolSerialE, &dwMaxCompLen, &dwFileSys, szFileSysName, 64 );
//	bResult = GetVolumeInformation( _T("F:\\"), szVolumeName, 256, &dwVolSerialF, &dwMaxCompLen, &dwFileSys, szFileSysName, 64 );



	//{
	//	CURI uriTest1( _T("http://me:here@someplace.com/test/dir/something.html?what=44&something=2") );
	//	sValue = uriTest1.GetMerged();
	//	CURI uriTest2( _T("http://me@someplace.com?what=44&something=2") );
	//	sValue = uriTest2.GetMerged();
	//	CURI uriTest3( _T("http://216.239.37.104/search?q=cache:nyUjarPsh1oJ:lxr.gingerall.cz/sablot/source/src/engine/uri.cpp%3Fv%3DSABLOT_0_90+splituri++source+code&hl=en&ie=UTF-8"));
	//	sValue = uriTest3.GetMerged();
	//	CURI uriTest4( _T("http://www.google.com/search?num=100&hl=en&lr=&ie=UTF-8&oe=UTF-8&safe=off&q=split+uri++source+code+c%2B%2B"));
	//	sValue = uriTest4.GetMerged();
	//	CURI uriTest5( _T("http://www.mail-archive.com/perl6-all@perl.org/msg10258.html"));
	//	sValue = uriTest5.GetMerged();
	//}

	sValue = _T("");
	PerfGetPerformance( _T("BuildSymbolics"), FALSE );
	
//	sValue = GetDiskFree('c', 1 );
//	sValue = GetDiskFree('e', 1 );
//	sValue = GetDiskFree('f', 1 );

	// Create Internal XMLTree used for internal skins, dialogs and whatnot
	
	int * piBinTree = &g_iBtTreeInternal[0];

	g_MenuInfo.m_pInternalInfo = BuildBinTreeNode( g_szBtTknsInternal, &piBinTree, NULL, true );

//	if ( g_MenuInfo.m_pInternalInfo )
//	{
//		g_MenuInfo.m_pInternalInfo->SaveNode( _T("Z:\\Internal.xml") );
//	}


	// Load directories to avoid re-scanning for these later
//	ScanDirectories();

//	PerfGetPerformance( _T("ScanDirs"), FALSE );


	/*

	Initialization and "XDI" mode

	If MXM.xml does not exist, and D:\MenuX\menu.xdi DOES exist, we go into "xdi mode"
	which means we have a specific skin to default to and load our items from the xdi
	file.

	In xdi mode:
	  If skins exist, they will potentially be loaded up.
	  If MXM_Entry.xml files exist, their settings will override the xdi entry for
	  the corresponding menu item.
	  There is no help, unless an MXM_Entry.xml file is found to define one

    XDI mode sucks, except that it allows MXM to replace MenuX without a fuss.
	and still works a LOT better than MenuX.... but the PREFERRED method is to use
	the 'native' mode with regular XMl config files and skins.


	*/
	m_matrix.SetStatusIdx(MSG_INIT_CONFIG);

	g_MenuInfo.m_bXDIMode = false;

	sConfigure = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("mxm.xml") );

	bool bMXMExists = FileExists( sConfigure )?true:false; // _T("D:\\MXM.xml") )?true:false;

	g_MenuInfo.UpdateSafeMode( _T("011: Load Preferences") );

	g_MenuInfo.LoadPrefs();

	if ( 0 ) // g_MenuInfo.m_pPrefInfo->GetLong( _T("local"), _T("tvtype"), 0 ) )
	{
		if ( IsNTSC() )
		{
			if ( g_MenuInfo.m_pPrefInfo->GetBool( _T("local"), _T("ntsc"), IsNTSC()) == FALSE )
			{
				g_MenuInfo.UpdateSafeMode( _T("012: Switching to PAL") );
				SetNTSC(FALSE);
			}
		}
		else
		{
			if ( g_MenuInfo.m_pPrefInfo->GetBool( _T("local"), _T("ntsc"), IsNTSC() ))
			{
				g_MenuInfo.UpdateSafeMode( _T("012: Switching to NTSC") );
				SetNTSC(TRUE);
			}
		}
	}

	bool bConfigLoaded = false;

	g_MenuInfo.m_sTitle = _T("MediaX Menu");
	g_MenuInfo.m_sSubTitle = _T(" ");
	g_MenuInfo.m_bSafeMode = FALSE;

	if ( g_MenuInfo.m_bDVDMode )
	{
		// Check for XDI mode
		if ( FileExists(_T("D:\\MenuX\\menu.xdi")) && !bMXMExists )
		{
			bConfigLoaded = InitializeXDIConfig();
		}
		if ( !bConfigLoaded )
		{
			if ( !bMXMExists )
			{
				g_MenuInfo.m_bSafeMode = FALSE;
			}

			// DVD-R mode! Loads MXM.xml
			bConfigLoaded = InitializeDVDRConfig();
		}
	}
	else
	{
		if ( !bMXMExists )
		{
			g_MenuInfo.m_bSafeMode = TRUE;
		}
		bConfigLoaded = InitializeHDConfig();
	}

	BuildContextMenus();

	// Last gasp? What to do here? Nothing?
	if ( !bConfigLoaded )
	{
	}

	if ( !( g_MenuInfo.SafeMode.m_dwFlags & SM_NONETWORK ) &&
		g_MenuInfo.m_pPrefInfo->GetBool( _T("network"), _T("enablenetwork"), TRUE ) )
	{
		// Save off our configuration file
		if ( bMXMExists )
		{

			// NewsFeed...
			CXMLNode * pNewsFeed;
			int iIndex = 0;

			while( pNewsFeed = g_MenuInfo.m_pCfgInfo->GetNode( _T("newsfeed"), iIndex++ ) )
			{
				g_MenuInfo.UpdateSafeMode( _T("015: Adding Newsfeeds") );
				g_NewsFeedManager.AddNewsFeed( pNewsFeed, true );
			}
		}

		m_matrix.SetStatusIdx(MSG_INIT_NETWORK);

		// Load a cache, if we have one...
		DNS_LoadCache();

		g_MenuInfo.UpdateSafeMode( _T("016: Initializing Network") );
		if (InitializeNetwork())
		{
			m_bNetInitialized = true;
		}
		else
		{
			m_bNetInitialized = false;
		}
	}

	g_MenuInfo.UpdateSafeMode( _T("017: Re-Reading EEPROM") );
	// Re-read the information....
	g_eepromInfo.ReadFromXBOX();
	g_eepromInfo.Decrypt();

	if ( !( g_MenuInfo.SafeMode.m_dwFlags & SM_NONETWORK ) &&
		g_MenuInfo.m_pPrefInfo->GetBool( _T("network"), _T("enablenetwork"), TRUE ) )
	{
		g_MenuInfo.UpdateSafeMode( _T("018: Starting Newsfeed action") );
		g_NewsFeedManager.Start();
	}

	m_matrix.SetStatusIdx(MSG_INIT_MENUS);

	g_MenuInfo.UpdateSafeMode( _T("019: Loading Entries") );
	LoadEntries();


	m_matrix.SetStatusIdx(MSG_INIT_SKINS);
	if ( g_MenuInfo.Overrides.m_bForceGlobalMusic )
	{
		g_MenuInfo.m_bGlobalMusic = true;
	}
	else if ( g_MenuInfo.Overrides.m_bForceLocalMusic )
	{
		g_MenuInfo.m_bGlobalMusic = false;
	}
	if ( g_MenuInfo.Overrides.m_bForceRandomMusic )
	{
		g_MenuInfo.m_bRandomMusic = true;
	}

	g_MenuInfo.UpdateSafeMode( _T("020: Loading Skins") );
	LoadSkins();


	if ( g_MenuInfo.m_pPrefInfo->GetString( _T("autocfg"), _T("saverdelay"), _T(""), true ).GetLength() )
	{
		g_MenuInfo.m_lSaverDelay = g_MenuInfo.m_pPrefInfo->GetLong( _T("autocfg"), _T("saverdelay"), (g_MenuInfo.m_lSaverDelay/60000) )*60000;
	}


	if ( (!( g_MenuInfo.SafeMode.m_dwFlags & SM_NOMUSIC )) && g_MenuInfo.m_bUseMusicManager )
	{
		g_MenuInfo.UpdateSafeMode( _T("021: Initializing Soundtrack") );
		if ( InitializeSoundtrack() == false )
		{
			g_MenuInfo.m_bUseMusicManager = FALSE;
		}
	}
	else
	{
		g_MenuInfo.m_bUseMusicManager = FALSE;
	}
	MusicSetVolume( g_MenuInfo.m_pPrefInfo->GetLong( _T("audio"), _T("musicvolume"), 10 ) );


//	sTempT = GetXSNewsFeed();

	m_matrix.SetStatusIdx(MSG_INIT_FINAL);


	if ( m_bNetInitialized && g_MenuInfo.m_bDoTimeServer )
	{
		g_MenuInfo.UpdateSafeMode( _T("022: Performing Clock Syncronization") );
		DoClockSynchronization(g_MenuInfo.m_sTimeServerIP, g_MenuInfo.m_lTimeSynchOffset );
	}

	// Check clock here, to make sure the time is OK
	// Set it to a default, if need be.
	CheckTime();

	PerfGetPerformance( _T("Load Skins"), FALSE );

	g_MenuInfo.UpdateSafeMode( _T("023: Loading TitleImage Data") );
	//
	// Create the TitleImage.xbx file in "UDATA"
	// This is the Iconic representation of MXM in the Dashboard
	//
	//
	if ( !FileExists( _T("U:\\TitleImage.xbx") ) )
	{
		DEBUG_LINE( _T("TitleImage does not Exist!") );
		if ( XLoadSection( _T("LogoData") ) )
		{
			// Write out file....
			HANDLE hFile;
			hFile = CreateFile( _T("U:\\TitleImage.xbx"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL );
			if ( hFile != INVALID_HANDLE_VALUE  )
			{
				DWORD dwWritten;

				WriteFile( hFile, g_pbTitleImage,g_pbTitleImageLength, &dwWritten, NULL );
				if ( dwWritten == g_pbTitleImageLength )
				{
					DEBUG_LINE( _T("TitleImage written!") );
				}
				CloseHandle( hFile );
			}
			else
			{
				DEBUG_LINE( _T("Unable to write TitleImage!") );
			}
			XFreeSection( _T("LogoData") );
		}
		else
		{
			DEBUG_LINE( _T("Unable to load TitleImage!") );
		}
	}
	else
	{
		DEBUG_LINE( _T("TitleImage already Exists!") );
	}


	//
	// Set up our game states here....
	//
	// We create and add each game state to the main
	// CXBStateApplication class object.
	//
	g_MenuInfo.UpdateSafeMode( _T("024: Adding App States") );

	CXBoxGameState* pGameState;

	DEBUG_LINE( _T("Adding Title State") );

	pGameState = new CXBoxTitleState(this);
	AddGameState( (CXBoxGameState*)pGameState );
	pGameState->SetGameStateID(	GS_TITLE );
	if ( g_MenuInfo.m_lTitleDelay )
	{
		SetGameState( GS_TITLE );
		if ( g_MenuInfo.m_lTitleDelay > 0 )
		{
			if ( g_MenuInfo.m_lTitleDelay < 2000 )
			{
				g_MenuInfo.m_lTitleDelay = 2000;
			}
		}
	}

	DEBUG_LINE( _T("Adding Menu State") );

	pGameState = new CXBoxMenuState(this);
	AddGameState( (CXBoxGameState*)pGameState );
	pGameState->SetGameStateID(	GS_MENU );
	if ( g_MenuInfo.m_lTitleDelay == 0 )
	{
		SetGameState( GS_MENU );
		g_MenuInfo.m_lTitleDelay = 5;
	}


	DEBUG_LINE( _T("Adding Saver State") );
//	DEBUG_WAIT();

	pGameState = new CXBoxSaverState(this);
	AddGameState( (CXBoxGameState*)pGameState );
	pGameState->SetGameStateID(	GS_SAVER );

	pGameState = new CXBoxGameLoadState(this);
	AddGameState( (CXBoxGameState*)pGameState );
	pGameState->SetGameStateID(	GS_LOAD );

	pGameState = new CXBoxGamePasscodeState(this);
	AddGameState( (CXBoxGameState*)pGameState );
	pGameState->SetGameStateID(	GS_PASS );

	pGameState = new CXBoxGameHelpState(this);
	AddGameState( (CXBoxGameState*)pGameState );
	pGameState->SetGameStateID(	GS_HELP );

	
	pGameState = new CXBoxInfoState(this);
	AddGameState( (CXBoxGameState*)pGameState );
	pGameState->SetGameStateID(	GS_INFO );


	PerfGetPerformance( _T("Ready To Run"), FALSE );
//	DEBUG_WAIT();

}


CMXMApp::CMXMApp() 
	: CXBoxStateApplication(),
	m_bPauseInitScreen(false),
	m_bFinishInitThread(false),
	m_dwFrameCount(0)
{
	
	PerfGetPerformance( _T("App Constructor Entry"), FALSE );

}


// AutoConfig
//  Each game directory can have a pre-set "MXM_Entry.ini" file
//  Describing: Title, Description, Media (relative path to MXM_Entry.ini subdir)
//              XBE name, Parental level, password
//
//  If no MXM_Entry.ini, we fall back to look for MXM_SS.* for screenshot
//  and default.xbe for executable. Next we look for FIRST executable
//
//  

//  Fill out entry as best as possible
//
//
//

//
// Autoconfig:
// First, read info from MXM.INI file
// Second, scan each subdirectory under MXM.ini file directory
//  For each subdirectory:
//   Read MXM_Entry.ini file, if it exists
//   If Exists
//    Verify XBE exists (if not, fall through to next)
//   Look for Default.xbe
// 

// extern unsigned char HalpTrayEjectRequiresReboot;

//extern "C" unsigned char HalpTrayEjectRequiresReboot;

//-----------------------------------------------------------------------------
// Name: main()
// Desc: Entry point to the program.
//-----------------------------------------------------------------------------
VOID __cdecl main()
{
    CMXMApp * pxbApp = NULL;

	PLAUNCH_DATA pldInfo;

	g_MenuInfo.m_dwLaunchParam = -54321;

	DBGPerfResetTimer();

	InitializeCriticalSection( &g_RenderCriticalSection );

//	CImageSource::m_bAllowVideo = true;

	g_dwLaunchInfoResult = XGetLaunchInfo( &g_MenuInfo.m_dwLaunchParam, &g_MenuInfo.m_LaunchData );
	pldInfo = &g_MenuInfo.m_LaunchData;
	CStdString sPartition;

	CStdString sRealDrive;

	WORD byTemp = 0;
	WORD wInitState;

	PANSI_STRING pImageFileName = (PANSI_STRING)XeImageFileName;

	g_MenuInfo.	m_sImageFileName = CStdString( pImageFileName->Buffer, pImageFileName->Length );

	if ( SUCCEEDED( HalReadSMBusValue(0x20, 0x1b, 0, (LPBYTE)&byTemp) ) )
	{
		wInitState = byTemp;
	}

	sRealDrive = GetRealCDROMPath();

	CStdString sTempDrivePath;
	sTempDrivePath = g_MenuInfo.m_sImageFileName;
	sTempDrivePath.MakeLower();
	bool bLaunchedFromDVD = false;

	if ( sTempDrivePath.Find( _T("cdrom0") ) >= 0 )
	{
		bLaunchedFromDVD = true;
	}

//	HRESULT hrDP = GetRealDrivePath( 'd', sTempDrivePath );
//	hrDP = GetRealDrivePath( 'D', sTempDrivePath );
//	hrDP = GetRealDrivePath( 'R', sTempDrivePath );


	DEBUG_FORMAT( _T("Beginning Debug Session for MXM - Compiled %s %s"), _T(__DATE__), _T(__TIME__) );

	DEBUG_FORMAT( _T("Real Device Path for D: = (%s)"), sRealDrive.c_str() );

	// Check against CDROM partition....
	if ( _tcsnicmp( sRealDrive, _T( CdRom ), strlen( CdRom ) ) == 0 )
	{
		// It's the DVD drive....
		CStdString sPath;

		sPath = sRealDrive.Mid( strlen( CdRom ) );
		sRealDrive = _T(CdRom);
		sRealDrive += sPath;
		DEBUG_FORMAT( _T("MXM Launched from DVDR/CDR/CDRW") );
	}


	OutputDebugString( _T("Reporting Drive:") );
	OutputDebugString( sRealDrive );
	OutputDebugString( _T("\r\n") );

	// DWORD dwStatus = (DWORD)HalpTrayEjectRequiresReboot;

	// HalpTrayEjectRequiresReboot(FALSE);
	// LPCSTR szExeFilePath = XepDashboardRedirectionPath;

//	PLAUNCH_DATA_PAGE pLaunchPage = &LaunchDataPage;

//	UINT uiDriveType = GetDriveType( _T("D:\\") );


//	pImageHeaders = RtlImageNtHeader(0);


	if ( g_dwLaunchInfoResult == ERROR_SUCCESS )
	{
		switch(g_MenuInfo.m_dwLaunchParam)
		{
			case LDT_FROM_DASHBOARD:
				g_sLaunchDescr.Format( _T("GLI OK: DASH LT=%d Ctx=0x%08x"), g_MenuInfo.m_dwLaunchParam, ((PLD_FROM_DASHBOARD)pldInfo)->dwContext);
				break;
			case LDT_FROM_DEBUGGER_CMDLINE:
				g_sLaunchDescr.Format( _T("GLI OK: DEBUG LT=%d CmdLine=(%s)"), g_MenuInfo.m_dwLaunchParam, ((PLD_FROM_DEBUGGER_CMDLINE)pldInfo)->szCmdLine);
				break;
			case LDT_TITLE:
				g_sLaunchDescr.Format( _T("GLI OK: TITLE LT=%d"), g_MenuInfo.m_dwLaunchParam );
				break;
			default:
				g_sLaunchDescr.Format( _T("GLI OK: UNKNOWN LT=%d"), g_MenuInfo.m_dwLaunchParam );
				break;
		}
	}
	else
	{
		g_sLaunchDescr.Format( _T("GLI FAILED: Rslt=0x%08x LT=%d"), g_dwLaunchInfoResult, g_MenuInfo.m_dwLaunchParam);
	}

	PXBOX_HARDWARE_INFO pHardwareInfo = (PXBOX_HARDWARE_INFO)XboxHardwareInfo;
//	PANSI_STRING pTemp;
//	pTemp = (PANSI_STRING)ExpCdRomBootROMStringBuffer;
//	pTemp = (PANSI_STRING)XepDefaultImagePath;
//	pTemp = (PANSI_STRING)XepLoaderLock;
//	pTemp = (PANSI_STRING)XboxHardwareInfo;
//	pTemp = (PANSI_STRING)KeHasQuickBooted;
//	pTemp = (PANSI_STRING)ObpDosDevicesDriveLetterMap;
//	pTemp = (PANSI_STRING)XepDataTableEntry;
//	pTemp = (PANSI_STRING)FactorySettingsInfo;
//	pTemp = (PANSI_STRING)UserSettingsInfo;
//	pTemp = (PANSI_STRING)ExpCdRomBootROMString;
//	pTemp = (PANSI_STRING)ExpHardDiskBootROMString;
//	pTemp = (PANSI_STRING)ExpHDXbdmDLL;
//	pTemp = (PANSI_STRING)ExpDVDXbdmDLL;
//	pTemp = (PANSI_STRING)XepDashboardRedirectionPath;
//	pTemp = (PANSI_STRING)XepDashboardImagePath;
//	pTemp = (PANSI_STRING)ObpIoDevicesDirectoryObject;
//	pTemp = (PANSI_STRING)IdexCdRomDeviceNameBuffer;
//	g_MenuInfo.	m_sImageFileName = (LPSTR)IdexCdRomDeviceNameBuffer;

	g_MenuInfo.m_sMXMPath = _T("");
	g_MenuInfo.m_bDashMode = true;
	g_MenuInfo.m_bDefaultDash = false;
	// Now check to see if we are the default dashboard or not...
	if ( IsDrivePresent('C') )
	{
		g_MenuInfo.m_sMXMPath = _T("C:\\");
		g_sLaunchDescr += _T(" C:");
	}
	if ( !IsDrivePresent('D') )
	{
		g_MenuInfo.m_bDefaultDash = true;
		g_sLaunchDescr.Format( _T("Def. Dash: %s"), g_sLaunchDescr.c_str() );
	}
	else
	{
		if ( sRealDrive.GetLength() == 0 )
		{
			sRealDrive = _T("D:\\");
		}
		g_MenuInfo.m_sMXMPath = _T("D:\\");
		g_MenuInfo.m_bDashMode = false;
		g_sLaunchDescr += _T(" D:");
	}
	if ( IsDrivePresent('E') )
	{
		if ( g_MenuInfo.m_sMXMPath.GetLength() == 0 )
		{
			g_MenuInfo.m_sMXMPath = _T("E:\\");
		}
		g_sLaunchDescr += _T(" E:");
	}
	if ( IsDrivePresent('F') )
	{
		if ( g_MenuInfo.m_sMXMPath.GetLength() == 0 )
		{
			g_MenuInfo.m_sMXMPath = _T("F:\\");
		}
		g_sLaunchDescr += _T(" F:");
	}
	CStdString sFetchTest;

	LPCTSTR szTestToken = _T("Test::One::two");
	
//	sFetchTest = FetchIDToken( 0, szTestToken );
//	sFetchTest = FetchIDToken( 1, szTestToken );
//	sFetchTest = FetchIDToken( 2, szTestToken );
//	sFetchTest = FetchIDToken( 3, szTestToken );


	g_MenuInfo.m_iIntroOption = 0;
	if ( g_szPatchKeyword[15] == '1' )
	{
		g_MenuInfo.m_iIntroOption = 1;
	}
	else if ( g_szPatchKeyword[15] == '2' )
	{
		g_MenuInfo.m_iIntroOption = 2;
	}
	else if ( g_szPatchKeyword[15] == '3' )
	{
		g_MenuInfo.m_iIntroOption = 3;
	}
	else if ( g_szPatchKeyword[15] == 'X' )
	{
		g_MenuInfo.m_iIntroOption = RandomNum()%4;
	}

	if ( g_szPatchKeyword[16] >= '0' && g_szPatchKeyword[16] <= '6' )
	{
		g_MenuInfo.m_iIntroColor = g_szPatchKeyword[16] - '0';
	}
	else
	{
		g_MenuInfo.m_iIntroColor = -1;
	}
	switch ( g_szPatchKeyword[17] )
	{
		case '0':
			// Not F: or G:
			g_MenuInfo.m_iDriveFOption = -1;
			g_MenuInfo.m_iDriveGOption = -1;
			break;
		case '1':
			// F: only
			g_MenuInfo.m_iDriveFOption = 1;
			g_MenuInfo.m_iDriveGOption = -1;
			break;
		case '2':
			// G: only (odd)
			g_MenuInfo.m_iDriveFOption = -1;
			g_MenuInfo.m_iDriveGOption = 1;
			break;
		case '3':
			// Force both
			g_MenuInfo.m_iDriveFOption = 1;
			g_MenuInfo.m_iDriveGOption = 1;
			break;
		default:
			g_MenuInfo.m_iDriveFOption = 0;
			g_MenuInfo.m_iDriveGOption = 0;
			break;
	}

	g_MenuInfo.m_iSaverOption = g_MenuInfo.m_iIntroOption;


	RebuildSymbolicLink( _T(DriveC), _T(CDrive) );
	RebuildSymbolicLink( _T(DriveE), _T(EDrive) );
	if ( g_MenuInfo.m_iDriveFOption != -1 )
	{
		RebuildSymbolicLink( _T(DriveF), _T(FDrive) );
	}
	if ( g_MenuInfo.m_iDriveGOption != -1 )
	{
		if ( g_MenuInfo.m_iDriveGOption == 1 )
		{
			RebuildSymbolicLink( _T(DriveG), _T(GDrive) );
		}
		else
		{
			// Check size of F: to determine what we are going to do.
			// F: will be a specific size, if there's a G:
			// What it is, I don't know for sure... so I'll check a range:
			ULARGE_INTEGER u64Free, u64FreeTotal, u64Total;
			if ( GetDiskFreeSpaceEx( _T("F:\\"), &u64Free, &u64Total, &u64FreeTotal ) )
			{
				// 129,403,207,680 Reported by a user. 
				if ( // (u64Total.QuadPart > 0i64 ) &&   Rather silly, isn't it?
					 ( u64Total.QuadPart > 128000000000i64 ) &&
					 ( u64Total.QuadPart < 132000000000i64 ) )
				{
					RebuildSymbolicLink( _T(DriveG), _T(GDrive) );
				}
			}

		}
	}

	// Insure Title and USer data directories are available and ready to use!!
	if( !FileExists( _T("E:\\UDATA\\00004321") ) )
	{
		CreateDirectory( _T("E:\\UDATA\\00004321"), NULL );
	}
	if( !FileExists( _T("E:\\TDATA\\00004321") ) )
	{
		CreateDirectory( _T("E:\\TDATA\\00004321"), NULL );
	}

	RebuildSymbolicLink( _T(DriveT), _T(TDrive) );
	RebuildSymbolicLink( _T(DriveU), _T(UDrive) );

	RebuildSymbolicLink( _T(DriveX), _T(XDrive) );
	RebuildSymbolicLink( _T(DriveY), _T(YDrive) );
	CStdString sFormatDrive;

//	RebuildSymbolicLink( _T(DriveZ), _T(ZDrive) );
	// Have to jog this before anything else....
	XMountUtilityDrive( FALSE );
//	HRESULT hrFormat = GetRealDrivePath( 'Z', sFormatDrive );
//	hrFormat = RebuildSymbolicLink( _T(DriveZ), _T(FDrive), true );
//	hrFormat = GetRealDrivePath( 'Z', sFormatDrive );
//	bResult = XFormatUtilityDrive();
	RebuildSymbolicLink( _T(DriveZ), _T(ZDrive), true );

	if ( FileExists( _T("Z:\\safemode.txt") ) )
	{
		CStdString sSafeMode;
		g_MenuInfo.SafeMode.m_bActive = true;
		g_MenuInfo.SafeMode.m_sSafeMsg = LoadStringFromFile(_T("Z:\\safemode.txt"));
	}

	g_MenuInfo.UpdateSafeMode( _T("000: Initializing") );

	bool bSetPathHardway = true;

	if ( g_MenuInfo.m_sImageFileName.GetLength() )
	{
		g_MenuInfo.m_sMXMPath = StripFilenameFromPath(g_MenuInfo.m_sImageFileName);
		g_MenuInfo.m_sActualDevicePath = g_MenuInfo.m_sMXMPath;
		g_MenuInfo.m_sActualPath = g_MenuInfo.m_sMXMPath;
		if ( g_MenuInfo.m_sMXMPath.GetLength() )
		{
			bSetPathHardway = false;
		}
	}

	if ( bSetPathHardway )
	{
		if ( g_MenuInfo.m_bDashMode )
		{
			if ( g_MenuInfo.m_sMXMPath.GetLength() == 0 )
			{
				if ( FindMXM( _T(CDrive) ) )
				{
					g_MenuInfo.m_sMXMPath = _T("C:\\");
				}
				else if ( FindMXM( _T(EDrive) ) )
				{
					g_MenuInfo.m_sMXMPath = _T("E:\\");
				}
				else if ( FindMXM( _T(FDrive) ) )
				{
					g_MenuInfo.m_sMXMPath = _T("F:\\");
				}
				else
				{
					g_MenuInfo.m_sMXMPath = _T("C:\\");
				}
			}
		}
		else
		{
			if ( sRealDrive.GetLength() )
			{
				g_MenuInfo.m_sMXMPath = sRealDrive;
				g_MenuInfo.m_sActualDevicePath = sRealDrive;
				g_MenuInfo.m_sActualPath = sRealDrive;
			}
		}
	}

	g_MenuInfo.m_sMXMPath.Replace( _T(CDrive), _T("C:") );
	g_MenuInfo.m_sMXMPath.Replace( _T(CdRom), _T("D:") );
	g_MenuInfo.m_sMXMPath.Replace( _T(EDrive), _T("E:") );
	g_MenuInfo.m_sMXMPath.Replace( _T(FDrive), _T("F:") );
	g_MenuInfo.m_sMXMPath.Replace( _T(GDrive), _T("G:") );


	g_MenuInfo.m_sActualPath = g_MenuInfo.m_sMXMPath;

	if ( g_MenuInfo.m_sMXMPath.Find( _T("D:\\"), 0 ) == 0 )
	{
		g_MenuInfo.m_sMXMPath = _T("D:\\");
	}

	{
		SYSTEMTIME SystemTime;
		GetSystemTime(&SystemTime);

		srand( (unsigned)GetTickCount()+SystemTime.wSecond+SystemTime.wMilliseconds );
	}

	// g_szPatchKeyword = "PATCHPOINT4321:00000000";
//	TCHAR szPath[1024];

//	GetCurrentDirectory( 1024, szPath );

	

	//Establish a connection to 192.168.123.111 on port 7654.
	//The "1" parameter means that we want this connection to block.
	//Specify "0" instead to get a nonblocking connection.
	//Nonblocking connections won't wait to send/receive
	//data, so calls to Recv and Send return immediately.

	PerfGetPerformance( _T("Debug Init"), FALSE );
	DBGPerfResetTimer();

	DEBUG_LINE( g_sLaunchDescr );
	
//	CxImage imgData;

//	imgData.Load( "test.bmp" );
//	imgData.GetDIB();
	CStdString sTemp;


	switch( g_MenuInfo.m_dwLaunchParam )
	{
		case LDT_FROM_DASHBOARD:
			sTemp.Format( _T("LDT_FROM_DASHBOARD"));
			break;
		case LDT_FROM_DEBUGGER_CMDLINE:
			sTemp.Format( _T("LDT_FROM_DEBUGGER_CMDLINE"));
			break;
		case LDT_TITLE:
			sTemp.Format( _T("LDT_TITLE: %d %d %d %d"), g_MenuInfo.m_LaunchData.Data[0],
				g_MenuInfo.m_LaunchData.Data[1],g_MenuInfo.m_LaunchData.Data[2],
				g_MenuInfo.m_LaunchData.Data[3]);
			break;
		default:
			sTemp.Format( _T("Unknown Launch Param: 0x%08lx"),g_MenuInfo.m_dwLaunchParam);
			break;
	}

	g_MenuInfo.UpdateSafeMode( _T("001: Reading from EEPROM") );

	g_eepromInfo.ReadFromXBOX();
	g_eepromInfo.Decrypt();

	DEBUG_LINE(sTemp.c_str());


	g_MenuInfo.UpdateSafeMode( _T("002: Disable Reset On Eject") );

	g_MenuInfo.m_bDVDMode = bLaunchedFromDVD;

	if ( g_MenuInfo.m_bDVDMode )
	{
			XKUtils xkUtils;

			RebuildSymbolicLink( _T(DriveD), _T(CdRom), true );
			xkUtils.DVDDisableEjectReset();
	}
	else
	{
		CStdString sPathTemp(g_MenuInfo.m_sActualPath);
		sPathTemp.MakeLower();
		if ( ( sPathTemp.Find( _T("d:") ) < 0 ) &&
			( sPathTemp.Find( _T("\\device\\cdrom0") ) < 0 ) )
		{
			XKUtils xkUtils;

			RebuildSymbolicLink( _T(DriveD), _T(CdRom), true );
			g_MenuInfo.m_bDVDMode = false;
			xkUtils.DVDDisableEjectReset();
		}
		else
		{
			XKUtils xkUtils;

			g_MenuInfo.m_bDVDMode = true;
			xkUtils.DVDDisableEjectReset();
			// xkUtils.DVDEnableEjectReset();
		}
	}

#ifdef XDEBUG

	g_MenuInfo.UpdateSafeMode( _T("003: Init Debug") );
	InitDebug( DEBUG_IP, 7654, 1 );

#endif


	g_MenuInfo.UpdateSafeMode( _T("004: Creating app") );
	pxbApp = new CMXMApp;
	PerfGetPerformance( _T("App Constructed"), FALSE );

	if ( pxbApp )
	{
		g_MenuInfo.m_sRealPath = GetRealCDROMPath();
		if( FAILED( pxbApp->Create() ) )
			return;

		// End Safe Mode
		g_MenuInfo.UpdateSafeMode( NULL );
		PerfGetPerformance( _T("App Created"), FALSE );
	    pxbApp->Run();
	}
}

//CStdString		g_sPSCCurDir(_T(""));
//TMapStrings		g_msEnvironment;

HRESULT ProcessScriptCommand( LPCTSTR szArg, CStdString & sResponse, LPCTSTR szPath, DWORD dwFlags, TMapStrings * pLocalEnv )
{
	HRESULT hr = E_FAIL;
	CStdString sArg(szArg);
	CStdString sTemp;

	if ( (dwFlags&PSC_GUI) && (szPath==NULL))
	{
		if ( g_sPSCCurDir.GetLength() == 0 )
		{
			g_sPSCCurDir = g_MenuInfo.m_sMXMPath;
		}
		szPath = g_sPSCCurDir;
	}

	sArg.Trim();
    if ( szArg && sArg.GetLength() )
	{
		if ( _tcsnicmp( sArg, _T("reboot"), 6 ) == 0 )
		{
			GetStateApp()->Reboot();
		}
		else if ( _tcsnicmp( sArg, _T("shutdown"), 8 ) == 0 )
		{
			XK_XBOXPowerOff();
		}
		else if ( _tcsnicmp( sArg, _T("powercycle"), 10 ) == 0 )
		{
			XK_XBOXPowerCycle();
		}
		else if ( _tcsnicmp( sArg, _T("trayopen"), 8 ) == 0 )
		{
			XK_TrayEject();
			hr = S_OK;
		}
		else if ( _tcsnicmp( sArg, _T("trayclose"), 9 ) == 0 )
		{
			XK_TrayClose();
			hr = S_OK;
		}
		else if ( _tcsnicmp( sArg, _T("allowformat "), 12 ) == 0 )
		{
			CStdString sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);
			int iArg1 = _tcstoul( sArg1, NULL, 10 );

			if ( iArg1 )
			{
				// HAve to do this before formatting!!
				XMountUtilityDrive(FALSE);
				g_MenuInfo.m_bAllowFormat = true;
			}
			else
			{
				g_MenuInfo.m_bAllowFormat = false;
			}
			hr = S_OK;
		}
		else if ( _tcsnicmp( sArg, _T("format "), 7 ) == 0 )
		{
			int iPartitionNum;
			CStdString sDrive;
			sDrive = TranslateVar(GetArg( sArg, 1 ), pLocalEnv); // .Mid(7);
			sDrive.Trim();
			sDrive.MakeUpper();
			if ( ( sDrive.GetLength() == 2 ) && _istalpha( sDrive[0] ) && ( sDrive[1] == _T(':') ) )
			{
				iPartitionNum = -1;
				switch( sDrive[0] )
				{
					case _T('X'):
						iPartitionNum = 3;
						break;
					case _T('Y'):
						iPartitionNum = 4;
						break;
					case _T('Z'):
						iPartitionNum = 5;
						break;
					case _T('F'):
						iPartitionNum = 6;
						break;
					case _T('G'):
						iPartitionNum = 7;
						break;
					case _T('C'):
						iPartitionNum = 2;
						break;
					case _T('E'):
						iPartitionNum = 1;
						break;
				}
				if ( iPartitionNum != -1 )
				{
					if ( g_MenuInfo.m_bAllowFormat )
					{
						HRESULT hrError = FormatPartition( iPartitionNum );
						if ( SUCCEEDED( hrError ) )
						{
							hr = S_OK;
							sResponse.Format( _T("Format Partition %d\r\nCompleted OK (0x%08X)"), iPartitionNum, hrError );
						}
						else
						{
							hr = hrError;
							sResponse.Format(_T("(Format) Error (0x%08X)\r\noccurred in format Part#%f"), hrError, iPartitionNum );
						}
						g_sFormatResult = sResponse;
						SetActionVar( _T("_formatresult"), sResponse );
						SetLastError( hrError );
					}
					else
					{
						g_sFormatResult = _T("Format FAILED!! Not enabled!");
						hr = E_FAIL;
						sResponse = _T("(Format) Partition Formatting must be enabled by user - System Menu");
						GetStateApp()->MessageBox( _T("ATTENTION!\r\nPartition Formatting must be enabled by user\r\nAccess System Menu to change") );
					}
				}
				else
				{
					g_sFormatResult = _T("Format FAILED!! Invalid drive selected!");
					hr = E_FAIL;
					sResponse = _T("(Format) Invalid drive selected");
				}
			}
			else
			{
				g_sFormatResult.Format( _T("Format FAILED!! Syntax Error!\r\n%s\r\n%s"), sDrive.c_str(), sArg.c_str() );
				hr = E_FAIL;
				sResponse = _T("(Format) Syntax Error");
			}
			// OK, the remainder should be one alpha and a colon
			// FormatPartition(6 or 7);
			hr = S_OK;
			// Force statics to reset drive conditionals
			IsConditionMet( 0, false, true );
		}
		else if ( _tcsnicmp( sArg, _T("resetmenucache"), 9 ) == 0 )
		{
			sTemp = MakeFullFilePath( g_MenuInfo.m_sMXMPath, _T("cache_menu.xml") );
			if ( FileExists( sTemp ) )
			{
				DeleteFile( sTemp );
			}
			hr = S_OK;
			// XK_XBOXPowerCycle();
		}
		//else if ( _tcsnicmp( sArg, _T("video pal"), 9 ) == 0 )
		//{
		//	g_MenuInfo.m_pPrefInfo->SetBool( _T("local"), _T("ntsc"), FALSE );
		//	g_MenuInfo.SavePrefs();
		//	SetNTSC(false);
		//	GetStateApp()->Reboot();
		//}
		//else if ( _tcsnicmp( sArg, _T("video ntsc"), 9 ) == 0 )
		//{
		//	g_MenuInfo.m_pPrefInfo->SetBool( _T("local"), _T("ntsc"), TRUE );
		//	g_MenuInfo.SavePrefs();
		//	SetNTSC(true);
		//	GetStateApp()->Reboot();
		//}
		else if ( _tcsnicmp( sArg, _T("autorenameon"), 12 ) == 0 )
		{
			g_MenuInfo.m_bAutoRenameFTP = true;
			hr = S_OK;
		}
		else if ( _tcsnicmp( sArg, _T("cd "), 3 ) == 0 )
		{
			if ( dwFlags & PSC_GUI )
			{
				hr = S_OK;
				CStdString sArg1, sArg2;
				sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);

				if ( sArg1.GetLength() )
				{
					// Do it...
					sArg1 = MakeFullFilePath( szPath, sArg1 );
					DWORD dwAttr = GetFileAttributes(sArg1);
					if ( ((int)dwAttr !=  (-1) ) && (dwAttr&FILE_ATTRIBUTE_DIRECTORY))
					{
						g_sPSCCurDir = sArg1;
					}
				}
				else
				{
					hr = E_FAIL;
					sResponse = _T("(cd) Syntax error");
				}
			}
			else
			{
				hr = E_FAIL;
				sResponse = _T("(cd) Only available to GUI");
			}
		}
		else if ( _tcsnicmp( sArg, _T("autorenameoff"), 13 ) == 0 )
		{
			g_MenuInfo.m_bAutoRenameFTP = false;
			hr = S_OK;
		}
		else if ( _tcsnicmp( sArg, _T("autorename"), 10 ) == 0 )
		{
			if ( g_MenuInfo.m_bAutoRenameFTP )
			{
				sResponse = _T("AutoRename ON");
			}
			else
			{
				sResponse = _T("AutoRename OFF\r\nWhassup!\nCool!\n");
			}
			hr = S_OK;
		}
		else if ( _tcsnicmp( sArg, _T("launchdvd"), 9 ) == 0 )
		{
			if ( FileExists( _T("D:\\default.xbe") ) )
			{
				g_MenuInfo.m_bSpecialLaunch = true;
				g_MenuInfo.m_sSpecialLaunch = _T("D:\\default.xbe");
				g_MenuInfo.m_sSpecialLaunchData = _T("");
				GetStateApp()->SetGameState( GS_LOAD );
			}
			else if ( FileExists( _T("D:\\VIDEO_TS") ) )
			{
			}
			else
			{
				// Search for any xbe...
			}
		}
		else if ( _tcsnicmp( sArg, _T("copy "), 5 ) == 0 )
		{
			CStdString sArg1, sArg2;
			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);
			sArg2 = TranslateVar(GetArg( sArg, 2 ), pLocalEnv);

			if ( sArg1.GetLength() && sArg2.GetLength() )
			{
				// Ready to copy? Let's do it....
				hr = CopyFilesOrPath( szPath, sArg1, sArg2 );
				if ( FAILED(hr) )
				{
					sResponse = _T("(copy) Operation failed");
				}
			}
		}
		else if ( _tcsnicmp( sArg, _T("rename "), 7 ) == 0 )
		{
			CStdString sArg1, sArg2;
			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);
			sArg2 = TranslateVar(GetArg( sArg, 2 ), pLocalEnv);

			if ( sArg1.GetLength() && sArg2.GetLength() )
			{
				// Ready to copy? Let's do it....
				sArg1 = MakeFullFilePath( szPath, sArg1 );
				sArg2 = MakeFullFilePath( szPath, sArg2 );

				if ( MoveFile( sArg1, sArg2 ) )
				{
					hr = S_OK;
				}
				else
				{
					hr = E_FAIL;
					sResponse = _T("(rename) Operation failed");
				}
			}
		}
		else if ( _tcsnicmp( sArg, _T("delete "), 7 ) == 0 )
		{
			CStdString sArg1;
			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);

			if ( sArg1.GetLength() )
			{
				// Ready to copy? Let's do it....
				sArg1 = MakeFullFilePath( szPath, sArg1 );

				if ( FileExists( sArg1 ) )
				{
					if ( DeleteFile( sArg1 ) )
					{
						hr = S_OK;
					}
					else
					{
						hr = E_FAIL;
						sResponse = _T("(delete) Operation failed");
					}
				}
				else
				{
					hr = S_OK;
				}
			}
		}
		else if ( _tcsnicmp( sArg, _T("mkdir "), 6 ) == 0 )
		{
			CStdString sArg1, sArg2;
			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);

			if ( sArg1.GetLength() )
			{
				// Do it...
				sArg1 = MakeFullFilePath( szPath, sArg1 );
				if ( CreateDirectory( sArg1, NULL ) )
				{
					hr = S_OK;
					sResponse = _T("Directory Created");
				}
				else
				{
					hr = E_FAIL;
					sResponse = _T("(mkdir) Unknown Error");
				}
			}
			else
			{
				hr = E_FAIL;
				sResponse = _T("(mkdir) Syntax error");
			}
		}
		else if ( _tcsnicmp( sArg, _T("debug "), 6 ) == 0 )
		{
			CStdString sArg1;
			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);

			hr = S_OK;
			OutputDebugString( sArg1 );
			OutputDebugString( _T("\r\n") );
		}
		else if ( _tcsnicmp( sArg, _T("mount "), 6 ) == 0 )
		{
			CStdString sArg1,sArg2;
			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);
			sArg2 = TranslateVar(GetArg( sArg, 2 ), pLocalEnv);
			// F: device

			hr = E_FAIL;

			if ( sArg1.GetLength() && _istalpha( sArg1[0] ) )
			{
				CStdString sDrive;
				CStdString sDevice(CDrive);
				sDevice = sDevice.Left(_tcslen(CDrive)-1);
				sDrive.Format( _T("\\??\\%c:"), _toupper( sArg1[0] ) );
				if ( ( sArg2.GetLength() == _tcslen(CDrive) ) && ( _tcsnicmp( CDrive, sArg2, _tcslen(CDrive)-1 ) == 0 ))
				{
					sDevice += sArg2[_tcslen(CDrive)-1];
				}
				else
				{
					// Try and do it auto-style...
					if ( _tcsicmp( sArg1, _T("C:") ) == 0 )
					{
						sDevice += _T('2');
					}
					else if ( _tcsicmp( sArg1, _T("E:") ) == 0 )
					{
						sDevice += _T('1');
					}
					else if ( _tcsicmp( sArg1, _T("F:") ) == 0 )
					{
						sDevice += _T('6');
					}
					else if ( _tcsicmp( sArg1, _T("G:") ) == 0 )
					{
						sDevice += _T('7');
					}
					else if ( _tcsicmp( sArg1, _T("X:") ) == 0 )
					{
						sDevice += _T('3');
					}
					else if ( _tcsicmp( sArg1, _T("Y:") ) == 0 )
					{
						sDevice += _T('4');
					}
					else if ( _tcsicmp( sArg1, _T("Z:") ) == 0 )
					{
						sDevice += _T('5');
					}
					else if ( _tcsicmp( sArg1, _T("D:") ) == 0 )
					{
						sDevice = _T(CdRom);
					}
					else
					{
						sDevice = _T("");
					}
				}
				if ( sDevice.GetLength() )
				{
					// OK, ready to go!
					hr = RebuildSymbolicLink( sDrive, sDevice, true );
				}
				else
				{
					sResponse = _T("(mount) Invalid device partiton path");
				}
			}
			else
			{
				sResponse = _T("(mount) Invalid drive letter");
			}
			// Force statics to reset drive conditionals
			IsConditionMet( 0, false, true );
		}
		else if ( _tcsnicmp( sArg, _T("exec "), 5 ) == 0 )
		{
			CStdString sArg1, sRemainder;
			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);
			int iPos = sArg.Find( sArg1, 4 )+sArg1.GetLength();

			sRemainder = sArg.Mid( iPos );
			sRemainder.Trim();

			GetMXMApp()->EXEC( NULL, szPath, sArg1, sRemainder );

			hr = S_OK;
		}
		else if ( _tcsnicmp( sArg, _T("delay "), 6 ) == 0 )
		{
			CStdString sArg1;
			long lValue;

			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);

			lValue = _tcstoul( sArg1, NULL, 10 );

			if ( ( lValue < 1000 ) && ( lValue > 0 ) )
			{
				Sleep( lValue*1000 );
			}
			hr = S_OK;
		}
		else if ( _tcsnicmp( sArg, _T("delayms "), 8 ) == 0 )
		{
			CStdString sArg1;
			long lValue;

			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);

			lValue = _tcstoul( sArg1, NULL, 10 );

			if ( ( lValue < 1000000 ) && ( lValue > 0 ) )
			{
				Sleep( lValue );
			}
			hr = S_OK;
		}
		else if ( _tcsnicmp( sArg, _T("time "), 5 ) == 0 )
		{
			CStdString sTime, sHours, sMinutes;
			int iHours, iMinutes, iPos;
			sTime = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);

			sTime.Trim();
			iPos = sTime.Find( _T(":") );
			if ( iPos > 0 )
			{
				SYSTEMTIME sysTime;

				sHours = sTime.Left( iPos );
				sMinutes = sTime.Mid( iPos+1 );
				iHours = _tcstol( sHours, NULL, 10 );
				iMinutes = _tcstol( sMinutes, NULL, 10 );
				GetLocalTime( &sysTime );
				if ( ( iHours < 24 ) && ( iHours >= 0 ) && ( iMinutes>= 0 ) && (iMinutes<60) )
				{
					sysTime.wHour = iHours;
					sysTime.wMinute = iMinutes;
					sysTime.wMilliseconds = 0;
					sysTime.wDayOfWeek = 0;
					SetNewTime( sysTime );
					hr = S_OK;
				}
				else
				{
					sResponse = _T("Invalid argument");
				}
			}
			else
			{
				sResponse = _T("Invalid argument");
			}
		}
		else if ( _tcsnicmp( sArg, _T("date "), 5 ) == 0 )
		{
			CStdString sDay, sMonth, sYear;
			int iDay, iMonth, iYear;

			sDay = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);
			sMonth = TranslateVar(GetArg( sArg, 2 ), pLocalEnv);
			sYear = TranslateVar(GetArg( sArg, 3 ), pLocalEnv);

			iDay = _tcstol( sDay, NULL, 10 );
			iMonth = _tcstol( sMonth, NULL, 10 );
			iYear =  _tcstol( sYear, NULL, 10 );
			if ( iYear > 0 && iYear < 100 )
			{
				iYear += 2000;
			}
			if ( ( iDay > 0 ) && ( iDay < 32 ) && ( iMonth > 0 ) && ( iMonth < 13 ) && ( iYear > 2002 ) && ( iYear < 2500 ) )
			{
				SYSTEMTIME sysTime;
				FILETIME  NewTime;
				FILETIME  OldTime;


				GetLocalTime( &sysTime );
				sysTime.wDay = iDay;
				sysTime.wMonth = iMonth;
				sysTime.wYear = iYear;
				sysTime.wMilliseconds = 0;
				sysTime.wDayOfWeek = 0;

				if ( SystemTimeToFileTime( &sysTime,  &OldTime) )
				{
					LocalFileTimeToFileTime( &OldTime, &NewTime );
					NtSetSystemTime(&NewTime,&OldTime);  // sets system time
					hr = S_OK;
				}
				else
				{
					sResponse = _T("Invalid argument");
				}
			}
			else
			{
				sResponse = _T("Invalid argument");
			}
		}
		else if ( _tcsnicmp( sArg, _T("rmdir "), 6 ) == 0 )
		{
			CStdString sArg1, sArg2;
			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);
			sArg2 = TranslateVar(GetArg( sArg, 2 ), pLocalEnv);
			bool bCascade = false;

			sArg2.Trim();
			if ( _tcstoul( sArg2, NULL, 10 ) )
			{
				bCascade = true;
			}
			else if ( _tcsicmp( sArg2, _T("true") ) == 0 )
			{
				bCascade = true;
			}
			else if ( _tcsicmp( sArg2, _T("yes") ) == 0 )
			{
				bCascade = true;
			}

			if ( sArg1.GetLength() )
			{
				// Ready to copy? Let's do it....
				sArg1 = MakeFullFilePath( szPath, sArg1 );
				DWORD dwAttr = GetFileAttributes( sArg1 );
				if ( ((int)dwAttr != (-1)) && (dwAttr&FILE_ATTRIBUTE_DIRECTORY) )
				{
					if ( IsPathEmpty(sArg1) )
					{
						if ( RemoveDirectory( sArg1 ) )
						{
							hr = S_OK;
							sResponse = _T("(rmdir) Path removed");
						}
						else
						{
							hr = E_FAIL;
							sResponse = _T("(rmdir) unknown error");
						}
					}
					else
					{
						if ( bCascade )
						{
							if ( RemovePath( sArg1 ) )
							{
								hr = S_OK;
								sResponse = _T("(rmdir) Path removed");
							}
							else
							{
								hr = E_FAIL;
								sResponse = _T("(rmdir) unknown error");
							}
						}
						else
						{
							hr = E_FAIL;
							sResponse = _T("(rmdir) path not empty. Cascade not specified.");
						}
					}
				}
				else
				{
					hr = E_FAIL;
					sResponse = _T("(rmdir) Path does not exist");
				}
			}
			else
			{
				hr = E_FAIL;
				sResponse = _T("(rmdir) Syntax error");
			}
		}
		else if ( _tcsnicmp( sArg, _T("attr "), 5 ) == 0 )
		{
			CStdString sArg1, sArg2;
			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);
			sArg2 = TranslateVar(GetArg( sArg, 2 ), pLocalEnv);

			if ( sArg1.GetLength() && sArg2.GetLength() )
			{
				// Change Attributes....
				// "ro" or "rw"
				sArg1 = MakeFullFilePath( szPath, sArg1 );

				if ( ( _tcsicmp( sArg1, _T("C:\\") ) == 0 ) ||
					 ( _tcsicmp( sArg1, _T("E:\\") ) == 0 ) ||
					 ( _tcsicmp( sArg1, _T("F:\\") ) == 0 ) ||
					 ( _tcsicmp( sArg1, _T("G:\\") ) == 0 ) ||
					 ( _tcsicmp( sArg1, _T("X:\\") ) == 0 ) ||
					 ( _tcsicmp( sArg1, _T("Y:\\") ) == 0 ) ||
					 ( _tcsicmp( sArg1, _T("Z:\\") ) == 0 ) ||
					 FileExists( sArg1 ) )
				{
					if ( _tcsicmp( sArg2, _T("rw") ) == 0 )
					{
						// Set attributes to read-write!
						if ( SUCCEEDED( SetReadonly( sArg1, false, true ) ) )
						{
							hr = S_OK;
							sResponse = _T("(attr) Attributes set to Read-Write");
						}
						else
						{
							hr = E_FAIL;
							sResponse = _T("(attr) Unknown Error");
						}
					}
					else if ( _tcsicmp( sArg2, _T("ro") ) == 0 )
					{
						// Set attributes to read-only!
						if ( SUCCEEDED( SetReadonly( sArg1, true, true ) ) )
						{
							hr = S_OK;
							sResponse = _T("(attr) Attributes set to Read-Write");
						}
						else
						{
							hr = E_FAIL;
							sResponse = _T("(attr) Unknown Error");
						}
					}
					else
					{
						hr = E_FAIL;
						sResponse = _T("(attr) Syntax Error - argument must be \"rw\" or \"ro\"");
					}
				}
				else
				{
					hr = E_FAIL;
					sResponse = _T("(attr) File or directory Does Not Exist");
				}
			}
			else
			{
				hr = E_FAIL;
				sResponse = _T("(attr) Syntax Error - attr filename ro|rw");
			}
		}
		else if ( _tcsnicmp( sArg, _T("eepromrestore "), 14 ) == 0 )
		{
			CStdString sArg1;
			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);
			sArg1.Trim();
			if ( sArg1.GetLength() == 0 )
			{
				sArg1 = _T("eeprom.bin");
			}
			sArg1 = MakeFullFilePath( szPath, sArg1 );
			if ( FileExists( sArg1 ) )
			{
				XKEEPROM eepromData;
				if ( eepromData.ReadFromBINFile( sArg1, true ) )
				{
					eepromData.WriteToXBOX();
					hr = S_OK;
				}
				else
				{
					sResponse = _T("(EEPROMRestore) Unable to read from file");
					hr = E_FAIL;
				}
			}
			else
			{
				sResponse = _T("(EEPROMRestore) File does not exist");
				hr = E_FAIL;
			}
		}
		else if ( _tcsnicmp( sArg, _T("eeprombackup "), 13 ) == 0 )
		{
			CStdString sArg1;
			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);

			sArg1.Trim();
			if ( sArg1.GetLength() == 0 )
			{
				sArg1 = _T("eeprom.bin");
			}
			sArg1 = MakeFullFilePath( szPath, sArg1 );

			XKEEPROM eepromData;
			eepromData.ReadFromXBOX();
			if ( !eepromData.IsEncrypted() )
			{
				eepromData.EncryptAndCalculateCRC();
			}
			if ( eepromData.WriteToBINFile(sArg1) )
			{
				hr = S_OK;
			}
			else
			{
				sResponse = _T("(EEPROMBackup) Unable to write to file");
				hr = E_FAIL;

			}
		}
		else if ( _tcsnicmp( sArg, _T("setfanspeed "), 12 ) == 0 )
		{
			CStdString sArg1;
			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);

			if ( _tcsicmp( sArg1, _T("manual") ) == 0 )
			{
				CStdString sFanspeed = TranslateVar(GetArg( sArg, 2 ), pLocalEnv);
				int iFanSpeed = _tcstol( sFanspeed, NULL, 10 );

				if ( iFanSpeed < 5 )
				{
					iFanSpeed = 5;
				}
				if ( iFanSpeed > 50 )
				{
					iFanSpeed = 50;
				}
				SetFanSpeed( iFanSpeed );
				sResponse.Format( _T("Fanspeed set to %d"), iFanSpeed );
				hr = S_OK;

			}
			else if ( _tcsicmp( sArg1, _T("auto") ) == 0 )
			{
				hr = S_OK;
			}
			else if ( _tcsicmp( sArg1, _T("default") ) == 0 )
			{
				sResponse = _T("Fanspeed set to DEFAULT (10)");
				SetFanSpeed( 10 );
				hr = S_OK;
			}
			else
			{
				hr = E_FAIL;
				sResponse = _T("(fanspeed) Syntax error");
			}
		}
		else if ( ( _tcsicmp( sArg, _T("systemmenu") ) == 0 ) || ( _tcsnicmp( sArg, _T("systemmenu "), 11 ) == 0 ) )
		{
			if ( dwFlags & PSC_GUI )
			{
				CStdString sArg1;
				sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);

				hr = S_OK;

				if ( ( _tcsicmp( sArg1, _T("activate") ) == 0 ) || sArg1.GetLength() == 0 )
				{
					if ( g_MenuInfo.m_bDVDMode )
					{
						GetStateApp()->EnterMenu( &g_MenuHandler, g_MainAppMenuDVD, 0 );
					}
					else
					{
						GetStateApp()->EnterMenu( &g_MenuHandler, g_MainAppMenu, 0 );
					}
				}
				if ( ( _tcsicmp( sArg1, _T("activatefull") ) == 0 ) || sArg1.GetLength() == 0 )
				{
					GetStateApp()->EnterMenu( &g_MenuHandler, g_MainAppMenu, 0 );
				}
				else if ( _tcsicmp( sArg1, _T("unhide") ) == 0 )
				{
					g_MenuInfo.m_bHideSystemMenu = FALSE;
				}
				else if ( _tcsicmp( sArg1, _T("hide") ) == 0 )
				{
					g_MenuInfo.m_bHideSystemMenu = TRUE;
				}
				else
				{
					hr = E_FAIL;
					sResponse.Format( _T("(messagebox) Invalid argument (%s)"), sArg1.c_str() );
				}
			}
			else
			{
				hr = E_FAIL;
				sResponse = _T("(messagebox) Only available to GUI");
			}
		}
		//else if ( _tcsnicmp( sArg, _T("chmod "), 6 ) == 0 )
		else if ( _tcsnicmp( sArg, _T("mediapatch "), 11 ) == 0 )
		{
			CStdString sFilename = MakeFullFilePath( szPath, TranslateVar(GetArg( sArg, 1 ), pLocalEnv ) );

			if ( MediaPatch( sFilename ) )
			{
				hr = S_OK;
				sResponse = _T("(mediapatch) File patched");
				// iterProgram++;
			}
			else
			{
				hr = E_FAIL;
				sResponse = _T("(mediapatch) Unable to patch file");
				//iterProgram = lsProgram.end();
				//bContinue = false;
				//sError.Format( _T("ERROR: (MediaPatch) Erorr applying media patch") );
			}
		}
		else if ( _tcsnicmp( sArg, _T("webfilefetch "), 13 ) == 0 )
		{
			// WebFileFetch <FileName> <URL>
			CStdString sFilename = MakeFullFilePath( szPath, TranslateVar(GetArg( sArg, 1 ), pLocalEnv ) );
			CStdString sURL = TranslateVar(GetArg( sArg, 2 ), pLocalEnv);
			CURI uriWebFile( sURL );

			CStdString sValue = GetWebFile( uriWebFile.GetServer(), uriWebFile.GetPathQuery(), uriWebFile.GetPort() );

			if (  sValue.GetLength() )
			{
				if ( SaveStringToFile( sFilename, sValue ) )
				{
					hr = S_OK;
					sResponse = _T("(webfilefetch) File acquired from web");
				}
				else
				{
					hr = E_FAIL;
					sResponse = _T("(webfilefetch) Unable to write file to disk");
				}
			}
			else
			{
				hr = E_FAIL;
				sResponse = _T("(webfilefetch) Unable to get file");
			}
		}
		else if ( _tcsnicmp( sArg, _T("chmod "), 6 ) == 0 )
		{
			CStdString sArg1, sArg2;
			sArg1 = TranslateVar(GetArg( sArg, 1 ), pLocalEnv);
			sArg2 = TranslateVar(GetArg( sArg, 2 ), pLocalEnv);

			if ( sArg1.GetLength() && sArg2.GetLength() )
			{
				// Change Attributes....
				// "ro" or "rw"
				sArg2 = MakeFullFilePath( szPath, sArg2 );
				if ( FileExists( sArg2 ) )
				{
					// Break down chmod word... it's base 8
					int iCHMOD = _tcstoul( sArg1, NULL, 8 );

					if ( iCHMOD & 0222 ) // Specified WRITEABLE
					{
						// Set attributes to read-write!
						if ( SUCCEEDED( SetReadonly( sArg2, false, false ) ) )
						{
							hr = S_OK;
							sResponse = _T("(chmod) Attributes set to Read-Write");
						}
						else
						{
							hr = E_FAIL;
							sResponse = _T("(chmod) Unknown Error");
						}
					}
					else
					{
						// Set attributes to read-only!
						if ( SUCCEEDED( SetReadonly( sArg2, true, false ) ) )
						{
							hr = S_OK;
							sResponse = _T("(chmod) Attributes set to Read-Only");
						}
						else
						{
							hr = E_FAIL;
							sResponse = _T("(chmod) Unknown Error");
						}
					}
				}
				else
				{
					hr = E_FAIL;
					sResponse = _T("(chmod) File or directory Does Not Exist");
				}
			}
			else
			{
				hr = E_FAIL;
				sResponse = _T("(chmod) Syntax Error - chmod 666|444 filename");
			}
		}
		else
		{
			hr = E_NOTIMPL;
#ifdef DEBUG
			sTemp.Format( _T("SITE COMMAND: \"%s\" unknown."), sArg.c_str() );
			OutputDebugString( sTemp.c_str() );
#endif
		}
	}
	return hr;
}

