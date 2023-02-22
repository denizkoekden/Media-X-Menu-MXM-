/*********************************************************************************\
 * MenuInfo.cpp
 * Class for handling global information
 * (C) 2002 MasterMind
\*********************************************************************************/

#include <xtl.h>
#include "MXM.h"
#include "MenuInfo.h"
#include "ImgFileCache.h"
#include "utils.h"

#define CHECKTRAY_INTERVAL	100
#define REMOUNT_INTERVAL	200
#define CHECKDISK_INTERVAL	400

bool IsNTSC( void );

CStdString GetString( int iIndex, TListStrings & slStrings )
{
	TListStrings::iterator iterCurrent = slStrings.begin();
	CStdString sReturn;

	while( iterCurrent != slStrings.end() )
	{
		if ( iIndex > 0 )
		{
			iIndex--;
		}
		else
		{
			sReturn = iterCurrent->c_str();
			break;
		}
		iterCurrent++;
	}
	return sReturn;
}


CXboxInfo::CXboxInfo()
{
	m_iNumFTPUsers = 0;
	m_bTrayOpen = false;
	m_bTrayClosed = false;
	m_bTrayEmpty = true;
	m_bTrayWasClosed = false;
	m_iDVDType = -1;
	m_iPrevDVDType = -2;
	m_dwTimeStamp = GetTickCount()+CHECKTRAY_INTERVAL;
	m_dwRemountTimeStamp = 0;
	m_dwCheckDVDTimeStamp = GetTickCount()+CHECKTRAY_INTERVAL+CHECKDISK_INTERVAL+CHECKDISK_INTERVAL;
}

CXboxInfo::~CXboxInfo()
{
}


bool CXboxInfo::CheckState( void )
{
	bool bChangedState = false;
	WORD wTrayState;
	bool bRemountDVD = false;

	if ( m_dwTimeStamp < GetTickCount() )
	{

		if ( SUCCEEDED( HalReadSMBusValue(0x20, 0x03, 0, (LPBYTE)&wTrayState) ) )
		{
			if ( wTrayState&16 ) // Tray is open
			{
				m_iDVDType = XBI_DVD_EMPTY;
				if ( ( m_bTrayOpen == false ) || (m_bTrayClosed == true ) )
				{
					bChangedState = true;
				}
				m_bTrayWasClosed = m_bTrayClosed;
				m_bTrayOpen = true;
				m_bTrayClosed = false;
				m_bTrayEmpty = true;
			}
			else
			{
				if ( (wTrayState & 0x40) == 0x40 ) // Tray is closed
				{
					if ( !m_bTrayClosed )
					{
						if ( (wTrayState & 0x20) == 0x20 )
						{
							m_iDVDType = XBI_DVD_UNKNOWN;
							m_bTrayEmpty = false;
							bRemountDVD = true;
							bChangedState = true;
						}
						else
						{
							m_iDVDType = XBI_DVD_EMPTY;
							m_bTrayEmpty = true;
						}
						m_bTrayClosed = true;
					}
					else
					{
						if ( (wTrayState & 0x20) == 0x20 )
						{
							if ( m_bTrayEmpty )
							{
								m_iDVDType = XBI_DVD_UNKNOWN;
								m_bTrayEmpty = false;
								bRemountDVD = true;
								bChangedState = true;
							}
						}
						else
						{
							m_iDVDType = XBI_DVD_EMPTY;
							m_bTrayEmpty = true;
						}
					}
					m_bTrayWasClosed = m_bTrayClosed;
					if ( ( m_bTrayOpen == true ) || (m_bTrayClosed == false ) )
					{
						bChangedState = true;
					}
				}
				else // Tray is in init state
				{
					if ( ( m_bTrayOpen == true ) || (m_bTrayClosed == true ) )
					{
						bChangedState = true;
					}
					m_iDVDType = XBI_DVD_EMPTY;
					m_bTrayWasClosed = m_bTrayClosed;
					m_bTrayEmpty = true;
					m_bTrayClosed = false;
				}
				m_bTrayOpen = false;
			}
		}
		m_dwTimeStamp = GetTickCount()+CHECKTRAY_INTERVAL;
	}
	if ( m_bTrayClosed )
	{
		if ( bRemountDVD ) // m_dwRemountTimeStamp && (m_dwRemountTimeStamp < GetTickCount()) )
		{
			// m_dwRemountTimeStamp = 0;
			Remount("D:","CdRom0");
			m_dwCheckDVDTimeStamp = GetTickCount()+CHECKDISK_INTERVAL;
		}
		if ( m_dwCheckDVDTimeStamp && (m_dwCheckDVDTimeStamp < GetTickCount()) )
		{
			m_iDVDType = XBI_DVD_UNKNOWN;
			bChangedState = true;
			if ( FileExists( _T("D:\\default.xbe") ) )
			{
				m_iDVDType = XBI_DVD_XBOX;
			}
			else if ( FileExists( _T("D:\\VIDEO_TS\\VIDEO_TS.IFO") ) )
			{
				m_iDVDType = XBI_DVD_MOVIE;
			}
			m_dwCheckDVDTimeStamp = 0;
		}
	}
	else
	{
		m_dwCheckDVDTimeStamp = 0;
	}
	return bChangedState;
}

CMenuInfo::CMenuInfo( void ) :
	m_pSkinInfo(NULL),
	m_pCfgInfo(NULL),
	m_pPrefInfo(NULL),
	m_pCacheMenu(NULL),
	m_pInternalInfo(NULL),
	m_pMenuNode(NULL),
	m_pMenuCurrNode(NULL),
	m_bDashMode(false),
	m_bDebugMenu(false),
	m_bAutoRenameFTP(false),
	m_bDVDMode(false),
	m_iScreenIndex(0),
	m_bXDIMode(false),
	m_iIntroColor(0),
	m_bItemPrev(false),
	m_bItemNext(false),
	m_bSpecialLaunch(false),
	m_bRemoveDuplicates(TRUE),
	m_bUseMusicManager(TRUE),
	m_bRemoveInvalid(TRUE),
	m_bSafeMode(FALSE),
	m_bAllowFormat(FALSE),
	m_bUseCelsius(false),
	m_bUseEuroNumbers(false),
	m_iDateFormat(0),
	m_iTimeFormat(0),
	m_bHideSystemMenu(FALSE),
	m_bAutoActionMenu(FALSE),
	m_bDeepMultiSearch(false),
	m_bInActionScript(false),
	m_iDriveFOption(0),
	m_iDriveGOption(0),
	m_bAutoLoadMulti(true),
	m_bAutoLaunchGame(false),
	m_bAutoLaunchMovie(false),
	m_bAllowBetaSkins(TRUE),
	m_bAllowXDISkin(TRUE),
	m_bDoTimeServer(false),
	m_bXnaValid(false),
	m_bUseSkinMenuSounds(true),
	m_iAutoCfgStyle(0),
	m_bAutoCfgAddDash(true),
	m_bAutoCfgSort(true),
	m_bNetParamsSet(false),
	m_iFTPPriority(8),
	m_bEnableFTP(true),
	m_sTimeServerIP(_T(""))
{
	NetParams.m_iUseDHCP = -1;
	NetParams.m_sIP = _T("");
	NetParams.m_sSubnetmask = _T("");
	NetParams.m_sDefaultgateway = _T("");
	NetParams.m_sDNS1 = _T("");
	NetParams.m_sDNS2 = _T("");

	Overrides.m_iPreviewSuspendMusic = 0;
	Overrides.m_bForceGlobalMusic = false;
	Overrides.m_bForceLocalMusic = false;
	Overrides.m_bForceRandomMusic = false;

	SafeMode.m_bActive = false;
	SafeMode.m_dwFlags = 0;
	SafeMode.m_bSetUserSafeMode = false;
	InitializeCriticalSection( &SafeMode.m_csSafeMode );
}


void CMenuInfo::UpdateSafeMode( LPCTSTR szMsg )
{
	EnterCriticalSection( &SafeMode.m_csSafeMode );
	if ( SafeMode.m_bSetUserSafeMode )
	{
		SaveStringToFile( _T("Z:\\safemode.txt"), _T("000: User Initiated Safe Mode") );
		XLaunchNewImage( NULL, NULL );
	}
	else
	{
		if ( szMsg && _tcslen( szMsg ))
		{
			SaveStringToFile( _T("Z:\\safemode.txt"), szMsg );
		}
		else
		{
			DeleteFile(_T("Z:\\safemode.txt"));
		}
	}
	LeaveCriticalSection( &SafeMode.m_csSafeMode );
}

void CMenuInfo::SavePrefs( void )
{
	m_pPrefInfo->SaveNode( _T("U:\\pref.xml") );
}

void CMenuInfo::LoadPrefs( void )
{
	if ( FileExists(_T("U:\\pref.xml")) )
	{
		CXMLLoader xmlCfgLoader(NULL,true);

		m_pPrefInfo = xmlCfgLoader.LoadXMLNodes( _T("U:\\pref.xml") );
		xmlCfgLoader.DetachMainNode();
	}
	else
	{
		 m_pPrefInfo = new CXMLNode();
		 m_pPrefInfo->m_sName = _T("preferences");
	}
}


CXMLNode *		CMenuInfo::FindItem( DWORD dwItemID, CXMLNode * pNode )
{
	CXMLNode * pReturnNode = NULL;
	int iIndex = 0;
	CXMLNode * pItemNode = NULL;
	CXMLNode * pSubMenuNode = NULL;

	if ( pNode == NULL )
	{
		pNode = m_pMenuNode;
	}
	// Now we locate the menu from the main node on out.
	while( ( pReturnNode == NULL ) && ( pItemNode = pNode->GetNode( _T("item"), iIndex++) ) )
	{
		if ( dwItemID == pItemNode->GetLong( NULL, _T("xbeid"), 0 ) )
		{
			pReturnNode = pItemNode;
		}
	}
	iIndex = 0;
	while( ( pReturnNode == NULL ) && ( pSubMenuNode = pNode->GetNode( _T("submenu"), iIndex++) ) )
	{
		pReturnNode = FindItem( dwItemID, pSubMenuNode );
	}
	return pReturnNode;
}

// 0xffadf32f is the code for the DVD player
bool CMenuInfo::FindDVDPlayer( CXMLNode * pNode )
{
	bool bReturn = false;
	CXMLNode * pItemNode = NULL;

	if ( _tcsicmp( m_pPrefInfo->GetString( _T("dvddrive"), _T("PrefPlayer"), _T("") ), _T("M$") ) )
	{
		pItemNode = FindItem( 0xffadf32f, pNode );
		if ( pItemNode )
		{
			m_sDVDPlayer = pItemNode->GetString( NULL, _T("exe"), _T("") );
			if ( m_sDVDPlayer.GetLength() )
			{
				bReturn = true;
			}
		}
	}
	if ( !bReturn )
	{

		pItemNode = FindItem( 0xfffe0000, pNode );
		if ( pItemNode )
		{
			m_sDVDPlayer = pItemNode->GetString( NULL, _T("exe"), _T("") );
			if ( m_sDVDPlayer.GetLength() )
			{
				bReturn = true;
			}
		}
	}
	return bReturn;
}

bool CopyNodeToItemInfo( CXMLNode * pNode, CItemInfo * pItemInfo )
{
	bool bReturn = false;

	CStdString sNodeName;
	CStdString sTemp;

	if ( pItemInfo && pNode )
	{
		sNodeName = pNode->m_sName;
		sNodeName.MakeLower();
		pItemInfo->Clear();
		TXMLElementList::iterator iterElement;
		CXMLElement * pElement;
		TMapStrings::iterator iterAttrib;
			
		sTemp = pNode->GetString( NULL, _T("thumbnail"), _T("") );

		if ( sTemp.GetLength() )
		{
			pItemInfo->m_pThumbEntry = g_FileManager.AddFileEntry(sTemp);
		}

		sTemp = pNode->GetString( NULL, szII_PasscodeMD5, _T("") );
		if ( sTemp.GetLength() )
		{
			pItemInfo->m_sPassMD5 = sTemp;
		}

		sTemp = pNode->GetString( NULL, _T("videomode"), _T("") );
		if ( _tcsicmp( sTemp, _T("ntsc") ) == 0 )
		{
			pItemInfo->m_iVideoMode = 2;
			// pItemInfo->m_bNTSCMode = true; 
		}
		else if ( _tcsicmp( sTemp, _T("pal") ) == 0 )
		{
			pItemInfo->m_iVideoMode = 4;
			// pItemInfo->m_bNTSCMode = false; 
		}
		else if ( _tcsicmp( sTemp, _T("pal60") ) == 0 )
		{
			pItemInfo->m_iVideoMode = 5;
			// pItemInfo->m_bNTSCMode = false; 
		}
		else if ( _tcsicmp( sTemp, _T("ntscm") ) == 0 )
		{
			pItemInfo->m_iVideoMode = 2;
			// pItemInfo->m_bNTSCMode = false; 
		}
		else if ( _tcsicmp( sTemp, _T("ntscj") ) == 0 )
		{
			pItemInfo->m_iVideoMode = 3;
			// pItemInfo->m_iVideoMode = -1;
			// pItemInfo->m_bNTSCMode = false; 
		}
		else if ( _tcsicmp( sTemp, _T("pal50") ) == 0 )
		{
			pItemInfo->m_iVideoMode = 4;
			// pItemInfo->m_bNTSCMode = false; 
		}
		else if ( _tcsicmp( sTemp, _T("default") ) == 0 )
		{
			// No change
			pItemInfo->m_iVideoMode = 0;
			// pItemInfo->m_bNTSCMode = IsNTSC(); 
		}
		else if ( _tcsicmp( sTemp, _T("auto") ) == 0 )
		{
			// Based on game....
			pItemInfo->m_iVideoMode = 1;
			// pItemInfo->m_bNTSCMode = false; 
		}
		else
		{
			pItemInfo->m_iVideoMode = 0;
			// pItemInfo->m_bNTSCMode = IsNTSC();
		}

		sTemp = pNode->GetString( NULL, _T("passcode"), _T("") );
		if ( sTemp.GetLength() )
		{
			pItemInfo->m_sPasscode = sTemp;
		}
		// Iterate through each of the attributes and elements
		iterAttrib = pNode->m_msAttributes.begin();
		while( iterAttrib != pNode->m_msAttributes.end() )
		{
			pItemInfo->SetValue( iterAttrib->first.c_str(), iterAttrib->second.c_str() );
			iterAttrib++;
		}
		iterElement = pNode->m_msElements.begin();
		while( iterElement != pNode->m_msElements.end() )
		{
			pElement = iterElement->GetElementPtr();
			pItemInfo->SetValue( pElement->m_sName, pElement->m_sValue );
			iterElement++;
		}

		if ( sNodeName.Compare( _T("item") ) == 0 )
		{
			pItemInfo->m_pMenuNode = pNode;
			pItemInfo->m_sAction = pNode->GetString( NULL, _T("action"), _T("") );

			pItemInfo->m_bIsMenu = false;
			if ( pItemInfo->m_sAction.GetLength() )
			{
				pItemInfo->m_bIsAction = true;
			}
			else
			{
				pItemInfo->m_bIsAction = false;
			}
			bReturn = true;
		}
		else if ( sNodeName.Compare( _T("submenu") ) == 0 )
		{
			pItemInfo->m_pMenuNode = pNode;
			pItemInfo->m_bIsMenu = true;
			bReturn = true;
		}
	}
	return bReturn;
}

HRESULT CMenuInfo::LoadMenu( CXMLNode * pNode, bool bClearCurrent )
{
	HRESULT hr = E_FAIL;
	CItemInfo itemInfo;
	int iItemCount = 0;
	// CStdString sSortField;
	TListStrings slSortFields;
	TItemList::iterator iterCurrent, iterSortStart, iterTemp, iterSortEnd;
	TSortData sortData;
	bool bFirstSubNode = true;



	// m_sMenuName needs to be set!!
	CopyNodeToItemInfo( pNode, &m_CurrentMenu );

	// Does current menu have a sortfield?
	if ( pNode->GetStrings( _T("sortfield"), slSortFields ) )
	{
		sortData.m_slSortFields = slSortFields;
		sortData.m_iterSortStart = m_ilEntries.end();
	}
	else
	{
		if ( pNode->GetStrings( _T("currsortfield"), slSortFields ) )
		{
			sortData.m_slSortFields = slSortFields;
			sortData.m_iterSortStart = m_ilEntries.end();
		}
	}
	// sSortField = pNode->GetString( NULL, _T("sortfield"), _T("") );


	// First off, save status of current item selection to current node.
	if ( m_pMenuCurrNode )
	{
		m_pMenuCurrNode->SetLong( NULL, _T("CurrentItem"), m_iCurrentItem );
	}
	if ( pNode )
	{
		int iNodeIndex = 0;
		CXMLNode * pSubNode;
		CStdString sNodeName;
		CStdString sTemp;

		if ( bClearCurrent )
		{
			ClearMenuItems();
		}

		if ( slSortFields.size() )
		{
			sortData.m_slSortFields = slSortFields;
			// sortData.m_sSortField = sSortField;
			sortData.m_iterSortStart = m_ilEntries.end();
		}

		// Add entries
		pSubNode = pNode->GetNode(iNodeIndex);
		while ( pSubNode )
		{

			if ( CopyNodeToItemInfo( pSubNode, &itemInfo ) )
			{
//				if ( bFirstSubNode )
//				{
					if ( pSubNode->GetString( NULL, _T("sortfield"), _T("") ).GetLength() )
					{
						// New set of sort fields
						if ( pSubNode->GetStrings( _T("sortfield"), slSortFields ) )
						{
							sortData.m_slSortFields = slSortFields;
							sortData.m_iterSortStart = m_ilEntries.end();
						}
						else
						{
							sortData.m_slSortFields.clear();
							sortData.m_iterSortStart = m_ilEntries.end();
						}
					}
					else
					{
						if ( bFirstSubNode && slSortFields.size() )
						{
							pSubNode->SetStrings( _T("sortfield"), slSortFields );
						}
					}
//				}
//				else
//				{
//				}
//				if ( sSortField.GetLength() == 0 )
//				{
//					sSortField = pSubNode->GetString( NULL, _T("sortfield"), _T("") );
//					if ( sSortField.GetLength() )
//					{
//						sortData.m_sSortField = sSortField;
//						sortData.m_iterSortStart = m_ilEntries.end();
//					}
//				}
					if ( !bFirstSubNode && itemInfo.m_bIsMenu && slSortFields.size() )
				{
					// Make it stick, too...
					pSubNode->SetStrings( _T("currsortfield"), slSortFields );
//					pSubNode->SetString( NULL, _T("sortfield"), sSortField );
//					itemInfo.SetValue( _T("sortfield"), sSortField );
				}
				iItemCount++;
				AddItem(itemInfo, sortData );
				if ( pSubNode->GetNode( _T("endsort") ) || pSubNode->GetElement( _T("endsort") ) )
				{
					slSortFields.clear();
					sortData.m_slSortFields.clear();
					sortData.m_iterSortStart = m_ilEntries.end();
					//sSortField = _T("");
					//sortData.m_sSortField = _T("");
					//sortData.m_iterSortStart = m_ilEntries.end();
				}
			}
			// If it's a submenu or item node, check to see if we need to start sorting...
			iNodeIndex++;
			pSubNode = pNode->GetNode(iNodeIndex);
			bFirstSubNode = false;
		}
		if ( iItemCount )
		{
			m_pMenuCurrNode = pNode;
			hr = S_OK;
			m_dwMenuHash = CalcMenuHash();
		}
	}
	if ( m_pMenuCurrNode )
	{
		m_iCurrentItem = m_pMenuCurrNode->GetLong( NULL, _T("CurrentItem"), 0 );
		if ( m_iCurrentItem >= iItemCount )
		{
			// Must be invalid... just reset to zero
			m_iCurrentItem = 0;
		}
		m_sMenuName = m_pMenuCurrNode->GetString( NULL, _T("title"), _T("") );
		if ( (m_sMenuName.GetLength()==0) && ( m_pMenuCurrNode == m_pMenuNode ) )
		{
			m_sMenuName = _T("Main Menu");
		}
	}
// 	SortItems();
	return hr;
}


int	CMenuInfo::AddItem( CItemInfo & itemInfo, TSortData & sortData )
{
	CStdString sDescr = itemInfo.GetValue( _T("descr") );
	if ( sDescr.GetLength() == 0 )
	{
		sDescr = itemInfo.GetValue( _T("description") );
		if ( sDescr.GetLength() )
		{
			itemInfo.SetValue( _T("descr"), sDescr );
		}
	}

	// Support three levels deep for now.
	if ( sortData.m_slSortFields.size() )
	{
		int iResult;
		TItemList::iterator iterCurrent = sortData.m_iterSortStart;

		CStdString sField;
		CStdString sVal1, sVal2;
		
		sField = GetString( 0, sortData.m_slSortFields );

		while( iterCurrent != m_ilEntries.end() )
		{
			//sVal1 = itemInfo.GetValue( sField );
			//sVal2 = iterCurrent->GetValue( sField );

			iResult = _tcsicmp( itemInfo.GetValue( sField ), iterCurrent->GetValue( sField ) );
			if ( iResult < 0 )
			{
				m_ilEntries.insert( iterCurrent, itemInfo );
				// We are inserting BEFORE the start, so move our start point back a notch
				if ( iterCurrent == sortData.m_iterSortStart )
				{
					sortData.m_iterSortStart--;
				}
				break;
			}
			else if ( iResult == 0 )
			{
				CStdString sField2;

				
				sField2 = GetString( 1, sortData.m_slSortFields );
				iResult = _tcsicmp( itemInfo.GetValue( sField2 ), iterCurrent->GetValue( sField2 ) );
				if ( iResult < 0 )
				{
					m_ilEntries.insert( iterCurrent, itemInfo );
					// We are inserting BEFORE the start, so move our start point back a notch
					if ( iterCurrent == sortData.m_iterSortStart )
					{
						sortData.m_iterSortStart--;
					}
					break;
				}
				else if ( iResult == 0 )
				{
					CStdString sField3;
					
					sField3 = GetString( 2, sortData.m_slSortFields );
					iResult = _tcsicmp( itemInfo.GetValue( sField3 ), iterCurrent->GetValue( sField3 ) );
					if ( iResult < 0 )
					{
						m_ilEntries.insert( iterCurrent, itemInfo );
						// We are inserting BEFORE the start, so move our start point back a notch
						if ( iterCurrent == sortData.m_iterSortStart )
						{
							sortData.m_iterSortStart--;
						}
						break;
					}
				}
			}
			iterCurrent++;
		}
		// Did we already insert? If not, put on end...
		if( iterCurrent == m_ilEntries.end() )
		{
			m_ilEntries.push_back( itemInfo );
			if ( sortData.m_iterSortStart == m_ilEntries.end() )
			{
				sortData.m_iterSortStart--;
			}
		}
	}
	else
	{
		m_ilEntries.push_back( itemInfo );
	}
	m_dwMenuHash = CalcMenuHash();
	
	// Check media and add it to the queue
	CStdString sMediaFilename = itemInfo.GetValue( _T("Media") );
	if ( IsImageFile( sMediaFilename, true ) )
	{
		g_FileManager.AddFileEntry(sMediaFilename);
	}

	return GetItemCount();
}

#if 0
void CMenuInfo::SwapItems( TItemList::iterator iterA, TItemList::iterator iterB )
{

}


void CMenuInfo::SortItems( void )
{
#if 0
	TItemList::iterator iterCurrent, iterSortStart, iterTemp, iterSortEnd;
	CStdString sSortField;
	bool bEndSort = false;

	iterCurrent = m_ilEntries.begin();
	while( iterCurrent != m_ilEntries.end() )
	{
		if ( sSortField.GetLength() )
		{
			CStdString sTemp, sCurrent;
			// Check to see if this is the end of the sort...
			if ( _tcsicmp( iterTemp->GetValue(_T("endsort")), sSortField )
			{
				bEndSort = true;
			}

			// Perform Sort
			iterSortEnd = iterCurrent
			// Search through entries
			iterSortEnd--;
			iterTemp = iterSortStart;
			while( iterTemp <= iterSortEnd )
			{
				sTemp = iterTemp->GetValue( sSortField );
				sCurrent = iterSortCurrent->GetValue( sSortField );
				if ( _tcsicmp( iterTemp->GetValue( sSortField ), iterSortCurrent->GetValue( sSortField ) ) < 0 )
				{
					// Swap them
					m_ilEntries.insert( 
					iterCurrent = iterSortEnd;
					break;
				}
				iterTemp++;
			}
			if ( bEndSort )
			{
				sSortField = _T("");
				bEndSort = false;
			}
		}
		else
		{
		}
		iterCurrent++;
	}
#endif
}

#endif


int	CMenuInfo::GetItemCount( void )
{
	return (int)m_ilEntries.size();
}

void CMenuInfo::ClearMenuItems( void )
{
	m_ilEntries.clear();
	m_dwMenuHash = CalcMenuHash();
}

DWORD GetStringHash( LPCTSTR szValue )
{
	DWORD dwReturn = 0;
	int iLength;

	if ( szValue && _tcslen( szValue ) )
	{
		iLength = _tcslen( szValue );
		while( iLength-- )
		{
			if( dwReturn & 0x80000000 )
			{
				dwReturn <<= 1;
				dwReturn |= 1;
			}
			else
			{
				dwReturn <<= 1;
			}
			dwReturn += *szValue;
			szValue++;
		}
	}

	return dwReturn;
}

DWORD	CMenuInfo::CalcMenuHash( void )
{
	DWORD dwReturn = 0;
	DWORD dwItemHash = 0;

	TItemList::iterator iterItem;
	CItemInfo *pReturn = NULL;
	CStdString sValue;

	iterItem = m_ilEntries.begin();
	while ( iterItem != m_ilEntries.end() )
	{
		dwItemHash = 0;

		dwItemHash += GetStringHash( iterItem->GetValue( szII_Title ) );
		dwItemHash += GetStringHash( iterItem->GetValue( szII_Dir ) );
		dwItemHash += GetStringHash( iterItem->GetValue( szII_Exe ) );

		iterItem++;
		dwItemHash += dwReturn;
		dwReturn <<= 1;
		dwReturn ^= dwItemHash;
	}

	return dwReturn;
}

CItemInfo *     CMenuInfo::MenuEntry(void)
{
	return &m_CurrentMenu;
}

CItemInfo *		CMenuInfo::Entry( int iIndex )
{
	TItemList::iterator iterItem;
	CItemInfo *pReturn = NULL;

	if ( (iIndex < 0) || (iIndex>=(int)m_ilEntries.size()))
	{
		iIndex = m_iCurrentItem;
	}
	if ( iIndex>=(int)m_ilEntries.size() )
	{
		iIndex = 0;
	}

	iterItem = m_ilEntries.begin();
	if ( iIndex>=0 && iIndex<(int)m_ilEntries.size() )
	{
		while( iIndex-- )
		{
			iterItem++;
		}
		if ( iterItem != m_ilEntries.end() )
		{
			pReturn = &(*iterItem);
		}
	}
	return pReturn;
}
