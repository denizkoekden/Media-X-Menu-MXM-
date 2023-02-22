//-----------------------------------------------------------------------------
// File: WMVPlayer.h
//
// Desc: Definition of WMV playback helper class
//
// Hist: 10.10.01 - New for November XDK
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef __WMVPLAYER_H__
#define __WMVPLAYER_H__

#include <xtl.h>
#include "wmvxmo.h"
//#include <xmv.h>



// This structure is used to hold data about a video frame
typedef struct {
    LPDIRECT3DTEXTURE8  pTexture;
    REFERENCE_TIME      rtTimestamp;
    BYTE*               pBits;
} VIDEOFRAME;

// You'll need to adjust NUM_PACKETS based off your video content
// to ensure that you never run out during the course of playback
// There is an assert that will fire if the class runs out of 
// packets
// Packet Size needs to be large enough to hold at least 2048
// decoded samples
#define NUM_PACKETS 256
#define PACKET_SIZE 2048 * 2 * 2


class CWMVPlayer
{
public:
    CWMVPlayer();
    ~CWMVPlayer();

    HRESULT Initialize( LPDIRECT3DDEVICE8 pd3dDevice, 
                        LPDIRECTSOUND8 pDSound );       // Initialize the player
    HRESULT OpenFile( CHAR* strFilename );              // Open a file for playback
    HRESULT CloseFile();                                // Close file
    HRESULT GetVideoInfo( WMVVIDEOINFO* pVideoInfo );   // Get video info

    BOOL    IsReady();                                  // Ready to switch?
    HRESULT GetTexture( LPDIRECT3DTEXTURE8 pTexture );  // Copy to texture
    HRESULT DecodeNext();                               // Decode next frame

	void	SetVideoSound( bool bSound = true ) { m_bVideoSound = bSound; };
	bool	GetVideoSound( void ) { return m_bVideoSound; };

	//void	SetVideoLoop( bool bLoop = true ) { m_bVideoLoop = bLoop; };
	//bool	GetVideoLoop( void ) { return m_bVideoLoop; };
	void	SetVideoLoop( bool bLoop = true ) { m_iLoopControl = bLoop?-1:m_iLoopControl+1; };
	bool	GetVideoLoop( void ) { return (m_iLoopControl==(-1)); };
	bool	IsDoneLooping( void );

	void	ReSync(void);
	void	SetLoopControl( int iLoopControl ) { m_iLoopControl = iLoopControl; };
	int		GetLoopControl( void ) { return m_iLoopControl; };
	int		GetLoopCount( void ) { return m_iLoopCount; };

private:
	int		m_iLoopCount;
	int		m_iLoopControl;

    BOOL FindFreePacket( DWORD * pdwIndex );            // Find a free audio packet


//	bool				m_bVideoLoop;
	bool				m_bVideoSound;
	bool				m_bHasAudio;

    LPDIRECT3DDEVICE8   m_pd3dDevice;                   // Direct3D device
    LPWMVDECODER        m_pWMVDecoder;                  // WMV Decoder XMO
//	XMVDecoder			*m_pXMVDecoder;
    WMVVIDEOINFO        m_wmvVideoInfo;                 // Video format info
    WAVEFORMATEX        m_wfx;                          // Audio format info
    REFERENCE_TIME      m_rtDuration;                   // Video duration

    REFERENCE_TIME      m_rtStartTime;                  // Start time
    BOOL                m_bFirstFrame;                  // BOOL to track first frame

    VIDEOFRAME          m_VideoFrame;                   // Video frame

    LPDIRECTSOUND8      m_pDSound;                      // DirectSound object
    LPDIRECTSOUNDSTREAM m_pStream;                      // Stream for playback
    DWORD               m_adwStatus[NUM_PACKETS];       // Packet status
    BYTE*               m_pbSampleData;                 // Audio buffer
};


#endif
