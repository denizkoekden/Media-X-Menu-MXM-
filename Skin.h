/*********************************************************************************\
 * Skin.h
 * Class used to abstract the loading of textures.
 * Support for additional formats (video and Flash, for example) will be added 
 * here
 * (C) 2002 MasterMind
\*********************************************************************************/


#ifndef __SKIN_H__
#define __SKIN_H__

#include <XbSound.h>

#include "StdString.h"
#include "ImageSource.h"

typedef struct _TColorBlend
{
    DWORD m_dwUpperLeft;
    DWORD m_dwUpperRight;
    DWORD m_dwLowerLeft;
    DWORD m_dwLowerRight;
} TColorBlend;

typedef struct _TLocation
{
    int	m_iLeft;
    int	m_iTop;
    int	m_iWidth;
    int	m_iHeight;
} TLocation;

typedef struct _TTextAttrSet
{
    DWORD m_dwTextColor;
    DWORD m_dwBackColor;
    DWORD m_dwShadowColor;
    DWORD m_dwBorderColor;
} TTextAttrSet;

class CSimpleSkin
{
protected:
	BOOL	m_bLoaded;
	BOOL	m_bResMade;
	void	ReleaseResources( void );
public:
	CSimpleSkin();
	~CSimpleSkin();

	BOOL MakeResources(LPDIRECT3DDEVICE8 pd3dDevice, LPDIRECTSOUND8 pDSound );
	BOOL Load(LPCTSTR szMainIni, LPCTSTR szSkinIni, bool bXDIMode = false );
    struct _TConfig
    {
        struct _TCfgBackgrounds
        {
            CStdString m_sTitle;
            CStdString m_sLoading;
            CStdString m_sMenu;
            CStdString m_sSaverLogo;
            TColorBlend m_cbTitleScreen;
            TColorBlend m_cbMenuScreen;
            TColorBlend m_cbLoadingScreen;
        } Backgrounds;
        struct _TCfgDelays
        {
            long m_lTitleDelay;
            long m_lSaverDelay;
            long m_lLoadingDelay;
        } Delays;
        struct _TCfgSounds
        {
            CStdString m_sTitleOpen;
            CStdString m_sTitleClose;
            CStdString m_sMenuUp;
            CStdString m_sMenuDown;
            CStdString m_sMenuLaunch;
            CStdString m_sSaverBounce;
        } Sounds;
        struct _TCfgTitle
        {
            TLocation    m_lLocation;
            TTextAttrSet m_attrSet;
        } Title;
        struct _TCfgSubTitle
        {
            TLocation    m_lLocation;
            TTextAttrSet m_attrSet;
            BOOL         m_bAttachToTitle;
        } SubTitle;
        struct _TCfgMenu
        {
            TLocation    m_lLocation;
            TTextAttrSet m_attrSet;
            int          m_iNumItems;
			int			 m_iSelPos;
            TTextAttrSet m_attrSelSet;
//            int          m_iSelPos;
        } Menu;
        struct _TCfgDescription
        {
            TLocation    m_lLocation;
            TTextAttrSet m_attrSet;
			BOOL		 m_bDraw;
        } Description;
        struct _TCfgScreenshot
        {
            TLocation    m_lLocation;
        } Screenshot;
        struct _TCfgOptions
        {
            BOOL    m_bSmoothMenu;
            BOOL    m_bSmoothDescr;
        } Options;
        struct _TCfgMusic
        {
            CStdString    m_sDirectory;
        } Music;
    } Cfg;
#if 1
	struct _TTextures
	{
		CImageSource m_imgTitle;
		CImageSource m_imgMenu;
		CImageSource m_imgLoading;
		CImageSource m_imgSaverLogo;
	} Textures;
	struct _TSounds
	{
        CXBSound	m_sndTitleOpen;
        CXBSound	m_sndTitleClose;
        CXBSound	m_sndMenuUp;
        CXBSound	m_sndMenuDown;
        CXBSound	m_sndMenuLaunch;
        CXBSound	m_sndSaverBounce;
	} Sounds;
#endif
};







#endif