/*********************************************************************************\
 * utils.h
 * Misc classes and routines to make life easier.
 * (C) 2002 MasterMind
\*********************************************************************************/


#ifndef __UTILS_H__
#define __UTILS_H__

#include "xntdll.h"
#include "MXM_Defs.h"
#include "XmlStream.h"

//#define DriveC "\\??\\C:"
//#define DriveD "\\??\\D:"
//#define DriveE "\\??\\E:"
//#define DriveF "\\??\\F:"
#define DriveG "\\??\\G:"
#define DriveS "\\??\\S:"
#define DriveT "\\??\\T:"
#define DriveU "\\??\\U:"
#define DriveV "\\??\\V:"
#define DriveQ "\\??\\Q:"
//#define CdRom "\\Device\\CdRom0"
#define CDrive "\\Device\\Harddisk0\\Partition2"
#define EDrive "\\Device\\Harddisk0\\Partition1"
#define FDrive "\\Device\\Harddisk0\\Partition6"
#define GDrive "\\Device\\Harddisk0\\Partition7"
#define SDrive "\\Device\\Harddisk0\\Partition1\\TDATA"
#define TDrive "\\Device\\Harddisk0\\Partition1\\TDATA\\00004321"
#define UDrive "\\Device\\Harddisk0\\Partition1\\UDATA\\00004321"
#define VDrive "\\Device\\Harddisk0\\Partition1\\UDATA"

#define DriveX "\\??\\X:"
#define DriveY "\\??\\Y:"
#define DriveZ "\\??\\Z:"
#define XDrive "\\Device\\Harddisk0\\Partition3"
#define YDrive "\\Device\\Harddisk0\\Partition4"
#define ZDrive "\\Device\\Harddisk0\\Partition5"

typedef	struct _DrawingAttributes
{
		DWORD m_dwBoxColor;
		DWORD m_dwBorderColor;
		DWORD m_dwTextColor;
		DWORD m_dwShadowColor;
		DWORD m_dwSelectColor;
		DWORD m_dwValueColor;
		DWORD m_dwSelectValueColor;
} DrawingAttributes;

bool CheckTime( void );

HRESULT FormatPartition( int iPartNum );

CStdString GetIPString( void );
// CStdString LocateDRoot( TCHAR tcBestGuessDrive );
// CStdString LocateDOnDVD( LPCTSTR szDevicePath );
BOOL InitializeNetwork( void );
BOOL IsEthernetConnected( void );

BOOL IsPathEmpty( LPCTSTR szPath );
BOOL RemovePath( LPCTSTR szPath );

bool IsValidFatXName( LPCTSTR szFilename );
bool IsValidXISOName( LPCTSTR szFilename );
bool IsValidUDFName( LPCTSTR szFilename );

CStdString GetArg( LPCTSTR szLine, int iArg );

HRESULT SetReadonly( LPCTSTR szPath, bool bSetReadOnly, bool bCascadeSubdirs );
bool ValidateForFATX( LPCTSTR szPath );
bool SplitURL( LPCTSTR szURL, CStdString & sServer, int & iPort, CStdString & sURI );

bool MakeFileBak( LPCTSTR szFilename );
bool MediaPatch( LPCTSTR szFilename );

CStdString FetchIDToken( int iIndex, LPCTSTR szText );

void Scramble( CStdString & sValue );
void UnScramble( CStdString & sValue );

DWORD GetFlashID( void );

LONGLONG DirectorySpace( LPCTSTR szDirectoryPath );

CStdString GetFlashString( DWORD dwID, bool bNumericOK );

CStdString GetNextCaptureFilename(void);

HRESULT GetRealDrivePath( char chDrive, CStdString & sReturn );

int RandomRange( int iLow, int iHigh );
DWORD RandomNum( void );

bool CheckList( TListStrings & slList, LPCTSTR szCheck, bool bLower = true, bool bAdd = true );

DWORD	ConvertUnsigned( LPCTSTR szValue );
long	ConvertSigned( LPCTSTR szValue );
double	ConvertFloat( LPCTSTR szValue );

int CorrectCoordinate( int iTrueDim, FLOAT fPctCoord, int iPseudoCoord = -1, int iPseudoDim = -1 );

HRESULT Mount(const char* szDrive, char* szDevice);
HRESULT Unmount(const char* szDrive);
HRESULT Remount(LPCSTR szDrive, LPSTR szDevice);

int SMBusReadCommand(unsigned char slave, unsigned char command, int isWord, unsigned short *data);
int SMBusWriteCommand(unsigned char slave, unsigned char command, int isWord, unsigned short data);


int GetFanSpeed( void );
void SetFanSpeed( int iFanSpeed );

DWORD LaunchTitlePatchFile( LPCTSTR szPath, LPCTSTR szXbe, LAUNCH_DATA * pldInfo = NULL );
bool GetTextureSize( LPDIRECT3DTEXTURE8 pTexture, int & iWidth, int & iHeight );
HRESULT DrawSpriteRects(IDirect3DDevice8* pDevice, LPDIRECT3DTEXTURE8 pTexture, RECT & rectDest, RECT & rectSrc, int iWidth = -1, int iHeight = -1, BOOL bTransparency = TRUE, BOOL bUseAlphaChannel = FALSE, int iGlobalBlend = -1, DWORD dwAddMode = D3DTADDRESS_WRAP );
HRESULT DrawSprite(IDirect3DDevice8* pDevice, LPDIRECT3DTEXTURE8 pTexture, int iPosX, int iPosY, int iSizeX, int iSizeY, BOOL bTransparency = TRUE, BOOL bUseAlphaChannel = FALSE, int iGlobalBlend = -1 );
HRESULT DrawSpriteOffset(IDirect3DDevice8* pDevice, LPDIRECT3DTEXTURE8 pTexture, int iPosX, int iPosY, int iSizeX, int iSizeY, FLOAT fTU1=0.0, FLOAT fTV1=0.0, FLOAT fTU2=1.0, FLOAT fTV2=1.0, BOOL bTransparency = TRUE, BOOL bUseAlphaChannel = FALSE, int iGlobalBlend = -1, DWORD dwAddMode = D3DTADDRESS_WRAP );
HRESULT GetPersistentTexture(IDirect3DDevice8* pDevice, LPDIRECT3DTEXTURE8 *ppTexture );
HRESULT GetBackBufferTexture(IDirect3DDevice8* pDevice, LPDIRECT3DTEXTURE8 *ppTexture );
HRESULT BuildRawTexture( IDirect3DDevice8* pDevice, int &iWidth, int &iHeight, LPDIRECT3DTEXTURE8 *ppTexture, D3DFORMAT d3dFormat = D3DFMT_A8R8G8B8 );
HRESULT DrawLine( IDirect3DDevice8* pDevice, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2,
                            DWORD dwOutlineColor );

HRESULT DrawBox( IDirect3DDevice8* pDevice, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2,
                    DWORD dwFillColor, DWORD dwOutlineColor );
HRESULT DrawTriangle( IDirect3DDevice8* pDevice, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2,
                            DWORD dwFillColor, DWORD dwOutlineColor, 
							int iDirection  );
HRESULT RenderQuadGradientBackground( IDirect3DDevice8* pDevice, DWORD dwTopLeftColor, 
										DWORD dwTopRightColor, 
										DWORD dwBottomLeftColor, 
                                        DWORD dwBottomRightColor, RECT * pRect = NULL );

HRESULT DrawTexture( IDirect3DDevice8* pDevice, IDirect3DBaseTexture8 * pTexture, int iPosX, int iPosY, int iSizeX, int iSizeY );

HRESULT DrawTextBox( IDirect3DDevice8* pDevice, LPCTSTR szText, CXBFont * pFont, DrawingAttributes * pAttributes, bool bCenterText = false );


HRESULT CopyToHardDrive( LPCTSTR szSource, LPCTSTR szDest );
HRESULT CopyFilesOrPath( LPCTSTR szBasePath, LPCTSTR szSource, LPCTSTR szDest );
HRESULT MakePath( LPCTSTR szPath );

#if 0
typedef struct _TCPIP_USER_SETTINGS
{
	bool		m_bUseStatic;
	CStdString	m_sGateway;
	CStdString	m_sSubnetMask;
	CStdString	m_sUserIP;
	CStdString	m_sDNS1;
	CStdString	m_sDNS2;
} TCPIP_USER_SETTINGS, *PTCPIP_USER_SETTINGS;
#endif

typedef struct
{
	DWORD m_dwID;
	CStdString m_sTitle;
	CStdString m_sPublisher;
	CStdString m_sDescription;
	CStdString m_sAltTitle;
} TXBEINFO;


CStdString MakeIPString( DWORD dwIPAddr );

CStdString	GetXbeTitle( LPCTSTR szFilePath );
BYTE *		GetXbeLogo( LPCTSTR szFilePath );
DWORD		GetXbeID( LPCTSTR szFilePath );
bool		GetXbeInfo( LPCTSTR szFilePath, TXBEINFO & xbeInfo );

CStdString ReadFileLine( HANDLE hFile );
int ReadFileLine( HANDLE hFile, char * szBuffer, int iMaxChars );
int WriteFileLine( HANDLE hFile, LPCSTR szString, bool bWriteEOL = true );
int ReadFileString( HANDLE hFile, char * szBuffer, int iMaxChars );

bool ParseXMLAttributes( string & attributes, TMapStrings & msAttributes, bool bLowerIDs = true );
int GetLiteral( LPCTSTR szString, CStdString & sLiteral );

void ParseFile( LPCTSTR szFilename, XmlNotify & Subscriber );
bool IsImageFile( LPCTSTR szFilename, bool bStaticOnly = false );
BYTE * LoadBufferFromFile( LPCTSTR szFilename, DWORD *pdwSize );
CStdString LoadStringFromFile( LPCTSTR szFilename );
bool SaveBufferToFile( LPCTSTR szFilename, BYTE *pbBuffer, DWORD pdwSize );
bool SaveStringToFile( LPCTSTR szFilename, LPCTSTR szString );

DWORD ConvertColorName( LPCTSTR szColor );
DWORD RebuildSymbolicLink( LPCTSTR szSymbolicDrive, LPCTSTR szNewDeviceLocation, bool bDelete = false );

DWORD DeleteSymbolicLink( TCHAR tcSymbolicDrive );

bool InitUniString( PUNICODE_STRING pUniString, int iMaxLen, bool bRawInit = true );
bool SetUniString(  PUNICODE_STRING pUniString, LPCTSTR szStr );
DWORD RebuildSymbolicLink( TCHAR tcSymbolicDrive, LPCTSTR szNewDeviceLocation );
CStdString PathSlasher( LPCTSTR szPath, bool bSlashIt );
CStdString GetTempFileName(	LPCTSTR lpPathName,      // directory name
						LPCTSTR lpPrefixString		// file name prefix
						);
CStdString FatXify( LPCTSTR szFilename, bool bIncludesPath, TCHAR tcReplace );
CStdString ReplaceFilenameExtension( LPCTSTR szFilepath, LPCTSTR szPath );
CStdString GetFilenameExtension( LPCTSTR szFilepath );
CStdString GetFilenameBase( LPCTSTR szFilepath );
CStdString ExtractFilenameFromPath( LPCTSTR szPath, bool bDotted = true );
CStdString StripFilenameFromPath( LPCTSTR szPath );
CStdString GetDeviceLocation( LPCTSTR szRealPath );
CStdString GetRealCDROMPath( void );
DWORD LaunchTitle( LPCTSTR szPath, LPCTSTR szXbe, LAUNCH_DATA * pldInfo = NULL );

BOOL  FileExists( LPCTSTR szFilename );
DWORD FileLength( LPCTSTR szFilename );

bool GetFileInfo( LPCTSTR szFilename, LPWIN32_FILE_ATTRIBUTE_DATA pFileInfo );

CStdString MakeFullDirPath( LPCTSTR szMainPath, LPCTSTR szPath, BOOL bStripTrailingSlash=FALSE );
CStdString MakeFullFilePath( LPCTSTR szMainPath, LPCTSTR szPath, BOOL bZapNullName = FALSE );
CStdString FindImageFile( LPCTSTR szMainPath, LPCTSTR szPath );

CStdString MakeNiceNumber( ULONGLONG & u64Value, TCHAR tcComma, bool bShort = false, bool bBytes = false );

CStdString GetDiskFree( char cDrive, DWORD dwDivisor, bool bShort = false );
CStdString GetDiskTotal( char cDrive, DWORD dwDivisor, bool bShort = false );
CStdString GetDiskUsed( char cDrive, DWORD dwDivisor, bool bShort = false );

class CUniString
{
	UNICODE_STRING m_uniData;
	CStdString     m_sActualData;
public:
	CUniString();
	~CUniString();
	PUNICODE_STRING GetUnicodeString( void );
	CStdString* GetString( void );
};




#endif