

#include "StdAfx.h"
#include "ActionScript.h"
#include "MenuInfo.h"
#include "XBStateApp.h"

TMapStrings	g_msGlobalEnvironment;

extern CStdString g_sFormatResult;
int GetGadgetStringID( LPCTSTR szToken );
bool GetGadgetString( int iID, CStdString & sValue, LPCTSTR szSection = NULL, LPCTSTR szKey  = NULL );
bool DoesScriptExist( LPCTSTR szName, bool bContext = false );
CStdString GetInternalScript( LPCTSTR szName, bool bContext = false );
void ReadInputToEnv( XBGAMEPAD* pGamePad );
bool HasInput(XBGAMEPAD* pGamePad );
void SyncInput( void );

bool SetActionVar( LPCTSTR szVarName, LPCTSTR szValue, TMapStrings * pMapLocal )
{
	bool bReturn = false;
	bool bLocal = true;
	CStdString sVarName;

	if ( szVarName && _tcslen( szVarName ) )
	{
		if ( szVarName[0] == _T('_') )
		{
			bLocal = false;
			szVarName++;
		}
		if ( _tcslen( szVarName ) )
		{
			int iCheck = 0;

			while( szVarName[iCheck] && ( _istalnum( szVarName[iCheck] )|| ( _T('_') == szVarName[iCheck]) ) )
			{
				iCheck++;
			}
			if ( szVarName[iCheck] == 0 ) // Passed check
			{
				sVarName = szVarName;
				sVarName.MakeLower();
				if ( szValue && _tcslen( szValue ) )
				{
					if ( bLocal )
					{
						if ( pMapLocal )
						{
							(*pMapLocal)[sVarName] = szValue;
						}
					}
					else
					{
						g_msGlobalEnvironment[sVarName] = szValue;
					}
				}
				else
				{
					// Reset variable - remove from list!
					if ( bLocal )
					{
						if ( pMapLocal )
						{
							(*pMapLocal)[sVarName] = _T("");
						}
					}
					else
					{
						g_msGlobalEnvironment[sVarName] = _T("");
					}
				}
				bReturn = true;
			}
		}
	}
	
	return bReturn;
}

CStdString GetEnvValue( LPCTSTR szVar,  TMapStrings * pLocal )
{
	CStdString sReturn;
	CStdString sVar(szVar);
	bool bGlobal = false;
	
	while ( sVar.GetLength() && sVar[0] == _T('_') )
	{
		bGlobal = true;
		sVar = sVar.Mid(1);
	}
	
	if ( sVar.GetLength() )
	{	
		sVar.MakeLower();
		if ( bGlobal )
		{
			if ( g_msGlobalEnvironment.find( sVar ) != g_msGlobalEnvironment.end() )
			{
				sReturn = g_msGlobalEnvironment[sVar];
			}
		}
		else
		{
			if ( pLocal )
			{
				if ( pLocal->find( sVar ) != pLocal->end() ) 
				{
					sReturn = (*pLocal)[sVar];
				}
			}
		}
	}
	return sReturn;
}


CStdString TranslateVar( LPCTSTR szArg, TMapStrings * pLocal )
{
	CStdString sReturn;

	if ( szArg && _tcslen( szArg ) )
	{
		int iPos = 0, iStart = 0;
		while( szArg[iPos] )
		{
			if ( szArg[iPos] == _T('%') )
			{
				if ( szArg[iPos+1] == _T('%') )
				{
					if ( iStart < iPos )
					{
						sReturn += _T('%');
					}
					iPos += 2;
					iStart = iPos;
				}
				else if ( szArg[iPos+1] == 0 )
				{
					// Just finish...
					iPos++;
				}
				else
				{
					// Add previous portion to return string
					if ( iStart < iPos )
					{
						sReturn += CStdString( &szArg[iStart], (iPos-iStart) );
					}
					iStart = iPos+1;
					iPos++;
					// Now read until next '%'
					while ( szArg[iPos] && szArg[iPos] != _T('%') )
					{
						iPos++;
					}
					if ( szArg[iPos] == _T('%') )
					{
						CStdString sVarName( &szArg[iStart], (iPos-iStart) );
						CStdString sValue;
						
						sValue = GetEnvValue( sVarName, pLocal );
#if 0						
						sVarName.MakeLower();
						if ( sVarName[0] == _T('_') )
						{
							sVarName = sVarName.Mid(1);
							if ( g_msGlobalEnvironment.size() && g_msGlobalEnvironment.find( sVarName ) != g_msGlobalEnvironment.end() )
							{
								sValue = g_msGlobalEnvironment[sVarName];
							}
						}
						else
						{
							if ( pLocal )
							{
								if ( pLocal->size() && pLocal->find( sVarName ) != pLocal->end() )
								{
									sValue = (*pLocal)[sVarName];
								}
							}
							else
							{
								sValue = _T("");
							}
						}
#endif						
						sReturn += sValue;
						iPos++;
						iStart = iPos;
					}
				}
			}
			else if ( szArg[iPos] == _T('$') )
			{
				if ( szArg[iPos+1] == _T('$') )
				{
					if ( iStart < iPos )
					{
						sReturn += _T('$');
					}
					iPos += 2;
					iStart = iPos;
				}
				else if ( szArg[iPos+1] == 0 )
				{
					// Just finish...
					iPos++;
				}
				else
				{
					// Add previous portion to return string
					if ( iStart < iPos )
					{
						sReturn += CStdString( &szArg[iStart], (iPos-iStart) );
					}
					iStart = iPos+1;
					iPos++;
					// Now read until next '$'
					while ( szArg[iPos] && szArg[iPos] != _T('$') )
					{
						iPos++;
					}
					if ( szArg[iPos] == _T('$') )
					{
						CStdString sVarName( &szArg[iStart], (iPos-iStart) );
						CStdString sValue;
						sVarName.MakeLower();

						if ( _tcscmp( sVarName, _T("eol") ) == 0 )
						{
							sValue = _T("\r\n");
						}
						if ( _tcscmp( sVarName, _T("formatresult") ) == 0 )
						{
							sValue = g_sFormatResult;
						}						
						if ( _tcscmp( sVarName, _T("timer") ) == 0 )
						{
							sValue.Format( _T("%u"), GetTickCount() );
						}						
						if ( _tcscmp( sVarName, _T("lasterror") ) == 0 )
						{
							sValue.Format( _T("ERROR CODE: 0x%08X"), GetLastError() );
						}
						if ( _tcscmp( sVarName, _T("dvd_mode") ) == 0 )
						{
							if ( g_MenuInfo.m_bDVDMode )
							{
								sValue = _T("1");
							}
							else
							{
								sValue = _T("0");
							}
						}
						else if ( _tcscmp( sVarName, _T("gt") ) == 0 )
						{
							sValue = _T(">");
						}
						else if ( _tcscmp( sVarName, _T("lt") ) == 0 )
						{
							sValue = _T("<");
						}
						else if ( _tcscmp( sVarName, _T("tab") ) == 0 )
						{
							sValue = _T("\x08");
						}
						else if ( _tcscmp( sVarName, _T("tm_time") ) == 0 )
						{
							SYSTEMTIME sysTime;
							GetLocalTime(&sysTime);
							sValue.Format( _T("%02d:%02d:%02d"), sysTime.wHour, sysTime.wMinute, sysTime.wSecond );
						}
						else if ( _tcscmp( sVarName, _T("tm_date") ) == 0 )
						{
							SYSTEMTIME sysTime;
							GetLocalTime(&sysTime);
							sValue.Format( _T("%02d/%02d/%04d"), sysTime.wDay, sysTime.wMonth, sysTime.wYear );
						}
						else if ( _tcscmp( sVarName, _T("tm_hours") ) == 0 )
						{
							SYSTEMTIME sysTime;
							GetLocalTime(&sysTime);
							sValue.Format( _T("%02d"), sysTime.wHour );
						}
						else if ( _tcscmp( sVarName, _T("tm_minutes") ) == 0 )
						{
							SYSTEMTIME sysTime;
							GetLocalTime(&sysTime);
							sValue.Format( _T("%02d"), sysTime.wMinute );
						}
						else if ( _tcscmp( sVarName, _T("tm_seconds") ) == 0 )
						{
							SYSTEMTIME sysTime;
							GetLocalTime(&sysTime);
							sValue.Format( _T("%02d"), sysTime.wSecond );
						}
						else if ( _tcscmp( sVarName, _T("tm_year") ) == 0 )
						{
							SYSTEMTIME sysTime;
							GetLocalTime(&sysTime);
							sValue.Format( _T("%04d"), sysTime.wYear );
						}
						else if ( _tcscmp( sVarName, _T("tm_month") ) == 0 )
						{
							SYSTEMTIME sysTime;
							GetLocalTime(&sysTime);
							sValue.Format( _T("%02d"), sysTime.wMonth );
						}
						else if ( _tcscmp( sVarName, _T("tm_day") ) == 0 )
						{
							SYSTEMTIME sysTime;
							GetLocalTime(&sysTime);
							sValue.Format( _T("%02d"), sysTime.wDay );
						}
						else
						{
							int iGstrID = GetGadgetStringID( sVarName );
							if ( iGstrID >= 0 )
							{
								GetGadgetString( iGstrID, sValue );
							}
						}
						sReturn += sValue;
						iPos++;
						iStart = iPos;
					}
				}
			}
			else
			{
				iPos++;
			}
		}
		if ( iStart < iPos && szArg[iStart] )
		{
			sReturn += CStdString( &szArg[iStart], (iPos-iStart) );
		}

	}
	return sReturn;
}


CStdString CActionScript::TranslateVar( LPCTSTR szArg )
{
	CStdString sReturn;

	sReturn = ::TranslateVar( szArg, &m_msLocalEnvironment );
#if 0

	if ( szArg && _tcslen( szArg ) )
	{
		int iPos = 0, iStart = 0;
		while( szArg[iPos] )
		{
			if ( szArg[iPos] == _T('%') )
			{
				if ( szArg[iPos+1] == _T('%') )
				{
					if ( iStart < iPos )
					{
						sReturn += _T('%');
					}
					iPos += 2;
					iStart = iPos;
				}
				else if ( szArg[iPos+1] == 0 )
				{
					// Just finish...
					iPos++;
				}
				else
				{
					// Add previous portion to return string
					if ( iStart < iPos )
					{
						sReturn += CStdString( &szArg[iStart], (iPos-iStart) );
					}
					iStart = iPos+1;
					iPos++;
					// Now read until next '%'
					while ( szArg[iPos] && szArg[iPos] != _T('%') )
					{
						iPos++;
					}
					if ( szArg[iPos] == _T('%') )
					{
						CStdString sVarName( &szArg[iStart], (iPos-iStart) );
						CStdString sValue;
						sVarName.MakeLower();
						if ( sVarName[0] == _T('_') )
						{
							sVarName = sVarName.Mid(1);
							if ( g_msGlobalEnvironment.size() && g_msGlobalEnvironment.find( sVarName ) != g_msGlobalEnvironment.end() )
							{
								sValue = g_msGlobalEnvironment[sVarName];
							}
						}
						else
						{
							if ( m_msLocalEnvironment.size() && m_msLocalEnvironment.find( sVarName ) != m_msLocalEnvironment.end() )
							{
								sValue = m_msLocalEnvironment[sVarName];
							}
						}
						sReturn += sValue;
						iPos++;
						iStart = iPos;
					}
				}
			}
			else if ( szArg[iPos] == _T('$') )
			{
				if ( szArg[iPos+1] == _T('$') )
				{
					if ( iStart < iPos )
					{
						sReturn += _T('$');
					}
					iPos += 2;
					iStart = iPos;
				}
				else if ( szArg[iPos+1] == 0 )
				{
					// Just finish...
					iPos++;
				}
				else
				{
					// Add previous portion to return string
					if ( iStart < iPos )
					{
						sReturn += CStdString( &szArg[iStart], (iPos-iStart) );
					}
					iStart = iPos+1;
					iPos++;
					// Now read until next '$'
					while ( szArg[iPos] && szArg[iPos] != _T('$') )
					{
						iPos++;
					}
					if ( szArg[iPos] == _T('$') )
					{
						CStdString sVarName( &szArg[iStart], (iPos-iStart) );
						CStdString sValue;
						sVarName.MakeLower();

						if ( _tcscmp( sVarName, _T("eol") ) == 0 )
						{
							sValue = _T("\r\n");
						}
						if ( _tcscmp( sVarName, _T("formatresult") ) == 0 )
						{
							sValue = g_sFormatResult;
						}						
						if ( _tcscmp( sVarName, _T("lasterror") ) == 0 )
						{
							sValue.Format( _T("ERROR CODE: 0x%08X"), GetLastError() );
						}
						if ( _tcscmp( sVarName, _T("dvd_mode") ) == 0 )
						{
							if ( g_MenuInfo.m_bDVDMode )
							{
								sValue = _T("1");
							}
							else
							{
								sValue = _T("0");
							}
						}
						else if ( _tcscmp( sVarName, _T("gt") ) == 0 )
						{
							sValue = _T(">");
						}
						else if ( _tcscmp( sVarName, _T("lt") ) == 0 )
						{
							sValue = _T("<");
						}
						else if ( _tcscmp( sVarName, _T("tab") ) == 0 )
						{
							sValue = _T("\x08");
						}
						else
						{
							int iGstrID = GetGadgetStringID( sVarName );
							if ( iGstrID >= 0 )
							{
								GetGadgetString( iGstrID, sValue );
							}
						}
						sReturn += sValue;
						iPos++;
						iStart = iPos;
					}
				}
			}
			else
			{
				iPos++;
			}
		}
		if ( iStart < iPos && szArg[iStart] )
		{
			sReturn += CStdString( &szArg[iStart], (iPos-iStart) );
		}

	}
#endif	
	return sReturn;
}



CStdString CActionScript::GetArgument( int iArg, bool bTranslate )
{
	CStdString sReturn;
	
	sReturn = ::GetArg( m_sCurrLine, iArg );
	if ( bTranslate )
	{
		sReturn = TranslateVar( sReturn );
	}	
	return sReturn;
}


bool CActionScript::GoToLine( int iLine )
{
	bool bReturn = false;
	
	if ( iLine < 0 )
	{
		m_iterProgLine = m_lsProgram.end();
	}
	else
	{
		m_iCurrLine = 0;
		m_iterProgLine = m_lsProgram.begin();
		while( iLine && (m_iterProgLine != m_lsProgram.end()))
		{
			m_iterProgLine++;
			iLine--;
			m_iCurrLine++;
		}

	}	
	return bReturn;
}

bool CActionScript::GoToLabel( LPCTSTR szLabel )
{
	bool bReturn = false;
	TMapLabels::iterator iterLabel;
	CStdString sLabel;

	if ( szLabel && _tcslen(szLabel) && szLabel[0] == _T(':') )
	{
		szLabel++;
	}
	sLabel = szLabel;
	sLabel.MakeLower();
	if ( szLabel && _tcslen(szLabel) && ((iterLabel = m_mlLabels.find( sLabel ) ) != m_mlLabels.end()) )
	{
		int iLine = m_mlLabels[sLabel];
		// Found it
		GoToLine( iLine );
	}
	else
	{
		SetError( _T("Unable to find label") );
	}
	return bReturn;
}


CStdString CActionScript::GetEnvValue( LPCTSTR szVarName )
{
	CStdString sReturn;
	CStdString sVarName;
	bool bLocal = true;

	if ( szVarName && _tcslen( szVarName ) )
	{
		if ( szVarName[0] == _T('_') )
		{
			bLocal = false;
			szVarName++;
		}
		if ( _tcslen( szVarName ) )
		{
			int iCheck = 0;

			sVarName = szVarName;
			sVarName.MakeLower();
			if ( bLocal )
			{
				if ( m_msLocalEnvironment.find( sVarName ) != m_msLocalEnvironment.end() )
				{
					sReturn = m_msLocalEnvironment[sVarName];
				}
			}
			else
			{
				if ( g_msGlobalEnvironment.find( sVarName ) != g_msGlobalEnvironment.end() )
				{
					sReturn = g_msGlobalEnvironment[sVarName];
				}
			}
		}
	}
	return sReturn;
}

bool CActionScript::SetEnvValue( LPCTSTR szVarName, LPCTSTR szValue )
{
	bool bReturn = false;

	bReturn = SetActionVar( szVarName, szValue, &m_msLocalEnvironment );
#if 0
	bool bLocal = true;
	CStdString sVarName;

	if ( szVarName && _tcslen( szVarName ) )
	{
		if ( szVarName[0] == _T('_') )
		{
			bLocal = false;
			szVarName++;
		}
		if ( _tcslen( szVarName ) )
		{
			int iCheck = 0;

			while( szVarName[iCheck] && ( _istalnum( szVarName[iCheck] )|| ( _T('_') == szVarName[iCheck]) ) )
			{
				iCheck++;
			}
			if ( szVarName[iCheck] == 0 ) // Passed check
			{
				sVarName = szVarName;
				sVarName.MakeLower();
				if ( szValue && _tcslen( szValue ) )
				{
					if ( bLocal )
					{
						m_msLocalEnvironment[sVarName] = szValue;
					}
					else
					{
						g_msGlobalEnvironment[sVarName] = szValue;
					}
				}
				else
				{
					// Reset variable - remove from list!
					if ( bLocal )
					{
						m_msLocalEnvironment[sVarName] = _T("");
					}
					else
					{
						g_msGlobalEnvironment[sVarName] = _T("");
					}
				}
				bReturn = true;
			}
		}
	}
#endif	
	return bReturn;
}

int CActionScript::ExecuteLine( LPCTSTR szCommand )
{
	int iResult = ASRC_NOTIMPL;
	
	return iResult;
}

bool CActionScript::ScanProgram( LPCTSTR szAction )
{
	bool bReturn = false;
	
	CStdString sAction(szAction);
	CStdString sLine, sLabel;
	int iPos = 0, iLen = 0, iLine = 0;
	bool bContinue = true;
	CStdString sError;

	m_lsProgram.clear();

	sAction.Trim();
	if ( szAction && sAction.GetLength() )
	{
		sAction.Replace( _T("\t"), _T(" ") );
		sAction.Replace( _T("\r\n"), _T("\x01") );
		sAction.Replace( _T("\n\r"), _T("\x01") );
		sAction.Replace( _T("\n"), _T("\x01") );
		sAction.Replace( _T("\r"), _T("\x01") );
		sAction.Replace( _T("\x01"), _T("\r") );
		sAction.Trim();
		iPos = 0;
		iLen = sAction.Find( _T('\r'), 0 );
		if ( iLen == -1 )
		{
			iLen = sAction.GetLength();
		}
		iLen -= iPos;
		while( (iPos < sAction.GetLength()) )
		{
			if ( iLen )
			{
				m_sCurrLine = sAction.Mid( iPos, iLen );
				m_sCurrLine.Trim();
				if( m_sCurrLine.GetLength() )
				{
					// Look for conditionals or skip to label
					if ( m_sCurrLine[0] == _T(':') ) 
					{
						// Label! Mark it and continue
						sLabel = TranslateVar( GetArgument( 0, false ).Mid(1) );
						sLabel.MakeLower();
						// Save off the position to the next program line in the program list
						// and associate it with the label.
						if ( m_mlLabels.find(sLabel) == m_mlLabels.end() )
						{
							m_mlLabels[sLabel] = (int)(m_lsProgram.size());
							m_lsProgram.push_back( _T(" ") );
						}
						else
						{
							// Label already exists!!
							bContinue = false;
							m_sLastError.Format( _T("ERROR: (%d) Label \'%s\' already exists"), iLine, sLabel.c_str() );
						}
					}
					else if ( ( m_sCurrLine[0] == _T(';') ) || ( m_sCurrLine[0] == _T('#') ) || ( m_sCurrLine[0] == _T('\'') )  )
					{
						// Comment. Ignore this line!
						m_lsProgram.push_back( _T(" ") );
					}
					else if ( ( _tcsnicmp( m_sCurrLine, _T("rem "), 4 ) == 0 ) || ( _tcsnicmp( m_sCurrLine, _T("// "), 3 ) == 0 )  )
					{
						// Comment. Ignore this line!
						m_lsProgram.push_back( _T(" ") );
					}
					else
					{
						m_lsProgram.push_back( m_sCurrLine );
					}
				}
			}
			iPos += iLen;
			iPos++;
			if ( iPos < sAction.GetLength() )
			{
				iLen = sAction.Find( _T('\r'), iPos );
				if ( iLen == -1 )
				{
					iLen = sAction.GetLength();
				}
				iLen -= iPos;
			}
		}
		iLine++;
	}
	
	return bReturn;
}

CActionScript::CActionScript( LPCTSTR szScript, CActionScript * pParent, bool bIsContext ) :
	m_pParent(pParent),
	m_pActionDrawList(NULL),
	m_bDeleteDrawList(false),
	m_bBeginDraw(false),
	m_hFind(NULL),
	m_bIsContext(bIsContext),
	m_bError(false)
{
	if ( pParent )
	{
		m_sCurDir = pParent->m_sCurDir;
		m_pActionDrawList = pParent->m_pActionDrawList;
	}
	else
	{
		m_sCurDir = g_MenuInfo.m_sMXMPath;
	}
	if ( m_pActionDrawList == NULL )
	{
		m_pActionDrawList = new TActionDrawList;
		m_bDeleteDrawList = true;
	}
	ScanProgram( szScript );
}


CActionScript::~CActionScript()
{
	if ( m_hFind )
	{
		FindClose( m_hFind );
	}
	ASXML_ClearList( m_mxXMLData );
	if ( m_bDeleteDrawList )
	{
		ClearActionDrawList( *m_pActionDrawList );
		delete m_pActionDrawList;
	}
	CloseFileData( m_mfFileData );
}

CActionScript * CActionScript::CreateActionScript( LPCTSTR szScript )
{
	CActionScript * pReturn;

	pReturn = new CActionScript( szScript, this );
	return pReturn;
}


void CActionScript::SetError( LPCTSTR szMessage )
{
	m_sLastError.Format( _T("ERROR (line %d): (%s) \"%s\""), m_iCurrLine, m_sCurrCmd.c_str(), szMessage );
	DEBUG_LINE( m_sLastError ); 
	m_bError = true;
	GoToLine(-1);
}

DWORD	GetNumeric( LPCTSTR szNumber, bool bSigned = false )
{
	DWORD dwReturn = 0;
	int iBase = 10;
	
	if ( szNumber && _tcslen( szNumber ) )
	{
		if ( _tolower(szNumber[0]) == _T('x') )
		{
			szNumber++;
			iBase = 16;
		}
		if ( szNumber[0] == _T('0') && _tolower(szNumber[1]) == _T('x') ) 
		{
			szNumber += 2;
			iBase = 16;
		}
		if ( bSigned )
		{
			dwReturn = (DWORD)_tcstol( szNumber, NULL, iBase );
		}
		else
		{
			dwReturn = _tcstoul( szNumber, NULL, iBase );
		}
	}	
	return dwReturn;
}

void CActionScript::EvaluateIf( bool bNumeric, bool bFunction )
{
	// Grab arguments to see what we can evaluate
	// We can incorporate function keywords here as well, I should think
	// Functions must be proceeded by an "@" symbol
	CStdString sArg = GetArgument(1);
	CStdString sOpCode = GetArgument(2);
	CStdString sArg2 = GetArgument(3);
	int iNum1, iNum2, iEval;	
	int	iEndEvalPos = 2;
	bool bPassed = false;
	
	sArg.Trim();
//	if ( sArg.GetLength() )
	{
		if ( bFunction )
		{
			// We'll handle functions specially.
			CStdString sFunc, sValue;
			
			sFunc = GetArgument( 1 );
			sValue = HandleFunction( sFunc, 2, &iEndEvalPos );
			if ( bNumeric )
			{
				if ( GetNumeric( sValue ) )
				{
					bPassed = true;
				}
			}
			else
			{
				if ( sValue.GetLength() )
				{
					bPassed = true;
				}
			}
			
		}
		else
		{
			// One of two remaining cases: Simple string or three argument opcode
			sOpCode.MakeLower();
			if ( sOpCode.Compare( _T("goto") ) == 0 )
			{
				iEndEvalPos = 2;
				// Just check first argument
				if ( bNumeric )
				{
					if ( GetNumeric( sArg ) )
					{
						bPassed = true;
					}
				}
				else
				{
					if ( sArg.GetLength() )
					{
						bPassed = true;
					}
				}
			}
			else if ( ( sOpCode.Compare( _T("then") ) == 0 ) && ( sArg2.Compare( _T("goto") ) == 0 ) )
			{
				iEndEvalPos = 3;
				// Just check first argument
				if ( bNumeric )
				{
					if ( GetNumeric( sArg ) )
					{
						bPassed = true;
					}
				}
				else
				{
					if ( sArg.GetLength() )
					{
						bPassed = true;
					}
				}
			}
			else
			{
				iEndEvalPos = 4;
				iEval = 0;
				if ( bNumeric )
				{
					iNum1 = GetNumeric( sArg );
					iNum2 = GetNumeric( sArg2 );
					if ( iNum1 > iNum2 )
					{
						iEval = 1;
					}
					else if ( iNum1 < iNum2 )
					{
						iEval = -1;
					}
				}
				else
				{
					iEval = _tcsicmp( sArg, sArg2 );
				}
				if ( _tcscmp( sOpCode, _T("==") ) == 0 )
				{
					if ( iEval == 0 )
					{
						bPassed = true;
					}
				}
				else if ( _tcscmp( sOpCode, _T("<") ) == 0 )
				{
					if ( iEval < 0 )
					{
						bPassed = true;
					}
				}
				else if ( _tcscmp( sOpCode, _T(">") ) == 0 )
				{
					if ( iEval > 0 )
					{
						bPassed = true;
					}
				}
				else if ( _tcscmp( sOpCode, _T("<=") ) == 0 )
				{
					if ( iEval <= 0 )
					{
						bPassed = true;
					}
				}
				else if ( _tcscmp( sOpCode, _T(">=") ) == 0 )
				{
					if ( iEval >= 0 )
					{
						bPassed = true;
					}
				}
				else if ( ( _tcscmp( sOpCode, _T("<>") ) == 0 ) || (_tcscmp( sOpCode, _T("!=") ) == 0 ) )
				{
					if ( iEval )
					{
						bPassed = true;
					}
				}
				else if ( _tcscmp( sOpCode, _T("##") ) == 0  )
				{
					if ( bNumeric )
					{
						SetError( _T("ERROR: Not a numeric conditional") );
					}
					else
					{
						if ( sArg.Find( sArg2 ) >= 0 )
						{
							bPassed = true;
						}
					}
				}
				else if ( _tcscmp( sOpCode, _T("#^") ) == 0  )
				{
					if ( bNumeric )
					{
						SetError( _T("ERROR: Not a numeric conditional") );
					}
					else
					{
						sArg.MakeLower();
						sArg2.MakeLower();
						if ( sArg.Find( sArg2 ) >= 0 )
						{
							bPassed = true;
						}
					}
				}
				else if ( _tcscmp( sOpCode, _T("!##") ) == 0  )
				{
					if ( bNumeric )
					{
						SetError( _T("ERROR: Not a numeric conditional") );
					}
					else
					{
						if ( sArg.Find( sArg2 ) < 0 )
						{
							bPassed = true;
						}
					}
				}
				else if ( _tcscmp( sOpCode, _T("!#^") ) == 0  )
				{
					if ( bNumeric )
					{
						SetError( _T("ERROR: Not a numeric conditional") );
					}
					else
					{
						sArg.MakeLower();
						sArg2.MakeLower();
						if ( sArg.Find( sArg2 ) < 0 )
						{
							bPassed = true;
						}
					}
				}
				else
				{
					SetError( _T("Unknown OpCode") );
				}
			}
		}
	}
	if ( bPassed )
	{
		// Find GOTO or THEN in line, and grab the associated label
		CStdString sLabel;
		sArg = GetArgument(iEndEvalPos);
		while( sArg.GetLength() && _tcsicmp( sArg, _T("goto") ) )
		{
			iEndEvalPos++;
			sArg = GetArgument(iEndEvalPos);
		}
		if ( sArg.GetLength() )
		{
			sArg = GetArgument( iEndEvalPos+1 );
		}
		if ( sArg.GetLength() )
		{
			GoToLabel( sArg );
		}
		else
		{
			SetError( _T("Syntax error - no GOTO") );
		}
	}
}

CStdString	CActionScript::HandleFunction( LPCTSTR szFunc, int iFirstArg, int * piLastArg )
{
	CStdString sReturn;
	
	if ( _tcsicmp( szFunc, _T("right") ) == 0 )
	{
		CStdString sString = GetArgument( iFirstArg+1 );
		int iCount = GetNumeric( GetArgument( iFirstArg ));

		sReturn = sString.Right(iCount);
		iFirstArg+= 2;
	}
	else if ( _tcsicmp( szFunc, _T("left") ) == 0 )
	{
		CStdString sString = GetArgument( iFirstArg+1 );
		int iCount = GetNumeric( GetArgument( iFirstArg ));

		sReturn = sString.Left(iCount);
		iFirstArg+= 2;
	}
	else if ( _tcsicmp( szFunc, _T("replace") ) == 0 )
	{
		CStdString sOld = GetArgument( iFirstArg );
		CStdString sNew = GetArgument( iFirstArg+1 );
		CStdString sSource = GetArgument( iFirstArg+2 );

		sSource.Replace( sOld, sNew );
		sReturn = sSource;
		iFirstArg+= 3;
	}
	else if ( _tcsicmp( szFunc, _T("mid") ) == 0 )
	{
		CStdString sString = GetArgument( iFirstArg+2 );
		int iPos = GetNumeric( GetArgument( iFirstArg ));
		int iCount = GetNumeric( GetArgument( iFirstArg+1 ));

		sReturn = sString.Mid(iPos, iCount);
		iFirstArg+= 3;
	}
	else if ( _tcsicmp( szFunc, _T("fileexists") ) == 0 )
	{
		CStdString sFilename;

		sFilename = MakeFullFilePath( m_sCurDir, GetArgument( iFirstArg ) );
		if ( FileExists( sFilename ) )
		{
			sReturn = _T("1");
		}
		else
		{
			sReturn = _T("0");
		}
		iFirstArg++;
	}
	else if ( _tcsicmp( szFunc, _T("contextscriptexists") ) == 0 )
	{
		CStdString sScriptName = GetArgument( iFirstArg );
		if ( DoesScriptExist( sScriptName, true ) )
		{
			sReturn = _T("1");
		}
		else
		{
			sReturn = _T("0");
		}
		iFirstArg++;
	}
	else if ( _tcsicmp( szFunc, _T("scriptexists") ) == 0 )
	{
		CStdString sScriptName = GetArgument( iFirstArg );
		if ( DoesScriptExist( sScriptName, false ) )
		{
			sReturn = _T("1");
		}
		else
		{
			sReturn = _T("0");
		}
		iFirstArg++;
	}
	else if ( _tcsicmp( szFunc, _T("filesize") ) == 0 )
	{
		CStdString sFilename;

		sFilename = MakeFullFilePath( m_sCurDir, GetArgument( iFirstArg ) );
		if ( FileExists( sFilename ) )
		{						
			DWORD dwFileSize = FileLength( sFilename );
			if ( dwFileSize )
			{
				sReturn.Format( _T("%d"), dwFileSize );
			}
			else
			{
				ULONGLONG u64DirSize;

				u64DirSize = DirectorySpace( sFilename );
				sReturn.Format( _T("%I64d"), u64DirSize );
			}
		}
		else
		{
			sReturn = _T("0");
		}
		iFirstArg++;
	}
	else if ( _tcsicmp( szFunc, _T("pathempty") ) == 0 )
	{
		CStdString sPath;
//		bool bHasFiles;
		HANDLE hFindPath;
		WIN32_FIND_DATA ffDataPath;


		sPath = PathSlasher( MakeFullDirPath( m_sCurDir, GetArgument( iFirstArg ) ), true );
		sPath += _T("*");
		iFirstArg++;

		hFindPath = FindFirstFile( sPath, &ffDataPath );
		if ( INVALID_HANDLE_VALUE != hFindPath )
		{
			// Don't care WHAT we found... just that we found it.
			FindClose( hFindPath );
			sReturn = _T("0");
		}
		else
		{
			sReturn = _T("1");
		}
	}
	else if ( _tcsicmp( szFunc, _T("xbeid") ) == 0 )
	{
		CStdString sFilename;

		sFilename = MakeFullFilePath( m_sCurDir, GetArgument( iFirstArg++ ) );
		if ( FileExists( sFilename ) )
		{						
			sReturn.Format( _T("%d"), GetXbeID( sFilename ) );
		}
		else
		{
			sReturn = _T("0");
		}
	}
	else if ( _tcsicmp( szFunc, _T("space") ) == 0 )
	{
		CStdString sCount = GetArgument( iFirstArg++ );
		int iCount = _tcstoul( sCount, NULL, 10 );

		if ( iCount )
		{
			sReturn.Format( _T("%*s"), iCount, _T("") );
		}
		else
		{
			sReturn = _T("");
		}
	}
	else if ( _tcsicmp( szFunc, _T("length") ) == 0 )
	{
		CStdString sText = GetArgument( iFirstArg++ );

		sReturn.Format( _T("%d"), sText.GetLength() );
	}
	else if ( _tcsicmp( szFunc, _T("trim") ) == 0 )
	{
		CStdString sText = GetArgument( iFirstArg++ );

		sText.Trim();
		sReturn = sText;
	}
	else if ( _tcsicmp( szFunc, _T("trimleft") ) == 0 )
	{
		CStdString sText = GetArgument( iFirstArg++ );

		sText.TrimLeft();
		sReturn = sText;
	}
	else if ( _tcsicmp( szFunc, _T("trimright") ) == 0 )
	{
		CStdString sText = GetArgument( iFirstArg++ );

		sText.TrimRight();
		sReturn = sText;
	}
	else if ( _tcsicmp( szFunc, _T("lower") ) == 0 )
	{
		CStdString sText = GetArgument( iFirstArg++ );

		sText.MakeLower();
		sReturn = sText;
	}
	else if ( _tcsicmp( szFunc, _T("upper") ) == 0 )
	{
		CStdString sText = GetArgument( iFirstArg++ );

		sText.MakeUpper();
		sReturn = sText;
	}
	else if ( _tcsicmp( szFunc, _T("tempfile") ) == 0 )
	{
		CStdString sPath = GetArgument( iFirstArg++ );
		CStdString sPrefix = GetArgument( iFirstArg++ );

		sReturn = GetTempFileName( sPath, sPrefix );
	}
	else if ( _tcsicmp( szFunc, _T("fatxify") ) == 0 )
	{
		CStdString sText = GetArgument( iFirstArg++ );
		CStdString sReplace = GetArgument( iFirstArg++ );

		// sText.MakeUpper();
		sReturn = FatXify( sText, false, sReplace.GetLength()?sReplace[0]:0 );
	}
	else if ( _tcsicmp( szFunc, _T("repeat") ) == 0 )
	{
		CStdString sCount = GetArgument( iFirstArg++ );
		CStdString sRepeat = GetArgument( iFirstArg++ );
		int iCount = _tcstoul( sCount, NULL, 10 );

		sReturn = _T("");
		if ( iCount )
		{
			while( iCount-- )
			{
				sReturn += sRepeat;
			}
		}
	}
	else if ( _tcsicmp( szFunc, _T("fnmerge") ) == 0 )
	{
		CStdString sPath, sBasename, sExt;
		CStdString sFilename;

		sPath =  MakeFullDirPath( m_sCurDir, GetArgument( iFirstArg++ ) );
		sFilename = GetArgument( iFirstArg++ );
		sExt = GetArgument( iFirstArg++ );
		if ( sExt.GetLength() )
		{
			sFilename = ReplaceFilenameExtension( sFilename, sExt );
		}
		sReturn = MakeFullFilePath( sPath, sFilename );
	}
	else if ( _tcsicmp( szFunc, _T("fnpath") ) == 0 )
	{
		CStdString sFilename;

		sFilename = MakeFullFilePath( m_sCurDir, GetArgument( iFirstArg++ ) );
		sReturn = StripFilenameFromPath( sFilename );
	}
	else if ( _tcsicmp( szFunc, _T("fnbasename") ) == 0 )
	{
		CStdString sFilename;

		sFilename = MakeFullFilePath( m_sCurDir, GetArgument( iFirstArg++ ) );
		sFilename = ExtractFilenameFromPath( sFilename, false );
		sReturn = GetFilenameBase( sFilename );
	}
	else if ( _tcsicmp( szFunc, _T("fnextension") ) == 0 )
	{
		CStdString sFilename;

		sFilename = MakeFullFilePath( m_sCurDir, GetArgument( iFirstArg++ ) );
		sFilename = ExtractFilenameFromPath( sFilename, false );
		sReturn = GetFilenameExtension( sFilename );
	}
	else if ( _tcsicmp( szFunc, _T("xbetitle") ) == 0 )
	{
		CStdString sFilename;

		sFilename = MakeFullFilePath( m_sCurDir, GetArgument( iFirstArg++ ) );
		if ( FileExists( sFilename ) )
		{						
			sReturn = GetXbeTitle( sFilename );
		}
		else
		{
			sReturn = _T("");
		}
	}
	else
	{
		// If it makes it this far, we've got problems!
		SetError( _T("Function not found!") );
	}
	
	if ( piLastArg )
	{
		(*piLastArg) = iFirstArg;
	}
	return sReturn;
}


CStdString CActionScript::GetScript( LPCTSTR szScriptName, bool bContext )
{
	return GetInternalScript( szScriptName, bContext );
}

bool CActionScript::ExecuteScript( void )
{
	bool bReturn = false;
	CStdString sError;

	if ( m_lsProgram.size() )
	{
		if ( m_bIsContext )
		{
			ContextInit();
		}
		m_iCurrLine = 0;
		m_iterProgLine = m_lsProgram.begin();
		while( m_iterProgLine != m_lsProgram.end() )
		{
			m_sCurrLine = m_iterProgLine->c_str();
			m_sCurrLine.Trim();
			// DEBUG_FORMAT( _T("ActionScript: (%d) %s"), m_iCurrLine, m_sCurrLine.c_str() );
			m_iterProgLine++; // We advance now... command might change it, though
			m_iCurrLine++;
			
			// We skip over blank lines.
			if ( m_sCurrLine.GetLength() )
			{
				m_sCurrCmd = GetArgument( 0, false );
				m_sCurrCmd.MakeLower();
				if ( m_sCurrCmd.Compare( _T("goto") ) == 0 )
				{
					GoToLabel( GetArgument( 1 ) );
				}
				else if ( m_sCurrCmd.Compare( _T("set") ) == 0 )
				{
					if ( !SetEnvValue( GetArgument( 1 ), GetArgument( 2 ) ) )
					{
						SetError( _T("Unable to set variable") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("setfunc") ) == 0 )
				{
					CStdString sVarName, sFunc, sValue;
					
					sVarName = GetArgument( 1 );
					sFunc = GetArgument( 2 );
					
					sValue = HandleFunction( sFunc, 3, NULL );
					if ( !m_bError )
					{
						if ( !SetEnvValue( sVarName, sValue ) )
						{
							SetError( _T("Unable to set variable") );
						}
					}
				}
				else if ( m_sCurrCmd.Compare( _T("add") ) == 0 )
				{
					int iVal, iAdd;
					CStdString sVarName = GetArgument(1);
					CStdString sValue;
					iAdd = GetNumeric( GetArgument(2) );
					iVal = GetNumeric( GetEnvValue( sVarName ) );
					iVal += iAdd;
					sValue.Format( _T("%d"), iVal );
					if ( !SetEnvValue( sVarName, sValue ) )
					{
						SetError( _T("Unable to set variable") );
					}
					
				}
				else if ( m_sCurrCmd.Compare( _T("sub") ) == 0 )
				{
					int iVal, iSub;
					CStdString sVarName = GetArgument(1);
					CStdString sValue;
					iSub = GetNumeric( GetArgument(2) );
					iVal = GetNumeric( GetEnvValue( sVarName ) );
					iVal -= iSub;
					sValue.Format( _T("%d"), iVal );
					if ( !SetEnvValue( sVarName, sValue ) )
					{
						SetError( _T("Unable to set variable") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("div") ) == 0 )
				{
					int iVal, iDiv;
					CStdString sVarName = GetArgument(1);
					CStdString sValue;
					iDiv = GetNumeric( GetArgument(2) );
					iVal = GetNumeric( GetEnvValue( sVarName ) );
					if ( iDiv )
					{
						iVal /= iDiv;
					}
					else
					{
						iVal = 0;
					}
					sValue.Format( _T("%d"), iVal );
					if ( !SetEnvValue( sVarName, sValue ) )
					{
						SetError( _T("Unable to set variable") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("mult") ) == 0 )
				{
					int iVal, iMult;
					CStdString sVarName = GetArgument(1);
					CStdString sValue;
					iMult = GetNumeric( GetArgument(2) );
					iVal = GetNumeric( GetEnvValue( sVarName ) );
					iVal *= iMult;
					sValue.Format( _T("%d"), iVal );
					if ( !SetEnvValue( sVarName, sValue ) )
					{
						SetError( _T("Unable to set variable") );
					}
				}
				else if ( _tcsnicmp( m_sCurrCmd, _T("if"), 2 ) == 0 )
				{
					bool bNumeric = false;
					bool bFunction = false;
					
					if ( m_sCurrCmd.GetLength() >= 3 )
					{
						if ( m_sCurrCmd[2] == _T('#') )
						{
							bNumeric = true;
						}
						else if ( m_sCurrCmd[2] == _T('@') )
						{
							bFunction = true;
						}
					}
					if ( m_sCurrCmd.GetLength() >= 4 )
					{
						if ( m_sCurrCmd[3] == _T('#') )
						{
							bNumeric = true;
						}
						else if ( m_sCurrCmd[3] == _T('@') )
						{
							bFunction = true;
						}
					}
					EvaluateIf( bNumeric, bFunction );
				}
				else if ( m_sCurrCmd.Compare( _T("quit") ) == 0 )
				{
					GoToLine(-1);
				}
				else if ( m_sCurrCmd.Compare( _T("callfile") ) == 0 )
				{
					CActionScript * pScript = NULL;
					CStdString	sScript = MakeFullFilePath( m_sCurDir, GetArgument(1) );					
					
					if ( FileExists( sScript ) )
					{
						{
							CStdString sActionSubScript;

							// Load the script into a string
							sActionSubScript = LoadStringFromFile( sScript );
			
							// Create the object
							pScript = CreateActionScript( sActionSubScript );
						}
						
						if ( pScript )
						{
							CStdString sArg1, sVarName;
							
							pScript->m_msLocalEnvironment = m_msLocalEnvironment;
							// We'll call it here... after copying arguments into local environment
							pScript->SetEnvValue(_T("ActionScriptFile"), sScript );
							pScript->SetEnvValue(_T("ActionScriptName"), _T("") );
							pScript->SetEnvValue(_T("0"), sScript );
							
							sArg1 = GetArgument( 2 );
							int iIndex = 0;
							while( sArg1.GetLength() )
							{
								iIndex++;
								sVarName.Format( _T("%d"), iIndex );
								pScript->SetEnvValue( sVarName, sArg1 );
								sArg1 = GetArgument( 2+iIndex );
							}
							sVarName.Format( _T("%d"), iIndex );
							pScript->SetEnvValue( _T("ArgCount"), sVarName );
							
							if ( pScript->ExecuteScript() )
							{
							}
							else
							{
								SetError( _T("ActionScript file failed execution") );
							}
						}
						else
						{
							SetError( _T("Unable to allocate memory for ActionScript") );
						}
					}
					else
					{
						SetError( _T("ActionScript file does not exist") );
					}
					
					if ( pScript )
					{
						delete pScript;
					}
				}
				else if ( m_sCurrCmd.Compare( _T("callscript") ) == 0 )
				{
					CActionScript * pScript = NULL;
					CStdString	sScript = GetArgument(1);					
					
//					if ( FileExists( sScript ) )
//					{
						{
							CStdString sActionSubScript;

							// Load the script into a string
							sActionSubScript = GetScript( sScript, false );
			
							// Create the object
							pScript = CreateActionScript( sActionSubScript );
						}
						
						if ( pScript )
						{
							CStdString sArg1, sVarName;
							
							pScript->m_msLocalEnvironment = m_msLocalEnvironment;
							// We'll call it here... after copying arguments into local environment
							pScript->SetEnvValue(_T("ActionScriptName"), sScript );
							pScript->SetEnvValue(_T("ActionScriptFile"), _T("") );
							pScript->SetEnvValue(_T("0"), sScript );
							
							sArg1 = GetArgument( 2 );
							int iIndex = 0;
							while( sArg1.GetLength() )
							{
								iIndex++;
								sVarName.Format( _T("%d"), iIndex );
								pScript->SetEnvValue( sVarName, sArg1 );
								sArg1 = GetArgument( 2+iIndex );
							}
							sVarName.Format( _T("%d"), iIndex );
							pScript->SetEnvValue( _T("ArgCount"), sVarName );
							
							if ( pScript->ExecuteScript() )
							{
							}
							else
							{
								SetError( _T("ActionScript file failed execution") );
							}
						}
						else
						{
							SetError( _T("Unable to allocate memory for ActionScript") );
						}
//					}
//					else
//					{
//						SetError( _T("ActionScript file does not exist") );
//					}
					
					if ( pScript )
					{
						delete pScript;
					}
				}
				else if ( m_sCurrCmd.Compare( _T("callcontextscript") ) == 0 )
				{
					CActionScript * pScript = NULL;
					CStdString	sScript = GetArgument(1);				
					
//					if ( FileExists( sScript ) )
//					{
						{
							CStdString sActionSubScript;

							// Load the script into a string
							sActionSubScript = GetScript( sScript, true );
			
							// Create the object
							pScript = CreateActionScript( sActionSubScript );
						}
						
						if ( pScript )
						{
							CStdString sArg1, sVarName;
							
							pScript->m_msLocalEnvironment = m_msLocalEnvironment;
							// We'll call it here... after copying arguments into local environment
							pScript->SetEnvValue(_T("ActionScriptName"), sScript );
							pScript->SetEnvValue(_T("ActionScriptFile"), _T("") );
							pScript->SetEnvValue(_T("0"), sScript );
							
							sArg1 = GetArgument( 2 );
							int iIndex = 0;
							while( sArg1.GetLength() )
							{
								iIndex++;
								sVarName.Format( _T("%d"), iIndex );
								pScript->SetEnvValue( sVarName, sArg1 );
								sArg1 = GetArgument( 2+iIndex );
							}
							sVarName.Format( _T("%d"), iIndex );
							pScript->SetEnvValue( _T("ArgCount"), sVarName );
							
							if ( pScript->ExecuteScript() )
							{
							}
							else
							{
								SetError( _T("ActionScript file failed execution") );
							}
						}
						else
						{
							SetError( _T("Unable to allocate memory for ActionScript") );
						}
//					}
//					else
//					{
//						SetError( _T("ActionScript file does not exist") );
//					}
					
					if ( pScript )
					{
						delete pScript;
					}
				}
				else if ( m_sCurrCmd.Compare( _T("openread") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sFilename = MakeFullFilePath( m_sCurDir, GetArgument( 2 ) );


					CMXMFileData * pFileData = CreateFileData( m_mfFileData, sHandle );
					if ( pFileData )
					{
						pFileData->m_hFileHandle = CreateFile( sFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
						if ( pFileData->m_hFileHandle && pFileData->m_hFileHandle != INVALID_HANDLE_VALUE )
						{
							pFileData->m_bRead = true;
						}
						else
						{
							SetError( _T("Unable to open file") );
						}
					}
					else
					{
						SetError( _T("Unable to create handle") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("openwrite") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sFilename = MakeFullFilePath( m_sCurDir, GetArgument( 2 ) );


					CMXMFileData * pFileData = CreateFileData( m_mfFileData, sHandle );
					if ( pFileData )
					{
						pFileData->m_hFileHandle = CreateFile( sFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
						if ( pFileData->m_hFileHandle && pFileData->m_hFileHandle != INVALID_HANDLE_VALUE )
						{
							pFileData->m_bRead = false;
						}
						else
						{
							SetError( _T("Unable to open file") );
						}
					}
					else
					{
						SetError( _T("Unable to create handle") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("readfileln") ) == 0 )
				{
					CStdString sArg1 = GetArgument( 1 );
					CStdString sArg2 = GetArgument( 2 );
					CMXMFileData * pFileData = GetFileData( m_mfFileData, sArg1 );

					if ( pFileData )
					{
						if ( pFileData->m_hFileHandle && pFileData->m_bRead )
						{
							CStdString sValue = ReadFileLine( pFileData->m_hFileHandle );
							
							if ( !SetEnvValue( sArg2, sValue ) )
							{
								SetError( _T("Unable to set variable") );
							}
						}
						else
						{
							SetError( _T("Unable to read file") );
						}
					}
					else
					{
						SetError( _T("Handle does not exist") );
					}
				}
				else if ( ( m_sCurrCmd.Compare( _T("writefile") ) == 0 ) || ( m_sCurrCmd.Compare( _T("writefileln") ) == 0 ) )
				{
					bool bWriteEOL = false;
					if ( m_sCurrCmd.Compare( _T("writefileln") ) == 0 )
					{
						bWriteEOL = true;
					}
					CStdString sArg1 = GetArgument( 1 );
					CStdString sArg2 = GetArgument( 2 );
					CMXMFileData * pFileData = GetFileData( m_mfFileData, sArg1 );

					if ( pFileData )
					{
						if ( pFileData->m_hFileHandle && !pFileData->m_bRead )
						{
							WriteFileLine( pFileData->m_hFileHandle, sArg2, true );
						}
						else
						{
							SetError( _T("Unable to write file") );
						}
					}
					else
					{
						SetError( _T("Handle does not exist") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("closefile") ) == 0 )
				{
					CStdString sArg1 = GetArgument( 1 );

					if ( !DeleteFileData( m_mfFileData, sArg1 ) )
					{
						SetError( _T("Unable to delete handle") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("filestat") ) == 0 )
				{
					CStdString sArg1 = GetArgument( 1 );
					CStdString sVarName = GetArgument( 2 );
					CStdString sValue;
					CMXMFileData * pFileData = GetFileData( m_mfFileData, sArg1 );

					if ( pFileData )
					{
						if ( pFileData->m_hFileHandle )
						{
							DWORD dwValueLow = 0, dwValueHigh = 0;
							if ( ((dwValueLow=SetFilePointer( pFileData->m_hFileHandle, 0, (PLONG)&dwValueHigh, FILE_CURRENT )) == INVALID_SET_FILE_POINTER) && (GetLastError()!=NO_ERROR) )
							{
								// Error!
								sValue = _T("");
							}
							else
							{
								if ( pFileData->m_bRead ) 
								{
									DWORD dwSizeLow = 0, dwSizeHigh = 0;
									if ( ( (dwSizeLow = GetFileSize( pFileData->m_hFileHandle, &dwSizeHigh )) == dwValueLow ) && ( dwSizeHigh == dwValueHigh ))
									{
										// End of file
										sValue = _T("0");
									}
									else
									{
										// In file...
										sValue = _T("1");
										// Have to reset file pointer!!
										SetFilePointer( pFileData->m_hFileHandle, dwValueLow, (PLONG)&dwValueHigh, FILE_BEGIN );
									}
								}
								else
								{
									// In file...
									sValue = _T("1");
								}
							}
						}
						else
						{
							// Doesn't exist. Return "NULL"
							sValue = _T("");
						}
					}
					else
					{
						// Doesn't exist. Return "NULL"
						sValue = _T("");
					}
					if ( !SetEnvValue( sVarName, sValue ) )
					{
						SetError( _T("Unable to set variable") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("begindraw") ) == 0 )
				{
					if ( !m_bBeginDraw )
					{
						CStdString sArg1 = GetArgument( 1 );
						sArg1.MakeLower();
						if ( m_pActionDrawList )
						{
							ClearActionDrawList( *m_pActionDrawList );
							// GetStateApp()->m_pdrawList = NULL;
						}

						m_bBeginDraw = true;
						if ( _tcsicmp( sArg1, _T("usecurrent") ) != 0  ) // && g_ActionDrawState.m_pBackTexture )
						{
							CADBox * pBox = new CADBox;

							if ( pBox )
							{
								pBox->SetPos( 0, 0, 640, 480 );
								pBox->SetColor( 0xFF000000, 0xFF000000 );
								if ( m_pActionDrawList )
								{
									AddActionDrawList( *m_pActionDrawList, pBox );
								}
								else
								{
									delete pBox;
								}
							}
						}
					}
				}
				else if ( m_sCurrCmd.Compare( _T("messagebox") ) == 0 )
				{
					CStdString sArg1;
					sArg1 = GetArgument( 1 );
					if ( m_bBeginDraw )
					{
						// Render to current drawing screen....
						CADMsgBox * pMsgBox = new CADMsgBox;

						if ( pMsgBox )
						{
							pMsgBox->SetMessage(sArg1);
							if ( m_pActionDrawList )
							{
								AddActionDrawList( *m_pActionDrawList, pMsgBox );
							}
							else
							{
								delete pMsgBox;
							}
						}

						// GetStateApp()->DrawMessageBox( sArg1 );
					}
					else
					{
						GetStateApp()->MessageBox( _T("%s"), sArg1.c_str() );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("box") ) == 0 )
				{
					CStdString sArgX, sArgY, sArgW, sArgH, sArgCol,sArgColBorder;
					DWORD dwColor, dwBorderColor;
					DWORD dwX, dwY, dwW, dwH;
					sArgX = GetArgument( 1 );
					sArgY = GetArgument( 2 );
					sArgW = GetArgument( 3 );
					sArgH = GetArgument( 4 );
					sArgCol = GetArgument( 5 );
					sArgColBorder = GetArgument( 6 );

					dwX = _tcstoul( sArgX, NULL, 10 ); 
					dwW = _tcstoul( sArgW, NULL, 10 ); 
					dwY = _tcstoul( sArgY, NULL, 10 ); 
					dwH = _tcstoul( sArgH, NULL, 10 ); 

					dwColor = ConvertColorName( sArgCol );
					dwBorderColor = ConvertColorName( sArgColBorder );
					if ( m_bBeginDraw )
					{
						CADBox * pBox = new CADBox;

						if ( pBox )
						{
							pBox->SetPos( dwX, dwY, dwW, dwH );
							pBox->SetColor( dwColor, dwBorderColor );
							if ( m_pActionDrawList )
							{
								AddActionDrawList( *m_pActionDrawList, pBox );
							}
							else
							{
								delete pBox;
							}
						}

						// Render to current drawing screen....
					}
				}
				else if ( m_sCurrCmd.Compare( _T("text") ) == 0 )
				{
					CStdString sArgX, sArgY, sArgJ, sArgText, sArgCol;
					DWORD dwColor, dwFlags;
					DWORD dwX, dwY;
					sArgX = GetArgument( 1 );
					sArgY = GetArgument( 2 );
					sArgJ = GetArgument( 3 );
					sArgText = GetArgument( 4 );
					sArgCol = GetArgument( 5 );
					// sArgColBorder = GetArgument( 5 ));

					dwFlags = XBFONT_LEFT;
					dwX = _tcstoul( sArgX, NULL, 10 ); 
					dwY = _tcstoul( sArgY, NULL, 10 ); 

					dwColor = ConvertColorName( sArgCol );

					if ( _tcsicmp( sArgJ, _T("center") ) == 0 )
					{
						dwFlags = XBFONT_CENTER_X;
					}
					if ( _tcsicmp( sArgJ, _T("right") ) == 0 )
					{
						dwFlags = XBFONT_RIGHT;
					}

					if ( m_bBeginDraw )
					{
						CADText * pText = new CADText;

						if ( pText )
						{
							pText->SetPos( dwX, dwY, dwFlags );
							pText->SetColor( dwColor );
							pText->SetText( sArgText );
							if ( m_pActionDrawList )
							{
								AddActionDrawList( *m_pActionDrawList, pText );
							}
							else
							{
								delete pText;
							}
						}

						// Render to current drawing screen....
					}

				}
				else if ( m_sCurrCmd.Compare( _T("enddraw") ) == 0 )
				{
					if ( m_bBeginDraw )
					{
						GetStateApp()->m_pdrawList = m_pActionDrawList;
						m_bBeginDraw = false;
					}
				}
				else if ( m_sCurrCmd.Compare( _T("input") ) == 0 )
				{
					CStdString sArg1 = GetArgument( 1 );
					bool bWait = true;

					if ( _tcsicmp( sArg1, _T("nowait") ) == 0 )
					{
						bWait = false;
					}
					// If currently drawing, wrap it up and display it!
					XBGAMEPAD* pGamePad = NULL;
					
					if ( m_bBeginDraw )
					{
						GetStateApp()->m_pdrawList = m_pActionDrawList;
						m_bBeginDraw = false;
					}

					
					pGamePad = GetStateApp()->GetDefaultGamepad();
					while( !HasInput(pGamePad) && bWait )
					{
						pGamePad = GetStateApp()->GetDefaultGamepad();
					}
					ReadInputToEnv(pGamePad);
				}

				else if ( m_sCurrCmd.Compare( _T("xmlcreate") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sNodeName = GetArgument( 2 );

					ASXML_Create( sHandle, sNodeName, m_mxXMLData );
				}
				else if ( m_sCurrCmd.Compare( _T("xmlopen") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sFilename = GetArgument( 2 );

					ASXML_Open( sHandle, sFilename, m_sCurDir, m_mxXMLData );
				}
				else if ( m_sCurrCmd.Compare( _T("xmlclose") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );

					ASXML_Close( sHandle, m_mxXMLData );
				}
				else if ( m_sCurrCmd.Compare( _T("xmlsave") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sFilename = GetArgument( 2 );
					
					ASXML_Save( sHandle, sFilename, m_sCurDir, m_mxXMLData );
				}
				else if ( m_sCurrCmd.Compare( _T("xmlsavenode") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sFilename = GetArgument( 2 );
					
					ASXML_SaveNode( sHandle, sFilename, m_sCurDir, m_mxXMLData );
				}
				else if ( m_sCurrCmd.Compare( _T("xmlsetnodeptr") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sLocation = GetArgument( 2 );
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );

					if ( pXML )
					{
						pXML->SetNodePtr(sLocation);
					}
					else
					{
						SetError( _T("Unable to get XML object)") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("xmlgetnodeptr") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sVarName = GetArgument( 2 );
					CStdString sValue;
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );

					if ( pXML )
					{
						sValue = pXML->GetCurrentNodeLocation();
						if ( !SetEnvValue( sVarName, sValue ) )
						{
							SetError( _T("Unable to set variable") );
						}
					}
					else
					{
						SetError( _T("Unable to get XML object)") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("xmlcreatenode") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sLocation = GetArgument( 2 );
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );

					if ( pXML )
					{
						pXML->CreateNode( sLocation );
					}
					else
					{
						SetError( _T("Unable to get XML object") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("xmlcreateelement") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sLocation = GetArgument( 2 );
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );

					if ( pXML )
					{
						pXML->CreateElement( sLocation );
					}
					else
					{
						SetError( _T("Unable to get XML object") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("xmlsetelementptr") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sLocation = GetArgument( 2 );
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );

					if ( pXML )
					{
						pXML->SetElementPtr( sLocation );
					}
					else
					{
						SetError( _T("Unable to get XML object") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("xmlgetelementptr") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sVarName = GetArgument( 2 );
					CStdString sValue;
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );

					if ( pXML )
					{
						sValue = pXML->GetCurrentElementLocation();
						if ( !SetEnvValue( sVarName, sValue ) )
						{
							SetError( _T("Unable to set variable") );
						}
					}
					else
					{
						SetError( _T("Unable to get XML object") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("xmlsetelementvalue") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sValue = GetArgument( 2 );
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );

					if ( pXML )
					{
						pXML->SetElementValue( NULL, sValue );
					}
					else
					{
						SetError( _T("Unable to get XML object") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("xmlsetelementattr") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sLocation = GetArgument( 2 );
					CStdString sValue = GetArgument( 3 );
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );


					if ( pXML )
					{
						pXML->SetElementAttr( sLocation, sValue );
					}
					else
					{
						SetError( _T("Unable to get XML object") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("xmlsetnodeattr") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sLocation = GetArgument( 2 );
					CStdString sValue = GetArgument( 3 );
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );

					if ( pXML )
					{
						pXML->SetNodeAttr( sLocation, sValue );
					}
					else
					{
						SetError( _T("Unable to get XML object") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("xmlsetvalue") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sLocation = GetArgument( 2 );
					CStdString sValue = GetArgument( 3 );
					CStdString sPrefAttr = GetArgument( 4 );
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );

					if ( pXML )
					{
						pXML->SetValue( sLocation, sValue, sPrefAttr.GetLength()?true:false );
					}
					else
					{
						SetError( _T("Unable to get XML object") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("xmlgetvalue") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sVarName = GetArgument( 2 );
					CStdString sLocation = GetArgument( 3 );
					CStdString sDefault = GetArgument( 4 );
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );

					if ( pXML )
					{
						CStdString sValue = pXML->GetValue( sLocation, sDefault);
						if ( !SetEnvValue( sVarName, sValue ) )
						{
							SetError( _T("Unable to set variable") );
						}
					}
					else
					{
						SetError( _T("Unable to get XML object") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("xmlgetnodecount") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sVarName = GetArgument( 2 );
					CStdString sName = GetArgument( 3 );
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );

					if ( pXML )
					{
						int iCount = pXML->GetNodeCount( sName );
						CStdString sValue;

						sValue.Format( _T("%d"), iCount );
						if ( !SetEnvValue( sVarName, sValue ) )
						{
							SetError( _T("Unable to set variable") );
						}
					}
					else
					{
						SetError( _T("Unable to get XML object") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("xmlgetelementcount") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sVarName = GetArgument( 2 );
					CStdString sName = GetArgument( 3 );
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );

					if ( pXML )
					{
						int iCount = pXML->GetElementCount( sName );
						CStdString sValue;

						sValue.Format( _T("%d"), iCount );
						if ( !SetEnvValue( sVarName, sValue ) )
						{
							SetError( _T("Unable to set variable") );
						}
					}
					else
					{
						SetError( _T("Unable to get XML object") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("xmlgetnodeattrcount") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sVarName = GetArgument( 2 );
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );

					if ( pXML )
					{
						int iCount = pXML->GetNodeAttrCount();
						CStdString sValue;

						sValue.Format( _T("%d"), iCount );
						if ( !SetEnvValue( sVarName, sValue ) )
						{
							SetError( _T("Unable to set variable") );
						}
					}
					else
					{
						SetError( _T("Unable to get XML object") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("xmlgetelementattrcount") ) == 0 )
				{
					CStdString sHandle = GetArgument( 1 );
					CStdString sVarName = GetArgument( 2 );
					CASXmlInfo * pXML = ASXML_GetInfo( sHandle, m_mxXMLData );

					if ( pXML )
					{
						int iCount = pXML->GetElementAttrCount();
						CStdString sValue;

						sValue.Format( _T("%d"), iCount );
						if ( !SetEnvValue( sVarName, sValue ) )
						{
							SetError( _T("Unable to set variable") );
						}
					}
					else
					{
						SetError( _T("Unable to get XML object") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("searchitem") ) == 0 )
				{
					CStdString sVarName, sValue;
					sVarName = GetArgument( 1 );

					//SEARCHITEM <VarName>   (Load VarName with current file/pathname)
					if ( m_hFind )
					{
						sValue = MakeFullFilePath( m_sSearchPath, m_ffData.cFileName );
					}
					if ( !SetEnvValue( sVarName, sValue ) )
					{
						SetError( _T("Unable to set variable") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("searchsize") ) == 0 )
				{
					CStdString sVarName, sValue;
					sVarName = GetArgument( 1 );

					//SEARCHSIZE <VarName>   (Load VarName with current file size)
					if ( m_hFind )
					{
						ULARGE_INTEGER ulXlate;

						ulXlate.LowPart = m_ffData.nFileSizeLow;
						ulXlate.HighPart = m_ffData.nFileSizeHigh;

						sValue.Format( _T("%I64d"), ulXlate.QuadPart );
					}
					if ( !SetEnvValue( sVarName, sValue ) )
					{
						SetError( _T("Unable to set variable") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("searchtype") ) == 0 )
				{
					CStdString sVarName, sValue;
					sVarName = GetArgument( 1 );

					//SEARCHTYPE <VarName>   (Load VarName with current type for entry - "file" or "dir")
					if ( m_hFind )
					{
						if ( m_ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
						{
							sValue = _T("dir");
						}
						else
						{
							sValue = _T("file");
						}
					}
					if ( !SetEnvValue( sVarName, sValue ) )
					{
						SetError( _T("Unable to set variable") );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("searchnext") ) == 0 )
				{
					//SEARCHNEXT
					if ( m_hFind )
					{
						if ( !FindNextFile( m_hFind, &m_ffData ) )
						{
							FindClose( m_hFind );
							m_hFind = NULL;
							SetEnvValue( _T("search_active"), NULL );
							SetEnvValue( _T("search_name"), NULL );
							SetEnvValue( _T("search_type"), NULL );
							SetEnvValue( _T("search_size"), NULL );
							SetEnvValue( _T("search_path"), NULL );
							SetEnvValue( _T("search_arg"), NULL );
						}
						else
						{
							SetEnvValue( _T("search_name"), m_ffData.cFileName );
							if ( m_ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
							{
								SetEnvValue( _T("search_type"), _T("dir"));
							}
							else
							{
								SetEnvValue( _T("search_type"), _T("file"));
							}
							{
								CStdString sTemp;
								ULARGE_INTEGER ulXlate;

								ulXlate.LowPart = m_ffData.nFileSizeLow;
								ulXlate.HighPart = m_ffData.nFileSizeHigh;

								sTemp.Format( _T("%I64d"), ulXlate.QuadPart );
								SetEnvValue( _T("search_size"), sTemp );
							}
						}
					}
				}
				else if ( m_sCurrCmd.Compare( _T("endsearch") ) == 0 )
				{
					//ENDSEARCH
					if ( m_hFind )
					{
						FindClose( m_hFind );
						m_hFind = NULL;
						SetEnvValue( _T("search_active"), NULL );
						SetEnvValue( _T("search_name"), NULL );
						SetEnvValue( _T("search_type"), NULL );
						SetEnvValue( _T("search_size"), NULL );
						SetEnvValue( _T("search_path"), NULL );
						SetEnvValue( _T("search_arg"), NULL );
					}
				}
				else if ( m_sCurrCmd.Compare( _T("beginsearch") ) == 0 )
				{
					//BEGINSEARCH <WildCardPath>
					if ( m_hFind )
					{
						SetError( _T("Search already in progress") );
					}
					else
					{
						CStdString sPath;

						m_sWildcard = MakeFullFilePath( m_sCurDir, GetArgument( 1 ) );
						m_sSearchPath = StripFilenameFromPath( m_sWildcard );
						SetEnvValue( _T("search_path"), m_sSearchPath );
						SetEnvValue( _T("search_arg"), m_sWildcard );
						// sPath = PathSlasher( m_sSearchPath, true );
						// sPath += _T("*");

						m_hFind = FindFirstFile( m_sWildcard, &m_ffData );
						if( INVALID_HANDLE_VALUE == m_hFind )
						{
							// Search didn't find anything
							m_hFind = NULL;
							SetEnvValue( _T("search_active"), NULL );
							SetEnvValue( _T("search_name"), NULL );
							SetEnvValue( _T("search_type"), NULL );
							SetEnvValue( _T("search_size"), NULL );
							SetEnvValue( _T("search_path"), NULL );
							SetEnvValue( _T("search_arg"), NULL );
						}
						else
						{
							SetEnvValue( _T("search_active"), _T("1"));
							SetEnvValue( _T("search_name"), m_ffData.cFileName );
							if ( m_ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
							{
								SetEnvValue( _T("search_type"), _T("dir"));
							}
							else
							{
								SetEnvValue( _T("search_type"), _T("file"));
							}
							{
								CStdString sTemp;
								ULARGE_INTEGER ulXlate;

								ulXlate.LowPart = m_ffData.nFileSizeLow;
								ulXlate.HighPart = m_ffData.nFileSizeHigh;

								sTemp.Format( _T("%I64d"), ulXlate.QuadPart );
								SetEnvValue( _T("search_size"), sTemp );
							}
						}
					}
				}

#if 0
//===================================================================================
//===================================================================================
//===================================================================================

			else
			{
				hr = ProcessScriptCommand( sLine, sResponse, NULL, PSC_GUI );
				iterProgram++;
			}
		}
		if ( m_bIsContext )
		{
			ContextExit();
		}
	}
//===================================================================================
//===================================================================================
//===================================================================================
//===================================================================================
//===================================================================================
#endif


				else
				{
					// OK, not one of our generic script commands
					// Try and execute it via the virtual function
					int iResult = ExecuteLine( m_sCurrCmd );
					if ( iResult != ASRC_OK )
					{
						switch( iResult )
						{
							case ASRC_NOTIMPL:
								SetError( _T("Command not implemented!") );
								break;
							case ASRC_EXECERROR:
								// Error should already be set
								break;
						}
					}
				}
			}
		}
	}


	if ( m_sLastError.GetLength() && m_bError )
	{
		GetStateApp()->MessageBox( _T("%s"), m_sLastError.c_str() );
		bReturn = false;
	}
	else
	{
		bReturn = true;
	}

	return bReturn;
}



CMXMFileData * GetFileData( TMapFileData & fileData, LPCTSTR szName )
{
	CMXMFileData * pReturn = NULL;

	if ( fileData.size() )
	{
		TMapFileData::iterator iterFileData;

		iterFileData = fileData.find( szName );
		if ( iterFileData != fileData.end() )
		{
			pReturn = &(iterFileData->second);
		}
	}
	return pReturn;
}

bool DeleteFileData( TMapFileData & fileData, LPCTSTR szName )
{
	bool bReturn = false;
	CMXMFileData * pReturn = NULL;

	if ( fileData.size() )
	{
		TMapFileData::iterator iterFileData;

		iterFileData = fileData.find( szName );
		if ( iterFileData != fileData.end() )
		{
			iterFileData->second.Close();
			fileData.erase( iterFileData );
			bReturn = true;
		}
	}
	return bReturn;
}

CMXMFileData * CreateFileData( TMapFileData & fileData, LPCTSTR szName )
{
	CMXMFileData * pReturn = NULL;

	if ( szName && _tcslen( szName ) && ( GetFileData( fileData, szName ) == NULL ) )
	{
		// Create a new entry!
		fileData[szName] = CMXMFileData();
		pReturn = GetFileData( fileData, szName );
	}
	return pReturn;
}

void CloseFileData( TMapFileData & fileData )
{
	if ( fileData.size() )
	{
		TMapFileData::iterator iterFileData;

		iterFileData = fileData.begin();
		while( iterFileData != fileData.end() )
		{
			iterFileData->second.Close();
			iterFileData++;
		}
	}
}
