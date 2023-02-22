/*********************************************************************************\
 * Tokens.h
 * Used to convert ASCII strings into token values
 * Includes support for sets of tokens 
 *
 * (C) 2002 MasterMind
\*********************************************************************************/


#ifndef __SKIN_H__
#define __SKIN_H__

#include "StdString.h"


#define		TKNSET_MAIN		0

#define		TKNMN_TITLE		1
#define		TKNMN_MENU		2
#define		TKNMN_HELP		3
#define		TKNMN_THUMBNAIL	4
#define		TKNMN_LOADING	5


long GetToken( LPCTSTR szTokenStr, int iSet = 0 );
CStdString GetTokenString( long lToken, int iSet = 0 );



#endif //  __SKIN_H__
