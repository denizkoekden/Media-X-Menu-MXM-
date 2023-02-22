/*********************************************************************************\
 * XBStateApp.cpp
 * Classes used to provide a state-driven Xbox application
 * (C) 2002 MasterMind
\*********************************************************************************/


#include "XBApp_mxm.h"
#include "XBFont_mxm.h"
#include <XBMesh.h>
#include <XBUtil.h>
#include "XBResource_mxm.h"
#define DEBUG_KEYBOARD
#include <xkbd.h>
#include <xgraphics.h>
#include <XbSound.h>
#include <WinNT.h>
#include "Resource.h"
#include "StdString.h"
// #include "XBIniCfgAccess.h"
//#include "XBXmlCfgAccess.h"
#include "wmvplayer.h"
#include "musicmanager.h"
#include "xntdll.h"
#include "MXM.h"
#include "utils.h"
#include "keyboard.h"
#include "ImageSource.h"
#include "XBInputEx.h"
#include "XBStateApp.h"
#include "Font16.h"
#include "CommDebug.h"


CXBoxStateApplication * g_MainApp = NULL;


CXBoxStateApplication * GetStateApp( void )
{
	return g_MainApp;
}


CXBoxStateApplication::CXBoxStateApplication() :
	CXBApplication(),
	m_bOneShot(false),
	m_bRebootAfterMsg(false),
	m_bDisplayBox(false),
	m_bDebounce(false),
    m_pMusicManager(NULL),
	m_iNumStates(0),
	m_bScreenCapture(false),
	m_bFirstRender(true),
	m_pCurrentGameState(NULL),
	m_bInitialized(false),
	m_bInitInputDevices(false),
	m_bInit3DWorld(false),
	m_bInitSound(false),
	m_bInitMusicManager(false),
	m_bInitDefaultFont(false),
	m_bInitStates(false),
	m_bPersistBlendRender(false),
	m_bPersistGrab(false),
	m_bPersistTransition(false),
	m_bInActionScript(false),
	m_pdrawList(NULL),
	m_fBlendAmount(0.0),
	m_pDSound(NULL),
	m_IR_Remote(NULL),
	m_ctrRtStick(700,200),
	m_ctrLfStick(700,200),
	m_ctrDpad(700,200),
	m_dwCurrentSong(0xfffffff),
	m_dwCurrentSoundtrack(0xffffff),
	m_ctrIR(7000,2000),
	m_bMusicPlay(false),
	m_pAppMenuPtr(NULL),
	m_pActionData(NULL),
	m_sMessageBox(_T(""))
{
	MessageAttr.m_dwBoxColor = 0x80000000;
	MessageAttr.m_dwBorderColor = 0xffffff00;
	MessageAttr.m_dwTextColor = 0xffffffff;
	MessageAttr.m_dwShadowColor = 0xff000000;
	MenuAttr.m_dwBoxColor = 0x80000000;
	MenuAttr.m_dwBorderColor = 0xffffff00;
	MenuAttr.m_dwTextColor = 0xff909090;
	MenuAttr.m_dwShadowColor = 0xff000000;
	MenuAttr.m_dwSelectColor = 0xffffffff;
	MenuAttr.m_dwValueColor = 0xff000090;
	MenuAttr.m_dwSelectValueColor = 0xffc0c0ff;
	g_MainApp = this;
}

CXBoxStateApplication::~CXBoxStateApplication()
{
	int iIndex=0;
	if ( m_pActionData )
	{
		delete m_pActionData;
		m_pActionData = NULL;
	}
	for( iIndex=0; iIndex<m_iNumStates; iIndex++ )
	{
		delete m_pGameStates[iIndex];
	}
	if ( m_pMusicManager )
	{
		delete m_pMusicManager;
		m_pMusicManager = NULL;
	}
}

void CXBoxStateApplication::ClearActionData( void )
{
	if ( m_pActionData )
	{
		delete m_pActionData;
		m_pActionData = NULL;
	}
	if ( m_pActionData == NULL )
	{
		m_pActionData = new CXMLNode( NULL, _T("actiondata") );
	}
}

CStdString CXBoxStateApplication::PerformActions( LPCTSTR szActions, HRESULT &hResult )
{
	CStdString sReturn;

	// Parse out actions line by line and process individually
	CStdString sActions(szActions);
	CStdString sActionLine;
	int iPos;

	hResult = S_OK;

	// Clean up action lines for easier parsing
	sActions.Replace( _T("\r\n"), _T("\r") );
	sActions.Replace( _T("\n"), _T("\r") );
	sActions.Replace( _T("\r\r"), _T("\r") );
	sActions.Replace( _T("\r\r"), _T("\r") );
	sActions.Trim();

	// Reset data area for action data
	ClearActionData();

	while( sActions.GetLength() )
	{
		iPos = sActions.Find( _T('\r') );
		if ( iPos >= 0 )
		{
			sActionLine = sActions.Left( iPos );
			sActions = sActions.Mid( iPos+1 );
		}
		else
		{
			sActionLine = sActions;
			sActions = _T("");
		}
		sActionLine.Trim();
		if ( sActionLine.GetLength() )
		{
			sReturn += PerformActionLine( sActionLine, hResult );
			if ( FAILED( hResult ) )
			{
				break;
			}
		}
	}
	return sReturn;
}

CStdString CXBoxStateApplication::PerformActionLine( LPCTSTR szActionLine, HRESULT &hResult )
{
	CStdString sReturn;

	hResult = S_OK;
	// Parse out action line

	return sReturn;
}


void	CXBoxStateApplication::MusicNextSong( void )
{
	if ( m_pMusicManager )
	{
		if ( m_pMusicManager->GetStatus() == MM_PLAYING )
		{
			m_pMusicManager->Stop();
			m_pMusicManager->SetNextSong();
			m_pMusicManager->Play();
		}
		else
		{
			m_pMusicManager->SetNextSong();
		}
	}
}

void CXBoxStateApplication::MusicCheck( void )
{
	if ( m_pMusicManager )
	{
		if ( MusicIsSuspended() )
		{
			if ( m_pMusicManager->GetStatus() == MM_PLAYING )
			{
				MusicPause();
			}
		}
		else
		{
			if ( m_pMusicManager->GetStatus() != MM_PLAYING )
			{
				MusicResume();
			}
		}
	}
}

bool CXBoxStateApplication::MusicIsSuspended( void )
{
	bool bReturn = false;

	if ( m_pCurrentGameState )
	{
		bReturn = m_pCurrentGameState->MusicIsSuspended();
	}
	return bReturn;
}


void	CXBoxStateApplication::MusicSetVolume( DWORD dwVolume )
{
	if ( m_pMusicManager )
	{
		long lRealVolume = 0;
		if ( dwVolume > 10 )
		{
			dwVolume = 64;
		}
		else
		{
			dwVolume *= 64;
			dwVolume /= 10;
		}
		lRealVolume = 64-dwVolume;
		if ( lRealVolume > 64  )
		{
			lRealVolume = 64;
		}
		lRealVolume *= -100;
		m_pMusicManager->SetVolume(lRealVolume);
	}
}

DWORD	CXBoxStateApplication::MusicGetVolume( void )
{
	DWORD dwReturn = 0;

	if ( m_pMusicManager )
	{
		int lRealVolume;
		lRealVolume = m_pMusicManager->GetVolume();
		lRealVolume /= -100;
		if ( lRealVolume > 64 )
		{
			lRealVolume = 64;
		}
		if ( lRealVolume < 0 )
		{
			lRealVolume = 0;
		}
		dwReturn = 64-lRealVolume;
		dwReturn *= 10;
		dwReturn /= 64;
	}
	return dwReturn;
}

void	CXBoxStateApplication::MusicSetRandom( bool bRandom )
{
	if ( m_pMusicManager )
	{
		m_pMusicManager->SetRandom(bRandom?TRUE:FALSE);
	}
}

void	CXBoxStateApplication::MusicSetGlobal( bool bGlobal )
{
	if ( m_pMusicManager )
	{
		m_pMusicManager->SetGlobal(bGlobal?TRUE:FALSE);
	}
}

bool	CXBoxStateApplication::MusicGetGlobal( void )
{
	bool bReturn = false;

	if ( m_pMusicManager )
	{
		bReturn = (m_pMusicManager->GetGlobal()==TRUE);
	}
	return bReturn;
}

bool	CXBoxStateApplication::MusicGetRandom( void )
{
	bool bReturn = false;

	if ( m_pMusicManager )
	{
		bReturn = (m_pMusicManager->GetRandom()==TRUE);
	}
	return bReturn;
}

void	CXBoxStateApplication::MusicPause( void )
{
	if ( m_pMusicManager )
	{
		m_pMusicManager->Pause();
	}
}


CStdString CXBoxStateApplication::MusicGetCurrentSong( void )
{
	CheckMusicInfo();
	return m_sCurrentSong;
}

void	CXBoxStateApplication::CheckMusicInfo( void )
{
	if ( m_pMusicManager )
	{
		DWORD dwCurrentSong = 0, dwCurrentSoundtrack = 0;

		MusicGetCurrentTrackAndSong( dwCurrentSong, dwCurrentSoundtrack );
		if ( ( dwCurrentSoundtrack != m_dwCurrentSoundtrack ) ||
			( dwCurrentSong != m_dwCurrentSong ) )
		{
			WCHAR szSong[MM_SONGNAME_SIZE], szSoundtrack[MM_SONGNAME_SIZE];
			DWORD dwLength;
			
			ZeroMemory( szSong, sizeof(WCHAR)*MM_SONGNAME_SIZE );
			ZeroMemory( szSoundtrack, sizeof(WCHAR)*MM_SONGNAME_SIZE );

			m_pMusicManager->GetCurrentInfo( szSoundtrack, szSong, &dwLength );

			m_sCurrentSong = szSong;
			m_sCurrentSoundtrack = szSoundtrack;
			dwCurrentSong = m_dwCurrentSong;
			dwCurrentSoundtrack = m_dwCurrentSoundtrack;
		}
	}
}

CStdString CXBoxStateApplication::MusicGetCurrentSoundtrack( void )
{
	CheckMusicInfo();
	return m_sCurrentSoundtrack;
}

void	CXBoxStateApplication::MusicGetCurrentTrackAndSong( DWORD & dwTrack, DWORD & dwSong )
{
	if ( m_pMusicManager )
	{
		m_pMusicManager->GetCurrentTrackAndSong( dwTrack, dwSong );
	}
}

void	CXBoxStateApplication::MusicStop( void )
{
	if ( m_pMusicManager )
	{
		m_pMusicManager->Stop();
	}
}

void	CXBoxStateApplication::MusicResume( void )
{
	if ( m_pMusicManager )
	{
		if ( MusicIsSuspended() )
		{
			m_pMusicManager->Pause();
		}
		else
		{
			if ( m_pMusicManager->GetStatus() != MM_PLAYING )
			{
				DEBUG_FORMAT( _T("MusicResume: MMLoopCount = %d\r\n"), m_pMusicManager->m_dwMMLoopCount );
				m_pMusicManager->Play();
			}
		}
	}
}

void CXBoxStateApplication::MusicInitSongSelection( void )
{
	if ( m_pMusicManager )
	{
		m_pMusicManager->Pause();
		m_pMusicManager->SetFirstSong();
		if ( m_pMusicManager->GetRandom() )
		{
			m_pMusicManager->RandomSong( m_pMusicManager->GetGlobal() );
		}
	}
}

void CXBoxStateApplication::EnterMenu( CAppMenuHandler * pHandler, _TAppMenuItem * pItemList, long lMenuID )
{
	if ( m_pAppMenuPtr == NULL )
	{
		DEBUG_FORMAT( _T("EnterMenu") );
		m_pAppMenuPtr = new CAppMenu( pHandler, pItemList, lMenuID );
	}
}


//-----------------------------------------------------------------------------
// Name: RenderGradientBackground()
// Desc: Draws a gradient filled background
//-----------------------------------------------------------------------------
HRESULT CXBoxStateApplication::RenderQuadGradientBackground( DWORD dwTopLeftColor, 
											DWORD dwTopRightColor, 
											DWORD dwBottomLeftColor, 
                                            DWORD dwBottomRightColor, RECT * pRect )
{
	
	return ::RenderQuadGradientBackground( m_pd3dDevice, dwTopLeftColor, dwTopRightColor, 
												dwBottomLeftColor, dwBottomRightColor, pRect );
}

void CXBoxStateApplication::RemoveDialog( CXBDialog * pDialog )
{
}

char	CXBoxStateApplication::GetKeyChar( void )
{
	return XBInput_GetKeyboardInput();
}

BYTE	CXBoxStateApplication::GetVirtualKey( void )
{
	return XBInput_GetKeyboardVirtualKey();
}


bool	CXBoxStateApplication::AnalogButtonsPressed( int iButton )
{
	bool bReturn = false;
	int iTopButton = XINPUT_GAMEPAD_RIGHT_TRIGGER;

	if ( iButton == (-2) )
	{
		iTopButton = XINPUT_GAMEPAD_WHITE;
	}
	if ( (iButton < 0) || (iButton > 7) )
	{
		for( iButton=0; iButton<=iTopButton; iButton++ )
		{
			if ( GetDefaultGamepad()->bPressedAnalogButtons[iButton] )
			{
				bReturn = true;
				break;
			}
		}
	}
	else
	{
		bReturn = GetDefaultGamepad()->bPressedAnalogButtons[iButton]?true:false;
	}
	return bReturn;
}

HRESULT CXBoxStateApplication::AddGameState( CXBoxGameState * pGameState )
{
	HRESULT hr = S_OK;
	if ( m_iNumStates < (MAX_STATES-1) )
	{
		pGameState->SetParent( this );
		pGameState->SetGameStateID(m_iNumStates);
		m_pGameStates[m_iNumStates] = pGameState;
		// Make sure this gets called, if the state come to the party late.
		if ( m_bInitialized )
		{
			pGameState->Initialize();
		}
		m_iNumStates++; 
	}
	return hr;
}


void CXBoxStateApplication::GetStickValues( int &iStickX, int &iStickY, bool bLeft, bool bRight, int iPad )
{
	FLOAT fX = 0.0, fY = 0.0;

	if ( iPad != -1 )
	{
		if ( bLeft )
		{
			fX = GetGamepad(iPad)->fX1;
			fY = GetGamepad(iPad)->fY1;
		}
		if ( bRight )
		{
			fX += GetGamepad(iPad)->fX2;
			fY += GetGamepad(iPad)->fY2;
		}
	}
	else
	{
		if ( bLeft )
		{
			fX = GetDefaultGamepad()->fX1;
			fY = GetDefaultGamepad()->fY1;
		}
		if ( bRight )
		{
			fX += GetDefaultGamepad()->fX2;
			fY += GetDefaultGamepad()->fY2;
		}
	}
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
}

HRESULT CXBoxStateApplication::SetGameState( int iID, bool bSetBlend, bool bSetGrab )
{
	int iIndex=0;
	HRESULT hr = E_FAIL;
	for( iIndex=0; (iIndex<m_iNumStates) && (hr == E_FAIL); iIndex++ )
	{
		if ( m_pGameStates[iIndex]->GetGameStateID() == iID )
		{
			DEBUG_MEM_REPORT();
			if ( m_pCurrentGameState )
			{
				hr = m_pCurrentGameState->OnDeactivation();
			}
			m_pCurrentGameState = m_pGameStates[iIndex];
			OutputDebugString( _T("Activating App State\r\n"));
			hr = m_pCurrentGameState->OnActivation();
			OutputDebugString( _T("App State Activated\r\n"));
			DEBUG_LINE( _T("===SETGAMESTATE==="));
			if ( bSetBlend )
			{
				SetBlend();
			} 
			else if ( bSetGrab )
			{
				SetBlend(false);
				SetGrab();
			}
			else
			{
				SetBlend(false);
			}
			break;

		}
	}
	m_bOneShot = true;
	return hr;
}



HRESULT CXBoxStateApplication::LoadLocalSoundtrack( void )
{
	return S_OK;
}



bool CXBoxStateApplication::InitializeSoundtrack( void )
{
	bool bReturn = false;
	
	if ( SUCCEEDED( InitMusicManager() ) )
	{
		// Load local soundtracks....
		if ( m_pMusicManager )
		{
			if( SUCCEEDED( LoadLocalSoundtrack() ) )
			{
				bReturn = true;
			}
			else
			{
				OnMusicManagerFail();
			}
		}
		else
		{
			OnMusicManagerFail();
		}
	}
	else
	{
		OnMusicManagerFail();
	}
	return bReturn;
}


HRESULT CXBoxStateApplication::InitInputDevices( void )
{
	HRESULT hr = E_FAIL;

	if ( m_bInitInputDevices )
	{
		hr = S_OK;
	}
	else
	{
		XBInput_InitDebugKeyboard();
		hr = XBInput_CreateIR_Remotes( &m_IR_Remote );
		PerfGetPerformance( _T("StateApp IR Initialized"), FALSE );
		m_bInitInputDevices = true;
	}
	return hr;
}

HRESULT CXBoxStateApplication::Init3DWorld( void )
{
	HRESULT hr = S_OK;

	if ( !m_bInit3DWorld )
	{
		// Misc render states
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
		m_pd3dDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
		m_pd3dDevice->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
		m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,   TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,        TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,        0xffffffff );
		m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,       D3DCULL_NONE );

		// Set up world matrix
		D3DXMATRIX matWorld;
		D3DXMatrixIdentity( &matWorld );
		m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

		// Set up view matrix
		D3DXMATRIX  matView;
		D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.0f, 0.0f,-250.0f );
		D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f,   0.0f );
		D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f,   0.0f );
		D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
		m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

		// Set up proj matrix
		D3DXMATRIX matProj;
		D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 4.0f/3.0f, 1.0f, 1000.0f );
		m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

		// Setup a base material
		D3DMATERIAL8 mtrl;
		XBUtil_InitMaterial( mtrl, 1.0f, 1.0f, 1.0f, 1.0f );
		m_pd3dDevice->SetMaterial( &mtrl );

		PerfGetPerformance( _T("StateApp 3D World Initialized"), FALSE );
		m_bInit3DWorld = true;
	}

	return hr;
}

HRESULT CXBoxStateApplication::InitSound( void )
{
	HRESULT hr = E_FAIL;

	DEBUG_FORMAT( _T("-------------Initializing Sound Device--------------") );
	if ( m_bInitSound )
	{
		hr = S_OK;
	}
	else
	{
		PerfGetPerformance( _T("StateApp Initializing Sound"), FALSE );
		hr = DirectSoundCreate( NULL, &m_pDSound, NULL );
		PerfGetPerformance( _T("StateApp Sound Initialized"), FALSE );
		Sleep(500);
		m_bInitSound = true;
	}
	return hr;
}

HRESULT CXBoxStateApplication::InitMusicManager( void )
{
	HRESULT hr = E_FAIL;

	DEBUG_FORMAT( _T("-------------Initializing Music Manager--------------") );
	if ( m_bInitMusicManager )
	{
		hr = S_OK;
	}
	else
	{
		// Create MusicManager
		g_sMusicFiles = _T("");
		m_pMusicManager = new CMusicManager();
		if( m_pMusicManager )
		{
			// Initialize it
			if( FAILED( hr = m_pMusicManager->Initialize(true) ) )
			{
				delete m_pMusicManager;
				m_pMusicManager = NULL;
			}
		}
		PerfGetPerformance( _T("StateApp MusicManager Initialized"), FALSE );
		m_bInitMusicManager = true;
	}
	return hr;
}

HRESULT CXBoxStateApplication::InitDefaultFont( void )
{
	HRESULT hr = E_FAIL;

	if ( m_bInitDefaultFont )
	{
		hr = S_OK;
	}
	else
	{
	    hr = m_Font16.CreateFromMem( m_pd3dDevice, _T("FontData"), (const LPBYTE)pbFontData ); // "Font16.xpr" );
		PerfGetPerformance( _T("StateApp Font16 Initialized"), FALSE );
		m_bInitDefaultFont = true;
	}

	return hr;
}

void	CXBoxStateApplication::SetNewSkin( void )
{
	int iIndex;

	for( iIndex=0; iIndex<m_iNumStates; iIndex++ )
	{
		m_pGameStates[iIndex]->OnNewSkin();
	}
}


HRESULT CXBoxStateApplication::InitStates( void )
{
	HRESULT hr = E_FAIL;
	int iIndex;

	if ( m_bInitStates )
	{
		hr = S_OK;
	}
	else
	{
		for( iIndex=0; (iIndex<m_iNumStates) && (hr == S_OK); iIndex++ )
		{
			hr = m_pGameStates[iIndex]->Initialize();
		}
		PerfGetPerformance( _T("StateApp States Initialized"), FALSE );
		// Now activate the first game state
		SetGameState(0);
		m_bInitStates = true;
	}
	return hr;
}


HRESULT CXBoxStateApplication::Initialize()
{
	// Initialize ALL game states here....
	HRESULT hr = S_OK;

	PerfGetPerformance( _T("StateApp Initialize"), FALSE );

    // Create DirectSound
    if( FAILED( hr = InitSound() ) )
	{
		DEBUG_LINE( _T("InitSound FAILED!"));
        return hr;
	}

	// Initialize XBOX IR Remote Port
	if ( FAILED(hr = InitInputDevices()) )
	{
		return hr;
	}


    // Create the fonts
    if( FAILED( hr = InitDefaultFont() ) )
        return hr;



	Init3DWorld();

	InitStates();

	m_imgPersistImage.Initialize( Get3dDevice(), GetSoundDevice() );

	m_bInitialized = true;
	return hr;
}

void CXBoxStateApplication::QueryInput( void )
{
	if ( (m_bGotInput == false) || (!m_bInActionScript) )
	{
		CXBApplication::QueryInput();
		QueryIRInput();
	}
	m_bGotInput = true;
}

void CXBoxStateApplication::QueryIRInput( void )
{
	//XBOX IR Remote Support
	XBInput_GetInput( m_IR_Remote, m_fTime );
	ZeroMemory( &m_DefaultIR_Remote, sizeof(m_DefaultIR_Remote) );


	for( DWORD i=0; i<4; i++ )
	{
		if( m_IR_Remote[i].hDevice)
		{
			m_DefaultIR_Remote.wButtons        = m_IR_Remote[i].wButtons;
			m_DefaultIR_Remote.wPressedButtons = m_IR_Remote[i].wPressedButtons;
			m_DefaultIR_Remote.wLastButtons    = m_IR_Remote[i].wLastButtons;
		}
	}
	//End XBOX IR Remote Support
}


HRESULT CXBoxStateApplication::FrameMove()
{
	HRESULT hr = E_FAIL;
	bool bPassInput = true;

	OnRunLoopTop();
	DirectSoundDoWork();

	if( !m_bInActionScript )
	{
		if ( m_bOneShot )
		{
			OutputDebugString( _T("Entering StateApp::FrameMove Oneshot\r\n"));
		}
		// Do sound work here so nobody else needs to call it.
		// Don't think it harms it if it's called more than once.
	//	if ( g_MenuInfo.m_bUseMusicManager )
	//	{
	//		if ( m_pMusicManager )
	//		{
	//			m_pMusicManager->MusicManagerDoWork();
	//		}
	//	}

		// Done through QueryInput override now
		//	QueryIRInput();

		if ( m_bDisplayBox )
		{
			char byVirtKey = this->GetKeyChar();
			WORD wButtons = GetDefaultGamepad()->wPressedButtons;
			WORD wRemotes = this->m_DefaultIR_Remote.wPressedButtons;
			if ( GetDefaultGamepad()->bLastAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] )
			{
				if ( GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_X] ) // && GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] )
				{
					SetScreenCapture();
					// bPassInput = false;
				}
			}
			else
			{
				if ( wButtons || AnalogButtonsPressed() || byVirtKey || wRemotes )
				{
					m_bDisplayBox = false;
					if ( m_bRebootAfterMsg )
					{
						Reboot();
					}
				}
			}
		}
		else
		{
			// This is when we need to handle input to a menu
			if ( m_pAppMenuPtr )
			{
				if ( m_pAppMenuPtr->MenuFrameMove() == FALSE )
				{
					// Top level menu backed out of.
					delete m_pAppMenuPtr;
					m_pAppMenuPtr = NULL;
				}
				if ( GetDefaultGamepad()->bLastAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] )
				{
					if ( GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_X] ) // && GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] )
					{
						SetScreenCapture();
						// bPassInput = false;
					}
				}
			}
			else if ( m_pCurrentGameState )
			{
				if ( m_bDebounce )
				{
					char byVirtKey = this->GetKeyChar();
					WORD wButtons = GetDefaultGamepad()->wButtons;
					WORD wRemotes = this->m_DefaultIR_Remote.wPressedButtons;
					int iStickX,iStickY;

					GetStickValues( iStickX, iStickY );
					if ( (wButtons == 0) && (wRemotes == 0) && (byVirtKey == 0 ) && (iStickY==0) && (iStickX==0))
					{
						m_bDebounce = false;
					}
				}
				else
				{
					// First check for screen capture SetScreenCapture()
					
					if ( GetDefaultGamepad()->bLastAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] )
					{
						if ( GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_X] ) // && GetDefaultGamepad()->bPressedAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] )
						{
							SetScreenCapture();
							bPassInput = false;
						}
					}

					if ( bPassInput )
					{
						hr = m_pCurrentGameState->FrameMove();
					}
					else
					{
						hr = S_OK;
					}
				}
			}
		}
		if ( m_bOneShot )
		{
			OutputDebugString( _T("Exiting StateApp::FrameMove Oneshot\r\n"));
		}
	}
	return hr;
}

void	CXBoxStateApplication::RebootMessageBox( LPCTSTR szFmt, ... )
{
	CStdString sFmtOut;
	va_list argList;
	va_start(argList, szFmt);

	m_bRebootAfterMsg = true;
	if ( szFmt )
	{
		sFmtOut.FormatV(szFmt, argList);
		va_end(argList);
		m_sMessageBox = sFmtOut;
		m_bDisplayBox = true;
	}
	else
	{
		// Turn off the message box.
		m_bDisplayBox = false;
	}
}

void CXBoxStateApplication::MessageBox( LPCTSTR szFmt, ...)
{
	CStdString sFmtOut;
	va_list argList;
	va_start(argList, szFmt);

	if ( szFmt )
	{
		sFmtOut.FormatV(szFmt, argList);
		va_end(argList);
		m_sMessageBox = sFmtOut;
		m_bDisplayBox = true;
	}
	else
	{
		// Turn off the message box.
		m_bDisplayBox = false;
	}
}


void CXBoxStateApplication::MessageBoxStr( LPCTSTR szString )
{
	if ( szString )
	{
		m_sMessageBox = szString;
		m_bDisplayBox = true;
	}
	else
	{
		// Turn off the message box.
		m_bDisplayBox = false;
	}
}

void	CXBoxStateApplication::FirstRender( void ) { }


HRESULT	CXBoxStateApplication::PumpRender( void )
{
	return Render();
}

HRESULT		CXBoxStateApplication::PumpInput( bool bDoFrameMove )
{
	HRESULT hr = S_OK;
	QueryInput();
	if ( bDoFrameMove )
	{
		hr = FrameMove();
	}
	return hr;
}


HRESULT CXBoxStateApplication::Render()
{
	HRESULT hr = S_OK;
	DWORD dwTicks;

	EnterCriticalSection( &g_RenderCriticalSection );
	if ( m_bOneShot )
	{
		OutputDebugString( _T("Entering StateApp::Render Oneshot\r\n"));
	}
	if ( m_bFirstRender )
	{
		FirstRender();
		m_bFirstRender =  false;
	}
	if ( m_bPersistGrab )
	{
		LPDIRECT3DTEXTURE8 pTexture;
		if ( SUCCEEDED( GetBackBufferTexture( Get3dDevice(), &pTexture ) ))
		{
			CStdString sTemp;
			DEBUG_LINE( _T("===GRABBING===") );
			m_imgPersistImage.Attach( pTexture );
			pTexture->Release();
			sTemp.Format( _T("Created Persist Texture: (%dx%d)"), m_imgPersistImage.GetWidth(), m_imgPersistImage.GetHeight() );
			DEBUG_LINE( sTemp.c_str() );
			m_bPersistGrab = false;
		}
		else
		{
			m_bPersistGrab = false;
			DEBUG_LINE( _T("===FAILED GRABBING===") );
		}
	}
	if ( m_bPersistBlendRender )
	{
		m_bPersistBlendRender = false;
		LPDIRECT3DTEXTURE8 pTexture;
		if ( SUCCEEDED( GetBackBufferTexture( Get3dDevice(), &pTexture ) ))
		{
			CStdString sTemp;
			DEBUG_LINE( _T("===BLENDING===") );
			m_imgPersistImage.Attach( pTexture );
			pTexture->Release();
			sTemp.Format( _T("Created Persist Texture: (%dx%d)"), m_imgPersistImage.GetWidth(), m_imgPersistImage.GetHeight() );
			DEBUG_LINE( sTemp.c_str() );
			m_bPersistTransition = true;
			m_fBlendAmount = 1.0;
			m_dwBlendTimer = GetTickCount();
			m_dwTransitionFrames = 0;
		}
		else
		{
			DEBUG_LINE( _T("===FAILED BLENDING===") );
		}
	}
	m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0L );
	if ( m_pCurrentGameState )
	{
		hr = m_pCurrentGameState->Render();
	}


	if( !m_bInActionScript )
	{


		// This is when we need to display a menu
		if ( m_pAppMenuPtr )
		{
			m_pAppMenuPtr->Render();
		}

		if ( m_bDisplayBox )
		{
			DrawTextBox( m_pd3dDevice, m_sMessageBox.c_str(), &m_Font16, &MessageAttr );
		}
	}
	else
	{
		if ( m_pdrawList )
		{
			DrawActionDrawList( *m_pdrawList );
		}
	}


	if ( m_bPersistTransition )
	{
		float fTimeElapsed;
		
		m_dwTransitionFrames++;
		dwTicks = (GetTickCount()-m_dwBlendTimer);
		fTimeElapsed = (float)dwTicks;
		fTimeElapsed /= 1500.0;
		m_fBlendAmount = (float)(1.0-fTimeElapsed);

		if ( m_fBlendAmount < 0.0 )
		{
			CStdString sTemp;
			m_bPersistTransition = false;
			m_fBlendAmount = 0.0;
			sTemp.Format( _T("Elapsed: %ld (%ld)"), dwTicks, m_dwTransitionFrames );
			DEBUG_LINE( sTemp.c_str() );
			DEBUG_LINE( _T("===END SETBLEND===") );
		}
		else
		{
			if( m_imgPersistImage.GetTexture() )
			{
				RenderQuad(m_imgPersistImage.GetTexture(), 640, 480, m_fBlendAmount, 0.0 );
			}
			else
			{
				CStdString sTemp;
				DEBUG_LINE( _T("Cannot render BLEND texture!!") );
				sTemp.Format( _T("Elapsed: %ld (%ld)"), dwTicks, m_dwTransitionFrames );
				DEBUG_LINE( sTemp.c_str() );
				DEBUG_LINE( _T("===END SETBLEND===") );
				m_bPersistTransition = false;
				m_fBlendAmount = 0.0;
			}
		}
	}
    // Present the scene
    m_pd3dDevice->Present( NULL, NULL, NULL, NULL );

	MusicCheck();

	//if ( m_pMusicManager )
	//{
	//		m_pMusicManager->Process();
	//}

	if ( m_bScreenCapture )
	{
		LPDIRECT3DTEXTURE8 pTexture;
		if ( SUCCEEDED( GetBackBufferTexture( Get3dDevice(), &pTexture ) ))
		{
			CStdString sTemp;
			IDirect3DSurface8 *pSurface = NULL;

			if ( SUCCEEDED(pTexture->GetSurfaceLevel(0, &pSurface )) )
			{
				D3DSURFACE_DESC surfDec;
				CStdString sFilename;

				pTexture->GetLevelDesc( 0, &surfDec );
				if ( pSurface )
				{
					sFilename = GetNextCaptureFilename();
					if ( sFilename.GetLength() )
					{
						DEBUG_LINE( _T("===CAPTURE===") );
						// m_imgPersistImage.Attach( pTexture );
						XGWriteSurfaceToFile( pSurface, sFilename );
					}
					pSurface->Release();
				}
			}
			pTexture->Release();
		}
		else
		{
			DEBUG_LINE( _T("===FAILED CAPTURE===") );
		}
		m_bScreenCapture = false;
	}
	if ( m_bOneShot )
	{
		OutputDebugString( _T("Exiting StateApp::Render Oneshot\r\n"));
		m_bOneShot = false;
	}
	LeaveCriticalSection( &g_RenderCriticalSection );
	
	return hr;

}

HRESULT CXBoxStateApplication::DrawMessageBox( LPCTSTR szMessage )
{
	return DrawTextBox( m_pd3dDevice, szMessage, &m_Font16, &MessageAttr );
}

HRESULT CXBoxStateApplication::Cleanup()
{
	HRESULT hr = S_OK;
	int iIndex;

	for( iIndex=0; (iIndex<m_iNumStates) && (hr == S_OK); iIndex++ )
	{
		hr=m_pGameStates[iIndex]->Cleanup();
	}
	return hr;
}

void CXBoxStateApplication::SetGrab(void)
{ 
	if ( m_imgPersistImage.GetTexture() )
	{
		m_imgPersistImage.ReleaseResources();
	}
	m_bPersistGrab = true; 
	m_bPersistBlendRender=false; 
	m_bPersistTransition=false; 
}

void CXBoxStateApplication::SetBlend(bool bEnable)
{ 

	if ( m_imgPersistImage.GetTexture() )
	{
		m_imgPersistImage.ReleaseResources();
	}
	if ( bEnable )
	{
//	DEBUG_LINE( _T("===SETBLEND===") );
		m_bPersistBlendRender=true; 
		m_bPersistGrab = false;
		m_bPersistTransition=false;
	}
	else
	{
		m_bPersistBlendRender=false; 
//		m_bPersistGrab = false;
//		m_bPersistTransition=false;
	}
}


HRESULT CXBoxStateApplication::DrawTexture( IDirect3DBaseTexture8 * pTexture, int iPosX, int iPosY, int iSizeX, int iSizeY )
{
#if 1
	return ::DrawTexture( m_pd3dDevice, pTexture, iPosX, iPosY, iSizeX, iSizeY );
#else

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );

    // Draw the analog gauges
    m_pd3dDevice->SetTexture( 0, pTexture );

	m_pd3dDevice->SetVertexShader( FVF_SPRITE );

    struct VERTEX { D3DXVECTOR4 p; FLOAT tu, tv; };
    VERTEX v[4];
    v[0].p = D3DXVECTOR4( (FLOAT)iPosX			- 0.5f, (FLOAT)iPosY			- 0.5f, 0, 0 );  v[0].tu =   0;		v[0].tv =   0;
    v[1].p = D3DXVECTOR4( (FLOAT)(iPosX+iSizeX)	- 0.5f, (FLOAT)iPosY			- 0.5f, 0, 0 );  v[1].tu = (FLOAT)iSizeX;	v[1].tv =   0;
    v[2].p = D3DXVECTOR4( (FLOAT)(iPosX+iSizeX)	- 0.5f, (FLOAT)(iPosY+iSizeY)	- 0.5f, 0, 0 );  v[2].tu = (FLOAT)iSizeX;	v[2].tv = (FLOAT)iSizeY;
    v[3].p = D3DXVECTOR4( (FLOAT)iPosX			- 0.5f, (FLOAT)(iPosY+iSizeY)	- 0.5f, 0, 0 );  v[3].tu =   0;		v[3].tv = (FLOAT)iSizeY;
    m_pd3dDevice->DrawPrimitiveUP( D3DPT_QUADLIST, 1, v, sizeof(v[0]) );
	return S_OK;
#endif
}


//-----------------------------------------------------------------------------
// Name: DrawBox()
// Desc: Graphics helper function
//-----------------------------------------------------------------------------
HRESULT CXBoxStateApplication::DrawLine( FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2,
                              DWORD dwOutlineColor )
{
#if 1
	return ::DrawLine( m_pd3dDevice, x1, y1, x2, y2, dwOutlineColor );
#else
    D3DXVECTOR4 v[2];
    v[0] = D3DXVECTOR4( x1-0.5f, y1-0.5f, 0, 0 );
    v[1] = D3DXVECTOR4( x2-0.5f, y2-0.5f, 0, 0 );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW );

    // Render the lines
    m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwOutlineColor );
    m_pd3dDevice->DrawVerticesUP( D3DPT_LINELIST, 2, v, sizeof(v[0]) );

    return S_OK;
#endif
}


//-----------------------------------------------------------------------------
// Name: DrawBox()
// Desc: Graphics helper function
//-----------------------------------------------------------------------------
HRESULT CXBoxStateApplication::DrawBox( FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2,
                              DWORD dwFillColor, DWORD dwOutlineColor )
{
#if 1
	return ::DrawBox( m_pd3dDevice, x1, y1, x2, y2, dwFillColor, dwOutlineColor );
#else
    D3DXVECTOR4 v[4];
    v[0] = D3DXVECTOR4( x1-0.5f, y1-0.5f, 0, 0 );
    v[1] = D3DXVECTOR4( x2-0.5f, y1-0.5f, 0, 0 );
    v[2] = D3DXVECTOR4( x2-0.5f, y2-0.5f, 0, 0 );
    v[3] = D3DXVECTOR4( x1-0.5f, y2-0.5f, 0, 0 );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW );

    // Render the box
    m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwFillColor );
    m_pd3dDevice->DrawVerticesUP( D3DPT_QUADLIST, 4, v, sizeof(v[0]) );

    // Render the lines
    m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwOutlineColor );
    m_pd3dDevice->DrawVerticesUP( D3DPT_LINELOOP, 4, v, sizeof(v[0]) );

    return S_OK;
#endif
}


//-----------------------------------------------------------------------------
// Name: DrawBox()
// Desc: Graphics helper function
//-----------------------------------------------------------------------------
HRESULT CXBoxStateApplication::DrawTriangle( FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2,
                              DWORD dwFillColor, DWORD dwOutlineColor, int iDirection )
{
#if 1
	return ::DrawTriangle( m_pd3dDevice, x1, y1, x2, y2, dwFillColor, dwOutlineColor, iDirection );
#else
    D3DXVECTOR4 v[3];
	FLOAT fDelta;

	switch( iDirection )
	{
		case 0: // Up
			fDelta = x2-x1;
			fDelta /= 2.0;
			fDelta += 0.5f;
			v[0] = D3DXVECTOR4( x1-0.5f, y2-0.5f, 0, 0 );
			v[1] = D3DXVECTOR4( x2-fDelta, y1-0.5f, 0, 0 );
			v[2] = D3DXVECTOR4( x2-0.5f, y2-0.5f, 0, 0 );
			break;
		case 2: // Left
			fDelta = y2-y1;
			fDelta /= 2.0;
			fDelta += 0.5f;
			v[0] = D3DXVECTOR4( x2-0.5f, y2-0.5f, 0, 0 );
			v[1] = D3DXVECTOR4( x1-0.5f, y2-fDelta, 0, 0 );
			v[2] = D3DXVECTOR4( x2-0.5f, y1-0.5f, 0, 0 );
			break;
		case 3: // Right
			fDelta = y2-y1;
			fDelta /= 2.0;
			fDelta += 0.5f;
			v[0] = D3DXVECTOR4( x1-0.5f, y1-0.5f, 0, 0 );
			v[1] = D3DXVECTOR4( x2-0.5f, y2-fDelta, 0, 0 );
			v[2] = D3DXVECTOR4( x1-0.5f, y2-0.5f, 0, 0 );
			break;
		case 1: // Down
		default:
			fDelta = x2-x1;
			fDelta /= 2.0;
			fDelta += 0.5f;
			v[0] = D3DXVECTOR4( x2-0.5f, y1-0.5f, 0, 0 );
			v[1] = D3DXVECTOR4( x2-fDelta, y2-0.5f, 0, 0 );
			v[2] = D3DXVECTOR4( x1-0.5f, y1-0.5f, 0, 0 );
			break;
	}

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW );

    // Render the box
    m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwFillColor );
    m_pd3dDevice->DrawVerticesUP( D3DPT_TRIANGLELIST, 3, v, sizeof(v[0]) );

    // Render the lines
    m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwOutlineColor );
    m_pd3dDevice->DrawVerticesUP( D3DPT_LINELOOP, 3, v, sizeof(v[0]) );

    return S_OK;
#endif
}


//-----------------------------------------------------------------------------
// Name: RenderQuad()
// Desc: Renders a quad textured with the persisted surface
//-----------------------------------------------------------------------------
HRESULT CXBoxStateApplication::RenderTile( LPDIRECT3DTEXTURE8 pTexture, int iScreenWidth, int iScreenHeight, FLOAT fDepth )
{
    // Set up the vertices (notice the pixel centers are shifted by -0.5f to
    // line them up with the texel centers). The texture coordinates assume
    // a linear texture will be used.
	

    struct VERTEX { D3DXVECTOR4 p; FLOAT tu, tv; };
    VERTEX v[4];
    v[0].p = D3DXVECTOR4(   0 - 0.5f,   0 - 0.5f, fDepth, 0.0f ); 
	v[0].tu =   0; v[0].tv =   0;
    v[1].p = D3DXVECTOR4( (FLOAT)iScreenWidth - 0.5f,   0 - 0.5f, fDepth, 0.0f ); 
	v[1].tu = (FLOAT)iScreenWidth; v[1].tv =   0.0;
    v[2].p = D3DXVECTOR4( iScreenWidth - 0.5f, iScreenHeight - 0.5f, fDepth, 0.0f ); 
	v[2].tu = (FLOAT)iScreenWidth; v[2].tv = (FLOAT)iScreenHeight;
    v[3].p = D3DXVECTOR4(   0 - 0.5f, (FLOAT)iScreenHeight - 0.5f, fDepth, 0.0f ); 
	v[3].tu =   0; v[3].tv = (FLOAT)iScreenHeight;


    // Set state to render the image
    m_pd3dDevice->SetTexture( 0, pTexture );
    m_pd3dDevice->SetPixelShader( NULL );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_WRAP );
//    m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
//    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
//    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
//    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,    FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_NONE );

    // Render the quad
    m_pd3dDevice->SetVertexShader( FVF_SPRITE ); // D3DFVF_XYZRHW|D3DFVF_TEX1 );
    m_pd3dDevice->DrawPrimitiveUP( D3DPT_QUADLIST, 1, v, sizeof(v[0]) );

	m_pd3dDevice->SetTexture( 0, NULL );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: RenderQuad()
// Desc: Renders a quad textured with the persisted surface
//-----------------------------------------------------------------------------
HRESULT CXBoxStateApplication::RenderQuad( LPDIRECT3DTEXTURE8 pTexture, int iScreenWidth, int iScreenHeight, FLOAT fAlpha, FLOAT fDepth )
{
    // Set up the vertices (notice the pixel centers are shifted by -0.5f to
    // line them up with the texel centers). The texture coordinates assume
    // a linear texture will be used.
    struct VERTEX { D3DXVECTOR4 p; FLOAT tu, tv; };
    VERTEX v[4];
    v[0].p = D3DXVECTOR4(   0 - 0.5f,   0 - 0.5f, fDepth, 0.0f ); v[0].tu =   0; v[0].tv =   0;
    v[1].p = D3DXVECTOR4( (FLOAT)iScreenWidth - 0.5f,   0 - 0.5f, fDepth, 0.0f ); v[1].tu = (FLOAT)iScreenWidth; v[1].tv =   0;
    v[2].p = D3DXVECTOR4( (FLOAT)iScreenWidth - 0.5f, (FLOAT)iScreenHeight - 0.5f, fDepth, 0.0f ); v[2].tu = (FLOAT)iScreenWidth; v[2].tv = (FLOAT)iScreenHeight;
    v[3].p = D3DXVECTOR4(   0 - 0.5f, (FLOAT)iScreenHeight - 0.5f, fDepth, 0.0f ); v[3].tu =   0; v[3].tv = (FLOAT)iScreenHeight;


    // Set state to render the image
    m_pd3dDevice->SetTexture( 0, pTexture );
    m_pd3dDevice->SetPixelShader( NULL );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, (DWORD)(255.0f*fAlpha)<<24L );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,    FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_NONE );

    // Render the quad
    m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW|D3DFVF_TEX1 );
    m_pd3dDevice->DrawPrimitiveUP( D3DPT_QUADLIST, 1, v, sizeof(v[0]) );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	// clean up
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, (DWORD)(255)<<24L );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	m_pd3dDevice->SetTexture( 0, NULL );

    return S_OK;
}

DWORD CXBoxStateApplication::GetDir( WORD wFlags, WORD * pwState )
{
	DWORD dwReturn = 0;
	FLOAT fX, fY;
	DWORD wCurrent, wDir, wState, wValue;
	WORD wButtons = GetDefaultGamepad()->wButtons;
	WORD wRemotes = m_DefaultIR_Remote.wPressedButtons;

	wDir = 0;
	wState = 0;
	if ( wFlags & DIR_IN_LFTHUMB )
	{
		wCurrent = 0;
		fX = GetDefaultGamepad()->fX1;
		fY = GetDefaultGamepad()->fY1;
		if ( fX <= (-0.5) )
		{
			wCurrent = DC_LEFT;
		} 
		else if ( fX >= 0.5 )
		{
			wCurrent |= DC_RIGHT;
		}
		if ( fY <= (-0.5) )
		{
			wCurrent |= DC_DOWN;
		}
		else if ( fY >= 0.5 )
		{
			wCurrent |= DC_UP;
		}
		wValue = m_ctrLfStick.DirInput((WORD)wCurrent);
		if ( ( wValue & DC_SKIP ) == 0 )
		{
			wDir = wValue;
		}
		wState = wCurrent;
	}
	if ( wFlags & DIR_IN_RTTHUMB )
	{
		wCurrent = 0;
		fX = GetDefaultGamepad()->fX2;
		fY = GetDefaultGamepad()->fY2;

		if ( fX <= (-0.5) )
		{
			wCurrent |= DC_LEFT;
		} 
		else if ( fX >= 0.5 )
		{
			wCurrent |= DC_RIGHT;
		}
		if ( fY <= (-0.5) )
		{
			wCurrent |= DC_DOWN;
		}
		else if ( fY >= 0.5 )
		{
			wCurrent |= DC_UP;
		}
		wValue = m_ctrRtStick.DirInput((WORD)wCurrent);
		if ( ( wValue & DC_SKIP ) == 0 )
		{
			wDir |= wValue;
		}
		wState |= wCurrent;
	}
	if ( wFlags & DIR_IN_DPAD )
	{
		wCurrent = 0;
		if ( wButtons & XINPUT_GAMEPAD_DPAD_UP )
		{
			wCurrent |= DC_UP;
		}
		else if ( wButtons & XINPUT_GAMEPAD_DPAD_DOWN )
		{
			wCurrent |= DC_DOWN;
		}
		if ( wButtons & XINPUT_GAMEPAD_DPAD_LEFT )
		{
			wCurrent |= DC_LEFT;
		}
		else if ( wButtons & XINPUT_GAMEPAD_DPAD_RIGHT )
		{
			wCurrent |= DC_RIGHT;
		}
		wValue = m_ctrDpad.DirInput((WORD)wCurrent);
		if ( ( wValue & DC_SKIP ) == 0 )
		{
			wDir |= wValue;
		}
		wState |= wCurrent;
	}
	if ( wFlags & DIR_IN_IR )
	{
		wCurrent = 0;
		if ( wRemotes == XINPUT_IR_REMOTE_UP )
		{
			wCurrent = DC_UP;
		}
		if ( wRemotes == XINPUT_IR_REMOTE_DOWN )
		{
			wCurrent = DC_DOWN;
		}
		if ( wRemotes == XINPUT_IR_REMOTE_LEFT )
		{
			wCurrent = DC_LEFT;
		}
		if ( wRemotes == XINPUT_IR_REMOTE_RIGHT )
		{
			wCurrent = DC_RIGHT;
		}
		wState |= wCurrent;
		wValue = m_ctrIR.IRInput(wRemotes);
		if ( ( wValue & DC_SKIP ) == 0 )
		{
			wDir |= wValue;
		}
	}


	if ( pwState )
	{
		dwReturn = (wDir);
		*pwState = (WORD)(wState&0x0ffff);
	}
	else
	{
		dwReturn = (wState<<16)|(wDir);
	}
	return dwReturn;
}

DWORD CXBoxStateApplication::GetKeyDir( BYTE byVirtKey, WORD * pwState )
{
	DWORD dwReturn = 0;
	DWORD wState = 0, wDir = 0;

	byVirtKey = GetVirtualKey();
	if ( byVirtKey == VK_UP )
	{
		wState = DC_UP;
	} 
	else if ( byVirtKey == VK_DOWN )
	{
		wState = DC_DOWN;
	}
	else if ( byVirtKey == VK_LEFT )
	{
		wState = DC_LEFT;
	}
	else if ( byVirtKey == VK_RIGHT )
	{
		wState = DC_RIGHT;
	}
	wDir = m_ctrIR.DirInput((WORD)wState);
	if ( pwState )
	{
		dwReturn = (wDir);
		*pwState = (WORD)(wState&0x0ffff);
	}
	else
	{
		dwReturn = (wState<<16)|(wDir);
	}
	return dwReturn;
}


void CXBoxStateApplication::SetDelays( DWORD dwMoveDelay, DWORD dwRepeatDelay )
{
	m_ctrRtStick.SetDelays( dwMoveDelay, dwRepeatDelay );
	m_ctrLfStick.SetDelays( dwMoveDelay, dwRepeatDelay );
	m_ctrDpad.SetDelays( dwMoveDelay, dwRepeatDelay );
	m_ctrIR.SetDelays( dwMoveDelay, dwRepeatDelay );
}

