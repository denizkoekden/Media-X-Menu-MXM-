/*********************************************************************************\
 * MappedAccess.cpp
 * Class used to provide mapped access to configuration items
 * (C) 2002 MasterMind
\*********************************************************************************/

#include "XBApp_mxm.h"
#include "XBFont_mxm.h"
#include <XBMesh.h>
#include <XBUtil.h>
#include "XBResource_mxm.h"
#include <xgraphics.h>
#include "MXM.h"
#include "MappedAccess.h"

#include "StdString.h"

#pragma warning(disable:4786)
#pragma warning(disable:4503)

#include <conio.h>

#include <iostream>
#include <string>
#include <map>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CMappedAccess::CMappedAccess()
{
}


void CMappedAccess::Clear( void )
{
	m_mapSections.clear();
}

void	CMappedAccess::SetString( LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szValue )
{
	TMapSections::iterator itSection;
	string sSection, sKey, sValue;
	TMapStrings::iterator itKey;


	if ( szSection )
	{
		sSection = szSection;
		itSection = m_mapSections.find( sSection );
		if ( itSection != m_mapSections.end() )
		{
			if ( szKey )
			{
				sKey = szKey;
				itKey = itSection->second.find( sKey );
				if ( itKey != itSection->second.end() )
				{
					if ( szValue )
					{
						sValue = szValue;
						itKey->second = sValue;
					}
					else
					{
						// Delete the key/value pair
						itSection->second.erase( sKey );
					}
				}
				else
				{
					// Add the key/value if value is valid
					if ( szValue )
					{
						sValue = szValue;
						itSection->second.insert( TMapStrings::value_type( sKey, sValue ));
					}
				}
			}
			else
			{
				// NULL key means want to delete the section
				m_mapSections.erase( sSection );
			}
		}
		else
		{
			// Didn't find this section....
			if ( szKey && szValue )
			{
				sKey = szKey;
				sValue = szValue;
				// Create new map, if we have valid key and value
				TMapStrings mapTmp;

				mapTmp.insert( TMapStrings::value_type( sKey, sValue ) );
				m_mapSections.insert( TMapSections::value_type( sSection, mapTmp ) );
			}
		}
	}
//	return sDefault.c_str();
}

int		CMappedAccess::GetNumberSections( void )
{
	return m_mapSections.size();
}


CStdString CMappedAccess::GetString( LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szDefault )
{
	TMapSections::iterator itSection;
	CStdString sSection, sKey, sDefault;
	TMapStrings::iterator itKey;
	CStdString sTemp;

	sSection = szSection;
	sKey = szKey;
	sDefault = szDefault;

	itSection = m_mapSections.find( sSection );
	if ( itSection != m_mapSections.end() )
	{
		itKey = itSection->second.find( sKey );
		if ( itKey != itSection->second.end() )
		{
			sDefault = itKey->second;
		}
	}
	return sDefault;
}


int		CMappedAccess::GetNumberKeys( LPCTSTR szSection )
{
	int iReturn = 0;
	TMapSections::iterator itSection;
	CStdString sSection;

	sSection = szSection;
	itSection = m_mapSections.find( sSection );
	if ( itSection != m_mapSections.end() )
	{
		iReturn = itSection->second.size();
	}
	return iReturn;
}

CStdString	CMappedAccess::GetSectionName( int iPos )
{
	TMapSections::iterator itSection;

	itSection = m_mapSections.begin();
	while( iPos && itSection != m_mapSections.end() )
	{
		iPos--;
		itSection++;
	}
	if ( (iPos == 0) && (itSection != m_mapSections.end()) )
	{
		return itSection->first.c_str();
	}
	else
	{
		return _T("");
	}
}

CStdString	CMappedAccess::GetKeyName( LPCTSTR szSection, int iPos )
{
	CStdString sReturn;
	TMapSections::iterator itSection;
	string sSection;
	TMapStrings::iterator itKey;

	sSection = szSection;
	itSection = m_mapSections.find( sSection );
	if ( itSection != m_mapSections.end() )
	{
		// now, iterate through this list....
		itKey = itSection->second.begin();
		while( iPos && ( itKey != itSection->second.end()))
		{
			iPos--;
			itKey++;
		}
		if( (iPos==0) && ( itKey != itSection->second.end()))
		{
			sReturn = itKey->first.c_str();
		}
	}
	return sReturn;
}

