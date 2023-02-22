
#pragma once

typedef struct 
{

	BYTE m_bManufacturerId;
	BYTE m_bDeviceId;
 	LPCTSTR m_szFlashDescription;
	DWORD m_dwLengthInBytes;
} TFlashDefs;


class CFlashData
{
public:
	BYTE 		m_byMfgID;
	BYTE 		m_byDevID;
	CStdString 	m_sFlashDesc;
	DWORD		m_dwLength;
	DWORD		m_dwStartOffset;
	DWORD		m_dwLengthUsed;
	bool		m_bDetectedVia28;
	bool		m_bWriteable;
	BYTE *		m_pbMMStartAddr;
	CFlashData();
	bool GetDescriptor( void );
}


CFlashData::CFlashData() :
	m_byMfgID(0),
	m_byDevID(0),
	m_sFlashDesc(_T("Unknown")),
	m_dwLength(0),
	m_dwStartOffset(0),
	m_dwLengthUsed(0),
	m_bDetectedVia28(false),
	m_bWriteable(false),
	m_pbMMStartAddr(NULL),
{
}
