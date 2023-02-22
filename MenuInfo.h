/*********************************************************************************\
 * MenuInfo.h
 * Class for handling global information
 * (C) 2002 MasterMind
\*********************************************************************************/

#ifndef __MENUINFO_H__
#define __MENUINFO_H__

#include "MXM_Defs.h"
#include "ItemInfo.h"
#include "Skin.h"
#include "xmltree.h"
#include "CherryPatch.h"
#include "ActionDraw.h"

using namespace std;

#define XBI_DVD_UNKNOWN		0
#define XBI_DVD_XBOX		1
#define XBI_DVD_MOVIE		2
#define XBI_DVD_DATA		3
#define XBI_DVD_MUSIC		4
#define XBI_DVD_EMPTY		5

#define SM_NONETWORK		0x00000001
#define SM_NOVIDEO			0x00000002
#define SM_BASICSKIN		0x00000004
#define SM_SAFEMENU			0x00000008
#define SM_NOFTP			0x00000010
#define SM_NOMUSIC			0x00000020

CStdString GetString( int iIndex, TListStrings & slStrings );

class CXboxInfo
{
	DWORD	m_dwTimeStamp;
	DWORD	m_dwRemountTimeStamp;
	DWORD	m_dwCheckDVDTimeStamp;
public:
	CXboxInfo();
	~CXboxInfo();
	CStdString m_sBIOSMD5;
	CStdString m_sBIOSName;
	CStdString	m_sNickName;
	bool		m_bTrayOpen;
	bool		m_bTrayClosed;
	bool		m_bTrayEmpty;
	int			m_iDVDType;
	int			m_iPrevDVDType;
	bool		m_bTrayWasClosed;
	int			m_iNumFTPUsers;
	bool		CheckState( void );
};

struct TSortData
{
//	CStdString				m_sSortField;
	TListStrings			m_slSortFields;
	TItemList::iterator		m_iterSortStart;
};

class CMenuInfo
{
protected:
//	void			SwapItems( TItemList::iterator iterA, TItemList::iterator iterB );
//	void			SortItems( void );
	int				AddItem( CItemInfo & itemInfo, TSortData & sortData );

public:

	class CXboxInfo Xbox;
	CMenuInfo( void );
	CStdStringW		m_sTitle;
	CStdStringW		m_sSubTitle;

	TActionDrawList m_drawList;

	int				m_iScreenIndex;

	bool			m_bAutoRenameFTP;
	bool			m_bDefaultDash;
	bool			m_bDashMode;
	bool			m_bXDIMode;
	bool			m_bDVDMode;

	int				m_iIntroOption;
	int				m_iIntroColor;
	int				m_iSaverOption;

//					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("audio"), _T("OvMenuSource"), 0 );
//					lResult = g_MenuInfo.m_pPrefInfo->GetLong( _T("autocfg"), _T("cfgstyle"), 0 );
//					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("adddash"), TRUE );
//					lResult = g_MenuInfo.m_pPrefInfo->GetBool( _T("autocfg"), _T("sort"), TRUE );
	bool			m_bUseSkinMenuSounds;
	int				m_iAutoCfgStyle;
	bool			m_bAutoCfgAddDash;
	bool			m_bAutoCfgSort;
	int				m_iCurrentItem;
	int				m_iNumItems;
	bool			m_bItemPrev;
	bool			m_bItemNext;
	CStdString		m_sImageFileName;

	bool			m_bInActionScript;
	list<CStdString> m_saMainHelp;

	int				m_iStyle;

	XNADDR			m_xnaInfo;
	DWORD			m_dwXnaResult;
	bool			m_bXnaValid;
	bool			m_bNetParamsSet;


	struct _NETPARAMS
	{
		int			m_iUseDHCP;
		CStdString	m_sIP;
		CStdString	m_sSubnetmask;      // 0x4c   
		CStdString	m_sDefaultgateway;  // 0x50   
		CStdString	m_sDNS1;            // 0x54   
		CStdString	m_sDNS2;            // 0x58   
	} NetParams, CurrentNetParams;

	bool			m_bDoTimeServer;
	long			m_lTimeSynchOffset;
	CStdString		m_sTimeServerIP;

	int				m_iFTPPriority;
	bool			m_bEnableFTP;

	//struct _NETPARAMS
	//{
	//	int			m_iUseDHCP;
	//	CStdString	m_sIP;
	//	CStdString	m_sSubnetmask;      // 0x4c   
	//	CStdString	m_sDefaultgateway;  // 0x50   
	//	CStdString	m_sDNS1;            // 0x54   
	//	CStdString	m_sDNS2;            // 0x58   
	//} CurrentNetParams;

	struct
	{
		int		m_iPreviewSuspendMusic;
		int		m_iPreviewEnableSound;
		bool	m_bForceGlobalMusic;
		bool	m_bForceLocalMusic;
		bool	m_bForceRandomMusic;
	} Overrides;

	struct _SAFEMODES
	{
		bool	m_bActive;
		DWORD	m_dwFlags;
		CStdString m_sSafeMsg;
		bool	m_bSetUserSafeMode;
		CRITICAL_SECTION m_csSafeMode;
	} SafeMode;

	void			UpdateSafeMode( LPCTSTR szMsg );
	
	// Preferences
	bool			m_bUseCelsius;
	bool			m_bUseEuroNumbers;
	int				m_iDateFormat;
	int				m_iTimeFormat;
	TCHAR			m_tcDateSepChar;
	TCHAR			m_tcTimeSepChar;

	int				m_iDriveFOption;
	int				m_iDriveGOption;

	TListStrings	m_slMenuFileList;
	TListStrings	m_slMenuDirList;
	TItemList		m_ilEntries;
	CXMLNode	*	m_pCacheMenu;
	CXMLNode	*	m_pMenuNode;
	CXMLNode	*	m_pMenuCurrNode;

	CStdString		m_sMenuName;

	CXMLNode	*	m_pSkinInfo;
	CXMLNode	*	m_pCfgInfo;
	CXMLNode	*	m_pPrefInfo;

	CXMLNode	*	m_pInternalInfo;

	CItemInfo *		Entry( int iIndex );
	CItemInfo *     MenuEntry(void);
	CItemInfo		m_CurrentMenu;

	int				GetItemCount( void );
	void			ClearMenuItems( void );
	DWORD			m_dwMenuHash;
	DWORD			GetMenuHash( void ) { return m_dwMenuHash; };
	DWORD			CalcMenuHash( void );

	HRESULT			LoadMenu( CXMLNode * pNode, bool bClearCurrent = true );


	CXBFont			m_pfSelectionFont;

	CStdStringW		m_sResult;
	long			m_lLoadDelay;
	long			m_lTitleDelay;
	long			m_lSaverDelay;

	BOOL			m_bAllowSaverMusic;
	BOOL			m_bAllowSaverSound;

	BOOL			m_bAllowVideoSound;

	BOOL			m_bAllowAutoConfig;
	BOOL			m_bAllowAutoTitle;
	BOOL			m_bAllowAutoDescr;
	BOOL			m_bAllowBetaSkins;
	BOOL			m_bAllowXDISkin;
	BOOL			m_bTestMode;

	BOOL			m_bRemoveDuplicates;
	BOOL			m_bRemoveInvalid;
	BOOL			m_bSafeMode;

	BOOL			m_bCircularMenus;
	BOOL			m_bMenuOrientedUpDown;

	BOOL			m_bGlobalHelp;

	CStdString		m_sSelectedSkin;

	bool			m_bDeepMultiSearch;
	bool			m_bAutoLoadMulti;
	bool			m_bAutoLaunchGame;
	bool			m_bAutoLaunchMovie;

	CStdString		m_sDVDPlayer;
	CStdString		m_sDVDPlayerCmdLine;

	CXMLNode *		FindItem( DWORD dwItemID, CXMLNode * pNode = NULL );

	bool			FindDVDPlayer( CXMLNode * pNode = NULL );

	void			SavePrefs( void );
	void			LoadPrefs( void );

	CStdString		m_sExitApp;
	CStdString		m_sExitAppParams;

	bool			m_bSpecialMXM;
	bool			m_bSpecialLaunch;
	CStdString		m_sSpecialLaunch;
	CStdString		m_sSpecialLaunchData;

	CStdString		m_sSkinsPath;

	CStdString		m_sRealPath;
	CStdString		m_sMXMPath;
	CStdString		m_sActualDevicePath;


	CStdString		m_sSkinPath;
	BOOL			m_bHideSystemMenu;

//	CStdString		m_sMenuPath;

	BOOL			m_bAllowFormat;
	BOOL			m_bAutoActionMenu;
	BOOL			m_bUseMusicManager;
	BOOL			m_bGlobalMusic;
	BOOL			m_bRandomMusic;

	list<CStdString> m_saMusicDirectories;
	list<CStdString> m_saMusicFiles;
//	CStdString		m_sMusicDirectory;
//	CStdString		m_sAmbientTrack;

	bool			m_bDebugMenu;

	CStdString		m_sActualPath;

	CSimpleSkin		m_skin;

	DWORD			m_dwMoveDelay;
	DWORD			m_dwRepeatDelay;

	DWORD			m_dwLaunchParam;
	LAUNCH_DATA		m_LaunchData;
	int				m_iMasterRandomIndex;

};


extern CMenuInfo g_MenuInfo;





#endif // __MENUINFO_H__
