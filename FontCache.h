/*********************************************************************************\
 * FontCache.h
 * Class used to hold fonts
 * (C) 2002 MasterMind
\*********************************************************************************/

#include <xtl.h>
#include "xmltree.h"
#include "StdString.h"
#include "XBFont_mxm.h"

#ifndef __FONTCACHE_H__
#define __FONTCACHE_H__


class CFontEntry
{
public:
	int					m_iRefCount;
	CXBFont *			m_pFont;
	TListStrings		m_slNames;
	CStdString			m_sFilename;
	CFontEntry();
	CFontEntry(const CFontEntry &src);
	CFontEntry( LPDIRECT3DDEVICE8 pDevice, LPCTSTR szFilename, LPCTSTR szName );
	CFontEntry( LPDIRECT3DDEVICE8 pDevice, LPCTSTR szSectionName, const LPBYTE pbyBuffer, LPCTSTR szName );
	~CFontEntry();
	CFontEntry * GetPtr(void);
	HRESULT LoadFont( LPDIRECT3DDEVICE8 pDevice, LPCTSTR szFilename );
	HRESULT LoadFontMem( LPDIRECT3DDEVICE8 pDevice, LPCTSTR szSectionName, const LPBYTE pbyBuffer );
	void	AddName( LPCTSTR szName );
	bool	CheckName( LPCTSTR szName );
	int		AddRef();
	int		Release();
};

typedef list<CFontEntry> TListFontEntries;

class CFontCache
{
	LPDIRECT3DDEVICE8	m_pDevice;
	TListFontEntries	m_flEntries;
public:
	CFontCache();
	~CFontCache();
	void SetDevice( LPDIRECT3DDEVICE8 pDevice );
	CFontEntry * GetFontEntryByResname( LPCTSTR szName );
	CFontEntry * GetFontEntryByFilename( LPCTSTR szFilename );
	CXBFont * AddMemFont( LPCTSTR szSectionName, const LPBYTE pbyBuffer, LPCTSTR szName );
	CXBFont * GetFontPtrByFilename( LPCTSTR szFilename, LPCTSTR szName );
	CXBFont * GetFontPtrByResname( LPCTSTR szName );
	void ReleaseFontByFilename( LPCTSTR szFilename );
	void ReleaseFontByResname( LPCTSTR szName );
};


#endif //  __FONTCACHE_H__
