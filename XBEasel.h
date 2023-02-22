/*********************************************************************************\
 * XBEasel.h
 * Class used to help redirect direct3d calls to a texture surface
 * (C) 2002 MasterMind
\*********************************************************************************/


#ifndef __XBEASEL_H__
#define __XBEASEL_H__

// This class helps out by saving the current drawing state
// and temporarily redirecting it to a texture surface.
// When the object goes out of scope or is otherwise destroyed,
// it restores the drawing state.
//
// Usage:
// 
// {    <- start of scope
//		CXBEasel easel( pD3DDevice, pTexture );   <- object created, state saved
//
//
//		pD3DDevice->SomeDrawingFunction();  <-draws to pTexture, not screen
//
//
// }  <- End of scope, object destroyed, drawing state restored
//
class CXBEasel
{
	LPDIRECT3DSURFACE8 m_pBackBuffer;
	LPDIRECT3DSURFACE8 m_pZBuffer;
	LPDIRECT3DSURFACE8 m_pTextureSurface;
	LPDIRECT3DTEXTURE8 m_pTexture;
	bool				m_bValid;
	D3DVIEWPORT8		m_vpOld;
	LPDIRECT3DDEVICE8	m_p3dDevice;
public:
	CXBEasel( LPDIRECT3DDEVICE8 pD3D, LPDIRECT3DTEXTURE8 pTexture = NULL );
	~CXBEasel();
	HRESULT	BeginDraw( LPDIRECT3DTEXTURE8 pTexture );
	HRESULT	EndDraw( void );
	bool	IsValid( void ) { return m_bValid; };
};	



#endif

