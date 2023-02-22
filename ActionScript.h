
// ActionScript.h
#pragma once

#include "StdString.h"
#include "MXM_Defs.h"
#include "xmltree.h"
#include "ASXML.h"
#include "ActionDraw.h"

#define ASRC_OK			0
#define ASRC_NOTIMPL	1
#define ASRC_EXECERROR	2

// TMapStrings		g_msEnvironment;
// TMapStrings		g_msLocalEnvironment;
class CXMLObject
{
public:
	CXMLObject(void) : m_pMainNode(NULL), m_pCurrentNode(NULL), m_pCurrentElement(NULL), m_bAttached(false)  {};
	CXMLObject(const CXMLObject &src ) 
	{ 
		m_bAttached = src.m_bAttached;
		m_pMainNode = src.m_pMainNode;
		m_pCurrentNode = src.m_pCurrentNode;
		m_pCurrentElement = src.m_pCurrentElement;
		m_sFilename = src.m_sFilename;
	};
	CXMLObject( CXMLNode * pMainNode, LPCTSTR szFilename ) : 
		m_pMainNode(NULL), m_pCurrentNode(NULL), m_pCurrentElement(NULL)
	{
		m_bAttached = false;
		m_sFilename = szFilename ;
	};
	bool m_bAttached;
	CStdString m_sFilename;
	CXMLNode * m_pMainNode;
	CXMLNode * m_pCurrentNode;
	CXMLNode * m_pCurrentElement;
};


typedef map<CStdString, int>		TMapLabels;
typedef map<string, HANDLE>			TMapHandles;
typedef map<string, CXMLObject>		TMapXMLObjects;


// Just to handle the file handle mapped to string
//
class CMXMFileData
{
public:
	CMXMFileData() : m_hFileHandle(NULL), m_bRead(true) {};
	CMXMFileData( const CMXMFileData &src )
	{ m_hFileHandle = src.m_hFileHandle; m_bRead = src.m_bRead; };
	void Close( void )
	{ 
		if ( m_hFileHandle )
		{
			CloseHandle( m_hFileHandle );
			m_hFileHandle = NULL;
		}
	}
	HANDLE	m_hFileHandle;
	bool	m_bRead;
};

typedef map<string, CMXMFileData>	TMapFileData;

// File Handles
// XML Handles
// ActionDrawList
// Current Search
class CActionScript
{
protected:
	// Only local environment copied into subscripts
	TMapStrings		m_msLocalEnvironment;
	TMapLabels		m_mlLabels;
	TMapFileData	m_mfFileData;
	TXMLMap			m_mxXMLData;
	TListStrings	m_lsProgram;
	TListStrings::iterator m_iterProgLine;
	CStdString		m_sLastError;
	CStdString		m_sCurrLine;
	CStdString		m_sCurrCmd;
	CStdString		m_sCurDir;
	CActionScript * m_pParent;
	int 			m_iCurrLine;
	bool			m_bError;
	TActionDrawList* m_pActionDrawList;
	bool			m_bDeleteDrawList;
	bool			m_bBeginDraw;
	CStdString		m_sWildcard;
	CStdString		m_sSearchPath;
	HANDLE			m_hFind;
	WIN32_FIND_DATA m_ffData;
	bool			m_bIsContext;
	
	CStdString		GetScript( LPCTSTR szScriptName, bool bContext );
	virtual CStdString	HandleFunction( LPCTSTR szFunc, int iFirstArg, int * piLastArg );
	void			EvaluateIf( bool bNumeric, bool bFunction );
	void			SetError( LPCTSTR szMessage );
	CStdString		GetArgument( int iArg, bool bTranslate = true );
	bool			GoToLabel( LPCTSTR szLabel );
	bool			GoToLine( int iLine );
	virtual int		ExecuteLine( LPCTSTR szCommand );
	bool			ScanProgram( LPCTSTR szAction );
	CStdString		TranslateVar( LPCTSTR szArg );
	virtual	void	ContextInit( void ) {};
	virtual	void	ContextExit( void ) {};
	virtual CActionScript * CreateActionScript( LPCTSTR szScript );
public:
	CStdString		GetEnvValue( LPCTSTR szVarName );
	bool			SetEnvValue( LPCTSTR szVarName, LPCTSTR szValue );

	CActionScript( LPCTSTR szScript, CActionScript * pParent = NULL, bool bIsContext = false );
	~CActionScript();
	
	bool			ExecuteScript( void );
};


CStdString TranslateVar( LPCTSTR szArg, TMapStrings * pLocal );
CStdString GetEnvValue( LPCTSTR szVar,  TMapStrings * pLocal );
bool SetActionVar( LPCTSTR szVarName, LPCTSTR szValue, TMapStrings * pMapLocal );

CMXMFileData * GetFileData( TMapFileData & fileData, LPCTSTR szName );
CMXMFileData * CreateFileData( TMapFileData & fileData, LPCTSTR szName );
bool DeleteFileData( TMapFileData & fileData, LPCTSTR szName );
void CloseFileData( TMapFileData & fileData );
//OPEN <WRITE|READ> <HandleName> <FileName>
//WRITELINE <HandleName> <Text>
//READLINE <HandleName> <VarName>
//STATUS <HandleName> <VarName>
//CLOSE <HandleName>

