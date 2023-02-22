// XbeInfo.h

#ifndef __XBEINFO_H__
#define __XBEINFO_H__


#include <map>
#include <string>
#include "StdString.h"

using namespace std;


#define XBEIMAGE_GAME_REGION_NA             0x00000001
#define XBEIMAGE_GAME_REGION_JAPAN          0x00000002
#define XBEIMAGE_GAME_REGION_RESTOFWORLD    0x00000004
#define XBEIMAGE_GAME_REGION_MANUFACTURING  0x80000000


#define XBEIMAGE_MEDIA_TYPE_HARD_DISK           0x00000001
#define XBEIMAGE_MEDIA_TYPE_DVD_X2              0x00000002
#define XBEIMAGE_MEDIA_TYPE_DVD_CD              0x00000004
#define XBEIMAGE_MEDIA_TYPE_CD                  0x00000008
#define XBEIMAGE_MEDIA_TYPE_DVD_5_RO            0x00000010
#define XBEIMAGE_MEDIA_TYPE_DVD_9_RO            0x00000020
#define XBEIMAGE_MEDIA_TYPE_DVD_5_RW            0x00000040
#define XBEIMAGE_MEDIA_TYPE_DVD_9_RW            0x00000080
#define XBEIMAGE_MEDIA_TYPE_DONGLE              0x00000100
#define XBEIMAGE_MEDIA_TYPE_MEDIA_BOARD         0x00000200
#define XBEIMAGE_MEDIA_TYPE_NONSECURE_HARD_DISK 0x40000000
#define XBEIMAGE_MEDIA_TYPE_NONSECURE_MODE      0x80000000
#define XBEIMAGE_MEDIA_TYPE_MEDIA_MASK          0x00FFFFFF



class CXBEInfo
{
public:
	CXBEInfo();
	~CXBEInfo();
	CStdString		m_sMD5;
	CStdString		m_sTitle;
	DWORD			m_dwID;
	DWORD			m_dwSize;
	DWORD			m_dwMediaTypes;
	DWORD			m_dwRegions;
	DWORD			m_dwGameRatings;
};

typedef map<string, CXBEInfo *> TMapXbeInfo;


extern TMapXbeInfo g_XbeInfo;

CXBEInfo * _GetXBEInfo( LPCTSTR szFilename );
CStdString GetXbeMD5( LPCTSTR szFilepath );







#endif //  __XBEINFO_H__
