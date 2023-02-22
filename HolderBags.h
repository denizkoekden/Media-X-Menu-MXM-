/*********************************************************************************\
 * HolderBags.h
 * Class used to hold images and fonts
 * (C) 2002 MasterMind
\*********************************************************************************/

#if !defined(__HOLDERBAGS_H__)
#define __HOLDERBAGS_H__

#include "MXM_Defs.h"

//#include <conio.h>


class CImageBag
{
private:
	int		m_iNextID;
	TMapNames	m_mnNames;
	TMapImages	m_mpiImages;
	void DeleteImages( void );
public:
	CImageBag() : m_iNextID(1)
	{
	};
	virtual ~CImageBag();
	int			GetID( LPCTSTR szName );
	CImageSource *	GetImage( int iID );
	CImageSource *	GetImage( LPCTSTR szName );
	CStdString	GetName( int iID );
	int			AddImage( LPCTSTR szName, CImageSource * pImage );
//	int			LoadImage( LPDIRECT3DDEVICE8 pDevice, LPCTSTR szName, LPCTSTR szFilename );

};


class CFontBag
{
private:
	int		m_iNextID;
	TMapNames	m_mnNames;
	TMapFonts	m_mpfFonts;
	void DeleteFonts( void );
public:
	CFontBag() : m_iNextID(1)
	{
	};
	virtual ~CFontBag();
	int			GetID( LPCTSTR szName );
	CXBFont *	GetFont( int iID );
	CXBFont *	GetFont( LPCTSTR szName );
	CStdString	GetName( int iID );
	int			AddFont( LPCTSTR szName, CXBFont * pFont );
	int			LoadFont( LPDIRECT3DDEVICE8 pDevice, LPCTSTR szName, LPCTSTR szFilename );

};















#endif // __HOLDERBAGS_H__

