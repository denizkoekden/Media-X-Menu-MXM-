/*********************************************************************************\
 * ImgFileCache.cpp
 * Class used to hold Img Files
 * (C) 2003 MasterMind
\*********************************************************************************/

#include "ImgFileCache.h"
#include "utils.h"
#include "XBStateApp.h"
#include "MXM.h"
#include "MenuInfo.h"
#include "ItemInfo.h"
#include "CommDebug.h"
#include "BinData.h"

CImageFileManager g_FileManager;
CImageResourceCache g_ImgResCache;
CSoundResourceCache g_SndResCache;
extern CMenuInfo g_MenuInfo;

int CFileCacheEntry::s_ID = 0;
LPDIRECT3DDEVICE8	CFileCacheEntry::m_pd3dDevice = NULL;


CSoundResourceCache::CSoundResourceCache()
{
}

CSoundResourceCache::~CSoundResourceCache()
{
	
}

void CSoundResourceCache::Clear( void )
{
	CSoundResourceEntry * pEntry;
	TListSoundResEntries::iterator iterEntries;
	iterEntries = m_slEntries.begin();
	
	while( iterEntries != m_slEntries.end() )
	{
		pEntry = (*iterEntries)->GetPtr();
		if ( pEntry )
		{
			delete pEntry;
		}
		iterEntries++;
	}
	m_slEntries.clear();
}

CXBSound * CSoundResourceCache::GetSound( LPCTSTR szName )
{
	CXBSound * pReturn = NULL;
	CSoundResourceEntry * pEntry;
	CStdString sName(szName);
	TListSoundResEntries::iterator iterEntries;
	TStringList::iterator iterString;

	sName.MakeLower();
	if ( szName && _tcslen( szName ) )
	{
		iterEntries = m_slEntries.begin();
		
//		int iSize = m_slEntries.size();

		while( ( pReturn == NULL ) && ( iterEntries != m_slEntries.end() ) )
		{
			pEntry = (*iterEntries)->GetPtr();
			if ( pEntry )
			{
				iterString = pEntry->m_slResNames.begin();
				while(  ( pReturn == NULL ) && ( iterString != pEntry->m_slResNames.end() ) )
				{
					if ( sName.Compare( iterString->c_str() ) == 0 )
					{
						pReturn = &(pEntry->m_snd);
						break;
					}
					iterString++;
				}
			}
			iterEntries++;
		}
	}

	return pReturn;
}

CSoundResourceEntry * CSoundResourceCache::GetSoundEntryByFilename( LPCTSTR szName )
{
	CSoundResourceEntry * pReturn = NULL;
	CSoundResourceEntry * pEntry;
	CStdString sName(szName);
	TListSoundResEntries::iterator iterEntries;

	sName.MakeLower();
	if ( szName && _tcslen( szName ) )
	{
		iterEntries = m_slEntries.begin();

		while( iterEntries != m_slEntries.end() )
		{
			pEntry = (*iterEntries)->GetPtr();
			if ( pEntry->m_sFileName.Compare( sName ) == 0 )
			{
				pReturn = pEntry;
				break;
			}
			iterEntries++;
		}
	}

	return pReturn;
}

void CSoundResourceCache::AddEntry( CXMLNode * pNode, LPCTSTR szDirectory  )
{
	CStdString sName, sFilename;

	if( pNode )
	{
		sName = pNode->GetString( NULL, _T("name"), _T("") );
		if ( sName.GetLength() )
		{
			sFilename = pNode->GetString( NULL, _T("file"), _T("") );
			if ( sFilename.GetLength() )
			{
				AddEntry( sName, sFilename, szDirectory );
			}
		}
	}
}

void CSoundResourceCache::AddEntry( LPCTSTR szName, LPCTSTR szFilename, LPCTSTR szDirectory  )
{
	CStdString sName(szName), sFilename(szFilename);
	CSoundResourceEntry * pNewEntry = NULL;

	sName.MakeLower();

	if( szName && szFilename && _tcslen(szName) && _tcslen(szFilename) )
	{
		// Make sure it's not already loaded!
		if ( szDirectory && _tcslen( szDirectory ) )
		{
			sFilename = MakeFullFilePath( szDirectory, sFilename );
		}
		if ( ( GetSound(szName) == NULL ) && FileExists(sFilename) )
		{
			pNewEntry = GetSoundEntryByFilename(sFilename);
			if ( pNewEntry )
			{
				// Add resource name to resource
				pNewEntry->m_slResNames.push_back(sName);
			}
			else
			{
				pNewEntry = new CSoundResourceEntry;
				if ( pNewEntry )
				{
					pNewEntry->m_slResNames.push_back(sName);
					pNewEntry->m_sFileName = sFilename;
					pNewEntry->m_sFileName.MakeLower();
					pNewEntry->m_snd.Create(sFilename);
					m_slEntries.push_back( pNewEntry );
				}
			}
		}
	}
}


CImageResourceEntry * CImageResourceCache::GetEntry( LPCTSTR szName )
{
	CImageResourceEntry * pReturn = NULL;
	TMapImgResEntries::iterator iterEntry;
	CStdString sName;

	if ( szName && _tcslen(szName) )
	{
		sName = szName;
		sName.MakeLower();
		iterEntry = m_miEntries.begin();
		while( !pReturn && (iterEntry != m_miEntries.end()) )
		{
			if ( sName.Compare(iterEntry->first.c_str()) == 0 )
			{
				pReturn = iterEntry->second;
			}
			iterEntry++;
		}
	}
	return pReturn;
}

void CImageResourceCache::Clear( void )
{
	CImageResourceEntry * pReturn = NULL;
	TMapImgResEntries::iterator iterEntry;
	iterEntry = m_miEntries.begin();
	while( !pReturn && (iterEntry != m_miEntries.end()) )
	{
		pReturn = iterEntry->second;
		if ( pReturn )
		{
			delete pReturn;
		}
		iterEntry++;
	}
	m_miEntries.clear();
}

void	CImageResourceCache::AddEntry( CXMLNode * pNode, LPCTSTR szDirectory  )
{
	CStdString sName;
	if( pNode )
	{
		sName = pNode->GetString( NULL, _T("name"), _T("") );
		if ( sName.GetLength() )
		{
			sName.MakeLower();
			if ( GetEntry(sName) == NULL )
			{
				m_miEntries.insert( TMapImgResEntries::value_type(sName, new CImageResourceEntry(pNode, szDirectory)) );
			}
		}
	}
}

CImageResourceCache::CImageResourceCache()
{
}

CImageResourceCache::~CImageResourceCache()
{
}



/*


The concept of image cache and precache

  On the Xbox, we can load image files from system memory,
  so we will take advantage of the faster texture loading by 
  allowing a certain amount of out images to be pre-loaded into 
  system memory.

  g_FileManager will have a method for pre-cacheing files by
  adding them to the load queue.

  CImageSource will always try and retrieve files from the file 
  manager.


  PROGRAM:
   g_FileManager.Precache( szFilename );



   CImageSource imgSource(FileName); // Precaches

  imgSource->GetTexture()
    Creates texture if it doesn't exist yet!

  */





bool CFileCacheEntry::Loaded( void )
{
	bool bReturn = false;

//	if ( m_pTexture ) bReturn = true;
	// else 
	if ( m_bMemImage || ( m_pbyBuffer && m_dwFileSize )) bReturn = true;
	return bReturn;
}

bool CFileCacheEntry::Ready( void )
{
	bool bReturn = false;

	if ( m_pTexture ) bReturn = true;
	return bReturn;
}


void CFileCacheEntry::DumpBuffer( void )
{
	if ( m_pbyBuffer )
	{
		// Release mem buffer of file...
		delete [] m_pbyBuffer;
		m_pbyBuffer = NULL;
		m_dwFileSize = 0;
	}
}

void CFileCacheEntry::ValidateTypes( void )
{
	CStdString sFilename;

	m_bPNGGIF = false;
	m_bMemImage = false;
	// Check to see if we need to redirect to TitleImage.xbx
	sFilename = m_sFilename;
	sFilename.MakeLower();
	if ( sFilename.Find( _T(".xbe"), 0 ) >= 0 )
	{
		// It's an XBE.... make it the correct TitleImage.xbx
		DWORD dwID;

		dwID = GetXbeID( m_sFilename );
		m_sFilename.Format( _T("E:\\UDATA\\%08x\\TitleImage.xbx"), dwID );
		m_bIsXPR = true;
	}
	else if ( sFilename.Find( _T(".xbx"), 0 ) >= 0 )
	{
		m_bIsXPR = true;
	}
	else if ( ( sFilename.Find( _T(".png"), 0 ) >= 0 ) || ( sFilename.Find( _T(".gif"), 0 ) >= 0 ) )
	{
		m_bPNGGIF = true;
	}
	else if ( (sFilename.GetLength()>1) && (sFilename[0] == _T(':')) && (sFilename[1] == _T(':')) )
	{
		m_bMemImage = true;
	}
}

HRESULT CFileCacheEntry::LoadFile( LPCTSTR szFilename, LPCTSTR szDirectory  )
{
	HRESULT hr = E_FAIL;
	CStdString sFilename;

	if ( szFilename && _tcslen(szFilename) && (m_sFilename.GetLength()==0) )
	{
		if ( szDirectory && _tcslen( szDirectory ) )
		{
			m_sFilename = MakeFullFilePath( szDirectory, szFilename );
		}
		else
		{
			m_sFilename = szFilename;
		}
	}

	ValidateTypes();

#if 0
	m_bPNGGIF = false;
	// Check to see if we need to redirect to TitleImage.xbx
	sFilename = m_sFilename;
	sFilename.MakeLower();
	if ( sFilename.Find( _T(".xbe"), 0 ) >= 0 )
	{
		// It's an XBE.... make it the correct TitleImage.xbx
		DWORD dwID;

		dwID = GetXbeID( m_sFilename );
		m_sFilename.Format( _T("E:\\UDATA\\%08x\\TitleImage.xbx"), dwID );
		m_bIsXPR = true;
	}
	else if ( sFilename.Find( _T(".xbx"), 0 ) >= 0 )
	{
		m_bIsXPR = true;
	}
	else if ( ( sFilename.Fond( _T(".png"), 0 ) >= 0 ) && ( sFilename.Fond( _T(".gif"), 0 ) >= 0 ) )
	{
		m_bPNGGIF = true;
	}
#endif

	// Do we have texture already? Don't bother loading...
	if ( m_bMemImage || ( m_pbyBuffer && m_dwFileSize ) || m_pTexture )
	{
		hr = S_OK;
	}
	else
	{
		if ( !FileExists(m_sFilename ) )
		{
			m_bExists = false;
		}
		else
		{
			m_pbyBuffer = LoadBufferFromFile( m_sFilename, &m_dwFileSize );
			m_dwLoadTimeStamp = GetTickCount();
		}
	}
	return hr;
}

HRESULT CFileCacheEntry::GetBuffer( LPBYTE * ppbyBuffer, DWORD * pdwSize )
{
	HRESULT hr = E_FAIL;

	if ( m_pTexture || m_bMemImage )
	{
		*ppbyBuffer = NULL;
		*pdwSize = 0;
		hr = S_OK;
	}
	else
	{
		if ( m_pbyBuffer == NULL )
		{
			hr = LoadFile();
		}
		else
		{
			hr = S_OK;
		}
		if ( m_pbyBuffer )
		{
			if ( ppbyBuffer )
			{
				*ppbyBuffer = m_pbyBuffer;
			}
			if ( pdwSize )
			{
				*pdwSize = m_dwFileSize;
			}
		}
	}

	return hr;
}

HRESULT CFileCacheEntry::GetTexture( LPDIRECT3DTEXTURE8 * ppTexture, D3DXIMAGE_INFO ** ppInfo /* = NULL */ )
{
	HRESULT hr = E_FAIL;
	D3DFORMAT d3dFormat = D3DFMT_UNKNOWN;
	PALETTEENTRY pal[256];

	if ( m_pTexture == NULL )
	{
		if ( m_bMemImage )
		{
			BYTE * pBuffer = NULL;
			DWORD dwSize = 0;
			DWORD dwLoc = 0;
			CStdString sSection;
			

			// Load from Mem Image....
			if ( _tcsicmp( m_sFilename, _T("::skytile") ) == 0 )
			{
				sSection = _T("LogoData");
				pBuffer = (BYTE*)g_pbSkyTileData;
				dwSize = g_dwSkyTileLength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::logo") ) == 0 )
			{
				sSection = _T("LogoData");
				pBuffer = (BYTE*)g_pbLargeLogoData;
				dwSize = g_dwLargeLogoLength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::ico_action") ) == 0 )
			{
				sSection = _T("LogoData");
				pBuffer = (BYTE*)g_pbICO_ACTIONData;
				dwSize = g_dwICO_ACTIONLength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::ico_dir") ) == 0 )
			{
				sSection = _T("LogoData");
				pBuffer = (BYTE*)g_pbICO_DIRData;
				dwSize = g_dwICO_DIRLength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::ico_dvd") ) == 0 )
			{
				sSection = _T("LogoData");
				pBuffer = (BYTE*)g_pbICO_DVDData;
				dwSize = g_dwICO_DVDLength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::ico_emu") ) == 0 )
			{
				sSection = _T("LogoData");
				pBuffer = (BYTE*)g_pbICO_EMUData;
				dwSize = g_dwICO_EMULength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::ico_file") ) == 0 )
			{
				sSection = _T("LogoData");
				pBuffer = (BYTE*)g_pbICO_FILEData;
				dwSize = g_dwICO_FILELength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::ico_hd") ) == 0 )
			{
				sSection = _T("LogoData");
				pBuffer = (BYTE*)g_pbICO_HDData;
				dwSize = g_dwICO_HDLength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::ico_lock") ) == 0 )
			{
				sSection = _T("LogoData");
				pBuffer = (BYTE*)g_pbICO_LOCKData;
				dwSize = g_dwICO_LOCKLength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::ico_media") ) == 0 )
			{
				sSection = _T("LogoData");
				pBuffer = (BYTE*)g_pbICO_MEDIAData;
				dwSize = g_dwICO_MEDIALength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::ico_music") ) == 0 )
			{
				sSection = _T("LogoData");
				pBuffer = (BYTE*)g_pbICO_MUSICData;
				dwSize = g_dwICO_MUSICLength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::ico_wave") ) == 0 )
			{
				sSection = _T("LogoData");
				pBuffer = (BYTE*)g_pbICO_WAVEData;
				dwSize = g_dwICO_WAVELength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::ico_xbe") ) == 0 )
			{
				sSection = _T("LogoData");
				pBuffer = (BYTE*)g_pbICO_XBEData;
				dwSize = g_dwICO_XBELength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::helpgcscreen") ) == 0 )
			{
				sSection = c_szHelpGCSection;
				pBuffer = (BYTE*)g_pbHelpGC;
				dwSize = g_dwHelpGCLength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::helpkbscreen") ) == 0 )
			{
				sSection = c_szHelpKBSection;
				pBuffer = (BYTE*)g_pbHelpKB;
				dwSize = g_dwHelpKBLength;
			}
			else if ( _tcsicmp( m_sFilename, _T("::helpirscreen") ) == 0 )
			{
				sSection = c_szHelpIRSection;
				pBuffer = (BYTE*)g_pbHelpIR;
				dwSize = g_dwHelpIRLength;
			}
			if ( sSection.GetLength() )
			{
				dwLoc = (DWORD)XLoadSection(sSection);
			}
			if(dwLoc!=NULL)
			{
				EnterCriticalSection( &g_RenderCriticalSection );

				hr = D3DXCreateTextureFromFileInMemoryEx( 
						m_pd3dDevice, 
						pBuffer, dwSize,
						D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
						0, d3dFormat, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
						m_dwAlpha, &m_info, pal,
						(LPDIRECT3DTEXTURE8*)&m_pTexture
						); // D3DFMT_UNKNOWN
				if ( SUCCEEDED( hr ) )
				{
					*ppTexture = m_pTexture;
				}
				LeaveCriticalSection( &g_RenderCriticalSection );
				XFreeSection(sSection);
			}
		}
		else
		{
			if ( m_pbyBuffer && m_dwFileSize )
			{
				hr = GetBuffer( NULL, NULL );
				if ( SUCCEEDED( hr ) )
				{
					if ( m_dwFileSize && m_pbyBuffer )
					{
						try
						{
							if ( m_bIsXPR )
							{
								CXBPackedResource PackedResource;

								hr = PackedResource.CreateFromMem( m_pd3dDevice, NULL, m_pbyBuffer, 1, NULL );

								if ( SUCCEEDED(hr) )
								{
									hr = BuildFromResource( PackedResource );
								}
							}
							else
							{
								if ( m_bPNGGIF )
								{
									d3dFormat = D3DFMT_A8R8G8B8;
								}
								EnterCriticalSection( &g_RenderCriticalSection );

								hr = D3DXCreateTextureFromFileInMemoryEx( 
										m_pd3dDevice, 
										m_pbyBuffer, m_dwFileSize,
										D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
										m_bPNGGIF?D3DUSAGE_RENDERTARGET:0, d3dFormat, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
										m_bPNGGIF?0xFF010203:m_dwAlpha, &m_info, pal,
										(LPDIRECT3DTEXTURE8*)&m_pTexture
										); // D3DFMT_UNKNOWN
								LeaveCriticalSection( &g_RenderCriticalSection );
							}
							if ( SUCCEEDED( hr ) )
							{
								// Release mem buffer of file...
								delete [] m_pbyBuffer;
								m_pbyBuffer = NULL;
								m_dwFileSize = 0;

								//if ( m_info.Format == D3DFMT_P8 )
								//{
								//	int iWidth = m_info.Width, iHeight = m_info.Height;
								//	LPDIRECT3DTEXTURE8 pNewTexture;
								//	D3DSURFACE_DESC desc;

								//	m_pTexture->GetLevelDesc( 0, &desc );

								//	if ( SUCCEEDED(BuildRawTexture( m_pd3dDevice, iWidth, iHeight, &pNewTexture, d3dFormat )))
								//	{
								//		LPDIRECT3DSURFACE8 pSrcSurface;
								//		LPDIRECT3DSURFACE8 pDestSurface;
								//		bool bResult = false;

								//		pNewTexture->GetLevelDesc( 0, &desc );

								//		if ( SUCCEEDED(m_pTexture->GetSurfaceLevel( 0, &pSrcSurface) ) )
								//		{
								//			if( SUCCEEDED( pNewTexture->GetSurfaceLevel( 0, &pDestSurface ) ) )
								//			{
								//				hr = D3DXLoadSurfaceFromSurface( pDestSurface, NULL, NULL, 
								//					pSrcSurface, pal, NULL,
								//					D3DX_DEFAULT, 0xFF010203 );
								//				pDestSurface->Release();
								//				bResult = true;
								//			}
								//			pSrcSurface->Release();
								//		}
								//		// Copy texture to different format
								//		if ( bResult )
								//		{
								//			m_pTexture->Release();
								//			m_pTexture = pNewTexture;
								//		}
								//	}
								//}
								// 
								*ppTexture = m_pTexture;
							}
						}
						catch (...)
						{
							// Unknown problem Just catch it!
							hr = E_FAIL;
						}
					}
					else
					{
						hr = E_FAIL;
					}
				}
			}
			else
			{
				try
				{
					ValidateTypes();
					if ( !FileExists(m_sFilename ) )
					{
						m_bExists = false;
					}
					else
					{
						if ( m_bIsXPR )
						{
							CXBPackedResource PackedResource;

							hr = PackedResource.Create( m_pd3dDevice, m_sFilename, 1, NULL );

							if ( SUCCEEDED(hr) )
							{
								hr = BuildFromResource( PackedResource );
							//	LPDIRECT3DTEXTURE8 pTexture;
							//	D3DSURFACE_DESC descSurface;

							//	pTexture = PackedResource.GetTexture((DWORD)0);

							//	if ( pTexture )
							//	{
							//		// Now make pTexture our texture.
							//		// First we need to get the params
							//		if ( SUCCEEDED( pTexture->GetLevelDesc( 0, &descSurface ) ) )
							//		{
							//			int iWidth = descSurface.Width, iHeight = descSurface.Height;

							//			BuildRawTexture( m_pd3dDevice, iWidth, iHeight, &m_pTexture );
							//			// MakeBlank( descSurface.Width, descSurface.Height );
							//			LPDIRECT3DSURFACE8 pSrcSurface = NULL;
							//			LPDIRECT3DSURFACE8 pDestSurface = NULL;

							//			pTexture->GetSurfaceLevel( 0, &pSrcSurface );
							//			m_pTexture->GetSurfaceLevel( 0, &pDestSurface );

							//			D3DXLoadSurfaceFromSurface( pDestSurface, NULL, NULL, 
							//				pSrcSurface, NULL, NULL,
							//				D3DX_DEFAULT, D3DCOLOR( 0 ) );

							//			m_info.Width = descSurface.Width;
							//			m_info.Height = descSurface.Height;
							//			m_info.Format = D3DFMT_A8R8G8B8;
							//			m_info.Depth = 32;
							//			m_info.MipLevels = 0;

							//			if ( SUCCEEDED( m_pTexture->GetLevelDesc( 0, &descSurface ) ) )
							//			{
							//				m_info.Format = descSurface.Format;
							//			}

							//			// Release Surfaces
							//			SAFE_RELEASE( pDestSurface );

							//			// Release pTexture
							//			SAFE_RELEASE( pTexture );
							//		}
							//	}
							}

						}
						else
						{
							// Direct loading!
							if ( m_bPNGGIF )
							{
								d3dFormat = D3DFMT_A8R8G8B8;
							}
							EnterCriticalSection( &g_RenderCriticalSection );
							hr = D3DXCreateTextureFromFileEx( 
									m_pd3dDevice, 
									m_sFilename, 
									D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
									m_bPNGGIF?D3DUSAGE_RENDERTARGET:0, d3dFormat, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
									m_dwAlpha, &m_info, NULL,
									(LPDIRECT3DTEXTURE8*)&m_pTexture
									); // D3DFMT_UNKNOWN
							LeaveCriticalSection( &g_RenderCriticalSection );
						}
	//					if ( FAILED( hr ) )
	//					{
	//					}
					}
				}
				catch (...)
				{
					hr = E_FAIL;
				}
				if ( SUCCEEDED(hr) )
				{
					*ppTexture = m_pTexture;
				}
			}
		}
	}
	else
	{
		hr = S_OK;
		*ppTexture = m_pTexture;
	}
	if ( m_pTexture )
	{
		m_dwTextureTimeStamp = GetTickCount();
		if ( ppInfo )
		{
			*ppInfo = &m_info;
		}
	}

	return hr;
}

HRESULT CFileCacheEntry::BuildFromResource( CXBPackedResource & PackedResource )
{
	HRESULT hr = E_FAIL;
	LPDIRECT3DTEXTURE8 pTexture;
	D3DSURFACE_DESC descSurface;

	pTexture = PackedResource.GetTexture((DWORD)0);

	if ( pTexture )
	{
		// Now make pTexture our texture.
		// First we need to get the params
		EnterCriticalSection( &g_RenderCriticalSection );
		if ( SUCCEEDED( pTexture->GetLevelDesc( 0, &descSurface ) ) )
		{
			int iWidth = descSurface.Width, iHeight = descSurface.Height;

			BuildRawTexture( m_pd3dDevice, iWidth, iHeight, &m_pTexture );
			// MakeBlank( descSurface.Width, descSurface.Height );
			LPDIRECT3DSURFACE8 pSrcSurface = NULL;
			LPDIRECT3DSURFACE8 pDestSurface = NULL;

			pTexture->GetSurfaceLevel( 0, &pSrcSurface );
			m_pTexture->GetSurfaceLevel( 0, &pDestSurface );

			D3DXLoadSurfaceFromSurface( pDestSurface, NULL, NULL, 
				pSrcSurface, NULL, NULL,
				D3DX_DEFAULT, D3DCOLOR( 0 ) );

			m_info.Width = descSurface.Width;
			m_info.Height = descSurface.Height;
			m_info.Format = D3DFMT_A8R8G8B8;
			m_info.Depth = 32;
			m_info.MipLevels = 0;

			if ( SUCCEEDED( m_pTexture->GetLevelDesc( 0, &descSurface ) ) )
			{
				m_info.Format = descSurface.Format;
			}

			// Release Surfaces
			SAFE_RELEASE( pDestSurface );

			// Release pTexture
			SAFE_RELEASE( pTexture );
		}
		LeaveCriticalSection( &g_RenderCriticalSection );
	}

	return hr;
}



HRESULT CFileCacheEntry::ReleaseTexture( void )
{
	HRESULT hr = E_FAIL;

	m_pTexture->Release();
	m_pTexture = NULL;
	return hr;
}


CFileCacheEntry::CFileCacheEntry( const CFileCacheEntry & src )
{
	CFileCacheEntry * pPtr = (CFileCacheEntry *)&src;
	m_sFilename = src.m_sFilename;
	m_bIsXPR = src.m_bIsXPR;
	pPtr->m_sFilename = _T("");
	m_iID = src.m_iID;
	pPtr->m_iID = -1;
	m_dwFileSize = src.m_dwFileSize;
	pPtr->m_dwFileSize = 0;
	m_bMemImage = src.m_bMemImage;
	m_pbyBuffer = src.m_pbyBuffer;
	pPtr->m_pbyBuffer = NULL;
	m_pTexture = src.m_pTexture;
	pPtr->m_pTexture = NULL;
	m_dwAlpha = src.m_dwAlpha;
	m_bExists = src.m_bExists;
	m_bPNGGIF = src.m_bPNGGIF;
}

CFileCacheEntry::CFileCacheEntry() :
	m_bIsXPR(false),
	m_bMemImage(false),
	m_bPNGGIF(false),
	m_iID(s_ID++),
	m_dwAlpha(0),
	m_dwFileSize(0),
	m_pbyBuffer(NULL),
	m_bExists(true),
	m_pTexture(NULL)
{
	if ( m_pd3dDevice == NULL )
	{
		m_pd3dDevice = GetStateApp()->Get3dDevice();
	}
}

CFileCacheEntry::CFileCacheEntry(LPCTSTR szFilename, LPCTSTR szDirectory ) :
	m_bIsXPR(false),
	m_bPNGGIF(false),
	m_bMemImage(false),
	m_iID(s_ID++),
	m_sFilename(szFilename),
	m_dwAlpha(0),
	m_dwFileSize(0),
	m_bExists(true),
	m_pbyBuffer(NULL),
	m_pTexture(NULL)
{
	if ( szDirectory && _tcslen( szDirectory ) )
	{
		m_sFilename = MakeFullFilePath( m_sFilename, szDirectory );
	}
	if ( m_pd3dDevice == NULL && GetStateApp() )
	{
		m_pd3dDevice = GetStateApp()->Get3dDevice();
	}
}


CFileCacheEntry::~CFileCacheEntry()
{
	if ( m_pTexture )
	{
		m_pTexture->Release();
	}
	if ( m_pbyBuffer )
	{
		delete [] m_pbyBuffer;
	}
}


CImageFileManager::CImageFileManager() :
	m_dwBufferMax(4L*1024L*1024L),
	m_bThreadStarted(false)
{
	CFileCacheEntry* pReturn = NULL;

//	pReturn = AddFileEntry( _T("TestEntry") );
}

CImageFileManager::~CImageFileManager()
{
}

CFileCacheEntry* CImageFileManager::GetFileEntry( LPCTSTR szFileName, LPCTSTR szDirectory )
{
	CFileCacheEntry* pReturn = NULL;
	CStdString sFileName(szFileName);

	if ( szFileName && _tcslen(szFileName) )
	{
		TMapFileEntries::iterator iterEntry;

		if ( szDirectory && _tcslen( szDirectory ) )
		{
			sFileName = MakeFullFilePath( szFileName, szDirectory );
		}

		sFileName.MakeLower();
		iterEntry = m_mfEntries.begin();
		while( !pReturn && iterEntry != m_mfEntries.end() )
		{
			if ( sFileName.Compare( iterEntry->first.c_str() ) == 0 )
			{
				pReturn = iterEntry->second->GetPtr();
			}
			iterEntry++;
		}

	}

	return pReturn;
}

CFileCacheEntry* CImageFileManager::AddFileEntry( LPCTSTR szFileName, LPCTSTR szDirectory )
{
	CFileCacheEntry* pReturn = NULL;
	CStdString sFileName(szFileName);
	CStdString sFileNameLower;

	if ( !m_bThreadStarted )
	{
		StartThread();
	}
	if ( szFileName && _tcslen(szFileName) )
	{
		if ( szDirectory && _tcslen( szDirectory ) )
		{
			sFileName = MakeFullFilePath( sFileName, szDirectory );
		}
		if ( IsImageFile( sFileName, true ) )
		{
			sFileNameLower = sFileName;
			sFileNameLower.MakeLower();
			pReturn = GetFileEntry( sFileNameLower );
			if ( pReturn == NULL )
			{
				// Add a new entry now...
				m_mfEntries.insert( TMapFileEntries::value_type( sFileNameLower, new CFileCacheEntry(sFileName) ) );
				pReturn = GetFileEntry( sFileNameLower );
				QueueLoad( sFileName );
			}
		}
	}

	return pReturn;
}


void CImageFileManager::StartThread( void )
{
	m_hQueuedEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	// Create the worker thread
	m_hThread = CreateThread( NULL, WORKER_THREAD_STACK_SIZE, InitializeThreadProc, this, 0, NULL );
	m_bThreadStarted = true;
}

CFileCacheEntry* CImageFileManager::FindOldestLoadedEntry( void )
{
	CFileCacheEntry* pEntry = NULL;

	return pEntry;
}


bool CImageFileManager::Prune( void )
{
	bool bReturn = true;
	DWORD dwBufferTotal = 0;
	int iLowPriority = -54321;
	TMapFileEntries::iterator iterEntry;
	CFileCacheEntry* pEntry = NULL;
	DWORD dwTimeStamp = GetTickCount();
	
	
	iterEntry = m_mfEntries.begin();
	while( iterEntry != m_mfEntries.end() )
	{
		if ( iterEntry->second->Loaded() )
		{
			dwBufferTotal += iterEntry->second->GetSize();
			if ( dwTimeStamp > iterEntry->second->GetLoadTimestamp() )
			{
				dwTimeStamp = iterEntry->second->GetLoadTimestamp();
				pEntry = iterEntry->second->GetPtr();
			}
			if ( iLowPriority == -54321 )
			{
			}
			else
			{
				// See if it's the lowest...
			}
		}
		iterEntry++;
	}
	if ( dwBufferTotal > m_dwBufferMax )
	{
		// Need to reduce the load!
		if ( pEntry )
		{
			dwBufferTotal-= pEntry->GetSize();
			pEntry->DumpBuffer();
			while( pEntry && ( dwBufferTotal > m_dwBufferMax ) )
			{
				pEntry = FindOldestLoadedEntry();
				if ( pEntry )
				{
					dwBufferTotal-= pEntry->GetSize();
					pEntry->DumpBuffer();
				}
			}
		}
	}
	return bReturn;
}


DWORD CImageFileManager::LoadAndPruneThread( void )
{
	DWORD dwReturn = 0;
	CStdString sLoadValue;
	CFileCacheEntry* pEntry;
	DWORD dwResult;
	
	while(1)
	{
		if ( m_sqLoad.size() )
		{
			sLoadValue = m_sqLoad.front();
			m_sqLoad.pop_front();
			pEntry = GetFileEntry(sLoadValue);
			if ( !pEntry->Ready() && !pEntry->Loaded() )
			{
				pEntry->LoadFile();
			}
		}
		Sleep(1);
		if ( m_sqLoad.size() == 0 )
		{
			// Wait on object...
	        dwResult = WaitForSingleObject( m_hQueuedEvent, 100 );
		}
	}
	return dwReturn;
}

void CImageFileManager::QueueLoad( LPCTSTR szFileName, bool bImmediate )
{
	if ( szFileName && _tcslen( szFileName ) )
	{
		if ( IsImageFile( szFileName, true ) )
		{
			if ( bImmediate )
			{
				m_sqLoad.push_front( szFileName );
			}
			else
			{
				m_sqLoad.push_back( szFileName );
			}
			SetEvent( m_hQueuedEvent );
		}
	}
}


DWORD CImageFileManager::InitializeThreadProc(LPVOID lpParameter )
{
	DWORD dwReturn = 0;
	CImageFileManager * pThis = (CImageFileManager*)lpParameter;

	dwReturn = pThis->LoadAndPruneThread();

	return dwReturn;
}

LPDIRECT3DTEXTURE8 CImageFileManager::GetTexture( LPCTSTR szFileName, D3DXIMAGE_INFO ** ppInfo /* = NULL */ )
{
	LPDIRECT3DTEXTURE8 pTexture = NULL;
	CFileCacheEntry* pEntry;
	int iTimeout = 0;

	pEntry = GetFileEntry(szFileName);
	if ( pEntry )
	{
		if( !pEntry->Ready() )
		{
			if ( !pEntry->Loaded() )
			{
				QueueLoad(szFileName, true );
				// Now wait until it is loaded....
				while( !pEntry->Loaded() && iTimeout<1000 )
				{
					Sleep(30);
					iTimeout+=10;
				}
			}
			else
			{
				pEntry->GetTexture(&pTexture,ppInfo);
			}
		}
		if( pEntry->Ready() )
		{
			pEntry->GetTexture(&pTexture,ppInfo);
			if ( pTexture )
			{
				pTexture->AddRef();
			}
		}
	}
	return pTexture;
}

void CImageFileManager::ReleaseTexture( LPCTSTR szFileName )
{
	CFileCacheEntry* pEntry;
	LPDIRECT3DTEXTURE8 pTexture = NULL;
	int iRefCount;

	pEntry = GetFileEntry(szFileName);
	if ( pEntry )
	{
		if ( pEntry->Ready() )
		{
			pEntry->GetTexture(&pTexture);
			if ( pTexture )
			{
				iRefCount = pTexture->Release();
				if ( iRefCount < 2 )
				{
					pEntry->ReleaseTexture();
					// Requeue file
					QueueLoad(szFileName, false );
				}
			}
		}
	}
}

int GetResourceFileEntryType( LPCTSTR szName )
{
	int iReturn = -1;

	if ( _tcsicmp( szName, _T("file") ) == 0 )
	{
		iReturn = IRE_STATIC;
	}
	else if ( _tcsicmp( szName, _T("entrynamed") ) == 0 )
	{
		iReturn = IRE_ENTRYNAMED;
	}
	else if ( _tcsicmp( szName, _T("entry") ) == 0 )
	{
		iReturn = IRE_ENTRY;
	}
	else if ( _tcsicmp( szName, _T("entrybasename") ) == 0 )
	{
		iReturn = IRE_ENTRYBASE;
	}
	else if ( _tcsicmp( szName, _T("entryindex") ) == 0 )
	{
		iReturn = IRE_ENTRYBASEIDX;
	}
	else if ( _tcsicmp( szName, _T("menunamed") ) == 0 )
	{
		iReturn = IRE_MENUNAMED;
	}
	else if ( _tcsicmp( szName, _T("menu") ) == 0 )
	{
		iReturn = IRE_MENU;
	}
	else if ( _tcsicmp( szName, _T("menubasename") ) == 0 )
	{
		iReturn = IRE_MENUBASE;
	}
	else if ( _tcsicmp( szName, _T("menuindex") ) == 0 )
	{
		iReturn = IRE_MENUBASEIDX;
	}
	else if ( _tcsicmp( szName, _T("filebasename") ) == 0 )
	{
		iReturn = IRE_STATICBASE;
	}
	else if ( _tcsicmp( szName, _T("fileindex") ) == 0 )
	{
		iReturn = IRE_STATICBASEIDX;
	}
	return iReturn;
}

int	GetResourceEntryCount( CXMLNode * pNode )
{
	int iReturn = 0;
	int iIndex = 0;
	CXMLElement * pElement;

	while( pElement = pNode->GetElement(iIndex++) )
	{
		if ( GetResourceFileEntryType( pElement->m_sName ) != (-1) )
		{
			iReturn++;
		}
	}
	return iReturn;
}

bool GetResourceEntryData( int iIndex, CXMLNode * pNode, CStdString & sName, CStdString & sValue )
{
	bool bReturn = false;
	int iElementIndex = 0;
	CXMLElement * pElement;

	while( pElement = pNode->GetElement(iElementIndex++) )
	{
		if ( GetResourceFileEntryType( pElement->m_sName ) != (-1) )
		{
			if ( iIndex )
			{
				iIndex--;
			}
			else
			{
				sName = pElement->m_sName;
				sValue = pElement->m_sValue;
				bReturn = true;
				break;
			}
		}
	}

	return bReturn;
}

void CImageResourceEntry::Configure(CXMLNode * pNode, LPCTSTR szDirectory )
{
	CStdString sValue;
	CStdString sString;
	int iMultiIndex = 0;
	CStdString sType;
//	int iType;


	if ( pNode )
	{
		if ( pNode->GetBool( NULL, _T("randomsource"), FALSE ) )
		{
			int iMaxEntries = GetResourceEntryCount(pNode);
			// Pick based on random number....
			if ( g_MenuInfo.m_iMasterRandomIndex == -1 )
			{
				iMultiIndex = RandomNum()%iMaxEntries;
			}
			else
			{
				iMultiIndex = (g_MenuInfo.m_iMasterRandomIndex%iMaxEntries);
			}
		}

		if ( GetResourceEntryData( iMultiIndex, pNode, sType, sValue ) )
		{
			m_iType = GetResourceFileEntryType( sType );
			if ( m_iType == IRE_STATIC )
			{
				sString = MakeFullFilePath( szDirectory, sValue );
			}
			else
			{
				sString = sValue;
			}
		}
		else
		{

			sValue = pNode->GetString( NULL, _T("file"), _T("") );
			if ( sValue.GetLength() )
			{
				if ( szDirectory && _tcslen( szDirectory ) )
				{
					sString = MakeFullFilePath( szDirectory, sValue );
				}
				else
				{
					sString = sValue;
				}
				DEBUG_FORMAT( _T("IMAGE RESOURCE: \"%s\""), sString.c_str() );
				m_iType = IRE_STATIC;
			}
			if ( sString.GetLength() == 0 )
			{
				sValue = pNode->GetString( NULL, _T("entrynamed"), _T("") );
				if ( sValue.GetLength() )
				{
					sString = sValue;
					m_iType = IRE_ENTRYNAMED;
				}
			}
			if ( sString.GetLength() == 0 )
			{
				sValue = pNode->GetString( NULL, _T("entry"), _T("") );
				if ( sValue.GetLength() )
				{
					sString = sValue;
					m_iType = IRE_ENTRY;
				}
			}
			if ( sString.GetLength() == 0 )
			{
				sValue = pNode->GetString( NULL, _T("entrybasename"), _T("") );
				if ( sValue.GetLength() )
				{
					sString = sValue;
					m_iType = IRE_ENTRYBASE;
				}
			}
			if ( sString.GetLength() == 0 )
			{
				sValue = pNode->GetString( NULL, _T("entryindex"), _T("") );
				if ( sValue.GetLength() )
				{
					sString = sValue;
					m_iType = IRE_ENTRYBASEIDX;
				}
			}

			if ( sString.GetLength() == 0 )
			{
				sValue = pNode->GetString( NULL, _T("menunamed"), _T("") );
				if ( sValue.GetLength() )
				{
					sString = sValue;
					m_iType = IRE_MENUNAMED;
				}
			}
			if ( sString.GetLength() == 0 )
			{
				sValue = pNode->GetString( NULL, _T("menu"), _T("") );
				if ( sValue.GetLength() )
				{
					sString = sValue;
					m_iType = IRE_MENU;
				}
			}
			if ( sString.GetLength() == 0 )
			{
				sValue = pNode->GetString( NULL, _T("menubasename"), _T("") );
				if ( sValue.GetLength() )
				{
					sString = sValue;
					m_iType = IRE_MENUBASE;
				}
			}
			if ( sString.GetLength() == 0 )
			{
				sValue = pNode->GetString( NULL, _T("menuindex"), _T("") );
				if ( sValue.GetLength() )
				{
					sString = sValue;
					m_iType = IRE_MENUBASEIDX;
				}
			}

			if ( sString.GetLength() == 0 )
			{
				sValue = pNode->GetString( NULL, _T("filebasename"), _T("") );
				if ( sValue.GetLength() )
				{
					sString = sValue;
					m_iType = IRE_STATICBASE;
				}
			}
			if ( sString.GetLength() == 0 )
			{
				sValue = pNode->GetString( NULL, _T("fileindex"), _T("") );
				if ( sValue.GetLength() )
				{
					sString = sValue;
					m_iType = IRE_STATICBASEIDX;
				}
			}
		}
		if ( sString.GetLength() )
		{
			m_sString = sString;
		}
	}
}

CImageResourceEntry::CImageResourceEntry(CXMLNode * pNode, LPCTSTR szDirectory )
{
	if ( pNode )
	{
		Configure( pNode, szDirectory );
	}
}

CImageResourceEntry::CImageResourceEntry(const CImageResourceEntry & src)
{
	m_iType = src.m_iType;
	m_sString = src.m_sString;
}

bool CImageResourceEntry::IsDynamic( void )
{
	return (IsScreenDynamic()||IsEntryDynamic());
}

bool CImageResourceEntry::IsEntryDynamic( void )
{
	bool bReturn = false;

	switch( m_iType )
	{
		case IRE_ENTRY:
		case IRE_ENTRYBASE:
		case IRE_ENTRYBASEIDX:
		case IRE_ENTRYNAMED:
		case IRE_MENU:
		case IRE_MENUBASE:
		case IRE_MENUBASEIDX:
		case IRE_MENUNAMED:
			bReturn = true;
			break;
	}
	return bReturn;
}

bool CImageResourceEntry::IsScreenDynamic( void )
{
	bool bReturn = false;

	switch( m_iType )
	{
		case IRE_STATICBASEIDX:
		case IRE_ENTRYBASEIDX:
		case IRE_MENUBASEIDX:
			bReturn = true;
			break;
	}
	return bReturn;
}


CStdString	CImageResourceEntry::GetFilename( int iItemNumber, int iScreenNumber )
{
	CStdString sReturn;
	CStdString sTemp;
	CStdString sEntryDir;
	CItemInfo * pEntry;
	CItemInfo * pMenuEntry;

	if ( iItemNumber < 0 )
	{
		iItemNumber = g_MenuInfo.m_iCurrentItem;
	}
	if ( iScreenNumber < 0 )
	{
		iScreenNumber = g_MenuInfo.m_iScreenIndex;
	}
/*
 <EntryNamed>Media</EntryNamed>
 <EntryBaseName>MXM_SS</EntryBaseName>
 <EntryIndex>MXM_HLP%02d</EntryIndex>
 <File>MenuBack.jpg</File>
 <FileIndex>filepath%02d</FileIndex>
 <FileBaseName>MenuBack</FileBaseName>
*/
	pEntry = g_MenuInfo.Entry(iItemNumber);
	pMenuEntry = g_MenuInfo.MenuEntry();
	switch( m_iType )
	{
		case IRE_STATIC:
			sReturn = MakeFullFilePath( g_MenuInfo.m_sMXMPath, m_sString );
			break;
		case IRE_STATICBASE:
			sReturn = FindImageFile( g_MenuInfo.m_sMXMPath, m_sString );
			break;
		case IRE_STATICBASEIDX:
			sTemp.Format( m_sString, iScreenNumber );
			sReturn = FindImageFile( g_MenuInfo.m_sMXMPath, sTemp );
			break;
		case IRE_ENTRY:
			if ( pEntry )
			{
				sEntryDir = pEntry->GetValue(szII_Dir);
				sReturn = MakeFullFilePath( sEntryDir, m_sString );
			}
			break;
		case IRE_ENTRYNAMED:
			if ( pEntry )
			{
				sEntryDir = pEntry->GetValue(szII_Dir);
				sReturn = MakeFullFilePath( sEntryDir, pEntry->GetValue( m_sString ) );
			}
			break;
		case IRE_ENTRYBASE:
			if ( pEntry )
			{
				sEntryDir = pEntry->GetValue(szII_Dir);
				sReturn = FindImageFile( sEntryDir, m_sString );
			}
			break;
		case IRE_ENTRYBASEIDX:
			sTemp.Format( m_sString, iScreenNumber );
			if ( pEntry )
			{
				sEntryDir = pEntry->GetValue(szII_Dir);
				sReturn = FindImageFile( sEntryDir, sTemp );
			}
			break;
		case IRE_MENU:
			if ( pEntry )
			{
				sEntryDir = pMenuEntry->GetValue(szII_Dir);
				sReturn = MakeFullFilePath( sEntryDir, m_sString );
			}
			break;
		case IRE_MENUNAMED:
			if ( pEntry )
			{
				sEntryDir = pMenuEntry->GetValue(szII_Dir);
				sReturn = MakeFullFilePath( sEntryDir, pMenuEntry->GetValue( m_sString ) );
			}
			break;
		case IRE_MENUBASE:
			if ( pEntry )
			{
				sEntryDir = pMenuEntry->GetValue(szII_Dir);
				sReturn = FindImageFile( sEntryDir, m_sString );
			}
			break;
		case IRE_MENUBASEIDX:
			sTemp.Format( m_sString, iScreenNumber );
			if ( pEntry )
			{
				sEntryDir = pMenuEntry->GetValue(szII_Dir);
				sReturn = FindImageFile( sEntryDir, sTemp );
			}
			break;
	}
	return sReturn;
}

