


#include "Helpers.h"
#include "MenuInfo.h"
#include "CommDebug.h"
#include "RenderGadget.h"
#include "StdString.h"
#include "Utils.h"
#include "ImgFileCache.h"
#include "BinData.h"

bool IsNTSC( void );

// Allocates a node and loads it with XDI info.
// Will return NULL if no XDI is found.
// Fills it with Item nodes
CXMLNode * GetXDI( void )
{
	CXMLNode * pReturn = NULL;
	CXMLNode * pItemNode = NULL;
	HANDLE hFile;
	DWORD dwRead;
	DWORD dwFileLength;
	DWORD dwItemSize;
	CStdString sStringValue;
	char * pBuffer;
	DWORD dwNumItems;
	char * pPos;
	char * pNextItem;
	LPCTSTR szTitle;


	dwFileLength = FileLength(_T("D:\\MenuX\\menu.xdi"));
	if ( dwFileLength > 0x118 )
	{
		dwFileLength -= 0x118;
		pBuffer = new char[dwFileLength];
		if ( pBuffer )
		{
			hFile = CreateFile( _T("D:\\MenuX\\menu.xdi"), 
								GENERIC_READ, 
								FILE_SHARE_READ, 
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL );

			if ( hFile != INVALID_HANDLE_VALUE )
			{
				if ( SetFilePointer(	hFile,  0x0c, NULL, FILE_BEGIN ) == 0x0c )
				{
					if ( !ReadFile( hFile, &dwNumItems, 4, &dwRead, NULL ) )
					{
						dwNumItems = 0;
					}
				}
				if ( dwNumItems )
				{
					if ( SetFilePointer( hFile,  0x118, NULL, FILE_BEGIN ) == 0x118 )
					{
						if ( ReadFile( hFile, pBuffer, dwFileLength, &dwRead, NULL ) )
						{
							if ( dwRead == dwFileLength )
							{
								// Now, allocate node and start reading data
								pReturn = new CXMLNode( NULL, _T("Config") );
								// Start filling data....
								pPos = pBuffer;
								while( dwNumItems-- )
								{
									dwItemSize = *((DWORD*)pPos );
									pNextItem = pPos + dwItemSize;

									pItemNode = pReturn->AddNode(_T("Item"));
									if ( pItemNode )
									{
										// Parse out item...
										pPos += 0x2c;

										// Grab Title
										sStringValue = pPos;
										pPos += (sStringValue.GetLength()+1);

										szTitle = sStringValue.c_str();

										pItemNode->AddElement( _T("Title"), sStringValue );


										// Unknown
										sStringValue = pPos;
										pPos += (sStringValue.GetLength()+1);

										// Rel XBE Path
										sStringValue = pPos;
										pPos += (sStringValue.GetLength()+1);

										sStringValue.Format( _T("D:\\%s"), sStringValue.c_str() );
										pItemNode->AddElement( _T("Dir"), StripFilenameFromPath(sStringValue) );
										pItemNode->AddElement( _T("Exe"), sStringValue );


										// Rel sshot Path
										sStringValue = pPos;
										pPos += (sStringValue.GetLength()+1);

										sStringValue.Format( _T("D:\\menux\\%s"), sStringValue.c_str() );
										pItemNode->AddElement( _T("Media"), sStringValue );


										// Unknown
										sStringValue = pPos;
										pPos += (sStringValue.GetLength()+1);

										// Unknown
										sStringValue = pPos;
										pPos += (sStringValue.GetLength()+1);

										// Unknown
										sStringValue = pPos;
										pPos += (sStringValue.GetLength()+1);

										// Description
										sStringValue = pPos;
										pItemNode->AddElement( _T("Description"), sStringValue );
										pItemNode->AddElement( _T("Descr"), sStringValue );

										pPos += (sStringValue.GetLength()+1);

										/*
										DWORD dwSize;
										DWORD dwUknown2[10];
										char szTitle[];
										char szUnknown1[];
										char szRelXbePath[];
										char szRelSshotPath[];
										char szUnknown2[];
										char szUnknown3[];
										char szTMicBas[];
										char szDesc[];
										*/
									}
									pPos = pNextItem;
								}
							}
						}
					}
				}
				CloseHandle(hFile);
			}
			delete pBuffer;
		}
	}
	if ( pReturn )
	{
		// DumpNode( pReturn );
	}
	return pReturn;
}


void SetPositionInNode( CXMLNode * pNode, int iLeft, int iTop, int iW, int iH, int iX, int iY )
{
	CXMLNode * pPosNode;

	if ( pNode )
	{
		pPosNode = pNode->AddNode(_T("pos"));
		if ( pPosNode )
		{
			if ( iTop != -1 )
			{
				pPosNode->SetLong( NULL, _T("t"), iTop, true );
			}
			if ( iLeft != -1 )
			{
				pPosNode->SetLong( NULL, _T("l"), iLeft, true );
			}
			if ( iW != -1 )
			{
				pPosNode->SetLong( NULL, _T("w"), iW, true );
			}
			if ( iH != -1 )
			{
				pPosNode->SetLong( NULL, _T("h"), iH, true );
			}
			if ( iX != -1 )
			{
				pPosNode->SetLong( NULL, _T("x"), iX, true );
			}
			if ( iY != -1 )
			{
				pPosNode->SetLong( NULL, _T("y"), iY, true );
			}
		}
	}
}

void SetScalingInNode( CXMLNode * pNode, bool bScaleX, bool bScaleY )
{
	CXMLNode * pScaleNode;

	if ( pNode )
	{
		pScaleNode = pNode->AddNode(_T("scale"));
		if ( pScaleNode )
		{
			pScaleNode->SetBool( NULL, _T("x"), bScaleX?TRUE:FALSE, true );
			pScaleNode->SetBool( NULL, _T("y"), bScaleY?TRUE:FALSE, true );
		}
	}
}

void MakeThrob( CXMLNode * pNode, LPCTSTR szTarget, int iRPeriod, int iGPeriod, int iBPeriod )
{
	CXMLNode * pModifierNode;
	pModifierNode = pNode->AddNode( _T("Modifier") );
	if ( pModifierNode )
	{
		pModifierNode->SetString( NULL, _T("Type"), _T("Throb"), true );
		pModifierNode->SetString( NULL, _T("Target"), szTarget, true );
		pModifierNode->SetBool( _T("Red"), _T("Enable"), TRUE, true );
		pModifierNode->SetLong( _T("Red"), _T("Min"), 0, true );
		pModifierNode->SetLong( _T("Red"), _T("Max"), 255, true );
		pModifierNode->SetLong( _T("Red"), _T("Period"), iRPeriod, true );
		pModifierNode->SetBool( _T("Green"), _T("Enable"), TRUE, true );
		pModifierNode->SetLong( _T("Green"), _T("Min"), 0, true );
		pModifierNode->SetLong( _T("Green"), _T("Max"), 255, true );
		pModifierNode->SetLong( _T("Green"), _T("Period"), iGPeriod, true );
		pModifierNode->SetBool( _T("Blue"), _T("Enable"), TRUE, true );
		pModifierNode->SetLong( _T("Blue"), _T("Min"), 0, true );
		pModifierNode->SetLong( _T("Blue"), _T("Max"), 255, true );
		pModifierNode->SetLong( _T("Blue"), _T("Period"), iBPeriod, true );
	}
}


void BuildSaverLayout( CXMLNode * pSkinNode )
{
	CXMLNode * pLayoutNode;

	pLayoutNode = pSkinNode->AddNode( _T("layout") );
	if ( pLayoutNode )
	{
		// Bunch of informational text fields here....
		pLayoutNode->SetString( NULL, _T("screen"), _T("saver") );
		CXMLNode * pElementNode;
		CXMLNode * pModifierNode;

		pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
		if ( pElementNode )
		{
			pElementNode->SetString( NULL, _T("type"), _T("ColorShiftBox") );
			pElementNode->SetString( NULL, _T("source"), _T("load") );
			SetPositionInNode( pElementNode, 0, 0, 640, 480 );
			SetScalingInNode( pElementNode, true, true );
			MakeThrob( pElementNode, _T("ColorUL"), 9500+(RandomNum()%500), 9500+(RandomNum()%500), 9500+(RandomNum()%500) );
			MakeThrob( pElementNode, _T("ColorUR"), 9500+(RandomNum()%500), 9500+(RandomNum()%500), 9500+(RandomNum()%500) );
			MakeThrob( pElementNode, _T("ColorLL"), 9500+(RandomNum()%500), 9500+(RandomNum()%500), 9500+(RandomNum()%500) );
			MakeThrob( pElementNode, _T("ColorLR"), 9500+(RandomNum()%500), 9500+(RandomNum()%500), 9500+(RandomNum()%500) );
		}
		pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
		if ( pElementNode )
		{
			pElementNode->SetString( NULL, _T("type"), _T("image") );
			pElementNode->SetString( NULL, _T("source"), _T("logo") );
			SetPositionInNode( pElementNode, (RandomNum()%300)+150, (RandomNum()%300)+150, 220, 150 );
			SetScalingInNode( pElementNode, true, true );
			pElementNode->SetBool( NULL, _T("proportional"), TRUE );
			pModifierNode = pElementNode->AddNode( _T("Modifier") );
			if ( pModifierNode )
			{
				pModifierNode->SetString( NULL, _T("Type"), _T("Pulse"), true );
				pModifierNode->SetLong( _T("Range"), _T("x"), 10, true );
				pModifierNode->SetLong( _T("Range"), _T("y"), 10, true );
			}
			pModifierNode = pElementNode->AddNode( _T("Modifier") );
			if ( pModifierNode )
			{
				pModifierNode->SetString( NULL, _T("Type"), _T("Bounce"), true );
				pModifierNode->SetLong( _T("Bounds"), _T("l"), 20, true );
				pModifierNode->SetLong( _T("Bounds"), _T("r"), 620, true );
				pModifierNode->SetLong( _T("Bounds"), _T("t"), 20, true );
				pModifierNode->SetLong( _T("Bounds"), _T("b"), 460, true );
			}
		}
		pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
		if ( pElementNode )
		{
			pElementNode->SetString( NULL, _T("type"), _T("MatrixEffect") );
			pElementNode->SetDWORD( NULL, _T("Color"), 0xff0000ff, false );
			MakeThrob( pElementNode, _T("Color"), 4500+(RandomNum()%5000), 4500+(RandomNum()%5000), 4500+(RandomNum()%5000) );
		}
		pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
		if ( pElementNode )
		{
			pElementNode->SetString( NULL, _T("type"), _T("text") );
			pElementNode->SetString( NULL, _T("source"), _T("datetime") );
			SetPositionInNode( pElementNode, 425, 55, 160, 32 );
			SetScalingInNode( pElementNode, true, true );
			pElementNode->SetString( NULL, _T("font"), _T("default") );
			pElementNode->SetString( _T("justify"), _T("x"), _T("center") );
			pElementNode->SetString( _T("justify"), _T("y"), _T("center") );
			pElementNode->SetDWORD( NULL, _T("textcolor"), 0xFFd0E53E );
			pElementNode->SetDWORD( NULL, _T("shadowcolor"), 0xa0434d0e );
			pElementNode->SetBool( NULL, _T("doglow"), TRUE );
			pElementNode->SetBool( NULL, _T("proportional"), TRUE );
			pModifierNode = pElementNode->AddNode( _T("Modifier") );
			if ( pModifierNode )
			{
				pModifierNode->SetString( NULL, _T("Type"), _T("Throb"), true );
				pModifierNode->SetString( NULL, _T("Target"), _T("TextColor"), true );
				pModifierNode->SetBool( _T("Red"), _T("Enable"), TRUE, true );
				pModifierNode->SetLong( _T("Red"), _T("Min"), 50, true );
				pModifierNode->SetLong( _T("Red"), _T("Max"), 120, true );
				pModifierNode->SetLong( _T("Red"), _T("Period"), 1000, true );
				pModifierNode->SetBool( _T("Green"), _T("Enable"), TRUE, true );
				pModifierNode->SetLong( _T("Green"), _T("Min"), 100, true );
				pModifierNode->SetLong( _T("Green"), _T("Max"), 255, true );
				pModifierNode->SetLong( _T("Green"), _T("Period"), 1000, true );
				pModifierNode->SetBool( _T("Blue"), _T("Enable"), TRUE, true );
				pModifierNode->SetLong( _T("Blue"), _T("Min"), 25, true );
				pModifierNode->SetLong( _T("Blue"), _T("Max"), 75, true );
				pModifierNode->SetLong( _T("Blue"), _T("Period"), 1000, true );
			}
			pModifierNode = pElementNode->AddNode( _T("Modifier") );
			if ( pModifierNode )
			{
				pModifierNode->SetString( NULL, _T("Type"), _T("Bounce"), true );
				pModifierNode->SetLong( _T("Bounds"), _T("l"), 20, true );
				pModifierNode->SetLong( _T("Bounds"), _T("r"), 620, true );
				pModifierNode->SetLong( _T("Bounds"), _T("t"), 20, true );
				pModifierNode->SetLong( _T("Bounds"), _T("b"), 460, true );
			}

		}
	}
}

/*




*/
void BuildDefaultSkin( void )
{
	CXMLNode * pSkinNode;

	pSkinNode = g_SkinNodes.AddNode( _T("skin") );
	if ( pSkinNode )
	{
		pSkinNode->SetString( NULL, _T("name"), _T("Internal"), true );
		pSkinNode->SetString( NULL, _T("version"), _T("2.0"), true );

		// Build Information....
		CXMLNode * pInformationNode;
		pInformationNode = pSkinNode->AddNode( _T("information") );
		if ( pInformationNode )
		{
			pInformationNode->SetString( NULL, _T("author"), _T("BenJeremy") );
			pInformationNode->SetString( NULL, _T("copyright"), _T("2003") );
			pInformationNode->SetString( NULL, _T("skinversion"), _T("1.0") );
		}

		
		CXMLNode * pDelayNode;
		pDelayNode = pSkinNode->AddNode( _T("delays") );
		if ( pDelayNode )
		{
			pDelayNode->SetLong( NULL, _T("load"), 1000 );
			pDelayNode->SetLong( NULL, _T("title"), 0 );
			pDelayNode->SetLong( NULL, _T("saver"), 300 );
		}

		// Sounds:
		//  up
		//  down
		//  launch
		//  SaverBounce
		//  titleopen
		//  titleclose
		CXMLNode * pMusicNode;

		pMusicNode = pSkinNode->AddNode( _T("music") );
		if ( pMusicNode )
		{
			pMusicNode->SetBool( NULL, _T("PlayGlobal"), TRUE );
			pMusicNode->SetBool( NULL, _T("PlayRandom"), TRUE );
		}
		
		// Add resources....
		CXMLNode * pResNode;

		pResNode = pSkinNode->AddNode( _T("resource") );
		if ( pResNode )
		{
			pResNode->SetString(NULL, _T("type"), _T("image") );
			pResNode->SetString(NULL, _T("name"), _T("ScreenShot") );
			pResNode->SetString(NULL, _T("EntryNamed"), _T("media") );
		}

		pResNode = pSkinNode->AddNode( _T("resource") );
		if ( pResNode )
		{
			pResNode->SetString(NULL, _T("type"), _T("image") );
			pResNode->SetString(NULL, _T("name"), _T("Logo") );
			pResNode->SetString(NULL, _T("File"), _T("::Logo") );
		}

		// Add Layouts....
		CXMLNode * pLayoutNode;

		// Title (yeah, we got this, for info purposes)
		pLayoutNode = pSkinNode->AddNode( _T("layout") );
		if ( pLayoutNode )
		{
			// Bunch of informational text fields here....
			pLayoutNode->SetString( NULL, _T("screen"), _T("title") );
		}

		// Menu Screen
		pLayoutNode = pSkinNode->AddNode( _T("layout") );
		if ( pLayoutNode )
		{
			// Bunch of informational text fields here....
			pLayoutNode->SetString( NULL, _T("screen"), _T("menu") );
			CXMLNode * pElementNode;
			CXMLNode * pModifierNode;

			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("box") );
				SetPositionInNode( pElementNode, 0, 0, 640, 480 );
				pElementNode->SetDWORD( NULL, _T("color"), 0xff202020, false );
				pElementNode->SetDWORD( NULL, _T("bordercolor"), 0x00000000, false );
				pModifierNode = pElementNode->AddNode( _T("Modifier") );
				if ( pModifierNode )
				{
					pModifierNode->SetString( NULL, _T("type"), _T("throb") );
					pModifierNode->SetString( NULL, _T("target"), _T("color") );
					pModifierNode->SetBool( _T("green"), _T("enable"), TRUE );
					pModifierNode->SetLong( _T("green"), _T("min"), 20 );
					pModifierNode->SetLong( _T("green"), _T("max"), 80 );
					pModifierNode->SetLong( _T("green"), _T("period"), 3000 );
				}
			}
			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("matrixeffect") );
				pElementNode->SetDWORD( NULL, _T("color"), 0xff0000ff, false );
			}
			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("box") );
				SetPositionInNode( pElementNode, 330, 118, 260, 195 );
				pElementNode->SetDWORD( NULL, _T("color"), 0x80000000, false );
				pElementNode->SetDWORD( NULL, _T("bordercolor"), 0x00000000, false );
			}
			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("box") );
				SetPositionInNode( pElementNode, 66, 96, 231, 235 );
				pElementNode->SetDWORD( NULL, _T("color"), 0x80000000, false );
				pElementNode->SetDWORD( NULL, _T("bordercolor"), 0x00000000, false );
			}
			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("image") );
				pElementNode->SetString( NULL, _T("source"), _T("ScreenShot") );
				SetPositionInNode( pElementNode, 330, 118, 260, 195 );
				SetScalingInNode( pElementNode, true, true );
				pElementNode->SetString( _T("justify"), _T("x"), _T("center") );
				pElementNode->SetString( _T("justify"), _T("y"), _T("center") );
				pElementNode->SetBool( NULL, _T("proportional"), TRUE );
				pElementNode->SetBool( _T("sound"), _T("enable"), TRUE );
				pElementNode->SetBool( _T("sound"), _T("suspendmusic"), TRUE );
			}
			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("menu") );
				pElementNode->SetString( NULL, _T("source"), _T("MenuX") );
				SetPositionInNode( pElementNode, 66, 96, 231, 235 );
				SetScalingInNode( pElementNode, true, true );
				pElementNode->SetString( NULL, _T("TitleFont"), _T("default"), false );
				pElementNode->SetString( NULL, _T("DescrFont"), _T("default"), false );
				pElementNode->SetLong( NULL, _T("TitleSquash"), -2, false );
				pElementNode->SetLong( NULL, _T("DescrSquash"), -5, false );
				pElementNode->SetLong( NULL, _T("ThrobPeriod"), 5000, false );
				pElementNode->SetBool( NULL, _T("DrawDesc"), TRUE, false );
			}
		}

		// Load Screen
		pLayoutNode = pSkinNode->AddNode( _T("layout") );
		if ( pLayoutNode )
		{
			// Bunch of informational text fields here....
			pLayoutNode->SetString( NULL, _T("screen"), _T("load") );
			CXMLNode * pElementNode;

			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("image") );
				pElementNode->SetString( NULL, _T("source"), _T("load") );
				SetPositionInNode( pElementNode, 0, 0, 640, 480 );
				SetScalingInNode( pElementNode, true, true );
			}
			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("text") );
				pElementNode->SetString( NULL, _T("source"), _T("cgtitle") );
				SetPositionInNode( pElementNode, -1, -1, 540, 46, 320, 55 );
				SetScalingInNode( pElementNode, true, true );
				pElementNode->SetString( NULL, _T("font"), _T("default") );
				pElementNode->SetString( _T("justify"), _T("x"), _T("center") );
				pElementNode->SetString( _T("justify"), _T("y"), _T("center") );
				pElementNode->SetDWORD( NULL, _T("textcolor"), 0xFFd0E53E );
				pElementNode->SetDWORD( NULL, _T("shadowcolor"), 0xa0434d0e );
				pElementNode->SetBool( NULL, _T("doglow"), TRUE );
				pElementNode->SetBool( NULL, _T("proportional"), TRUE );
			}
			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("text") );
				pElementNode->SetString( NULL, _T("source"), _T("cgdescr") );
				SetPositionInNode( pElementNode, -1, -1, 540, 32, 320, 94 );
				SetScalingInNode( pElementNode, true, true );
				pElementNode->SetString( NULL, _T("font"), _T("default") );
				pElementNode->SetString( _T("justify"), _T("x"), _T("center") );
				pElementNode->SetString( _T("justify"), _T("y"), _T("center") );
				pElementNode->SetDWORD( NULL, _T("textcolor"), 0xFFd0E53E );
				pElementNode->SetDWORD( NULL, _T("shadowcolor"), 0xa0434d0e );
				pElementNode->SetBool( NULL, _T("doglow"), TRUE );
				pElementNode->SetBool( NULL, _T("proportional"), TRUE );
			}
			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("text") );
				pElementNode->SetString( NULL, _T("constant"), _T("Loading...") );
				SetPositionInNode( pElementNode, -1, -1, 540, 64, 320, 194 );
				SetScalingInNode( pElementNode, true, true );
				pElementNode->SetString( NULL, _T("font"), _T("default") );
				pElementNode->SetString( _T("justify"), _T("x"), _T("center") );
				pElementNode->SetString( _T("justify"), _T("y"), _T("center") );
				pElementNode->SetDWORD( NULL, _T("textcolor"), 0xFFd0E53E );
				pElementNode->SetDWORD( NULL, _T("shadowcolor"), 0xa0434d0e );
				pElementNode->SetBool( NULL, _T("doglow"), TRUE );
				pElementNode->SetBool( NULL, _T("proportional"), TRUE );
			}
			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("text") );
				pElementNode->SetString( NULL, _T("constant"), _T("Please Wait") );
				SetPositionInNode( pElementNode, -1, -1, 540, 64, 320, 258 );
				SetScalingInNode( pElementNode, true, true );
				pElementNode->SetString( NULL, _T("font"), _T("default") );
				pElementNode->SetString( _T("justify"), _T("x"), _T("center") );
				pElementNode->SetString( _T("justify"), _T("y"), _T("center") );
				pElementNode->SetDWORD( NULL, _T("textcolor"), 0xFFd0E53E );
				pElementNode->SetDWORD( NULL, _T("shadowcolor"), 0xa0434d0e );
				pElementNode->SetBool( NULL, _T("doglow"), TRUE );
				pElementNode->SetBool( NULL, _T("proportional"), TRUE );
			}
		}
		BuildSaverLayout( pSkinNode );
	}
}


void BuildXDISkin( void )
{
	CXMLNode * pSkinNode;

	pSkinNode = g_SkinNodes.AddNode( _T("skin") );
	if ( pSkinNode )
	{
		pSkinNode->SetString( NULL, _T("name"), _T("XDI"), true );
		pSkinNode->SetString( NULL, _T("version"), _T("2.0"), true );

		// Build Information....
		CXMLNode * pInformationNode;
		pInformationNode = pSkinNode->AddNode( _T("information") );
		if ( pInformationNode )
		{
			pInformationNode->SetString( NULL, _T("author"), _T("BenJeremy") );
			pInformationNode->SetString( NULL, _T("copyright"), _T("2003") );
			pInformationNode->SetString( NULL, _T("skinversion"), _T("1.0") );
		}

		
		CXMLNode * pDelayNode;
		pDelayNode = pSkinNode->AddNode( _T("delays") );
		if ( pDelayNode )
		{
			pDelayNode->SetLong( NULL, _T("load"), 1000 );
			pDelayNode->SetLong( NULL, _T("title"), 0 );
			pDelayNode->SetLong( NULL, _T("saver"), 300 );
		}

		// Sounds:
		//  up
		//  down
		//  launch
		//  SaverBounce
		//  titleopen
		//  titleclose
		CXMLNode * pSoundNode;

		pSoundNode = pSkinNode->AddNode( _T("resource") );
		if ( pSoundNode )
		{
			pSoundNode->SetString( NULL, _T("name"), _T("menuup") );
			pSoundNode->SetString( NULL, _T("type"), _T("sound"), true );
			pSoundNode->SetString( NULL, _T("file"), _T("D:\\MenuX\\media\\menu.wav") );
		}

		pSoundNode = pSkinNode->AddNode( _T("resource") );
		if ( pSoundNode )
		{
			pSoundNode->SetString( NULL, _T("name"), _T("menudown") );
			pSoundNode->SetString( NULL, _T("type"), _T("sound"), true );
			pSoundNode->SetString( NULL, _T("file"), _T("D:\\MenuX\\media\\menu.wav") );
		}

		pSoundNode = pSkinNode->AddNode( _T("resource") );
		if ( pSoundNode )
		{
			pSoundNode->SetString( NULL, _T("name"), _T("menulaunch") );
			pSoundNode->SetString( NULL, _T("type"), _T("sound"), true );
			pSoundNode->SetString( NULL, _T("file"), _T("D:\\MenuX\\media\\launch.wav") );
		}

		CXMLNode * pMusicNode;

		pMusicNode = pSkinNode->AddNode( _T("music") );
		if ( pMusicNode )
		{
			pMusicNode->AddElement( _T("directory"), _T("D:\\MenuX\\media") );
			pMusicNode->SetBool( NULL, _T("PlayGlobal"), FALSE );
			pMusicNode->SetBool( NULL, _T("PlayRandom"), FALSE );
		}
		
		// Add resources....
		CXMLNode * pResNode;

		// Fonts
		pResNode = pSkinNode->AddNode( _T("resource") );
		if ( pResNode )
		{
			pResNode->SetString( NULL, _T("name"), _T("fontb"), true );
			pResNode->SetString( NULL, _T("type"), _T("font"), true );
			pResNode->SetString( NULL, _T("file"), _T("D:\\MenuX\\media\\fontb.xpr"), false );
		}
		pResNode = pSkinNode->AddNode( _T("resource") );
		if ( pResNode )
		{
			pResNode->SetString( NULL, _T("name"), _T("fontn"), true );
			pResNode->SetString( NULL, _T("type"), _T("font"), true );
			pResNode->SetString( NULL, _T("file"), _T("D:\\MenuX\\media\\fontn.xpr"), false );
		}

		// Screens
		pResNode = pSkinNode->AddNode( _T("resource") );
		if ( pResNode )
		{
			pResNode->SetString( NULL, _T("name"), _T("menu"), true );
			pResNode->SetString( NULL, _T("type"), _T("image"), true );
			pResNode->SetString( NULL, _T("filebasename"), _T("D:\\MenuX\\media\\screen"), false );
		}
		pResNode = pSkinNode->AddNode( _T("resource") );
		if ( pResNode )
		{
			pResNode->SetString( NULL, _T("name"), _T("load"), true );
			pResNode->SetString( NULL, _T("type"), _T("image"), true );
			pResNode->SetString( NULL, _T("filebasename"), _T("D:\\MenuX\\media\\wait"), false );
			// pResNode->SetString( NULL, _T("file"), _T("D:\\MenuX\\media\\wait.png"), false );
		}

		pResNode = pSkinNode->AddNode( _T("resource") );
		if ( pResNode )
		{
			pResNode->SetString(NULL, _T("type"), _T("image") );
			pResNode->SetString(NULL, _T("name"), _T("ScreenShot") );
			pResNode->SetString(NULL, _T("EntryNamed"), _T("media") );
		}

		pResNode = pSkinNode->AddNode( _T("resource") );
		if ( pResNode )
		{
			pResNode->SetString(NULL, _T("type"), _T("image") );
			pResNode->SetString(NULL, _T("name"), _T("Logo") );
			pResNode->SetString(NULL, _T("File"), _T("::Logo") );
		}


		// Add Layouts....
		CXMLNode * pLayoutNode;

		// Title (yeah, we got this, for info purposes)
		pLayoutNode = pSkinNode->AddNode( _T("layout") );
		if ( pLayoutNode )
		{
			// Bunch of informational text fields here....
			pLayoutNode->SetString( NULL, _T("screen"), _T("title") );
		}

		// Menu Screen
		pLayoutNode = pSkinNode->AddNode( _T("layout") );
		if ( pLayoutNode )
		{
			// Bunch of informational text fields here....
			pLayoutNode->SetString( NULL, _T("screen"), _T("menu") );
			CXMLNode * pElementNode;

			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("image") );
				pElementNode->SetString( NULL, _T("source"), _T("menu") );
				SetPositionInNode( pElementNode, 0, 0, 640, 480 );
				SetScalingInNode( pElementNode, true, true );
				pElementNode->SetBool( _T("sound"), _T("enable"), TRUE, false );
				pElementNode->SetBool( _T("sound"), _T("SuspendMusic"), FALSE, false );
			}
			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("image") );
				pElementNode->SetString( NULL, _T("source"), _T("ScreenShot") );
				SetPositionInNode( pElementNode, 330, 118, 260, 195 );
				SetScalingInNode( pElementNode, true, true );
				pElementNode->SetBool( _T("sound"), _T("enable"), TRUE );
				pElementNode->SetBool( _T("sound"), _T("suspendmusic"), TRUE );
				pElementNode->SetString( _T("justify"), _T("x"), _T("center") );
				pElementNode->SetString( _T("justify"), _T("y"), _T("center") );
				pElementNode->SetBool( NULL, _T("proportional"), TRUE );
			}
			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("menu") );
				pElementNode->SetString( NULL, _T("source"), _T("MenuX") );
				SetPositionInNode( pElementNode, 66, 96, 231, 235 );
				SetScalingInNode( pElementNode, true, true );
				pElementNode->SetString( NULL, _T("TitleFont"), _T("fontb"), false );
				pElementNode->SetString( NULL, _T("DescrFont"), _T("fontn"), false );
				pElementNode->SetLong( NULL, _T("TitleSquash"), -4, false );
				pElementNode->SetLong( NULL, _T("DescrSquash"), -7, false );
				pElementNode->SetLong( NULL, _T("ThrobPeriod"), 5000, false );
				pElementNode->SetBool( NULL, _T("DrawDesc"), TRUE, false );
//				pElementNode->SetDWORD( NULL, _T("SelColor"), 0xFFFFFF00 );
//				pElementNode->SetDWORD( NULL, _T("ArrowColor"), 0xFF00FFFF );
				pElementNode->SetBool( NULL, _T("DrawDesc"), true );
//				pElementNode->SetDWORD( NULL, _T("MenuColor"), 0xFFFF0000 );
			}
		}

		// Load Screen
		pLayoutNode = pSkinNode->AddNode( _T("layout") );
		if ( pLayoutNode )
		{
			// Bunch of informational text fields here....
			pLayoutNode->SetString( NULL, _T("screen"), _T("load") );
			CXMLNode * pElementNode;

			pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
			if ( pElementNode )
			{
				pElementNode->SetString( NULL, _T("type"), _T("image") );
				pElementNode->SetString( NULL, _T("source"), _T("load") );
				SetPositionInNode( pElementNode, 0, 0, 640, 480 );
				SetScalingInNode( pElementNode, true, true );
			}
		}
		BuildSaverLayout( pSkinNode );

	}
}

void BuildOldSkins( CXMLNode * pMXMXML, CXMLNode * pOldSkinXML, LPCTSTR szName, LPCTSTR szBasePath )
{
	CXMLNode * pSkinNode;
	CStdString sName;
	CXMLNode xmlEmptyNode;

	if ( pMXMXML == NULL )
	{
		pMXMXML = &xmlEmptyNode;
	}
	if ( szName && _tcslen(szName) )
	{
		sName.Format(_T("%s"), szName );
	}
	if ( pOldSkinXML )
	{
		pSkinNode = g_SkinNodes.AddNode( _T("skin") );
		if ( pSkinNode )
		{
			DEBUG_FORMAT(_T("BUILDING OLD SKIN: %s"), sName.c_str() );
			pSkinNode->SetString( NULL, _T("name"), sName, true );
			pSkinNode->SetString( NULL, _T("directory"), szBasePath, false );
			pSkinNode->SetString( NULL, _T("version"), _T("2.0"), true );

			// Build Information....
			CXMLNode * pInformationNode;
			pInformationNode = pSkinNode->AddNode( _T("information") );
			if ( pInformationNode )
			{
				pInformationNode->SetString( NULL, _T("author"), _T("Unknown") );
				pInformationNode->SetString( NULL, _T("copyright"), _T("2003") );
				pInformationNode->SetString( NULL, _T("skinversion"), _T("1.0") );
			}
			// Uses only default fonts, so no need to get that info at the moment.
			// We do need to load in the image info....
			// and set up our defaults for screen shots and such
			// Screens:
			//  Title
			//  Menu
			//  Load
			//  ScreenShot
			CXMLNode * pResNode;

			// Screenshots are automatic....
			// We either used media references in entry or basenamed entries if available
			pResNode = pSkinNode->AddNode( _T("resource") );
			if ( pResNode )
			{
				pResNode->SetString(NULL, _T("type"), _T("image") );
				pResNode->SetString(NULL, _T("name"), _T("ScreenShot") );
				pResNode->SetString(NULL, _T("EntryNamed"), _T("media") );
			}
			pResNode = pSkinNode->AddNode( _T("resource") );
			if ( pResNode )
			{
				pResNode->SetString(NULL, _T("type"), _T("image") );
				pResNode->SetString(NULL, _T("name"), _T("ScreenShotAlt") );
				pResNode->SetString(NULL, _T("EntryBaseName"), _T("MXM_SS") );
			}

			// Title background
			// Default to MXM.xml setting, if it's there
			sName = pMXMXML->GetString( _T("main"), _T("titlescreen"), _T(""), true );
			if ( sName.GetLength() )
			{
				// Only append full path here if we actually got something here.
				// Otherwise, relative filenames will get skin path appended
				sName = MakeFullFilePath( g_MenuInfo.m_sMXMPath, sName );
			}
			sName = pOldSkinXML->GetString( _T("backgrounds"), _T("title"), sName, true );
			if ( sName.GetLength() )
			{
				sName = MakeFullFilePath( szBasePath, sName );
				pResNode = pSkinNode->AddNode( _T("resource") );
				if ( pResNode )
				{
					pResNode->SetString(NULL, _T("type"), _T("image") );
					pResNode->SetString(NULL, _T("name"), _T("title") );
					pResNode->SetString(NULL, _T("file"), sName );
				}
			}

			// Loading screen
			// Default to MXM.xml setting, if it's there
			sName = pMXMXML->GetString( _T("main"), _T("WaitScreen"), _T(""), true );
			if ( sName.GetLength() )
			{
				// Only append full path here if we actually got something here.
				// Otherwise, relative filenames will get skin path appended
				sName = MakeFullFilePath( g_MenuInfo.m_sMXMPath, sName );
			}
			sName = pOldSkinXML->GetString( _T("backgrounds"), _T("loading"), sName, true );
			if ( sName.GetLength() )
			{
				sName = MakeFullFilePath( szBasePath, sName );
				pResNode = pSkinNode->AddNode( _T("resource") );
				if ( pResNode )
				{
					pResNode->SetString(NULL, _T("type"), _T("image") );
					pResNode->SetString(NULL, _T("name"), _T("load") );
					pResNode->SetString(NULL, _T("file"), sName );
				}
			}

			// Menu screen
			// Default to MXM.xml setting, if it's there
			sName = pMXMXML->GetString( _T("main"), _T("MenuBack"), _T(""), true );
			if ( sName.GetLength() )
			{
				// Only append full path here if we actually got something here.
				// Otherwise, relative filenames will get skin path appended
				sName = MakeFullFilePath( g_MenuInfo.m_sMXMPath, sName );
			}
			sName = pOldSkinXML->GetString( _T("backgrounds"), _T("menu"), sName, true );
			if ( sName.GetLength() )
			{
				sName = MakeFullFilePath( szBasePath, sName );
				pResNode = pSkinNode->AddNode( _T("resource") );
				if ( pResNode )
				{
					pResNode->SetString(NULL, _T("type"), _T("image") );
					pResNode->SetString(NULL, _T("name"), _T("menu") );
					pResNode->SetString(NULL, _T("file"), sName );
				}
			}


			// Saver Logo
			// Default to MXM.xml setting, if it's there
			sName = pMXMXML->GetString( _T("main"), _T("SaverLogo"), _T(""), true );
			if ( sName.GetLength() )
			{
				// Only append full path here if we actually got something here.
				// Otherwise, relative filenames will get skin path appended
				sName = MakeFullFilePath( g_MenuInfo.m_sMXMPath, sName );
			}
			sName = pOldSkinXML->GetString( _T("backgrounds"), _T("saverlogo"), sName, true );
			if ( sName.GetLength() )
			{
				sName = MakeFullFilePath( szBasePath, sName );
				pResNode = pSkinNode->AddNode( _T("resource") );
				if ( pResNode )
				{
					pResNode->SetString(NULL, _T("type"), _T("image") );
					pResNode->SetString(NULL, _T("name"), _T("logo") );
					pResNode->SetString(NULL, _T("file"), sName );
				}
			}
			else
			{
				pResNode = pSkinNode->AddNode( _T("resource") );
				if ( pResNode )
				{
					pResNode->SetString(NULL, _T("type"), _T("image") );
					pResNode->SetString(NULL, _T("name"), _T("Logo") );
					pResNode->SetString(NULL, _T("File"), _T("::Logo") );
				}
			}
			// OK, image resources are taken care of.

			// Sounds:
			//  up
			//  down
			//  launch
			//  SaverBounce
			//  titleopen
			//  titleclose
//			CXMLNode * pSoundNode;

			sName = pMXMXML->GetString( _T("menu"), _T("sndup"), _T(""), true );
			if ( sName.GetLength() )
			{
				sName = MakeFullFilePath( g_MenuInfo.m_sMXMPath, sName );
			}
			sName = pOldSkinXML->GetString( _T("sounds"), _T("menuup"), sName, true );
			if ( sName.GetLength() )
			{
				sName = MakeFullFilePath( szBasePath, sName );
				pResNode = pSkinNode->AddNode( _T("resource") );
				if ( pResNode )
				{
					pResNode->SetString(NULL, _T("type"), _T("sound") );
					pResNode->SetString(NULL, _T("name"), _T("menuup") );
					pResNode->SetString(NULL, _T("file"), sName );
				}
			}

			sName = pMXMXML->GetString( _T("menu"), _T("snddn"), _T(""), true );
			if ( sName.GetLength() )
			{
				sName = MakeFullFilePath( g_MenuInfo.m_sMXMPath, sName );
			}
			sName = pOldSkinXML->GetString( _T("sounds"), _T("menudown"), sName, true );
			if ( sName.GetLength() )
			{
				sName = MakeFullFilePath( szBasePath, sName );
				pResNode = pSkinNode->AddNode( _T("resource") );
				if ( pResNode )
				{
					pResNode->SetString(NULL, _T("type"), _T("sound") );
					pResNode->SetString(NULL, _T("name"), _T("menudown") );
					pResNode->SetString(NULL, _T("file"), sName );
				}
			}

			sName = pMXMXML->GetString( _T("menu"), _T("sndlaunch"), _T(""), true );
			if ( sName.GetLength() )
			{
				sName = MakeFullFilePath( g_MenuInfo.m_sMXMPath, sName );
			}
			sName = pOldSkinXML->GetString( _T("sounds"), _T("menulaunch"), sName, true );
			if ( sName.GetLength() )
			{
				sName = MakeFullFilePath( szBasePath, sName );
				pResNode = pSkinNode->AddNode( _T("resource") );
				if ( pResNode )
				{
					pResNode->SetString(NULL, _T("type"), _T("sound") );
					pResNode->SetString(NULL, _T("name"), _T("menulaunch") );
					pResNode->SetString(NULL, _T("file"), sName );
				}
			}

			sName = pOldSkinXML->GetString( _T("sounds"), _T("saverbounce"), _T(""), true );
			if ( sName.GetLength() )
			{
				sName = MakeFullFilePath( szBasePath, sName );
				pResNode = pSkinNode->AddNode( _T("resource") );
				if ( pResNode )
				{
					pResNode->SetString(NULL, _T("type"), _T("sound") );
					pResNode->SetString(NULL, _T("name"), _T("saverbounce") );
					pResNode->SetString(NULL, _T("file"), sName );
				}
			}

			sName = pOldSkinXML->GetString( _T("sounds"), _T("titleclose"), _T(""), true );
			if ( sName.GetLength() )
			{
				sName = MakeFullFilePath( szBasePath, sName );
				pResNode = pSkinNode->AddNode( _T("resource") );
				if ( pResNode )
				{
					pResNode->SetString(NULL, _T("type"), _T("sound") );
					pResNode->SetString(NULL, _T("name"), _T("titleclose") );
					pResNode->SetString(NULL, _T("file"), sName );
				}
			}

			sName = pOldSkinXML->GetString( _T("sounds"), _T("titleopen"), _T(""), true );
			if ( sName.GetLength() )
			{
				sName = MakeFullFilePath( szBasePath, sName );
				pResNode = pSkinNode->AddNode( _T("resource") );
				if ( pResNode )
				{
					pResNode->SetString(NULL, _T("type"), _T("sound") );
					pResNode->SetString(NULL, _T("name"), _T("titleopen") );
					pResNode->SetString(NULL, _T("file"), sName );
				}
			}

			// Music
			CXMLNode * pMusicNode;

			sName = pOldSkinXML->GetString( _T("Music"), _T("Dir"), _T(""), true );
			if ( sName )
			{
				pMusicNode = pSkinNode->AddNode( _T("music") );
				if ( pMusicNode )
				{
					pMusicNode->AddElement( _T("directory"), sName );
					pMusicNode->SetBool( NULL, _T("PlayGlobal"), FALSE );
					pMusicNode->SetBool( NULL, _T("PlayRandom"), FALSE );
				}
			}

			// Delays....
			CXMLNode * pDelayNode;
			pDelayNode = pSkinNode->AddNode( _T("delays") );
			if ( pDelayNode )
			{
				long lDelay;

				lDelay = pOldSkinXML->GetLong( _T("defaultdelays"), _T("defaultloadingdelay"), 0 );
				lDelay = pOldSkinXML->GetLong( _T("delays"), _T("loadingdelay"), lDelay );
				lDelay *= 1000;
				pDelayNode->SetLong( NULL, _T("load"), lDelay );

				lDelay = pOldSkinXML->GetLong( _T("defaultdelays"), _T("defaultsaverdelay"), 300 );
				lDelay = pOldSkinXML->GetLong( _T("delays"), _T("saverdelay"), lDelay );
				pDelayNode->SetLong( NULL, _T("saver"), lDelay );

				lDelay = pOldSkinXML->GetLong( _T("defaultdelays"), _T("defaulttitledelay"), 0 );
				lDelay = pOldSkinXML->GetLong( _T("delays"), _T("titledelay"), lDelay );
				lDelay *= 1000;
				pDelayNode->SetLong( NULL, _T("title"), lDelay );
			}

			// Now let's fire up our layouts....
			CXMLNode * pLayoutNode;

			//*******************************************************************
			// Title
			//*******************************************************************
			pLayoutNode = pSkinNode->AddNode( _T("layout") );
			if ( pLayoutNode )
			{
				// Bunch of informational text fields here....
				pLayoutNode->SetString( NULL, _T("screen"), _T("title") );
				CXMLNode * pElementNode;

				// Title image Element
				pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
				if ( pElementNode )
				{
					pElementNode->SetString( NULL, _T("type"), _T("image") );
					pElementNode->SetString( NULL, _T("source"), _T("title") );
					SetPositionInNode( pElementNode, 0, 0, 640, 480 );
					SetScalingInNode( pElementNode, true, true );
				}
				// Need a title box here... (render before title text)

				// OldSkin defined attributes

//		Cfg.Title.m_attrSet.m_dwBackColor = iniFile.GetDWORD( _T("Title"), _T("BackColor"), 0 );
//		Cfg.Title.m_attrSet.m_dwBorderColor = iniFile.GetDWORD( _T("Title"), _T("BorderColor"), 0 );

				// Title Element
				pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
				if ( pElementNode )
				{
					int iTop	=	pOldSkinXML->GetLong( _T("title"), _T("top"), -1 );
					int iLeft	=	pOldSkinXML->GetLong( _T("title"), _T("left"), -1 );
					int iWidth	=	pOldSkinXML->GetLong( _T("title"), _T("width"), -1 );
					int iHeight	=	pOldSkinXML->GetLong( _T("title"), _T("height"), -1 );
					DWORD dwShadowColor, dwTextColor;
					CStdString sJustifyX(_T("left")), sJustifyY(_T("top")); 

					pElementNode->SetString( NULL, _T("type"), _T("text") );
					pElementNode->SetString( NULL, _T("source"), _T("disctitle") );
					if ( iTop == -1 ) 
					{
						iTop = 200;
						sJustifyY = _T("center");
					}
					if ( iLeft == -1 )
					{
						iLeft = 320;
						sJustifyX = _T("center");
					}
					SetPositionInNode( pElementNode, -1, -1, iWidth, iHeight, iLeft, iTop  );
					SetScalingInNode( pElementNode, false, false );
					pElementNode->SetString( _T("justify"), _T("x"), sJustifyX, true );
					pElementNode->SetString( _T("justify"), _T("y"), sJustifyY, true );

					dwTextColor = pMXMXML->GetDWORD( _T("main"), _T("titlecolor"), 0xffffffff );
					dwTextColor = pOldSkinXML->GetDWORD( _T("title"), _T("textcolor"), dwTextColor );
					pElementNode->SetDWORD( NULL, _T("textcolor"), dwTextColor );
					dwShadowColor = pMXMXML->GetBool( _T("main"), _T("shadowtitle"), TRUE )?0xff000000:0;
					dwShadowColor = pOldSkinXML->GetDWORD( _T("title"), _T("shadowcolor"), dwShadowColor );
					pElementNode->SetDWORD( NULL, _T("shadowcolor"), dwShadowColor );
				}

				// SubTitle Element
				pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
				if ( pElementNode )
				{
					int iTop	=	pOldSkinXML->GetLong( _T("subtitle"), _T("top"), -1 );
					int iLeft	=	pOldSkinXML->GetLong( _T("subtitle"), _T("left"), -1 );
					int iWidth	=	pOldSkinXML->GetLong( _T("subtitle"), _T("width"), -1 );
					int iHeight	=	pOldSkinXML->GetLong( _T("subtitle"), _T("height"), -1 );
					DWORD dwShadowColor, dwTextColor;
					CStdString sJustifyX(_T("left")), sJustifyY(_T("top")); 

					pElementNode->SetString( NULL, _T("type"), _T("text") );
					pElementNode->SetString( NULL, _T("source"), _T("discsubtitle") );
					if ( iTop == -1 ) 
					{
						iTop = 260;
						sJustifyY = _T("center");
					}
					if ( iLeft == -1 )
					{
						iLeft = 320;
						sJustifyX = _T("center");
					}
					SetPositionInNode( pElementNode, -1, -1, iWidth, iHeight, iLeft, iTop );
					SetScalingInNode( pElementNode, false, false );
					pElementNode->SetString( _T("justify"), _T("x"), sJustifyX, true );
					pElementNode->SetString( _T("justify"), _T("y"), sJustifyY, true );

					dwTextColor = pMXMXML->GetDWORD( _T("main"), _T("subtitlecolor"), 0xffffffff );
					dwTextColor = pOldSkinXML->GetDWORD( _T("subtitle"), _T("textcolor"), dwTextColor );
					pElementNode->SetDWORD( NULL, _T("textcolor"), dwTextColor );
					dwShadowColor = pMXMXML->GetBool( _T("main"), _T("shadowsubtitle"), TRUE )?0xff000000:0;
					dwShadowColor = pOldSkinXML->GetDWORD( _T("subtitle"), _T("shadowcolor"), dwShadowColor );
					pElementNode->SetDWORD( NULL, _T("shadowcolor"), dwShadowColor );
				}

			}


			//*******************************************************************
			// Load Screen
			//*******************************************************************
			pLayoutNode = pSkinNode->AddNode( _T("layout") );
			if ( pLayoutNode )
			{
				// Bunch of informational text fields here....
				pLayoutNode->SetString( NULL, _T("screen"), _T("load") );
				CXMLNode * pElementNode;

				pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
				if ( pElementNode )
				{
					pElementNode->SetString( NULL, _T("type"), _T("image") );
					pElementNode->SetString( NULL, _T("source"), _T("load") );
					SetPositionInNode( pElementNode, 0, 0, 640, 480 );
					SetScalingInNode( pElementNode, true, true );
				}
			}

			BuildSaverLayout( pSkinNode );

			//*******************************************************************
			// Menu Screen
			//*******************************************************************
			pLayoutNode = pSkinNode->AddNode( _T("layout") );
			if ( pLayoutNode )
			{
				// Bunch of informational text fields here....
				pLayoutNode->SetString( NULL, _T("screen"), _T("menu") );
				CXMLNode * pElementNode;

				//----------------------------------------------
				// Background Image
				//----------------------------------------------
				pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
				if ( pElementNode )
				{
					pElementNode->SetString( NULL, _T("type"), _T("image") );
					pElementNode->SetString( NULL, _T("source"), _T("menu") );
					SetPositionInNode( pElementNode, 0, 0, 640, 480 );
					SetScalingInNode( pElementNode, true, true );
				}

				//----------------------------------------------
				// Screenshot Image
				//----------------------------------------------
				pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
				if ( pElementNode )
				{
					
					int iTop	=	pOldSkinXML->GetLong( _T("screenshot"), _T("top"), 
										pMXMXML->GetLong( _T("screenshot"), _T("top"), -1 ) );
					int iLeft	=	pOldSkinXML->GetLong( _T("screenshot"), _T("left"), 
										pMXMXML->GetLong( _T("screenshot"), _T("left"), -1 ) );
					int iWidth	=	pOldSkinXML->GetLong( _T("screenshot"), _T("width"), 
										pMXMXML->GetLong( _T("screenshot"), _T("width"), -1 ) );
					int iHeight	=	pOldSkinXML->GetLong( _T("screenshot"), _T("height"), 
										pMXMXML->GetLong( _T("screenshot"), _T("height"), -1 ) );

					pElementNode->SetString( NULL, _T("type"), _T("image") );
					pElementNode->SetString( NULL, _T("source"), _T("screenshot") );
					SetPositionInNode( pElementNode, iLeft, iTop, iWidth, iHeight );
					SetScalingInNode( pElementNode, true, true );
					pElementNode->SetString( _T("justify"), _T("x"), _T("center") );
					pElementNode->SetString( _T("justify"), _T("y"), _T("center") );
					pElementNode->SetBool( NULL, _T("proportional"), TRUE );
					pElementNode->SetBool( _T("sound"), _T("enable"), TRUE );
					pElementNode->SetBool( _T("sound"), _T("suspendmusic"), TRUE );
				}

				DWORD dwBorderColor, dwBackColor, dwItemColor, dwShadowColor, dwSelColor, dwSelShadow;
				BOOL bSmoothScroll;

				//----------------------------------------------
				// Menu Block
				//----------------------------------------------
				pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
				if ( pElementNode )
				{
					
					int iTop	=	pOldSkinXML->GetLong( _T("menu"), _T("top"), pMXMXML->GetLong( _T("menu"), _T("top"), -1 ) );
					int iLeft	=	pOldSkinXML->GetLong( _T("menu"), _T("left"), pMXMXML->GetLong( _T("menu"), _T("left"), -1 ) );
					int iWidth	=	pOldSkinXML->GetLong( _T("menu"), _T("width"), pMXMXML->GetLong( _T("menu"), _T("width"), -1 ) );
					int iHeight	=	pOldSkinXML->GetLong( _T("menu"), _T("height"), pMXMXML->GetLong( _T("menu"), _T("height"), -1 ) );
					int iNumItems = pOldSkinXML->GetLong( _T("menu"), _T("numitems"), pMXMXML->GetLong( _T("menu"), _T("items"), 7 ) );
					int iSelPos = pOldSkinXML->GetLong( _T("menu"), _T("selpos"), pMXMXML->GetLong( _T("menu"), _T("selpos"), iNumItems/2 ) );

					pElementNode->SetString( NULL, _T("type"), _T("menu") );
					pElementNode->SetString( NULL, _T("source"), _T("mxm") );

					pElementNode->SetLong( NULL, _T("numitems"), iNumItems );
					pElementNode->SetLong( NULL, _T("selpos"), iSelPos );

					pElementNode->SetString( _T("primary"), _T("source"), _T("title") );
					SetPositionInNode( pElementNode, iLeft, iTop, iWidth, iHeight );
					SetScalingInNode( pElementNode, true, true );

					dwBorderColor = pOldSkinXML->GetDWORD( _T("menu"), _T("bordercolor"), pMXMXML->GetDWORD( _T("menu"), _T("MenuBorderColor"), 0xa04444ff ) );
					dwBackColor = pOldSkinXML->GetDWORD( _T("menu"), _T("BackColor"), pMXMXML->GetDWORD( _T("menu"), _T("MenuBackColor"), 0x80000000 ) );
					dwItemColor = pOldSkinXML->GetDWORD( _T("menu"), _T("TextColor"), pMXMXML->GetDWORD( _T("menu"), _T("ItemColor"), 0xa0808080 ) );
					dwShadowColor = pOldSkinXML->GetBool( _T("menu"), _T("ShadowColor"), pMXMXML->GetDWORD( _T("menu"), _T("ShadowItem"), TRUE ) )?0xFF000000:0;
					dwSelShadow = pOldSkinXML->GetBool( _T("menu"), _T("ShadowSelItem"), pMXMXML->GetDWORD( _T("menu"), _T("ShadowSelItem"), TRUE ) )?0xFF000000:0;
					dwSelColor = pOldSkinXML->GetDWORD( _T("menu"), _T("SelTextColor"), pMXMXML->GetDWORD( _T("menu"), _T("SelItemColor"), 0xffffffff ) );
					pElementNode->SetDWORD( _T("unselitem"), _T("textcolor"), dwItemColor );
					pElementNode->SetDWORD( _T("unselitem"), _T("backcolor"), dwBackColor );
					pElementNode->SetDWORD( _T("unselitem"), _T("bordercolor"), dwBorderColor );
					pElementNode->SetDWORD( _T("unselitem"), _T("shadowcolor"), dwShadowColor );
					pElementNode->SetDWORD( _T("selection"), _T("textcolor"), dwSelColor );
					pElementNode->SetDWORD( _T("selection"), _T("shadowcolor"), dwSelShadow );

					bSmoothScroll = pOldSkinXML->GetBool( _T("options"), _T("SmoothMenu"), pMXMXML->GetDWORD( _T("menu"), _T("SmoothScroll"), TRUE ) );
					
					pElementNode->SetBool( NULL, _T("SmoothScroll"), bSmoothScroll );
					pElementNode->SetLong( NULL, _T("SmoothFector"), 2 );
					pElementNode->SetBool( NULL, _T("DrawSecondary"), FALSE );
					pElementNode->SetBool( NULL, _T("DoGlow"), FALSE );

				}

				//----------------------------------------------
				// Description Block
				//----------------------------------------------
				pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
				if ( pElementNode )
				{
					
					int iTop	=	pOldSkinXML->GetLong( _T("descr"), _T("top"), pMXMXML->GetLong( _T("descr"), _T("top"), -1 ) );
					int iLeft	=	pOldSkinXML->GetLong( _T("descr"), _T("left"), pMXMXML->GetLong( _T("descr"), _T("left"), -1 ) );
					int iWidth	=	pOldSkinXML->GetLong( _T("descr"), _T("width"), pMXMXML->GetLong( _T("descr"), _T("width"), -1 ) );
					int iHeight	=	pOldSkinXML->GetLong( _T("descr"), _T("height"), pMXMXML->GetLong( _T("descr"), _T("height"), -1 ) );

					pElementNode->SetLong( NULL, _T("numitems"), 1 );
					pElementNode->SetLong( NULL, _T("selpos"), 0 );

					pElementNode->SetString( NULL, _T("type"), _T("menu") );
					pElementNode->SetString( NULL, _T("source"), _T("mxm") );
					pElementNode->SetString( _T("primary"), _T("source"), _T("descr") );
					SetPositionInNode( pElementNode, iLeft, iTop, iWidth, iHeight );
					SetScalingInNode( pElementNode, true, true );


					dwBorderColor = pOldSkinXML->GetDWORD( _T("descr"), _T("bordercolor"), pMXMXML->GetDWORD( _T("descr"), _T("MenuBorderColor"), 0xa04444ff ) );
					dwBackColor = pOldSkinXML->GetDWORD( _T("descr"), _T("BackColor"), pMXMXML->GetDWORD( _T("descr"), _T("MenuBackColor"), 0x80000000 ) );
					dwItemColor = pOldSkinXML->GetDWORD( _T("descr"), _T("TextColor"), pMXMXML->GetDWORD( _T("descr"), _T("ItemColor"), 0xa0808080 ) );
					dwShadowColor = pOldSkinXML->GetBool( _T("descr"), _T("ShadowColor"), pMXMXML->GetDWORD( _T("descr"), _T("ShadowItem"), TRUE ) )?0xFF000000:0;
					dwSelShadow = pOldSkinXML->GetBool( _T("descr"), _T("ShadowSelItem"), pMXMXML->GetDWORD( _T("descr"), _T("ShadowSelItem"), TRUE ) )?0xFF000000:0;
					dwSelColor = pOldSkinXML->GetDWORD( _T("descr"), _T("SelTextColor"), pMXMXML->GetDWORD( _T("descr"), _T("SelItemColor"), 0xffffffff ) );
					
					pElementNode->SetDWORD( _T("unselitem"), _T("textcolor"), dwItemColor );
					pElementNode->SetDWORD( _T("unselitem"), _T("backcolor"), dwBackColor );
					pElementNode->SetDWORD( _T("unselitem"), _T("bordercolor"), dwBorderColor );
					pElementNode->SetDWORD( _T("unselitem"), _T("shadowcolor"), dwShadowColor );
					pElementNode->SetDWORD( _T("selection"), _T("textcolor"), dwSelColor );
					pElementNode->SetDWORD( _T("selection"), _T("shadowcolor"), dwSelShadow );

					bSmoothScroll = pOldSkinXML->GetBool( _T("options"), _T("SmoothDescr"), pMXMXML->GetDWORD( _T("descr"), _T("SmoothScroll"), TRUE ) );
					
					pElementNode->SetBool( NULL, _T("SmoothScroll"), bSmoothScroll );
					pElementNode->SetLong( NULL, _T("SmoothFector"), 2 );
					pElementNode->SetBool( NULL, _T("DrawSecondary"), FALSE );
					pElementNode->SetBool( NULL, _T("DoGlow"), FALSE );

				}

#if 0
				// Test Text
				// actualpath
				pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
				if ( pElementNode )
				{
					pElementNode->SetString( NULL, _T("type"), _T("text") );
					// actualpath
					pElementNode->SetString( NULL, _T("source"), _T("cgtitle") );
					SetPositionInNode( pElementNode, -1, -1, 560, 60, 320, 80 );
					SetScalingInNode( pElementNode, true, true );
					pElementNode->SetDWORD( NULL, _T("textcolor"), 0xffffffff );
					pElementNode->SetDWORD( NULL, _T("shadowcolor"), 0xff000000 );
					pElementNode->SetBool( NULL, _T("proportional"), true );

					pElementNode->SetString( _T("justify"), _T("x"), _T("center"), true );
					pElementNode->SetString( _T("justify"), _T("y"), _T("center"), true );
				}
#endif
#if 0
				//----------------------------------------------
				// Test Elements
				//----------------------------------------------
				pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
				if ( pElementNode )
				{
					pElementNode->SetString( NULL, _T("type"), _T("line") );

					pElementNode->SetLong( _T("pos"), _T("x1"), 100 );
					pElementNode->SetLong( _T("pos"), _T("x2"), 200 );
					pElementNode->SetLong( _T("pos"), _T("y1"), 100 );
					pElementNode->SetLong( _T("pos"), _T("y2"), 200 );
					pElementNode->SetString( NULL, _T("condition"), _T("ItemSubMenu") );

					pElementNode->SetLong( NULL, _T("color"), 0xFFFFFFFF );
				}
				pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
				if ( pElementNode )
				{
					pElementNode->SetString( NULL, _T("type"), _T("box") );
					pElementNode->SetString( NULL, _T("condition"), _T("ItemEntry") );
					SetPositionInNode( pElementNode, 200, 150, 50, 50 );
					pElementNode->SetLong( NULL, _T("color"), 0xFFFF0000 );
					pElementNode->SetLong( NULL, _T("bordercolor"), 0x80FFFFFF );
				}
				pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
				if ( pElementNode )
				{
					pElementNode->SetString( NULL, _T("type"), _T("triangle") );
					SetPositionInNode( pElementNode, 400, 150, 50, 50 );
					pElementNode->SetString( NULL, _T("direction"), _T("up") );
					pElementNode->SetString( NULL, _T("condition"), _T("ItemPrev") );
					pElementNode->SetDWORD( NULL, _T("color"), 0xFFFF0000 );
					pElementNode->SetDWORD( NULL, _T("bordercolor"), 0x80FFFFFF );
				}
				pElementNode = pLayoutNode->AddNode( _T("LayoutElement") );
				if ( pElementNode )
				{
					pElementNode->SetString( NULL, _T("type"), _T("triangle") );
					SetPositionInNode( pElementNode, 300, 200, 50, 50 );
					pElementNode->SetString( NULL, _T("direction"), _T("down") );
					pElementNode->SetString( NULL, _T("condition"), _T("ItemNext") );
					pElementNode->SetDWORD( NULL, _T("color"), 0xFF00FF00 );
					pElementNode->SetDWORD( NULL, _T("bordercolor"), 0x80FFFFFF );
				}
#endif
			}

		}
	}
}

static LPCTSTR szDashboards[] = { 
	_T("xboxdash.xbe"),
	_T("yboxdash.xbe"),
	_T("evoxdash.xbe"),
	_T("msxboxdash.xbe"),
	_T("evox.xbe"),
	_T("nexgen.xbe"),
	_T("mxm.xbe"),
	NULL
};

// Look for MXM.xml and executable file on path specified.
bool FindMXM( LPCTSTR szPath )
{
	bool bReturn = false;
	CStdString sFilename;
	int iIndex;
	DWORD dwIdentifier;

	if ( szPath && _tcslen( szPath ) )
	{
		// First, look for dashboards... check each one
		// xboxdash
		// evoxdash
		// msxboxdash
		// yboxdash
		// evox
		// nexgen
		// mxm
		// Detect number....
		for( iIndex=0; szDashboards[iIndex]; iIndex++ )
		{
			sFilename = MakeFullFilePath( szPath, szDashboards[iIndex] );
			if ( FileExists( sFilename ) )
			{
				dwIdentifier = GetXbeID( sFilename );
				if ( dwIdentifier == 0x00004321 )
				{
					bReturn = true;
					// DEBUG_FORMAT( _T("DASHBOARD: FOUND ON %s"), szPath );
				}
			}
		}
	}
	return bReturn;
}



bool IsDrivePresent( char cDrive )
{
	bool bReturn = false;
	HANDLE hFind;
	WIN32_FIND_DATA ffData;
	CStdString sWildcard;

	sWildcard.Format( _T("%c:\\*"), toupper(cDrive) );
	hFind = FindFirstFile( sWildcard, &ffData );
	if( INVALID_HANDLE_VALUE != hFind )
	{
		bReturn = true;
		FindClose(hFind);
	}
	else
	{
		ULARGE_INTEGER uFree1, uTotal1, uTotal2;
		
		sWildcard.Format( _T("%c:\\"), toupper(cDrive) );

		if ( GetDiskFreeSpaceEx( sWildcard, &uFree1, &uTotal1, &uTotal2 ) )
		{
				bReturn = true;
		}
		else
		{
			// Should do a final check....
			char szVolume[256];
			char szFileSys[32];
			DWORD dwVolSer, dwMaxCompLen, dwFileSysFlags;
			if ( GetVolumeInformation( sWildcard, szVolume, 255, &dwVolSer, &dwMaxCompLen, &dwFileSysFlags, szFileSys, 32 ) )
			{
				bReturn = true;
			}
		}
	}
	return bReturn;
}

// Search for 'autos' - MXM_SS.* screenshot files, MXM_GHxx.* GameHelp files
// and anything complex skin defined.
// 
void SearchForAutos( CItemInfo & pItem )
{
	CStdString sDir = pItem.GetValue( szII_Dir );
	CStdString sKey;
	// Find screenshot... First available
	HANDLE hFind;
	WIN32_FIND_DATA ffData;
	CStdString sWildcard;
	CStdString sFileName;
	CStdString sTemp;
	int iIndex = 0;
	bool bFoundFile = false;
	
	// Let's fill out anything that isn't set yet.
	// Do we have valid media?
	if ( pItem.GetValue( szII_Media ).GetLength() == 0 )
	{

		iIndex = 0;
		bFoundFile = false;
		sTemp.Format( _T("Searching directory (%s) for image files"), sDir.c_str() );
		DEBUG_LINE( sTemp );

		sWildcard.Format( _T("%s\\MXM_SS.*"), sDir.c_str() );
		hFind = FindFirstFile( sWildcard.c_str(), &ffData );
		if( INVALID_HANDLE_VALUE != hFind )
		{
			do
			{
				sFileName = ffData.cFileName;
				if ( IsImageFile( sFileName.c_str() ) )
				{
					sTemp.Format( _T("%s\\%s"), sDir.c_str(), ffData.cFileName );
					if ( FileExists( sTemp.c_str() ) )
					{
						// pItem->m_sMedia = sTemp;
						pItem.SetValue( szII_Media, sTemp.c_str() );
						bFoundFile = true;
					}
				}
			} while( !bFoundFile && FindNextFile( hFind, &ffData ) );

			// Close the find handle.
			FindClose( hFind );
		}
	}

	// Last resort... look for save game info and use there, if possible
	if ( pItem.GetValue( szII_Media ).GetLength() == 0 )
	{
	}

	//	sKey.Format( _T("HelpScr_%02d"), iHelpKey++ );

	// Do we have help screens already loaded? If so, don't try to autoload
	if ( pItem.GetValue( _T("HelpScr_01") ).GetLength() == 0 )
	{
		iIndex = 1;
		sTemp.Format( _T("Searching directory (%s) for help files"), sDir.c_str() );
		DEBUG_LINE( sTemp );

		do
		{
			bFoundFile = false;
			sKey.Format( _T("HelpScr_%02d"), iIndex );
			sWildcard.Format( _T("%s\\MXM_GH%02d.*"), sDir.c_str(), iIndex );
			hFind = FindFirstFile( sWildcard.c_str(), &ffData );
			if( INVALID_HANDLE_VALUE != hFind )
			{
				do
				{
					sFileName = ffData.cFileName;
					if ( IsImageFile( sFileName.c_str() ) )
					{
						sTemp.Format( _T("%s\\%s"), sDir.c_str(), ffData.cFileName );
						if ( FileExists( sTemp.c_str() ) )
						{
							// pItem->m_sMedia = sTemp;
							pItem.SetValue( sKey, sTemp );
							bFoundFile = true;
						}
						DEBUG_LINE( sTemp );
					}
				} while( !bFoundFile && FindNextFile( hFind, &ffData ) );

				// Close the find handle.
				FindClose( hFind );
			}
			iIndex++;
		} while( bFoundFile && (iIndex<99));
	}
	// Now we hopefully have media. If not, we need to load it from the file.
	// We'll let the screenshot loading take care of that detail
}

// In each directory (except "Media"), we look for MXM_Entry.ini
// then default.xbe, then any xbes at all.
bool CheckDirectory( LPCTSTR szDirectory, CItemInfo & pItem, bool bKeepD, bool bDefaultOnly )
//bool CheckDirectory( LPCTSTR szDirectory, T_ItemInfo * pItem )
{
	bool bHasGameID = false;
	DWORD dwGameID = 0;
	bool bReturn = false;
	CStdString sTemp;
	CStdString sTempFile;
	CStdString sDir;
	const LPCTSTR szItem = _T("Item");
	CStdString sExe;

	// First look for ini file
	sDir = MakeFullDirPath( _T("D:\\"), szDirectory, TRUE ); // .Format( _T("D:\\%s"), szDirectory );

	DEBUG_FORMAT( _T("CheckDirectory: %s"), sDir.c_str() );

	pItem.Clear();
	pItem.SetValue( szII_Dir, sDir.c_str() );

	sTemp = MakeFullFilePath( sDir, _T("MXM_Entry.xml") );
	// sTemp.Format( _T("%s\\MXM_Entry.xml"), sDir.c_str() );
	if( FileExists( sTemp ) )
	{
		CXBXmlCfgAccess iniItemFile( sTemp );

		bReturn = GetEntry( sDir, sDir, iniItemFile, szItem, pItem );
		bHasGameID = true;
		dwGameID = GetXbeID( sTemp );

	}
	else
	{
		sTemp = MakeFullFilePath( sDir, _T("default.xbe") );
	//	sTemp.Format( _T("%s\\default.xbe"), sDir.c_str() );
		if( FileExists( sTemp ) == FALSE )
		{
			pItem.SetValue( szII_Exe, c_szEmpty );
		}
		else
		{
			dwGameID = GetXbeID( sTemp );
			if ( ( dwGameID != 0x00004321 ) &&
				 ( dwGameID != 0x4d415800 ) )
			{
				bHasGameID = true;
				pItem.SetValue( szII_Exe, sTemp.c_str() );
			}
		}
	}

	// Did we find something yet?
	if ( pItem.GetValue( szII_Exe ).GetLength() == 0 && !bDefaultOnly )
	{
		// Search for executable path... first available!!
		HANDLE hFind;
		int iIndex = 0;
		WIN32_FIND_DATA ffData;
		CStdString sWildcard;
		bool bFoundFile = false;
		//CStdString sDir;

//		sDir.Format( _T("D:\\%s"), szDirectory );

		sWildcard = MakeFullFilePath( sDir, _T("*.xbe") ); // .Format( _T("D:\\%s"), szDirectory );
		// sWildcard.Format( _T("%s\\*.xbe"), sDir.c_str() );
		hFind = FindFirstFile( sWildcard.c_str(), &ffData );
		if( INVALID_HANDLE_VALUE != hFind )
		{
			do
			{
				sTemp = MakeFullFilePath( sDir, ffData.cFileName );
				// sTemp.Format( _T("%s\\%s"), sDir.c_str(), ffData.cFileName );
				if ( FileExists( sTemp.c_str() ) )
				{
					dwGameID = GetXbeID( sTemp );
					// Don't load MXM as an item in autoconfig...
					if ( ( dwGameID != 0x00004321 ) &&
						 ( dwGameID != 0x4d415800 ) )
					{
						bHasGameID = true;
						pItem.SetValue( szII_Exe, sTemp.c_str() );
						pItem.SetValue( szII_Dir, sDir );
						bFoundFile = true;
					}
				}
			} while( !bFoundFile && FindNextFile( hFind, &ffData ) );

			// Close the find handle.
			FindClose( hFind );
		}

	}

	if ( bHasGameID )
	{
		DEBUG_FORMAT( _T("Found Xbe Game ID: 0x%08x"), dwGameID );
	}

	// OK, we're golden if we have a valid string here at this point...
	if ( pItem.GetValue( szII_Exe ).GetLength() )
	{

		SearchForAutos( pItem );

		// Let's fill in other blanks...
		sExe = pItem.GetValue( szII_Exe );
		if ( pItem.GetValue( szII_Title ).GetLength() == 0 )
		{
			if ( g_MenuInfo.m_bAllowAutoTitle && bHasGameID && dwGameID )
			{
				LPCTSTR szTitle = GetGameTitleFromID( dwGameID );
				if ( szTitle )
				{
					pItem.SetValue( szII_Title, szTitle );
				}
			}
			if ( pItem.GetValue( szII_Title ).GetLength() == 0 )
			{
				// Extract the title from the certificate information.
				pItem.SetValue( szII_Title, GetXbeTitle( sExe.c_str() ).c_str() );
			}
		}
		if ( pItem.GetValue( szII_Descr ).GetLength() == 0 )
		{
			if ( g_MenuInfo.m_bAllowAutoDescr && bHasGameID )
			{
				LPCTSTR szDescr = GetGameDescriptionFromID( dwGameID, true );
				if ( szDescr )
				{
					DEBUG_FORMAT( _T("Found AutoDescr: %s"), szDescr );
					pItem.SetValue( szII_Descr, szDescr );
				}
				else
				{
					DEBUG_LINE( _T("Did not find AutoDescr") );
				}
			}
		}

//		sExe.Replace( _T("\\\\"), _T("\\") );

		if ( ( g_MenuInfo.m_bDVDMode == false ) && (g_MenuInfo.m_bDashMode == false) && (bKeepD == false) )
		{
			sExe.Replace( _T("D:\\"), _T("") );
		}
		sExe = MakeFullFilePath( g_MenuInfo.m_sMXMPath.c_str(), sExe.c_str() );

		pItem.SetValue( szII_Exe, sExe.c_str() );
		
		bReturn = true;
	}
	return bReturn;
}


bool GetEntry( LPCTSTR szDir, LPCTSTR szExeDir, CXBXmlCfgAccess & iniItemFile, LPCTSTR szSection, CItemInfo & pItem )
{
	bool bReturn = false;
	CStdString sDir = szDir;
	CStdString sExeDir = szExeDir;
	CStdString sTemp, sTempFile;

	int iNumKeys, iIndex;
	CStdString sKey, sValue;



	pItem.Clear();

	// Set the relative path for the Exe file, determined before entering
	// This will be a path from the MXM.xml, or from an autoconfig search
	// 
	pItem.SetValue( szII_Dir, sDir.c_str() );

	sTemp.Format( _T("GetEntry: %s <%s>"), sDir.c_str(), szSection );
	DEBUG_LINE( sTemp.c_str() );


	pItem.SetValue( szII_Title, iniItemFile.GetString( szSection, szII_Title, c_szEmpty ).c_str() );
	sTemp = iniItemFile.GetString( szSection, _T("Descr"), c_szEmpty );
	pItem.SetValue( szII_Descr, iniItemFile.GetString( szSection, _T("Description"), sTemp ) );
//	pItem.SetValue( szII_Descr, iniItemFile.GetString( szSection, _T("Description"), c_szEmpty ).c_str() );

	sTemp = iniItemFile.GetString( szSection, _T("videomode"), c_szEmpty );
	if ( _tcsicmp( sTemp, _T("ntsc") ) == 0 )
	{
		pItem.m_iVideoMode = 2;
		// pItemInfo->m_bNTSCMode = true; 
	}
	else if ( _tcsicmp( sTemp, _T("pal") ) == 0 )
	{
		pItem.m_iVideoMode = 4;
		// pItemInfo->m_bNTSCMode = false; 
	}
	else if ( _tcsicmp( sTemp, _T("pal60") ) == 0 )
	{
		pItem.m_iVideoMode = 5;
		// pItemInfo->m_bNTSCMode = false; 
	}
	else if ( _tcsicmp( sTemp, _T("ntscm") ) == 0 )
	{
		pItem.m_iVideoMode = 2;
		// pItemInfo->m_bNTSCMode = false; 
	}
	else if ( _tcsicmp( sTemp, _T("ntscj") ) == 0 )
	{
		pItem.m_iVideoMode = 3;
		// pItemInfo->m_iVideoMode = -1;
		// pItemInfo->m_bNTSCMode = false; 
	}
	else if ( _tcsicmp( sTemp, _T("pal50") ) == 0 )
	{
		pItem.m_iVideoMode = 4;
		// pItemInfo->m_bNTSCMode = false; 
	}
	else if ( _tcsicmp( sTemp, _T("default") ) == 0 )
	{
		// No change
		pItem.m_iVideoMode = 0;
		// pItemInfo->m_bNTSCMode = IsNTSC(); 
	}
	else if ( _tcsicmp( sTemp, _T("auto") ) == 0 )
	{
		// Based on game....
		pItem.m_iVideoMode = 1;
		// pItemInfo->m_bNTSCMode = false; 
	}
	else
	{
		pItem.m_iVideoMode = 0;
		// pItemInfo->m_bNTSCMode = IsNTSC();
	}
	
	sTemp = iniItemFile.GetString( szSection, szII_PasscodeMD5, c_szEmpty );
	if ( sTemp.GetLength() )
	{
		sTemp.MakeUpper();
		pItem.m_sPassMD5 = sTemp;
	}

	// Handle passcode stuff...
	sTemp = iniItemFile.GetString( szSection, szII_EncPasscode, c_szEmpty );

	if ( sTemp.GetLength() )
	{
		sTemp.MakeUpper();
		for( iIndex=0; iIndex<sTemp.GetLength(); iIndex++ )
		{
			if ( iswalpha(sTemp[iIndex]) )
			{
				sTemp[iIndex] = L'Z'-sTemp[iIndex];
			}
			else if ( iswdigit(sTemp[iIndex]) )
			{
				sTemp[iIndex] = L'9'-sTemp[iIndex];
			}
			else
			{
				sTemp[iIndex] = L'A';
			}
		}
		pItem.m_sPasscode = sTemp;
	}
	else
	{
		
		sTemp = iniItemFile.GetString( szSection, szII_Passcode, c_szEmpty );
		sTemp.MakeUpper();
		for( iIndex=0; iIndex<sTemp.GetLength(); iIndex++ )
		{
			if ( !iswalnum(sTemp[iIndex]) )
			{
				sTemp[iIndex] = L'A';
			}
		}
		pItem.m_sPasscode = sTemp;
	}

	if ( pItem.m_sPasscode.GetLength() > 5 )
	{
		pItem.m_sPasscode = pItem.m_sPasscode.Left(5);
	}

	pItem.TryPasscode(c_szEmpty);

	sTemp = MakeFullFilePath( sDir, iniItemFile.GetString( szSection, szII_Media, c_szEmpty ));
	pItem.SetValue( szII_Media, sTemp );
	if( IsImageFile( sTemp, true ) )
	{
		g_FileManager.AddFileEntry( sTemp );
	}
	
	
	if ( FileExists( pItem.GetValue(szII_Media).c_str() ) == FALSE )
	{
		pItem.SetValue( szII_Media, c_szEmpty );
	}
	
	sTemp = iniItemFile.GetString( szSection, szII_Exe, _T("default.xbe") );
	
	// sTempFile.Format( _T("%s\\%s"), pItem->m_sDir.c_str(), sTemp.c_str() );
	sTempFile.Format( _T("%s\\%s"), szExeDir, sTemp.c_str() );
	pItem.SetValue( szII_Exe, sTempFile.c_str() );
	sTemp.Format(_T("Trying file: %s"), sTempFile.c_str() );
	DEBUG_LINE( sTemp.c_str() );
	if ( FileExists( sTempFile ) == FALSE )
	{
		DEBUG_LINE( _T("FileExists Failed") );
		bReturn = true;
	}
	else
	{
		pItem.SetValue( szII_XbeCurPath, sTempFile );
		bReturn = true;
	}

	iNumKeys = iniItemFile.GetNumberKeys( szSection );
	sKey.Format( _T("Dumping %d keys from section %s"), iNumKeys, szSection );
	DEBUG_LINE( sKey );
	for( iIndex=0; iIndex<iNumKeys; iIndex++ )
	{
		sKey = iniItemFile.GetKeyName( szSection, iIndex );
		sValue = iniItemFile.GetString( szSection, sKey.c_str(), c_szEmpty );
		if ( pItem.GetValue( sKey ).GetLength() == 0 )
		{
			pItem.SetValue( sKey , sValue );
		}
		sTemp.Format( _T("Key#%d: (%s)=(%s)"), iIndex, sKey.c_str(), sValue.c_str() );
		DEBUG_LINE( sTemp );
	}

	int iHelpKey = 1;
	int iNumScreens = 0;
	do
	{
		sKey.Format( _T("HelpScr_%02d"), iHelpKey++ );
		sValue = pItem.GetValue(sKey);
		DEBUG_LINE(_T("Checking Help Screen"));
		DEBUG_LINE( sKey );
		if ( sValue.GetLength() )
		{
			sValue = MakeFullFilePath( sDir.c_str(), sValue );
			if ( !FileExists( sValue ) )
			{
				DEBUG_LINE(_T("Help Screen Does Not exist!"));
				DEBUG_LINE( sValue );
				sValue = c_szEmpty;
			}
			else
			{
				DEBUG_LINE(_T("Help Screen exists!"));
				DEBUG_LINE( sValue );
			}
			pItem.SetValue( sKey, sValue );
		}
		else
		{
			sValue = c_szEmpty;
		}
	} while( sValue.GetLength() && (iHelpKey<99) );


	return bReturn;
}


LPCTSTR GetGameTitleFromID( DWORD dwID )
{
	LPCTSTR szReturn = NULL;
	int iIndex;

	for( iIndex=0; g_Presets[iIndex].m_szAppTitle; iIndex++ )
	{
		if ( g_Presets[iIndex].m_dwAppCode == dwID )
		{
			szReturn = g_Presets[iIndex].m_szAppTitle;
			break;
		}
	}
	return szReturn;
}

LPCTSTR GetGameDescriptionFromID( DWORD dwID, bool bSearchPublisher )
{
	LPCTSTR szReturn = NULL;
	int iIndex;

	for( iIndex=0; g_Presets[iIndex].m_szAppTitle; iIndex++ )
	{
		if ( g_Presets[iIndex].m_dwAppCode == dwID )
		{
			szReturn = g_Presets[iIndex].m_szDescr;
			break;
		}
	}
	if ( szReturn == NULL && bSearchPublisher )
	{
		dwID &= 0xffff0000;
		for( iIndex=0; g_Publishers[iIndex].m_szPublisher; iIndex++ )
		{
			if ( g_Publishers[iIndex].m_dwAppCode == dwID )
			{
				szReturn = g_Publishers[iIndex].m_szPublisher;
				break;
			}
		}
	}
	return szReturn;
}

// Locates all MXM_Skins.xml files in subdirectories of the szDirectory
void FindSkins ( list<CStdString> & saLocations, LPCTSTR szDirectory )
{
	CStdString sWildCard;
	HANDLE hFind;
	WIN32_FIND_DATA ffData;
	CStdString sFilename;
	CStdString sNewPath;

	if ( szDirectory && _tcslen(szDirectory) )
	{
		sWildCard = MakeFullFilePath( szDirectory, _T("*") );
		hFind = FindFirstFile( sWildCard, &ffData );
			
		if( INVALID_HANDLE_VALUE != hFind )
		{
			do
			{
				if ( ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
				{
					sNewPath = MakeFullFilePath( szDirectory, ffData.cFileName );
					sFilename = MakeFullFilePath( sNewPath, _T("MXM_Skin.xml") );
					if ( FileExists( sFilename ) )
					{
						saLocations.push_back( sFilename );
					}
				}
			} while( FindNextFile( hFind, &ffData ) );
			// Close the find handle.
			FindClose( hFind );
		}
	}
}

CStdString GetXbeFile( LPCTSTR szDir, int iIndex )
{
	CStdString sWildCard;
	HANDLE hFind;
	WIN32_FIND_DATA ffData;
	CStdString sFilename;
	CStdString sTemp;

	if ( szDir && _tcslen(szDir) )
	{
		sWildCard = MakeFullFilePath( szDir, _T("*.xbe") );
		hFind = FindFirstFile( sWildCard, &ffData );
			
		if( INVALID_HANDLE_VALUE != hFind )
		{
			do
			{
				if ( (ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==0 )
				{
					sFilename = MakeFullFilePath( szDir, ffData.cFileName );
					// Filter out this thing...
					sTemp = sFilename;
					sTemp.MakeLower();
					if ( sTemp.Find( _T("dashupdate.xbe") ) != -1 )
					{
						sFilename = _T("");
					}
					else if ( sTemp.Find( _T("reboot.xbe") ) != -1 )
					{
						sFilename = _T("");
					}
					if ( sFilename.GetLength() )
					{
						if ( iIndex>0 )
						{
							iIndex--;
							sFilename = _T("");
						}
					}
				}
			} while( (sFilename.GetLength()==0) && FindNextFile( hFind, &ffData ) );
			// Close the find handle.
			FindClose( hFind );
		}				
	}
	return sFilename;
}


FLOAT GetTexturePos( DWORD dwTextureMax, DWORD dwTexturePixPos )
{
	FLOAT fReturn = 0.0;

	if ( dwTextureMax )
	{
		fReturn = (FLOAT)dwTexturePixPos;
		fReturn /= (FLOAT)dwTextureMax;
	}

	return fReturn;
}

D3DFORMAT GetTextureFormat( LPDIRECT3DTEXTURE8 pTexture )
{
	D3DFORMAT fmtReturn = D3DFMT_UNKNOWN;

	if ( pTexture )
	{
		D3DSURFACE_DESC lvlDesc;

		pTexture->GetLevelDesc( 0, &lvlDesc );
		fmtReturn = lvlDesc.Format;
	}
	return fmtReturn;
}

DWORD MakeColor( int iAlpha, int iRed, int iGreen, int iBlue )
{
	DWORD dwResult = (iAlpha&0x0ff);

	dwResult <<= 8;
	dwResult |= (iRed&0x0ff);
	dwResult <<= 8;
	dwResult |= (iGreen&0x0ff);
	dwResult <<= 8;
	dwResult |= (iBlue&0x0ff);
	return dwResult;
}

DWORD GetPeriodValue(DWORD dwTimeStamp, DWORD dwPeriod )
{
	DWORD dwReturn;
	float fReturn = 0.0;

	if ( dwPeriod )
	{
		dwTimeStamp %= dwPeriod;

		fReturn = (float)dwTimeStamp*2.0f*3.14159265359f;
		fReturn /= (float)dwPeriod;
	}
	fReturn = cosf(fReturn);
	// We now have a value between -1.0 and 1.0
	fReturn *= 42.0;
	fReturn += 213.5;
	dwReturn = (DWORD)fReturn;
	if ( dwReturn>255 )
	{
		dwReturn = 255;
	}
	dwReturn <<= 24;
	return dwReturn;
}

CStdString FindFile( LPCTSTR szDirectory, LPCTSTR szBaseName, bool bRecurse  )
{
	CStdString sReturn;
	CStdString sAttempt;

	if ( szDirectory && _tcslen(szDirectory) && szBaseName && _tcslen( szBaseName ) )
	{
		sAttempt = MakeFullFilePath( szDirectory, szBaseName );
		if ( FileExists( sAttempt ) )
		{
			sReturn = sAttempt;
		}
		else
		{
			if ( bRecurse )
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

							if (	sWildcard.Compare( _T(".") ) &&
									sWildcard.Compare( _T("..") ) )
							{
								sWildcard = MakeFullDirPath( szDirectory, ffData.cFileName, FALSE );
								sReturn = FindFile( sWildcard, szBaseName, true );
							}
						}
					} while( (sReturn.GetLength()==0) && FindNextFile( hFind, &ffData ) );
					FindClose( hFind );
				}
			}
		}
	}
	return sReturn;
}

//  Searches for MXM_SS.xxx files. WMV first, followed by JPG, BMP, TGA, PNG, and GIF
CStdString FindMedia( LPCTSTR szDirectory, bool bSS )
{
	CStdString sReturn;
	CStdString sAttempt;
	CStdString sBase;
	CStdString sBaseFile;

	if( bSS )
	{
		sBase = _T("MXM_SS");
	}
	else
	{
		sBase = _T("MXM_Thumb");
	}

	if ( szDirectory && _tcslen( szDirectory ) )
	{
		sBaseFile.Format( _T("%s.wmv"), sBase.c_str() );
		sAttempt = MakeFullFilePath( szDirectory, sBaseFile );
		if ( FileExists( sAttempt ) )
		{
			sReturn = sAttempt;
		}
		else
		{
			sBaseFile.Format( _T("%s.jpg"), sBase.c_str() );
			// sBaseFile = sBase + _T(".jpg");
			sAttempt = MakeFullFilePath( szDirectory, sBaseFile );
			if ( FileExists( sAttempt ) )
			{
				sReturn = sAttempt;
			}
			else
			{
				sBaseFile.Format( _T("%s.dds"), sBase.c_str() );
				// sBaseFile = sBase + _T(".dds");
				sAttempt = MakeFullFilePath( szDirectory, sBaseFile );
				if ( FileExists( sAttempt ) )
				{
					sReturn = sAttempt;
				}
				else
				{
					sBaseFile.Format( _T("%s.bmp"), sBase.c_str() );
					// sBaseFile = sBase + _T(".bmp");
					sAttempt = MakeFullFilePath( szDirectory, sBaseFile );
					if ( FileExists( sAttempt ) )
					{
						sReturn = sAttempt;
					}
					else
					{
						sBaseFile.Format( _T("%s.tga"), sBase.c_str() );
						// sBaseFile = sBase + _T(".tga");
						sAttempt = MakeFullFilePath( szDirectory, sBaseFile );
						if ( FileExists( sAttempt ) )
						{
							sReturn = sAttempt;
						}
						else
						{
							sBaseFile.Format( _T("%s.tga"), sBase.c_str() );
							// sBaseFile = sBase + _T(".tga");
							sAttempt = MakeFullFilePath( szDirectory, sBaseFile );
							if ( FileExists( sAttempt ) )
							{
								sReturn = sAttempt;
							}
							else
							{
								sBaseFile.Format( _T("%s.gif"), sBase.c_str() );
								// sBaseFile = sBase + _T(".gif");
								sAttempt = MakeFullFilePath( szDirectory, sBaseFile );
								if ( FileExists( sAttempt ) )
								{
									sReturn = sAttempt;
								}
								else
								{
									sBaseFile.Format( _T("%s.xbx"), sBase.c_str() );
									// sBaseFile = sBase + _T(".xbx");
									sAttempt = MakeFullFilePath( szDirectory, sBaseFile );
									if ( FileExists( sAttempt ) )
									{
										sReturn = sAttempt;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return sReturn;
}

bool HasString( TListStrings & slStrings, LPCTSTR szValue, bool bIgnoreCase )
{
	bool bReturn = false;
	TListStrings::iterator iterStrings;

	if ( slStrings.size() )
	{
		iterStrings = slStrings.begin();
		while( !bReturn && iterStrings != slStrings.end() )
		{
			if ( bIgnoreCase )
			{
				if ( _tcsicmp( szValue, iterStrings->c_str() ) == 0 )
				{
					bReturn = true;
				}
			}
			else
			{
				if ( _tcscmp( szValue, iterStrings->c_str() ) == 0 )
				{
					bReturn = true;
				}
			}
			iterStrings++;
		}
	}
	return bReturn;
}

void CleanInvalidChars(CXBFont * pFnt, CStdString & sString )
{
	WCHAR * pwszString;

	pwszString = new WCHAR[sString.GetLength()+4];
	if ( pwszString )
	{
		CStdStringW sTempString(sString);
		wcscpy( pwszString, sTempString );
		if ( pFnt )
		{
			pFnt->ReplaceInvalidChars(pwszString, L' ' );
			sString = pwszString;
		}
	}
}
