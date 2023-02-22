/*********************************************************************************\
 * ImageSource.cpp
 * Class used to abstract the loading of textures.
 * Support for additional formats (video and Flash, for example) will be added 
 * here
 * (C) 2002 MasterMind
\*********************************************************************************/

#include <XTL.h>
#include <WinNT.h>
#include <XBUtil.h>
#include <XGraphics.h>
#include "StdString.h"
#include "ImageSource.h"
#include "utils.h"
#include "XBEasel.h"
#include "MXM.h"
#include "CommDebug.h"
#include "ImgFileCache.h"
#include "XBStateApp.h"

#pragma pack(push, 1)
struct _TYUYV
{
	unsigned char V;
	unsigned char Y2;
	unsigned char U;
	unsigned char Y;
};
#pragma pack (pop)

/*

XBX is a packed resource file with this entry:

Texture MyTex
{
   Source title.bmp
   Format D3DFMT_DXT1
   Width 128
   Height 128
}


*/


#define Clip(x,min,max) (x>max?max:(x<min?min:x))


DWORD YUV2ARGB( DWORD dwYUYV, bool bY2 )
{
    DWORD dwReturn;
	struct _TYUYV *pYUV = (struct _TYUYV *)&dwYUYV;
    long C, D, E;
    long R, G, B;

    C = ((int)(bY2?pYUV->Y2:pYUV->Y))-16;
    D = ((int)(pYUV->U))-128;
    E = ((int)(pYUV->V))-128;

    R = ((298*C)+(409*E)+128)>>8;
//	R = Clip(R,0,255);
    G = ((298*C)-(100*D)-(208*E)+128)>>8;
//	G = Clip(G,0,255);
    B = ((298*C)+(516*D)+128)>>8;
//	B = Clip(B,0,255);
    dwReturn = ColorRGBA(0xff,(int)R,(int)G,(int)B);
    return dwReturn;
}


void ConvertYUV2toARGB( LPDIRECT3DTEXTURE8 pYUVTexture, LPDIRECT3DTEXTURE8 pARGBTexture, LPDIRECT3DTEXTURE8 pARGBSwizTexture)
{
	LPDIRECT3DSURFACE8 pYUVSurface;
	LPDIRECT3DSURFACE8 pARGBSurface;
	LPDIRECT3DSURFACE8 pARGBSwizSurface;
	D3DSURFACE_DESC descYUV;
	D3DSURFACE_DESC descARGB;
	D3DSURFACE_DESC descARGBSwiz;
	D3DLOCKED_RECT rectYUV;
	D3DLOCKED_RECT rectARGB;
	D3DLOCKED_RECT rectSwizARGB;
	DWORD * pdwYUV;
	DWORD * pdwARGB;

	int iRows, iX;

	if ( pYUVTexture && pARGBTexture )
	{
		pYUVTexture->GetLevelDesc( 0, &descYUV );
		pARGBTexture->GetLevelDesc( 0, &descARGB );
		pARGBSwizTexture->GetLevelDesc( 0, &descARGBSwiz );

		if ( ( descYUV.Format == D3DFMT_YUY2 ) && (descARGB.Format == D3DFMT_LIN_A8R8G8B8 ))
		{

			pYUVTexture->GetSurfaceLevel( 0, &pYUVSurface );
			pARGBTexture->GetSurfaceLevel( 0, &pARGBSurface );
			pARGBSwizTexture->GetSurfaceLevel( 0, &pARGBSwizSurface );
			pYUVSurface->LockRect( &rectYUV, NULL, D3DLOCK_READONLY );
			pARGBSurface->LockRect( &rectARGB, NULL, 0 );
			pARGBSwizSurface->LockRect( &rectSwizARGB, NULL, 0 );

			// Ready to read one and output the other...
			for ( iRows=0; (iRows<(int)descYUV.Height)&&(iRows<(int)descARGB.Height); iRows++ )
			{
				// YUV is 32 bits for two pixels
				pdwYUV = &((DWORD*)rectYUV.pBits)[(descYUV.Width/2)*iRows];
				pdwARGB = &((DWORD*)rectARGB.pBits)[descARGB.Width*iRows];
				for( iX=0; (iX<(int)descYUV.Width)&&(iX<(int)descARGB.Width); iX+=2 )
				{
//					*pdwARGB = ColorRGBA(0xff,iRows,256-iRows,0); 
					*pdwARGB = YUV2ARGB( *pdwYUV, false );
					pdwARGB++;
//					*pdwARGB = ColorRGBA(0xff,iRows,256-iRows,0); 
					*pdwARGB = YUV2ARGB( *pdwYUV, true );
				}
#if 0
				pdwARGB = &((DWORD*)rectARGB.pBits)[descARGB.Width*iRows];
				for( iX=0; (iX<descARGB.Width); iX++, pdwARGB++ )
				{
					if ( iRows == 3 )
					{
						*pdwARGB = ColorRGBA( 0xff, 0xff, 0xff, 0xff );
					}
					else if (iRows = 50 )
					{
						*pdwARGB = ColorRGBA( 0xff, 0xff, 0xff, 0xff );
					}
					else
					{
						if ( iX == 10 || iX == 100 )
						{
							*pdwARGB = ColorRGBA( 0xff, 0xff, 0xff, 0xff );
						}
						else
						{
							*pdwARGB = ColorRGBA( 0xff, 0xff, 0, 0 );
						}
					}
				}
#endif
			}


			// Now take the LinTexture and Swizzle it to Texture
			{
				RECT rect;
				POINT pt;

				pt.x = 0;
				pt.y = 0;
				rect.left = 0;
				rect.top = 0;
				rect.right = descARGB.Width;
				rect.bottom = descARGB.Height;

				XGSwizzleRect(
					 rectARGB.pBits,
						rectARGB.Pitch,
						NULL,
					    rectSwizARGB.pBits,
						descARGBSwiz.Width,
						descARGBSwiz.Height,
						NULL,
						4 );

			}

			pARGBSwizSurface->UnlockRect();
			pARGBSurface->UnlockRect();
			pYUVSurface->UnlockRect();
			SAFE_RELEASE(pYUVSurface);
			SAFE_RELEASE(pARGBSurface);
			SAFE_RELEASE(pARGBSwizSurface);
		}
	}

}


/*

YUV2RGB_PROC	MACRO	procname,uyvy=0,rgb32=1

	PUBLIC	C _&procname

;;void __cdecl procname(
;;	[esp+ 4] const unsigned char* src,
;;	[esp+ 8] unsigned char* dst,
;;	[esp+12] const unsigned char* src_end,
;;	[esp+16] int stride);

_&procname	PROC

	push	esi
	push	edi

	mov	eax,[esp+16+8]
	mov	esi,[esp+12+8]		; read source bottom-up
	mov	edi,[esp+8+8]
	mov	edx,offset yuv2rgb_constants

loop0:
	lea	ecx,[esi-8]
	sub	esi,eax

	align 32
loop1:
	YUV2RGB_INNER_LOOP	uyvy,rgb32,0
	jb	loop1

	YUV2RGB_INNER_LOOP	uyvy,rgb32,1

	sub	esi,eax
	cmp	esi,[esp+4+8]
	ja	loop0

	emms
	pop	edi
	pop	esi
	retn

_&procname	ENDP

	ENDM



  */


DWORD ColorRGBA( int iAlpha, int iRed, int iGreen, int iBlue )
{
	DWORD dwResult = 0;
	if( iAlpha > 255 )
	{
		iAlpha = 255;
	}
	if( iRed > 255 )
	{
		iRed = 255;
	}
	if( iGreen > 255 )
	{
		iGreen = 255;
	}
	if( iBlue > 255 )
	{
		iBlue = 255;
	}
	if( iAlpha < 0 )
	{
		iAlpha = 0;
	}
	if( iRed < 0 )
	{
		iRed = 0;
	}
	if( iGreen < 0 )
	{
		iGreen = 0;
	}
	if( iBlue < 0 )
	{
		iBlue = 0;
	}
	dwResult = iAlpha&0xff;
	dwResult<<=8;
	dwResult |= iRed&0xff;
	dwResult<<=8;
	dwResult |= iGreen&0xff;
	dwResult<<=8;
	dwResult |= iBlue&0xff;
	return dwResult;
}

bool CImageSource::m_bAllowVideo = true;

CImageSource::CImageSource( LPDIRECT3DDEVICE8 pd3dDevice, LPDIRECTSOUND8 pDSound, LPCTSTR szFileName, DWORD dwAlphaColor )
{
//	m_bLoopingEnded = false;
	m_bPackedResource = false;
	m_pPackedResource = false;
	m_bCache = false;
	m_bVideoXMV = false;
	m_bVideoLoop = true;
	m_bVideoSound = false;
	m_iWidth = 0;
	m_iHeight = 0;
	m_iTextWidth = m_iWidth;
	m_iTextHeight = m_iHeight;
	m_bVideo = false;
	m_pWMVPlayer = NULL;
	m_pTexture = NULL;
	m_pVideoTexture = NULL;
	m_pLinTexture = NULL;
	Initialize( pd3dDevice, pDSound, szFileName, dwAlphaColor );
}

void CImageSource::GetCorrectedTC( FLOAT & fU, FLOAT & fV )
{
	D3DSURFACE_DESC descSurface;

	fU = 1.0f;
	fV = 1.0f;

	if ( m_pTexture )
	{
		// Now make pTexture our texture.
		// First we need to get the params
		if ( SUCCEEDED( m_pTexture->GetLevelDesc( 0, &descSurface ) ) )
		{
			fU = (FLOAT)m_iWidth;
			fU /= (FLOAT)descSurface.Width;
			fV = (FLOAT)m_iHeight;
			fV /= (FLOAT)descSurface.Height;
		}
	}
}

void CImageSource::SetLoopControl( int iLoopControl )
{
	if ( m_bVideo && m_pWMVPlayer )
	{
		m_pWMVPlayer->SetLoopControl( iLoopControl );
	}
}

int CImageSource::GetLoopControl( void )
{
	int iReturn = -1;

	if ( m_bVideo && m_pWMVPlayer )
	{
		iReturn = m_pWMVPlayer->GetLoopControl();
	}
	return iReturn;
}

int CImageSource::GetLoopCount( void )
{
	int iReturn = 0;

	if ( m_bVideo && m_pWMVPlayer )
	{
		iReturn = m_pWMVPlayer->GetLoopCount();
	}
	return iReturn;
}

bool CImageSource::IsLoopingEnded( void )
{
	bool bReturn = false;

	if ( m_bVideo && m_pWMVPlayer )
	{
		bReturn = m_pWMVPlayer->IsDoneLooping();
	}
	return bReturn;
}

HRESULT	CImageSource::Attach( LPDIRECT3DTEXTURE8 pTexture, D3DXIMAGE_INFO * pInfo /* = NULL */ )
{
	HRESULT hr = E_FAIL;
	
	ReleaseResources();
	if ( pTexture )
	{
		D3DSURFACE_DESC descSurface;

		// Now make pTexture our texture.
		// First we need to get the params
		if ( SUCCEEDED( pTexture->GetLevelDesc( 0, &descSurface ) ) )
		{
			// Free up everything here...
			ReleaseResources();

			// Copy the pointer and add the reference
			m_pTexture = pTexture;
			m_pTexture->AddRef();

			// Fill the dimentions and just make sure 
			// everything else is set properly
			if ( pInfo )
			{
				m_iTextWidth = descSurface.Width;
				m_iTextHeight = descSurface.Height;
				m_iWidth = pInfo->Width;
				m_iHeight = pInfo->Height;
				memcpy( &m_info, pInfo, sizeof(D3DXIMAGE_INFO) );
			}
			else
			{
				m_iTextWidth = descSurface.Width;
				m_iTextHeight = descSurface.Height;
				m_iWidth = descSurface.Width;
				m_iHeight = descSurface.Height;
			}

			m_bVideo = false;
			m_bVideoXMV = false;
			m_pVideoTexture = NULL;
			m_pLinTexture = NULL;
			m_pWMVPlayer = NULL;
			m_pVideoSurface = NULL;
			m_dwAlphaColor = 0;
			m_bVideoLoop = false;
			m_bVideoSound = false;
		}
	}
	return hr;
}


void CImageSource::ReleaseResources( void )
{
	if ( m_bVideo && m_pWMVPlayer)
	{
		m_pWMVPlayer->CloseFile();
		delete m_pWMVPlayer;
		m_pWMVPlayer = NULL;
	}
	if ( m_pTexture )
	{
		if ( m_bCache )
		{
			g_FileManager.ReleaseTexture( m_sFilename );
		}
		else
		{
			m_pTexture->Release();
		}
		m_pTexture = NULL;
	}
	if ( m_pVideoTexture )
	{
		m_pVideoTexture->Release();
		m_pVideoTexture= NULL;
	}
	SAFE_RELEASE(m_pLinTexture);
	if ( m_bPackedResource )
	{
		if ( m_pPackedResource )
		{
			delete m_pPackedResource;
		}
	}
	m_bPackedResource = false;
	m_pPackedResource = NULL;
	m_iWidth = 0;
	m_iHeight = 0;
	m_iTextWidth = 0;
	m_iTextHeight = 0;
	m_bVideo = false;
	m_bCache = false;
	m_dwAlphaColor = 0;
	
}

CImageSource::~CImageSource()
{
	ReleaseResources();
}

HRESULT CImageSource::Initialize( LPDIRECT3DDEVICE8 pd3dDevice, LPDIRECTSOUND8 pDSound, LPCTSTR szFileName, DWORD dwAlphaColor )
{
	HRESULT hr = S_OK;

	m_pd3dDevice = pd3dDevice;
	m_pDSound = pDSound;
	m_bVideo = false;
	m_dwAlphaColor = dwAlphaColor;
	if ( szFileName && szFileName[0] )
	{
		hr = LoadImage( szFileName, dwAlphaColor );
	}
	return hr;
}


HRESULT CImageSource::LoadImageMem( const BYTE *pbBuffer, DWORD dwLength, LPTSTR szSection, DWORD dwAlphaColor )
{
	HRESULT hr = E_FAIL;
	PVOID pLoc = NULL;

	ReleaseResources();
	if ( szSection )
	{
		pLoc = XLoadSection(szSection);
	}
	if ( pLoc || (szSection == NULL))
	{
		m_dwAlphaColor = dwAlphaColor;
		if ( m_bVideo )
		{
			// Scope the next series
			{
				// Now draw it on to our "real" texture
	//			CXBEasel easel( m_pd3dDevice, m_pTexture );

				// Update the overlay surface to use this texture
	//			m_pd3dDevice->EnableOverlay( FALSE );

			}
			m_pWMVPlayer->CloseFile();
			delete m_pWMVPlayer;
			m_pWMVPlayer = NULL;
		}
		m_bVideo = false;

		if ( pbBuffer && dwLength )
		{
			if ( m_pd3dDevice )
			{

				if ( m_pTexture )
				{
					m_pTexture->Release();
					m_pTexture = NULL;
				}
				if ( m_pVideoTexture )
				{
					m_pVideoTexture->Release();
					m_pVideoTexture= NULL;
				}
				hr = D3DXCreateTextureFromFileInMemoryEx( 
						m_pd3dDevice, 
						pbBuffer, dwLength,
						// "D:\\title.bmp", 
						D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
						0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
						m_dwAlphaColor, &m_info, NULL,
						(LPDIRECT3DTEXTURE8*)&m_pTexture
						); // D3DFMT_UNKNOWN
				if ( SUCCEEDED( hr ) )
				{
					DEBUG_LINE(_T("Loaded Image OK"));
				}
				else
				{
					DEBUG_LINE(_T("Load Image FAILED"));
				}
				D3DSURFACE_DESC descLevel;

				m_pTexture->GetLevelDesc(0, &descLevel );
				m_iTextWidth = descLevel.Width;
				m_iTextHeight = descLevel.Height;
				m_iWidth = m_info.Width;
				m_iHeight = m_info.Height;
			}
		}
		if ( szSection )
		{
			XFreeSection(szSection);
		}
	}
	else
	{
		CStdString sTemp;

		sTemp.Format(_T("CSourceImage::LoadImageMem: FAILED SECTION LOAD (%s)"), szSection );
		DEBUG_LINE(sTemp);
	}
	return hr;
}


HRESULT CImageSource::LoadImage( LPCTSTR szFileName, DWORD dwAlphaColor )
{
	HRESULT hr = E_FAIL;
	CStdString sFileName, sCheckName(szFileName);

	ReleaseResources();
	// Determine if file format is WMV or XMV
	sFileName = szFileName;
	sFileName.MakeLower();
	m_dwAlphaColor = dwAlphaColor;

	if (sFileName.Find( ".xbe" ) > 0 )
	{
		if ( FileExists( szFileName ) )
		{
			// Locate file ID and get TitleImage.xbx E:\UDATA\<ID>\TitleImage.xbx
			sCheckName.Format( _T("E:\\UDATA\\%08x\\TitleImage.xbx"), GetXbeID( szFileName) );
			sFileName = sCheckName;
			sFileName.MakeLower();
		}
	}

	if ( FileExists( sCheckName ) )
	{
		m_bPackedResource = false;
		m_bVideoXMV = false;
		m_bVideo = false;
		if (sFileName.Find( ".wmv" ) > 0 )
		{
			m_bVideo = true;
		}
		else if (sFileName.Find( ".xmv" ) > 0)
		{
			m_bVideoXMV = true;
			m_bVideo = true;
		}
		else if (sFileName.Find( ".xpr" ) > 0)
		{
			m_bPackedResource = true;
		}
		else if (sFileName.Find( ".xbx" ) > 0)
		{
			m_bPackedResource = true;
		}

		// Cancel out if we don't support the video!!
		if ( !m_bAllowVideo && m_bVideo )
		{
			m_bVideo = false;
			sFileName = _T("");
		}

		if ( sFileName.GetLength() )
		{
			if ( m_pd3dDevice == NULL )
			{
				m_pd3dDevice = GetStateApp()->Get3dDevice();
			}
			if ( m_pd3dDevice )
			{
#ifdef NOCACHE
				if ( m_bPackedResource )
				{
					m_pPackedResource = new CXBPackedResource;
					if ( m_pPackedResource )
					{
						if( SUCCEEDED( m_pPackedResource->Create( m_pd3dDevice, sFileName, 1, NULL ) ) )
						{
							LPDIRECT3DTEXTURE8 pTexture;
							D3DSURFACE_DESC descSurface;

							pTexture = m_pPackedResource->GetTexture((DWORD)0);

							if ( pTexture )
							{
								// Now make pTexture our texture.
								// First we need to get the params
								if ( SUCCEEDED( pTexture->GetLevelDesc( 0, &descSurface ) ) )
								{
									MakeBlank( descSurface.Width, descSurface.Height );
									LPDIRECT3DSURFACE8 pSrcSurface = NULL;
									LPDIRECT3DSURFACE8 pDestSurface = NULL;

									pTexture->GetSurfaceLevel( 0, &pSrcSurface );
									m_pTexture->GetSurfaceLevel( 0, &pDestSurface );

								    D3DXLoadSurfaceFromSurface( pDestSurface, NULL, NULL, 
										pSrcSurface, NULL, NULL,
										D3DX_DEFAULT, D3DCOLOR( 0 ) );

									m_iTextWidth = descSurface.Width;
									m_iTextHeight = descSurface.Height;
									m_iWidth = descSurface.Width;
									m_iHeight = descSurface.Height;

									// Release Surfaces
//								    SAFE_RELEASE( pSrcSurface );
								    SAFE_RELEASE( pDestSurface );

									// Release pTexture
								    SAFE_RELEASE( pTexture );

//								    LPDIRECT3DSURFACE8 pNewImageSurface = NULL;
//									hr = m_pd3dDevice->CreateImageSurface( m_iWidth, m_iHeight, D3DX_DEFAULT,
//																			&pNewImageSurface );


									m_bCache = false;
								}
							}
						}						
					}

				}
				else 
#endif
				if ( m_bVideo )
				{
					if ( m_pDSound == NULL )
					{
						m_pDSound = GetStateApp()->GetSoundDevice();
					}
					if ( m_pDSound && m_bAllowVideo )
					{
						//if ( m_pTexture )
						//{
						//	m_iWidth = 0;
						//	m_iHeight = 0;
						//	m_pTexture->Release();
						//	m_pTexture = NULL;
						//}
						//SAFE_RELEASE(m_pLinTexture);
						//if ( m_pVideoTexture )
						//{
						//	SAFE_RELEASE(m_pVideoSurface);
						//	m_pVideoTexture->Release();
						//	m_pVideoTexture= NULL;
						//}
						m_pWMVPlayer = new CWMVPlayer;
						if ( m_pWMVPlayer )
						{
							if( SUCCEEDED( m_pWMVPlayer->Initialize( m_pd3dDevice, m_pDSound ) ) )
							{

								if( SUCCEEDED( m_pWMVPlayer->OpenFile((char *)sFileName.c_str()) ) )
								{

									// Get information about the video format
									m_pWMVPlayer->GetVideoInfo( &m_videoInfo );
									m_iWidth = (int)m_videoInfo.dwWidth;
									m_iHeight = (int)m_videoInfo.dwHeight;

									int iWidth = m_iWidth;
									int iHeight = m_iHeight;

									hr = BuildRawTexture( m_pd3dDevice, iWidth, iHeight, &m_pTexture, D3DFMT_YUY2 );
									if ( SUCCEEDED(hr) )
									{
										m_iWidth = iWidth;
										m_iHeight = iHeight;
										m_iTextWidth = iWidth;
										m_iTextHeight = iHeight;
									}
								}
							}
						}

					}
				}
				else
				{

					// We need to do something special here with buffering.
					// Let's have a static interface to an image cache using
					// loaded-from-memory buffers to cache up to a MB of data.

					//if ( m_pTexture )
					//{
					//	m_pTexture->Release();
					//	m_pTexture = NULL;
					//}
					//if ( m_pVideoTexture )
					//{
					//	m_pVideoTexture->Release();
					//	m_pVideoTexture= NULL;
					//}
#ifdef NOCACHE
					hr = D3DXCreateTextureFromFileEx( 
							m_pd3dDevice, 
							sFileName.c_str(), 
							// "D:\\title.bmp", 
							D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
							0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
							m_dwAlphaColor, &m_info, NULL,
							(LPDIRECT3DTEXTURE8*)&m_pTexture
							); // D3DFMT_UNKNOWN


					D3DSURFACE_DESC descLevel;

					m_pTexture->GetLevelDesc(0, &descLevel );
					m_iTextWidth = descLevel.Width;
					m_iTextHeight = descLevel.Height;
					m_iWidth = m_info.Width; // descLevel.Width;
					m_iHeight = m_info.Height; // descLevel.Height;
#else
					m_sFilename = sCheckName;
					if ( g_FileManager.AddFileEntry(sCheckName) )
					{
						hr = S_OK;
					}
					m_bCache = true;
#endif
				}
			}
		}
	}
	else
	{
		hr = E_FAIL;
	}
	return hr;
}


void CImageSource::LoadFromCache( void )
{
	D3DXIMAGE_INFO *pInfo = NULL;
	if ( m_pTexture == NULL )
	{
		m_pTexture = g_FileManager.GetTexture(m_sFilename, &pInfo );
		if ( m_pTexture )
		{
			m_pTexture->AddRef();

			D3DSURFACE_DESC descLevel;
			m_pTexture->GetLevelDesc(0, &descLevel );
			m_iTextWidth = descLevel.Width;
			m_iTextHeight = descLevel.Height;
			if ( pInfo )
			{
				m_iWidth = pInfo->Width;
				m_iHeight = pInfo->Height;
				memcpy( &m_info, pInfo, sizeof(D3DXIMAGE_INFO) );
			}
			else
			{
				m_iWidth = descLevel.Width;
				m_iHeight = descLevel.Height;
			}
		}
	}
}

int	CImageSource::GetWidth( void ) 
{
	if ( m_bCache && !m_pTexture )
	{
		LoadFromCache();
	}
	return m_iWidth; 
}

int	CImageSource::GetHeight( void ) 
{ 
	if ( m_bCache && !m_pTexture )
	{
		LoadFromCache();
	}
	return m_iHeight; 
}



void	CImageSource::SetVideoLoop( bool bLoop ) 
{ 
	m_bVideoLoop = bLoop;
	if ( IsVideoTexture() )
	{
		m_pWMVPlayer->SetVideoLoop(bLoop);
	}
}

void	CImageSource::SetVideoSound( bool bSound ) 
{ 
	m_bVideoSound = bSound; 
	if ( IsVideoTexture() && m_pWMVPlayer )
	{
		m_pWMVPlayer->SetVideoSound(bSound);
	}
}

void	CImageSource::ReSync( void )
{
	if ( m_bVideo && m_pWMVPlayer  )
	{
        m_pWMVPlayer->ReSync();

	}
}


LPDIRECT3DTEXTURE8 CImageSource::GetTexture( void )
{
	if ( m_bVideo && m_pWMVPlayer  )
	{
		// Get next frame here....
        if( m_pWMVPlayer->IsReady() )
        {
            // Get the next frame into our texture
            m_pWMVPlayer->GetTexture( m_pTexture );
//			ConvertYUV2toARGB( m_pVideoTexture, m_pLinTexture, m_pTexture );

			// Scope the next series
			{
				// Now draw it on to our "real" texture
//				CXBEasel easel( m_pd3dDevice, m_pTexture );

				// Update the overlay surface to use this texture
//				m_pd3dDevice->UpdateOverlay( m_pVideoSurface, &m_SrcRect, &m_DestRect, FALSE, 0 );

			}

            // Decode the next frame
            m_pWMVPlayer->DecodeNext();
//			return m_pVideoTexture;
		}
	}
	if ( m_bCache )
	{
		if ( !m_pTexture ) // && m_bPackedResource )
		{
			LoadFromCache();
		}
	}
	return m_pTexture;
}

#if 0
HRESULT CImageSource::BuildRawTexture( int &iWidth, int &iHeight, D3DFORMAT d3dFormat, LPDIRECT3DTEXTURE8 *ppTexture )
{
	HRESULT hr = E_FAIL;

	if ( m_pd3dDevice )
	{
		switch( d3dFormat )
		{
			case D3DFMT_A8R8G8B8:
			case D3DFMT_X8R8G8B8:
			case D3DFMT_R5G6B5:
			case D3DFMT_X1R5G5B5:
				// For swizzled textures, make sure the dimensions are a power of two
				for( DWORD wmask=1; iWidth&(iWidth-1); wmask = (wmask<<1)+1 )
					iWidth = ( iWidth + wmask ) & ~wmask;
				for( DWORD hmask=1; iHeight&(iHeight-1); hmask = (hmask<<1)+1 )
					iHeight = ( iHeight + hmask ) & ~hmask;
				break;

			case D3DFMT_LIN_A8R8G8B8:
			case D3DFMT_LIN_X8R8G8B8:
			case D3DFMT_LIN_R5G6B5:
			case D3DFMT_LIN_X1R5G5B5:
				// For linear textures, make sure the stride is a multiple of 64 bytes
				iWidth  = ( iWidth + 0x1f ) & ~0x1f;
				break;

		}

		hr = m_pd3dDevice->CreateTexture( iWidth,
									 iHeight,
									 1,
									 0,
									 d3dFormat,
									 0,
									 ppTexture );
	}
	return hr;
}
#endif


bool CImageSource::CopySurface( LPDIRECT3DTEXTURE8 pTexture )
{
	bool bReturn = false;


	if ( pTexture && m_pTexture )
	{
		LPDIRECT3DSURFACE8 pSrcSurface;
		LPDIRECT3DSURFACE8 pDestSurface;

		if ( SUCCEEDED(pTexture->GetSurfaceLevel( 0, &pSrcSurface) ) )
		{
			if( SUCCEEDED( m_pTexture->GetSurfaceLevel( 0, &pDestSurface ) ) )
			{
				D3DXLoadSurfaceFromSurface( pDestSurface, NULL, NULL, 
					pSrcSurface, NULL, NULL,
					D3DX_DEFAULT, D3DCOLOR( 0 ) );
				pDestSurface->Release();
				bReturn = true;
			}
			pSrcSurface->Release();
		}
	}

	return bReturn;
}

bool CImageSource::DuplicateTexture( LPDIRECT3DTEXTURE8 pTexture, bool bClear )
{
	bool bReturn = false;

	if ( pTexture )
	{
		D3DSURFACE_DESC descSurface;

		// Now make pTexture our texture.
		// First we need to get the params
		if ( SUCCEEDED( pTexture->GetLevelDesc( 0, &descSurface ) ) )
		{
			ReleaseResources();
			MakeBlank( descSurface.Width, descSurface.Height, descSurface.Format );

			if ( m_pTexture )
			{

				bReturn = true;

				if ( bClear )
				{
					// EnterCriticalSection( &g_RenderCriticalSection );
					CXBEasel easel( m_pd3dDevice, m_pTexture );
					// LeaveCriticalSection( &g_RenderCriticalSection );
				}
				else
				{
					bReturn = CopySurface( pTexture );
				}
			}
		}
	}
	return bReturn;
}

HRESULT	CImageSource::MakeBlank( int iWidth, int iHeight, D3DFORMAT d3dFormat )
{
	HRESULT hr = E_FAIL;
	int iOrigWidth = iWidth, iOrigHeight = iHeight;

	if ( m_bVideo )
	{
		m_pWMVPlayer->CloseFile();
		delete m_pWMVPlayer;
		m_pWMVPlayer = NULL;
	}
	if ( m_pTexture )
	{
		m_pTexture->Release();
		m_pTexture = NULL;
	}
	if ( m_pVideoTexture )
	{
		m_pVideoTexture->Release();
		m_pVideoTexture= NULL;
	}

	hr = BuildRawTexture( m_pd3dDevice, iWidth, iHeight, &m_pTexture, d3dFormat );
	if ( SUCCEEDED( hr ) )
	{
		m_iTextWidth = iWidth;
		m_iTextHeight = iHeight;
		m_iWidth = iOrigWidth;
		m_iHeight = iOrigHeight;
	}
	return hr;
}
