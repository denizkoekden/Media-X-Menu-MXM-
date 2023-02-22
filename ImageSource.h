/*********************************************************************************\
 * ImageSource.h
 * Class used to abstract the loading of textures.
 * Support for additional formats (video and Flash, for example) will be added 
 * here
 * (C) 2002 MasterMind
\*********************************************************************************/


#ifndef __IMAGESOURCE_H__
#define __IMAGESOURCE_H__

#include "wmvplayer.h"
#include "XBResource_mxm.h"

#define XNOCACHE


DWORD ColorRGBA( int iAlpha, int iRed, int iGreen, int iBlue );

// Manage movies and picture sources	
class CImageSource
{
	int					m_iWidth;
	int					m_iHeight;
	int					m_iTextWidth;
	int					m_iTextHeight;
	bool				m_bVideo;
	bool				m_bVideoXMV;
	bool				m_bPackedResource;
	LPDIRECT3DTEXTURE8	m_pTexture;
	LPDIRECT3DTEXTURE8	m_pVideoTexture;
	LPDIRECT3DTEXTURE8	m_pLinTexture;
	LPDIRECT3DDEVICE8	m_pd3dDevice;
	LPDIRECTSOUND8		m_pDSound;
	CWMVPlayer		*	m_pWMVPlayer;
	// m_pXMVPlayer
	WMVVIDEOINFO		m_videoInfo;
    RECT				m_SrcRect;
    RECT				m_DestRect;
	LPDIRECT3DSURFACE8	m_pVideoSurface;
	DWORD				m_dwAlphaColor;
	bool				m_bVideoLoop;
	bool				m_bVideoSound;
	D3DXIMAGE_INFO		m_info;

	CXBPackedResource * m_pPackedResource;

	bool				m_bCache;
	CStdString			m_sFilename;

//	bool				m_bLoopingEnded;

	void				LoadFromCache( void );
	HRESULT LoadTextureFromImageFile( LPCTSTR szFileName  );
//	HRESULT BuildRawTexture( int &iWidth, int &iHeight, D3DFORMAT fmt, LPDIRECT3DTEXTURE8 *ppTexture );
public:
	static	bool		m_bAllowVideo;
	bool	HasAudio( void ) { return (m_bVideo|m_bVideoXMV); };
	bool	IsVideoTexture( void ) { return (m_bVideo|m_bVideoXMV); };
	bool	IsPackedResource( void ) { return (m_bPackedResource&&m_pPackedResource); };
	HRESULT	Attach( LPDIRECT3DTEXTURE8 pTexture, D3DXIMAGE_INFO * pInfo = NULL );

	bool	IsLoopingEnded( void );
	void	SetLoopControl( int iLoopControl );
	int		GetLoopControl( void );
	int		GetLoopCount( void );

	void	SetVideoLoop( bool bLoop = true );
	void	SetVideoSound( bool bSound = true );
	bool	GetVideoLoop( void ) { return m_bVideoLoop; };
	bool	GetVideoSound( void ) { return m_bVideoSound; };

	void	GetCorrectedTC( FLOAT & fU, FLOAT & fV );

	bool	DuplicateTexture( LPDIRECT3DTEXTURE8 pTexture, bool bClear = true );
	bool	CopySurface( LPDIRECT3DTEXTURE8 pTexture );

	void	ReSync( void );

	void ReleaseResources( void );
	int	GetWidth( void );
	int	GetHeight( void );
	CImageSource( LPDIRECT3DDEVICE8 pd3dDevice = NULL, LPDIRECTSOUND8 pDSound = NULL, LPCTSTR szFileName = NULL, DWORD dwAlphaColor = 0xff000000 );
	~CImageSource();
	HRESULT Initialize( LPDIRECT3DDEVICE8 pd3dDevice, LPDIRECTSOUND8 pDSound, LPCTSTR szFileName = NULL, DWORD dwAlphaColor = 0xff000000 );
	HRESULT LoadImage( LPCTSTR szFileName, DWORD dwAlphaColor = 0xff000000 );
	HRESULT LoadImageMem( const BYTE *pbBuffer, DWORD dwLength, LPSTR szSection = NULL, DWORD dwAlphaColor = 0xff000000 );
	HRESULT	MakeBlank( int iWidth, int iHeight, D3DFORMAT fmt = D3DFMT_A8R8G8B8 );
	LPDIRECT3DTEXTURE8 GetTexture( void );
};








#endif