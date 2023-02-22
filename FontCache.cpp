/*********************************************************************************\
 * FontCache.cpp
 * Class used to hold fonts
 * (C) 2003 MasterMind
\*********************************************************************************/

#include <xtl.h>
#include "FontCache.h"


CFontEntry::CFontEntry(const CFontEntry &src)
{
	m_iRefCount = src.m_iRefCount;
	m_pFont = src.m_pFont;
	m_slNames = src.m_slNames;
	m_sFilename = src.m_sFilename;
}


CFontEntry::CFontEntry() :
	m_pFont(NULL),
	m_iRefCount(0)
{
}

CFontEntry::CFontEntry( LPDIRECT3DDEVICE8 pDevice, LPCTSTR szFilename, LPCTSTR szName ) :
	m_pFont(NULL),
	m_iRefCount(0)
{
	if ( SUCCEEDED(LoadFont( pDevice, szFilename)) )
	{
		AddName( szName );
	}
}

CFontEntry::CFontEntry( LPDIRECT3DDEVICE8 pDevice, LPCTSTR szSectionName, const LPBYTE pbyBuffer, LPCTSTR szName ) :
	m_pFont(NULL),
	m_iRefCount(0)
{
	if ( SUCCEEDED(LoadFontMem( pDevice, szSectionName, pbyBuffer )) )
	{
		AddName( szName );
	}
}


CFontEntry::~CFontEntry()
{
}

CFontEntry * CFontEntry::GetPtr(void)
{
	return this;
}

int CFontEntry::AddRef()
{
	m_iRefCount++;
	return m_iRefCount;
}

int CFontEntry::Release()
{
	if ( m_iRefCount )
	{
		m_iRefCount--;
	}
	return m_iRefCount;
}

bool CFontEntry::CheckName( LPCTSTR szName )
{
	bool bReturn = false;
	CStdString sName;
	TListStrings::iterator iterName;

	if ( szName && _tcslen( szName ) )
	{
		sName = szName;
		sName.MakeLower();
		iterName = m_slNames.begin();
		while( iterName != m_slNames.end() )
		{
			if ( sName.Compare(iterName->c_str()) == 0 )
			{
				bReturn = true;
				break;
			}
			iterName++;
		}
	}
	
	return bReturn;
}

void CFontEntry::AddName( LPCTSTR szName )
{
	CStdString sName;
	if ( szName && _tcslen( szName ) )
	{
		sName = szName;
		sName.MakeLower();
		m_slNames.push_back( sName );
	}
}


HRESULT CFontEntry::LoadFontMem( LPDIRECT3DDEVICE8 pDevice, LPCTSTR szSectionName, const LPBYTE pbyBuffer )
{
	HRESULT hr = E_FAIL;
	if ( m_pFont == NULL )
	{
		m_pFont = new CXBFont();
		hr = m_pFont->CreateFromMem( pDevice, szSectionName, pbyBuffer );
	}
	return hr;
}


HRESULT CFontEntry::LoadFont( LPDIRECT3DDEVICE8 pDevice, LPCTSTR szFilename )
{
	HRESULT hr = E_FAIL;
	if ( m_pFont == NULL )
	{
		m_pFont = new CXBFont();
		hr = m_pFont->Create( pDevice, szFilename );
	}
	return hr;
}



CFontCache::CFontCache() :
	m_pDevice(NULL)
{
}

CFontCache::~CFontCache()
{
}

void CFontCache::SetDevice( LPDIRECT3DDEVICE8 pDevice )
{
	m_pDevice = pDevice;
}

CFontEntry * CFontCache::GetFontEntryByResname( LPCTSTR szName )
{
	CFontEntry *pReturn = NULL;
	TListFontEntries::iterator iterFonts;

	iterFonts = m_flEntries.begin();
	while( !pReturn && (iterFonts != m_flEntries.end()) )
	{
		if ( iterFonts->CheckName( szName ) )
		{
			pReturn = iterFonts->GetPtr();
		}
		iterFonts++;
	}
	return pReturn;
}

CFontEntry * CFontCache::GetFontEntryByFilename( LPCTSTR szFilename )
{
	CFontEntry *pReturn = NULL;
	TListFontEntries::iterator iterFonts;
	CStdString sFilename;
	CStdString sEntryFilename;

	if ( szFilename && _tcslen( szFilename ) )
	{
		sFilename = szFilename;
		sFilename.MakeLower();
		iterFonts = m_flEntries.begin();
		while( !pReturn && (iterFonts != m_flEntries.end()) )
		{
			sEntryFilename = iterFonts->m_sFilename;
			sEntryFilename.MakeLower();
			if ( sEntryFilename.Compare( sFilename ) == 0 )
			{
				pReturn = iterFonts->GetPtr();
			}
			iterFonts++;
		}
	}
	return pReturn;
}

CXBFont * CFontCache::AddMemFont( LPCTSTR szSectionName, const LPBYTE pbyBuffer, LPCTSTR szName )
{
	CXBFont *pReturn = NULL;
	CFontEntry * pEntry = NULL;

	if ( szName && _tcslen( szName ) )
	{
		pEntry = GetFontEntryByResname(szName);
		if ( !pEntry )
		{
			// Create a new font entry....
			m_flEntries.push_back( CFontEntry(m_pDevice, szSectionName, pbyBuffer, szName ) );
			pEntry = GetFontEntryByResname(szName);
		}
		if ( pEntry )
		{
			pReturn = pEntry->m_pFont;
		}
	}
	return pReturn;
}

CXBFont * CFontCache::GetFontPtrByFilename( LPCTSTR szFilename, LPCTSTR szName )
{
	CXBFont *pReturn = NULL;
	CFontEntry * pEntry = NULL;

	if ( szFilename && _tcslen( szFilename ) )
	{
		pEntry = GetFontEntryByFilename( szFilename );
		if ( pEntry )
		{
			pReturn = pEntry->m_pFont;
		}
		else
		{
			// Create new entry...?
			if ( szName && _tcslen( szName ) )
			{
				pEntry = GetFontEntryByResname(szName);
				if ( pEntry )
				{
					pReturn = pEntry->m_pFont;
				}
				else
				{
					// Not in cache, Now load one up.
					m_flEntries.push_back( CFontEntry(m_pDevice, szFilename , szName ) );
					pEntry = GetFontEntryByResname(szName);
				}
			}
			else
			{
				// Add and fetch by filename....
				m_flEntries.push_back( CFontEntry(m_pDevice, szFilename , szName ) );
				pEntry = GetFontEntryByFilename(szFilename);
			}
		}
	}
	else
	{
		if ( szName && _tcslen( szName ) )
		{
			pEntry = GetFontEntryByResname(szName);
		}
	}
	if ( pEntry )
	{
		pReturn = pEntry->m_pFont;
	}

	return pReturn;
}

CXBFont * CFontCache::GetFontPtrByResname( LPCTSTR szName )
{
	CXBFont *pReturn = NULL;
	CFontEntry * pEntry = NULL;

	if ( szName && _tcslen( szName ) )
	{
		pEntry = GetFontEntryByResname(szName);
	}
	if ( pEntry )
	{
		pReturn = pEntry->m_pFont;
	}

	return pReturn;
}

void CFontCache::ReleaseFontByFilename( LPCTSTR szFilename )
{
}

void CFontCache::ReleaseFontByResname( LPCTSTR szName )
{
}

