//-----------------------------------------------------------------------------
// File: TestOne.cpp
//
// Desc: Testing the creation of a simple application for the XBox
//
// Hist: 02.08.04 - Modified gamepad.cpp sample
//
// 
//-----------------------------------------------------------------------------
#include <XBApp.h>
#include <XBFont.h>
#include <XBMesh.h>
#include <XBUtil.h>
#include <XBResource.h>
#include <xgraphics.h>
#include "Resource.h"

#include "BubblesMatrix.h"
#include "score.h"


#define GP_LEFTTHUMBSTICK  0x0040
#define GP_RIGHTTHUMBSTICK 0x0080
#define GP_BACKBUTTON      0x0020
#define GP_STARTBUTTON     0x0010
#define GP_DIRUP		   0x0001
#define GP_DIRDOWN		   0x0002
#define GP_DIRLEFT		   0x0004
#define GP_DIRRIGHT		   0x0008

#define GP_ANALOG_A			0
#define GP_ANALOG_B			1
#define GP_ANALOG_X			2
#define GP_ANALOG_Y			3

#define GS_TITLE		1
#define GS_STARTMENU	2
#define GS_CONFIG		3
#define GS_HIGHSCORE	4
#define GS_GAMESCREEN	5


/*

Adding Display Text Console classes:

  TTY Interpreter - LF, CR, TAB, other characters
  Monospaced character set font ASCII only supported for now.
  Define text grind by number of columns and rows, as well as font size.
  Include "autofit" option based on screen size
  Ability to change Top Left X,Y
  "Attach" to current display
  Hide/Restore(/shrink??)
  Should be presented as a surface.
  Allow gradient alpha and color for background
  Default text color (needs attributes) Blinkies??? Cursor????
  Border size, alpha and color ->gradient?
  

  Functions:


  class CXBCharConsole

  Screen Manipulation functions
  WriteString( col, row, string, ... );
  WriteStringAttr( col, row, attr, string, ... );
  ScrollUp()
  ScrollDown()
  ScrollRight()
  ScrollLeft()

  class CXBTTYConsole : public CXBCharConsole

  TTY will be a different class, allows larger-than-visible 
	screen for buffering, adds scroll bar option
  TTY attribute settings
  

  SetAttr()
  SetTextColor()
  SetBackColor()
  SetBlink()

  PrintF(s,...) (TTY)
  PutC(c)		(TTY)



  class CXBVirtualKeyboard

  This class wil be used to present a graphical keyboard for typing in 
  user information. It will maintain information on current letter selection,
  if the user has entered anything, and render itself on the given surface
  in a number of styles.




	Text buffer management routines:

  




  */

#define SCR_BOTHBUFF		3
#define SCR_TEXTBUFF		1
#define SCR_ATTRBUFF		2

CHighScores g_highScores[3];

int g_iDifficulty;

WCHAR *wcszDifficulty[] = { L"Easy", L"Normal", L"Hard" };


class CXBTextBuffer
{
public:
	int		m_iColumns;
	int		m_iRows;
	WORD *	m_pBuffer;
	CXBTextBuffer( int iCols = 80, int iRows = 50 )
		: m_iColumns( iCols )
		, m_iRows( iRows )
		, m_pBuffer( NULL )
	{
		// Allocate the buffer, based on the size
		if ( iCols && iRows )
		{
			m_pBuffer = new WORD[iCols*iRows];
			if ( m_pBuffer )
			{
				memset( m_pBuffer, 0, sizeof(WORD)*iCols*iRows );
			}
		}
	};
	~CXBTextBuffer()
	{
		if ( m_pBuffer )
		{
			delete [] m_pBuffer;
		}
	};

	// Accessing the buffer
	void PutChar( int iCol, int iRow, WCHAR wcChar );
	void PutChar( int iCol, int iRow, char cChar );
	void PutCharAttr( int iCol, int iRow, WCHAR wcChar, BYTE byAttr );
	void PutCharAttr( int iCol, int iRow, char cChar, BYTE byAttr );
	void PutAttr( int iCol, int iRow, BYTE byAttr );
	void PutRow( int iRow, WORD * wData );

	void ScrollUp( int iRows = 1, BYTE byFlags = SCR_BOTHBUFF );
	void ScrollDown( int iRows = 1, BYTE byFlags = SCR_BOTHBUFF );
	void ScrollRight( int iCols = 1, BYTE byFlags = SCR_BOTHBUFF );
	void ScrollLeft( int iCols = 1, BYTE byFlags = SCR_BOTHBUFF );

	WORD *	GetRow( int iRow );
	WORD	GetCharAttr( int iCol, int iRow );
	char	GetChar( int iCol, int iRow );
	BYTE	GetAttr( int iCol, int iRow );

	void	ClearRow( int iRow );
	void	ClearScreen( void );
	BYTE	m_byDefaultAttr;
};

void CXBTextBuffer::PutChar( int iCol, int iRow, WCHAR wcChar )
{
	PutChar( iCol, iRow, (char)(wcChar&0x0ff) );
}

#define TB_GETATTR(x)  (x>>8)
#define TB_GETCHAR(x)  (x&0x0ff)

void CXBTextBuffer::PutChar( int iCol, int iRow, char cChar )
{
	if ( m_pBuffer )
	{
		if ( ( iCol > -1 ) && ( iCol < m_iColumns ) &&
			 ( iRow > -1 ) && ( iRow < m_iRows ) )
		{
			int iIndex = (iRow*m_iColumns)+iCol;
			WORD wcValue = m_pBuffer[iIndex]&0xff00;
			wcValue |= (WORD)((BYTE)cChar);
			m_pBuffer[iIndex] = wcValue;
			
		}
	}
}

void CXBTextBuffer::PutCharAttr( int iCol, int iRow, WCHAR wcChar, BYTE byAttr )
{
	PutCharAttr( iCol, iRow, (char)(wcChar&0x0ff), byAttr );
}

void CXBTextBuffer::PutCharAttr( int iCol, int iRow, char cChar, BYTE byAttr )
{
	if ( m_pBuffer )
	{
		if ( ( iCol > -1 ) && ( iCol < m_iColumns ) &&
			 ( iRow > -1 ) && ( iRow < m_iRows ) )
		{
			int iIndex = (iRow*m_iColumns)+iCol;
			m_pBuffer[iIndex] = MAKEWORD( byAttr, cChar );
			
		}
	}
}

void CXBTextBuffer::PutAttr( int iCol, int iRow, BYTE byAttr )
{
	if ( m_pBuffer )
	{
		if ( ( iCol > -1 ) && ( iCol < m_iColumns ) &&
			 ( iRow > -1 ) && ( iRow < m_iRows ) )
		{
			int iIndex = (iRow*m_iColumns)+iCol;
			WORD wcValue = m_pBuffer[iIndex]&0x0ff;
			m_pBuffer[iIndex] = MAKEWORD( byAttr, wcValue );
			
		}
	}
}

void CXBTextBuffer::PutRow( int iRow, WORD * wData )
{
	if ( m_pBuffer )
	{
		if ( ( iRow > -1 ) && ( iRow < m_iRows ) )
		{
			memcpy( &m_pBuffer[iRow*m_iColumns], wData, sizeof(WORD)*m_iColumns );
		}
	}
}

void	CXBTextBuffer::ClearRow( int iRow )
{
	if ( m_pBuffer )
	{
		if ( ( iRow > -1 ) && ( iRow < m_iRows ) )
		{
			memset( &m_pBuffer[iRow], 0, sizeof(WORD)*m_iColumns );
		}
	}
}

void	CXBTextBuffer::ClearScreen( void )
{
	if ( m_pBuffer )
	{
		memset( m_pBuffer, 0, sizeof(WORD)*m_iColumns*m_iRows );
	}
}

void CXBTextBuffer::ScrollUp( int iRows, BYTE byFlags )
{
	if ( m_pBuffer )
	{
		if ( iRows > 0 ) 
		{
			if ( iRows >= m_iRows )
			{
				// Clear whole screen
				ClearScreen();
			}
			else
			{
				int iIndex;
				for(  iIndex=0; iRows<m_iRows; iIndex++ )
				{
					PutRow( iIndex, &m_pBuffer[iRows++] );
				}
				for(  ; iIndex<m_iRows; iIndex++ )
				{
					// Clear remaining rows
					ClearRow( iIndex );
				}
			}
		}
	}
}

void CXBTextBuffer::ScrollDown( int iRows, BYTE byFlags )
{
	if ( m_pBuffer )
	{
		if ( iRows > 0 ) 
		{
			if ( iRows >= m_iRows )
			{
				// Clear whole screen
				ClearScreen();
			}
			else
			{
				int iIndex;
				iRows = (m_iRows-iRows)-1;
				for(  iIndex=(m_iRows-1); iRows>-1; iIndex-- )
				{
					PutRow( iIndex, &m_pBuffer[iRows++] );
				}
				for(  ; iIndex>-1; iIndex-- )
				{
					// Clear remaining rows
					ClearRow( iIndex );
				}
			}
		}
	}
}

void CXBTextBuffer::ScrollRight( int iCols, BYTE byFlags )
{
	if ( m_pBuffer )
	{
		if ( iCols >= m_iColumns )
		{
			ClearScreen();
		}
		else
		{
			int iIndex;
			WORD *pwRow = m_pBuffer;
			for( iIndex=0; iIndex<m_iRows; iIndex++ )
			{
				memmove( &pwRow[iCols], pwRow, sizeof(WORD)*(m_iColumns-iCols) );
				memset( pwRow, 0, sizeof(WORD)*iCols );
				pwRow = &pwRow[m_iColumns];
			}
		}
	}
}

void CXBTextBuffer::ScrollLeft( int iCols, BYTE byFlags )
{
	if ( m_pBuffer )
	{
		if ( iCols >= m_iColumns )
		{
			ClearScreen();
		}
		else
		{
			int iIndex;
			WORD *pwRow = m_pBuffer;
			for( iIndex=0; iIndex<m_iRows; iIndex++ )
			{
				memmove( pwRow, &pwRow[iCols], sizeof(WORD)*(m_iColumns-iCols) );
				memset( &pwRow[(m_iColumns-iCols)-1], 0, sizeof(WORD)*iCols );
				pwRow = &pwRow[m_iColumns];
			}
		}
	}
}


WORD *	CXBTextBuffer::GetRow( int iRow )
{
	WORD *pReturn = NULL;
	if ( m_pBuffer )
	{
		if ( ( iRow >= 0 ) &&
			 ( iRow < m_iRows ) )
		{
			pReturn = &m_pBuffer[iRow*m_iColumns];
		}
	}
	return pReturn;
}

WORD	CXBTextBuffer::GetCharAttr( int iCol, int iRow )
{
	WORD wReturn = 0;
	if ( m_pBuffer )
	{
		if ( ( iRow >= 0 ) &&
			 ( iRow < m_iRows ) &&
			 ( iCol >= 0 ) &&
			 ( iCol < m_iColumns ) )
		{
			wReturn = m_pBuffer[(iRow*m_iColumns)+iCol];
		}
	}
	return wReturn;
}

char	CXBTextBuffer::GetChar( int iCol, int iRow )
{
	return GetCharAttr(iCol, iRow ) & 0x0ff;
}


BYTE	CXBTextBuffer::GetAttr( int iCol, int iRow )
{
	return GetCharAttr(iCol, iRow ) >> 8;
}



// Now we need a class to render the screen from above.
// This should be a class we can derive from with virtual functions
// so the attributes can be set by the programmer.

class CXBTextConsoleRenderer
{
public:
	CXBTextConsoleRenderer( );
	~CXBTextConsoleRenderer();
	CXBTextBuffer *m_pTextBuffer;
	RECT m_rcArea;
	void SetTextBuffer(CXBTextBuffer *pTextBuffer ) { m_pTextBuffer = pTextBuffer; };
	HRESULT RenderBuffer(LPDIRECT3DDEVICE8 p3DDevice );
	void SetArea( LPRECT pRect ) 
	{
		m_rcArea.top = pRect->top;
		m_rcArea.bottom = pRect->bottom;
		m_rcArea.left = pRect->left;
		m_rcArea.right = pRect->right; 
	};



};




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


// Offsets for building matrices which are used to animate the gamepad controls.
D3DXVECTOR3 g_vLeftTriggerAxis    = D3DXVECTOR3( 1.0f, -0.13f, 0.0f );
D3DXVECTOR3 g_vLeftTriggerOffset  = D3DXVECTOR3( 0.0f, 12.0f, 38.0f );
D3DXVECTOR3 g_vRightTriggerAxis   = D3DXVECTOR3( 1.0f, 0.13f, 0.0f );
D3DXVECTOR3 g_vRightTriggerOffset = D3DXVECTOR3( 0.0f, 12.0f, 38.0f );
D3DXVECTOR3 g_vDPadOffset         = D3DXVECTOR3( -36.70f, -26.27f, 12.0f );
D3DXVECTOR3 g_vLeftStickOffset    = D3DXVECTOR3( -48.54f,   8.72f, 18.0f );
D3DXVECTOR3 g_vRightStickOffset   = D3DXVECTOR3(  36.70f, -26.27f, 18.0f );


// Structures for animating, highlighting, and texturing the gamepad controls.
BOOL               g_ControlActive[NUM_CONTROLS];
D3DXMATRIX         g_ControlMatrix[NUM_CONTROLS];
LPDIRECT3DTEXTURE8 g_ControlTexture[NUM_CONTROLS];


// Global world matrix, so the app can orient the gamepad
D3DXMATRIX g_matWorld( 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 );


class CXBoxStateApplication;

#define MAX_STATES    20

class CXBoxGameState
{
	// Maintain parent application object.
protected:
	CXBoxStateApplication *	m_pbxApplication;
	int					m_iID;
public:
	CXBoxGameState();
	CXBoxGameState( CXBoxStateApplication * pbxApplication );
	void	SetParent( CXBoxStateApplication * pbxApplication ) { m_pbxApplication = pbxApplication; };
    virtual HRESULT Initialize()            { return S_OK; }
    virtual HRESULT FrameMove()             { return S_OK; }
    virtual HRESULT Render()                { return S_OK; }
    virtual HRESULT Cleanup()               { return S_OK; }
	void	SetGameStateID( int iID )		{ m_iID = iID; };
	int		GetGameStateID( void )			{ return m_iID; };
	virtual HRESULT OnActivation( void )	{ return S_OK; };
};


CXBoxGameState::CXBoxGameState( CXBoxStateApplication * pbxApplication )
{
	SetParent( pbxApplication );
}

class CXBoxStateApplication : public CXBApplication
{
	CXBoxGameState *	m_pGameStates[MAX_STATES];
	CXBoxGameState *	m_pCurrentGameState;
	int					m_iNumStates;
protected:
    virtual HRESULT Initialize();
    virtual HRESULT FrameMove();
    virtual HRESULT Render();
    virtual HRESULT Cleanup();
	HRESULT AddGameState( CXBoxGameState * pGameState );
public:
    HRESULT RenderGradientBackground( DWORD dwTopColor, DWORD dwBottomColor )
	{
		return CXBApplication::RenderGradientBackground( dwTopColor, dwBottomColor );
	};

	HRESULT SetGameState( int iID );
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
    HRESULT DrawBox( FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2,
                     DWORD dwFillColor, DWORD dwOutlineColor );

	HRESULT DrawTexture( IDirect3DBaseTexture8 * pTexture, int iPosX, int iPosY, int iSizeX, int iSizeY );
    CXBPackedResource  m_xprResource;      // Packed resources for the app
    CXBFont            m_Font16;           // 16-point font class
    CXBFont            m_Font12;           // 12-point font class

    // Active gamepad
    DWORD              m_dwNumInsertedGamepads;
    XBGAMEPAD*         m_pGamepad;

	void Reboot( LPCSTR szTitlePath = NULL )
	{
		LAUNCH_DATA  LaunchData;

		memset( &LaunchData, 0, sizeof( LAUNCH_DATA) );
		
		XLaunchNewImage( NULL, &LaunchData );
	};

};

CXBoxStateApplication::CXBoxStateApplication() :
	CXBApplication(),
	m_iNumStates(0),
	m_pCurrentGameState(NULL)

{
}

CXBoxStateApplication::~CXBoxStateApplication()
{
	int iIndex=0;
	for( iIndex=0; iIndex<m_iNumStates; iIndex++ )
	{
		delete m_pGameStates[iIndex];
	}
}

HRESULT CXBoxStateApplication::AddGameState( CXBoxGameState * pGameState )
{
	HRESULT hr = S_OK;
	if ( m_iNumStates < (MAX_STATES-1) )
	{
		pGameState->SetParent( this );
		pGameState->SetGameStateID(m_iNumStates);
		m_pGameStates[m_iNumStates] = pGameState;
		m_iNumStates++; 
	}
	return hr;
}

HRESULT CXBoxStateApplication::SetGameState( int iID )
{
	int iIndex=0;
	HRESULT hr = E_FAIL;
	for( iIndex=0; (iIndex<m_iNumStates) && (hr == E_FAIL); iIndex++ )
	{
		if ( m_pGameStates[iIndex]->GetGameStateID() == iID )
		{
			m_pCurrentGameState = m_pGameStates[iIndex];
			hr = m_pCurrentGameState->OnActivation();
		}
	}
	return hr;
}

HRESULT CXBoxStateApplication::Initialize()
{
	// Initialize ALL game states here....
	HRESULT hr = S_OK;
	int iIndex;

    // Create the resources
    if( FAILED( m_xprResource.Create( m_pd3dDevice, "Resource.xpr", 
                                      resource_NUM_RESOURCES ) ) )
        return XBAPPERR_MEDIANOTFOUND;

    // Create the fonts
    if( FAILED( m_Font16.Create( m_pd3dDevice, "Font16.xpr" ) ) )
        return XBAPPERR_MEDIANOTFOUND;
    if( FAILED( m_Font12.Create( m_pd3dDevice, "Font12.xpr" ) ) )
        return XBAPPERR_MEDIANOTFOUND;

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

	for( iIndex=0; (iIndex<m_iNumStates) && (hr == S_OK); iIndex++ )
	{
		hr = m_pGameStates[iIndex]->Initialize();
	}
	// Now activate the first game state
	SetGameState(0);
	return hr;
}

HRESULT CXBoxStateApplication::FrameMove()
{
	HRESULT hr = E_FAIL;
	if ( m_pCurrentGameState )
	{
		hr = m_pCurrentGameState->FrameMove();
	}
	return hr;
}

HRESULT CXBoxStateApplication::Render()
{
	HRESULT hr = S_OK;
	if ( m_pCurrentGameState )
	{
		hr = m_pCurrentGameState->Render();
	}
    // Present the scene
    m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	return hr;
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

#define FVF_SPRITE (D3DFVF_XYZRHW | D3DFVF_TEX1)


HRESULT CXBoxStateApplication::DrawTexture( IDirect3DBaseTexture8 * pTexture, int iPosX, int iPosY, int iSizeX, int iSizeY )
{

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );

    // Draw the analog gauges
    m_pd3dDevice->SetTexture( 0, pTexture );

	m_pd3dDevice->SetVertexShader( FVF_SPRITE );


#if 0
//    m_pd3dDevice->SetStreamSource( 0, m_pVB, sizeof(XBFONTVERTEX) );
    m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW|D3DFVF_TEX1 );
    m_pd3dDevice->SetPixelShader( NULL );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA );
//    m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
//    m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,         0x08 );
//    m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC,        D3DCMP_GREATEREQUAL );
//    m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,         D3DFILL_SOLID );
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,         D3DCULL_NONE );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,		  FALSE );
//    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,         D3DCULL_CCW );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,        FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE,     D3DFOG_NONE );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_EDGEANTIALIAS,    FALSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
//    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_NONE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
     m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_CLAMP );
     m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_CLAMP );
#endif
////    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
////    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
////    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
//    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
//    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
//    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
//    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
//    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
//    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
//    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
//    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
//    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_CLAMP );
//    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_CLAMP );
//    m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW|D3DFVF_TEX1 );
    struct VERTEX { D3DXVECTOR4 p; FLOAT tu, tv; };
    VERTEX v[4];
    v[0].p = D3DXVECTOR4( iPosX			- 0.5f, iPosY			- 0.5f, 0, 0 );  v[0].tu =   0;		v[0].tv =   0;
    v[1].p = D3DXVECTOR4( iPosX+iSizeX	- 0.5f, iPosY			- 0.5f, 0, 0 );  v[1].tu = iSizeX;	v[1].tv =   0;
    v[2].p = D3DXVECTOR4( iPosX+iSizeX	- 0.5f, iPosY+iSizeY	- 0.5f, 0, 0 );  v[2].tu = iSizeX;	v[2].tv = iSizeY;
    v[3].p = D3DXVECTOR4( iPosX			- 0.5f, iPosY+iSizeY	- 0.5f, 0, 0 );  v[3].tu =   0;		v[3].tv = iSizeY;
    m_pd3dDevice->DrawPrimitiveUP( D3DPT_QUADLIST, 1, v, sizeof(v[0]) );
	return S_OK;
}


HRESULT DrawSprite(IDirect3DDevice8* pDevice, IDirect3DBaseTexture8 * pTexture, int iPosX, int iPosY, int iSizeX, int iSizeY, BOOL bTransparency = TRUE )
{

	IDirect3DVertexBuffer8* pVB;

	struct VERTEX_SPRITE
	{
	float x, y, z, rhw;  // z always 0.0, rhw always 1.0
	float u, v;          // texture coords
	};


	// single square sprite
	VERTEX_SPRITE sprite[4] =
	{ //   x    y  z  w  tu  tv
	{ iPosX, iPosY, 0, 1, 0.0, 0.0 },
	{ iPosX+iSizeX, iPosY, 0, 1, 1.0, 0.0 },
	{ iPosX+iSizeX, iPosY+iSizeY, 0, 1, 1.0, 1.0 },
	{ iPosX, iPosY+iSizeY, 0, 1, 0.0, 1.0 }
	};

	// create vertex buffer
	pDevice->CreateVertexBuffer( 4 * sizeof( VERTEX_SPRITE ), D3DUSAGE_WRITEONLY, FVF_SPRITE, D3DPOOL_DEFAULT, &pVB );

	// lock vertex buffer
	VERTEX_SPRITE* pVerts;
	pVB->Lock( 0, 0, (BYTE**)&pVerts, 0 );

	// fill vertex buffer
	memcpy( pVerts, sprite, sizeof( sprite ));

	// unlock vertex buffer
	pVB->Unlock();

	

	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	if ( bTransparency )
	{
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	}
	else
	{
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
	}
	pDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	pDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );

	// now draw sprite
//	if( SUCCEEDED( pDevice->BeginScene() ))
//	{
		// render objects
		pDevice->SetTexture( 0, pTexture );

		pDevice->SetVertexShader( FVF_SPRITE );
		pDevice->SetStreamSource( 0, pVB, sizeof( VERTEX_SPRITE ));
		pDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );

		// clean up
		pDevice->SetTexture( 0, NULL );
//		pDevice->Render();
//		pDevice->EndScene();

//	}  // if
	pVB->Release();
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DrawBox()
// Desc: Graphics helper function
//-----------------------------------------------------------------------------
HRESULT CXBoxStateApplication::DrawBox( FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2,
                              DWORD dwFillColor, DWORD dwOutlineColor )
{
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
}


class CXBoxStartMenuGameState : public CXBoxGameState
{
	int m_iCurrentSelection;
	LPDIRECT3DTEXTURE8  m_pBackTexture;
	LPDIRECT3DTEXTURE8  m_pChoiceTexture[3];
	LPDIRECT3DTEXTURE8  m_pChoiceTextureH[3];
public:
	CXBoxStartMenuGameState(CXBoxStateApplication * pbxApplication ) : CXBoxGameState ( pbxApplication ) {};
    virtual HRESULT Initialize();
    virtual HRESULT FrameMove();
    virtual HRESULT Render();
    virtual HRESULT Cleanup() { return S_OK; };
};

char * szChoiceNames[] = { "D:\\Media\\Textures\\MenuBegin.bmp",
"D:\\Media\\Textures\\MenuOptions.bmp",
"D:\\Media\\Textures\\MenuScores.bmp" };

char * szChoiceNamesH[] = { "D:\\Media\\Textures\\MenuBeginH.bmp",
"D:\\Media\\Textures\\MenuOptionsH.bmp",
"D:\\Media\\Textures\\MenuScoresH.bmp" };

HRESULT CXBoxStartMenuGameState::Initialize()
{
	m_iCurrentSelection = GS_GAMESCREEN;

	D3DXCreateTextureFromFileEx( 
			m_pbxApplication->Get3dDevice(), 
			"D:\\Media\\Textures\\Menu.bmp", 
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0xff000000, NULL, NULL,
			(LPDIRECT3DTEXTURE8*)&m_pBackTexture
			);
	int iIndex;

	for ( iIndex=0; iIndex<3; iIndex++ )
	{
		D3DXCreateTextureFromFileEx( 
				m_pbxApplication->Get3dDevice(), 
				szChoiceNames[iIndex], 
				D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
				0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
				0xff000000, NULL, NULL,
				(LPDIRECT3DTEXTURE8*)&m_pChoiceTexture[iIndex]
				);
		D3DXCreateTextureFromFileEx( 
				m_pbxApplication->Get3dDevice(), 
				szChoiceNamesH[iIndex], 
				D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
				0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
				0xff000000, NULL, NULL,
				(LPDIRECT3DTEXTURE8*)&m_pChoiceTextureH[iIndex]
				);
	}
	
	return S_OK; 
}

HRESULT CXBoxStartMenuGameState::FrameMove()             
{
	WORD wButtons = m_pbxApplication->GetDefaultGamepad()->wPressedButtons;

	if ( (m_pbxApplication->GetDefaultGamepad()->wButtons & ( GP_STARTBUTTON|GP_BACKBUTTON )) == ( GP_STARTBUTTON|GP_BACKBUTTON ) )
	{
		// Proceed to next state	
		m_pbxApplication->Reboot();
	}
	else if ( ( wButtons & GP_STARTBUTTON ) || (m_pbxApplication->GetDefaultGamepad()->bPressedAnalogButtons[0]) )
	{
		// Proceed to next state
		m_pbxApplication->SetGameState( m_iCurrentSelection );

		// this->m_pbxApplication->Reboot();
	}
	else if ( wButtons & GP_DIRDOWN )
	{
		if ( m_iCurrentSelection == GS_GAMESCREEN )
		{
			m_iCurrentSelection = GS_CONFIG;
		}
		else if ( m_iCurrentSelection == GS_CONFIG )
		{
			m_iCurrentSelection = GS_HIGHSCORE;
		}
		else if ( m_iCurrentSelection == GS_HIGHSCORE )
		{
			m_iCurrentSelection = GS_GAMESCREEN;
		}
	}
	else if ( wButtons & GP_DIRUP )
	{
		if ( m_iCurrentSelection == GS_GAMESCREEN )
		{
			m_iCurrentSelection = GS_HIGHSCORE;
		}
		else if ( m_iCurrentSelection == GS_CONFIG )
		{
			m_iCurrentSelection = GS_GAMESCREEN;
		}
		else if ( m_iCurrentSelection == GS_HIGHSCORE )
		{
			m_iCurrentSelection = GS_CONFIG;
		}
	}
	return S_OK; 
}

HRESULT CXBoxStartMenuGameState::Render()                
{

	
	DrawSprite(m_pbxApplication->Get3dDevice(), m_pBackTexture, 0, 0, 640, 480 );
//	this->m_pbxApplication->RenderGradientBackground( 0xffff0000, 0xff000000 );

//	this->m_pbxApplication->DrawBox( 60, 60, 580, 420, 0x40000000, 0xff0000ff );
//    this->m_pbxApplication->m_Font16.DrawText( 300, 200, 0xffffffff, L"Start Menu");

//    this->m_pbxApplication->m_Font12.DrawText( 300, 240, 0xffffffff, L"Start Game");
//    this->m_pbxApplication->m_Font12.DrawText( 300, 270, 0xffffffff, L"Configure");
//    this->m_pbxApplication->m_Font12.DrawText( 300, 310, 0xffffffff, L"High Scores");
	if ( m_iCurrentSelection == GS_GAMESCREEN )
	{
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pChoiceTextureH[0], 300, 209, 313, 41 );
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pChoiceTexture[1], 220, 295, 313, 41 );
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pChoiceTexture[2], 140, 381, 313, 41 );
//		this->m_pbxApplication->DrawBox( 280, 238, 450, 262, 0x00000000, 0xff00ffff );
	}
	else if ( m_iCurrentSelection == GS_CONFIG )
	{
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pChoiceTexture[0], 300, 209, 313, 41 );
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pChoiceTextureH[1], 220, 295, 313, 41 );
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pChoiceTexture[2], 140, 381, 313, 41 );
//		this->m_pbxApplication->DrawBox( 280, 268, 450, 292, 0x00000000, 0xff00ffff );
	}
	else if ( m_iCurrentSelection == GS_HIGHSCORE )
	{
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pChoiceTexture[0], 300, 209, 313, 41 );
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pChoiceTexture[1], 220, 295, 313, 41 );
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pChoiceTextureH[2], 140, 381, 313, 41 );
//		this->m_pbxApplication->DrawBox( 280, 308, 450, 332, 0x00000000, 0xff00ffff );
	}

	return S_OK; 
}





class CXBoxConfigGameState : public CXBoxGameState
{
	LPDIRECT3DTEXTURE8 m_pBackTexture;
public:
	CXBoxConfigGameState(CXBoxStateApplication * pbxApplication ) : CXBoxGameState ( pbxApplication ) {};
    virtual HRESULT Initialize();
    virtual HRESULT FrameMove();
    virtual HRESULT Render();
    virtual HRESULT Cleanup() { return S_OK; };
};


HRESULT CXBoxConfigGameState::Initialize() 
{ 
	D3DXCreateTextureFromFileEx( 
			m_pbxApplication->Get3dDevice(), 
			"D:\\Media\\Textures\\Options.bmp", 
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0xff000000, NULL, NULL,
			(LPDIRECT3DTEXTURE8*)&m_pBackTexture
			);

	return S_OK; 
}

HRESULT CXBoxConfigGameState::FrameMove()             
{
	WORD wButtons = this->m_pbxApplication->GetDefaultGamepad()->wButtons;

	if ( wButtons & GP_BACKBUTTON )
	{
		// Proceed to next state
		m_pbxApplication->SetGameState( GS_STARTMENU );

		// this->m_pbxApplication->Reboot();
	}
	return S_OK; 
}

HRESULT CXBoxConfigGameState::Render()                
{ 
	DrawSprite(m_pbxApplication->Get3dDevice(), m_pBackTexture, 0, 0, 640, 480 );
//	this->m_pbxApplication->RenderGradientBackground( 0xffffff00, 0xff000000 );

//	this->m_pbxApplication->DrawBox( 60, 60, 580, 420, 0x40000000, 0xff0000ff );
    this->m_pbxApplication->m_Font16.DrawText( 200, 240, 0xffffffff, L"Difficulty:");
    this->m_pbxApplication->m_Font16.DrawText( 350, 240, 0xff44ffff, L"Normal");
	return S_OK; 
}



class CXBoxHighScoreGameState : public CXBoxGameState
{
	int m_iDisplayDifficulty;
	LPDIRECT3DTEXTURE8 m_pBackTexture;
public:
	CXBoxHighScoreGameState(CXBoxStateApplication * pbxApplication ) : CXBoxGameState ( pbxApplication ) {};
    virtual HRESULT Initialize();
    virtual HRESULT FrameMove();
    virtual HRESULT Render();
    virtual HRESULT Cleanup() { return S_OK; };
};

HRESULT CXBoxHighScoreGameState::Initialize() 
{
	m_iDisplayDifficulty = g_iDifficulty;
	D3DXCreateTextureFromFileEx( 
			m_pbxApplication->Get3dDevice(), 
			"D:\\Media\\Textures\\Scores.bmp", 
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0xff000000, NULL, NULL,
			(LPDIRECT3DTEXTURE8*)&m_pBackTexture
			);

	return S_OK; 
}


HRESULT CXBoxHighScoreGameState::FrameMove()             
{
	WORD wButtons = this->m_pbxApplication->GetDefaultGamepad()->wButtons;

	if ( wButtons & GP_BACKBUTTON )
	{
		// Proceed to next state
		m_pbxApplication->SetGameState( GS_STARTMENU );

		// this->m_pbxApplication->Reboot();
	}
	return S_OK; 
}

HRESULT CXBoxHighScoreGameState::Render()                
{ 
	int iScore;
	WCHAR wczName[100];
	int iIndex;

//	this->m_pbxApplication->RenderGradientBackground( 0xffffffff, 0xff000000 );
	DrawSprite(m_pbxApplication->Get3dDevice(), m_pBackTexture, 0, 0, 640, 480 );

	for( iIndex=0; iIndex<10; iIndex++ )
	{
		iScore = g_highScores[m_iDisplayDifficulty].GetScore(iIndex);
		XBUtil_GetWide( g_highScores[m_iDisplayDifficulty].GetName(iIndex), wczName, 100 );

		// Now display info
		m_pbxApplication->m_Font16.DrawText( 100, 140+(30*iIndex), 0xffffffff, wczName );
		swprintf( wczName, L"%d %d %d", iScore, g_highScores[m_iDisplayDifficulty].m_iChanges, m_iDisplayDifficulty );
		m_pbxApplication->m_Font16.DrawText( 400, 140+(30*iIndex), 0xffffffff, wczName );
	}

//	this->m_pbxApplication->DrawBox( 60, 60, 580, 420, 0x40000000, 0xff0000ff );
//    this->m_pbxApplication->m_Font16.DrawText( 300, 240, 0xffffffff, L"**High Scores**");
	return S_OK; 
}




class CXBoxGamescreenGameState : public CXBoxGameState
{
	int m_iCursorX, m_iCursorY;
	HRESULT hr;
	LPDIRECT3DTEXTURE8 m_pArrowTexture;
	LPDIRECT3DTEXTURE8 m_pHighlightTexture;
	LPDIRECT3DTEXTURE8 m_pHighlightTexture2;
	LPDIRECT3DTEXTURE8 m_pMarbleTextures[6];
	CBubblesMatrix * m_pBubbleMatrix;

public:
	CXBoxGamescreenGameState(CXBoxStateApplication * pbxApplication ) : CXBoxGameState ( pbxApplication ) {};
    virtual HRESULT Initialize();
    virtual HRESULT FrameMove();
    virtual HRESULT Render();
    virtual HRESULT Cleanup();
	void DrawHighlight1( void );
	void DrawHighlight2( void );
	void DrawBoard( void );
};

int GetMarbleRow( int iPosY ) 
{
	int iReturn = 0;

	if ( iPosY > 50 )
	{
		iPosY -= 50;
		iPosY /= 50;
		if ( iPosY > 6 )
		{
			iPosY = 6;
		}
		iReturn = iPosY;
	}
	return iReturn;
}

int GetMarbleColumn( int iPosX ) 
{
	int iReturn = 0;

	if ( iPosX > 60 )
	{
		iPosX -= 60;
		iPosX /= 50;
		if ( iPosX > 9 )
		{
			iPosX = 9;
		}
		iReturn = iPosX;
	}
	return iReturn;
}

void CXBoxGamescreenGameState::DrawHighlight1( void )
{
	int iIndex, iInnerIndex;

	// First pass, place the yellow highlights down
	for( iIndex=0; iIndex<7; iIndex++ )
	{
		for( iInnerIndex=0; iInnerIndex<10; iInnerIndex++ )
		{
			if ( m_pBubbleMatrix->At(iIndex, iInnerIndex) != CBubblesMatrix::InvalidColor )
			{
				if ( m_pBubbleMatrix->HighlightAt( iIndex, iInnerIndex ) )
				{
					DrawSprite(m_pbxApplication->Get3dDevice(), m_pHighlightTexture, 65+(iInnerIndex*50), 55+(iIndex*50), 60, 60 );
				}
			}
		}
	}
}

void CXBoxGamescreenGameState::DrawHighlight2( void )
{
	int iIndex, iInnerIndex;

	// First pass, place the yellow highlights down
	for( iIndex=0; iIndex<7; iIndex++ )
	{
		for( iInnerIndex=0; iInnerIndex<10; iInnerIndex++ )
		{
			if ( m_pBubbleMatrix->At(iIndex, iInnerIndex) != CBubblesMatrix::InvalidColor )
			{
				if ( m_pBubbleMatrix->HighlightAt( iIndex, iInnerIndex ) )
				{
					DrawSprite(m_pbxApplication->Get3dDevice(), m_pHighlightTexture2, 68+(iInnerIndex*50), 58+(iIndex*50), 54, 54 );
				}
			}
		}
	}
}

void CXBoxGamescreenGameState::DrawBoard( void )
{
	int iIndex, iInnerIndex;

	// First pass, place the yellow highlights down
	for( iIndex=0; iIndex<7; iIndex++ )
	{
		for( iInnerIndex=0; iInnerIndex<10;iInnerIndex++ )
		{
			if ( m_pBubbleMatrix->At(iIndex, iInnerIndex) != CBubblesMatrix::InvalidColor )
			{
				DrawSprite(m_pbxApplication->Get3dDevice(), m_pMarbleTextures[m_pBubbleMatrix->At(iIndex, iInnerIndex)], 70+(iInnerIndex*50), 60+(iIndex*50), 50, 50 );
			}
		}
	}
}

HRESULT CXBoxGamescreenGameState::Cleanup() 
{ 
	int iIndex;

	for( iIndex=0; iIndex<6; iIndex++ )
	{
		m_pMarbleTextures[iIndex]->Release();
	}
	m_pArrowTexture->Release();
	m_pHighlightTexture->Release();
	m_pHighlightTexture2->Release();

	delete m_pBubbleMatrix;

	return S_OK; 
}

char *szMarbleNames[] = { "D:\\Media\\Textures\\MarbleSky.bmp",
"D:\\Media\\Textures\\MarbleYellow.bmp",
"D:\\Media\\Textures\\MarbleOrange.bmp",
"D:\\Media\\Textures\\MarbleGreen.bmp",
"D:\\Media\\Textures\\MarblePurple.bmp",
"D:\\Media\\Textures\\MarbleGray.bmp" };

HRESULT CXBoxGamescreenGameState::Initialize() 
{ 
	m_iCursorX = 320;
	m_iCursorY = 240;
	
	m_pBubbleMatrix = new CBubblesMatrix (7, 10, 5, 0 );

	hr = D3DXCreateTextureFromFileEx( 
			m_pbxApplication->Get3dDevice(), 
			"D:\\Media\\Textures\\arrow.bmp", 
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0xff000000, NULL, NULL,
			(LPDIRECT3DTEXTURE8*)&m_pArrowTexture
			);
	hr = D3DXCreateTextureFromFileEx( 
			m_pbxApplication->Get3dDevice(), 
			"D:\\Media\\Textures\\highlight.bmp", 
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0xffffffff, NULL, NULL,
			(LPDIRECT3DTEXTURE8*)&m_pHighlightTexture
			);
	hr = D3DXCreateTextureFromFileEx( 
			m_pbxApplication->Get3dDevice(), 
			"D:\\Media\\Textures\\highlight2.bmp", 
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0xffffffff, NULL, NULL,
			(LPDIRECT3DTEXTURE8*)&m_pHighlightTexture2
			);
	
	int iIndex;

	for( iIndex=0; iIndex<6; iIndex++ )
	{
		hr = D3DXCreateTextureFromFileEx( 
			m_pbxApplication->Get3dDevice(), 
			szMarbleNames[iIndex], 
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0xff000000, NULL, NULL,
			(LPDIRECT3DTEXTURE8*)&m_pMarbleTextures[iIndex]
			);
	}
	return S_OK; 
}

int GetPixelX( int iCol )
{
	int iReturn;

	if ( iCol > 9 )
	{
		iCol = 9;
	}
	iReturn = 95+(iCol*50);
	return iReturn;
}

int GetPixelY( int iRow )
{
	int iReturn;

	if ( iRow > 6 )
	{
		iRow = 6;
	}
	iReturn = 85+(iRow*50);
	return iReturn;
}




HRESULT CXBoxGamescreenGameState::FrameMove()             
{
	WORD wButtons = this->m_pbxApplication->GetDefaultGamepad()->wPressedButtons;
	FLOAT fXVal, fYVal;
	int iMarbleCol, iMarbleRow;

	if ( m_pBubbleMatrix->EndOfGame() )
	{
		// Switch to displaying a message and reset game...
		if ( wButtons || 
			m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[0] ||
			m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[1] ||
			m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[2] ||
			m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[3] ||
			m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[4] ||
			m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[5] ||
			m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[6] ||
			m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[7] )
		{
			// Restart game and set state if it's a high score
			if ( g_highScores[g_iDifficulty].IsOnList(m_pBubbleMatrix->GetScore()) )
			{
				// Save score off...
				g_highScores[g_iDifficulty].AddScoreToList( m_pBubbleMatrix->GetScore(), "Lee Sr" );
				g_highScores[g_iDifficulty].SaveScores( "Normal" );
			}
			delete m_pBubbleMatrix;
			m_pBubbleMatrix = new CBubblesMatrix (7, 10, 5, 0 );
		}
	}
	else
	{
		if ( wButtons & GP_BACKBUTTON )
		{
			// Proceed to next state
			m_pbxApplication->SetGameState( GS_STARTMENU );

			// this->m_pbxApplication->Reboot();
		}
		fXVal = m_pbxApplication->GetGamepad(0)->fX1;
		if ( fXVal < 0.10 )
		{
			fXVal = 0.0;
		}
		else
		{
			fXVal -= 0.10;
		}
		fYVal = m_pbxApplication->GetGamepad(0)->fY1;
		if ( fYVal < 0.10 )
		{
			fYVal = 0.0;
		}
		else
		{
			fYVal -= 0.10;
		}

		fXVal *= 20.0;
		fYVal *= -20.0;

		m_iCursorX += (int)fXVal;
		if ( m_iCursorX < 0 )
		{
			m_iCursorX  = 0;
		}
		if ( m_iCursorX > 640 )
		{
			m_iCursorX = 640;
		}
		m_iCursorY += (int)fYVal;
		if ( m_iCursorY < 0 )
		{
			m_iCursorY  = 0;
		}
		if ( m_iCursorY > 480 )
		{
			m_iCursorY = 480;
		}

		iMarbleCol = GetMarbleColumn( m_iCursorX );
		iMarbleRow = GetMarbleRow( m_iCursorY );

		if ( wButtons & GP_DIRUP )
		{
			if ( iMarbleRow > 0 )
			{
				iMarbleRow--;
				m_iCursorY = GetPixelY( iMarbleRow );
			}
		}
		if ( wButtons & GP_DIRDOWN )
		{
			if ( iMarbleRow < 6 )
			{
				iMarbleRow++;
				m_iCursorY = GetPixelY( iMarbleRow );
			}
		}
		if ( wButtons & GP_DIRLEFT )
		{
			if ( iMarbleCol > 0 )
			{
				iMarbleCol--;
				m_iCursorX = GetPixelX( iMarbleCol );
			}
		}
		if ( wButtons & GP_DIRRIGHT )
		{
			if ( iMarbleCol < 9 )
			{
				iMarbleCol++;
				m_iCursorX = GetPixelX( iMarbleCol );
			}
		}

		// User pressed "A"
		if ( m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[GP_ANALOG_A] )
		{
			if ( m_pBubbleMatrix->At(iMarbleRow, iMarbleCol) != CBubblesMatrix::InvalidColor )
			{
				m_pBubbleMatrix->RemoveBubble( iMarbleRow, iMarbleCol );
			}
			else
			{
				m_pBubbleMatrix->ClearHighlight();
			}
		}
		if ( m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[GP_ANALOG_B] )
		{
			// Clear highlight
			m_pBubbleMatrix->ClearHighlight();
		}
	}
	// GP_ANALOG_A

	return S_OK; 
}

HRESULT CXBoxGamescreenGameState::Render()                
{ 
	WCHAR szString[200];


	this->m_pbxApplication->RenderGradientBackground( 0xff00ffff, 0xff000000 );

	this->m_pbxApplication->DrawBox( 60, 60, 580, 420, 0x40000000, 0xff00ffff );

	// Draw the marbles....
	// m_pHighlightTexture
	int iIndex, iInnerIndex;


	DrawHighlight1();
	DrawHighlight2();
	DrawBoard();
#if 0
	// First pass, place the yellow highlights down
	for( iIndex=0; iIndex<6; iIndex++ )
	{
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pHighlightTexture, 65+(4*50), 55+(iIndex*50), 60, 60 );
	}
	for( iInnerIndex=4; iInnerIndex<8; iInnerIndex++ )
	{
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pHighlightTexture, 65+(iInnerIndex*50), 55+(6*50), 60, 60 );
	}

	// Second pass, place the black highlights down
	for( iIndex=0; iIndex<6; iIndex++ )
	{
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pHighlightTexture2, 68+(4*50), 58+(iIndex*50), 54, 54 );
	}
	for( iInnerIndex=4; iInnerIndex<8; iInnerIndex++ )
	{
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pHighlightTexture2, 68+(iInnerIndex*50), 58+(6*50), 54, 54 );
	}

	for( iIndex=0; iIndex<6; iIndex++ )
	{
		for( iInnerIndex=0; iInnerIndex<10; iInnerIndex++ )
		{
			DrawSprite(m_pbxApplication->Get3dDevice(), m_pMarbleTextures[iIndex], 70+(iInnerIndex*50), 60+(iIndex*50), 50, 50 );
		}
	}
	for( iInnerIndex=0; iInnerIndex<10; iInnerIndex++ )
	{
		DrawSprite(m_pbxApplication->Get3dDevice(), m_pMarbleTextures[0], 70+(iInnerIndex*50), 60+(iIndex*50), 50, 50 );
	}
#endif
    this->m_pbxApplication->m_Font16.DrawText( 302, 22, 0xff000000, L"**GameScreen**");
    this->m_pbxApplication->m_Font16.DrawText( 300, 20, 0xffffffff, L"**GameScreen**");

	swprintf( szString, L"Score: %d", m_pBubbleMatrix->GetScore() );

    m_pbxApplication->m_Font12.DrawText( 32, 422, 0xff000000, szString);
    m_pbxApplication->m_Font12.DrawText( 28, 418, 0xff000000, szString);
    m_pbxApplication->m_Font12.DrawText( 30, 420, 0xffffffff, szString);

	if ( m_pBubbleMatrix->GetUndoScore() )
	{
		swprintf( szString, L"Undo: %d", m_pBubbleMatrix->GetUndoScore() );

		m_pbxApplication->m_Font12.DrawText( 300, 422, 0xff000000, szString);
		m_pbxApplication->m_Font12.DrawText( 296, 418, 0xff000000, szString);
		m_pbxApplication->m_Font12.DrawText( 298, 420, 0xffffffff, szString);
	}

	if ( m_pBubbleMatrix->GetWouldBeScore() )
	{
		swprintf( szString, L"Current: %d", m_pBubbleMatrix->GetWouldBeScore() );

		m_pbxApplication->m_Font12.DrawText( 420, 422, 0xff000000, szString);
		m_pbxApplication->m_Font12.DrawText( 416, 418, 0xff000000, szString);
		m_pbxApplication->m_Font12.DrawText( 418, 420, 0xffffffff, szString);
	}


//	swprintf( szString, L"Gamepad: %d / %d / %d / %d", m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[4],
//		m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[5],
//		m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[6],
//		m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[7]);

//    m_pbxApplication->m_Font12.DrawText( 20, 40, 0xffffffff, szString);

//	m_xprResource.GetTexture( dwTexture )
	// Draw a cursor on the screen
	// resource_CursorArrow_OFFSET
//	m_pbxApplication->DrawTexture( m_pArrowTexture, m_iCursorX, m_iCursorY, 256, 256 );
	if ( m_pBubbleMatrix->EndOfGame() )
	{
		swprintf( szString, L"Great Game!\nHit any button to continue!" );

		m_pbxApplication->m_Font12.DrawText( 280, 222, 0xff000000, szString);
		m_pbxApplication->m_Font12.DrawText( 276, 218, 0xff000000, szString);
		m_pbxApplication->m_Font12.DrawText( 278, 220, 0xffffffff, szString);
	}
	else
	{

		DrawSprite(m_pbxApplication->Get3dDevice(), m_pArrowTexture, m_iCursorX, m_iCursorY, 32, 32 );

		swprintf( szString, L"Marble: X=%d Y=%d  %s%s%s%s", GetMarbleColumn( m_iCursorX ), GetMarbleRow( m_iCursorY ),
			m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[4]?L"X":L"O",
			m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[5]?L"X":L"O",
			m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[6]?L"X":L"O",
			m_pbxApplication->GetGamepad(0)->bPressedAnalogButtons[7]?L"X":L"O"
			);
		m_pbxApplication->m_Font12.DrawText( 22, 442, 0xff000000, szString);
		m_pbxApplication->m_Font12.DrawText( 18, 438, 0xff000000, szString);
		m_pbxApplication->m_Font12.DrawText( 20, 440, 0xffffffff, szString);
	}


		
//		m_pbxApplication->m_xprResource.GetTexture(resource_CursorArrow_OFFSET), m_iCursorX, m_iCursorY, 32, 32 );

	return S_OK; 
}


class CXBoxTitleGameState : public CXBoxGameState
{
	DWORD m_dwTimeStamp;
	LPDIRECT3DTEXTURE8  m_pTitleTexture;
public:
	CXBoxTitleGameState(CXBoxStateApplication * pbxApplication ) : CXBoxGameState ( pbxApplication ) {};
    virtual HRESULT Initialize();
    virtual HRESULT FrameMove();
    virtual HRESULT Render();
    virtual HRESULT Cleanup();
	HRESULT OnActivation( void )	{ m_dwTimeStamp = GetTickCount()+20000; return S_OK; };
};


class CXMarblesApp : public CXBoxStateApplication
{
public:
	CXMarblesApp();

};

HRESULT CXBoxTitleGameState::Initialize()            
{
	D3DXCreateTextureFromFileEx( 
			m_pbxApplication->Get3dDevice(), 
			"D:\\Media\\Textures\\title.bmp", 
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			0xff000000, NULL, NULL,
			(LPDIRECT3DTEXTURE8*)&m_pTitleTexture
			);
	return S_OK; 
}

HRESULT CXBoxTitleGameState::FrameMove()             
{
	WORD wButtons = this->m_pbxApplication->GetDefaultGamepad()->wButtons;

	if ( m_dwTimeStamp && m_dwTimeStamp < GetTickCount() )
	{
		// Proceed to next state
		m_pbxApplication->SetGameState( GS_STARTMENU );
	}
	if ( wButtons & GP_STARTBUTTON )
	{
		// Proceed to next state
		m_pbxApplication->SetGameState( GS_STARTMENU );

		// this->m_pbxApplication->Reboot();
	}
	return S_OK; 
}

HRESULT CXBoxTitleGameState::Render()                
{ 
//	WCHAR szGamePad[80];


	this->m_pbxApplication->RenderGradientBackground( 0xff00ff00, 0xff000000 );
	DrawSprite(m_pbxApplication->Get3dDevice(), m_pTitleTexture, 0, 0, 640, 480, FALSE );

//	swprintf( szGamePad, L"Gamepad: 0x%04X", this->m_pbxApplication->GetGamepad(0)->wButtons );
	// Draw "Hello World!" in the center....

//	this->m_pbxApplication->DrawBox( 0, 0, 640, 480, 0x40000000, 0xff000000 );
//	this->m_pbxApplication->DrawBox( 60, 60, 580, 420, 0x40000000, 0xff0000ff );
//    this->m_pbxApplication->m_Font16.DrawText( 300, 240, 0xffffffff, L"XMarbles!");
//    this->m_pbxApplication->m_Font12.DrawText( 300, 280, 0xffffffff, szGamePad );

//	swprintf( szGamePad, L"Font12: FontHeight=%d", this->m_pbxApplication->m_Font12.m_dwFontHeight );
//    this->m_pbxApplication->m_Font12.DrawText( 60, 300, 0xffffffff, szGamePad );

//	swprintf( szGamePad, L"Font12: TexWidth/Height=%d/%d", this->m_pbxApplication->m_Font12.m_dwTexWidth, this->m_pbxApplication->m_Font12.m_dwTexHeight );
//    this->m_pbxApplication->m_Font12.DrawText( 60, 320, 0xffffffff, szGamePad );

//	swprintf( szGamePad, L"Font12: Min/Max Widths=%d/%d (%d glyphs)", iNarrowest, iWidest, dwNumGlyphs );
//    this->m_pbxApplication->m_Font12.DrawText( 60, 340, 0xffffffff, szGamePad );

	return S_OK; 

}

HRESULT CXBoxTitleGameState::Cleanup()               
{ 
	return S_OK; 
}

CXMarblesApp::CXMarblesApp() 
	: CXBoxStateApplication()
{
	CXBoxGameState* pGameState;
	// Set up our game states here....

	g_highScores[0].LoadScores( "Easy" );
	g_highScores[1].LoadScores( "Normal" );
	g_highScores[2].LoadScores( "Hard" );

	pGameState = new CXBoxTitleGameState(this);
	AddGameState( (CXBoxGameState*)pGameState );
	pGameState->SetGameStateID(	GS_TITLE );
	SetGameState( GS_TITLE );

	pGameState = new CXBoxStartMenuGameState(this);
	AddGameState( (CXBoxGameState*)pGameState );
	pGameState->SetGameStateID(	GS_STARTMENU );

	pGameState = new CXBoxConfigGameState(this);
	AddGameState( (CXBoxGameState*)pGameState );
	pGameState->SetGameStateID(	GS_CONFIG );

	pGameState = new CXBoxHighScoreGameState(this);
	AddGameState( (CXBoxGameState*)pGameState );
	pGameState->SetGameStateID(	GS_HIGHSCORE );

	pGameState = new CXBoxGamescreenGameState(this);
	AddGameState( (CXBoxGameState*)pGameState );
	pGameState->SetGameStateID(	GS_GAMESCREEN );



}

//-----------------------------------------------------------------------------
// Name: main()
// Desc: Entry point to the program.
//-----------------------------------------------------------------------------
VOID __cdecl main()
{
	g_iDifficulty = 1;
    CXMarblesApp xbApp;
    if( FAILED( xbApp.Create() ) )
        return;
    xbApp.Run();
}


