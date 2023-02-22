
#include <xtl.h>
#include "MXM.h"
#include "XbeInfo.h"
#include "XBMD5.h"



TMapXbeInfo g_XbeInfo;

CXBEInfo::CXBEInfo() :
	m_sMD5(_T("")),
	m_sTitle(_T("")),
	m_dwID(0),
	m_dwSize(0),
	m_dwMediaTypes(0),
	m_dwRegions(0),
	m_dwGameRatings(0)
{
}

CXBEInfo::~CXBEInfo()
{
}




CStdString GetXbeMD5( LPCTSTR szFilepath )
{
	CXBEInfo * pXbeInfo = NULL;
	CStdString sReturn;
	
	pXbeInfo = _GetXBEInfo( szFilepath );
	if ( pXbeInfo )
	{
		if ( pXbeInfo->m_sMD5.GetLength() )
		{
			sReturn = pXbeInfo->m_sMD5;
		}
		else
		{
			CXBMD5 md5Xbe;

			if ( md5Xbe.CalcFile( szFilepath ) )
			{
				sReturn = md5Xbe.GetMD5String();
				pXbeInfo->m_sMD5 = sReturn;
			}
		}
	}

	return sReturn;
}


CXBEInfo * _GetXBEInfo( LPCTSTR szFilename )
{
	CXBEInfo * pReturn = NULL;
	TMapXbeInfo::iterator itKey;
	CStdString sKey;
	CStdString sFilepath(szFilename);

	sFilepath.MakeLower();
	if ( g_XbeInfo.size() )
	{
		itKey = g_XbeInfo.find( sFilepath );
		if ( itKey != g_XbeInfo.end() )
		{
			pReturn = itKey->second;
		}
	}
	if ( !pReturn )
	{
		if ( FileExists( szFilename ) )
		{
			// OK, we are in business... now let's save the data off.
			pReturn = new CXBEInfo;

			if ( pReturn )
			{
				HANDLE hFile;
				DWORD dwCertificateLocation;
				DWORD dwLoadAddress;
				DWORD dwRead;
				DWORD dwMedia = 0;
				DWORD dwRegion = 0;
				DWORD dwID = 0;
//				int iIndex;
				WCHAR wcTitle[41];
				CStdString sTitle;

				hFile = CreateFile( szFilename, 
									GENERIC_READ, 
									FILE_SHARE_READ, 
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL,
									NULL );

				if ( hFile != INVALID_HANDLE_VALUE )
				{
					pReturn->m_dwSize = GetFileSize( hFile, NULL );
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
									if ( SetFilePointer(	hFile,  (dwCertificateLocation+0x9C), NULL, FILE_BEGIN ) == (dwCertificateLocation+0x9C) )
									{
										dwMedia = 0;
										ReadFile( hFile, &dwMedia, sizeof(DWORD), &dwRead, NULL );
										if ( dwRead != sizeof(DWORD) )
										{
											dwMedia = 0;
										}
									}
									if ( SetFilePointer(	hFile,  (dwCertificateLocation+0xA0), NULL, FILE_BEGIN ) == (dwCertificateLocation+0xA0) )
									{
										dwRegion = 0;
										ReadFile( hFile, &dwRegion, sizeof(DWORD), &dwRead, NULL );
										if ( dwRead != sizeof(DWORD) )
										{
											dwRegion = 0;
										}
									}

								}
							}
						}
					}
					CloseHandle( hFile );
					pReturn->m_dwID = dwID;
					pReturn->m_sTitle = sTitle;
					pReturn->m_dwMediaTypes = dwMedia;
					pReturn->m_dwRegions = dwRegion;
					g_XbeInfo.insert( TMapXbeInfo::value_type( sFilepath, pReturn ));
				}
			}
		}
	}
	return pReturn;
}


