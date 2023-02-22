/*********************************************************************************\
 * XBStateApp.h
 * Classes used to provide a state-driven Xbox application
 * (C) 2002 MasterMind
\*********************************************************************************/


#ifndef __XBSTATEAPP_H__
#define __XBSTATEAPP_H__

#include "DelayController.h"
#include "XBApp_mxm.h"
#include "XBFont_mxm.h"
#include "MXM.h"
#include "ImageSource.h"
#include "musicmanager.h"
#include "XBInputEx.h"
#include "Font16.h"
#include "keyboard.h"
#include "AppMenu.h"
#ifdef USE_XBFILEZILLA
#include "xbfilezilla.h"
#endif
#include "XKUtils\XKUtils.h"

#include "XBDialog.h"

#include "ActionDraw.h"

class CXBoxStateApplication;

#define DTRI_UP			0
#define DTRI_DN			1
#define DTRI_LF			2
#define DTRI_RT			3

#define DIR_IN_LFTHUMB	0x0001
#define DIR_IN_RTTHUMB	0x0002
#define DIR_IN_DPAD		0x0004
#define DIR_IN_IR		0x0008
// #define DIR_IN_KEYBOARD	0x0010

#define DIR_IN_THUMBS	(DIR_IN_RTTHUMB|DIR_IN_LFTHUMB)
#define DIR_IN_CTRLR	(DIR_IN_THUMBS|DIR_IN_DPAD)
#define DIR_IN_ALL		(DIR_IN_CTRLR|DIR_IN_IR) 


CXBoxStateApplication * GetStateApp( void );


class CXBoxGameState
{
	// Maintain parent application object.
protected:
	CXBoxStateApplication *	m_pbxApplication;
	XKUtils					m_xkUtils;
	int					m_iID;
public:
	CXBoxGameState();
	CXBoxGameState( CXBoxStateApplication * pbxApplication )
	{
		SetParent( pbxApplication );
	};
	XKUtils	*		GetXKUtils( void ) { return &m_xkUtils; };
	virtual bool MusicIsSuspended( void )   { return false; };
	virtual void OnNewSkin( void ) {};
	void	SetParent( CXBoxStateApplication * pbxApplication ) { m_pbxApplication = pbxApplication; };
    virtual HRESULT Initialize()            { return S_OK; }
    virtual HRESULT FrameMove()             { return S_OK; }
    virtual HRESULT Render()                { return S_OK; }
    virtual HRESULT Cleanup()               { return S_OK; }
	void	SetGameStateID( int iID )		{ m_iID = iID; };
	int		GetGameStateID( void )			{ return m_iID; };
	virtual HRESULT OnActivation( void )	{ return S_OK; };
	virtual HRESULT OnDeactivation( void )	{ return S_OK; };
};

class CXBoxStateApplication : public CXBApplication
#ifdef USE_XBFILEZILLA
	, public CXFNotificationClient
#endif
{
protected:
	bool				m_bOneShot;
	CXBoxGameState *	m_pGameStates[MAX_STATES];
	CXBoxGameState *	m_pCurrentGameState;
	int					m_iNumStates;

	CAppMenu		*	m_pAppMenuPtr;
	bool				m_bDebounce;

	CStdString			m_sMessageBox;
	bool				m_bDisplayBox;
    LPDIRECTSOUND8      m_pDSound;
	bool				m_bFirstRender;
	bool				m_bScreenCapture;
	bool				m_bInitialized;
	bool				m_bPersistGrab;
	bool				m_bPersistBlendRender;
	bool				m_bPersistTransition;
	float				m_fBlendAmount;
	DWORD				m_dwBlendTimer;

	CDelayController	m_ctrRtStick;
	CDelayController	m_ctrLfStick;
	CDelayController	m_ctrDpad;
	CDelayController	m_ctrIR;


protected:
	bool	InitializeSoundtrack( void );
	virtual void	FirstRender( void );
	virtual void OnMusicManagerFail( void ) {};
	virtual HRESULT LoadLocalSoundtrack( void );
    virtual HRESULT Initialize();
	virtual void	OnRunLoopTop()			{};
    virtual HRESULT FrameMove();
    virtual HRESULT Render();
    virtual HRESULT Cleanup();
	HRESULT AddGameState( CXBoxGameState * pGameState );

	void	CheckMusicInfo( void );


	bool	m_bInitInputDevices;
	bool	m_bInit3DWorld;
	bool	m_bInitSound;
	bool	m_bInitMusicManager;
	bool	m_bInitDefaultFont;
	bool	m_bInitStates;
	DWORD	m_dwTransitionFrames;

	CStdString m_sCurrentSong;
	CStdString m_sCurrentSoundtrack;
	DWORD	m_dwCurrentSong;
	DWORD	m_dwCurrentSoundtrack;

	bool	m_bMusicPlay;
	bool	m_bRebootAfterMsg;

	virtual bool MusicIsSuspended( void );

public:

	bool				m_bGotInput;
	bool				m_bInActionScript;
	TActionDrawList	*	m_pdrawList;


	virtual void	QueryInput( void );
	void	QueryIRInput( void );

#ifdef USE_XBFILEZILLA
	virtual XFSTATUS CriticalOperationCallback(SXFCriticalOperation Operation) { return XFS_OK; };
	// OnShowStatus is the equivalent of the remote admin main output window
	// i.e. it receives all output text message xbfilezilla produces
	virtual void OnShowStatus(_int64 eventDate, LPCTSTR Msg, enumStatusSource Source) {};
	virtual void OnConnection(int ConnectionId, enumConnectionStatus ConnectionStatus) {};
	// don't store TransferInfo, it's a temporary object
	virtual void OnFileTransfer(SXFTransferInfo* TransferInfo) {};
	virtual void OnServerWillQuit() {};
	// OnIncSendCount and OnIncRecvCount will be called 10 times a second
	// the Count parameter is the total amount of bytes sent/received by all users together
	// in the last 1/10th of a second.
	virtual void OnIncSendCount(int Count) {};
	virtual void OnIncRecvCount(int Count) {};
#endif

	HRESULT		PumpRender( void );
	HRESULT		PumpInput( bool bDoFrameMove = true );
	CXMLNode *	m_pActionData;
	void		ClearActionData( void );
	virtual		CStdString PerformActions( LPCTSTR szActions, HRESULT &hResult );
	virtual		CStdString PerformActionLine( LPCTSTR szActionLine, HRESULT &hResult );
	void		RemoveDialog( CXBDialog * pDialog );
	bool		GetScreenCaptureState( void ) { return m_bScreenCapture; };
	void		SetScreenCapture( void ) { m_bScreenCapture = true; };
	void		SetDebounce( bool bDebounce = true ) {	m_bDebounce = bDebounce; };
	DWORD		GetKeyDir( BYTE byVirtKey, WORD * pwState = NULL );
	DWORD		GetDir( WORD wFlags = DIR_IN_ALL, WORD * pwState = NULL );
	void		SetDelays( DWORD dwMoveDelay, DWORD dwRepeatDelay );

	HRESULT		RenderQuad( LPDIRECT3DTEXTURE8 pTexture, int iScreenWidth, int iScreenHeight, FLOAT fAlpha, FLOAT fDepth = 1.0 );
	HRESULT		RenderTile( LPDIRECT3DTEXTURE8 pTexture, int iScreenWidth, int iScreenHeight, FLOAT fDepth = 1.0 );

	void	SetBlend( bool bEnable = true );
	void	SetGrab(void);
	CImageSource		m_imgPersistImage;

	HRESULT InitInputDevices( void );
	HRESULT Init3DWorld( void );
	HRESULT InitSound( void );
	HRESULT InitMusicManager( void );
	HRESULT InitDefaultFont( void );
	HRESULT InitStates( void );

	void	GetStickValues( int &iStickX, int &iStickY, bool bLeft = true, bool bRight = true, int iPad = -1 );

	char	GetKeyChar( void );
	void	SetNewSkin( void );

	virtual CStdString MusicGetCurrentSong( void );
	virtual CStdString MusicGetCurrentSoundtrack( void );
	virtual void	MusicGetCurrentTrackAndSong( DWORD & dwTrack, DWORD & dwSong );
	virtual void	MusicPause( void );
	virtual void	MusicResume( void );
	virtual void	MusicSetVolume( DWORD dwVolume );
	virtual DWORD	MusicGetVolume( void );
	virtual void	MusicSetRandom( bool bRandom );
	virtual void	MusicSetGlobal( bool bGlobal );
	virtual bool	MusicGetGlobal( void );
	virtual bool	MusicGetRandom( void );
	virtual void	MusicNextSong( void );
	virtual void	MusicStop( void );
	virtual void	MusicInitSongSelection( void );
	void	MusicCheck( void );

	BYTE	GetVirtualKey( void );
    CMusicManager*		m_pMusicManager;    // Music Manager
    LPDIRECTSOUND8      GetSoundDevice( void ) { return m_pDSound; };
	HRESULT LoadTextureFromImageFile( LPCTSTR szFileName, LPDIRECT3DTEXTURE8 *ppD3DTexture );
	DrawingAttributes MessageAttr;
	DrawingAttributes MenuAttr;
#if 0
	struct
	{
		DWORD m_dwBoxColor;
		DWORD m_dwBorderColor;
		DWORD m_dwTextColor;
		DWORD m_dwShadowColor;
	} MessageAttr;
	struct
	{
		DWORD m_dwBoxColor;
		DWORD m_dwBorderColor;
		DWORD m_dwTextColor;
		DWORD m_dwShadowColor;
		DWORD m_dwSelectColor;
		DWORD m_dwValueColor;
		DWORD m_dwSelectValueColor;
	} MenuAttr;
#endif
	void	MessageBoxStr( LPCTSTR szString );
	void	MessageBox( LPCTSTR szString, ... );
	void	RebootMessageBox( LPCTSTR szString, ... );
	void	EnterMenu( CAppMenuHandler * pHandler, _TAppMenuItem * pItemList, long lMenuID );
	bool	AnalogButtonsPressed( int iButton = -1 );
    HRESULT RenderGradientBackground( DWORD dwTopColor, DWORD dwBottomColor )
	{
		return CXBApplication::RenderGradientBackground( dwTopColor, dwBottomColor );
	};
	HRESULT SetGameState( int iID, bool bSetBlend = true, bool bSetGrab = false );
	int	GetGameState( void ) { return m_pCurrentGameState?m_pCurrentGameState->GetGameStateID():(-1); };
	CXBoxStateApplication();
	virtual ~CXBoxStateApplication();
    XBGAMEPAD* GetGamePad( int iID ) { return &m_Gamepad[iID]; };
    XBGAMEPAD* GetDefaultGamepad( void ) { return &m_DefaultGamepad; };

	LPDIRECT3DDEVICE8 Get3dDevice( void )		{ return m_pd3dDevice; };
    LPDIRECT3DSURFACE8  GetBackBuffer( void )	{ return m_pBackBuffer; };
    LPDIRECT3DSURFACE8  GetDepthBuffer( void )	{ return m_pDepthBuffer; };
	LPDIRECT3D8 GetD3D( void )					{ return m_pD3D; };
	D3DPRESENT_PARAMETERS GetD3DParams( void )	{ return m_d3dpp; };

    XDEVICE_PREALLOC_TYPE* GetInputDeviceTypes( void )	{ return m_InputDeviceTypes; };
    DWORD                  GetNumInputDeviceTypes( void ) { return m_dwNumInputDeviceTypes; };

    XBGAMEPAD*             GetGamepad( int iDeviceNum = -1 )
	{
		XBGAMEPAD* pReturn = NULL;

		if ( iDeviceNum == -1 )
		{
			pReturn = &m_DefaultGamepad;
		}
		else
		{
			if ( iDeviceNum > -1 && iDeviceNum < 4 )
			{
				pReturn = &m_Gamepad[iDeviceNum];
			}
		}
		return pReturn;
	};

	FLOAT	GetTime( void )			{ return m_fTime; };		// Current absolute time in seconds
	FLOAT	GetElapsedTime( void )	{ return m_fElapsedTime; };	// Elapsed absolute time since last frame
    FLOAT   GetAppTime( void )		{ return m_fAppTime; };		// Current app time in seconds
    FLOAT	GetElapsedAppTime( void )	{ return m_fElapsedAppTime; };	// Elapsed app time since last frame
    BOOL	IsPaused( void )		{ return m_bPaused; };		// Whether app time is paused by user
    FLOAT	GetFPS( void )			{ return m_fFPS; };			// instantaneous frame rate
    WCHAR*	GetFrameRateString(void)	{ return m_strFrameRate; };		// Frame rate written to a string
    HANDLE  GetFrameRateHandle( void )	{ return m_hFrameCounter; };	// Handle to frame rate perf counter
    VOID    ShowTexture( LPDIRECT3DTEXTURE8 pTexture );
	HRESULT DrawLine( FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2,
                              DWORD dwOutlineColor );

    HRESULT DrawBox( FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2,
                     DWORD dwFillColor, DWORD dwOutlineColor );
	HRESULT DrawTriangle( FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2,
                              DWORD dwFillColor, DWORD dwOutlineColor, 
							  int iDirection  );
	HRESULT RenderQuadGradientBackground( DWORD dwTopLeftColor, 
											DWORD dwTopRightColor, 
											DWORD dwBottomLeftColor, 
                                            DWORD dwBottomRightColor, RECT * pRect = NULL );

	HRESULT DrawMessageBox( LPCTSTR szMessage );
	HRESULT DrawTexture( IDirect3DBaseTexture8 * pTexture, int iPosX, int iPosY, int iSizeX, int iSizeY );
//    CXBPackedResource  m_xprResource;      // Packed resources for the app
    CXBFont            m_Font16;           // 16-point font class
//    CXBFont            m_Font12;           // 12-point font class

    // Active gamepad
    DWORD              m_dwNumInsertedGamepads;
    XBGAMEPAD*         m_pGamepad;

	//IR Remote
	XBIR_REMOTE*						m_IR_Remote;
	XBIR_REMOTE							m_DefaultIR_Remote;
	//IR Remote

	void Reboot( LPCSTR szTitlePath = NULL )
	{
		LAUNCH_DATA  LaunchData;

		memset( &LaunchData, 0, sizeof( LAUNCH_DATA) );
		
		XLaunchNewImage( NULL, &LaunchData );
	};

};



#endif //  __XBSTATEAPP_H__


