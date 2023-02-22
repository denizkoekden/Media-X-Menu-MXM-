/*********************************************************************************\
 * utils.cpp
 * Misc classes and routines to make life easier.
 * (C) 2002 MasterMind
\*********************************************************************************/


#include <XTL.h>
#include <WinNT.h>
#include <xgraphics.h>
#include "MXM.h"
#include "StdString.h"
#include "xntdll.h"
#include "utils.h"
#include "BinData.h"
#include "CommDebug.h"
#include "MenuInfo.h"
#include "flash.h"

//#include "XKUtils\XKEEPROM.h"

#define TWOD_Z		1.0f

CStdString g_sIP;

CXBoxFlash * g_pFlashControl = NULL;
bool	g_bUseNetRandom = false;
bool	g_bFirstRandom = true;

void InitFlashControl( void )
{
	if ( g_pFlashControl == NULL )
	{
		g_pFlashControl = new CXBoxFlash();
	}
}


HRESULT SetReadonly( LPCTSTR szPath, bool bSetReadOnly, bool bCascadeSubdirs )
{
	HRESULT hr = E_FAIL;
	bool bDoDir = false;

	if ( ( _tcsicmp( szPath, _T("C:\\") ) == 0 ) ||
			( _tcsicmp( szPath, _T("E:\\") ) == 0 ) ||
			( _tcsicmp( szPath, _T("F:\\") ) == 0 ) ||
			( _tcsicmp( szPath, _T("G:\\") ) == 0 ) ||
			( _tcsicmp( szPath, _T("X:\\") ) == 0 ) ||
			( _tcsicmp( szPath, _T("Y:\\") ) == 0 ) ||
			( _tcsicmp( szPath, _T("Z:\\") ) == 0 ) )
	{
		bDoDir = true;
	}
	else
	{
		DWORD dwAttr = GetFileAttributes( szPath );
		if ( (int)dwAttr != (-1) )
		{
			if ( dwAttr & FILE_ATTRIBUTE_DIRECTORY && bCascadeSubdirs )
			{
				bDoDir = true;
				SetReadonly( szPath, bSetReadOnly, false ); // Set this directory, first...
			}
			else
			{
				SetFileAttributes( szPath, bSetReadOnly?FILE_ATTRIBUTE_READONLY:FILE_ATTRIBUTE_NORMAL );
				hr = S_OK;
			}
		}
	}
	if ( bDoDir )
	{
		// Now scan that directory and call this on all members...
		CStdString sWildcard;
		HANDLE hFind;
		WIN32_FIND_DATA ffData;
		CStdString sFilename;
		CStdString sNewPath;

		sWildcard = MakeFullFilePath( szPath, _T("*.*") );
		hFind = FindFirstFile( sWildcard, &ffData );

		if( INVALID_HANDLE_VALUE != hFind )
		{
			// hr = S_OK;
			do
			{
				sNewPath = MakeFullFilePath( szPath, ffData.cFileName );
				SetReadonly( sNewPath, bSetReadOnly, true ); // Set this directory, first...
			} while( FindNextFile( hFind, &ffData ) );
			// Close the find handle.
			FindClose( hFind );
		}
		hr = S_OK;
	}
#if 0
	DWORD dwAttr = GetFileAttributes( szPath );
	if ( (int)dwAttr != (-1) )
	{
		if ( dwAttr & FILE_ATTRIBUTE_DIRECTORY && bCascadeSubdirs )
		{
			SetReadonly( szPath, bSetReadOnly, false ); // Set this directory, first...
			// Now scan that directory and call this on all members...
			CStdString sWildcard;
			HANDLE hFind;
			WIN32_FIND_DATA ffData;
			CStdString sFilename;
			CStdString sNewPath;

			sWildcard = MakeFullFilePath( szPath, _T("*.*") );
			hFind = FindFirstFile( sWildcard, &ffData );

			if( INVALID_HANDLE_VALUE != hFind )
			{
				// hr = S_OK;
				do
				{
					sNewPath = MakeFullFilePath( szPath, ffData.cFileName );
					SetReadonly( sNewPath, bSetReadOnly, true ); // Set this directory, first...
				} while( FindNextFile( hFind, &ffData ) );
				// Close the find handle.
				FindClose( hFind );
			}
		}
		else
		{
			SetFileAttributes( szPath, bSetReadOnly?FILE_ATTRIBUTE_READONLY:FILE_ATTRIBUTE_NORMAL );
		}
		hr = S_OK;
	}
#endif
	return hr;
}

CStdString PathSlasher( LPCTSTR szPath, bool bSlashIt )
{
	CStdString sReturn;
	int iLen;

	if ( szPath && (iLen = _tcslen( szPath )) )
	{
		iLen--;
		if ( szPath[iLen] == _T('\\') )
		{
			if ( !bSlashIt )
			{
				sReturn = CStdString( szPath, iLen );
			}
			else
			{
				sReturn = szPath;
			}
		}
		else
		{
			if ( bSlashIt )
			{
				sReturn = szPath;
				sReturn += _T('\\');
			}
			else
			{
				sReturn = szPath;
			}
		}
		if ( !bSlashIt ) // Special case: Root directory MUST be slashed
		{
			if ( sReturn.GetLength() == 2 && sReturn[1] == _T(':') && _istalpha( sReturn[0]) )
			{
				sReturn += _T('\\');
			}
		}
	}
	return sReturn;
}

CStdString ReplaceFilenameExtension( LPCTSTR szFilepath, LPCTSTR szExt )
{
	CStdString sReturn(szFilepath);
	CStdString sExt(szExt);
	int iPos;

	// Remove leading period.
	if ( sExt.GetLength() && sExt[0] == _T('.') )
	{
		sExt = sExt.Mid(1);
	}
	iPos = sReturn.ReverseFind( _T(".") );
	if ( iPos >= 0 )
	{
		sReturn = sReturn.Left( iPos+1 );
		sReturn += sExt;
	}
	else
	{
		// No "."? Tack an extension on the end!
		sReturn += _T(".");
		sReturn += sExt;
	}
	return sReturn;
}

CStdString GetFilenameExtension( LPCTSTR szFilepath )
{
	CStdString sReturn(szFilepath);
	int iPos;

	iPos = sReturn.ReverseFind( _T(".") );
	if ( iPos >= 0 )
	{
		sReturn = sReturn.Mid( iPos+1 );
	}
	else
	{
		// No "."? 
		// Return nothing!
		sReturn = _T("");
	}
	return sReturn;
}

CStdString GetFilenameBase( LPCTSTR szFilepath )
{
	CStdString sReturn(szFilepath);
	int iPos;


	iPos = sReturn.ReverseFind( _T(".") );
	if ( iPos >= 0 )
	{
		sReturn = sReturn.Mid( iPos+1 );
		sReturn = ExtractFilenameFromPath( sReturn, false );
	}
	else
	{
		// No "."? 
		// Return nothing!
		sReturn = _T("");
	}
	return sReturn;
}

CStdString MakeBackExtension( LPCTSTR szExtension )
{
	CStdString sReturn(_T("bak"));
	if ( szExtension && (_tcslen( szExtension )>2) )
	{
		if ( _tcsicmp( szExtension, _T("bak") ) == 0 )
		{
			sReturn = _T("b00");
		}
		else if ( _tolower(szExtension[0]) == _T('b') && _istdigit(szExtension[1]) && _istdigit(szExtension[2]) )
		{
			long lDigit;

			lDigit = _tcstol( &szExtension[1], NULL, 10 );
			lDigit++;
			sReturn.Format( _T("b%02d"), lDigit );
		}
	}
	return sReturn;
}

bool MakeFileBak( LPCTSTR szFilename )
{
	bool bReturn = false;
	DWORD dwFileAttr = 0;

	dwFileAttr = GetFileAttributes( szFilename );
	if ( (dwFileAttr != (-1)) && !(dwFileAttr&FILE_ATTRIBUTE_DIRECTORY) )
	{
		CStdString sName(szFilename), sExt, sNewExt;
		if ( dwFileAttr&FILE_ATTRIBUTE_READONLY )
		{
			SetFileAttributes( szFilename, dwFileAttr^FILE_ATTRIBUTE_READONLY );
		}
		// Now we are ready to change it....
		// BAK, but if it exists, BAK->B01, and so forth...
		sExt = GetFilenameExtension( szFilename );
		sNewExt = MakeBackExtension( sExt );
		sName = ReplaceFilenameExtension( sName, sNewExt );
		if ( FileExists( sName ) )
		{
			if ( MakeFileBak( sName ) )
			{
				if ( !FileExists( sName ) )
				{
					if ( MoveFile( szFilename, sName ) )
					{
						bReturn = true;
					}
				}
			}
		}
		else
		{
			if ( MoveFile( szFilename, sName ) )
			{
				bReturn = true;
			}
		}
	}
	return bReturn;
}

CStdString GetTempFileName(	LPCTSTR lpPathName, LPCTSTR lpPrefixString )
{
	CStdString sReturn;
	CStdString sPrefix( lpPrefixString );
	CStdString sPath( lpPathName );
	DWORD dwNum;

	if ( sPath.GetLength() == 0 )
	{
		sPath = _T("Z:\\");
	}
	dwNum = GetTickCount();
	dwNum <<= 16;
	dwNum ^= (RandomNum()&0x0ffff);
	sReturn.Format( _T("%s%08x.tmp"), sPrefix.c_str(), dwNum );
	sReturn = MakeFullFilePath( sPath, sReturn );
	while ( FileExists( sReturn ) )
	{
		dwNum += 1;
		sReturn.Format( _T("%s%08x.tmp"), sPrefix.c_str(), dwNum );
		sReturn = MakeFullFilePath( sPath, sReturn );
	}
	return sReturn;
}

static const BYTE s_pbPatchComp[8] = { 0xE8, 0xCA, 0xFD, 0xFF, 0xFF, 0x85, 0xC0, 0x7D };
// E8 CA FD FF FF 85 C0 7D.E8CAFDFFFF85C0EB!
//UNDO E8CAFDFFFF85C0EB.E8CAFDFFFF85C07D!
// We'll create a new file on Z:, then copy it over after renaming the old file
// successfully.
// We'll create a buffer that we will work with in halves. 
// We'll load up the first half, then the second.
// We'll scan the first half for our scan byte, and make changes....
// Write the first half, move the remainder bytes down, then read the second half
bool MediaPatch( LPCTSTR szFilename )
{
	bool bReturn = false;
	LPBYTE pBuffer = NULL;
	bool bChanged = false;
	CStdString sNewFilename;

	if ( FileExists( szFilename ) )
	{
		// We'll scan the body of code for the particular strings
		pBuffer = new BYTE[32768];
		if ( pBuffer )
		{
			HANDLE hFileSrc, hFileDest;
			sNewFilename = GetTempFileName( _T("Z:\\"), _T("MP") );

			// Open Source File
			hFileSrc = CreateFile( szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if ( hFileSrc != INVALID_HANDLE_VALUE )
			{
				// Open Dest File
				hFileDest = CreateFile( sNewFilename, 
					   GENERIC_WRITE, 
					   0, 
					   NULL,
					   CREATE_ALWAYS,
					   FILE_ATTRIBUTE_NORMAL,
					   NULL );
				if ( hFileDest != INVALID_HANDLE_VALUE )
				{
					bool bError = false;
					DWORD dwRead = 0, dwWritten = 0;
					// Get First Chunk
					if ( ReadFile( hFileSrc, pBuffer, 16384, &dwRead, NULL ) )
					{
						// First sector... change bytes 214-217
						if ( dwRead > 0x11c )
						{
							DWORD dwAddress = *((DWORD*)&pBuffer[0x104]);
							DWORD dwOffset = *((DWORD*)&pBuffer[0x118]);
							dwOffset -= dwAddress; // Subtract XBE address
							dwOffset += 0x009C; // Add Media Flag offset
							if ( dwRead > dwOffset )
							{
								if ( pBuffer[dwOffset] != 0xff )
								{
									pBuffer[dwOffset] = 0xff;
									pBuffer[dwOffset+1] = 0xff;
									pBuffer[dwOffset+2] = 0xff;
									pBuffer[dwOffset+3] = 0x80;
									bChanged = true;
								}
								else if ( pBuffer[dwOffset+1] != 0xff )
								{
									pBuffer[dwOffset+1] = 0xff;
									pBuffer[dwOffset+2] = 0xff;
									pBuffer[dwOffset+3] = 0x80;
									bChanged = true;
								}
								else if ( pBuffer[dwOffset+2] != 0xff )
								{
									pBuffer[dwOffset+2] = 0xff;
									pBuffer[dwOffset+3] = 0x80;
									bChanged = true;
								}
								else if ( pBuffer[dwOffset+3] != 0x80 )
								{
									pBuffer[dwOffset+3] = 0x80;
									bChanged = true;
								}
							}
						}
						// while not finished, get second chunk
						while ( dwRead == 16384 && !bError )
						{
							if ( ReadFile( hFileSrc, &pBuffer[16384], 16384, &dwRead, NULL ) )
							{
								// Scan first chunk for patch code data
								LPBYTE pbPos = NULL;

								pbPos = (LPBYTE)memchr( pBuffer, 0xe8, 16384 );

								while( pbPos && (pbPos < &(pBuffer[16384])))
								{
									if ( memcmp( pbPos, s_pbPatchComp, 8 ) == 0 )
									{
										// Patch it!
										bChanged = true;
										pbPos[7] = 0xEB;
									}
									pbPos = (LPBYTE)memchr( &pbPos[1], 0xe8, 16384 );
								}
								// Write first chunk
								if ( !WriteFile( hFileDest, pBuffer, 16384, &dwWritten, NULL ) )
								{
									// Error!
									bError = true;
									dwRead = 0;
									break;
								}
								// Move second chunk to first chunk space
								memcpy( pBuffer, &pBuffer[16384], dwRead );
							}
						}

						// If we have remaining bytes read in the buffer,
						if ( dwRead > 0 )
						{
							// Scan bytes for patch code
							// If present, set bChanged and change bytes
							// Write remaining data
							if ( !WriteFile( hFileDest, pBuffer, dwRead, &dwWritten, NULL ) )
							{
								// Error!
								bError = true;
								dwRead = 0;
							}
						}
					}

					// Close Dest file
					CloseHandle( hFileDest );
					// Close Source file
					CloseHandle( hFileSrc );
					
					if (  !bError )
					{
						if ( bChanged )
						{
							// Rename Dest file to replace .bak with .xbe
							if ( MakeFileBak( szFilename ) )
							{
								// Now copy over the "new" file to the "original" location...
								if ( CopyFile( sNewFilename, szFilename, true ) )
								{
									DeleteFile( sNewFilename );
									bReturn = true;
								}
							}
						}
						else
						{
							// Don't need the new file.
							DeleteFile( sNewFilename );
							bReturn = true;
						}
					}
					else
					{
						// Don't need the new file.
						DeleteFile( sNewFilename );
					}
				}
			}
			delete [] pBuffer;
		}
	}
	return bReturn;
}

HRESULT CopyFilesOrPath( LPCTSTR szBasePath, LPCTSTR szSource, LPCTSTR szDest )
{
	HRESULT hr = E_FAIL;
	CStdString sSource(szSource), sDest( szDest );

	if ( sSource.GetLength() && sDest.GetLength() )
	{
		// sSource can be:
		// 1. A file
		// 2. A Path
		// 3. Contain wildcards
		sSource = MakeFullFilePath( szBasePath, szSource );
		sDest = MakeFullFilePath( szBasePath, szDest );
		DWORD dwAttr = GetFileAttributes( sSource );
		if ( (int)dwAttr == (-1) )
		{
			// Is it a wildcard?
			if ( sSource.FindOneOf( _T("*?") ) >= 0 )
			{
				HANDLE hFind;
				WIN32_FIND_DATA ffData;
				CStdString sFilename;
				CStdString sNewPath;
				CStdString sNewDestPath;

				int iPos = sSource.ReverseFind( _T("\\") );

				if ( iPos >= 0 ) // If not, we've got issues!!
				{
					sNewPath = sSource.Left( iPos );
					if ( ( sNewPath.GetLength() == 2 ) && (sNewPath[1] == _T(':')) )
					{
						sNewPath += _T("\\");
					}
				}
				else
				{
					sNewPath = szBasePath;
				}

				hFind = FindFirstFile( sSource, &ffData );

				if( INVALID_HANDLE_VALUE != hFind )
				{
					// hr = S_OK;
					do
					{
						sFilename = MakeFullFilePath( sNewPath, ffData.cFileName );
						if ( ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
						{
							sNewDestPath = PathSlasher( MakeFullFilePath( sDest, ffData.cFileName ), true );
							hr = CopyFilesOrPath( szBasePath, sFilename, sNewDestPath );
						}
						else
						{
							hr = CopyFilesOrPath( szBasePath, sFilename, sDest );
						}
					} while( SUCCEEDED(hr) && FindNextFile( hFind, &ffData ) );
					// Close the find handle.
					FindClose( hFind );
				}
			}
			// ELSE: Failed...
		}
		else if ( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
		{
			// It's a directory
			// Read contents of directory and copy to destination!
			// szDest had better not be a file!!
			if ( !FileExists( sDest ) || (GetFileAttributes( sDest )&FILE_ATTRIBUTE_DIRECTORY) )
			{
				hr = S_OK;
				if ( !FileExists( sDest ) )
				{
					// Make destination!
					hr = MakePath( sDest );
				}
				if ( SUCCEEDED(hr) )
				{
					// OK, now get contents and copy out.... We'll use a wildcard
					sSource = MakeFullFilePath( sSource, _T("*") );
					hr = CopyFilesOrPath( szBasePath, sSource, sDest );
				}
			}
		}
		else
		{
			// It's a file
			// Tricky on dest... might be a path, might be a filename!
			if ( !FileExists( sDest ) )
			{
				// Do we make path, or is this a filename??!??
				// If it ends in a backslash, it's a path....
				int iPos = sDest.GetLength()-1;
				if ( iPos >= 0 && sDest[iPos] == _T('\\') )
				{
					// It's a path....
					hr = MakePath( sDest );

					if( SUCCEEDED(hr) )
					{
						CStdString sBaseName = ExtractFilenameFromPath( sSource, false );
						sDest = MakeFullFilePath( sDest, sBaseName );
						if ( CopyFile( sSource, sDest, FALSE ) )
						{
							hr = S_OK;
						}
						else
						{
							hr = E_FAIL;
						}
					}
				}
				else
				{
					// It's a filename...
					if ( CopyFile( sSource, sDest, FALSE ) )
					{
						hr = S_OK;
					}
				}
			}
			else if ( GetFileAttributes( sDest )&FILE_ATTRIBUTE_DIRECTORY )
			{
				CStdString sBaseName = ExtractFilenameFromPath( sSource );
				sDest = MakeFullFilePath( sDest, sBaseName );
				if ( CopyFile( sSource, sDest, FALSE ) )
				{
					hr = S_OK;
				}
				else
				{
					hr = E_FAIL;
				}
			}
			else // File exists...
			{
				if ( CopyFile( sSource, sDest, FALSE ) )
				{
					hr = S_OK;
				}
				else
				{
					hr = E_FAIL;
				}
			}
		}

	}
#if 0
			CStdString sWildcard;
			HANDLE hFind;
			WIN32_FIND_DATA ffData;
			CStdString sFilename;
			CStdString sNewPath;

			sWildcard = MakeFullFilePath( szPath, _T("*.*") );
			hFind = FindFirstFile( sWildcard, &ffData );

			if( INVALID_HANDLE_VALUE != hFind )
			{
				// hr = S_OK;
				do
				{
					sNewPath = MakeFullFilePath( szPath, ffData.cFileName );
					SetReadonly( sNewPath, bSetReadOnly, true ); // Set this directory, first...
				} while( FindNextFile( hFind, &ffData ) );
				// Close the find handle.
				FindClose( hFind );
			}
#endif
	return hr;
}

HRESULT MakePath( LPCTSTR szPath )
{
	HRESULT hr = E_FAIL;

	// Generate path from start to finish!
	CStdString sPath(szPath);

	sPath = PathSlasher( szPath, false );
#if 0
	int iPos = sPath.GetLength()-1;
	// Strip trailing backslash...
	if ( iPos >= 0 && sPath[iPos] == _T('\\') )
	{
		sPath = sPath.Left( iPos );
	}
#endif
	DWORD dwAttr = GetFileAttributes( sPath );

	if ( sPath.GetLength() > 2 )
	{
		if ( dwAttr == (-1))
		{
			// OK, let's step through the process of making a path
			if ( _istalpha( sPath[0] ) && sPath[1] == _T(':') && sPath[2] == _T('\\') )
			{
				// OK so far... let's see if we need to go further:
				if ( sPath.GetLength() > 3 )
				{
					// Call MakePath on parent directory....
					CStdString sParent;
					int iPos = sPath.ReverseFind( _T("\\") );
					if ( iPos >= 2 )
					{
						sParent = sPath.Left( iPos );
						hr = MakePath( sParent );
					}
					else
					{
						// else parent was root...
						hr = S_OK;
					}
					dwAttr = GetFileAttributes( sPath );
					if ( (int)dwAttr == (-1) )
					{
						if ( SUCCEEDED(hr) && CreateDirectory( sPath, NULL ) )
						{
							hr = S_OK;
						}
						else
						{
							hr = E_FAIL;
						}
					}
					else
					{
						if ( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
						{
							hr = S_OK;
						}
						else
						{
							hr = E_FAIL;
						}
					}
				}
				else
				{
					//Drive root, just say it's "OK"
					hr = S_OK;
				}
			}
			// Else: Fail!
		}
		else if ( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
		{
			// Already exists... that's fine.
			hr = S_OK;
		}
		// Else, failed. Must be file. We cannot make a path!
	}
	return hr;
}


CStdString GetArg( LPCTSTR szLine, int iArg )
{
	CStdString sReturn;
	int iStart = 0;
	int iEnd = 0;
	bool bEscaped = false;

	if ( szLine && _tcslen( szLine ) && (iArg >= 0) )
	{
		// Trim leading white space....
		while( szLine[iStart] && _istspace( szLine[iStart] ) )
		{
			iStart++;
		}
		// Split it....
		while( iArg )
		{
			if ( szLine[iStart] == _T('\"') ) // Escaped, quoted argument...
			{
				iStart++;

				// Find next starting point...
				while( szLine[iStart] && (szLine[iStart] != _T('\"'))  )
				{
					// Escaped character
					if ( szLine[iStart] == _T('\\') )
					{
						iStart++;
					}
					if ( szLine[iStart] )
						iStart++;
				}
				if ( szLine[iStart] )
					iStart++;
			}
			else
			{
				// Find next starting point...
				while( szLine[iStart] && !_istspace( szLine[iStart] ) )
				{
					iStart++;
				}
			}
			// Trim leading white space....
			while( szLine[iStart] && _istspace( szLine[iStart] ) )
			{
				iStart++;
			}
			iArg--;
		}
		iEnd = iStart;

		// Now get the argument out....
		if ( szLine[iEnd] == _T('\"') ) // Escaped, quoted argument...
		{
			bEscaped = true;
			iEnd++;
			iStart++; // We don't want the actual quote in the string!

			// Find next starting point...
			while( szLine[iEnd] && (szLine[iEnd] != _T('\"'))  )
			{
				// Escaped character
				if ( szLine[iEnd] == _T('\\') )
				{
					iEnd++;
				}
				if ( szLine[iEnd] )
					iEnd++;
			}
			if ( (szLine[iEnd]==0) || (szLine[iEnd] == _T('\"')) )
			{
				iEnd--;
			}
		}
		else
		{
			// Find next starting point...
			while( szLine[iEnd] && !_istspace( szLine[iEnd] ) )
			{
				iEnd++;
			}
			if ( (szLine[iEnd]==0) || _istspace(szLine[iEnd]) )
			{
				iEnd--;
			}
		}
		if ( iEnd >= iStart )
		{
			sReturn = CStdString( &szLine[iStart], (iEnd-iStart)+1 );
		}

	}


	if ( bEscaped )
	{
		// Replace escaped characters (we only really support quotes, ATM)
		sReturn.Replace( _T("\\\\"), _T("\x01") );
		sReturn.Replace( _T("\\"), _T("") );
		sReturn.Replace( _T("\x01"), _T("\\") );
	}

	return sReturn;
}


int RandomRange( int iLow, int iHigh )
{
	int iReturn = iLow;
	int iDiff = (iHigh-iLow)+1;

	if ( iDiff > 0 )
	{
		iReturn = (int)(RandomNum()%(DWORD)iDiff);
		iReturn += iLow;
	}
	return iReturn;
}

DWORD RandomNum( void )
{
	DWORD dwReturn;

	if ( g_bUseNetRandom )
	{
		XNetRandom( (BYTE*)&dwReturn, 4 );
	}
	else
	{
		if ( g_bFirstRandom )
		{
			srand((unsigned)time( NULL ));
			g_bFirstRandom = false;
		}
		dwReturn += rand();
		dwReturn <<= 8;
		dwReturn += rand();
		dwReturn <<= 8;
		dwReturn += rand();
		dwReturn <<= 8;
		dwReturn += rand();
	}

	return dwReturn;
}

DWORD GetFlashID( void )
{
	DWORD dwReturn = 0;

	InitFlashControl();

	if ( g_pFlashControl )
	{
		dwReturn = g_pFlashControl->CheckID();
		dwReturn &= 0x0000ffff;
	}
	return dwReturn;
}

CStdString GetFlashString( DWORD dwID, bool bNumericOK )
{
	CStdString sReturn(_T("Unknown"));
	int iIndex = 0;

	if ( dwID && bNumericOK )
	{
		sReturn.Format(_T("FlashID=%04X"), dwID );
	}
	while( g_flashInfo[iIndex].m_wFlashID )
	{
		if ( g_flashInfo[iIndex].m_wFlashID == (dwID&0x0000FFFF) )
		{
			sReturn = g_flashInfo[iIndex].m_szDescription;
			break;
		}
		iIndex++;
	}
	return sReturn;
}

CStdString GetIPString( void )
{
	CStdString sReturn;
	CStdString sNetType;
//	XKEEPROM eepromInfo;
//	XNADDR xnaInfo;

//	eepromInfo.ReadFromXBOX();

//	XNetStartup(NULL);

	if ( !g_MenuInfo.m_bXnaValid )
	{
		g_MenuInfo.m_dwXnaResult = XNetGetTitleXnAddr(&g_MenuInfo.m_xnaInfo);
		if ( g_MenuInfo.m_xnaInfo.ina.S_un.S_addr && g_MenuInfo.m_bNetParamsSet )
		{
			g_MenuInfo.m_bXnaValid = true;
		}
	}

//	XNetCleanup();

	sNetType = _T("");
	if ( g_MenuInfo.m_dwXnaResult&XNET_GET_XNADDR_DHCP )
	{
		sNetType += _T("DHCP ");
	}
	if ( g_MenuInfo.m_dwXnaResult&XNET_GET_XNADDR_ETHERNET )
	{
		sNetType += _T("ENET ");
	}
	if ( g_MenuInfo.m_dwXnaResult&XNET_GET_XNADDR_STATIC )
	{
		sNetType += _T("Static ");
	}

	DWORD dwIP;
	if ( g_MenuInfo.m_xnaInfo.inaOnline.S_un.S_addr )
	{
		dwIP = g_MenuInfo.m_xnaInfo.inaOnline.S_un.S_addr;
	}
	else
	{
		dwIP = g_MenuInfo.m_xnaInfo.ina.S_un.S_addr;
	}

	//DWORD dwIP_A, dwIP_B, dwIP_C, dwIP_D;
	//
	//dwIP_A = dwIP&0x0FF;
	//dwIP >>= 8;
	//dwIP_B = dwIP&0x0FF;
	//dwIP >>= 8;
	//dwIP_C = dwIP&0x0FF;
	//dwIP >>= 8;
	//dwIP_D = dwIP&0x0FF;
	
	// sReturn.Format( _T("%ld.%ld.%ld.%ld %s"), dwIP_A, dwIP_B, dwIP_C, dwIP_D, sNetType.c_str() );

	sReturn.Format( _T("%s %s"), MakeIPString(dwIP).c_str(), sNetType.c_str() );

	return sReturn;
}

CStdString MakeIPString( DWORD dwIPAddr )
{
	CStdString sReturn;
	DWORD dwIP_A, dwIP_B, dwIP_C, dwIP_D;
	
	dwIP_A = dwIPAddr&0x0FF;
	dwIPAddr >>= 8;
	dwIP_B = dwIPAddr&0x0FF;
	dwIPAddr >>= 8;
	dwIP_C = dwIPAddr&0x0FF;
	dwIPAddr >>= 8;
	dwIP_D = dwIPAddr&0x0FF;
	
	sReturn.Format( _T("%ld.%ld.%ld.%ld"), dwIP_A, dwIP_B, dwIP_C, dwIP_D );

	return sReturn;
}

BOOL IsEthernetConnected()
{
	if (!(XNetGetEthernetLinkStatus() & XNET_ETHERNET_LINK_ACTIVE))
		return FALSE;

	return TRUE;
}

CStdString MakeNetParamsString( TXNetConfigParams * pConfigParams )
{
	CStdString sReturn;
	CStdString sTemp;
	CStdString sIP, sGateway, sDNS1, sDNS2, sSubnet;


	sTemp.Format( _T("Data1: %08x %08x %08x %08x %08x"), 
			pConfigParams->Data_00, 
			pConfigParams->Data_04, 
			pConfigParams->Data_08, 
			pConfigParams->Data_0c, 
			pConfigParams->Data_10 );

	sReturn = sTemp;

	sTemp.Format( _T("\r\nData2: %08x %08x %08x %08x %08x"), 
			pConfigParams->Data_28, 
			pConfigParams->Data_2c, 
			pConfigParams->Data_30, 
			pConfigParams->Data_34, 
			pConfigParams->Data_38 );

	sReturn += sTemp;

	sIP = MakeIPString( pConfigParams->V1_IP );
	sGateway = MakeIPString( pConfigParams->V1_Defaultgateway );
	sDNS1 = MakeIPString( pConfigParams->V1_DNS1 );
	sDNS2 = MakeIPString( pConfigParams->V1_DNS2 );
	sSubnet = MakeIPString( pConfigParams->V1_Subnetmask );

	sTemp.Format( _T("\r\nV1: IP=%s Gateway=%s\r\nV1: DNS1=%s DNS2=%s\r\nV1: Subnet=%s"), sIP.c_str(),
		sGateway.c_str(), sDNS1.c_str(), sDNS2.c_str(), sSubnet.c_str() );

	sReturn += sTemp;

	sIP = MakeIPString( pConfigParams->V2_IP );
	sGateway = MakeIPString( pConfigParams->V2_Defaultgateway );
	sDNS1 = MakeIPString( pConfigParams->V2_DNS1 );
	sDNS2 = MakeIPString( pConfigParams->V2_DNS2 );
	sSubnet = MakeIPString( pConfigParams->V2_Subnetmask );

	sTemp.Format( _T("\r\nV2: IP=%s Gateway=%s\r\nV2: DNS1=%s DNS2=%s\r\nV2: Subnet=%s"), sIP.c_str(),
		sGateway.c_str(), sDNS1.c_str(), sDNS2.c_str(), sSubnet.c_str() );

	sReturn += sTemp;

	sTemp.Format( _T("\r\nFlag = 0x%08x  Tag=\"%c%c%c%c\" Data_44 = 0x%08x"), pConfigParams->Flag, 
		((char *)(&pConfigParams->V2_Tag))[0],
		((char *)(&pConfigParams->V2_Tag))[1],
		((char *)(&pConfigParams->V2_Tag))[2],
		((char *)(&pConfigParams->V2_Tag))[3],		
		pConfigParams->Data_44 );
	
	sReturn += sTemp;


#if 0
typedef struct   
{   
  

	DWORD	V2_Tag;             // V2 Tag "XBV2"   
 
	DWORD   Data_xx[0x200-0x5c]; // 420

} TXNetConfigParams,*PTXNetConfigParams;   
#endif
	return sReturn;
}

bool SetNetParams( void )
{
	bool bReturn = false;

	sockaddr_in	saHost;

	TXNetConfigParams configParams; 

	XNetLoadConfigParams( (LPBYTE) &configParams );
	
//	SaveBufferToFile( _T("E:\\NetParams.bin"), (LPBYTE) &configParams, sizeof(TXNetConfigParams) );
//	SaveStringToFile( _T("E:\\NetParams.txt"), MakeNetParamsString(&configParams) );
	

	bool bXboxVersion2 = (configParams.V2_Tag == 0x58425632 ); // "XBV2"
	bool bDirty = false;


	if ( g_MenuInfo.NetParams.m_sIP.GetLength() )
	{
		if (bXboxVersion2)
		{
			if (configParams.V2_IP != inet_addr(g_MenuInfo.NetParams.m_sIP.c_str()))
			{
				configParams.V2_IP = inet_addr(g_MenuInfo.NetParams.m_sIP.c_str());
				bDirty = true;
			}
			saHost.sin_addr.S_un.S_addr = configParams.V2_IP;
		}
		else
		{
			if (configParams.V1_IP != inet_addr(g_MenuInfo.NetParams.m_sIP.c_str()))
			{
				configParams.V1_IP = inet_addr(g_MenuInfo.NetParams.m_sIP.c_str());
				bDirty = true;
			}
			saHost.sin_addr.S_un.S_addr = configParams.V1_IP;
		}
	}

	if ( g_MenuInfo.NetParams.m_sSubnetmask.GetLength() )
	{
		if (bXboxVersion2)
		{
			if (configParams.V2_Subnetmask != inet_addr(g_MenuInfo.NetParams.m_sSubnetmask.c_str()))
			{
				configParams.V2_Subnetmask = inet_addr(g_MenuInfo.NetParams.m_sSubnetmask.c_str());
				bDirty = true;
			}
		}
		else
		{
			if (configParams.V1_Subnetmask != inet_addr(g_MenuInfo.NetParams.m_sSubnetmask.c_str()))
			{
				configParams.V1_Subnetmask = inet_addr(g_MenuInfo.NetParams.m_sSubnetmask.c_str());
				bDirty = true;
			}
		}
	}


	if ( g_MenuInfo.NetParams.m_sDefaultgateway.GetLength() )
	{
		if (bXboxVersion2)
		{
			if (configParams.V2_Defaultgateway != inet_addr(g_MenuInfo.NetParams.m_sDefaultgateway.c_str()))
			{
				configParams.V2_Defaultgateway = inet_addr(g_MenuInfo.NetParams.m_sDefaultgateway.c_str());
				bDirty = true;
			}
		}
		else
		{
			if (configParams.V1_Defaultgateway != inet_addr(g_MenuInfo.NetParams.m_sDefaultgateway.c_str()))
			{
				configParams.V1_Defaultgateway = inet_addr(g_MenuInfo.NetParams.m_sDefaultgateway.c_str());
				bDirty = true;
			}
		}
	}

	if ( g_MenuInfo.NetParams.m_sDNS1.GetLength() )
	{

		if (bXboxVersion2)
		{
			if (configParams.V2_DNS1 != inet_addr(g_MenuInfo.NetParams.m_sDNS1.c_str()))
			{
				configParams.V2_DNS1 = inet_addr(g_MenuInfo.NetParams.m_sDNS1.c_str());
				bDirty = true;
			}
		}
		else
		{
			if (configParams.V1_DNS1 != inet_addr(g_MenuInfo.NetParams.m_sDNS1.c_str()))
			{
				configParams.V1_DNS1 = inet_addr(g_MenuInfo.NetParams.m_sDNS1.c_str());
				bDirty = true;
			}
		}
	}

	if ( g_MenuInfo.NetParams.m_sDNS2.GetLength() )
	{

		if (bXboxVersion2)
		{
			if (configParams.V2_DNS2 != inet_addr(g_MenuInfo.NetParams.m_sDNS2.c_str()))
			{
				configParams.V2_DNS2 = inet_addr(g_MenuInfo.NetParams.m_sDNS2.c_str());
				bDirty = true;
			}
		}
		else
		{
			if (configParams.V1_DNS2 != inet_addr(g_MenuInfo.NetParams.m_sDNS2.c_str()))
			{
				configParams.V1_DNS2 = inet_addr(g_MenuInfo.NetParams.m_sDNS2.c_str());
				bDirty = true;
			}
		}
	}

	if ( g_MenuInfo.NetParams.m_iUseDHCP == 1 )
	{
		if (configParams.Flag != (0) )
		{
			configParams.Flag = 0;
			bDirty = true;
		}
	}
	else if ( g_MenuInfo.NetParams.m_iUseDHCP == 0 )
	{
		if (configParams.Flag != (0x04|0x08) )
		{
			configParams.Flag = 0x04 | 0x08;
			bDirty = true;
		}
	}

	g_MenuInfo.CurrentNetParams.m_iUseDHCP = (configParams.Flag==0)?1:0;

	if (bXboxVersion2)
	{
		g_MenuInfo.CurrentNetParams.m_sIP = MakeIPString( configParams.V2_IP );
		g_MenuInfo.CurrentNetParams.m_sSubnetmask = MakeIPString( configParams.V2_Subnetmask );
		g_MenuInfo.CurrentNetParams.m_sDefaultgateway = MakeIPString( configParams.V2_Defaultgateway );
		g_MenuInfo.CurrentNetParams.m_sDNS1 = MakeIPString( configParams.V2_DNS1 );
		g_MenuInfo.CurrentNetParams.m_sDNS2 = MakeIPString( configParams.V2_DNS2 );
	}
	else
	{
		g_MenuInfo.CurrentNetParams.m_sIP = MakeIPString( configParams.V1_IP );
		g_MenuInfo.CurrentNetParams.m_sSubnetmask = MakeIPString( configParams.V1_Subnetmask );
		g_MenuInfo.CurrentNetParams.m_sDefaultgateway = MakeIPString( configParams.V1_Defaultgateway );
		g_MenuInfo.CurrentNetParams.m_sDNS1 = MakeIPString( configParams.V1_DNS1 );
		g_MenuInfo.CurrentNetParams.m_sDNS2 = MakeIPString( configParams.V1_DNS2 );
	}

	if (bDirty)
	{
		XNetSaveConfigParams( (LPBYTE) &configParams );
		bReturn = true;
	}

	if ( !g_MenuInfo.m_bXnaValid )
	{
		g_MenuInfo.m_dwXnaResult = XNetGetTitleXnAddr(&g_MenuInfo.m_xnaInfo);
		if ( g_MenuInfo.m_xnaInfo.ina.S_un.S_addr && g_MenuInfo.m_bNetParamsSet )
		{
			g_MenuInfo.m_bXnaValid = true;
		}
	}
	else
	{
		if ( g_MenuInfo.m_xnaInfo.ina.S_un.S_addr == 0 && saHost.sin_addr.S_un.S_addr )
		{
			g_MenuInfo.m_xnaInfo.ina.S_un.S_addr = saHost.sin_addr.S_un.S_addr;
		}
	}

	g_MenuInfo.m_bNetParamsSet = true;

	return bReturn;
}


BOOL InitializeNetwork()
{
	BOOL bReturn = FALSE;
	//TCPIP_USER_SETTINGS tcpParams;

	//tcpParams.m_sSubnetMask = _T("255.255.255.0");
	//tcpParams.m_sGateway = _T("192.168.0.1");
	//tcpParams.m_sDNS1 = _T("192.168.0.1");

	SetNetParams();
	
	if (IsEthernetConnected())
	{

		// allocate enough memory for heavy network load
		XNetStartupParams xnsp;
		memset(&xnsp, 0, sizeof(xnsp));
		xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
		// create more memory for networking
		xnsp.cfgPrivatePoolSizeInPages = 64; // == 256kb, default = 12 (48kb)
		xnsp.cfgEnetReceiveQueueLength = 16; // == 32kb, default = 8 (16kb)
		xnsp.cfgIpFragMaxSimultaneous = 16; // default = 4
		xnsp.cfgIpFragMaxPacketDiv256 = 32; // == 8kb, default = 8 (2kb)
		xnsp.cfgSockMaxSockets = 64; // default = 64
		xnsp.cfgSockDefaultRecvBufsizeInK = 128; // default = 16
		xnsp.cfgSockDefaultSendBufsizeInK = 128; // default = 16


		// Bypass security so that we may connect to 'untrusted' hosts
		xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
		INT err = XNetStartup(&xnsp);

		g_bUseNetRandom = true;

		WSADATA WsaData;
		err = WSAStartup( MAKEWORD(2,2), &WsaData );
		
		g_sIP = GetIPString();

		bReturn = ( err == NO_ERROR );
	}
	return bReturn;
}



CStdString GetDiskFree( char cDrive, DWORD dwDivisor, bool bShort )
{
	CStdString sReturn;
	ULARGE_INTEGER u64Free, u64FreeTotal, u64Total;
	LPCSTR szDirectory;

	cDrive = tolower(cDrive);
	switch ( cDrive )
	{
		case 'c':
			szDirectory = _T("C:\\");
			break;
		case 'e':
			szDirectory = _T("E:\\");
			break;
		case 'f':
			szDirectory = _T("F:\\");
			break;
		case 'g':
			szDirectory = _T("G:\\");
			break;
	}
	if ( GetDiskFreeSpaceEx( szDirectory, &u64Free, &u64Total, &u64FreeTotal ) )
	{
		//qwTotalBytes = lTotalNumberOfBytes.QuadPart;
		//qwFreeBytes  = lFreeBytesAvailable.QuadPart;
		//qwUsedBytes  = qwTotalBytes - qwFreeBytes;

		ULONGLONG u64FreeBytes = u64Free.QuadPart;
		if ( bShort )
		{
			dwDivisor = 0;
		}
		else if ( dwDivisor )
		{
			u64FreeBytes /= (ULONGLONG)dwDivisor;
		}
		sReturn = MakeNiceNumber( u64FreeBytes, _T(','), bShort, true );
		// sReturn.Format( _T("%I64d"), u64FreeBytes );
	}
	return sReturn;
}

CStdString GetDiskTotal( char cDrive, DWORD dwDivisor, bool bShort )
{
	CStdString sReturn;
	ULARGE_INTEGER u64Free, u64FreeTotal, u64Total;
	LPCSTR szDirectory;

	cDrive = tolower(cDrive);
	switch ( cDrive )
	{
		case 'c':
			szDirectory = _T("C:\\");
			break;
		case 'e':
			szDirectory = _T("E:\\");
			break;
		case 'f':
			szDirectory = _T("F:\\");
			break;
		case 'g':
			szDirectory = _T("G:\\");
			break;
	}
	if ( GetDiskFreeSpaceEx( szDirectory, &u64Free, &u64Total, &u64FreeTotal ) )
	{
		//qwTotalBytes = lTotalNumberOfBytes.QuadPart;
		//qwFreeBytes  = lFreeBytesAvailable.QuadPart;
		//qwUsedBytes  = qwTotalBytes - qwFreeBytes;

		ULONGLONG u64TotalBytes = u64Total.QuadPart;
		if ( bShort )
		{
			dwDivisor = 0;
		}
		else if ( dwDivisor )
		{
			u64TotalBytes /= (ULONGLONG)dwDivisor;
		}
		sReturn = MakeNiceNumber( u64TotalBytes, _T(','), bShort, true );
		// sReturn.Format( _T("%I64d"), u64TotalBytes );
	}
	return sReturn;
}

CStdString GetDiskUsed( char cDrive, DWORD dwDivisor, bool bShort )
{
	CStdString sReturn;
	ULARGE_INTEGER u64Free, u64FreeTotal, u64Total;
	LPCSTR szDirectory;

	cDrive = tolower(cDrive);
	switch ( cDrive )
	{
		case 'c':
			szDirectory = _T("C:\\");
			break;
		case 'e':
			szDirectory = _T("E:\\");
			break;
		case 'f':
			szDirectory = _T("F:\\");
			break;
		case 'g':
			szDirectory = _T("G:\\");
			break;
	}
	if ( GetDiskFreeSpaceEx( szDirectory, &u64Free, &u64Total, &u64FreeTotal ) )
	{
		//qwTotalBytes = lTotalNumberOfBytes.QuadPart;
		//qwFreeBytes  = lFreeBytesAvailable.QuadPart;
		//qwUsedBytes  = qwTotalBytes - qwFreeBytes;

		ULONGLONG u64FreeBytes = u64Free.QuadPart;
		ULONGLONG u64TotalBytes = u64Total.QuadPart;
		ULONGLONG u64UsedBytes = u64TotalBytes-u64FreeBytes;
		if ( bShort )
		{
			dwDivisor = 0;
		}
		else if ( dwDivisor )
		{
			u64UsedBytes /= (ULONGLONG)dwDivisor;
		}
		sReturn = MakeNiceNumber( u64UsedBytes, _T(','), bShort, true );
		// sReturn.Format( _T("%I64d"), u64UsedBytes );
	}
	return sReturn;
}

CStdString MakeNiceNumber( ULONGLONG & u64Value, TCHAR tcComma, bool bShort, bool bBytes )
{
	CStdString sReturn;
	ULONGLONG u64Trillions, u64Billions, u64Millions, u64Thousands, u64Remainder;

	u64Thousands = u64Value/((ULONGLONG)1000);
	u64Remainder = u64Value-(u64Thousands*((ULONGLONG)1000));
	u64Millions = u64Thousands/((ULONGLONG)1000);
	u64Thousands = u64Thousands-(u64Millions*((ULONGLONG)1000));
	u64Billions = u64Millions/((ULONGLONG)1000);
	u64Millions = u64Millions-(u64Billions*((ULONGLONG)1000));
	u64Trillions = u64Billions/((ULONGLONG)1000);
	u64Billions = u64Billions-(u64Trillions*((ULONGLONG)1000));
	if ( u64Trillions )
	{
		if ( bShort )
		{
			sReturn.Format( _T("%I64d%c%03I64d%s"), 
					u64Trillions, tcComma, 
					u64Billions, bBytes? _T("GB"):_T("") );
		}
		else
		{
			sReturn.Format( _T("%I64d%c%03I64d%c%03I64d%c%03I64d%c%03I64d"), 
					u64Trillions, tcComma, 
					u64Billions, tcComma, 
					u64Millions, tcComma, 
					u64Thousands, tcComma, 
					u64Remainder );
		}
	}
	else if ( u64Billions )
	{
		if ( bShort )
		{
			sReturn.Format( _T("%I64d%c%03I64d%s"), 
					u64Billions, tcComma, 
					u64Millions, bBytes?_T("MB"):_T("") );
		}
		else
		{
			sReturn.Format( _T("%I64d%c%03I64d%c%03I64d%c%03I64d"), 
					u64Billions, tcComma, 
					u64Millions, tcComma, 
					u64Thousands, tcComma, 
					u64Remainder );
		}
	}
	else if ( u64Millions )
	{
		if ( bShort )
		{
			sReturn.Format( _T("%I64d%c%03I64d%s"), 
					u64Millions, tcComma, 
					u64Thousands, bBytes?_T("KB"):_T("") );
		}
		else
		{
			sReturn.Format( _T("%I64d%c%03I64d%c%03I64d"), 
					u64Millions, tcComma, 
					u64Thousands, tcComma, 
					u64Remainder );
		}
	}
	else if ( u64Thousands )
	{
		if ( bShort )
		{
			sReturn.Format( _T("%I64d%c%03I64d%s"), 
					u64Thousands, tcComma, 
					u64Remainder, bBytes?_T("bytes"):_T("") );
		}
		else
		{
			sReturn.Format( _T("%I64d%c%03I64d"), 
					u64Thousands, tcComma, 
					u64Remainder );
		}
	}
	else
	{
		if ( bShort )
		{
			sReturn.Format( _T("%I64d%s"), 
					u64Remainder, bBytes?_T("bytes"):_T("") );
		}
		else
		{
			sReturn.Format( _T("%I64d"), 
					u64Remainder );
		}
	}
	return sReturn;
}

bool CheckTime( void )
{
	bool bReturn = false;

	// If it's less than July 1, 2003, set it to July 1, 2003, 4:20pm
	SYSTEMTIME sysTime;
	SYSTEMTIME sysTime2;
	FILETIME currTime;
	FILETIME compTime;
	FILETIME compTime2;

	sysTime.wYear = 2003;
	sysTime.wMonth = 7;
	sysTime.wDay = 1;
	sysTime.wHour = 14;
	sysTime.wMinute = 0;
	sysTime.wSecond = 0;
	sysTime.wMilliseconds = 2;

	SystemTimeToFileTime( &sysTime, &compTime );
	GetSystemTimeAsFileTime( &currTime );
	if ( CompareFileTime( &currTime, &compTime ) < 0 )
	{
		bReturn = true;
		NtSetSystemTime(&compTime,&currTime);  // sets system time
	}
	else
	{
		sysTime2.wYear = 2003;
		sysTime2.wMonth = 7;
		sysTime2.wDay = 1;
		sysTime2.wHour = 14;
		sysTime2.wMinute = 0;
		sysTime2.wSecond = 0;
		sysTime2.wMilliseconds = 2;

		SystemTimeToFileTime( &sysTime2, &compTime2 );
		if ( CompareFileTime( &currTime, &compTime2 ) < 0 )
		{
			bReturn = true;
			NtSetSystemTime(&compTime,&currTime);  // sets system time
		}
	}
	return bReturn;
}


LONGLONG DirectorySpace( LPCTSTR szDirectoryPath )
{
	LONGLONG llValue = 0;
	LONGLONG llResult = 0;
	LARGE_INTEGER liSize;

	CStdString sWildCard;
	HANDLE hFind;
	WIN32_FIND_DATA ffData;
	CStdString sFilename;
	CStdString sNewPath;


	if ( szDirectoryPath && _tcslen( szDirectoryPath ) )
	{
		sWildCard = MakeFullFilePath( szDirectoryPath, _T("*.*") );
		hFind = FindFirstFile( sWildCard, &ffData );

		if( INVALID_HANDLE_VALUE != hFind )
		{
			// hr = S_OK;
			do
			{
				if ( ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
				{
					sNewPath = MakeFullFilePath( szDirectoryPath, ffData.cFileName );
					llValue = DirectorySpace( sNewPath );
					llResult += llValue;
				}
				else
				{
					// Copy file over, unless ZERO byte file?
					if ( ffData.nFileSizeLow || ffData.nFileSizeHigh )
					{
						liSize.LowPart = ffData.nFileSizeLow;
						liSize.HighPart = ffData.nFileSizeHigh;
						llValue += liSize.QuadPart;
					}
					else
					{
						// If ZERO byte, just open and close a file on the target directory
					}
				}
			} while( FindNextFile( hFind, &ffData ) );
			// Close the find handle.
			FindClose( hFind );
		}
	}
	return llValue;
}

static struct
{
	LPCTSTR m_szName;
	DWORD m_dwLight;
	DWORD m_dwMedium;
	DWORD m_dwDark;
} s_ColorTable[] = 
{
	{ _T("white"), 0xffffffff, 0xffffffff, 0xffffffff },
	{ _T("black"), 0xff000000, 0xff000000, 0xff000000 },
	{ _T("gray"),  0xffc0c0c0, 0xff808080, 0xff404040 },
	{ _T("red"),   0xffff4040, 0xffff0000, 0xff800000 },
	{ _T("blue"),  0xff4040ff, 0xff0000ff, 0xff000080 },
	{ _T("green"), 0xff40ff40, 0xff00ff00, 0xff008000 },
	{ _T("yellow"),0xffffff40, 0xffffff00, 0xff808000 },
	{ _T("cyan"),  0xff40ffff, 0xff00ffff, 0xff008080 },
	{ _T("violet"),0xffff40ff, 0xffff00ff, 0xff800080 },
	{ _T("orange"),0xffff8040, 0xffff8000, 0xff804000 },
	{ _T("brown"), 0xff808040, 0xff808000, 0xff404000 },
	{ NULL, 0, 0, 0 }
};

DWORD ConvertColorName( LPCTSTR szColor )
{
	DWORD dwReturn = 0;
	bool bLight = false;
	bool bDark = false;
	CStdString sColor;

	if ( _tcsnicmp( szColor, _T("light"), 5 ) == 0 )
	{
		bLight = true;
		szColor += 5;
	}
	else if ( _tcsnicmp( szColor, _T("dark"), 4 ) == 0 )
	{
		bDark = true;
		szColor += 4;
	}

	sColor = szColor;
	sColor.Trim();
	int iIndex = 0;

	while( s_ColorTable[iIndex].m_szName )
	{
		if ( _tcsicmp( sColor, s_ColorTable[iIndex].m_szName ) == 0 )
		{
			if ( bLight )
			{
				dwReturn = s_ColorTable[iIndex].m_dwLight;
			}
			else if ( bDark )
			{
				dwReturn = s_ColorTable[iIndex].m_dwDark;
			}
			else
			{
				dwReturn = s_ColorTable[iIndex].m_dwMedium;
			}
			break;
		}
		iIndex++;
	}
	if ( dwReturn == 0 && szColor )
	{
		int iBase = 10;
		// Is the name a hex number?
		if ( (_tcslen( szColor ) > 2) )
		{
			if ( szColor[0] == _T('0') && _toupper(szColor[1]) == _T('X') )
			{
				szColor += 2;
				iBase = 16;
			}
		}
		if ( _tcslen( szColor ) )
		{
			if ( szColor[0] == _T('-') )
			{
				dwReturn = _tcstol( szColor, NULL, iBase );
			}
			else
			{
				dwReturn = _tcstoul( szColor, NULL, iBase );
			}
		}
	}
	return dwReturn;
}


HRESULT CopyToHardDrive( LPCTSTR szSource, LPCTSTR szDest )
{
	HRESULT hr = E_FAIL;
	CStdString sWildCard;
	HANDLE hFind;
	WIN32_FIND_DATA ffData;
	CStdString sFilename;
	CStdString sNewPath;
	CStdString sDestFilename;
	CStdString sNewDestPath;

	// Copy from one directory to another....
	// Includes subdirectories (nests these calls)
	sWildCard = MakeFullFilePath( szSource, _T("*.*") );
	hFind = FindFirstFile( sWildCard, &ffData );

	if( INVALID_HANDLE_VALUE != hFind )
	{
		hr = S_OK;
		do
		{
			if ( ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
			{
				sNewPath = MakeFullFilePath( szSource, ffData.cFileName );
				sNewDestPath = MakeFullFilePath( szDest, ffData.cFileName );
				CopyToHardDrive( sNewPath, sNewDestPath );
			}
			else
			{
				// Copy file over, unless ZERO byte file?
				if ( ffData.nFileSizeLow || ffData.nFileSizeHigh )
				{
				}
				else
				{
					// If ZERO byte, just open and close a file on the target directory
				}
			}
		} while( FindNextFile( hFind, &ffData ) );
		// Close the find handle.
		FindClose( hFind );
	}


	return hr;
}


CStdString MakeFullDirPath( LPCTSTR szMainPath, LPCTSTR szPath, BOOL bStripTrailingSlash )
{
	CStdString sPath, sMainPath;
	CStdString sReturn;
	int iPos;
	bool bNotMemFile = true;

	sPath = szPath;
	sMainPath = szMainPath;

	sMainPath.Replace( _T("\\\\"), _T("\\") );
	sMainPath.Replace( _T("\\\\"), _T("\\") );
	sMainPath.Replace( _T("\\\\"), _T("\\") );
	sMainPath.Replace( _T("/"), _T("\\") );

	sPath.ToUpper();
	sMainPath.ToUpper();

	if (  ( sPath.GetLength() > 1 ) && (sPath[0] == _T(':')) && (sPath[0] == _T(':')) )
	{
		bNotMemFile = false;
	}
	if ( bNotMemFile )
	{
		// Invalid filename?
		if ( ( sMainPath.GetLength()<3) || (sMainPath[1] != _T(':')) || (sMainPath[2] != _T('\\'))) 
		{
			sMainPath = _T("D:\\");
		}

		// Insure mainpath has '\' at the end of it!
		iPos = sMainPath.GetLength()-1;
		if ( sMainPath[iPos] != _T('\\') )
		{
			sMainPath += _T('\\');
		}
		if ( sPath.GetLength() )
		{
			if ( sPath.Find(_T("D:\\"))==0 )
			{
				// Must be a directory
				sReturn = sPath;
			}
			else if ( sPath.Find(_T("C:\\"))==0 )
			{
				// Must be a directory
				sReturn = sPath;
			}
			else if ( sPath.Find(_T("E:\\"))==0 )
			{
				// Must be a directory
				sReturn = sPath;
			}
			else if ( sPath.Find(_T("F:\\"))==0 )
			{
				// Must be a directory
				sReturn = sPath;
			}
			else if ( sPath.Find(_T("G:\\"))==0 )
			{
				// Must be a directory
				sReturn = sPath;
			}
			else
			{
				// Is this an absolute path?
				if ( sPath[0] == _T('\\') )
				{
					// Then we get the first two characters of szMainPath!
					sReturn = sMainPath.Left(2);
					sReturn += sPath;
				}
				else
				{
					sReturn.Format( _T("%s%s"), sMainPath.c_str(), sPath.c_str() );
				}
			}
		}
		else
		{
			// Return 
			sReturn = szMainPath;
		}

		// Insure mainpath has '\' at the end of it!
		iPos = sReturn.GetLength()-1;
		if ( sReturn[iPos] != _T('\\') )
		{
			if ( !bStripTrailingSlash )
			{
				sReturn += _T('\\');
			}
		}
		else
		{
			if ( bStripTrailingSlash )
			{
				sReturn = sReturn.Left( iPos );
			}
		}
	}
	else
	{
		sReturn = sPath;
	}
	return sReturn;

}


CStdString MakeFullFilePath( LPCTSTR szMainPath, LPCTSTR szFilePath, BOOL bZapNullName )
{
	CStdString sPath, sMainPath;
	CStdString sReturn;
//	int iPos;
	bool bNotMemFile = true;

	sPath = szFilePath;
	sMainPath = szMainPath;

	sMainPath.Replace( _T("\\\\"), _T("\\") );
	sMainPath.Replace( _T("\\\\"), _T("\\") );
	sMainPath.Replace( _T("\\\\"), _T("\\") );
	sMainPath.Replace( _T("/"), _T("\\") );

	if (  ( sPath.GetLength() > 1 ) && (sPath[0] == _T(':')) && (sPath[0] == _T(':')) )
	{
		bNotMemFile = false;
	}
	if ( bNotMemFile )
	{
		if ( sPath.GetLength() )
		{
			// Is it a simple drive letter? If so, correct it as a path
			if ( ( sMainPath.GetLength()==2 ) && ( sMainPath[1] == _T(':') ) && _istalpha(sMainPath[0]) )
			{
				sMainPath += _T("\\");
			}
			// Invalid filename?
			if ( ( sMainPath.GetLength()<3) || (sMainPath[1] != _T(':')) || (sMainPath[2] != _T('\\')) || !_istalpha(sMainPath[0])) 
			{
				sMainPath = _T("D:\\");
			}

			// Insure mainpath has '\' at the end of it!
			sMainPath = PathSlasher( sMainPath, true );
#if 0
			iPos = sMainPath.GetLength()-1;
			if ( sMainPath[iPos] != _T('\\') )
			{
				sMainPath += _T('\\');
			}
#endif
			if ( sPath.GetLength() )
			{
				// Is this an absolute path?
				//if ( sPath[0] == _T('*') ) // Wildcard?
				//{
				//	sReturn = sMainPath+sPath;
				//}
				//else 
					if ( sPath[0] == _T('\\') )
				{
					// Then we get the first two characters of szMainPath!
					sReturn.Format( _T("%s%s"), sMainPath.Left(2).c_str(), sPath.c_str() );
				}
				else
				{
					if ( sPath.GetLength() > 2 )
					{
						if ( (_istalpha(sPath[0])) && (sPath[1] == _T(':')) && (sPath[2] == _T('\\') ))
						{
							// Absolute path with drive letter. Just passs it back!
							sReturn = sPath;
						}
						else
						{
							sReturn.Format( _T("%s%s"), sMainPath.c_str(), sPath.c_str() );
						}
					}
					else
					{
						// sReturn = sMainPath+sPath;		
						sReturn.Format( _T("%s%s"), sMainPath.c_str(), sPath.c_str() );
					}
				}
			}
			else
			{
				// Return default xbe if no filename
				sReturn.Format( _T("%s%s"), szMainPath, _T("default.xbe") );
			}
		}
		else
		{
			sReturn = _T("");
		}
	}
	else
	{
		sReturn = sPath;
	}

	return sReturn;

}



BOOL  FileExists( LPCTSTR szFilename )
{
	BOOL bReturn = FALSE;
	DWORD dwAttributes;

	if ( szFilename && _tcslen( szFilename ) )
	{
		// MemFile check
		if ( _tcslen( szFilename ) > 1 )
		{
			if ( szFilename[0] == _T(':') && szFilename[1] == _T(':') )
			{
				bReturn = true;
			}
		}

		if ( !bReturn )
		{
			dwAttributes = GetFileAttributes( szFilename );
			if ( dwAttributes != (DWORD)-1 )
			{
				bReturn = TRUE;
			}
		}
	}

	return bReturn;
}


// Thanks, Nightshade...
int GetFanSpeed( void )
{
	int iReturn = 0;
	int bySpeed;
//	BYTE bySpeed;
	// read it
	HalReadSMBusValue( 0x20, 0x10, 0, (LPBYTE)&bySpeed );

	iReturn = (int)bySpeed;
	return iReturn;
}

void SetFanSpeed( int iFanSpeed )
{
	// enable user specified speed
	HalWriteSMBusValue( 0x20, 0x05, 0, 1 ); 

	Sleep( 10 );
	// write it
	HalWriteSMBusValue( 0x20, 0x06, 0, iFanSpeed );

	Sleep( 10 );
	// Lock it
	HalWriteSMBusValue( 0x20, 0x05, 0, 0 ); 
}


DWORD FileLength( LPCTSTR szFilename )
{
	DWORD dwReturn = 0;

	if ( FileExists(szFilename) )
	{
		HANDLE hTemp;
		hTemp = CreateFile( szFilename, 
					   GENERIC_READ, 
					   FILE_SHARE_READ, 
					   NULL,
					   OPEN_EXISTING,
					   FILE_ATTRIBUTE_NORMAL,
					   NULL );
		if ( hTemp != INVALID_HANDLE_VALUE )
		{
			dwReturn = GetFileSize( hTemp, NULL );
		}
		CloseHandle( hTemp );
	}
	return dwReturn;
}



bool InitUniString( PUNICODE_STRING pUniString, int iMaxLen, bool bRawInit )
{
	bool bReturn = false;

	if ( !bRawInit )
	{
		// Delete anything pre-existing
		if ( pUniString->Buffer )
		{
			delete [] pUniString->Buffer;
		}
	}
	pUniString->Length = 0;
	pUniString->Buffer = NULL;
	pUniString->MaximumLength = 0;
	if ( iMaxLen )
	{
		pUniString->Buffer = new char[iMaxLen];
		if ( pUniString->Buffer )
		{
			memset(pUniString->Buffer, 0, sizeof(char)*iMaxLen);
			pUniString->MaximumLength = iMaxLen;
			bReturn = true;
		}
	}
	else
	{
		// Empty uni
		bReturn = true;
	}
	return bReturn;
}


bool SetUniString(  PUNICODE_STRING pUniString, LPCTSTR szStr )
{
	bool bReturn = false;

	if ( szStr )
	{
		if ( (!pUniString->Buffer)||(pUniString->MaximumLength < (_tcslen( szStr )+1) ))
		{
			InitUniString( pUniString, _tcslen(szStr)+1, false );
		}
		if ( (pUniString->Buffer)&&(pUniString->MaximumLength > _tcslen( szStr ) ))
		{
			int iIndex;

			for( iIndex=0; iIndex<(int)_tcslen( szStr ); iIndex++ )
			{
				((char*)pUniString->Buffer)[iIndex] = (char)(szStr[iIndex]);
			}
			pUniString->Length = iIndex;
			((char*)pUniString->Buffer)[iIndex] = 0;
			bReturn = true;
		}
	}
	else
	{
		// Just clear out the uni...
		InitUniString( pUniString, 0, false );
	}
	return bReturn;
}



CUniString::CUniString()
{
	InitUniString( &m_uniData, 0 );
}

CUniString::~CUniString()
{
	if ( m_uniData.Buffer )
	{
		delete [] m_uniData.Buffer;
		m_uniData.Buffer = NULL;
		m_uniData.MaximumLength = 0;
		m_uniData.Length = 0;
	}
}

PUNICODE_STRING CUniString::GetUnicodeString( void )
{
	PUNICODE_STRING pReturn = NULL;

	// Prep the UNICODE string area with the data from the string
	if ( SetUniString( &m_uniData, m_sActualData.c_str() ) )
	{
		pReturn = &m_uniData;
	}
	return pReturn;
}


CStdString* CUniString::GetString( void )
{
	return &m_sActualData;
}


LPCTSTR szCD = _T("\\Device\\Cdrom0");
LPCTSTR szHD = _T("\\Device\\Harddisk0\\Partition");

CStdString ExtractFilenameFromPath( LPCTSTR szPath, bool bDotted )
{
	CStdString sReturn(_T(""));
	CStdString sPath(szPath);

	int iSlashPos, iDotPos;
	iDotPos = sPath.ReverseFind( _T(".") );
	iSlashPos = sPath.ReverseFind( _T("\\") );

	// Must have "DOT" to be a filename.
	if ( (!bDotted) || (iDotPos > (-1)) )
	{
		if ( iSlashPos > (-1) )
		{
			sReturn = sPath.Mid(iSlashPos+1);
		}
		else
		{
			// No slashes. Just return what we have!
			sReturn = sPath;
		}
	}
	return sReturn;
}

// Strip out any filename and trailing slash
CStdString StripFilenameFromPath( LPCTSTR szPath )
{
	CStdString sReturn;
	CStdString sPath(szPath);

	int iSlashPos, iDotPos;

	iDotPos = sPath.ReverseFind( _T(".") );
	iSlashPos = sPath.ReverseFind( _T("\\") );
	if ( iDotPos > (-1) && iSlashPos > (-1) )
	{
		if ( iDotPos > iSlashPos )
		{
			sReturn = sPath.Left( iSlashPos );
		}
		else
		{
			// Only remove slashes if they are at the very end!
			if ( iSlashPos == sPath.GetLength()-1)
			{
				sReturn = sPath.Left( iSlashPos );
			}
			else
			{
				// Just return the thing... no telling WHAT is going on here
				sReturn = sPath;
			}
		}
	}
	else
	{
		if ( iDotPos == -1 )
		{
			// No dot
			if ( iSlashPos == -1 )
			{
				// No slashes? No dot?
				sReturn = sPath;
			}
			else
			{
				// No dot, but slash.
				// Only remove slashes if they are at the very end!
//				if ( iSlashPos == sPath.GetLength()-1)
//				{
				if ( iSlashPos != 2 )
				{
					sReturn = sPath.Left( iSlashPos );
				}
				else
				{
					// Must be <Drive>:\ type path
					sReturn = sPath;
				}
//				}
//				else
//				{
//					// Just return the thing... no telling WHAT is going on here
//					sReturn = sPath;
//				}
			}
		}
		else
		{
			// Dot but no slash...
			// This is a root directory!!
			sReturn = _T("");
		}
	}

	// Last chance check
	iSlashPos = sReturn.ReverseFind( _T("\\") );
	if ( (iSlashPos > 0) && iSlashPos == (sReturn.GetLength()-1) )
	{
		sReturn = sReturn.Left( iSlashPos );
	}

	// OK, if it's a root with a drive letter, put the slash back in!
	iSlashPos = sReturn.ReverseFind( _T(":") );
	if ( (iSlashPos == 1) && iSlashPos == (sReturn.GetLength()-1) )
	{
		sReturn += _T("\\");
	}

	return sReturn;
}

CStdString GetDeviceLocation( LPCTSTR szRealPath )
{
	CStdString sRealPath = StripFilenameFromPath( szRealPath );
	CStdString sReturn;
	TCHAR tcDrive;
	int iPartitionNumber;

	// Invalid path?? Must have valid drive letter and colon!!
	if (  (sRealPath.Find( _T("C:"))==0) || (sRealPath.Find( _T("c:"))==0))
	{
		sReturn.Format( _T("%s2%s"), szHD, sRealPath.Mid(2).c_str() );
	}
	else if (  (sRealPath.Find( _T("E:"))==0) || (sRealPath.Find( _T("e:"))==0))
	{
		sReturn.Format( _T("%s1%s"), szHD, sRealPath.Mid(2).c_str() );
	}
	else if (  (sRealPath.Find( _T("F:"))==0) || (sRealPath.Find( _T("f:"))==0))
	{
		sReturn.Format( _T("%s6%s"), szHD, sRealPath.Mid(2).c_str() );
	}
	else if (  (sRealPath.Find( _T("G:"))==0) || (sRealPath.Find( _T("g:"))==0))
	{
		sReturn.Format( _T("%s7%s"), szHD, sRealPath.Mid(2).c_str() );
	}
	else if ( (sRealPath.Find( _T("D:"))==0) || (sRealPath.Find( _T("d:"))==0))
	{
		sReturn.Format( _T("%s%s"), szCD, sRealPath.Mid(2).c_str() );
	}
	else if ( sRealPath[1] == _T(':') )
	{
		tcDrive = sRealPath[0];
		if ( tcDrive >= _T('G') && tcDrive <= _T('Z') )
		{
			iPartitionNumber = (tcDrive-_T('G'))+6;
		}
		if ( tcDrive >= _T('g') && tcDrive <= _T('z') )
		{
			iPartitionNumber = (tcDrive-_T('g'))+6;
		}
		else
		{
			iPartitionNumber = -1;
		}
		if ( iPartitionNumber > 0 )
		{
			sReturn.Format( _T("%s%d%s"), szHD, iPartitionNumber, sRealPath.Mid(2).c_str() );
		}
	}
	else
	{
		// Assume path is relative from D:
		if( sRealPath.GetLength() )
		{
			if ( sRealPath[0] == _T('\\') )
			{
				sReturn.Format( _T("%s%s"), szCD, sRealPath.c_str() );
			}
			else
			{
				sReturn.Format( _T("%s\\%s"), szCD, sRealPath.c_str() );
			}
		}
		else
		{
			sReturn = szCD;
		}
	}

	return sReturn;
}


// #define CdRom "\\??\\D:"
UNICODE_STRING DDeviceName = 
{
	strlen(CdRom),
	strlen(CdRom)+1,
	CdRom
};

UNICODE_STRING DDrivePath = 
{
	strlen(DriveD),
	strlen(DriveD)+1,
	DriveD
};

// "\\??\\D:"

UNICODE_STRING CDeviceName = 
{
	strlen(CDrive),
	strlen(CDrive)+1,
	CDrive
};


OBJECT_ATTRIBUTES DDriveObject = 
{
    NULL,
    &DDrivePath,
    OBJ_OPENLINK,
};


// szDrive e.g. "D:"
// szDevice e.g. "Cdrom0" or "Harddisk0\Partition6"

HRESULT Mount(const char* szDrive, char* szDevice)
{
	char szSourceDevice[256];
	char szDestinationDrive[16];

	sprintf(szSourceDevice,"\\Device\\%s",szDevice);
	sprintf(szDestinationDrive,"\\??\\%s",szDrive);

	UNICODE_STRING DeviceName =
	{
		strlen(szSourceDevice),
		strlen(szSourceDevice) + 1,
		szSourceDevice
	};

	UNICODE_STRING LinkName =
	{
		strlen(szDestinationDrive),
		strlen(szDestinationDrive) + 1,
		szDestinationDrive
	};

	IoCreateSymbolicLink(&LinkName, &DeviceName);

	return S_OK;
}



// szDrive e.g. "D:"

HRESULT Unmount(const char* szDrive)
{
	char szDestinationDrive[16];
	sprintf(szDestinationDrive,"\\??\\%s",szDrive);

	UNICODE_STRING LinkName =
	{
		strlen(szDestinationDrive),
		strlen(szDestinationDrive) + 1,
		szDestinationDrive
	};

	IoDeleteSymbolicLink(&LinkName);
	
	return S_OK;
}

// DEVICE_TYPEs (I took a guess as to which the XBOX might have.)
#define FILE_DEVICE_CD_ROM              0x00000002
#define FILE_DEVICE_CD_ROM_FILE_SYSTEM  0x00000003
#define FILE_DEVICE_CONTROLLER          0x00000004
#define FILE_DEVICE_DISK                0x00000007
#define FILE_DEVICE_DISK_FILE_SYSTEM    0x00000008
#define FILE_DEVICE_FILE_SYSTEM         0x00000009
#define FILE_DEVICE_NULL                0x00000015
#define FILE_DEVICE_SCREEN              0x0000001c
#define FILE_DEVICE_SOUND               0x0000001d
#define FILE_DEVICE_UNKNOWN             0x00000022
#define FILE_DEVICE_VIDEO               0x00000023
#define FILE_DEVICE_VIRTUAL_DISK        0x00000024
#define FILE_DEVICE_FULLSCREEN_VIDEO    0x00000034

// Characteristics
#define FILE_REMOVABLE_MEDIA            0x00000001
#define FILE_READ_ONLY_DEVICE           0x00000002
#define FILE_FLOPPY_DISKETTE            0x00000004
#define FILE_WRITE_ONCE_MEDIA           0x00000008
#define FILE_REMOTE_DEVICE              0x00000010
#define FILE_DEVICE_IS_MOUNTED          0x00000020
#define FILE_VIRTUAL_VOLUME             0x00000040
#define FILE_AUTOGENERATED_DEVICE_NAME  0x00000080
#define FILE_DEVICE_SECURE_OPEN         0x00000100

#define CTLCODE(DeviceType, Function, Method, Access) ( ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method)  ) 
#define FSCTL_DISMOUNT_VOLUME  CTLCODE( FILE_DEVICE_FILE_SYSTEM, 0x08, METHOD_BUFFERED, FILE_ANY_ACCESS )

bool GetFileInfo( LPCTSTR szFilename, LPWIN32_FILE_ATTRIBUTE_DATA pFileInfo )
{
	bool bReturn = false;
	CStdString sFilename(szFilename);

	if ( pFileInfo && szFilename && GetFileAttributesEx( sFilename, GetFileExInfoStandard, pFileInfo ) )
	{
		bReturn = true;
	}
	return bReturn;
}


//int SMBusWriteCommand(unsigned char slave, unsigned char command, int isWord, unsigned short data) 
//{
//again:
//    _outp(0xc004, (slave<<1)&0xfe);
//    _outp(0xc008, command);
//    _outpw(0xc006, data);
//    _outpw(0xc000, _inpw(0xc000));
//    _outp(0xc002, (isWord) ? 0x0b : 0x0a);
//    while ((_inp(0xc000) & 8)); /* wait while busy */
//    if (_inp(0xc000) & 0x02) goto again; /* retry transmission */
//    if (_inp(0xc000) & 0x34) return 0;  /* fatal error */
//    return 1;
//}
//
//int SMBusReadCommand(unsigned char slave, unsigned char command, int isWord, unsigned short *data) 
//{
//again:
//    _outp(0xc004, (slave<<1)|0x01);
//    _outp(0xc008, command);
//    _outpw(0xc000, _inpw(0xc000));
//    _outp(0xc002, (isWord) ? 0x0b : 0x0a);
//    while ((_inp(0xc000) & 8)); /* wait while busy */
//    if (_inp(0xc000) & 0x02) goto again; /* retry transmission */
//    if (_inp(0xc000) & 0x34) return 0;  /* fatal error */
//    *data = _inpw(0xc006);
//    return 1;
//}



HRESULT Remount(LPCSTR szDrive, LPSTR szDevice)
{
	char szSourceDevice[48];
	sprintf(szSourceDevice,"\\Device\\%s",szDevice);

	Unmount(szDrive);
	
	ANSI_STRING filename;
	OBJECT_ATTRIBUTES attributes;
	IO_STATUS_BLOCK status;
	HANDLE hDevice;
	LONG error;
	DWORD dummy;

	RtlInitAnsiString(&filename, szSourceDevice);
	InitializeObjectAttributes(&attributes, &filename, OBJ_CASE_INSENSITIVE, NULL);

	if (SUCCEEDED(error = NtCreateFile(&hDevice, GENERIC_READ |
		SYNCHRONIZE | FILE_READ_ATTRIBUTES, &attributes, &status, NULL, 0,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, FILE_OPEN,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT)))
	{

		if (!DeviceIoControl(hDevice, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &dummy, NULL))
		{
			CloseHandle(hDevice);
			return E_FAIL;
		}

		CloseHandle(hDevice);
	}
	
	Mount(szDrive,szDevice);

	return S_OK;
}



DWORD DeleteSymbolicLink( TCHAR tcSymbolicDrive )
{
	DWORD dwReturn = 0;
	CUniString usDrive;

	usDrive.GetString()->Format( _T("\\??\\%c:"), tcSymbolicDrive );
	dwReturn = IoDeleteSymbolicLink(usDrive.GetUnicodeString());
	return dwReturn;
}

DWORD RebuildSymbolicLink( LPCTSTR szSymbolicDrive, LPCTSTR szNewDeviceLocation, bool bDelete )
{
	DWORD dwReturn = 0;
	CUniString usDrive, usDeviceLoc;

	
	*(usDeviceLoc.GetString()) = szNewDeviceLocation;
	*(usDrive.GetString()) = szSymbolicDrive;
	// Delete the link, in case it already exists...
	if ( bDelete )
	{
		dwReturn = IoDeleteSymbolicLink(usDrive.GetUnicodeString());
	}

	// Now make a new link
	dwReturn = IoCreateSymbolicLink(usDrive.GetUnicodeString(), usDeviceLoc.GetUnicodeString() );
	return dwReturn;
}

BOOL RemovePath( LPCTSTR szPath )
{
	BOOL bReturn = TRUE;

	WIN32_FIND_DATA ffData;
	CStdString sWildcard;
	CStdString sFilename;
	HANDLE hFind;

	sWildcard = MakeFullFilePath( szPath, _T("*") );
	hFind = FindFirstFile( sWildcard, &ffData );

	if( INVALID_HANDLE_VALUE != hFind )
	{
		while( bReturn == TRUE )
		{
			sFilename = ffData.cFileName;
			if ( sFilename.Compare( _T(".") ) && sFilename.Compare( _T("..") ) )
			{
				sFilename = MakeFullFilePath( szPath, sFilename );
				if ( ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
					bReturn = RemovePath( sFilename );
				}
				else
				{
					bReturn = DeleteFile( sFilename );
				}
			}
			if ( !FindNextFile( hFind, &ffData ) )
			{
				break;
			}
		}
		FindClose( hFind );
	}
	if ( bReturn )
	{
		bReturn = RemoveDirectory( szPath );
	}
	return bReturn;
}


BOOL IsPathEmpty( LPCTSTR szPath )
{
	BOOL bReturn = TRUE;

	WIN32_FIND_DATA ffData;
	CStdString sWildcard;
	CStdString sFilename;
	HANDLE hFind;

	sWildcard = MakeFullFilePath( szPath, _T("*") );
	hFind = FindFirstFile( sWildcard, &ffData );

	if( INVALID_HANDLE_VALUE != hFind )
	{
		while( bReturn == TRUE )
		{
			sFilename = ffData.cFileName;
			if ( sFilename.Compare( _T(".") ) && sFilename.Compare( _T("..") ) )
			{
				bReturn = FALSE;
				break;
			}
			if ( !FindNextFile( hFind, &ffData ) )
			{
				break;
			}
		}
		FindClose( hFind );
	}
	return bReturn;
}


HRESULT FormatPartition( int iPartNum )
{
	HRESULT hr = E_FAIL;
	CStdString sPartition;

	sPartition.Format( _T("\\Device\\Harddisk0\\Partition%d"), iPartNum );

	hr = RebuildSymbolicLink( _T(DriveZ), sPartition, true );
	if ( SUCCEEDED(hr) )
	{
		if ( XFormatUtilityDrive() )
		{
			hr = S_OK;
		}
		else
		{
			hr = GetLastError();
		}
	}
	RebuildSymbolicLink( _T(DriveZ), _T(ZDrive), true );
	return hr;
}


//later on in the code...
#if 0
if( m_DefaultGamepad.bPressedAnalogButtons[ XINPUT_GAMEPAD_X ] )
{
	m_dwStatus = IoDeleteSymbolicLink(&DSymbolicLinkName);
	if( m_dwStatus == 0 )
	{
		m_dwStatus = IoCreateSymbolicLink(&DSymbolicLinkName, &CDeviceName);
		if( m_dwStatus == 0 )
			XLaunchNewImage("D:\\wmvcutscene.xbe", NULL);
	}
}
#endif


HRESULT GetRealDrivePath( char chDrive, CStdString & sReturn )
{
	HRESULT hr = E_FAIL;;
	HANDLE hObject;
	OBJECT_NAME_INFORMATION objName;
	DWORD lLength;
	DWORD dwStatus;
	UNICODE_STRING DriveName;
	char szDrive[10];

	DriveName.Length = 6;
	DriveName.MaximumLength = 7;
	DriveName.Buffer = &szDrive[0];

	if ( isalpha( chDrive ) )
	{

		strcpy( szDrive, "\\??\\D:" );
		szDrive[4] = toupper(chDrive);
		
		OBJECT_ATTRIBUTES DriveObject = 
		{
			NULL,
			&DriveName,
			OBJ_OPENLINK,
		};

		dwStatus = NtOpenSymbolicLinkObject( &hObject, &DriveObject );
		if ( hObject )
		{
			InitUniString( &objName.Name, 520, true );
			dwStatus = NtQuerySymbolicLinkObject( hObject, &objName, &lLength );
			if ( objName.Name.Buffer && lLength )
			{
				hr = S_OK;
				sReturn = (char *)objName.Name.Buffer;
			}
			else
			{
				sReturn = _T("");
			}
			NtClose( hObject );
		}
		else
		{
			sReturn = _T("");
		}
	}
	else
	{
		sReturn = _T("");
	}
	return hr;
}

CStdString GetRealCDROMPath( void )
{
	HANDLE hObject;
	OBJECT_NAME_INFORMATION objName;
	DWORD lLength;
	CStdString sReturn;
	DWORD dwStatus;

	dwStatus = NtOpenSymbolicLinkObject( &hObject, &DDriveObject );
	if ( hObject )
	{
		InitUniString( &objName.Name, 520, true );
		dwStatus = NtQuerySymbolicLinkObject( hObject, &objName, &lLength );
		if ( objName.Name.Buffer && lLength )
		{
			sReturn = (char *)objName.Name.Buffer;
		}
		else
		{
			sReturn.Format( _T("Path failed. Length=%ld dwStatus =%ld"), lLength, dwStatus );
		}
		NtClose( hObject );
	}
	else
	{
		sReturn.Format( _T("Path failed. dwStatus=0x%08lx"), dwStatus );
	}
	return sReturn;
}

DWORD LaunchTitleRaw( LPCTSTR szXbePath, LPCTSTR szDPath, LAUNCH_DATA * pldInfo = NULL )
{
	DWORD dwReturn = 0;
	LAUNCH_DATA ldInfo;
	CStdString sXbePath(szXbePath), sDPath(szDPath);
	DWORD dwXbeID;

	// sXbePath.Format( _T("%s\\%s"), _T(FDrive), _T("sensex.xbe") );
	// sDPath = _T(FDrive);


	memset(&ldInfo,0,sizeof(LAUNCH_DATA));

	if ( pldInfo == NULL )
	{
		pldInfo = &ldInfo;
	}

	if ( szXbePath && _tcslen(szXbePath) )
	{
		sXbePath.Replace( _T("D:"), _T(CdRom) );
		sXbePath.Replace( _T("C:"), _T(CDrive) );
		sXbePath.Replace( _T("E:"), _T(EDrive) );
		sXbePath.Replace( _T("F:"), _T(FDrive) );
		sXbePath.Replace( _T("G:"), _T(GDrive) );
//		if ( FileExists(sXbePath) )
		{
			dwXbeID = 0; // GetXbeID(sXbePath);
			sDPath.Replace( _T("D:"), _T(CdRom) );
			sDPath.Replace( _T("C:"), _T(CDrive) );
			sDPath.Replace( _T("E:"), _T(EDrive) );
			sDPath.Replace( _T("F:"), _T(FDrive) );
			sDPath.Replace( _T("G:"), _T(GDrive) );
			if ( szDPath && _tcslen( szDPath ) )
			{
				dwReturn = XWriteTitleInfoAndReboot( sXbePath, sDPath, LDT_TITLE, dwXbeID, pldInfo);
			}
			else
			{
				dwReturn = XWriteTitleInfoAndReboot( sXbePath, "", LDT_TITLE, dwXbeID, pldInfo);
			}
		}
	}
	return dwReturn;
}



DWORD LaunchTitle( LPCTSTR szPath, LPCTSTR szXbe, LAUNCH_DATA * pldInfo )
{
	DWORD dwStatus;
	CUniString sSymbolicName;
	CStdString sLaunch;
	CStdString sPath(szPath);

	int iPos = sPath.GetLength()-1;

	if ( iPos > -1 )
	{
		if ( sPath[iPos] == _T('\\') )
		{
			sPath = sPath.Left( iPos );
		}
	}
//	*(sSymbolicName.GetString()) = szPath;
	*(sSymbolicName.GetString()) = sPath.c_str();

	// sLaunch.Format( _T("D:\\%s"), szXbe );

	sLaunch.Format( _T("%s"), szXbe );

	UNICODE_STRING DSymbolicLinkName = 
	{
		strlen(DriveD),
		strlen(DriveD)+1,
		DriveD
	};
//	UNICODE_STRING TitleDeviceName;
//
//	TitleDeviceName.Length = strlen( pszPath );
//	TitleDeviceName.MaximumLength = strlen( pszPath )+1;
//	TitleDeviceName.Buffer = pszPath;
//
	dwStatus = IoDeleteSymbolicLink(&DSymbolicLinkName);

	if( dwStatus == 0 )
	{
		dwStatus = IoCreateSymbolicLink(&DSymbolicLinkName, sSymbolicName.GetUnicodeString() );
		if( dwStatus == 0 )
		{
			// sLaunch.Replace(_T("D:"), szPath );
			// dwStatus = LaunchTitleRaw( sLaunch, sPath, pldInfo );
			sLaunch.Format( _T("D:\\%s"), sLaunch.c_str() );
			dwStatus = XLaunchNewImage(sLaunch.c_str(), pldInfo );
		}
		else
		{
			dwStatus |= 0x04000000;
		}
	}
	else
	{
		dwStatus |= 0x08000000;
	}
	return dwStatus;
}

DWORD LaunchTitlePatchFile( LPCTSTR szPath, LPCTSTR szXbe, LAUNCH_DATA * pldInfo )
{
	DWORD dwStatus;
	CUniString sSymbolicName;
	CStdString sLaunch;

	*(sSymbolicName.GetString()) = szPath;

	sLaunch = szXbe;
	sLaunch.Replace( _T("C:"), _T(CDrive) );
	sLaunch.Replace( _T("E:"), _T(EDrive) );
	sLaunch.Replace( _T("F:"), _T(FDrive) );
	sLaunch.Replace( _T("G:"), _T(GDrive) );

	UNICODE_STRING DSymbolicLinkName = 
	{
		strlen(DriveD),
		strlen(DriveD)+1,
		DriveD
	};
//	UNICODE_STRING TitleDeviceName;
//
//	TitleDeviceName.Length = strlen( pszPath );
//	TitleDeviceName.MaximumLength = strlen( pszPath )+1;
//	TitleDeviceName.Buffer = pszPath;
//
	dwStatus = IoDeleteSymbolicLink(&DSymbolicLinkName);

	if( dwStatus == 0 )
	{
		dwStatus = IoCreateSymbolicLink(&DSymbolicLinkName, sSymbolicName.GetUnicodeString() );
		if( dwStatus == 0 )
		{
			dwStatus = XLaunchNewImage(sLaunch.c_str(), pldInfo);
		}
		else
		{
			dwStatus |= 0x04000000;
		}
	}
	else
	{
		dwStatus |= 0x08000000;
	}
	return dwStatus;
}



unsigned char strcksum8( LPCTSTR szStr )
{
	unsigned char byValue = 0;

	while( szStr && *szStr )
	{
		byValue += (unsigned char)(*szStr);
		szStr++;
	}
	byValue ^= 0xff;
	return byValue;
}

// Takes directory in format: D:\MUSIC\ **
__int64	DirectorySignature( LPCTSTR szDirectory )
{
	__int64	i64Result = 0;
	int iCount = 0;
	DWORD dwSize=0;
	DWORD dwCksum = 0;
	HANDLE hFind;
	int iIndex = 0;
	WIN32_FIND_DATA ffData;
	CStdString sWildcard;
//	CStdString sTemp;

	sWildcard.Format( _T("%s*.*"), szDirectory );
	hFind = FindFirstFile( sWildcard.c_str(), &ffData );
	
	if( INVALID_HANDLE_VALUE != hFind )
	{
		do
		{
			dwSize += ffData.nFileSizeLow;
			dwCksum <<= 1;
			dwCksum += (DWORD)strcksum8( ffData.cFileName );

//			sTemp.Format( _T("%s%s"), szDirectory, ffData.cFileName );
			iCount++;
			iIndex++;
		} while( FindNextFile( hFind, &ffData ) );

		// Close the find handle.
		FindClose( hFind );
	}
	i64Result = (__int64)iCount;
	i64Result <<= 48;
	return i64Result;
}

bool SaveStringToFile( LPCTSTR szFilename, LPCTSTR szString )
{
	DWORD dwSize;
	bool bReturn = false;
	if ( szString )
	{
		dwSize = _tcslen(szString)*sizeof(TCHAR);
		if ( dwSize )
		{
			bReturn = SaveBufferToFile( szFilename, (BYTE*)szString, dwSize );
		}
	}
	return bReturn;
}

bool SaveBufferToFile( LPCTSTR szFilename, BYTE *pbBuffer, DWORD dwSize )
{
	DWORD dwWritten = 0;
	bool bReturn = false;

	if ( dwSize && pbBuffer )
	{
		HANDLE hTemp;
		hTemp = CreateFile( szFilename, 
					   GENERIC_WRITE, 
					   0, 
					   NULL,
					   CREATE_ALWAYS,
					   FILE_ATTRIBUTE_NORMAL,
					   NULL );
		if ( hTemp != INVALID_HANDLE_VALUE )
		{
			if ( WriteFile( hTemp, pbBuffer, dwSize, &dwWritten, NULL ) == TRUE )
			{
				bReturn = true;
			}
			CloseHandle( hTemp );
		}
	}
	return bReturn;
}

BYTE * LoadBufferFromFile( LPCTSTR szFilename, DWORD *pdwSize )
{
	BYTE *pBuffer = NULL;
	DWORD dwRead = 0;
	if (pdwSize )
	{
		if ( FileExists(szFilename) )
		{
			HANDLE hTemp;
			hTemp = CreateFile( szFilename, 
						   GENERIC_READ, 
						   FILE_SHARE_READ, 
						   NULL,
						   OPEN_EXISTING,
						   FILE_ATTRIBUTE_NORMAL,
						   NULL );
			if ( hTemp != INVALID_HANDLE_VALUE )
			{
				*pdwSize = GetFileSize( hTemp, NULL );
				if ( *pdwSize )
				{
					pBuffer = new BYTE [ *pdwSize+1 ];
					if ( pBuffer )
					{
						pBuffer[*pdwSize]=0;
						if ( ReadFile( hTemp, pBuffer, *pdwSize, &dwRead, NULL ) == FALSE )
						{
							delete [] pBuffer;
							pBuffer = NULL;
						}						
					}
				}
			}
			CloseHandle( hTemp );
		}
	}
	return pBuffer;
}




bool IsImageFile( LPCTSTR szFilename, bool bStaticOnly )
{
	bool bReturn = false;
	CStdString sFilename(szFilename);

	sFilename.MakeLower();

	if ( sFilename.Find( _T(".wmv") ) != -1 )
	{
		if ( bStaticOnly )
		{
			bReturn = false;
		}
		else
		{
			bReturn = true;
		}
	}
	else if ( sFilename.Find( _T(".jpg") ) != -1 )
	{
		bReturn = true;
	}
	else if ( sFilename.Find( _T(".png") ) != -1 )
	{
		bReturn = true;
	}
	else if ( sFilename.Find( _T(".bmp") ) != -1 )
	{
		bReturn = true;
	}
	else if ( sFilename.Find( _T(".tga") ) != -1 )
	{
		bReturn = true;
	}
	//else if ( sFilename.Find( _T(".gif") ) != -1 )
	//{
	//	bReturn = true;
	//}
	else if ( sFilename.Find( _T(".xbe") ) != -1 )
	{
		bReturn = true;
	}
	else if ( sFilename.Find( _T(".xbx") ) != -1 )
	{
		bReturn = true;
	}
	else if ( sFilename.Find( _T(".dds") ) != -1 )
	{
		bReturn = true;
	}
	else if ( (sFilename.GetLength() > 1) && (sFilename[0] == _T(':')) && (sFilename[1] == _T(':')) )
	{
		// Internal Image File
		bReturn = true;
	}
	return bReturn;
}



void ParseFile( LPCTSTR szFilename, XmlNotify & Subscriber )
{
	BYTE * pBuffer = NULL;
	DWORD dwSize;

	pBuffer = LoadBufferFromFile( szFilename, &dwSize );
	if ( pBuffer  )
	{
		if ( dwSize )
		{
			DWORD dwCheckSize = strlen( (const char*)pBuffer );

			XmlStream xml;
			xml.setSubscriber( Subscriber );
			xml.parse( (char *) pBuffer, dwSize );
		}
		delete [] pBuffer;
	}
	else
	{
		DEBUG_FORMAT( _T("Unable to load file: %s"), szFilename );
	}
}


//typedef map<string, string> TMapStrings;
int GetLiteral( LPCTSTR szString, CStdString & sLiteral )
{
	int iPos, iStartPos;
	TCHAR tcStopChar = _T(' ');
	LPTSTR szLiteral = NULL;
	int iReturn = 0;

	iStartPos = 0;
	if ( szString )
	{
		szLiteral = new TCHAR[_tcslen(szString)+1];
		if ( szLiteral )
		{
			if ( szString[0] == _T('\"') )
			{
				// Quoted string literal
				iStartPos = 1;
				tcStopChar = _T('\"');
			}
			else
			{
				iStartPos = 0;
			}
			iPos = iStartPos;
			while( szString[iPos] && szString[iPos]!=tcStopChar )
			{
				if ( szString[iPos] == _T('\\') )
				{
					iPos++;
					switch( szString[iPos] )
					{
						case _T('n'):
						case _T('N'):
							szLiteral[iReturn] = 10;
							iPos++;
							iReturn++;
							break;
						case _T('t'):
						case _T('T'):
							szLiteral[iReturn] = 9;
							iPos++;
							iReturn++;
							break;
						case _T('v'):
						case _T('V'):
							szLiteral[iReturn] = 11;
							iPos++;
							iReturn++;
							break;
						case _T('r'):
						case _T('R'):
							szLiteral[iReturn] = 13;
							iPos++;
							iReturn++;
							break;
	//					case _T('x'):
	//					case _T('X'):
	//						szLiteral[iReturn] = 13;
	//						iReturn++;
	//						iPos++;
	//						break;
						case 0:
							break;
						case _T('\\'):
						case _T('\"'):
						case _T('\''):
						default:
							szLiteral[iReturn] = szString[iPos];
							iReturn++;
							iPos++;
							break;
					}
				}
				else
				{
					szLiteral[iReturn] = szString[iPos];
					iReturn++;
					iPos++;
				}
			}
			szLiteral[iReturn] = 0;
			if ( iReturn )
			{
				sLiteral = szLiteral;
			}
			else
			{
				sLiteral = _T("");
			}
			if ( szString[iPos] == tcStopChar )
			{
				iPos++;
			}
			// Skip spaces after literal
			while ( szString[iPos] == _T(' ') )
			{
				iPos++;
			}
			delete [] szLiteral;
		}
	}
	return iPos;
}

bool ParseXMLAttributes( string & attributes, TMapStrings & msAttributes, bool bLowerIDs )
{
	bool bReturn = false;

	CStdString sAttributes( attributes );
	CStdString sKey, sValue;
	int iPos = 0;
	int iStartPos = 0;
//	CStdString sTemp;

	iPos = sAttributes.Find( _T('='), iStartPos );
	while( iPos != -1 )
	{
		sKey = sAttributes.Mid( iStartPos, (iPos-iStartPos) );
		sKey.TrimLeft();
		sKey.TrimRight();
		if ( bLowerIDs )
		{
			sKey.MakeLower();
		}
		iStartPos = iPos+1;
		sValue = sAttributes.Mid( iPos+1 );
		iStartPos += GetLiteral( sAttributes.Mid( iStartPos ).c_str(), sValue );
		//sValue.TrimLeft();
		//sValue.TrimRight();
		UnScramble( sValue );
		msAttributes.insert( TMapStrings::value_type( sKey, sValue ));
//		DEBUG_FORMAT( _T("  Attribute Added: (%s)=(%s)"), sKey.c_str(), sValue.c_str() );
		iPos = sAttributes.Find( _T('='), iStartPos );
	}
	return bReturn;
}


int ReadFileString( HANDLE hFile, char * szBuffer, int iMaxChars )
{
	int iIndex = 0;
	DWORD dwRead;

	if (szBuffer && hFile && iMaxChars )
	{
		szBuffer[0] = 0;
		while( iMaxChars-- )
		{
			ReadFile( hFile, (LPVOID)&szBuffer[iIndex], 1, &dwRead, NULL);
			if ( dwRead == 0 )
			{
				szBuffer[iIndex] = 0;
			}
			if ( szBuffer[iIndex] == 0 )
			{
				break;
			}
			else
			{
				iIndex++;
			}
		}
	}
	return iIndex;
}

int WriteFileLine( HANDLE hFile, LPCSTR szString, bool bWriteEOL  )
{
	int iReturn = -1;
	int iLength;
	DWORD dwWritten;
	CStdString sLine( szString );

	if ( bWriteEOL )
	{
		sLine += "\r\n";
	}
	iLength = sLine.GetLength();
	if ( WriteFile( hFile, sLine.c_str(), iLength, &dwWritten, NULL) )
	{
		iReturn = dwWritten;
	}
	return iReturn;
}

CStdString ReadFileLine( HANDLE hFile )
{
	CStdString sReturn;
	char * pBuffer = NULL;

	pBuffer = new char [1024];
	if ( pBuffer )
	{
		int iLen = ReadFileLine( hFile, pBuffer, 1024 );
		sReturn += pBuffer;
		while ( iLen == 1024 )
		{
			iLen = ReadFileLine( hFile, pBuffer, 1024 );
			sReturn += pBuffer;
		}
		delete [] pBuffer;
	}
	return sReturn;
}

int ReadFileLine( HANDLE hFile, char * szBuffer, int iMaxChars )
{
	int iIndex = 0;
	DWORD dwRead;
	BOOL bSuccess = TRUE;

	if (szBuffer && hFile && iMaxChars )
	{
		szBuffer[0] = 0;
		while( bSuccess && iMaxChars-- )
		{
			bSuccess = ReadFile( hFile, (LPVOID)&szBuffer[iIndex], 1, &dwRead, NULL);
			if ( dwRead == 0 )
			{
				szBuffer[iIndex] = 0;
				bSuccess = FALSE;
				break;
			}
			if ( szBuffer[iIndex] == '\n' ) 
			{
				szBuffer[iIndex] = 0;
				break;
			}
			else
			{
				// Ignore CRs
				if ( szBuffer[iIndex] != '\r' )
				{
					iIndex++;
				}
			}
		}
		if ( (bSuccess == FALSE) && (iIndex == 0) )
		{
			iIndex = -1;
		}
	}
	return iIndex;
}


BYTE * GetXbeLogo( LPCTSTR szFilePath )
{
	BYTE *pReturn = NULL;
	HANDLE hFile;
	DWORD dwLogoLocation;
	DWORD dwLogoSize;
	DWORD dwLoadAddress;
	DWORD dwRead;
	
	hFile = CreateFile( szFilePath, 
						GENERIC_READ, 
						FILE_SHARE_READ, 
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL );
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		if ( SetFilePointer(	hFile,  0x104, NULL, FILE_BEGIN ) == 0x104 )
		{
			if ( ReadFile( hFile, &dwLoadAddress, 4, &dwRead, NULL ) )
			{
				if ( SetFilePointer(	hFile,  0x170, NULL, FILE_BEGIN ) == 0x170 )
				{
					if ( ReadFile( hFile, &dwLogoLocation, 4, &dwRead, NULL ) && 
						 ReadFile( hFile, &dwLogoSize, 4, &dwRead, NULL ) )
					{
						if ( dwLogoLocation && dwLogoSize && (dwLogoSize<0x40000))
						{
							if ( SetFilePointer(	hFile,  dwLogoLocation, NULL, FILE_BEGIN ) == dwLogoLocation )
							{
								pReturn = new BYTE[dwLogoSize];
								if ( pReturn )
								{
									memset( pReturn, 0, dwLogoSize );
									if ( !ReadFile( hFile, pReturn, dwLogoSize, &dwRead, NULL ) )
									{
										// Didn't load? Delete this mess then.
										delete pReturn;
										pReturn = NULL;
									}
								}
							}
						}

					}
				}
			}
		}
		CloseHandle(hFile);
	}
	return pReturn;
}

bool GetXbeInfo( LPCTSTR szFilePath, TXBEINFO & xbeInfo )
{
	bool bReturn = false;

	CStdString sTitle;
	HANDLE hFile;
	DWORD dwCertificateLocation;
	DWORD dwLoadAddress;
	DWORD dwRead;
	DWORD dwID;
	int iIndex;
	WCHAR wcTitle[41];
	
	hFile = CreateFile( szFilePath, 
						GENERIC_READ, 
						FILE_SHARE_READ, 
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL );
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		if ( SetFilePointer(	hFile,  0x104, NULL, FILE_BEGIN ) == 0x104 )
		{
			if ( ReadFile( hFile, &dwLoadAddress, 4, &dwRead, NULL ) )
			{
				if ( SetFilePointer(	hFile,  0x118, NULL, FILE_BEGIN ) == 0x118 )
				{
					if ( ReadFile( hFile, &dwCertificateLocation, 4, &dwRead, NULL ) )
					{
						dwCertificateLocation -= dwLoadAddress;
						// Add offset into file
						if ( SetFilePointer(	hFile,  (dwCertificateLocation+8), NULL, FILE_BEGIN ) == (dwCertificateLocation+8) )
						{
							dwID = 0;
							ReadFile( hFile, &dwID, sizeof(DWORD), &dwRead, NULL );
							if ( dwRead != sizeof(DWORD) )
							{
								dwID = 0;
							}
						}


						// dwCertificateLocation += 12;
						if ( SetFilePointer(	hFile,  (dwCertificateLocation+12), NULL, FILE_BEGIN ) == (dwCertificateLocation+12) )
						{
							memset( wcTitle, 0, 41*sizeof(WCHAR) );
							if ( ReadFile( hFile, &wcTitle, 40*sizeof(WCHAR), &dwRead, NULL ) )
							{
								sTitle = wcTitle;
							}
						}

					}
				}
			}
		}

		CloseHandle(hFile);

		xbeInfo.m_dwID = dwID;
		xbeInfo.m_sTitle = sTitle;
		for( iIndex=0; g_Presets[iIndex].m_dwAppCode != 0xffffffff; iIndex++ )
		{
			if ( g_Presets[iIndex].m_dwAppCode == dwID )
			{
				xbeInfo.m_sAltTitle = g_Presets[iIndex].m_szAppTitle;
				xbeInfo.m_sDescription = g_Presets[iIndex].m_szDescr;
			}
		}
		for( iIndex=0; g_Publishers[iIndex].m_dwAppCode != 0xffffffff; iIndex++ )
		{
			if ( g_Publishers[iIndex].m_dwAppCode == (dwID&0xffff0000) )
			{
				xbeInfo.m_sPublisher = g_Publishers[iIndex].m_szPublisher;
			}
		}
		bReturn = true;
	}
	return bReturn;


	return bReturn;
}


CStdString GetXbeTitle( LPCTSTR szFilePath )
{
	CStdString sReturn;
	HANDLE hFile;
	DWORD dwCertificateLocation;
	DWORD dwLoadAddress;
	DWORD dwRead;
	WCHAR wcTitle[41];
	
	hFile = CreateFile( szFilePath, 
						GENERIC_READ, 
						FILE_SHARE_READ, 
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL );
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		if ( SetFilePointer(	hFile,  0x104, NULL, FILE_BEGIN ) == 0x104 )
		{
			if ( ReadFile( hFile, &dwLoadAddress, 4, &dwRead, NULL ) )
			{
				if ( SetFilePointer(	hFile,  0x118, NULL, FILE_BEGIN ) == 0x118 )
				{
					if ( ReadFile( hFile, &dwCertificateLocation, 4, &dwRead, NULL ) )
					{
						dwCertificateLocation -= dwLoadAddress;
						// Add offset into file

						dwCertificateLocation += 12;
						if ( SetFilePointer(	hFile,  dwCertificateLocation, NULL, FILE_BEGIN ) == dwCertificateLocation )
						{
							memset( wcTitle, 0, 41*sizeof(WCHAR) );
							if ( ReadFile( hFile, &wcTitle, 40*sizeof(WCHAR), &dwRead, NULL ) )
							{
								sReturn = wcTitle;
							}
						}

					}
				}
			}
		}
		CloseHandle(hFile);
	}
	return sReturn;
}

DWORD GetXbeID( LPCTSTR szFilePath )
{
	DWORD dwReturn = 0;
	HANDLE hFile;
	DWORD dwCertificateLocation;
	DWORD dwLoadAddress;
	DWORD dwRead;
//	WCHAR wcTitle[41];
	
	hFile = CreateFile( szFilePath, 
						GENERIC_READ, 
						FILE_SHARE_READ, 
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL );
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		if ( SetFilePointer(	hFile,  0x104, NULL, FILE_BEGIN ) == 0x104 )
		{
			if ( ReadFile( hFile, &dwLoadAddress, 4, &dwRead, NULL ) )
			{
				if ( SetFilePointer(	hFile,  0x118, NULL, FILE_BEGIN ) == 0x118 )
				{
					if ( ReadFile( hFile, &dwCertificateLocation, 4, &dwRead, NULL ) )
					{
						dwCertificateLocation -= dwLoadAddress;
						// Add offset into file
						dwCertificateLocation += 8;
						if ( SetFilePointer(	hFile,  dwCertificateLocation, NULL, FILE_BEGIN ) == dwCertificateLocation )
						{
							dwReturn = 0;
							ReadFile( hFile, &dwReturn, sizeof(DWORD), &dwRead, NULL );
							if ( dwRead != sizeof(DWORD) )
							{
								dwReturn = 0;
							}
						}

					}
				}
			}
		}
		CloseHandle(hFile);
	}
	return dwReturn;
}

bool GetTextureSize( LPDIRECT3DTEXTURE8 pTexture, int & iWidth, int & iHeight )
{
	bool bReturn = false;
	if ( pTexture )
	{
		D3DSURFACE_DESC descSurface;

		// Now make pTexture our texture.
		// First we need to get the params
		if ( SUCCEEDED( pTexture->GetLevelDesc( 0, &descSurface ) ) )
		{
			iWidth = descSurface.Width;
			iHeight = descSurface.Height;
			bReturn = true;
		}
	}
	return bReturn;
}


HRESULT DrawSpriteRects(IDirect3DDevice8* pDevice, LPDIRECT3DTEXTURE8 pTexture, RECT & rectDest, RECT & rectSrc, int iWidth, int iHeight, BOOL bTransparency, BOOL bUseAlphaChannel, int iGlobalBlend, DWORD dwAddMode )
{
	HRESULT hrReturn = S_OK;
//	int iPosX, iPosY, iSizeX, iSizeY;
	FLOAT fTU1, fTV1, fTU2, fTV2;
//	int iDeltaX, iDeltaY, iDeltaWidth, iDeltaHeight;

	int iDstWidth = (rectDest.right-rectDest.left);
	int iDstHeight = (rectDest.bottom-rectDest.top);
	int iTexWidth = (rectSrc.right-rectSrc.left);;
	int iTexHeight = (rectSrc.bottom-rectSrc.top);

	GetTextureSize( pTexture, iTexWidth, iTexHeight );

	if ( iHeight != -1 )
	{
		iTexHeight = iHeight;
	}
	if ( iWidth != -1 )
	{
		iTexWidth = iWidth;
	}

	fTU1 = (FLOAT)rectSrc.left;
	fTU1 /= (FLOAT)iTexWidth;
	fTU2 = (FLOAT)rectSrc.right;
	fTU2 /= (FLOAT)iTexWidth;
	fTV1 = (FLOAT)rectSrc.top;
	fTV1 /= (FLOAT)iTexHeight;
	fTV2 = (FLOAT)rectSrc.bottom;
	fTV2 /= (FLOAT)iTexHeight;

	hrReturn = DrawSpriteOffset(pDevice, pTexture, rectDest.left, rectDest.top, iDstWidth, iDstHeight, fTU1, fTV1, fTU2, fTV2, bTransparency, bUseAlphaChannel, iGlobalBlend, dwAddMode );

	return hrReturn;
}

HRESULT DrawSpriteOffset(IDirect3DDevice8* pDevice, LPDIRECT3DTEXTURE8 pTexture, int iPosX, int iPosY, int iSizeX, int iSizeY, FLOAT fTU1, FLOAT fTV1, FLOAT fTU2, FLOAT fTV2, BOOL bTransparency, BOOL bUseAlphaChannel, int iGlobalBlend, DWORD dwAddMode )
{
//	if ( iDestSizeY == -1 ) iDestSizeY = iSizeY-iOffY;
//	if ( iDestSizeX == -1 ) iDestSizeY = iSizeX-iOffX;
//	FLOAT fTU1, fTU2, fTV1, fTV2;
	IDirect3DVertexBuffer8* pVB;
	D3DSURFACE_DESC descInfo;
	BOOL bSwizzled;
	DWORD dwVertexHandle = FVF_SPRITE;


	if ( iGlobalBlend > (-1) )
	{
		dwVertexHandle = D3DFVF_XYZRHW|D3DFVF_TEX1;
//		dwVertexHandle = D3DFVF_NORMAL|D3DFVF_TEX1;
	}
	pTexture->GetLevelDesc( 0, &descInfo );


	bSwizzled = XGIsSwizzledFormat(descInfo.Format);
	// Have to use clamp mode and pixel size addressing
//	if ( descInfo.Format == D3DFMT_YUY2 )
	if ( !bSwizzled )
	{
		fTU1 *= (FLOAT)descInfo.Width;
		fTU2 *= (FLOAT)descInfo.Width;
		fTV1 *= (FLOAT)descInfo.Height;
		fTV2 *= (FLOAT)descInfo.Height;
	}

	struct VERTEX_SPRITE
	{
	float x, y, z, rhw;  // z always 0.0, rhw always 1.0
	float u, v;          // texture coords
	};


	// single square sprite
	VERTEX_SPRITE sprite[4] =
	{ //   x    y  z  w  tu  tv
	{ (FLOAT)iPosX, (FLOAT)iPosY, TWOD_Z, 1, fTU1, fTV1 },
	{ (FLOAT)(iPosX+iSizeX), (FLOAT)iPosY, TWOD_Z, 1, fTU2, fTV1 },
	{ (FLOAT)(iPosX+iSizeX), (FLOAT)(iPosY+iSizeY), TWOD_Z, 1, fTU2, fTV2 },
	{ (FLOAT)iPosX, (FLOAT)(iPosY+iSizeY), TWOD_Z, 1, fTU1, fTV2 }
	};


	HRESULT hr;
	
	// create vertex buffer
	hr = pDevice->CreateVertexBuffer( 4 * sizeof( VERTEX_SPRITE ), D3DUSAGE_WRITEONLY, dwVertexHandle, D3DPOOL_DEFAULT, &pVB );
	if ( !SUCCEEDED(hr) )
	{
		// Bad news!
		
	}
	else
	{

		// lock vertex buffer
		VERTEX_SPRITE* pVerts;
		pVB->Lock( 0, 0, (BYTE**)&pVerts, 0 );

		// fill vertex buffer
		memcpy( pVerts, sprite, sizeof( sprite ));

		// unlock vertex buffer
		pVB->Unlock();

		// now draw sprite
		// render objects
		pDevice->SetTexture( 0, pTexture );

		
	//	if ( descInfo.Format == D3DFMT_YUY2 )
		if ( !bSwizzled )
		{
			if ( descInfo.Format == D3DFMT_YUY2 )
			{
				pDevice->SetRenderState(D3DRS_YUVENABLE, TRUE);
			}
			pDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,   D3DTADDRESS_CLAMP );
			pDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,   D3DTADDRESS_CLAMP );
		}
		if ( iGlobalBlend > (-1) )
		{
			pDevice->SetPixelShader( NULL );
			//Enable alpha blending so we can use transparent textures
			pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

			pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
			//Set how the texture should be blended (use alpha)
			pDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
			pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
			pDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_CLAMP );
			pDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_CLAMP );
			pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, (DWORD)(iGlobalBlend&0xff)<<24L );
			pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
			pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
			pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	//		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	//		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
	//		pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	//		pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	//		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	//		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		}
		else if ( bUseAlphaChannel )
		{
			//Enable alpha blending so we can use transparent textures
			pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

			//Set how the texture should be blended (use alpha)
			pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			// pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
			pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

	//		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_BLENDTEXTUREALPHA );
		}
		else
		{

			if ( bTransparency )
			{

				pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
				pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

				pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
				pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			}
			else
			{
		//		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_BLENDTEXTUREALPHA );
				// pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
		//		pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		//		pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				if ( bSwizzled )
				{
					pDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,   dwAddMode );
					pDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,   dwAddMode );
				}

				pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
				pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				// Get the colour information from the inverted alpha.
		//		pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		//		pDevice->SetTextureStageState(0, D3DTSS_COLORARG1,
		//                D3DTA_ALPHAREPLICATE | D3DTA_COMPLEMENT | D3DTA_TEXTURE);
	  			 
				// pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
			}
		}

		pDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
		pDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );

		//// now draw sprite
		//// render objects
		pDevice->SetTexture( 0, pTexture );

		pDevice->SetVertexShader( dwVertexHandle );
		pDevice->SetStreamSource( 0, pVB, sizeof( VERTEX_SPRITE ));
		pDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );

		if ( descInfo.Format == D3DFMT_YUY2 )
		{
			pDevice->SetRenderState(D3DRS_YUVENABLE, FALSE);
		}
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		// clean up
		pDevice->SetTexture( 0, NULL );
		pVB->Release();
	}
	return S_OK;

//	fTU1 = iSizeXiOffX
}




HRESULT GetPersistentTexture(IDirect3DDevice8* pDevice, LPDIRECT3DTEXTURE8 *ppTexture )
{
	HRESULT hr = E_FAIL;
	D3DSURFACE_DESC descSurface;
	bool bReleaseSurface = false;

    // Create a texture surface for the persisted surface
    LPDIRECT3DSURFACE8 pPersistedSurface = NULL;
    LPDIRECT3DSURFACE8 pPersistedSurfaceTgt = NULL;
    if( SUCCEEDED(hr = pDevice->GetPersistedSurface( &pPersistedSurface )) )
	{
		if( pPersistedSurface )
		{
			pPersistedSurface->GetDesc( &descSurface );
			// descSurface.Format;
			// descSurface.Type;
			// descSurface.Usage;
			// descSurface.Size;
			// descSurface.MultiSampleType;
			// descSurface.Width;
			// descSurface.Height;

			// Create an empty surface to hold the persistent screen
			if ( descSurface.Width && descSurface.Height)
			{
				CStdString sTemp;

//				sTemp.Format( _T("Getting Surface: W=%d H=%d Fmt=0x%08x"),descSurface.Width, descSurface.Height, descSurface.Format );
//				DEBUG_LINE( sTemp.c_str() );
				pDevice->CreateTexture( descSurface.Width, descSurface.Height, 1, 0, descSurface.Format, 0, ppTexture );
				(*ppTexture)->GetSurfaceLevel( 0, &pPersistedSurfaceTgt );
    
				// Copy persisted bits to our surface, scaling and converting if necessary
				D3DXLoadSurfaceFromSurface( pPersistedSurfaceTgt, NULL, NULL, pPersistedSurface, 
										NULL, NULL, D3DX_FILTER_NONE, 0 );


				hr = S_OK;
				pPersistedSurfaceTgt->Release();
			}
		}
    }
	return hr;
}

HRESULT GetBackBufferTexture(IDirect3DDevice8* pDevice, LPDIRECT3DTEXTURE8 *ppTexture )
{
	HRESULT hr = E_FAIL;
	D3DSURFACE_DESC descSurface;
	bool bReleaseSurface = false;

    // Create a texture surface for the persisted surface
    LPDIRECT3DSURFACE8 pPersistedSurface = NULL;
    LPDIRECT3DSURFACE8 pPersistedSurfaceTgt = NULL;
	// We don't have a persisted surface, can we get the currently used
	// one from the D3D device?
	hr = pDevice->GetPersistedSurface( &pPersistedSurface );
	if ( SUCCEEDED( hr ) && pPersistedSurface )
	{
		//SaveStringToFile( _T("D:\\step.txt"), _T("XSelector GetPersistentSurface OK") );
		pPersistedSurface->AddRef();
	}
	else
	{
		//SaveStringToFile( _T("D:\\step.txt"), _T("XSelector GetPersistentSurface FAILED") );
		if ( !SUCCEEDED(hr = pDevice->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&pPersistedSurface)))
		{
			//SaveStringToFile( _T("D:\\step.txt"), _T("XSelector GetBackBuffer FAILED") );
			pPersistedSurface = NULL;
		}
		//else
		//{
		//	SaveStringToFile( _T("D:\\step.txt"), _T("XSelector GetBackBuffer OK") );
		//}
	}
	if ( pPersistedSurface ) // SUCCEEDED(hr = pDevice->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&pPersistedSurface)))
	{
		const POINT ptSrc = { 0, 0 };
		const RECT rc = { 0,          0, 
						  640, 480 };
		const POINT ptDest = { 0, 0 };
		pPersistedSurface->GetDesc( &descSurface );
		if ( SUCCEEDED(hr = pDevice->CreateTexture( 640, 480, 1, 0, descSurface.Format, 0, ppTexture )))
		{
			if ( SUCCEEDED(hr = (*ppTexture)->GetSurfaceLevel( 0, &pPersistedSurfaceTgt )))
			{
				try
				{
					if ( SUCCEEDED(hr = pDevice->CopyRects( pPersistedSurface, &rc, 1, 
										pPersistedSurfaceTgt, &ptDest )))
					{
	//					DEBUG_LINE(_T("CREATED BACK BUFFER"));
					}
				}
				catch( ... )
				{
				}
			}
			pPersistedSurfaceTgt->GetDesc( &descSurface );
			pPersistedSurfaceTgt->Release();
		}
		pPersistedSurface->Release();
	}
	return hr;
}



HRESULT DrawSprite(IDirect3DDevice8* pDevice, LPDIRECT3DTEXTURE8 pTexture, int iPosX, int iPosY, int iSizeX, int iSizeY, BOOL bTransparency, BOOL bUseAlphaChannel, int iGlobalBlend )
{
	return DrawSpriteOffset( pDevice, pTexture, iPosX, iPosY, iSizeX, iSizeY, 0.0, 0.0, 1.0, 1.0, bTransparency, bUseAlphaChannel, iGlobalBlend );
}

DWORD ConvertUnsigned( LPCTSTR szValue )
{
	DWORD dwReturn = 0;
	CStdString sValue(szValue);
	int iPos = 0, iBase = 10;

	// Can handle a number of formats....
	if( sValue.GetLength() )
	{
		sValue.MakeLower();

		if ( (sValue[0] == _T('x')) ||
			 (sValue[0] == _T('$')) )
		{
			iBase = 16;
			iPos=1;
		}
		else if ( sValue[0] == _T('%'))
		{
			iBase = 2;
			iPos=1;
		}
		else
		{
			if ( ( sValue.GetLength() > 2 ) &&
				 (sValue[0] == _T('0'))&&
				 (sValue[1] == _T('x')) )
			{
				iPos = 2;
				iBase = 16;
			}
		}
		sValue = sValue.Mid( iPos );
		dwReturn = _tcstoul( sValue, NULL, iBase );
	}
	return dwReturn;
}

long ConvertSigned( LPCTSTR szValue )
{
	long lReturn = 0;
	
	// Decimal conversion only
	lReturn = _tcstol( szValue, NULL, 10 );

	return lReturn;
}


double ConvertFloat( LPCTSTR szValue )
{
	double fReturn = 0.0;

	fReturn = _tcstod(szValue, NULL );
	return fReturn;
}


CStdString FindImageFile( LPCTSTR szMainPath, LPCTSTR szPath )
{
	CStdString sBaseName;
	CStdString sFilename;
	CStdString sFullPath;
	CStdString sDir;
	HANDLE hFind;
	WIN32_FIND_DATA ffData;

	if ( szPath && _tcslen( szPath ) )
	{
		sBaseName.Format(_T("%s.*"), szPath );
		sDir = MakeFullFilePath( szMainPath, sBaseName );
		hFind = FindFirstFile( sDir, &ffData );
		sDir = StripFilenameFromPath(sDir);
		if( INVALID_HANDLE_VALUE != hFind )
		{
			do
			{
				sFilename = ffData.cFileName;
				if ( IsImageFile( sFilename ) )
				{
					sFullPath = MakeFullFilePath( sDir, sFilename );
				}
			} while( (sFullPath.GetLength()==0) && FindNextFile( hFind, &ffData ) );
		}
	}
	return sFullPath;
}

#if 0
CStdString LocateFile( LPCTSTR szFileName, DWORD dwA, DWORD dwB, DWORD dwC, LPCTSTR szDirectory );

// Search a directory and find the file.
// Recurse if needed
CStdString LocateFile( LPCTSTR szFileName, DWORD dwA, DWORD dwB, DWORD dwC, LPCTSTR szDirectory )
{
	CStdString sDir(szDirectory);
	CStdString sReturn;
	CStdString sPathName;

	sPathName = MakeFullFilePath( szDirectory, szFileName );
	if ( FileExists( sPathName ) )
	{
		// Check to see if this file matches.
		HANDLE hFile;
		hFile = CreateFile( sPathName, 
						GENERIC_READ, 
						FILE_SHARE_READ, 
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL );
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			DWORD dwRead, dwValue = 54321;
			ReadFile( hFile, &dwValue, sizeof(DWORD), &dwRead, NULL );
			if ( dwValue == dwA )
			{
				ReadFile( hFile, &dwValue, sizeof(DWORD), &dwRead, NULL );
				if ( dwValue == dwB )
				{
					ReadFile( hFile, &dwValue, sizeof(DWORD), &dwRead, NULL );
					if ( dwValue == dwC )
					{
						sReturn = sPathName;
					}
				}
			}
			CloseHandle( hFile );
			// OK, written... let's go look for it.
		}
	}
	// Have we found it yet? If now, recurse through the directories!
	if ( sReturn.GetLength() == 0 )
	{
		CStdString sWildcard;
		HANDLE hFind;
		WIN32_FIND_DATA ffData;
		sWildcard = MakeFullFilePath( szDirectory, _T("*") );
		hFind = FindFirstFile( sWildcard, &ffData );
		if( INVALID_HANDLE_VALUE != hFind )
		{
			do
			{
				if ( ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
				{
					sWildcard = ffData.cFileName;
					sWildcard.MakeUpper();

					if ( sWildcard.Compare( _T("UDATA") ) &&
						 sWildcard.Compare( _T("TDATA") ) &&
						 sWildcard.Compare( _T(".") ) &&
						 sWildcard.Compare( _T("..") ) )
					{
						sWildcard = MakeFullDirPath( szDirectory, ffData.cFileName, FALSE );
						sReturn = LocateFile( szFileName, dwA, dwB, dwC, sWildcard );
					}
				}
			} while( (sReturn.GetLength()==0) && FindNextFile( hFind, &ffData ) );
		}
	}
	return sReturn;
}
#endif

bool CompareFilesExact( LPCTSTR szFileA, LPCTSTR szFileB )
{
	bool bReturn = false;
	WIN32_FILE_ATTRIBUTE_DATA fadFileA;
	WIN32_FILE_ATTRIBUTE_DATA fadFileB;
	
	if ( GetFileAttributesEx( szFileA, GetFileExInfoStandard, &fadFileA ) )
	{
		if ( GetFileAttributesEx( szFileB, GetFileExInfoStandard, &fadFileB ) )
		{
			// Compare attributes
			if ( ( fadFileA.dwFileAttributes == fadFileB.dwFileAttributes ) &&
				 ( fadFileA.ftCreationTime.dwLowDateTime == fadFileB.ftCreationTime.dwLowDateTime ) &&
				 ( fadFileA.ftCreationTime.dwHighDateTime == fadFileB.ftCreationTime.dwHighDateTime ) &&
				 ( fadFileA.ftLastWriteTime.dwLowDateTime == fadFileB.ftLastWriteTime.dwLowDateTime ) &&
				 ( fadFileA.ftLastWriteTime.dwHighDateTime == fadFileB.ftLastWriteTime.dwHighDateTime ) &&
				 ( fadFileA.nFileSizeHigh == fadFileA.nFileSizeHigh ) &&
				 ( fadFileA.nFileSizeLow == fadFileA.nFileSizeLow ) )
			{
				bReturn = true;
			}
		}
	}

	return bReturn;
}

bool CompareDirectories( LPCTSTR szOrigPath, LPCTSTR szTryPath )
{
	bool bReturn = false;
	HANDLE hFind;
	WIN32_FIND_DATA ffData;
	CStdString sWildcard;
	CStdString sTryFile, sOrigFile;
	sWildcard.Format(_T("%s\\*"), szOrigPath );
	hFind = FindFirstFile( sWildcard, &ffData );

	if( INVALID_HANDLE_VALUE != hFind )
	{
		bReturn = true;
		do
		{
			if ( ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
			{
				sTryFile = MakeFullDirPath( szTryPath, ffData.cFileName, FALSE );
				if ( !FileExists( sTryFile ) )
				{
					bReturn = false;
				}
			}
			else
			{
				sTryFile = MakeFullFilePath( szTryPath, ffData.cFileName );
				if ( !FileExists( sTryFile ) )
				{
					bReturn = false;
				}
				else
				{
					sOrigFile = MakeFullFilePath( szOrigPath, ffData.cFileName );
					bReturn = CompareFilesExact( sOrigFile, sTryFile );
				}
			}
		} while( bReturn && FindNextFile( hFind, &ffData ) );
	}
	return bReturn;
}

#if 0
CStdString LocateDOnDVD( LPCTSTR szDevicePath )
{
	CStdString sReturn;
	CStdString sNewPath;

	if ( CompareDirectories( szDevicePath, _T("D:\\") ) )
	{
		sReturn = szDevicePath;
		sReturn.Replace( _T(CdRom), _T("D:\\") );
		sReturn.Replace( _T(CDrive), _T("C:\\") );
		sReturn.Replace( _T(EDrive), _T("E:\\") );
		sReturn.Replace( _T(FDrive), _T("F:\\") );
		sReturn.Replace( _T(GDrive), _T("G:\\") );
	}
	if ( sReturn.GetLength() == 0 )
	{
		// Iterate through directories to locate our root!
		HANDLE hFind;
		WIN32_FIND_DATA ffData;
		CStdString sWildcard;
		

		sWildcard.Format(_T("%s\\*"), szDevicePath );
		hFind = FindFirstFile( sWildcard, &ffData );
		if( INVALID_HANDLE_VALUE != hFind )
		{
			do
			{
				if ( ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
				{
					sNewPath.Format(_T("%s\\%s"), szDevicePath, ffData.cFileName, FALSE );
					sReturn = LocateDOnDVD( sNewPath );
				}
			} while( (sReturn.GetLength() == 0) && FindNextFile( hFind, &ffData ) );
		}

	}
	return sReturn;
}
#endif

CStdString GetNextCaptureFilename(void)
{
	CStdString sReturn;
	int iIndex = 0;

	sReturn.Format( _T("Z:\\SCRCAP_%04d.bmp"), iIndex );
	while( FileExists( sReturn ) && iIndex<9999 )
	{
		iIndex++;
		sReturn.Format( _T("Z:\\SCRCAP_%04d.bmp"), iIndex );
	}
	return sReturn;
}


bool CheckList( TListStrings & slList, LPCTSTR szCheck, bool bLower, bool bAdd )
{
	bool bReturn = false;
	CStdString sCheck(szCheck);
	TListStrings::iterator iterString;

	if ( szCheck && _tcslen(szCheck) )
	{
		if ( bLower )
		{
			sCheck.MakeLower();
		}
		iterString = slList.begin();
		while( (!bReturn) && (iterString != slList.end()) )
		{
			if ( sCheck.Compare( iterString->c_str() ) == 0 )
			{
				bReturn = true;
			}
			iterString++;
		}
		if ( (!bReturn) && (bAdd) )
		{
			slList.push_back( sCheck );
		}
	}
	return bReturn;
}



int CorrectCoordinate( int iTrueDim, FLOAT fPctCoord, int iPseudoCoord, int iPseudoDim )
{
	int iTrueCoord = -1;
	FLOAT fTrueCoord;
	long lTrueCoord;

	if ( iTrueDim > 0 )
	{
		if ( fPctCoord > 0.0 )
		{
			fTrueCoord = (FLOAT)iTrueDim;
			fTrueCoord *= fPctCoord;
			fTrueCoord /= 100.0;
			iTrueCoord = (int)(fTrueCoord+0.5);
		}
		else if ( (iPseudoCoord > 0 ) && (iPseudoDim > 0 ) )
		{
			lTrueCoord = (long)iTrueDim;
			lTrueCoord *= (long)iPseudoCoord;
			lTrueCoord /= (long)iPseudoDim;
		}
	}
	return iTrueCoord;
}

HRESULT BuildRawTexture( IDirect3DDevice8* pDevice, int &iWidth, int &iHeight, LPDIRECT3DTEXTURE8 *ppTexture, D3DFORMAT d3dFormat )
{
	HRESULT hr = E_FAIL;

	if ( pDevice )
	{
		switch( d3dFormat )
		{
			case D3DFMT_A8R8G8B8:
			case D3DFMT_X8R8G8B8:
			case D3DFMT_R5G6B5:
			case D3DFMT_X1R5G5B5:
				// For swizzled textures, make sure the dimensions are a power of two
				for( DWORD wmask=1; iWidth&(iWidth-1); wmask = (wmask<<1)+1 )
					iWidth = ( iWidth + wmask ) & ~wmask;
				for( DWORD hmask=1; iHeight&(iHeight-1); hmask = (hmask<<1)+1 )
					iHeight = ( iHeight + hmask ) & ~hmask;
				break;

			case D3DFMT_LIN_A8R8G8B8:
			case D3DFMT_LIN_X8R8G8B8:
			case D3DFMT_LIN_R5G6B5:
			case D3DFMT_LIN_X1R5G5B5:
				// For linear textures, make sure the stride is a multiple of 64 bytes
				iWidth  = ( iWidth + 0x1f ) & ~0x1f;
				break;

		}

		hr = pDevice->CreateTexture( iWidth,
									 iHeight,
									 1,
									 0,
									 d3dFormat,
									 0,
									 ppTexture );
	}
	return hr;
}

static LPCTSTR s_strLegalFatX = " !#$%&'()-.@[]^_`{}~0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

CStdString FatXify( LPCTSTR szFilename, bool bIncludesPath, TCHAR tcReplace )
{
	CStdString sReturn;
	CStdString sScan(szFilename);
	CStdString sReplace;
	int iPos;
	int iLen = 0;
	
	// Strip out non-FATX characters
	if ( szFilename && (iLen = _tcslen(szFilename)) )
	{
		if ( bIncludesPath )
		{
			iPos = sScan.ReverseFind( _T("\\") );
			if ( iPos >= 0 )
			{
				sReturn = sScan.Left( iPos+1 );
				sScan = sScan.Mid( iPos+1 );
				iLen = sScan.GetLength();
			}
			iPos = 0;
		}
		else
		{
			iPos = 0;
		}
		while( iPos < iLen )
		{
			if ( _tcschr( s_strLegalFatX, sScan[iPos] ) == NULL )
			{
				sScan.SetAt( iPos, _T('\x01'));
			}
			iPos++;
		}
		if ( tcReplace && _tcschr( s_strLegalFatX, tcReplace ))
		{
			sReplace.Format( _T("%c"), tcReplace );
		}
		sScan.Replace( _T("\x01"), sReplace );
		sReturn += sScan;
	}
	return sReturn;
}

bool ValidateForFATX( LPCTSTR szPath, int * piCount )
{
	bool bReturn = true;
	HANDLE hFind;
	WIN32_FIND_DATA ffData;
	CStdString sWildcard;
	CStdString sNewPath;
	int iCount = 0;
	int iCountSub;
		
	
	if ( szPath && _tcslen( szPath ) )
	{
		sWildcard.Format(_T("%s\\*"), szPath );
		hFind = FindFirstFile( sWildcard, &ffData );
		if( INVALID_HANDLE_VALUE != hFind )
		{
			do
			{
				bReturn = IsValidFatXName( ffData.cFileName );
				if ( bReturn )
				{
					iCount++;
					if ( ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
					{
						sNewPath.Format(_T("%s\\%s"), szPath, ffData.cFileName, FALSE );
						bReturn = ValidateForFATX( sNewPath, &iCountSub );
						iCount += iCountSub;
					}
				}
			} while( (bReturn) && FindNextFile( hFind, &ffData ) );
		}
	}
	else
	{
		bReturn = false;
	}
	if ( piCount )
	{
		*piCount = iCount;
	}
	return bReturn;
}

bool IsValidFatXName( LPCTSTR szFilename )
{
	bool bReturn = true;

	if ( szFilename && _tcslen(szFilename) && (_tcslen(szFilename) <= 42) )
	{
		while( (*szFilename) && bReturn )
		{
			if ( ( *szFilename > 127 ) ||
				 ( _tcschr( s_strLegalFatX, *szFilename ) ) )
			{
				szFilename++;
			}
			else
			{
				bReturn = false;
			}
		}
	}
	return bReturn;
}

bool IsValidXISOName( LPCTSTR szFilename )
{
	bool bReturn = true;

	return bReturn;
}

bool IsValidUDFName( LPCTSTR szFilename )
{
	bool bReturn = true;

	return bReturn;
}


// &amp;
// &lt;
// &gt;
// 

struct
{
	LPCTSTR szEntChar;
	LPCTSTR szEntName;
	LPCTSTR szEntCode;
} entityValues[] =
{
	{ _T("\""), _T("&quot;"), _T("&#34;") },
	{ _T("<"), _T("&lt;"), _T("&#60;") },
	{ _T(">"), _T("&gt;"), _T("&#62;") },
#if 0
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
	{ _T(''), _T("&;"), _T("&#00;") },
#endif
	{ NULL, NULL, NULL },
};

void UnScramble( CStdString & sValue )
{
	int iIndex = 0;
	while( entityValues[iIndex].szEntChar )
	{
		sValue.Replace( entityValues[iIndex].szEntName, entityValues[iIndex].szEntChar );
		sValue.Replace( entityValues[iIndex].szEntCode, entityValues[iIndex].szEntChar );
		iIndex++;
	}
	sValue.Replace( _T("&amp;"), _T("&") );
}

void Scramble( CStdString & sValue )
{
	int iIndex = 0;

	sValue.Replace( _T("&"), _T("&amp;") );

	while( entityValues[iIndex].szEntChar )
	{
		if ( entityValues[iIndex].szEntName )
		{
			sValue.Replace( entityValues[iIndex].szEntChar, entityValues[iIndex].szEntName );
		}
		else
		{
			sValue.Replace( entityValues[iIndex].szEntChar, entityValues[iIndex].szEntCode );
		}
		iIndex++;
	}
}



bool SplitURL( LPCTSTR szURL, CStdString & sServer, int & iPort, CStdString & sURI )
{
	bool bReturn = false;
	CStdString sTemp;
	int iPos;

	if ( szURL && _tcslen( szURL ) )
	{
		CStdString sURL(szURL);

		if ( _tcsnicmp( szURL, _T("http://"), 7 ) == 0 )
		{
			sTemp = sURL.Mid(7);
			iPos = sTemp.Find( _T('/') );
			if ( iPos >= 0 )
			{
				sURI = sTemp.Mid( iPos );
				sTemp = sTemp.Left( iPos );
			}
			else
			{
				sURI = _T("/");
			}
			iPos = sTemp.Find( _T(':') );
			iPort = 80;
			if ( iPos >= 0 )
			{
				sServer = sTemp.Left( iPos );
				sTemp = sTemp.Mid( iPos+1 );
				iPort = _tcstoul( sTemp, NULL, 10 );
				if ( iPort == 0 )
				{
					iPort = 80;
				}
			}
			else
			{
				sServer = sTemp;
			}
			if ( sServer.GetLength() && sURI.GetLength() )
			{
				bReturn = true;
			}
		}
	}
	return bReturn;
}

// Separate sequences seperated by "::"
CStdString FetchIDToken( int iIndex, LPCTSTR szText )
{
	CStdString sReturn;
	CStdString sText(szText);
	int iPos;

	sText.Trim();
	if ( szText && sText.GetLength() )
	{
		iPos = sText.Find( _T("::") );
		while( iIndex && (iPos>(-1)))
		{
			// Strip fron away
			sText = sText.Mid(iPos+2);
			iPos = sText.Find( _T("::") );
			iIndex--;
		}
		if ( iIndex == 0 )
		{
			if ( iPos > (-1) )
			{
				sReturn = sText.Left( iPos );
			}
			else
			{
				sReturn = sText;
			}
		}
	}
	return sReturn;
}

CStdString LoadStringFromFile( LPCTSTR szFilename )
{
	CStdString sResult;
	DWORD dwResult = 0;
	LPSTR szBuffer = NULL;

	szBuffer = (LPSTR)LoadBufferFromFile( szFilename, &dwResult );
	if ( szBuffer )
	{
		if ( dwResult )
		{
			sResult = szBuffer;
		}
		delete [] szBuffer;
	}
	return sResult;
}


HRESULT DrawTexture( IDirect3DDevice8* pDevice, IDirect3DBaseTexture8 * pTexture, int iPosX, int iPosY, int iSizeX, int iSizeY )
{

	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	pDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	pDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );

    // Draw the analog gauges
    pDevice->SetTexture( 0, pTexture );

	pDevice->SetVertexShader( FVF_SPRITE );

    struct VERTEX { D3DXVECTOR4 p; FLOAT tu, tv; };
    VERTEX v[4];
    v[0].p = D3DXVECTOR4( (FLOAT)iPosX			- 0.5f, (FLOAT)iPosY			- 0.5f, TWOD_Z, 0 );  v[0].tu =   0;		v[0].tv =   0;
    v[1].p = D3DXVECTOR4( (FLOAT)(iPosX+iSizeX)	- 0.5f, (FLOAT)iPosY			- 0.5f, TWOD_Z, 0 );  v[1].tu = (FLOAT)iSizeX;	v[1].tv =   0;
    v[2].p = D3DXVECTOR4( (FLOAT)(iPosX+iSizeX)	- 0.5f, (FLOAT)(iPosY+iSizeY)	- 0.5f, TWOD_Z, 0 );  v[2].tu = (FLOAT)iSizeX;	v[2].tv = (FLOAT)iSizeY;
    v[3].p = D3DXVECTOR4( (FLOAT)iPosX			- 0.5f, (FLOAT)(iPosY+iSizeY)	- 0.5f, TWOD_Z, 0 );  v[3].tu =   0;		v[3].tv = (FLOAT)iSizeY;
    pDevice->DrawPrimitiveUP( D3DPT_QUADLIST, 1, v, sizeof(v[0]) );
	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: DrawBox()
// Desc: Graphics helper function
//-----------------------------------------------------------------------------
HRESULT DrawLine( IDirect3DDevice8* pDevice, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2,
                              DWORD dwOutlineColor )
{
    D3DXVECTOR4 v[2];
    v[0] = D3DXVECTOR4( x1-0.5f, y1-0.5f, TWOD_Z, 0 );
    v[1] = D3DXVECTOR4( x2-0.5f, y2-0.5f, TWOD_Z, 0 );

    pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
    pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
    pDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    pDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    pDevice->SetVertexShader( D3DFVF_XYZRHW );

    // Render the lines
    pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwOutlineColor );
    pDevice->DrawVerticesUP( D3DPT_LINELIST, 2, v, sizeof(v[0]) );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: DrawBox()
// Desc: Graphics helper function
//-----------------------------------------------------------------------------
HRESULT DrawBox( IDirect3DDevice8* pDevice, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2,
                              DWORD dwFillColor, DWORD dwOutlineColor )
{
    D3DXVECTOR4 v[4];
    v[0] = D3DXVECTOR4( x1-0.5f, y1-0.5f, TWOD_Z, 0 );
    v[1] = D3DXVECTOR4( x2-0.5f, y1-0.5f, TWOD_Z, 0 );
    v[2] = D3DXVECTOR4( x2-0.5f, y2-0.5f, TWOD_Z, 0 );
    v[3] = D3DXVECTOR4( x1-0.5f, y2-0.5f, TWOD_Z, 0 );

    pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
    pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
    pDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    pDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    pDevice->SetVertexShader( D3DFVF_XYZRHW );

    // Render the box
    pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwFillColor );
    pDevice->DrawVerticesUP( D3DPT_QUADLIST, 4, v, sizeof(v[0]) );

    // Render the lines
    pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwOutlineColor );
    pDevice->DrawVerticesUP( D3DPT_LINELOOP, 4, v, sizeof(v[0]) );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: DrawBox()
// Desc: Graphics helper function
//-----------------------------------------------------------------------------
HRESULT DrawTriangle( IDirect3DDevice8* pDevice, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2,
                              DWORD dwFillColor, DWORD dwOutlineColor, int iDirection )
{
    D3DXVECTOR4 v[3];
	FLOAT fDelta;

	switch( iDirection )
	{
		case 0: // Up
			fDelta = x2-x1;
			fDelta /= 2.0;
			fDelta += 0.5f;
			v[0] = D3DXVECTOR4( x1-0.5f, y2-0.5f, TWOD_Z, 0 );
			v[1] = D3DXVECTOR4( x2-fDelta, y1-0.5f, TWOD_Z, 0 );
			v[2] = D3DXVECTOR4( x2-0.5f, y2-0.5f, TWOD_Z, 0 );
			break;
		case 2: // Left
			fDelta = y2-y1;
			fDelta /= 2.0;
			fDelta += 0.5f;
			v[0] = D3DXVECTOR4( x2-0.5f, y2-0.5f, TWOD_Z, 0 );
			v[1] = D3DXVECTOR4( x1-0.5f, y2-fDelta, TWOD_Z, 0 );
			v[2] = D3DXVECTOR4( x2-0.5f, y1-0.5f, TWOD_Z, 0 );
			break;
		case 3: // Right
			fDelta = y2-y1;
			fDelta /= 2.0;
			fDelta += 0.5f;
			v[0] = D3DXVECTOR4( x1-0.5f, y1-0.5f, TWOD_Z, 0 );
			v[1] = D3DXVECTOR4( x2-0.5f, y2-fDelta, TWOD_Z, 0 );
			v[2] = D3DXVECTOR4( x1-0.5f, y2-0.5f, TWOD_Z, 0 );
			break;
		case 1: // Down
		default:
			fDelta = x2-x1;
			fDelta /= 2.0;
			fDelta += 0.5f;
			v[0] = D3DXVECTOR4( x2-0.5f, y1-0.5f, TWOD_Z, 0 );
			v[1] = D3DXVECTOR4( x2-fDelta, y2-0.5f, TWOD_Z, 0 );
			v[2] = D3DXVECTOR4( x1-0.5f, y1-0.5f, TWOD_Z, 0 );
			break;
	}

    pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
    pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
    pDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    pDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    pDevice->SetVertexShader( D3DFVF_XYZRHW );

    // Render the box
    pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwFillColor );
    pDevice->DrawVerticesUP( D3DPT_TRIANGLELIST, 3, v, sizeof(v[0]) );

    // Render the lines
    pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwOutlineColor );
    pDevice->DrawVerticesUP( D3DPT_LINELOOP, 3, v, sizeof(v[0]) );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: RenderGradientBackground()
// Desc: Draws a gradient filled background
//-----------------------------------------------------------------------------
HRESULT RenderQuadGradientBackground( IDirect3DDevice8* pDevice, DWORD dwTopLeftColor, 
											DWORD dwTopRightColor, 
											DWORD dwBottomLeftColor, 
                                            DWORD dwBottomRightColor, RECT * pRect )
{
	
	RECT rect;

	if ( pRect )
	{
		rect.top = pRect->top;
		rect.left = pRect->left;
		rect.bottom = pRect->bottom;
		rect.right = pRect->right;
	}
	else
	{
		rect.top = 0;
		rect.left = 0;
		rect.bottom = 480;
		rect.right = 640;
	}

    // First time around, allocate a vertex buffer
    static LPDIRECT3DVERTEXBUFFER8 g_pVB  = NULL;
    if( g_pVB == NULL )
    {
        pDevice->CreateVertexBuffer( 4*5*sizeof(FLOAT), D3DUSAGE_WRITEONLY, 
                                          0L, D3DPOOL_DEFAULT, &g_pVB );
	}
//	else
	{
        struct BACKGROUNDVERTEX { D3DXVECTOR4 p; D3DCOLOR color; };
        BACKGROUNDVERTEX* v;
        g_pVB->Lock( 0, 0, (BYTE**)&v, 0L );
        v[0].p = D3DXVECTOR4( (FLOAT)(rect.left)  - 0.5f, (FLOAT)(rect.top)    - 0.5f, TWOD_Z, 1.0f );  v[0].color = dwTopLeftColor;
        v[1].p = D3DXVECTOR4( (FLOAT)(rect.right) - 0.5f, (FLOAT)(rect.top)    - 0.5f, TWOD_Z, 1.0f );  v[1].color = dwTopRightColor;
        v[2].p = D3DXVECTOR4( (FLOAT)(rect.left)  - 0.5f, (FLOAT)(rect.bottom) - 0.5f, TWOD_Z, 1.0f );  v[2].color = dwBottomLeftColor;
        v[3].p = D3DXVECTOR4( (FLOAT)(rect.right) - 0.5f, (FLOAT)(rect.bottom) - 0.5f, TWOD_Z, 1.0f );  v[3].color = dwBottomRightColor;
        g_pVB->Unlock();
    }

    // Set states
    pDevice->SetTexture( 0, NULL );
    pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DISABLE );
    pDevice->SetRenderState( D3DRS_ZENABLE, FALSE ); 
    pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE ); 
    pDevice->SetVertexShader( D3DFVF_XYZRHW|D3DFVF_DIFFUSE );
    pDevice->SetStreamSource( 0, g_pVB, 5*sizeof(FLOAT) );

    pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    // Clear the zbuffer
    pDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0L );

    return S_OK;
}

HRESULT DrawTextBox( IDirect3DDevice8* pDevice, LPCTSTR szText, CXBFont * pFont, DrawingAttributes * pAttributes, bool bCenterText )
{
	FLOAT fWidth, fHeight, fLeft, fTop;
	pFont->GetTextExtent( szText, &fWidth, &fHeight, FALSE );
	fWidth += 10.0; fLeft = (FLOAT)(((640.0-fWidth)/2.0)+0.5);
	fHeight += 10.0; fTop = (FLOAT)(((480.0-fHeight)/2.0)+0.5);
	DrawBox( pDevice, fLeft, fTop, fLeft+fWidth, fTop+fHeight, 0, pAttributes->m_dwBorderColor );
	DrawBox( pDevice, fLeft-1, fTop-1, fLeft+fWidth+2, fTop+fHeight+2, 0, pAttributes->m_dwBorderColor );
	DrawBox( pDevice, fLeft+1, fTop+1, fLeft+fWidth-1, fTop+fHeight-1, pAttributes->m_dwBoxColor, pAttributes->m_dwBorderColor );
	// Now draw the text over the box, with shadows to highlight it.
	pFont->DrawText( (FLOAT)(fLeft+6.0), (FLOAT)(fTop+6.0), pAttributes->m_dwShadowColor, szText ); // , XBFONT_CENTER_X|XBFONT_CENTER_Y );
	pFont->DrawText( (FLOAT)(fLeft+5.0), (FLOAT)(fTop+5.0), pAttributes->m_dwTextColor, szText ); // , XBFONT_CENTER_X|XBFONT_CENTER_Y );
	return S_OK;
}
