

#include <xtl.h>
#include <XBUtil.h>
#include "XBEasel.h"



CXBEasel::CXBEasel( LPDIRECT3DDEVICE8 pD3D, LPDIRECT3DTEXTURE8 pTexture ) :
	 m_bValid(false)
	,m_pBackBuffer(NULL)
	,m_pZBuffer(NULL)
	,m_pTextureSurface(NULL)
	,m_pTexture(NULL)
	,m_p3dDevice(pD3D)
{
	if ( pTexture )
	{
		BeginDraw( pTexture );
	}
}

CXBEasel::~CXBEasel()
{
	EndDraw();
}

HRESULT	CXBEasel::BeginDraw( LPDIRECT3DTEXTURE8 pTexture )
{
	HRESULT hr = S_OK;
	if ( m_p3dDevice )
	{
		if ( m_pTexture == NULL )
		{
			// Set the texture
			m_pTexture = pTexture;

			// Get the current backbuffer and zbuffer
			m_p3dDevice->GetViewport( &m_vpOld );
			m_p3dDevice->GetRenderTarget( &m_pBackBuffer );
			m_p3dDevice->GetDepthStencilSurface( &m_pZBuffer );

			// Set the new texture as the render target
			m_pTexture->GetSurfaceLevel( 0, &m_pTextureSurface );

			D3DSURFACE_DESC surfInfo;
			m_pTexture->GetLevelDesc( 0, &surfInfo );
			
			D3DVIEWPORT8 vp = { 0, 0, surfInfo.Width, surfInfo.Height, 0.0f, 1.0f };
			m_p3dDevice->SetRenderTarget( m_pTextureSurface, NULL );
			m_p3dDevice->SetViewport( &vp );
			m_p3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0, 1.0f, 0L );
			m_bValid = true;
		}
	}
	else
	{
		hr = E_FAIL;
	}
	return hr;
}

HRESULT	CXBEasel::EndDraw( void )
{
	HRESULT hr = S_OK;
	if ( m_pTexture != NULL )
	{
		if ( m_p3dDevice )
		{
			// Restore the render target
			m_p3dDevice->SetRenderTarget( m_pBackBuffer, m_pZBuffer );
			m_p3dDevice->SetViewport( &m_vpOld );
			SAFE_RELEASE( m_pBackBuffer );
			SAFE_RELEASE( m_pZBuffer );
			SAFE_RELEASE( m_pTextureSurface );
		}
	}
	return hr;
}

