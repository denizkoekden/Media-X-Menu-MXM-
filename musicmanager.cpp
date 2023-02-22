//-----------------------------------------------------------------------------
// File: MusicManager.cpp
//
// Desc: Implementation file for CMusicManager class.
//
// Hist: 8.20.01 - New for October XDK
//      10.05.02 - Modified to handle dynamic game soundtrack
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "musicmanager.h"
#include "XbUtil.h"
#include <assert.h>
#include <stdio.h>
#include <tchar.h>
#include "utils.h"
#include "MXM.h"
#include "CommDebug.h"

CStdString g_sMusicFiles;

short int g_MMOutBuffer[MMOUT_BUFFER_SIZE];

//
// Here is our game soundtrack including WMA files we ship
// with our game.  If needed, this concept could be extended
// to include several different game soundtracks
//


//MM_SONG g_aGameSoundtrack[] =
//{
//    { L"Becky", "D:\\Media\\Sounds\\Becky.wma", 165000 },
//    { L"Becky remix", "D:\\Media\\Sounds\\Becky.wma", 165000 },
//};

// #define NUM_GAME_SONGS ( sizeof( g_aGameSoundtrack ) / sizeof( g_aGameSoundtrack[0] ) )

#define WORKER_THREAD_STACK_SIZE 12 * 1024
#define WMA_LOOKAHEAD 64 * 1024
#define YIELD_RATE 3

//
// The following methods are safe to be called from the main rendering thread.
// In order to avoid CPU-intensive thread synchronization, we assume that only
// one thread will ever be calling into these functions.  That way, we know
// that if there is no state change already pending, then the state cannot 
// possibly change during a function call.  Threading issues are noted for
// each of these functions
//

//-----------------------------------------------------------------------------
// Name: CMusicManager (ctor)
// Desc: Initializes member variables
// Thread issues: None - worker thread is not created yet.
//-----------------------------------------------------------------------------
CMusicManager::CMusicManager()
{
	m_bVolumeNotSet = FALSE;
	m_bEnable		= TRUE;
    m_mmOldState    = MM_STOPPED;
    m_mmState       = MM_STOPPED;
    m_fRandom       = FALSE;
    m_fGlobal       = TRUE;
    m_lVolume       = DSBVOLUME_MAX;
    m_dwStream      = 0;
    m_dwSong        = 0;
    m_szSong[0]     = 0;
    m_dwLength      = 0;
    m_dwSongID      = 0;

	m_pdwRandomTracks	=	new DWORD [1024];
	m_lRandomTrackAlloc	=	1024;
	m_lTrackCount		=	0;
	m_lTrackPos			=	0;


    m_aSoundtracks          = NULL;
    m_uSoundtrackCount      = 0;
    m_uCurrentSoundtrack    = 0;
    m_dwPacketsCompleted    = 0;

    m_pbSampleData  = NULL;
    m_pDecoder      = NULL;
    m_pStream[0]    = NULL;
    m_pStream[1]    = NULL;
    m_hDecodingFile = INVALID_HANDLE_VALUE;

    m_hThread           = INVALID_HANDLE_VALUE;
    m_hShutdownEvent    = INVALID_HANDLE_VALUE;

    for( INT i = 0; i < PACKET_COUNT + EXTRA_PACKETS; i++ )
        m_adwPacketStatus[i] = XMEDIAPACKET_STATUS_SUCCESS;
}

DWORD CMusicManager::GetNextRandom( void )
{
	DWORD dwReturn = 0;

	if ( m_bRandGlobal != m_fGlobal )
	{
		BuildRandomTracks();
	}
	if ( m_pdwRandomTracks != NULL )
	{
		int iTrack, iSong;
		m_lTrackPos++;
		if ( m_lTrackCount )
		{
			m_lTrackPos %= m_lTrackCount;
		}
		else
		{
			m_lTrackPos = 0;
		}
		if ( m_lTrackPos == 0 && m_lTrackCount > 1 )
		{
			BuildRandomTracks();
		}
		dwReturn = m_pdwRandomTracks[m_lTrackPos];
		iTrack  = dwReturn&0xFFFF0000;
		iTrack >>= 16;
		iSong = dwReturn&0x0ffff;
		if ( IsValidSong( iTrack, iSong ) == FALSE )
		{
			dwReturn = 0;
		}
	}
	return dwReturn;
}

void CMusicManager::BuildRandomTracks( void )
{
	DWORD lSongCount = 0;
	DWORD lGlobalSongCount = 0;
	DWORD lGameSongCount = 0;
	int iCurTrack = 0;

	m_bRandGlobal = TRUE;

	for( iCurTrack = 0; iCurTrack<(int)m_uSoundtrackCount; iCurTrack++ )
	{
		lSongCount += m_aSoundtracks[ iCurTrack ].m_uSongCount;
		if ( !m_fGlobal )
		{
			m_bRandGlobal = FALSE;
			break;
		}
	}

	if ( m_lRandomTrackAlloc < (int)lSongCount )
	{
		delete [] m_pdwRandomTracks;
		m_pdwRandomTracks = NULL;
		m_lRandomTrackAlloc = ((lSongCount/1024)+1)*1024;
		m_pdwRandomTracks = new DWORD [m_lRandomTrackAlloc];
		ZeroMemory(m_pdwRandomTracks, sizeof(DWORD)*m_lRandomTrackAlloc);
	}
	if ( m_pdwRandomTracks )
	{
		int iCurSong, iRandomIndex = 0;
		DWORD dwSongCode;

		ZeroMemory( m_pdwRandomTracks, m_lRandomTrackAlloc*sizeof(DWORD) );
		m_lTrackCount = lSongCount;
		m_lTrackPos = 0;
		for( iCurTrack = 0; iCurTrack<(int)m_uSoundtrackCount; iCurTrack++ )
		{
			dwSongCode = iCurTrack;
			dwSongCode <<= 16;
			for( iCurSong=0; iCurSong<(int)(m_aSoundtracks[ iCurTrack ].m_uSongCount); iCurSong++ )
			{
				m_pdwRandomTracks[iRandomIndex] = dwSongCode;
				dwSongCode++;
				iRandomIndex++;
			}
			if ( !m_fGlobal )
			{
				break;
			}
		}
		if ( m_lTrackCount )
		{
			// Now we've created an index list, scramble it like eggs!!
			int iSwap1 = RandomNum()%m_lTrackCount;
			int iSwap2 = RandomNum()%m_lTrackCount;
			int	iSwap3 = RandomNum()%m_lTrackCount;
			DWORD dwTemp1, dwTemp2; // , dwTemp3;
			for ( iCurSong=0; iCurSong<m_lTrackCount; iCurSong++ )
			{
				iSwap1 = RandomNum()%m_lTrackCount;
				iSwap2 = RandomNum()%m_lTrackCount;
				if ( iSwap1 == iCurSong )
				{
					iSwap1++;
					iSwap1 %= m_lTrackCount;
				}
				if ( iSwap2 == iCurSong )
				{
					iSwap2++;
					iSwap2 %= m_lTrackCount;
				}
				if ( iSwap2 == iSwap1 )
				{
					iSwap2++;
					iSwap2 %= m_lTrackCount;
				}
				dwTemp1 = m_pdwRandomTracks[iCurSong];
				m_pdwRandomTracks[iCurSong] = m_pdwRandomTracks[iSwap1];
				m_pdwRandomTracks[iSwap1] = dwTemp1;

				dwTemp2 = m_pdwRandomTracks[iCurSong];
				m_pdwRandomTracks[iCurSong] = m_pdwRandomTracks[iSwap2];
				m_pdwRandomTracks[iSwap2] = dwTemp2;
			}
		}
	}







	//m_pdwRandomTracks	=	new DWORD [1024];
	//m_lRandomTrackAlloc	=	1024;
	//m_lTrackCount		=	0;
	//m_lTrackPos			=	0;

}

//-----------------------------------------------------------------------------
// Name: ~CMusicManager (dtor)
// Desc: Releases any resources allocated by the object
// Thread issues: This function waits for the worker thread to shut down
//                before continuing processing.  
//-----------------------------------------------------------------------------
CMusicManager::~CMusicManager()
{
	if ( m_pdwRandomTracks )
	{
		delete [] m_pdwRandomTracks;
		m_pdwRandomTracks = NULL;
	}

    // Signal our worker thread to shutdown, then wait for it
    // before releasing objects
    if( m_hShutdownEvent != INVALID_HANDLE_VALUE )
        SetEvent( m_hShutdownEvent );
    
    if( m_hThread != INVALID_HANDLE_VALUE )
    {
        WaitForSingleObject( m_hThread, INFINITE );
        CloseHandle( m_hThread );
    }

    // We can't close the handle until we're sure the worker
    // thread has terminated.
    if( m_hShutdownEvent != INVALID_HANDLE_VALUE )
        CloseHandle( m_hShutdownEvent );

    // Now that we've shut down the worker thread, it's safe
    // to clean these guys up.
    if( m_pStream[0] )
    {
        m_pStream[0]->Pause( DSSTREAMPAUSE_RESUME );
        m_pStream[0]->Release();
    }

    if( m_pStream[1] )
    {
        m_pStream[1]->Pause( DSSTREAMPAUSE_RESUME );
        m_pStream[1]->Release();
    }

    if( m_pbSampleData )
        delete[] m_pbSampleData;

	delete [] m_aSoundtracks;
//    free( m_aSoundtracks );

}

void	CMusicManager::GetCurrentTrackAndSong( DWORD & dwTrack, DWORD & dwSongID )
{
	dwSongID = m_dwSongID;
	dwTrack = m_uCurrentSoundtrack;
}

//-----------------------------------------------------------------------------
// Name: Initialize
// Desc: Sets up the object to run
// Thread issues: None - worker thread isn't created until end of function
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::Initialize( BOOL bSpinThread /* TRUE */ )
{
    HRESULT hr;

//	DEBUG_LINE( _T("MM: Initialize") );
    // Load up soundtrack information
    hr = LoadSoundtracks();
    if( FAILED( hr ) )
        return hr;

//	return S_OK;

	SelectSoundtrack(0);

    srand( GetTickCount() );

    // Allocate sample data buffer
    m_pbSampleData = new BYTE[ ( PACKET_COUNT + EXTRA_PACKETS ) * MMPACKET_SIZE ];
    if( !m_pbSampleData )
        return E_OUTOFMEMORY;

    // Soundtrack songs are ripped to this format.  Any individual
    // WMA tracks shipped with the game will need to be encoded
    // similarly.
    // Alternately, streams could be released and re-created when
    // switching tracks, but this would incur additional CPU costs.  
    // In the future, a SetFormat() routine on DirectSound streams 
    // and buffers will make this simpler.
    WAVEFORMATEX wfx = {0};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = 44100;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

    // Set up a stream desc to create our streams
    DSSTREAMDESC dssd = {0};
    dssd.dwMaxAttachedPackets = PACKET_COUNT;
    dssd.lpwfxFormat = &wfx;
    dssd.lpfnCallback = StreamCallback;
    dssd.lpvContext = this;

    // Create the streams
    hr = DirectSoundCreateStream( &dssd, &m_pStream[0] );
    if( FAILED( hr ) )
        return hr;
    hr = DirectSoundCreateStream( &dssd, &m_pStream[1] );
    if( FAILED( hr ) )
        return hr;

    // Set up attack envelopes to fade in for us
    DSENVELOPEDESC dsed = {0};
    dsed.dwEG = DSEG_AMPLITUDE;
    dsed.dwMode = DSEG_MODE_ATTACK;
    dsed.dwAttack = DWORD(48000 * FADE_TIME / 512);
    dsed.dwRelease = DWORD(48000 * FADE_TIME / 512);
    dsed.dwSustain = 255;
    m_pStream[0]->SetEG( &dsed );
    m_pStream[1]->SetEG( &dsed );


#if _DEBUG
    if( FAILED( DebugVerify() ) )
        return E_FAIL;
#endif // _DEBUG

    if( bSpinThread )
    {
        // Create an event to signal the worker thread to shutdown
        m_hShutdownEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if( NULL == m_hShutdownEvent )
            return E_FAIL;

        // Create the worker thread
        m_hThread = CreateThread( NULL, WORKER_THREAD_STACK_SIZE, MusicManagerThreadProc, this, 0, NULL );
        if( NULL == m_hThread )
        {
            return E_FAIL;
        }
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Play
// Desc: Starts playing background music
// Thread issues: Depends on worker thread state:
//                MM_PLAYING: No effective change.  Worker thread could be
//                            processing a state change to MM_PAUSED or
//                            MM_STOPPED, in which case we will switch back
//                            to MM_PLAYING on the next iteration of Process()
//                MM_PAUSED:  Next iteration of Process() will pick up the 
//                            state change and resume playback
//                MM_STOPPED: Next iteration of Process() will pick up the
//                            state change and stop playback
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::Play()
{
//	DEBUG_LINE( _T("MM: Play") );
//	CStdString sTemp;
	bool bPlay = true;
	if ( m_fGlobal )
	{
		// Verify that current song selection is there
		if ( GetSongCount() == 0 )
		{
			bPlay = false;
		}
	}
	else
	{
		if( m_aSoundtracks && (m_uCurrentSoundtrack<m_uSoundtrackCount) )
		{
			if ( m_dwSong >= m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() )
			{
				SetFirstSong();
			}
			if ( m_dwSong >= m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() )
			{
//				sTemp.Format(_T("MM: Play - Unable to find song! ST=%d Sng=%d SngCnt=%d"), m_uCurrentSoundtrack, m_dwSong, m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() );
//				DEBUG_LINE( sTemp.c_str() );
				bPlay = false;
			}
		}
		else
		{
			SetFirstSong();
			if ( m_aSoundtracks && (m_uCurrentSoundtrack<m_uSoundtrackCount) )
			{
//			sTemp.Format(_T("MM: Play - Invalid soundtrack value! ST=%d STCnt=%d"), m_uCurrentSoundtrack, m_uSoundtrackCount );
//			DEBUG_LINE( sTemp.c_str() );
				if ( m_dwSong >= m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() )
				{
					bPlay = false;
				}
			}
			else
			{
				bPlay = false;
			}
		}
	}

    if( m_mmState == MM_STOPPED )
	{
        Prepare();
	}
    else if( m_mmState == MM_PAUSED )
    {
        if( m_pStream[0] )
            m_pStream[0]->Pause( DSSTREAMPAUSE_RESUME );
        if( m_pStream[1] )
            m_pStream[1]->Pause( DSSTREAMPAUSE_RESUME );
    }

    m_mmState = MM_PLAYING;

	if ( m_bVolumeNotSet )
	{
		m_bVolumeNotSet = TRUE;
		SetVolume( m_lVolume );
	}

 //   // Can always transition to playing
	//if ( bPlay )
	//{
	//	m_mmState = MM_PLAYING;
	//}
	//else
	//{
	//	m_mmState = MM_STOPPED;
	//}
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Stop
// Desc: Stops background music playback
// Thread issues: Depends on worker thread state:
//                MM_PLAYING: Next iteration of Process() will pick up the
//                            state change and stop playback
//                MM_PAUSED:  Next iteration of Process() will pick up the 
//                            state change and resume playback
//                MM_STOPPED: No effective change.  Worker thread could be
//                            processing a state change to MM_PLAYING, in
//                            which case we will switch back to MM_STOPPED
//                            on the next iteration of Process()
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::Stop()
{

    if( m_mmState != MM_STOPPED )
    {
        if( m_pStream[0] )
            m_pStream[0]->FlushEx( 0, DSSTREAMFLUSHEX_ASYNC );
        if( m_pStream[1] )
            m_pStream[1]->FlushEx( 0, DSSTREAMFLUSHEX_ASYNC );
        Cleanup();
    }

    m_mmState = MM_STOPPED;
	
	////	DEBUG_LINE( _T("MM: Stop") );
 //   // Can always transition to stopped
 //   if( m_mmOldState != MM_STOPPED &&
 //       m_mmState    != MM_STOPPED )
 //   {
 //       m_mmState = MM_STOPPED;

 //       // Pause the streams right now so that we
 //       // stop playback.  They'll get shut down
 //       // fully on the worker thread
 //       m_pStream[0]->Pause( DSSTREAMPAUSE_PAUSE );
 //       m_pStream[1]->Pause( DSSTREAMPAUSE_PAUSE );
 //   }

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Pause
// Desc: Pauses background music playback
// Thread issues: Since we can only transition to MM_PAUSED from MM_PLAYING,
//                make sure that we're already at MM_PLAYING, and don't have
//                a pending state change.  Next iteration of Process() will
//                pick up the change to MM_PAUSED and pause playback
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::Pause()
{
    if( m_mmState != MM_PLAYING )
        return S_FALSE;

    // Can only transition to paused from playing.
    if( m_mmState == MM_PLAYING )
    {
        if( m_pStream[0] )
            m_pStream[0]->Pause( DSSTREAMPAUSE_PAUSE );
        if( m_pStream[1] )
            m_pStream[1]->Pause( DSSTREAMPAUSE_PAUSE );
    }

    m_mmState = MM_PAUSED;

	
	////	DEBUG_LINE( _T("MM: Pause") );
 //   // Can only transition to paused from playing.
 //   if( m_mmOldState == MM_PLAYING &&
 //       m_mmState == MM_PLAYING )
 //   {
 //       m_pStream[0]->Pause( DSSTREAMPAUSE_PAUSE );
 //       m_pStream[1]->Pause( DSSTREAMPAUSE_PAUSE );
 //       m_mmState = MM_PAUSED;
 //       return S_OK;
 //   }

    return S_FALSE;
}



//-----------------------------------------------------------------------------
// Name: SetRandom
// Desc: Sets the playback mode for how to pick the next song.  If fRandom is
//       true, the next track is picked randomly, otherwise it's sequential.
//       If fGlobal is true, we'll move between soundtracks, otherwise we stay
//       within the current soundtrack
// Thread issues: The worker thread could be in the middle of picking which
//                track to play next.  The new setting may or may not be 
//                used, depending on exactly when the calls occurs.
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::SetRandom( BOOL fRandom )
{
//	DEBUG_LINE( _T("MM: Set Random") );
    m_fRandom = fRandom;

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: SetGlobal
// Desc: Sets the playback mode for how to pick the next song.  If fGlobal is 
//       true, we'll move between soundtracks, otherwise we stay  within the 
//       current soundtrack
// Thread issues: The worker thread could be in the middle of picking which
//                track to play next.  The new setting may or may not be 
//                used, depending on exactly when the calls occurs.
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::SetGlobal( BOOL fGlobal )
{
//	DEBUG_LINE( _T("MM: SetGlobal") );
    m_fGlobal = fGlobal;

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: SetVolume
// Desc: Sets the overall volume level for music playback.  Note that this
//       won't really work properly until we start using envelope release
// Thread issues: None - the worker thread doesn't set volume levels.
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::SetVolume( LONG lVolume )
{
//	DEBUG_LINE( _T("MM: SetVolume") );
    assert( m_pStream[0] && m_pStream[1] );

	m_lVolume = lVolume;
	if ( m_mmState != MM_STOPPED )
	{
		try
		{
			if( m_pStream[0] )
				m_pStream[0]->SetVolume( lVolume );
			if( m_pStream[1] )
				m_pStream[1]->SetVolume( lVolume );
			m_bVolumeNotSet = TRUE;
		}
		catch(...)
		{
			m_bVolumeNotSet = FALSE;
		}
	}
	else
	{
		m_bVolumeNotSet = TRUE;
	}
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: GetCurrentInfo
// Desc: Returns pointers to info.  Buffers should be appropriately sized, ie
//       MAX_SOUNDTRACK_NAME and MAX_SONG_NAME, respectively
// Thread issues: The worker thread could be in the middle of switching
//                between songs and/or soundtracks.  In this case, we could
//                return mixed information.  This is OK, since this is meant
//                to be called periodically to update state, so the results
//                will eventually be consistent.
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::GetCurrentInfo( WCHAR * szSoundtrack, WCHAR * szSong, DWORD * pdwLength )
{
//	DEBUG_LINE( _T("MM: GetCurrentInfo") );
    if( szSoundtrack && m_aSoundtracks && (m_uCurrentSoundtrack<m_uSoundtrackCount) )
	{
        m_aSoundtracks[ m_uCurrentSoundtrack ].GetSoundtrackName( szSoundtrack );
	}
    if( szSong )
	{
        wcscpy( szSong, m_szSong );
	}
    if( pdwLength )
	{
        *pdwLength = m_dwLength;
	}

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: GetPlaybackPosition
// Desc: Returns the current playback position, in seconds
//-----------------------------------------------------------------------------
FLOAT
CMusicManager::GetPlaybackPosition()
{
    return PACKET_TIME * m_dwPacketsCompleted;
}



//-----------------------------------------------------------------------------
// Name: NextSoundtrack
// Desc: Switches to the next soundtrack.  This is only safe to do if playback
//       is stopped, and we don't have a state change pending.  
// Thread issues: The worker thread could be trying to switch songs or 
//                soundtracks, so we can't make a call unless we know that
//                the worker thread is in MM_STOPPED and will stay there for
//                the duration of this call
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::NextSoundtrack()
{
//	DEBUG_LINE( _T("MM: NextSoundTrack") );
    if( m_mmOldState == MM_STOPPED &&
        m_mmState    == MM_STOPPED )
    {
		if ( m_fGlobal )
		{
			if ( m_uSoundtrackCount )
			{
				return SelectSoundtrack( ( m_uCurrentSoundtrack + 1 ) % m_uSoundtrackCount );
			}
		}
		else
		{
			m_uCurrentSoundtrack = 0;
			return SelectSoundtrack( ( 0 ) );
		}
    }

    return S_FALSE;
}



//-----------------------------------------------------------------------------
// Name: NextSong
// Desc: Switches to the next song in the current soundtrack.  This is only 
//       safe to do if playback is stopped, and we don't have a state change 
//       pending.
// Thread issues: The worker thread could be trying to switch songs or 
//                soundtracks, so we can't make a call unless we know that
//                the worker thread is in MM_STOPPED and will stay there for
//                the duration of this call
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::NextSong()
{
//	DEBUG_LINE( _T("MM: NextSong") );
    if( m_mmOldState == MM_STOPPED &&
        m_mmState    == MM_STOPPED )
    {
		if ( m_fGlobal == FALSE )
		{
			m_uCurrentSoundtrack = 0;
		}
		if ( m_fRandom )
		{
			DWORD dwNextSong = GetNextRandom();
			m_uCurrentSoundtrack = dwNextSong&0xFFFF0000;
			m_uCurrentSoundtrack >>= 16;
			m_dwSong = (dwNextSong&0x0FFFF);
			return SelectSong(m_dwSong);
		}
		else
		{
			if ( m_aSoundtracks && (m_uCurrentSoundtrack<m_uSoundtrackCount))
			{
				if ( m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() )
				{
					return SelectSong( ( m_dwSong + 1 ) % m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() );
				}
			}
		}
    }

    return S_FALSE;
}


int	CMusicManager::GetValidSoundtrack( int iTrack )
{
	int iReturn = -1;

	if ( m_fGlobal )
	{
		iTrack %= (int)m_uSoundtrackCount;

		iReturn = iTrack;

		if ( GetSongCount( iReturn ) == 0 )
		{
			iReturn++;
			iReturn %= (int)m_uSoundtrackCount;

			while( ( GetSongCount( iReturn ) == 0 ) && ( iReturn != iTrack ) )
			{
				iReturn++; 
				iReturn %= (int)m_uSoundtrackCount;
			}
			if ( GetSongCount( iReturn ) == 0 )
			{
				iReturn = -1;
			}
		}
	}
	else
	{
		if ( GetSongCount( 0 ) )
		{
			iReturn = 0;
		}
	}
	return iReturn;
}

BOOL CMusicManager::IsValidSong( int iTrack, int iSong )
{
	BOOL bReturn = FALSE;

	if ( ( iTrack >= 0 ) && ( iTrack < (int)m_uSoundtrackCount) )
	{
		if ( GetSongCount( iTrack ) > iSong )
		{
			bReturn = TRUE;
		}
	}
	return bReturn;
}

int	CMusicManager::GetValidSong( int iTrack, int iSong )
{
	int iReturn = -1;

	if ( ( iTrack >= 0 ) && ( iTrack < (int)m_uSoundtrackCount) )
	{
		if ( GetSongCount( iTrack ) )
		{
			iReturn = iSong % GetSongCount( iTrack );
		}
	}
	return iReturn;
}

long	CMusicManager::GetSongCount( int iSndTrack )
{
	long lReturn = 0;
	int iIndex;

	if ( m_aSoundtracks && m_uSoundtrackCount)
	{
		if ( iSndTrack < 0 )
		{
			for( iIndex=0; iIndex<(int)m_uSoundtrackCount; iIndex++ )
			{
				lReturn += (long)m_aSoundtracks[iIndex].GetSongCount();
			}
		}
		else
		{
			if ( iSndTrack < (int)m_uSoundtrackCount )
			{
				lReturn = (long)m_aSoundtracks[iSndTrack].GetSongCount();
			}
		}
	}
	return lReturn;
}

void CMusicManager::SetNextSong( void )
{
	if ( m_fRandom )
	{
		// This function does what this one does...
		DWORD dwNextSong = GetNextRandom();
		m_uCurrentSoundtrack = dwNextSong&0xFFFF0000;
		m_uCurrentSoundtrack >>= 16;
		m_dwSong = (dwNextSong&0x0FFFF);
		SelectTrackAndSong( m_uCurrentSoundtrack, m_dwSong );
	}
	else
	{
		int iSong, iSndTrack;
		iSndTrack = (int)m_uCurrentSoundtrack;
      
		iSong = GetValidSong(iSndTrack, (int)(m_dwSong+1));
		if ( iSong < 1 )
		{
			// If not global, this will always return 0 or -1
			iSndTrack = GetValidSoundtrack(iSndTrack+1);
			iSong = 0;
		}
		if ( iSndTrack >= 0 && iSong >= 0 )
		{
			SelectTrackAndSong( iSndTrack, iSong );  
		}
		else
		{
			// There are no valid songs.
			// Stop player
			Stop();
			SetEnable( FALSE );
		}
	}
}



void CMusicManager::SetFirstSong( void )
{
	int iSndTrack = GetValidSoundtrack(0);
	int iSong = 0;

	if ( m_bEnable )
	{
		if ( m_fRandom )
		{
			DWORD dwNextSong = GetNextRandom();
			m_uCurrentSoundtrack = dwNextSong&0xFFFF0000;
			m_uCurrentSoundtrack >>= 16;
			m_dwSong = (dwNextSong&0x0FFFF);
			SelectTrackAndSong( m_uCurrentSoundtrack, m_dwSong );
#if 0
			iSndTrack = GetValidSoundtrack( RandomNum() );
			if ( iSndTrack >= 0 )
			{
				iSong = GetValidSong( iSndTrack, RandomNum() );
				if (  ( m_uCurrentSoundtrack == (DWORD)iSndTrack ) &&
					  ( m_dwSong == (DWORD)iSong ) )
				{
					iSong = GetValidSong( iSndTrack, iSong+1 );
				}
			}
#endif
		}
		if ( iSndTrack >= 0 && iSong >= 0 )
		{
			SelectTrackAndSong( iSndTrack, iSong );  
		}
		else
		{
			// There are no valid songs.
			// Stop player
			Stop();
			SetEnable( FALSE );
		}
	}
}

//-----------------------------------------------------------------------------
// Name: RandomSong
// Desc: Switches to a random song, either in this soundtrack if fGlobal is
//       FALSE or globally random if fGlobal is TRUE.  This is only safe
//       to do if playback is stopped, and we don't have a state change 
//       pending.
// Thread issues: The worker thread could be trying to switch songs or 
//                soundtracks, so we can't make a call unless we know that
//                the worker thread is in MM_STOPPED and will stay there for
//                the duration of this call
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::RandomSong( BOOL fGlobal )
{
//	DEBUG_LINE( _T("MM: RandomSong") );
	HRESULT hr = S_FALSE;
    if( m_mmOldState == MM_STOPPED &&
        m_mmState    == MM_STOPPED )
    {
		DWORD dwNextSong = GetNextRandom();
		m_uCurrentSoundtrack = dwNextSong&0xFFFF0000;
		m_uCurrentSoundtrack >>= 16;
		m_dwSong = (dwNextSong&0x0FFFF);
		hr = SelectTrackAndSong( m_uCurrentSoundtrack, m_dwSong );
#if 0
        if( fGlobal )
        {
			if ( m_uSoundtrackCount )
			{
				SelectSoundtrack( RandomNum() % m_uSoundtrackCount );
			}
			if ( m_aSoundtracks && (m_uCurrentSoundtrack<m_uSoundtrackCount) )
			{
				if ( m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() )
				{
					hr = SelectSong( RandomNum() % m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() );
				}
			}
        }
		else
		{
			SelectSoundtrack( 0 );
			if ( m_aSoundtracks )
			{
				if ( m_aSoundtracks[ 0 ].GetSongCount() )
				{
					hr = SelectSong( RandomNum() % m_aSoundtracks[ 0 ].GetSongCount() );
				}
			}
		}
#endif
	}
	return S_FALSE;
}




//
// The following methods are not public and are meant only to be called 
// internally by the CMusicManager class.  Any tricky threading issues are
// pointed out as needed.
//



//-----------------------------------------------------------------------------
// Name: LoadSoundtracks
// Desc: Loads soundtrack info for user soundtracks stored on HD
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::LoadSoundtracks()
{
    HANDLE hSoundtrack;
    XSOUNDTRACK_DATA stData;
    UINT uAllocatedSoundtracks;

//	DEBUG_LINE( _T("MM: LoadSoundTracks") );
    m_aSoundtracks = new CSoundtrack[1]; // (CSoundtrack *)malloc( sizeof( CSoundtrack ) );

    if( !m_aSoundtracks )
        return E_OUTOFMEMORY;
    uAllocatedSoundtracks = 1;
	m_uSoundtrackCount = 0;

    // Set up our game soundtrack as soundtrack 0.
    // If we had more than 1 game soundtrack, we could
    // set them all up here
	m_aSoundtracks[0].Initialize( TRUE, 100 );
	g_sMusicFiles += _T("LT: Initialize Game Soundtrack\r\n");

	m_aSoundtracks[0].m_uSoundtrackIndex    = 0;
	wcscpy( m_aSoundtracks[0].m_szName, L"Game Soundtrack" );
	m_uSoundtrackCount++;

#if 1
    // Start scanning the soundtrack DB
    hSoundtrack = XFindFirstSoundtrack( &stData );
    if( INVALID_HANDLE_VALUE != hSoundtrack )
    {
        do
        {
            // Double our buffer if we need more space
            if( m_uSoundtrackCount + 1 > uAllocatedSoundtracks )
            {
                void * pNewAlloc = realloc( m_aSoundtracks, ( uAllocatedSoundtracks * 2 ) * sizeof( CSoundtrack ) );
                if( !pNewAlloc )
                {
                    // We couldn't expand our buffer, so clean up
                    // and bail out
                    free( m_aSoundtracks );
                    m_aSoundtracks = NULL;
                    XFindClose( hSoundtrack );
                    return E_OUTOFMEMORY;
                }
                m_aSoundtracks = (CSoundtrack *)pNewAlloc;
                uAllocatedSoundtracks *= 2;
            }

            // Ignore empty soundtracks
            if( stData.uSongCount > 0 )
            {
                // Copy the data over
                m_aSoundtracks[ m_uSoundtrackCount ].m_fGameSoundtrack = FALSE;
                m_aSoundtracks[ m_uSoundtrackCount ].m_uSoundtrackID   = stData.uSoundtrackId;
                m_aSoundtracks[ m_uSoundtrackCount ].m_uSongCount      = stData.uSongCount;
                wcscpy( m_aSoundtracks[ m_uSoundtrackCount ].m_szName, stData.szName );

                m_uSoundtrackCount++;
            }

        } while( XFindNextSoundtrack( hSoundtrack, &stData ) );

        XFindClose( hSoundtrack );
    }

    // Shrink our allocation down to what's actually needed, since
    // we can't add soundtracks on the fly.
    VOID* pRealloc = realloc( m_aSoundtracks, m_uSoundtrackCount * sizeof( CSoundtrack ) );
    if( !pRealloc )
    {
        free( m_aSoundtracks );
        m_aSoundtracks = NULL;
        return E_OUTOFMEMORY;
    }
    m_aSoundtracks = (CSoundtrack *)pRealloc;
#endif
	BuildRandomTracks();

//	return E_FAIL;
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: SelectSoundtrack
// Desc: Changes to the specified soundtrack
// NOTE: This should only be called by the worker thread OR
//       after making sure that playback is stopped with no state change
//       pending.
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::SelectSoundtrack( DWORD dwSoundtrack )
{
	DWORD dwOldSoundtrack;
	HRESULT hr = E_FAIL;
	int iMaxCount = this->m_uSoundtrackCount+1;


//	DEBUG_LINE( _T("MM: SelectSoundtrack") );
	if ( m_fGlobal )
	{
		if ( this->m_uSoundtrackCount )
		{
			if ( dwSoundtrack >= this->m_uSoundtrackCount )
			{
				dwSoundtrack = 0;
			}

			dwOldSoundtrack = dwSoundtrack;

			while (iMaxCount-- && (m_aSoundtracks[ dwSoundtrack ].GetSongCount() == 0))
			{
				dwSoundtrack++;
				if ( dwSoundtrack >= this->m_uSoundtrackCount )
				{
					dwSoundtrack = 0;
				}
				if (dwOldSoundtrack != dwSoundtrack)
				{
					break;
				}
			}

			if ( m_aSoundtracks[ dwSoundtrack ].GetSongCount() )
			{
				m_uCurrentSoundtrack = dwSoundtrack;
				SelectSong( 0 );
				hr  = S_OK;
			}
			else
			{
				// No songs.
				m_uCurrentSoundtrack = 0;
				hr = S_OK;
			}
		}
		else
		{
			m_uCurrentSoundtrack = 0;
		}
	}
	else
	{
		m_uCurrentSoundtrack = 0;
		hr = S_OK;
	}

    return hr;
}


HRESULT CMusicManager::SelectTrackAndSong( int iTrack, int iSong )
{
	HRESULT hr = E_FAIL;


	if ( ( iTrack >= 0 ) &&
		 ( iSong >= 0 ) &&
		 ( iTrack == GetValidSoundtrack(iTrack)) && 
		 ( iSong == GetValidSong( iTrack, iSong ) ) )
	{
		m_uCurrentSoundtrack = (DWORD)iTrack;
		m_dwSong = (DWORD)iSong;
		if ( m_aSoundtracks && (m_uCurrentSoundtrack<m_uSoundtrackCount))
		{
			ZeroMemory( m_szSong, sizeof(WCHAR)*MM_SONGNAME_SIZE );
			m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongInfo( m_dwSong,
																&m_dwSongID,
																&m_dwLength,
																m_szSong );

			CStdString sSong( m_szSong );

			DEBUG_FORMAT( _T("SelectTrackAndSong( %d, %d, %d, \"%s\" )"), m_dwSong, m_dwSongID, m_dwLength, sSong.c_str() );

		}
		hr = S_OK;
	}
	return hr;
}



//-----------------------------------------------------------------------------
// Name: SelectSong
// Desc: Switches to the specified song and caches song info
// NOTE: This should only be called by the worker thread OR
//       after making sure that playback is stopped with no state change
//       pending.
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::SelectSong( DWORD dwSong )
{
//	DEBUG_LINE( _T("MM: SelectSong") );
    m_dwSong = dwSong;
	if ( m_aSoundtracks && (m_uCurrentSoundtrack<m_uSoundtrackCount))
	{
		ZeroMemory( m_szSong, sizeof(WCHAR)*MM_SONGNAME_SIZE );
		m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongInfo( m_dwSong,
															&m_dwSongID,
															&m_dwLength,
															m_szSong );
		CStdString sSong( m_szSong );

		DEBUG_FORMAT( _T("SelectSong( %d, %d, %d, \"%s\" )"), m_dwSong, m_dwSongID, m_dwLength, sSong.c_str() );
		return S_OK;
	}
	return E_FAIL;
}



//-----------------------------------------------------------------------------
// Name: FindNextSong
// Desc: Simple helper function to switch to the next song, based on the
//       global repeat flag.
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::FindNextSong()
{
	DWORD dwCurrentSoundtrack = m_uCurrentSoundtrack;
	DWORD dwNewSong;

//	DEBUG_LINE( _T("MM: FindNextSong") );
    if( m_fRandom )
    {
        if( m_fGlobal )
        {
			if ( m_uSoundtrackCount )
			{
				SelectSoundtrack( RandomNum() % m_uSoundtrackCount );
			}
        }
		else
		{
			dwCurrentSoundtrack = 0;
			SelectSoundtrack( 0 );
		}
		if ( m_aSoundtracks && ( m_uCurrentSoundtrack<m_uSoundtrackCount ) )
		{
			if ( m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() )
			{
				dwNewSong = RandomNum() % m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount();
				if ( dwCurrentSoundtrack == m_uCurrentSoundtrack )
				{
					// Just go to next song if we select the same one again
					if ( dwNewSong == m_dwSong )
					{
						if( m_fGlobal && 
							m_dwSong == m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() - 1 )
						{
							if ( m_uSoundtrackCount )
							{
								SelectSoundtrack( ( m_uCurrentSoundtrack + 1 ) % m_uSoundtrackCount );
							}
						}
						else
						{
							if ( m_aSoundtracks && (m_uCurrentSoundtrack<m_uSoundtrackCount))
							{
								if ( m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() )
								{
									SelectSong( ( m_dwSong + 1 ) % m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() );
								}
							}
						}
					}
					else
					{
						SelectSong( dwNewSong );
					}
				}
				else
				{
					SelectSong( dwNewSong );
				}
			}
		}
    }
    else
    {
        if( m_fGlobal && 
            m_dwSong == m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() - 1 )
        {
			if ( m_uSoundtrackCount )
			{
	            SelectSoundtrack( ( m_uCurrentSoundtrack + 1 ) % m_uSoundtrackCount );
			}
        }
        else
		{
			if ( m_aSoundtracks && (m_uCurrentSoundtrack<m_uSoundtrackCount))
			{
				if ( m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() )
				{
					SelectSong( ( m_dwSong + 1 ) % m_aSoundtracks[ m_uCurrentSoundtrack ].GetSongCount() );
				}
			}
		}
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: Prepare
// Desc: Prepares to begin playback of the currently set track:
//       1) Clean up the old decoder, if we already had one
//       2) Create a new decoder on the soundtrack handle or our WMA file
//       3) Lower the volume on the output stream for the crossfade
//       4) Mark the start time of the track for rough timing
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::Prepare()
{
    // Open the song
    m_hDecodingFile = m_aSoundtracks[m_uCurrentSoundtrack].OpenSong( m_dwSongID );

    // Create the new decoder
    WMAXMODECODERPARAMETERS Params = {0};
    Params.hFile                    = m_hDecodingFile;
    Params.dwLookaheadBufferSize    = WMA_LOOKAHEAD;

    HRESULT hr = XWmaDecoderCreateMediaObject( (LPCWMAXMODECODERPARAMETERS)&Params, &m_pDecoder );
    if( FAILED( hr ) )
        return hr;

    return S_OK;
}
//{
//    WAVEFORMATEX wfx;
//
//    // Kill the old decoder
//    Cleanup();
//
//    // Open soundtrack file
//    m_hDecodingFile = m_aSoundtracks[ m_uCurrentSoundtrack ].OpenSong( m_dwSongID );
//    if( INVALID_HANDLE_VALUE == m_hDecodingFile )
//        return E_FAIL;
//
//    // Create the decoder
//    if( FAILED( WmaCreateDecoderEx( NULL, 
//                                    m_hDecodingFile, 
//                                    FALSE,              // Synchronous
//                                    WMA_LOOKAHEAD,      // 64k lookahead
//                                    PACKET_COUNT,       // # of packets
//                                    YIELD_RATE,         // Yield rate
//                                    &wfx,               // We know the format already
//                                    &m_pDecoder ) ) )   // New decoder
//        return E_FAIL;
//
//    // Remember when we started
//    m_dwPacketsCompleted = 0;
//
//    return S_OK;
//}


//-----------------------------------------------------------------------------
// Name: Cleanup
// Desc: Cleans up decoding resources
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::Cleanup()
{
    // Free up the decoder
    if( m_pDecoder )
    {
        m_pDecoder->Release();
        m_pDecoder = NULL;
    }

    // Close soundtrack file, if that's what we had
    if( INVALID_HANDLE_VALUE != m_hDecodingFile )
    {
        CloseHandle( m_hDecodingFile );
        m_hDecodingFile = INVALID_HANDLE_VALUE;
    }

    m_dwPacketsCompleted = 0;

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: Process
// Desc: Manages audio packets, filling them from the source XMO and 
//       dispatching them to the appropriate stream.
// Thread issues: This function manages state changes requested from the
//                main game thread.  The main game thread will write a new 
//                value for m_mmState, and then the worker thread will 
//                transition to that state as appropriate
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::Process()
{
    HRESULT hr;
    DWORD   dwPacket;

    // If we're currently playing, then process packets.  Note that
    // we could be playing, and not yet have a decoder if the worker
    // thread is still trying to open the file.
    if( m_pDecoder )
    {
		if( m_mmState == MM_PLAYING )
		{
			// Pump the decoder so it can handle its asynchronous file I/O
			m_pDecoder->DoWork();

			// Process packets
			while( FindFreePacket( &dwPacket, m_dwStream ) )
			{
				XMEDIAPACKET xmp;

				hr = ProcessSource( dwPacket, &xmp );
				if( FAILED( hr ) )
					return hr;

				// If we got a packet of size zero, that means that we either
				// 1) Hit the end of the current track, or 
				// 2) The decoder is not yet ready to produce output.  
				// Either way, we'll skip out, let the decoder do some work,
				// and try again next time.
				if( xmp.dwMaxSize > 0 )
				{
					hr = ProcessStream( dwPacket, &xmp );
					if( FAILED( hr ) )
						return hr;
				}
				else
					break;
			}
		}
    }

    return S_OK;
}

//{
//    HRESULT hr;
//    DWORD   dwPacket;
//    MM_STATE mmCurrentState = m_mmState;
//
//    // Check to see if our state has changed from a call
//    // from the main rendering loop, and if it has,
//    // update accordingly.
//    if( m_mmOldState != mmCurrentState )
//    {
//        // We can't transition from MM_STOPPED to
//        // anything but MM_PLAYING
//        assert( !( m_mmOldState == MM_STOPPED &&
//                   mmCurrentState != MM_PLAYING ) );
//
//        switch( m_mmOldState )
//        {
//        //
//        // Valid transitions from MM_STOPPED:
//        // -> MM_PLAYING = Prepare to play
//        //
//        case MM_STOPPED:
//            switch( mmCurrentState )
//            {
//            case MM_PLAYING:
//                // Make sure the streams are unpaused
//                m_pStream[0]->Pause( DSSTREAMPAUSE_RESUME );
//                m_pStream[1]->Pause( DSSTREAMPAUSE_RESUME );
//                Prepare();
//                break;
//            }
//            break;
//
//        //
//        // Valid transitions from MM_PAUSED:
//        // -> MM_PLAYING = Resume playback
//        // -> MM_STOPPED = Stop playback and clean up
//        //
//        case MM_PAUSED:
//            switch( mmCurrentState )
//            {
//            case MM_PLAYING:
//                m_pStream[0]->Pause( DSSTREAMPAUSE_RESUME );
//                m_pStream[1]->Pause( DSSTREAMPAUSE_RESUME );
//                break;
//            case MM_STOPPED:
//                m_pStream[0]->Flush();
//                m_pStream[1]->Flush();
//                Cleanup();
//                break;
//            }
//            break;
//
//        //
//        // Valid transitions from MM_PLAYING:
//        // -> MM_PAUSED = Pause streams (already done)
//        // -> MM_STOPPED = Stop playback and clean up
//        case MM_PLAYING:
//            switch( mmCurrentState )
//            {
//            case MM_STOPPED:
//                m_pStream[0]->Flush();
//                m_pStream[1]->Flush();
//                Cleanup();
//                break;
//            }
//            break;
//        }
//
//        // We're done with the transition.  At this point,
//        // we're really in the appropriate state, and the
//        // other thread can push a new state change through
//        m_mmOldState = mmCurrentState;
//    }
//
//
//    // If we're currently playing, then process packets
//    // and fades
//    if( mmCurrentState == MM_PLAYING )
//    {
//        // Process packets
//        while( FindFreePacket( &dwPacket, m_dwStream ) )
//        {
//            XMEDIAPACKET xmp;
//
//            hr = ProcessSource( dwPacket, &xmp );
//            if( FAILED( hr ) )
//                return hr;
//
//            // Make sure we got data from the decoder.  When we
//            // hit the end of a track, we may end up with an
//            // empty packet.  We'll just loop again and fill it
//            // up from the next track.
//            if( xmp.dwMaxSize )
//            {
//                hr = ProcessStream( dwPacket, &xmp );
//                if( FAILED( hr ) )
//                    return hr;
//            }
//        }
//    }
//
//
//    return S_OK;
//}



//-----------------------------------------------------------------------------
// Name: FindFreePacket
// Desc: Looks for a free audio packet.  Returns TRUE if one was found and
//       returns the index
//-----------------------------------------------------------------------------
BOOL CMusicManager::FindFreePacket( DWORD * pdwPacket, DWORD dwStream )
{
    for( DWORD dwIndex = dwStream; dwIndex < PACKET_COUNT; dwIndex += 2 )
    {
        // The first EXTRA_PACKETS * 2 packets are reserved - odd packets
        // for stream 1, even packets for stream 2.  This is to ensure
        // that there are packets available during the crossfade
        if( XMEDIAPACKET_STATUS_PENDING != m_adwPacketStatus[ dwIndex ] &&
            ( dwIndex > EXTRA_PACKETS * 2 || dwIndex % 2 == dwStream ) )
        {
            if( pdwPacket )
                *pdwPacket = dwIndex;
            return TRUE;
        }
    }

    return FALSE;
}


//-----------------------------------------------------------------------------
// Name: ProcessSource
// Desc: Fills audio packets from the decoder XMO
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::ProcessSource( DWORD dwPacket, XMEDIAPACKET * pxmp )
{
    HRESULT      hr;
    DWORD        dwBytesDecoded;

	if ( m_pDecoder )
	{
		ZeroMemory( pxmp, sizeof( XMEDIAPACKET ) );

		// See if the decoder is ready to provide output yet
		DWORD dwStatus;
		m_pDecoder->GetStatus( &dwStatus );
		if( ( dwStatus & XMO_STATUSF_ACCEPT_OUTPUT_DATA ) == 0 )
			return S_FALSE;

		// Set up the XMEDIAPACKET structure
		pxmp->pvBuffer          = m_pbSampleData + dwPacket * PACKET_SIZE;
		pxmp->dwMaxSize         = PACKET_SIZE;
		pxmp->pdwCompletedSize  = &dwBytesDecoded;

		hr = m_pDecoder->Process( NULL, pxmp );
		if( FAILED( hr ) )
			return hr;

		if( 0 == dwBytesDecoded )
		{
			// We hit the end of the current track.  Key the fade-out
			// Note that we queue an asynchronous flush to be done, and
			// then (in Cleanup), we release the stream.  The flush should
			// still occur on the next call to DirectSoundDoWork().
			m_pStream[m_dwStream]->Discontinuity();
			m_pStream[m_dwStream]->FlushEx( 0, DSSTREAMFLUSHEX_ASYNC | DSSTREAMFLUSHEX_ENVELOPE );

			// Clean up resources from this song, and get
			// ready for the next one:
			Cleanup();
			SetNextSong();
			m_dwStream = ( m_dwStream + 1 ) % 2;
			Prepare();
		}

		pxmp->dwMaxSize = dwBytesDecoded;
	}

    return S_OK;
}


//{
//    HRESULT      hr;
//    DWORD        dwBytesDecoded;
//
//    // Set up the XMEDIAPACKET structure
//    ZeroMemory( pxmp, sizeof( XMEDIAPACKET ) );
//    pxmp->pvBuffer          = m_pbSampleData + dwPacket * MMPACKET_SIZE;
//    pxmp->dwMaxSize         = MMPACKET_SIZE;
//    pxmp->pdwCompletedSize  = &dwBytesDecoded;
//
//    hr = m_pDecoder->Process( NULL, pxmp );
//    if( FAILED( hr ) )
//        return hr;
//
//    if( 0 == dwBytesDecoded )
//    {
//        // We hit the end of the current track.  Key the fade-out
//        m_pStream[ m_dwStream ]->Discontinuity();
//        m_pStream[ m_dwStream ]->FlushEx( 0, DSSTREAMFLUSHEX_ASYNC | DSSTREAMFLUSHEX_ENVELOPE );
//
//        // Clean up resources from this song, and get
//        // ready for the next one:
//        Cleanup();
//		SetNextSong();
//        // FindNextSong();
//        m_dwStream = ( m_dwStream + 1 ) % 2;
//        Prepare();
//
//    }
//    pxmp->dwMaxSize = dwBytesDecoded;
//
//    return S_OK;
//}

void	CMusicManager::SnagBuffer( XMEDIAPACKET * pxmp )
{
	DWORD dwGrabBufferSize = MMOUT_BUFFER_SIZE*sizeof(short int);

	if ( pxmp->dwMaxSize < dwGrabBufferSize )
	{
		dwGrabBufferSize = pxmp->dwMaxSize;
	}
	if ( dwGrabBufferSize )
	{
		memcpy( g_MMOutBuffer, pxmp->pvBuffer, dwGrabBufferSize );
	}
}


//-----------------------------------------------------------------------------
// Name: ProcessStream
// Desc: Submits audio packets to the appropriate stream
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::ProcessStream( DWORD dwPacket, XMEDIAPACKET * pxmp )
{
    HRESULT      hr;

    // The XMEDIAPACKET should already have been filled out
    // by the call to ProcessSource().  In addition, ProcessSource()
    // should have properly set the dwMaxSize member to reflect
    // how much data was decoded.
    pxmp->pdwStatus = &m_adwPacketStatus[ dwPacket ];
    pxmp->pdwCompletedSize = NULL;
    pxmp->pContext = (LPVOID)m_dwStream;

	SnagBuffer(pxmp);

    hr = m_pStream[ m_dwStream ]->Process( pxmp, NULL );
    if( FAILED( hr ) )
        return hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: MusicManagerDoWork
// Desc: Workhorse function for the CMusicManager class.  Either called by
//       the MusicManager's threadproc, or manually by a game worker thread
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::MusicManagerDoWork()
{
    Process();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: MusicManagerThreadProc
// Desc: Thread procedure on which the music manager decodes
//-----------------------------------------------------------------------------
DWORD WINAPI MusicManagerThreadProc( LPVOID lpParameter )
{
    CMusicManager * pThis = (CMusicManager *)lpParameter;
    DWORD dwResult;

	pThis->m_dwMMLoopCount = 0;
    //
    // Our thread procedure basically just loops indefinitely,
    // processing packets and switching between songs as 
    // appropriate, until we're signalled to shutdown.
    //
    do
    {
        pThis->MusicManagerDoWork();
        dwResult = WaitForSingleObject( pThis->m_hShutdownEvent, 1000 / 60 );
		pThis->m_dwMMLoopCount++;
    } while( dwResult == WAIT_TIMEOUT );

    // Since we created the decoder on this thread, we'll
    // release it on this thread.
    pThis->Cleanup();

    return 0;
}



//-----------------------------------------------------------------------------
// Name: StreamCallback
// Desc: Called back on completion of stream packets.  The stream context
//       contains a pointer to the CMusicManager object, and the packet
//       context contains the stream number
//-----------------------------------------------------------------------------
void CALLBACK 
StreamCallback( LPVOID pStreamContext, LPVOID pPacketContext, DWORD dwStatus )
{
    CMusicManager * pThis = (CMusicManager *)pStreamContext;

    if( DWORD(pPacketContext) == pThis->m_dwStream &&
        dwStatus == XMEDIAPACKET_STATUS_SUCCESS )
        ++pThis->m_dwPacketsCompleted;
}




#if _DEBUG
//-----------------------------------------------------------------------------
// Name: DebugVerify
// Desc: Debug routine to verify that everything is properly set up:
//       * Must have at least 1 game soundtrack, since a game can't depend
//         on there being user soundtracks on the Xbox hard drive
//       * Check that we can open all soundtrack songs
//-----------------------------------------------------------------------------
HRESULT
CMusicManager::DebugVerify()
{
#if 0
    // Make sure we have at least 1 game soundtrack
    if( !m_aSoundtracks[ 0 ].m_fGameSoundtrack )
    {
        OUTPUT_DEBUG_STRING( "Must have at least 1 game soundtrack.\n" );
        return E_FAIL;
    }

    // Verify we can open all soundtrack files.  This could take a while.
    for( UINT uSoundtrack = 0; uSoundtrack < m_uSoundtrackCount; uSoundtrack++ )
    {
        for( UINT uSong = 0; uSong < m_aSoundtracks[ uSoundtrack ].GetSongCount(); uSong++ )
        {
            DWORD dwSongID;
            DWORD dwSongLength;
            WCHAR szSongName[ MM_SONGNAME_SIZE ];

			ZeroMemory( szSongName, sizeof(WCHAR)*MM_SONGNAME_SIZE );
            m_aSoundtracks[ uSoundtrack ].GetSongInfo( uSong, &dwSongID, &dwSongLength, szSongName );
            HANDLE h = m_aSoundtracks[ uSoundtrack ].OpenSong( dwSongID );
            if( INVALID_HANDLE_VALUE == h )
            {
                OUTPUT_DEBUG_STRING( "Failed to open a soundtrack file.\n" );
                return E_FAIL;
            }
            CloseHandle( h );
        }
    }
#endif
    return S_OK;
}
#endif // _DEBUG

CSoundtrack * CMusicManager::GetGameSoundtrack( void )
{
	CSoundtrack * pReturn = m_aSoundtracks;

	return pReturn;
}



//
// CSoundtrack functions
//

CSoundtrack::CSoundtrack()
{
}

BOOL CSoundtrack::Initialize( BOOL bGameSoundtrack, int iMaxSongs )
{
	BOOL bReturn = FALSE;
	m_fGameSoundtrack = bGameSoundtrack;

	m_pSongs=NULL;
	m_uSongCount=0;
	m_uiAllocCount=0;

	if ( bGameSoundtrack )
	{
		m_uSongCount = 0;
		m_pSongs = new MM_SONG[iMaxSongs];
		if ( m_pSongs )
		{
			m_uiAllocCount=iMaxSongs;
			bReturn = TRUE;
		}
	}
	else
	{
		bReturn = TRUE;
	}
	return bReturn;
}


//-----------------------------------------------------------------------------
// Name: GetSongInfo
// Desc: Returns information about the given song
//-----------------------------------------------------------------------------
void
CSoundtrack::GetSongInfo( UINT uSongIndex, DWORD * pdwID, DWORD * pdwLength, WCHAR szName[MM_SONGNAME_SIZE] )
{

    if( m_fGameSoundtrack )
    {
		if ( uSongIndex < m_uSongCount )
		{
			CStdStringW swName(m_pSongs[uSongIndex].sName);
			if ( pdwID )
			{
				(DWORD)(*pdwID) = (DWORD)uSongIndex;
			}
			if ( pdwLength )
			{
				(DWORD)(*pdwLength) = (DWORD)m_pSongs[uSongIndex].dwLength;
			}
	        wcscpy( szName, swName.c_str() );
		}
		else
		{
			if ( pdwID )
			{
				(DWORD)(*pdwID) = (DWORD)0;
			}
			if ( pdwLength )
			{
				(DWORD)(*pdwLength) = (DWORD)0;
			}
			szName[0] = 0;
		}

//        *pdwLength = g_aGameSoundtrack[ uSongIndex ].dwLength;
//        wcscpy( szName, g_aGameSoundtrack[ uSongIndex ].szName.c_str() );
    }
    else
    {
        XGetSoundtrackSongInfo( m_uSoundtrackID, uSongIndex, pdwID, pdwLength, szName, MAX_SONG_NAME );
    }
}


BOOL CSoundtrack::AddSong( LPCTSTR szName, LPCTSTR szFilename, DWORD dwLength, BOOL bCheckFile )
{
	BOOL bReturn = FALSE;

	if ( m_fGameSoundtrack )
	{
		if ( !m_pSongs )
		{
			g_sMusicFiles += _T("AS: Initialize Game Soundtrack\r\n");
			Initialize( TRUE, 100 );
		}
		if ( m_pSongs && (m_uSongCount<m_uiAllocCount) )
		{
			if ( bCheckFile )
			{
				if ( szFilename )
				{
					if ( FileExists( szFilename ) )
					{
						// Succeeded
						dwLength = FileLength( szFilename );
						if ( dwLength )
						{
							g_sMusicFiles += szName;
							g_sMusicFiles += _T("\r\n");

							m_pSongs[m_uSongCount].sName = szName;
							m_pSongs[m_uSongCount].sFilename = szFilename;
							m_pSongs[m_uSongCount].dwLength = dwLength;
							m_uSongCount++;
							bReturn = TRUE;
						}
					}
				}
			}
			else
			{
				g_sMusicFiles += szName;
				g_sMusicFiles += _T("\r\n");

				m_pSongs[m_uSongCount].sName = szName;
				m_pSongs[m_uSongCount].sFilename = szFilename;
				m_pSongs[m_uSongCount].dwLength = dwLength;
				m_uSongCount++;
				bReturn = TRUE;
			}
		}
		else
		{
			CStdString sTemp;
			sTemp.Format( _T("AS: Problem (%d<%d)\r\n"),m_uSongCount,m_uiAllocCount);
			g_sMusicFiles += sTemp;
		}
	}
	else
	{
		g_sMusicFiles += _T("AS: NOT Game Soundtrack\r\n");
	}
	return bReturn;
}

//-----------------------------------------------------------------------------
// Name: OpenSong
// Desc: Opens the song with the given ID and returns a handle to the file
//-----------------------------------------------------------------------------
HANDLE
CSoundtrack::OpenSong( DWORD dwSongID )
{
	HANDLE hReturn = NULL;

    if( m_fGameSoundtrack )
	{
		if ( dwSongID < m_uSongCount )
		{
	        hReturn = CreateFile( m_pSongs[dwSongID].sFilename.c_str(), 
                           GENERIC_READ, 
                           FILE_SHARE_READ, 
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL );
		}
	}
    else
	{
        hReturn = XOpenSoundtrackSong( dwSongID, FALSE );
	}
	return hReturn;
}



// Takes directory in format: D:\MUSIC\ **
int	CSoundtrack::LoadSongs( LPCTSTR szDirectory )
{
	int iCount = 0;
	HANDLE hFind;
	int iIndex = 0;
	WIN32_FIND_DATA ffData;
	CStdString sWildcard;
	CStdString sTemp;

	DEBUG_LINE( _T("Loading from directory:"));
	DEBUG_LINE( szDirectory );
	if ( m_fGameSoundtrack )
	{
		sWildcard.Format( _T("%s*.wma"), szDirectory );
		hFind = FindFirstFile( sWildcard.c_str(), &ffData );
		
		if( INVALID_HANDLE_VALUE != hFind )
		{
			do
			{
				sTemp.Format( _T("%s%s"), szDirectory, ffData.cFileName );
				DEBUG_LINE( sTemp.c_str()  );
				if ( AddSong( ffData.cFileName, sTemp.c_str(), ffData.nFileSizeLow, FALSE ) )
				{
					iCount++;
				}
				iIndex++;
			} while( FindNextFile( hFind, &ffData ) );

			// Close the find handle.
			FindClose( hFind );
		}
	}
	return iCount;
}



