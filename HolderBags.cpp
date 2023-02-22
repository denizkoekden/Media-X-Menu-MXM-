/*********************************************************************************\
 * HolderBags.h
 * Class used to hold images and fonts
 * (C) 2002 MasterMind
\*********************************************************************************/


#include <XTL.h>
#include <WinNT.h>
#include <XBUtil.h>
#include <XGraphics.h>
#include "StdString.h"
#include "ImageSource.h"
#include "utils.h"
#include "HolderBags.h"


void CImageBag::DeleteImages( void )
{
	TMapImages::iterator itImages;

	itImages = m_mpiImages.begin();
	while ( itImages != m_mpiImages.end() )
	{
		delete itImages->second;
		itImages->second = NULL;
	}
	m_mpiImages.clear();
}

CImageBag::~CImageBag()
{
	DeleteImages();
}

int	CImageBag::AddImage( LPCTSTR szName, CImageSource * pImage )
{
	int iID = -1;
	if ( szName && pImage )
	{
		iID = GetID( szName );
		CStdString sName(szName);

		if ( iID == -1 )
		{
			iID = m_iNextID;
			m_iNextID++;
			m_mnNames.insert( TMapNames::value_type( iID, sName ));
			m_mpiImages.insert( TMapImages::value_type( iID, pImage ));
		}
	}
	return iID;
}

/*
int	CImageBag::LoadImage( LPDIRECT3DDEVICE8 pDevice, LPCTSTR szName, LPCTSTR szFilename )
{
	int iID = -1;

	if ( szFilename && szName && pDevice )
	{
		// Have we already loaded a font with this "name"?
		// If so, just return the ID
		iID = GetID( szName );
		if ( iID == -1 )
		{
			CImageSource * pImage;
			pImage = new CImageSource;

			if ( pImage )
			{
				if( SUCCEEDED( pImage->Create( pDevice, szFilename ) ) )
				{
					iID = AddImage( szName, pImage );
					if ( iID == -1 )
					{
						delete pImage;
					}
				}
				else
				{
					delete pImage;
				}
			}
		}
	}
	return iID;
}
*/

CImageSource * CImageBag::GetImage( int iID )
{
	TMapImages::iterator itImages;
	CImageSource *pImage = NULL;

	itImages = m_mpiImages.find( iID );
	if ( itImages != m_mpiImages.end() )
	{
		pImage = itImages->second;
	}
	return pImage;
}

CStdString	CImageBag::GetName( int iID )
{
	CStdString sReturn;

	TMapNames::iterator itNames;

	itNames = m_mnNames.find(iID);
	if ( itNames != m_mnNames.end() )
	{
		sReturn = itNames->second;
	}
	return sReturn;
}

CImageSource *	CImageBag::GetImage( LPCTSTR szName )
{
	int iID = GetID( szName );
	CImageSource *pImage = NULL;

	if ( iID != -1 )
	{
		pImage = GetImage( iID );
	}
	return pImage;
}

int CImageBag::GetID( LPCTSTR szName )
{
	TMapNames::iterator itNames;
	CStdString sName(szName);
	int iID = -1;

	if ( szName )
	{
		itNames = m_mnNames.begin();
		while( itNames != m_mnNames.end() )
		{
			if ( sName.Compare(itNames->second) == 0 )
			{
				iID = itNames->first;
				break;
			}
			itNames++;
		}
	}
	return iID;
}



void CFontBag::DeleteFonts( void )
{
	TMapFonts::iterator itFonts;

	itFonts = m_mpfFonts.begin();
	while ( itFonts != m_mpfFonts.end() )
	{
		delete itFonts->second;
		itFonts->second = NULL;
	}
	m_mpfFonts.clear();
}

CFontBag::~CFontBag()
{
	DeleteFonts();
}

int	CFontBag::AddFont( LPCTSTR szName, CXBFont * pFont )
{
	int iID = -1;
	if ( szName && pFont )
	{
		iID = GetID( szName );
		CStdString sName(szName);

		if ( iID == -1 )
		{
			iID = m_iNextID;
			m_iNextID++;
			m_mnNames.insert( TMapNames::value_type( iID, sName ));
		}
	}
	return iID;
}

int	CFontBag::LoadFont( LPDIRECT3DDEVICE8 pDevice, LPCTSTR szName, LPCTSTR szFilename )
{
	int iID = -1;

	if ( szFilename && szName && pDevice )
	{
		// Have we already loaded a font with this "name"?
		// If so, just return the ID
		iID = GetID( szName );
		if ( iID == -1 )
		{
			CXBFont * pFont;
			pFont = new CXBFont;

			if ( pFont )
			{
				if( SUCCEEDED( pFont->Create( pDevice, szFilename ) ) )
				{
					iID = AddFont( szName, pFont );
					if ( iID == -1 )
					{
						delete pFont;
					}
				}
				else
				{
					delete pFont;
				}
			}
		}
	}
	return iID;
}


CXBFont * CFontBag::GetFont( int iID )
{
	TMapFonts::iterator itFonts;
	CXBFont *pFont = NULL;

	itFonts = m_mpfFonts.find( iID );
	if ( itFonts != m_mpfFonts.end() )
	{
		pFont = itFonts->second;
	}
	return pFont;
}

CStdString	CFontBag::GetName( int iID )
{
	CStdString sReturn;

	TMapNames::iterator itNames;

	itNames = m_mnNames.find(iID);
	if ( itNames != m_mnNames.end() )
	{
		sReturn = itNames->second;
	}
	return sReturn;
}

CXBFont *	CFontBag::GetFont( LPCTSTR szName )
{
	int iID = GetID( szName );
	CXBFont *pFont = NULL;

	if ( iID != -1 )
	{
		pFont = GetFont( iID );
	}
	return pFont;
}

int CFontBag::GetID( LPCTSTR szName )
{
	TMapNames::iterator itNames;
	CStdString sName(szName);
	int iID = -1;

	if ( szName )
	{
		itNames = m_mnNames.begin();
		while( itNames != m_mnNames.end() )
		{
			if ( sName.Compare(itNames->second) == 0 )
			{
				iID = itNames->first;
				break;
			}
			itNames++;
		}
	}
	return iID;
}
