/*********************************************************************************\
 * ImgFileCache.h
 * Class used to hold Img Files
 * (C) 2003 MasterMind
\*********************************************************************************/

#ifndef __IMGFILECACHE_H__
#define __IMGFILECACHE_H__

#include <xtl.h>
#include <string>
#include <map>
#include <list>
#include <XbSound.h>
#include "StdString.h"
#include "ImageSource.h"
#include "xmltree.h"
#include "xbfont_mxm.h"
#include "XBResource_mxm.h"

#define IRE_STATIC			0
#define IRE_STATICBASE		1
#define IRE_STATICBASEIDX	2
#define IRE_ENTRY			3
#define IRE_ENTRYNAMED		4
#define IRE_ENTRYBASE		5
#define IRE_ENTRYBASEIDX	6
#define IRE_MENU			7
#define IRE_MENUNAMED		8
#define IRE_MENUBASE		9
#define IRE_MENUBASEIDX		10

using namespace std;

typedef list<string> TStringList;

class CSoundResourceEntry
{
public:
	TStringList		m_slResNames;
	CStdString		m_sFileName;
	CXBSound		m_snd;
	CSoundResourceEntry * GetPtr( void ) { return this; };
};

typedef list<CSoundResourceEntry *> TListSoundResEntries;


class CSoundResourceCache
{
	TListSoundResEntries	m_slEntries;
public:
	CSoundResourceCache();
	~CSoundResourceCache();
	CXBSound * GetSound( LPCTSTR szName );
	CSoundResourceEntry * GetSoundEntryByFilename( LPCTSTR szName );
	void AddEntry( CXMLNode * pNode, LPCTSTR szDirectory = NULL );
	void AddEntry( LPCTSTR szName, LPCTSTR szFilename, LPCTSTR szDirectory = NULL );
	void Clear( void );
};

class CImageResourceEntry
{
	CStdString	m_sString;
	int			m_iType;
protected:
public:
	CImageResourceEntry() {};
	CImageResourceEntry(const CImageResourceEntry & src);
	CImageResourceEntry(CXMLNode * pNode, LPCTSTR szDirectory = NULL);
	~CImageResourceEntry() {};
	bool		IsDynamic( void );
	bool		IsEntryDynamic( void );
	bool		IsScreenDynamic( void );
	CStdString	GetFilename( int iItemNumber = -1, int iScreenNumber = -1 );
	void Configure(CXMLNode * pNode, LPCTSTR szDirectory = NULL);
};

typedef map<string, CImageResourceEntry*>	TMapImgResEntries;

class CImageResourceCache
{
	TMapImgResEntries m_miEntries;
public:
	CImageResourceEntry * GetEntry( LPCTSTR szName );
	void	AddEntry( CXMLNode * pNode, LPCTSTR szDirectory = NULL );
	CImageResourceCache();
	~CImageResourceCache();
	void Clear( void );
};


class CFileCacheEntry
{
	D3DXIMAGE_INFO		m_info;
	bool				m_bPNGGIF;
	bool				m_bMemImage;
	CStdString			m_sFilename;
	int					m_iID;
	DWORD				m_dwFileSize;
	LPBYTE				m_pbyBuffer;
	LPDIRECT3DTEXTURE8	m_pTexture;
	bool				m_bExists;
	static int			s_ID;
	DWORD				m_dwAlpha;
	int					m_iPriority;
	int					m_iCategory;
	bool				m_bIsXPR;
	DWORD				m_dwLoadTimeStamp;
	DWORD				m_dwTextureTimeStamp;
	static LPDIRECT3DDEVICE8	m_pd3dDevice;

	void	ValidateTypes( void );

	HRESULT BuildFromResource( CXBPackedResource & PackedResource );
public:
	CFileCacheEntry( const CFileCacheEntry & src );
	CFileCacheEntry(LPCTSTR szFilename, LPCTSTR szDirectory = NULL);
	CFileCacheEntry();
	~CFileCacheEntry();

	void				DumpBuffer( void );
	DWORD				GetLoadTimestamp( void )   { return m_dwLoadTimeStamp; };
	DWORD				GetTextureTimestamp( void ){ return m_dwTextureTimeStamp; };
	bool				Loaded( void );
	bool				Ready( void );
	bool				Exists( void ) { return m_bExists; };
	CFileCacheEntry *	GetPtr( void )			{ return this; };

	DWORD				GetSize( void ) { return m_dwFileSize; };
	LPBYTE				GetBuffer( void ) { return m_pbyBuffer; };
	CStdString			GetFilename( void )		{ return m_sFilename; };
	int					GetID( void )			{ return m_iID; };

	DWORD				GetAlpha( void ) { return m_dwAlpha; };
	void				SetAlpha( DWORD dwAlpha ) { m_dwAlpha = dwAlpha; };
	void				SetPriority( DWORD iPriority ) { m_iPriority = iPriority; };
	int					GetPriority( void ) { return m_iPriority; };
	void				SetCategory( DWORD iCategory ) { m_iCategory = iCategory; };
	int					GetCategory( void ) { return m_iCategory; };

	HRESULT				LoadFile( LPCTSTR szFilename = NULL, LPCTSTR szDirectory = NULL  );
	HRESULT				GetBuffer( LPBYTE * ppbyBuffer, DWORD * pdwSize );
	HRESULT				GetTexture( LPDIRECT3DTEXTURE8 * ppTexture, D3DXIMAGE_INFO ** ppInfo = NULL );
	HRESULT				ReleaseTexture( void );
};


typedef map<string, CFileCacheEntry*>	TMapFileEntries;

class CImageFileManager
{
	D3DXIMAGE_INFO			m_info;
	TStringList				m_sqLoad;
	TMapFileEntries			m_mfEntries;
    HANDLE                  m_hThread;                          // Worker thread
	HANDLE					m_hQueuedEvent;
	DWORD					m_dwBufferMax;
	bool					m_bThreadStarted;
public:
    static DWORD WINAPI		InitializeThreadProc(LPVOID lpParameter );
	DWORD					LoadAndPruneThread( void );
	bool					Prune( void );
	void					SetMaxBuffer( DWORD dwSize ) { m_dwBufferMax = dwSize; };
	void					StartThread( void );
	void					QueueLoad( LPCTSTR szFileName, bool bImmediate = false );
	LPDIRECT3DTEXTURE8		GetTexture( LPCTSTR szFileName, D3DXIMAGE_INFO ** ppInfo = NULL );
	void					ReleaseTexture( LPCTSTR szFileName );
	CFileCacheEntry*		FindOldestLoadedEntry( void );
	CFileCacheEntry*		GetFileEntry( LPCTSTR szFileName, LPCTSTR szDirectory = NULL );
	CFileCacheEntry*		AddFileEntry( LPCTSTR szFileName, LPCTSTR szDirectory = NULL );
	CImageFileManager();
	~CImageFileManager();
};


extern CImageFileManager g_FileManager;
extern CImageResourceCache g_ImgResCache;
extern CSoundResourceCache g_SndResCache;


#endif // __IMGFILECACHE_H__