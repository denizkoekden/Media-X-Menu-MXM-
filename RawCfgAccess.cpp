/*********************************************************************************\
 * RawCfgAccess.cpp
 * Class used to abstract access to ini/registry and xml configuration items
 * (C) 2002 MasterMind
\*********************************************************************************/

#include "XBApp_mxm.h"
#include "XBFont_mxm.h"
#include <XBMesh.h>
#include <XBUtil.h>
#include "XBResource_mxm.h"
#include <xgraphics.h>

#include "utils.h"

#include "RawCfgAccess.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



void CRawCfgAccess::Clear( void )
{
	m_mapSections.Clear();
}

CRawCfgAccess::CRawCfgAccess( LPCTSTR szName )
{
	m_szName = szName;
}


int		CRawCfgAccess::Load( LPCTSTR szName )
{
	int iReturn = 0;


	return iReturn;
}

int		CRawCfgAccess::Save( LPCTSTR szName )
{
	int iReturn = 0;


	return iReturn;
}

int		CRawCfgAccess::GetNumberSections( void )
{
	return m_mapSections.GetNumberSections();
}

int		CRawCfgAccess::GetNumberKeys( LPCTSTR szSection )
{
	CStdString sSection(szSection);
	sSection.MakeLower();
	return m_mapSections.GetNumberKeys(sSection);
}

CStdString	CRawCfgAccess::GetSectionName( int iPos )
{
	return m_mapSections.GetSectionName(iPos);
}

CStdString	CRawCfgAccess::GetKeyName( LPCTSTR szSection, int iPos )
{
	CStdString sSection(szSection);
	sSection.MakeLower();
	return m_mapSections.GetKeyName(sSection, iPos);
}


CStdString CRawCfgAccess::GetString( LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szDefault )
{
	CStdString sReturn;
	CStdString sKey(szKey), sSection(szSection);

	sSection.MakeLower();
	sKey.MakeLower();

	sReturn = m_mapSections.GetString( sSection, sKey, szDefault );

	return sReturn;
}

BOOL	CRawCfgAccess::GetBool( LPCTSTR szSection, LPCTSTR szKey, BOOL bDefault )
{
	BOOL bReturn = bDefault;
	CStdString sValue;
	CStdString sKey(szKey), sSection(szSection);

	sSection.MakeLower();
	sKey.MakeLower();

	sValue = m_mapSections.GetString( sSection, sKey, _T("") );
	if ( sValue.GetLength() )
	{
		sValue.TrimLeft();
		sValue.TrimRight();
		sValue.MakeLower();
		if ( ( sValue.Compare( _T("on")) == 0 ) ||
			( sValue.Compare( _T("yes")) == 0 ) ||
			( sValue.Compare( _T("true")) == 0 ) )
		{
			bReturn = TRUE;
		}
		else if ( ( sValue.Compare( _T("off")) == 0 ) ||
			( sValue.Compare( _T("no")) == 0 ) ||
			( sValue.Compare( _T("false")) == 0 ) )
		{
			bReturn = FALSE;
		}
		else
		{
			if ( ConvertSigned(sValue) || ConvertUnsigned(sValue) )
			{
				bReturn = TRUE;
			}
			else
			{
				bReturn = FALSE;
			}
		}
	}

	return bReturn;
}

long	CRawCfgAccess::GetLong( LPCTSTR szSection, LPCTSTR szKey, long lDefault )
{
	long lReturn = lDefault;

	CStdString sValue;
	CStdString sKey(szKey), sSection(szSection);

	sSection.MakeLower();
	sKey.MakeLower();

	sValue = m_mapSections.GetString( sSection, sKey, _T("") );
	if ( sValue.GetLength() )
	{
		lReturn = ConvertSigned( sValue );
	}

	return lReturn;
}

DWORD	CRawCfgAccess::GetDWORD( LPCTSTR szSection, LPCTSTR szKey, DWORD dwDefault )
{
	DWORD dwReturn = dwDefault;

	CStdString sValue;
	CStdString sKey(szKey), sSection(szSection);

	sSection.MakeLower();
	sKey.MakeLower();

	sValue = m_mapSections.GetString( sSection, sKey, _T("") );
	if ( sValue.GetLength() )
	{
		dwReturn = ConvertUnsigned( sValue );
	}

	return dwReturn;
}

void	CRawCfgAccess::SetString( LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szValue )
{
	CStdString sKey(szKey), sSection(szSection);

	sSection.MakeLower();
	sKey.MakeLower();
	if ( szSection )
	{
		if ( szKey )
		{
			m_mapSections.SetString( sSection, sKey, szValue );
		}
		else
		{
			m_mapSections.SetString( sSection, NULL, szValue );
		}
	}
	else
	{
		if ( szKey )
		{
			m_mapSections.SetString( NULL, sKey, szValue );
		}
		else
		{
			m_mapSections.SetString( NULL, NULL, szValue );
		}
	}
}

void	CRawCfgAccess::SetBool( LPCTSTR szSection, LPCTSTR szKey, BOOL bValue )
{
	CStdString sKey(szKey), sSection(szSection);

	sSection.MakeLower();
	sKey.MakeLower();
	if ( szSection )
	{
		if ( szKey )
		{
			m_mapSections.SetString( sSection, sKey, bValue?_T("TRUE"):_T("FALSE") );
		}
		else
		{
			m_mapSections.SetString( sSection, NULL, bValue?_T("TRUE"):_T("FALSE") );
		}
	}
	else
	{
		if ( szKey )
		{
			m_mapSections.SetString( NULL, sKey, bValue?_T("TRUE"):_T("FALSE") );
		}
		else
		{
			m_mapSections.SetString( NULL, NULL, bValue?_T("TRUE"):_T("FALSE") );
		}
	}
}

void	CRawCfgAccess::SetLong( LPCTSTR szSection, LPCTSTR szKey, long lValue )
{
	CStdString sValue;

	sValue.Format( _T("%ld"), lValue );
	CStdString sKey(szKey), sSection(szSection);

	sSection.MakeLower();
	sKey.MakeLower();
	if ( szSection )
	{
		if ( szKey )
		{
			m_mapSections.SetString( sSection, sKey, sValue );
		}
		else
		{
			m_mapSections.SetString( sSection, NULL, sValue );
		}
	}
	else
	{
		if ( szKey )
		{
			m_mapSections.SetString( NULL, sKey, sValue );
		}
		else
		{
			m_mapSections.SetString( NULL, NULL, sValue );
		}
	}

}

void	CRawCfgAccess::SetDWORD( LPCTSTR szSection, LPCTSTR szKey, DWORD dwValue )
{
	CStdString sValue;

	sValue.Format( _T("%lu"), dwValue );
//	m_mapSections.SetString( szSection, szKey, sValue );
	CStdString sKey(szKey), sSection(szSection);

	sSection.MakeLower();
	sKey.MakeLower();
	if ( szSection )
	{
		if ( szKey )
		{
			m_mapSections.SetString( sSection, sKey, sValue );
		}
		else
		{
			m_mapSections.SetString( sSection, NULL, sValue );
		}
	}
	else
	{
		if ( szKey )
		{
			m_mapSections.SetString( NULL, sKey, sValue );
		}
		else
		{
			m_mapSections.SetString( NULL, NULL, sValue );
		}
	}

}
