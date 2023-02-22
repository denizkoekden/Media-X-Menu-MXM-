

#ifndef __BINDATA_H__
#define __BINDATA_H__

//extern const BYTE g_pbMatrixFont[34816];
extern const BYTE g_pbMatrixFont[36864];

extern BYTE g_pbICO_ACTIONData[1592];
const DWORD g_dwICO_ACTIONLength = 1592;

extern BYTE g_pbICO_DIRData[620];
const DWORD g_dwICO_DIRLength = 620;

extern BYTE g_pbICO_DVDData[819];
const DWORD g_dwICO_DVDLength = 819;

extern BYTE g_pbICO_EMUData[1455];
const DWORD g_dwICO_EMULength = 1455;

extern BYTE g_pbICO_FILEData[857];
const DWORD g_dwICO_FILELength = 857;

extern BYTE g_pbICO_HDData[851];
const DWORD g_dwICO_HDLength = 851;

extern BYTE g_pbICO_LOCKData[603];
const DWORD g_dwICO_LOCKLength = 603;

extern BYTE g_pbICO_MEDIAData[1077];
const DWORD g_dwICO_MEDIALength = 1077;

extern BYTE g_pbICO_MUSICData[1006];
const DWORD g_dwICO_MUSICLength = 1006;

extern BYTE g_pbICO_WAVEData[1305];
const DWORD g_dwICO_WAVELength = 1305;

extern BYTE g_pbICO_XBEData[666];
const DWORD g_dwICO_XBELength = 666;




extern const BYTE g_pbSkyTileData[3135];
const DWORD g_dwSkyTileLength = 3135;

extern const BYTE g_pbLargeLogoData[4675];
const DWORD g_dwLargeLogoLength = 4675;

extern const BYTE g_pbTitleImage[10240];
const DWORD g_pbTitleImageLength = 10240;


// #define HELPSCREENS "HelpData"
#define HELPGCSCREEN "HelpData"
#define HELPKBSCREEN "HelpData"
#define HELPIRSCREEN "HelpData"

extern const BYTE g_pbHelpKB[29505];
const DWORD g_dwHelpKBLength = 29505;
const LPTSTR c_szHelpKBSection = HELPKBSCREEN; // "HelpKBScreen";
//const LPTSTR c_szHelpKBSection = "LogoData";


extern const BYTE g_pbHelpGC[30037];
const DWORD g_dwHelpGCLength = 30037;
const LPTSTR c_szHelpGCSection = HELPGCSCREEN; // "HelpGCScreen";
//const LPTSTR c_szHelpGCSection = "LogoData";

extern const BYTE g_pbHelpIR[24740];
const DWORD g_dwHelpIRLength = 24740;
const LPTSTR c_szHelpIRSection = HELPIRSCREEN; // "HelpIRScreen";
// const LPTSTR c_szHelpIRSection = "LogoData";

struct _TPresetApps
{
    DWORD m_dwAppCode;
    LPCTSTR m_szAppTitle;
    LPCTSTR m_szDescr;
};

struct _TPublishers
{
    DWORD m_dwAppCode;
    LPCTSTR m_szPublisher;
};

extern struct _TPublishers g_Publishers[];
extern struct _TPresetApps g_Presets[];


#endif // __BINDATA_H__