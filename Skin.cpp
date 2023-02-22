/*********************************************************************************\
 * Skin.cpp
 * Class used to abstract the loading of textures.
 * Support for additional formats (video and Flash, for example) will be added 
 * here
 * (C) 2002 MasterMind
\*********************************************************************************/

#include <XTL.h>
#include <WinNT.h>

#include "MXM.h"
#include "Skin.h"
#include "utils.h"
#include "XBXmlCfgAccess.h"



CSimpleSkin::CSimpleSkin() :
	m_bLoaded(FALSE),
	m_bResMade(FALSE)
{
}

CSimpleSkin::~CSimpleSkin()
{
	ReleaseResources();
}

void	CSimpleSkin::ReleaseResources( void )
{
	if ( m_bLoaded && m_bResMade )
	{
#if 1
		// Release associated textures
		Textures.m_imgLoading.ReleaseResources();
		Textures.m_imgMenu.ReleaseResources();
		Textures.m_imgSaverLogo.ReleaseResources();
		Textures.m_imgTitle.ReleaseResources();

		// Release associated sounds
		Sounds.m_sndMenuDown.Destroy();
		Sounds.m_sndMenuLaunch.Destroy();
		Sounds.m_sndMenuUp.Destroy();
		Sounds.m_sndSaverBounce.Destroy();
		Sounds.m_sndTitleClose.Destroy();
		Sounds.m_sndTitleOpen.Destroy();
#endif		
		m_bLoaded = FALSE;
		m_bResMade = FALSE;
	}
}

	
BOOL CSimpleSkin::Load(LPCTSTR szMainIni, LPCTSTR szSkinIni, bool bXDIMode )
{
	CStdString sTemp, sValue;
	CStdString sSkinPath, sIniPath;

	sIniPath = StripFilenameFromPath( szMainIni );
	sSkinPath = StripFilenameFromPath( szSkinIni );

	sIniPath += _T("\\");
	sSkinPath += _T("\\");

	

	Cfg.Backgrounds.m_sMenu.Format( _T("%s\r\n%s"), sIniPath.c_str(),sSkinPath.c_str() );
	// release all resources!
	ReleaseResources();

	// Load main ini items first (with defaults)
	if (1)
	{
		CXBXmlCfgAccess iniFile( szMainIni );

		// Determines "true" menu path. Defaults to "D:\" if we can't determine
		// true symbolic path

		Cfg.Title.m_attrSet.m_dwBackColor = 0;
		Cfg.Title.m_attrSet.m_dwBorderColor = 0;
		Cfg.Title.m_attrSet.m_dwShadowColor = 0;
		Cfg.Title.m_attrSet.m_dwTextColor = 0;
		Cfg.Title.m_lLocation.m_iHeight = 0;
		Cfg.Title.m_lLocation.m_iLeft = 0;
		Cfg.Title.m_lLocation.m_iTop = 0;
		Cfg.Title.m_lLocation.m_iWidth = 0;

		Cfg.Backgrounds.m_sTitle = MakeFullFilePath( sIniPath.c_str(), iniFile.GetString( _T("Main"), _T("TitleScreen"), _T("Title.jpg") ));
		if ( !FileExists(Cfg.Backgrounds.m_sTitle) )
		{
			Cfg.Backgrounds.m_sTitle = _T("");
		}
		Cfg.Backgrounds.m_sMenu = MakeFullFilePath( sIniPath.c_str(), iniFile.GetString( _T("Main"), _T("MenuBack"), _T("MenuBack.jpg") ));
		if ( !FileExists(Cfg.Backgrounds.m_sMenu) )
		{
			if ( bXDIMode )
			{
				Cfg.Backgrounds.m_sMenu = _T("D:\\MenuX\\media\\screen.png");
			}
			else
			{
				Cfg.Backgrounds.m_sMenu = _T("");
			}
		}
		Cfg.Backgrounds.m_sSaverLogo = MakeFullFilePath( sIniPath.c_str(), iniFile.GetString( _T("Main"), _T("SaverLogo"), _T("SaverLogo.png") ));
		if ( !FileExists(Cfg.Backgrounds.m_sSaverLogo) )
		{
			Cfg.Backgrounds.m_sSaverLogo = _T("");
		}
		Cfg.Backgrounds.m_sLoading = MakeFullFilePath( sIniPath.c_str(), iniFile.GetString( _T("Main"), _T("WaitScreen"), _T("wait.jpg") ));
		if ( !FileExists(Cfg.Backgrounds.m_sLoading) )
		{
			if ( bXDIMode )
			{
				Cfg.Backgrounds.m_sLoading = _T("D:\\MenuX\\media\\wait.png");
			}
			else
			{
				Cfg.Backgrounds.m_sLoading = _T("");
			}
		}

		// Don't multiply these yet....
		Cfg.Delays.m_lTitleDelay = iniFile.GetLong( _T("Main"), _T("TitleDelay"), 20 ); 
		Cfg.Delays.m_lSaverDelay = iniFile.GetLong( _T("Main"), _T("SaverDelay"), 300 );

		Cfg.Screenshot.m_lLocation.m_iTop = iniFile.GetLong( _T("ScreenShot"), _T("Top"), 120 );
		Cfg.Screenshot.m_lLocation.m_iLeft = iniFile.GetLong( _T("ScreenShot"), _T("Left"), 334 );
		Cfg.Screenshot.m_lLocation.m_iWidth = iniFile.GetLong( _T("ScreenShot"), _T("Width"), 260 );
		Cfg.Screenshot.m_lLocation.m_iHeight = iniFile.GetLong( _T("ScreenShot"), _T("Height"), 193 );

		Cfg.Menu.m_lLocation.m_iTop = iniFile.GetLong( _T("Menu"), _T("Top"), 100 );
		Cfg.Menu.m_lLocation.m_iLeft = iniFile.GetLong( _T("Menu"), _T("Left"), 63 );
		Cfg.Menu.m_lLocation.m_iWidth = iniFile.GetLong( _T("Menu"), _T("Width"), 230 );
		Cfg.Menu.m_lLocation.m_iHeight = iniFile.GetLong( _T("Menu"), _T("Height"), 200 );

		Cfg.Menu.m_iNumItems = iniFile.GetLong( _T("Menu"), _T("Items"), 7 );
		if ( Cfg.Menu.m_iNumItems < 1 )
		{
			Cfg.Menu.m_iNumItems = 1;
		}
		if ( Cfg.Menu.m_iNumItems > 15 )
		{
			Cfg.Menu.m_iNumItems = 15;
		}

//		Cfg.Menu.m_iSelPos = iniFile.GetLong( _T("Menu"), _T("SelPos"), Cfg.Menu.m_iNumItems/2 );
//		if ( Cfg.Menu.m_iSelPos < 0 )
//		{
//			Cfg.Menu.m_iSelPos = 0;
//		}
//		if ( Cfg.Menu.m_iSelPos >= Cfg.Menu.m_iNumItems )
//		{
//			Cfg.Menu.m_iSelPos = Cfg.Menu.m_iNumItems-1;
//		}

		Cfg.Menu.m_attrSet.m_dwBorderColor = iniFile.GetLong( _T("Menu"), _T("MenuBorderColor"), 0xa04444ff );
		Cfg.Menu.m_attrSet.m_dwBackColor = iniFile.GetLong( _T("Menu"), _T("MenuBackColor"), 0x80000000 );
		Cfg.Menu.m_attrSet.m_dwShadowColor = iniFile.GetBool( _T("Menu"), _T("ShadowItem"), TRUE )?0xff000000:0;
		Cfg.Menu.m_attrSet.m_dwTextColor = iniFile.GetDWORD( _T("Menu"), _T("ItemColor"), 0xa0808080 );

		Cfg.Menu.m_attrSelSet.m_dwShadowColor = iniFile.GetBool( _T("Menu"), _T("ShadowSelItem"), TRUE )?0xff000000:0;
		Cfg.Menu.m_attrSelSet.m_dwTextColor = iniFile.GetDWORD( _T("Menu"), _T("SelItemColor"), 0xffffffff );

		Cfg.Description.m_attrSet.m_dwTextColor = iniFile.GetDWORD( _T("Menu"), _T("DescrColor"), 0xffffffff );
		Cfg.Description.m_attrSet.m_dwShadowColor = iniFile.GetBool( _T("Menu"), _T("ShadowDescr"), TRUE )?0xff000000:0;

		Cfg.Description.m_lLocation.m_iTop = iniFile.GetLong( _T("Descr"), _T("Top"), -1 );
		Cfg.Description.m_lLocation.m_iLeft = iniFile.GetLong( _T("Descr"), _T("Left"), -1 );
		Cfg.Description.m_lLocation.m_iHeight = iniFile.GetLong( _T("Descr"), _T("Height"), -1 );
		Cfg.Description.m_lLocation.m_iWidth = iniFile.GetLong( _T("Descr"), _T("Width"), -1 );
		Cfg.Options.m_bSmoothDescr = iniFile.GetBool( _T("Descr"), _T("SmoothScroll"), TRUE ); 

		Cfg.Title.m_attrSet.m_dwTextColor = iniFile.GetDWORD( _T("Main"), _T("TitleColor"), 0xffffffff );
		Cfg.Title.m_attrSet.m_dwShadowColor = iniFile.GetBool( _T("Main"), _T("ShadowTitle"), TRUE )?0xff000000:0;

		Cfg.SubTitle.m_attrSet.m_dwTextColor = iniFile.GetDWORD( _T("Main"), _T("SubTitleColor"), 0xffffffff );
		Cfg.SubTitle.m_attrSet.m_dwShadowColor = iniFile.GetBool( _T("Main"), _T("ShadowSubTitle"), TRUE )?0xff000000:0;

		Cfg.Description.m_bDraw = iniFile.GetBool( _T("Menu"), _T("DrawDescription"), TRUE );


		if ( bXDIMode )
		{
			sTemp = _T("D:\\MenuX\\media\\menu.wav");
		}
		else
		{
			sTemp;
		}
		sTemp = MakeFullFilePath( sIniPath.c_str(), iniFile.GetString( _T("Menu"), _T("SndUp"), sTemp ) );
		if ( FileExists(sTemp.c_str()) )
		{
			Cfg.Sounds.m_sMenuUp = sTemp;
		}

		if ( bXDIMode )
		{
			sTemp = _T("D:\\MenuX\\media\\menu.wav");
		}
		else
		{
			sTemp;
		}
		sTemp = MakeFullFilePath( sIniPath.c_str(), iniFile.GetString( _T("Menu"), _T("SndDn"), sTemp ) );
		if ( FileExists(sTemp.c_str()) )
		{
			Cfg.Sounds.m_sMenuDown = sTemp;
			if ( Cfg.Sounds.m_sMenuUp.GetLength() == 0 )
			{
				Cfg.Sounds.m_sMenuUp = Cfg.Sounds.m_sMenuDown;
			}
		}
		else
		{
			if ( Cfg.Sounds.m_sMenuUp.GetLength()  )
			{
				Cfg.Sounds.m_sMenuDown = Cfg.Sounds.m_sMenuUp;
			}
		}

		if ( bXDIMode )
		{
			sTemp = _T("D:\\MenuX\\media\\launch.wav");
		}
		else
		{
			sTemp;
		}
		sTemp = MakeFullFilePath( sIniPath.c_str(), iniFile.GetString( _T("Menu"), _T("SndLaunch"), sTemp ) );
		if ( FileExists(sTemp.c_str()) )
		{
			Cfg.Sounds.m_sMenuLaunch = sTemp;
		}
		
	}

	// Legacy Defaults now loaded

	// Now load from skin
	if ( 1 )
	{
//		Cfg.Backgrounds.m_sMenu.Format( _T("%s Exists=%d"),szSkinIni, FileExists(szSkinIni) );
		CXBXmlCfgAccess iniFile( szSkinIni, TRUE );

//		Cfg.Backgrounds.m_sMenu  = iniFile.GetString( _T("Backgrounds"), _T("Menu"), _T("xxMenu.jpg") );
		//Cfg.Backgrounds.m_sMenu  = MakeFullFilePath( sSkinPath.c_str(), iniFile.GetString( _T("Backgrounds"), _T("Menu"), _T("Menu.jpg") ));

		sTemp = MakeFullFilePath( sSkinPath.c_str(), iniFile.GetString( _T("Backgrounds"), _T("Menu"), _T("Menu.jpg") ), TRUE );
		if ( FileExists(sTemp) )
		{
			Cfg.Backgrounds.m_sMenu = sTemp;
		}

		sTemp = MakeFullFilePath( sSkinPath.c_str(), iniFile.GetString( _T("Backgrounds"), _T("Title"), _T("Title.jpg") ), TRUE );
		if ( FileExists(sTemp) )
		{
			Cfg.Backgrounds.m_sTitle = sTemp;
		}
		sTemp = MakeFullFilePath( sSkinPath.c_str(), iniFile.GetString( _T("Backgrounds"), _T("SaverLogo"), _T("SaverLogo.png") ), TRUE );
		if ( FileExists(sTemp) )
		{
			Cfg.Backgrounds.m_sSaverLogo = sTemp;
		}
		sTemp = MakeFullFilePath( sSkinPath.c_str(), iniFile.GetString( _T("Backgrounds"), _T("Loading"), _T("Loading.jpg") ), TRUE );
		if ( FileExists(sTemp) )
		{
			Cfg.Backgrounds.m_sLoading = sTemp;
		}

		Cfg.Backgrounds.m_cbLoadingScreen.m_dwUpperLeft = iniFile.GetDWORD( _T("Backgrounds"), _T("LoadingColorUL"), 0 );
		Cfg.Backgrounds.m_cbLoadingScreen.m_dwUpperRight = iniFile.GetDWORD( _T("Backgrounds"), _T("LoadingColorUR"), 0 );
		Cfg.Backgrounds.m_cbLoadingScreen.m_dwLowerLeft = iniFile.GetDWORD( _T("Backgrounds"), _T("LoadingColorLL"), 0 );
		Cfg.Backgrounds.m_cbLoadingScreen.m_dwLowerRight = iniFile.GetDWORD( _T("Backgrounds"), _T("LoadingColorLR"), 0 );
		Cfg.Backgrounds.m_cbMenuScreen.m_dwUpperLeft = iniFile.GetDWORD( _T("Backgrounds"), _T("MenuColorUL"), 0 );
		Cfg.Backgrounds.m_cbMenuScreen.m_dwUpperRight = iniFile.GetDWORD( _T("Backgrounds"), _T("MenuColorUR"), 0 );
		Cfg.Backgrounds.m_cbMenuScreen.m_dwLowerLeft = iniFile.GetDWORD( _T("Backgrounds"), _T("MenuColorLL"), 0 );
		Cfg.Backgrounds.m_cbMenuScreen.m_dwLowerRight = iniFile.GetDWORD( _T("Backgrounds"), _T("MenuColorLR"), 0 );
		Cfg.Backgrounds.m_cbTitleScreen.m_dwUpperLeft = iniFile.GetDWORD( _T("Backgrounds"), _T("TitleColorUL"), 0 );
		Cfg.Backgrounds.m_cbTitleScreen.m_dwUpperRight = iniFile.GetDWORD( _T("Backgrounds"), _T("TitleColorUR"), 0 );
		Cfg.Backgrounds.m_cbTitleScreen.m_dwLowerLeft = iniFile.GetDWORD( _T("Backgrounds"), _T("TitleColorLL"), 0 );
		Cfg.Backgrounds.m_cbTitleScreen.m_dwLowerRight = iniFile.GetDWORD( _T("Backgrounds"), _T("TitleColorLR"), 0 );

		Cfg.Delays.m_lLoadingDelay = iniFile.GetDWORD( _T("Delays"), _T("LoadingDelay"), 4 );
		Cfg.Delays.m_lSaverDelay = iniFile.GetLong( _T("Delays"), _T("SaverDelay"), Cfg.Delays.m_lSaverDelay );
		Cfg.Delays.m_lTitleDelay = iniFile.GetLong( _T("Delays"), _T("TitleDelay"), Cfg.Delays.m_lTitleDelay );

		Cfg.Description.m_attrSet.m_dwBackColor = iniFile.GetDWORD( _T("Descr"), _T("BackColor"), Cfg.Description.m_attrSet.m_dwBackColor );
		Cfg.Description.m_attrSet.m_dwBorderColor = iniFile.GetDWORD( _T("Descr"), _T("BorderColor"), Cfg.Description.m_attrSet.m_dwBorderColor );
		Cfg.Description.m_attrSet.m_dwShadowColor = iniFile.GetDWORD( _T("Descr"), _T("ShadowColor"), Cfg.Description.m_attrSet.m_dwShadowColor );
		Cfg.Description.m_attrSet.m_dwTextColor = iniFile.GetDWORD( _T("Descr"), _T("TextColor"), Cfg.Description.m_attrSet.m_dwTextColor );

		Cfg.Description.m_bDraw = iniFile.GetBool( _T("Descr"), _T("DrawDescr"), Cfg.Description.m_bDraw );
		Cfg.Description.m_lLocation.m_iHeight = iniFile.GetLong( _T("Descr"), _T("Height"), Cfg.Description.m_lLocation.m_iHeight );
		Cfg.Description.m_lLocation.m_iLeft = iniFile.GetLong( _T("Descr"), _T("Left"), Cfg.Description.m_lLocation.m_iLeft );
		Cfg.Description.m_lLocation.m_iTop = iniFile.GetLong( _T("Descr"), _T("Top"), Cfg.Description.m_lLocation.m_iTop );
		Cfg.Description.m_lLocation.m_iWidth = iniFile.GetLong( _T("Descr"), _T("Width"), Cfg.Description.m_lLocation.m_iWidth );

		Cfg.Menu.m_attrSet.m_dwBackColor = iniFile.GetDWORD( _T("Menu"), _T("BackColor"), Cfg.Menu.m_attrSet.m_dwBackColor );
		Cfg.Menu.m_attrSet.m_dwBorderColor = iniFile.GetDWORD( _T("Menu"), _T("BorderColor"), Cfg.Menu.m_attrSet.m_dwBorderColor );
		Cfg.Menu.m_attrSet.m_dwShadowColor = iniFile.GetDWORD( _T("Menu"), _T("ShadowColor"), Cfg.Menu.m_attrSet.m_dwShadowColor );
		Cfg.Menu.m_attrSet.m_dwTextColor = iniFile.GetDWORD( _T("Menu"), _T("TextColor"), Cfg.Menu.m_attrSet.m_dwTextColor );

		// These default to whatever the menu items were, if not specified
		Cfg.Menu.m_attrSelSet.m_dwBackColor = iniFile.GetDWORD( _T("Menu"), _T("SelBackColor"), Cfg.Menu.m_attrSet.m_dwBackColor );
		Cfg.Menu.m_attrSelSet.m_dwBorderColor = iniFile.GetDWORD( _T("Menu"), _T("SelBorderColor"), Cfg.Menu.m_attrSet.m_dwBorderColor );

		Cfg.Menu.m_attrSelSet.m_dwShadowColor = iniFile.GetDWORD( _T("Menu"), _T("SelShadowColor"), Cfg.Menu.m_attrSelSet.m_dwShadowColor );
		Cfg.Menu.m_attrSelSet.m_dwTextColor = iniFile.GetDWORD( _T("Menu"), _T("SelTextColor"), Cfg.Menu.m_attrSelSet.m_dwTextColor );

//		sValue = iniFile.GetString( _T("Menu"), _T("SelItemColor"), _T("N/A") );
//		sTemp.Format( _T("MXM.ini SelItemColor = 0x%08x (%s)"),Cfg.Menu.m_attrSelSet.m_dwTextColor, sValue.c_str() );
//		DEBUG_LINE( sTemp.c_str() );

		Cfg.Menu.m_iNumItems = iniFile.GetLong( _T("Menu"), _T("NumItems"), Cfg.Menu.m_iNumItems );
		Cfg.Menu.m_iSelPos = iniFile.GetLong( _T("Menu"), _T("SelPos"), (Cfg.Menu.m_iNumItems-1)/2 );
		if ( Cfg.Menu.m_iSelPos < 0 )
		{
			Cfg.Menu.m_iSelPos = 0;
		}
		if ( Cfg.Menu.m_iSelPos >= Cfg.Menu.m_iNumItems )
		{
			Cfg.Menu.m_iSelPos = Cfg.Menu.m_iNumItems-1;
		}

		Cfg.Menu.m_lLocation.m_iHeight = iniFile.GetLong( _T("Menu"), _T("Height"), Cfg.Menu.m_lLocation.m_iHeight );
		Cfg.Menu.m_lLocation.m_iLeft = iniFile.GetLong( _T("Menu"), _T("Left"), Cfg.Menu.m_lLocation.m_iLeft );
		Cfg.Menu.m_lLocation.m_iTop = iniFile.GetLong( _T("Menu"), _T("Top"), Cfg.Menu.m_lLocation.m_iTop );
		Cfg.Menu.m_lLocation.m_iWidth = iniFile.GetLong( _T("Menu"), _T("Width"), Cfg.Menu.m_lLocation.m_iWidth );

		Cfg.Music.m_sDirectory = iniFile.GetString( _T("Music"), _T("Dir"), _T("") );


		Cfg.Options.m_bSmoothDescr = iniFile.GetBool( _T("Options"), _T("SmoothDescr"), Cfg.Options.m_bSmoothDescr );
		Cfg.Options.m_bSmoothMenu = iniFile.GetBool( _T("Options"), _T("SmoothMenu"), TRUE );

		Cfg.Screenshot.m_lLocation.m_iHeight = iniFile.GetLong( _T("ScreenShot"), _T("Height"), Cfg.Screenshot.m_lLocation.m_iHeight );
		Cfg.Screenshot.m_lLocation.m_iLeft = iniFile.GetLong( _T("ScreenShot"), _T("Left"), Cfg.Screenshot.m_lLocation.m_iLeft );
		Cfg.Screenshot.m_lLocation.m_iTop = iniFile.GetLong( _T("ScreenShot"), _T("Top"), Cfg.Screenshot.m_lLocation.m_iTop );
		Cfg.Screenshot.m_lLocation.m_iWidth = iniFile.GetLong( _T("ScreenShot"), _T("Width"), Cfg.Screenshot.m_lLocation.m_iWidth );

		sTemp = MakeFullFilePath( sSkinPath.c_str(), iniFile.GetString( _T("Sounds"), _T("MenuUp"), _T("") ) );
		if ( FileExists(sTemp.c_str()) )
		{
			Cfg.Sounds.m_sMenuUp = sTemp;
		}
		
		sTemp = MakeFullFilePath( sSkinPath.c_str(), iniFile.GetString( _T("Sounds"), _T("MenuDown"), _T("") ) );
		if ( FileExists(sTemp.c_str()) )
		{
			Cfg.Sounds.m_sMenuDown = sTemp;
		}
		sTemp = MakeFullFilePath( sSkinPath.c_str(), iniFile.GetString( _T("Sounds"), _T("MenuLaunch"), _T("") ) );
		if ( FileExists(sTemp.c_str()) )
		{
			Cfg.Sounds.m_sMenuLaunch = sTemp;
		}
		sTemp = MakeFullFilePath( sSkinPath.c_str(), iniFile.GetString( _T("Sounds"), _T("SaverBounce"), _T("") ) );
		if ( FileExists(sTemp.c_str()) )
		{
			Cfg.Sounds.m_sSaverBounce = sTemp;
		}
		sTemp = MakeFullFilePath( sSkinPath.c_str(), iniFile.GetString( _T("Sounds"), _T("TitleClose"), _T("") ) );
		if ( FileExists(sTemp.c_str()) )
		{
			Cfg.Sounds.m_sTitleClose = sTemp;
		}
		sTemp = MakeFullFilePath( sSkinPath.c_str(), iniFile.GetString( _T("Sounds"), _T("TitleOpen"), _T("") ) );
		if ( FileExists(sTemp.c_str()) )
		{
			Cfg.Sounds.m_sTitleOpen = sTemp;
		}

		Cfg.SubTitle.m_attrSet.m_dwBackColor = iniFile.GetDWORD( _T("SubTitle"), _T("BackColor"), 0 );
		Cfg.SubTitle.m_attrSet.m_dwBorderColor = iniFile.GetDWORD( _T("SubTitle"), _T("BorderColor"), 0 );
		Cfg.SubTitle.m_attrSet.m_dwShadowColor = iniFile.GetDWORD( _T("SubTitle"), _T("ShadowColor"), Cfg.SubTitle.m_attrSet.m_dwShadowColor );
		Cfg.SubTitle.m_attrSet.m_dwTextColor = iniFile.GetDWORD( _T("SubTitle"), _T("TextColor"), Cfg.SubTitle.m_attrSet.m_dwTextColor );

		Cfg.SubTitle.m_bAttachToTitle = iniFile.GetBool( _T("SubTitle"), _T("AttachToTitle"), TRUE );

		Cfg.SubTitle.m_lLocation.m_iHeight = iniFile.GetLong( _T("SubTitle"), _T("Height"), -1 );
		Cfg.SubTitle.m_lLocation.m_iLeft = iniFile.GetLong( _T("SubTitle"), _T("Left"), -1 );
		Cfg.SubTitle.m_lLocation.m_iTop = iniFile.GetLong( _T("SubTitle"), _T("Top"), -1 );
		Cfg.SubTitle.m_lLocation.m_iWidth = iniFile.GetLong( _T("SubTitle"), _T("Width"), -1 );

		Cfg.Title.m_attrSet.m_dwBackColor = iniFile.GetDWORD( _T("Title"), _T("BackColor"), 0 );
		Cfg.Title.m_attrSet.m_dwBorderColor = iniFile.GetDWORD( _T("Title"), _T("BorderColor"), 0 );
		Cfg.Title.m_attrSet.m_dwShadowColor = iniFile.GetDWORD( _T("Title"), _T("ShadowColor"), Cfg.Title.m_attrSet.m_dwShadowColor );
		Cfg.Title.m_attrSet.m_dwTextColor = iniFile.GetDWORD( _T("Title"), _T("TextColor"), Cfg.Title.m_attrSet.m_dwTextColor );

		
		Cfg.Title.m_lLocation.m_iHeight = iniFile.GetLong( _T("Title"), _T("Height"), -1 );
		Cfg.Title.m_lLocation.m_iLeft = iniFile.GetLong( _T("Title"), _T("Left"), -1 );
		Cfg.Title.m_lLocation.m_iTop = iniFile.GetLong( _T("Title"), _T("Top"), -1 );
		Cfg.Title.m_lLocation.m_iWidth = iniFile.GetLong( _T("Title"), _T("Width"), -1 );
	}

	Cfg.Delays.m_lLoadingDelay *= 1000;
	Cfg.Delays.m_lSaverDelay *= 1000;
	Cfg.Delays.m_lTitleDelay *= 1000;
	return m_bLoaded;
}

BOOL CSimpleSkin::MakeResources(LPDIRECT3DDEVICE8 pd3dDevice, LPDIRECTSOUND8 pDSound )
{
	if ( m_bResMade == FALSE )
	{
//		DEBUG_LINE( _T("Making Resources") );

		// Now, load files
		if ( Cfg.Backgrounds.m_sMenu.GetLength() )
		{
//			DEBUG_LINE( Cfg.Backgrounds.m_sMenu.c_str() );
			Textures.m_imgMenu.Initialize(pd3dDevice, pDSound, Cfg.Backgrounds.m_sMenu.c_str() );
		}
		if ( Cfg.Backgrounds.m_sLoading.GetLength() )
		{
//			DEBUG_LINE( Cfg.Backgrounds.m_sLoading.c_str() );
			Textures.m_imgLoading.Initialize(pd3dDevice, pDSound, Cfg.Backgrounds.m_sLoading.c_str() );
		}
		if ( Cfg.Backgrounds.m_sSaverLogo.GetLength() )
		{
			Textures.m_imgSaverLogo.Initialize(pd3dDevice, pDSound, Cfg.Backgrounds.m_sSaverLogo.c_str() );
		}
		if ( Cfg.Backgrounds.m_sTitle.GetLength() )
		{
//			DEBUG_LINE( Cfg.Backgrounds.m_sTitle.c_str() );
			Textures.m_imgTitle.Initialize(pd3dDevice, pDSound, Cfg.Backgrounds.m_sTitle.c_str() );
		}

		if ( Cfg.Sounds.m_sMenuUp.GetLength() )
		{
			Sounds.m_sndMenuUp.Create(Cfg.Sounds.m_sMenuUp);
		}
		if ( Cfg.Sounds.m_sMenuDown.GetLength() )
		{
			Sounds.m_sndMenuDown.Create(Cfg.Sounds.m_sMenuDown);
		}
		if ( Cfg.Sounds.m_sMenuLaunch.GetLength() )
		{
			Sounds.m_sndMenuLaunch.Create(Cfg.Sounds.m_sMenuLaunch);
		}
		if ( Cfg.Sounds.m_sSaverBounce.GetLength() )
		{
			Sounds.m_sndSaverBounce.Create(Cfg.Sounds.m_sSaverBounce);
		}
		if ( Cfg.Sounds.m_sTitleOpen.GetLength() )
		{
			Sounds.m_sndTitleOpen.Create(Cfg.Sounds.m_sTitleOpen);
		}
		if ( Cfg.Sounds.m_sTitleClose.GetLength() )
		{
			Sounds.m_sndTitleClose.Create(Cfg.Sounds.m_sTitleClose);
		}
		m_bResMade = TRUE;
	}
	return m_bResMade;
}
