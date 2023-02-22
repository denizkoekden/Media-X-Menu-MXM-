
#include <xtl.h>
#include <list>
#include <XbSound.h>
#include "StdString.h"
#include "xmltree.h"


/*

Modes available:

  RenderToBuffer
  RenderToDevice



  <TextColor>0xFFFFFFFF</TextColor>
  <ShadowColor>0xFF000000</ShadowColor>
  <BackColor>0</BackColor>
  <BorderColor>0</BorderColor>
  <DoGlow>true|false</DoGlow>
  <ShadowSize>2</ShadowSize>


  */

class CTripTimer
{
	long	m_lDelay;
	DWORD	m_dwTimeStamp;
	bool	m_bStarted;
	bool	m_bElapsed;
public:
	void	Reset( void );
	bool	IsElapsed( DWORD dwTimestamp = 0 );
	void	Start( DWORD dwTimeStamp = 0 );
	DWORD	GetEndTime( void ) { return m_dwTimeStamp+m_lDelay; };
	CTripTimer( long lDelay = 0, bool bStart = false );
	void	SetDelay( long lDelay );
};


#define CIDX_ALPHA	0
#define CIDX_RED	1
#define CIDX_GREEN	2
#define CIDX_BLUE	3
#define CIDX_ARGB	-1
#define CIDX_RGB	-2

class CModColor
{
protected:
	bool m_bEnable;
	struct
	{
		bool	m_bEnableColor;
		int		m_iMin;
		int		m_iMax;
		DWORD	m_dwPeriod;
	} m_colorInfo[4];
public:
	CModColor( const CModColor & src );
	CModColor();
	~CModColor();
	void	SetPeriod( int iColorElement, DWORD dwPeriod );
	DWORD	GetPeriod( int iColorElement );
	void	SetColorEnable( int iColorElement, BOOL bEnable );
	BOOL	GetColorEnable( int iColorElement );
	void	SetRange( int iColorElement, int iMin, int iMax );
	void	SetMin( int iColorElement, int iMin );
	void	SetMax( int iColorElement, int iMax );
	int		GetMin( int iColorElement );
	int		GetMax( int iColorElement );
	virtual DWORD ModifyColor( DWORD dwARGB ) { return dwARGB; };
	virtual void InternalConfigure( CXMLNode * pNode ) = 0;
	bool	Configure( CXMLNode * pNode );
	bool	Enabled( void ) { return m_bEnable; };
	void	SetEnable( bool bEnable ) { m_bEnable = bEnable; };
};

class CModClrThrob : public CModColor
{
public:
	CModClrThrob();
	~CModClrThrob();
	virtual DWORD ModifyColor( DWORD dwARGB );
	virtual void InternalConfigure( CXMLNode * pNode );
};

class CModClrFlicker : public CModColor
{
public:
	CModClrFlicker();
	~CModClrFlicker();
	virtual DWORD ModifyColor( DWORD dwARGB );
	virtual void InternalConfigure( CXMLNode * pNode );
};


#define CM_FLICKER	0
#define CM_THROB	1

class CColorModHandler
{
	CModClrFlicker	m_clrModFlicker;
	CModClrThrob	m_clrModThrob;
public:
	CModColor * GetColorMod( int iColorModID );
	CColorModHandler(const CColorModHandler & src );
	CColorModHandler();
	DWORD	ModifyColor( DWORD dwARGB );
	bool	Configure( CXMLNode * pNode, LPCTSTR szTarget );
	bool	Enabled( int iColorModID );
	void	SetEnable( int iColorModID, bool bEnable );
	void	SetPeriod( int iColorModID, int iColorElement, DWORD dwPeriod );
	DWORD	GetPeriod( int iColorModID, int iColorElement );
	void	SetColorEnable( int iColorModID, int iColorElement, BOOL bEnable );
	BOOL	GetColorEnable( int iColorModID, int iColorElement );
	void	SetRange( int iColorModID, int iColorElement, int iMin, int iMax );
	void	SetMin( int iColorModID, int iColorElement, int iMin );
	void	SetMax( int iColorModID, int iColorElement, int iMax );
	int		GetMin( int iColorModID, int iColorElement );
	int		GetMax( int iColorModID, int iColorElement );
	
};

class CModShifter
{
protected:
	long	m_lLength;
	DWORD	m_dwTimeStamp;
	DWORD	m_dwPeriod;
	bool	m_bWrap;
	bool	m_bEnable;
	bool	m_bIncrement;
public:
	CModShifter();
	~CModShifter();
	void InternalConfigure( CXMLNode * pNode );
	void Configure( CXMLNode * pNode );

	long	GetOffset( DWORD dwTimeStamp );
	bool	Enabled( void ) { return m_bEnable; };
	void	SetEnable( bool bEnable ) { m_bEnable = bEnable; };
	bool	GetWrap( void ) { return m_bWrap; };
	void	SynchronizeTimer( DWORD dwTimeStamp ) { m_dwTimeStamp = dwTimeStamp; };
	void	SetLength( long lLength ) { m_lLength = lLength; };
	long	GetLength( void )		{	return m_lLength; };
};



class CModPosition
{
protected:
	bool m_bEnable;
public:
	CModPosition();
	~CModPosition();
	virtual bool ModifyPosRect( RECT & rectInit, RECT & rectResult ) { return false; };
	virtual void InternalConfigure( CXMLNode * pNode ) = 0;
	void Configure( CXMLNode * pNode );
	bool	Enabled( void ) { return m_bEnable; };
	void	SetEnable( bool bEnable ) { m_bEnable = bEnable; };
};

class CModPosBounce : public CModPosition
{
	bool	m_bInitRect;
	int     m_iXVector;
	int     m_iYVector;
	RECT	m_rectBounds;
	RECT	m_rectCurrent;
public:
	CModPosBounce();
	~CModPosBounce();
	void	SetSize( int iWidth, int iHeight );
	virtual bool ModifyPosRect( RECT & rectInit, RECT & rectResult );
	virtual void InternalConfigure( CXMLNode * pNode );
};

class CModPosTremor : public CModPosition
{
	int		m_iOffsetX;
	int		m_iOffsetY;
	int		m_iRangeX;
	int		m_iRangeY;
public:
	CModPosTremor();
	~CModPosTremor();
	virtual bool ModifyPosRect( RECT & rectInit, RECT & rectResult );
	virtual void InternalConfigure( CXMLNode * pNode );
};

class CModSizeSpaz : public CModPosition
{
	int		m_iOffsetX;
	int		m_iOffsetY;
	int		m_iRangeX;
	int		m_iRangeY;
public:
	CModSizeSpaz();
	~CModSizeSpaz();
	virtual bool ModifyPosRect( RECT & rectInit, RECT & rectResult );
	virtual void InternalConfigure( CXMLNode * pNode );
};

class CModSizePulse : public CModPosition
{
	int		m_iOffsetX;
	int		m_iOffsetY;
	int		m_iRangeX;
	int		m_iRangeY;
	DWORD				m_dwThrobPeriod;
public:
	CModSizePulse();
	~CModSizePulse();
	virtual bool ModifyPosRect( RECT & rectInit, RECT & rectResult );
	virtual void InternalConfigure( CXMLNode * pNode );
};

class CTextAttributes
{
public:
	CColorModHandler	m_colorHandlerText;
	CColorModHandler	m_colorHandlerShadow;
	CColorModHandler	m_colorHandlerBack;
	CColorModHandler	m_colorHandlerBorder;
	DWORD		m_dwTextColor;
	DWORD		m_dwShadowColor;
	DWORD		m_dwBackColor;
	DWORD		m_dwBorderColor;
	bool		m_bDoGlow;
	int			m_iShadowSize;
	int			m_iJustify;

	DWORD GetTextColor( void ) { return m_colorHandlerText.ModifyColor( m_dwTextColor ); };
	DWORD GetShadowColor( void ) { return m_colorHandlerShadow.ModifyColor( m_dwShadowColor ); };
	DWORD GetBackColor( void ) { return m_colorHandlerBack.ModifyColor( m_dwBackColor ); };
	DWORD GetBorderColor( void ) { return m_colorHandlerBorder.ModifyColor( m_dwBorderColor ); };

	CTextAttributes();
	CTextAttributes( const CTextAttributes & src );
	void		Configure( CXMLNode * pNode );
};


class CGadgetLayout;

class CRenderGadget
{
protected:
	bool				m_bInLifetime;
	bool				m_bCycleOn;
	CTripTimer			m_tmrDelay;
	CTripTimer			m_tmrLifetime;
	CTripTimer			m_tmrCycleOn;
	CTripTimer			m_tmrCycleOff;
	LPDIRECT3DDEVICE8	m_pDevice;
	LPDIRECT3DTEXTURE8	m_pRenderTexture;
	bool				m_bInit;
	bool				m_bSoundSupport;
	bool				m_bNotCondition;
	int					m_iConditionID;
	int					m_iWidth;
	int					m_iHeight;
	int					m_iTop;
	int					m_iLeft;
	int					m_iOriginX;
	int					m_iOriginY;
	int					m_iJustifyX;
	int					m_iJustifyY;
	bool				m_bScaleX;
	bool				m_bScaleY;
	bool				m_bProportional;
	int					m_iOffsetX;
	int					m_iOffsetY;
	DWORD				m_dwClearAttribute;
	RECT				m_rectDest;
	RECT				m_rectSrc;
	CGadgetLayout *		m_pParent;
	void				UpdateMusicStatus(void);
	void				MakeRects( int iGadgetWidth = -1, int iGadgetHeight = -1 );
	virtual void		InternalInitialize( void );
	virtual void		InternalDestroy( void );
	virtual void		InternalRender( void );
	virtual void		InternalConfigure( CXMLNode * pNode );
	
public:
	CRenderGadget();
	~CRenderGadget();

	void				SetParent( CGadgetLayout * pParent ) { m_pParent = pParent; };
	CGadgetLayout *		GetParent( void ) { return m_pParent; };

	virtual bool		AllowMusic( void ) { return true; };

	virtual bool		SequenceDone( void ) { return false; };
	CRenderGadget * GetPtr( void ) { return this; };
	void _OnActivation( void );
	void _OnDeactivation( void );
	virtual	void OnActivation( void ) {};
	virtual	void OnDeactivation( void ) {};
	virtual void Configure( CXMLNode * pNode );
	void	SetClear( DWORD dwARGB );
	void	Initialize( LPDIRECT3DDEVICE8 pDevice, int iWidth=-1, int iHeight=-1, int iOriginX=-1, int iOriginY=-1 );
	void	Destroy( void );
	LPDIRECT3DTEXTURE8	Render( LPDIRECT3DDEVICE8 pDevice = NULL, bool bRenderToTexture=false );
	bool	HasSoundSupport( void ) { return m_bSoundSupport; };
};

CRenderGadget * CreateGadget( CXMLNode * pNode, CStdString & sGadgetName );
int GetConditionID( LPCTSTR szName );
bool IsConditionMet( int iID, bool bNotCondition, bool bResetDrives = false );

typedef list<CRenderGadget*> TRenderGadgetList;

class CGadgetLayout
{
	LPDIRECT3DDEVICE8	m_pDevice;
	TRenderGadgetList	m_rgl;
	bool				m_bFadeIn;
	bool				m_bMusicEnableCfg;
	bool				m_bMusicEnable;
	bool				m_bMusicTally;
	bool				m_bTalliedMusicEnable;
	DWORD				m_dwRenderTimestamp;
public:
	CGadgetLayout();
	~CGadgetLayout();
	bool	IsFadeIn( void ) { return m_bFadeIn; };
	void	DeleteLayout( void );
	DWORD	GetRenderTimestamp( void );
	bool	IsSequenceEnded( void );
	void	Configure( CXMLNode * pNode );
	void	Render( void );

	bool	AllowMusic( bool bForceCheck = false );
	void	UpdateMusicStatus( void );
	void	OnActivation( void );
	void	OnDeactivation( void );
	void	SetMusicEnable( bool bEnable );
};

class CGadgetSkin
{
	int				m_iSoundVolume;
	bool			m_bSkinLoaded;
	CGadgetLayout	m_glTitle;
	CGadgetLayout	m_glMenu;
	CGadgetLayout	m_glHelp;
	CGadgetLayout	m_glGameHelp;
	CGadgetLayout	m_glLoad;
	CGadgetLayout	m_glSaver;
	CGadgetLayout	m_glInfo01;
	CGadgetLayout	m_glInfo02;
	CGadgetLayout	m_glInfo03;
public:
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
    } SoundSources;
	struct _TSounds
	{
        CXBSound	m_sndTitleOpen;
        CXBSound	m_sndTitleClose;
        CXBSound	m_sndMenuUp;
        CXBSound	m_sndMenuDown;
        CXBSound	m_sndMenuLaunch;
        CXBSound	m_sndSaverBounce;
	} Sounds;

    struct _TCfgMusic
    {
		bool			m_bLocalOnly;
		bool			m_bRandom;
        CStdString		m_sDirectory;
    } Music;

	struct _TCfgInformation
	{
		CStdString	m_sAuthor;
		CStdString	m_sName;
		CStdString	m_sCopyright;
		CStdString	m_sSkinVersion;

		TMapStrings m_msValues;
	} Information;

	int		m_iMasterRandomIndex;

	CGadgetSkin();
	~CGadgetSkin();
	void	SetSoundVolume( int iVolume );
	void	DeleteSkin( void );
	void	Configure( CXMLNode * pNode );
	CGadgetLayout * GetLayout( LPCTSTR szLayout );
};

extern CGadgetSkin g_GadgetSkin;
extern CXMLNode g_SkinNodes;






