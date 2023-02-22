/*********************************************************************************\
 * ItemInfo.h
 * Class for handling Menu Item information
 * (C) 2002 MasterMind
\*********************************************************************************/

#ifndef __ITEMINFO_H__
#define __ITEMINFO_H__

#include "MXM_Defs.h"
#include "xmltree.h"
#include "ImgFileCache.h"
#include "CherryPatch.h"


using namespace std;

const LPCTSTR szII_XbeCurPath = _T("XbeCurPath");
const LPCTSTR szII_Title = _T("Title");
const LPCTSTR szII_Dir = _T("Dir");
const LPCTSTR szII_Exe = _T("Exe");
const LPCTSTR szII_Media = _T("Media");
const LPCTSTR szII_LaunchData = _T("LaunchData");
const LPCTSTR szII_Descr = _T("Descr");
const LPCTSTR szII_Passcode = _T("Passcode");
const LPCTSTR szII_EncPasscode = _T("EncPasscode");
const LPCTSTR szII_PasscodeMD5 = _T("PasscodeMD5");
const LPCTSTR szII_Thumbnail = _T("Thumbnail");


class CItemInfo
{
	TMapStrings	m_mapValues;
public:
	int				m_iVideoMode;
//	bool			m_bNTSCMode;
	CItemInfo();
	CItemInfo( const CItemInfo &src );
	bool operator<(CItemInfo &src) 
	{
		return GetValue(szII_Title) < src.GetValue(szII_Title);
	};
	bool operator>(CItemInfo &src) 
	{
		return GetValue(szII_Title) > src.GetValue(szII_Title);
	};
	CItemInfo &operator=(const CItemInfo &src)
	{
		Clear();
		m_mapValues = src.m_mapValues;
		m_pMenuNode = src.m_pMenuNode;
		m_bIsAction = src.m_bIsAction;
		m_sAction = src.m_sAction;
//		m_iTitleWidth = src.m_iTitleWidth;
//		m_iDescrWidth = src.m_iDescrWidth;
		m_iVideoMode = src.m_iVideoMode;
		m_iCountryAction	= src.m_iCountryAction;
//		m_bNTSCMode	= src.m_bNTSCMode;
		m_sPasscode = src.m_sPasscode;
		m_bPasscodeOK = src.m_bPasscodeOK;
		m_bFileExists = src.m_bFileExists;
		m_bIsMenu = src.m_bIsMenu;
	};
	void Clear();

	CXMLNode	*	BuildNode( CXMLNode * pParentNode );

	CStdString		GetValue( LPCTSTR szKey );
	CStdStringW		GetValueW( LPCTSTR szKey );
	void			SetValue( LPCTSTR szKey, LPCTSTR szValue );
	void			SetValue( CStdString & sKey, CStdString & sValue );
	CXMLNode	*	m_pMenuNode;

//	int				m_iTitleWidth;
//	int				m_iDescrWidth;
	CStdString		m_sAction;
	CStdString		m_sPassMD5;
	CStdString		m_sPasscode;
	bool			m_bPasscodeOK;
	bool			m_bFileExists;
	int				m_iCountryAction;
//	int				m_iVideoAction;
	bool			m_bIsMenu;
	bool			m_bIsAction;
	CFileCacheEntry* m_pThumbEntry;
	bool			TryPasscode( LPCTSTR szPasscode );
	bool			CanRun( void );
	bool			HasPassed( void );
};

typedef list<CItemInfo> TItemList;


#endif //  __ITEMINFO_H__