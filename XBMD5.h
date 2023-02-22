
#ifndef __XBMD5_H__
#define __XBMD5_H__



class CXBMD5
{
	unsigned char  m_pbDigestVal[16];
	bool m_bCalculated;
	TCHAR m_szDigestValue[33];

public:
	CXBMD5( BYTE * pbData = NULL, long lSize = -1 );
	~CXBMD5();
	bool	Calculate( BYTE * pbData, long lSize = -1 );
	bool	CalcFile( LPCTSTR szFilepath );
	LPCTSTR GetMD5String( void ) { return m_szDigestValue; };
	BYTE *	GetMD5Buffer( void ) { return m_pbDigestVal; };
	bool	IsMD5Valid( void ) { return m_bCalculated; };
};




#endif //  __XBMD5_H__