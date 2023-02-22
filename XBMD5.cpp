

#include <xtl.h>
#include <WinNT.h>
#include <stdio.h>
#include <tchar.h>
#include "md5.h"
#include "xbmd5.h"



CXBMD5::CXBMD5( BYTE * pbData, long lSize )
{
	if ( pbData )
	{
		Calculate( pbData, lSize );
	}
}

CXBMD5::~CXBMD5()
{
}


bool CXBMD5::CalcFile( LPCTSTR szFilepath )
{
	m_bCalculated = false;
	HANDLE hFile;
	BYTE *pbData;


	hFile = CreateFile( szFilepath, 
						GENERIC_READ, 
						FILE_SHARE_READ, 
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL );

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		pbData = new BYTE[8192];

		if ( pbData )
		{
			DWORD dwRead = 0;
			// Perform check!
			MD5_CTX context;
			MD5Init(&context); 

			if ( ReadFile( hFile, pbData, 8192, &dwRead, NULL ) )
			{
				while( dwRead > 0 )
				{
					if ( dwRead )
					{
						MD5Update(&context, pbData, dwRead );
					}
					if ( !ReadFile( hFile, pbData, 8192, &dwRead, NULL ) )
					{
						dwRead = 0;
					}
				}
			}

			MD5Final(reinterpret_cast <unsigned char *>(m_pbDigestVal),&context);

			int iIndex, iPos;
			for ( iIndex = iPos = 0; iIndex<16; iIndex++)
			{
				_stprintf( &m_szDigestValue[iPos],_T("%02x"), m_pbDigestVal[iIndex]);
				iPos += 2;
			}
			m_bCalculated = true;
		}
		CloseHandle( hFile );

	}
	return m_bCalculated;
}


bool CXBMD5::Calculate( BYTE * pbData, long lSize )
{
	m_bCalculated = false;

	if ( pbData )
	{
		if ( lSize == -1 )
		{
			lSize = strlen( (const char *)pbData );
		}
		// Perform check!
		MD5_CTX context;
		MD5Init(&context); 

		MD5Update(&context, reinterpret_cast<unsigned char *>(pbData), lSize );
		MD5Final(reinterpret_cast <unsigned char *>(m_pbDigestVal),&context);

		int iIndex, iPos;
		for ( iIndex = iPos = 0; iIndex<16; iIndex++)
		{
			_stprintf( &m_szDigestValue[iPos],_T("%02x"), m_pbDigestVal[iIndex]);
			iPos += 2;
		}
		m_bCalculated = true;

	}
	return m_bCalculated;
}




