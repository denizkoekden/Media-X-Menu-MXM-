/*********************************************************************************\
 * XBIniCfgAccess.cpp
 * Xbox-specific ini file access routines
 * (C) 2002 MasterMind
\*********************************************************************************/

#include "XBApp_mxm.h"
#include "XBFont_mxm.h"
#include <XBMesh.h>
#include <XBUtil.h>
#include "XBResource_mxm.h"
#include <xgraphics.h>
#include "utils.h"

#include "XBIniCfgAccess.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



CXBIniCfgAccess::CXBIniCfgAccess( LPCTSTR szFileName, BOOL bDebug ) :
	CRawCfgAccess(szFileName)
{
	if (m_szName)
	{
		Load(m_szName, bDebug);
	}
}


int	CXBIniCfgAccess::Load( LPCTSTR szName, BOOL bDebug )
{
	int iReturn = -1;
	CStdString sReturn;
	CStdString sSection;
	CStdString sIniLine;
	CStdString sKey;
	CStdString sValue;
	HANDLE hFile;
	CStdString sFileName;
	char * szLineBuffer;
	int iPos;

	szLineBuffer = NULL;
	szLineBuffer = new char [1025];

	if ( szLineBuffer )
	{
		if ( szName && _tcslen(szName ) )
		{
			sFileName = szName;
		}
		else
		{
			sFileName = m_szName;
		}

		if ( sFileName.GetLength() )
		{
			hFile = CreateFile( sFileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if ( hFile != INVALID_HANDLE_VALUE  )
			{
				while( ReadFileLine( hFile, szLineBuffer, 1024 ) != -1 )
				{
					// Is this a section name, key value or comment?
					sIniLine = szLineBuffer;
					if ( sIniLine.GetLength() )
					{
						if ( sIniLine[0] == _T('[') )
						{
							// Section name....
							sValue = sIniLine.Mid(1);
							iPos = sValue.ReverseFind(_T("]"));
							if ( iPos > 0 )
							{
								sSection = sValue.Left( iPos );
								// Delete the section first, so we start fresh
								SetString( sSection, NULL, NULL );
							}
						}
						else if ( sIniLine[0] == _T(';') )
						{
							// Comment line... ignore...
						}
						else if ( (iPos = sIniLine.Find( _T('=') )) >= 0 )
						{
							sKey = sIniLine.Left( iPos );
							sValue = sIniLine.Mid( iPos+1 );
							if( sKey.GetLength() && sValue.GetLength() && sSection.GetLength() )
							{
								SetString( sSection, sKey, sValue );
							}
							else if ( sKey.GetLength() && (sValue.GetLength()==0) && sSection.GetLength() )
							{
								SetString( sSection, sKey, _T("") );
							}
						}
						else
						{
							// Ignore...
						}
					}
				}
				CloseHandle( hFile );
			}
			else
			{
	//			SetString( "LOAD", "RESULT", "FAILED" );
			}
			m_szName = sFileName;

		}
		delete [] szLineBuffer;
	}

	return iReturn;
}

int	CXBIniCfgAccess::Save( LPCTSTR szName )
{
	int iReturn = -1;
	HANDLE hFile;
	CStdString sFileName;
	int iCnt, iInnerIndex, iIndex, iKeyCnt;
	CStdString sSection, sKey, sValue, sData;

	if ( szName && _tcslen(szName ) )
	{
		sFileName = szName;
	}
	else
	{
		sFileName = m_szName;
	}

	if ( sFileName.GetLength() )
	{
		hFile = CreateFile( sFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hFile != INVALID_HANDLE_VALUE )
		{


			iCnt = GetNumberSections();
			for( iIndex=0; iIndex<iCnt; iIndex++ )
			{
				sSection = GetSectionName( iIndex );
				iKeyCnt = GetNumberKeys( sSection );

				sValue.Format( "[%s]", sSection.c_str() );
				WriteFileLine( hFile, sValue.c_str() );

				for( iInnerIndex=0; iInnerIndex<iKeyCnt; iInnerIndex++ )
				{
					sKey = GetKeyName( sSection, iInnerIndex );
					sValue = GetString( sSection, sKey, "" );
					if ( sValue.GetLength() )
					{
						sData.Format( "%s=%s", sKey.c_str(), sValue.c_str() );
						WriteFileLine( hFile, sData.c_str() );
					}
				}
			}
			CloseHandle( hFile );
		}
		
	}
	return iReturn;
}

