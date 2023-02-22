


/*

Title Screen



X-Marbles

A game by MasterMind

Version 0.5 (August 12, 2002)



Configuration:

Difficulty:
Easy
Normal
Hard

(More to come later)


High Scores:

  [HIGH SCORES]
    **NORMAL**

ZZZ          99999
XXX          99999
AAA          99999
...          88888



  On game start up:

  Load High Scores


  When high score is achieved, Save High Scores

  Display high scores for difficulty level (Up/Down switches difficulty score display)

  When entering, Insert score and display, then edit at position?


*/


#include <XBApp.h>
#include <XBFont.h>
#include <XBMesh.h>
#include <XBUtil.h>
#include <XBResource.h>
#include <xgraphics.h>
#include "score.h"


CHighScores::CHighScores()
{
	int iIndex;
	m_iChanges = 0;
	for( iIndex=0; iIndex<10; iIndex++ )
	{
		m_szNames[iIndex] = strdup( "Lee Sr" );
		m_iaScores[iIndex] = 0;
	}
}

CHighScores::~CHighScores()
{
	ClearList();
	if ( m_szLastName )
	{
		delete [] m_szLastName;
	}
}

BOOL CHighScores::IsOnList( int iScore )
{
	return (GetInsertPosition( iScore )==(-1))?FALSE:TRUE;
}

int CHighScores::AddScoreToList( int iScore, char * szName )
{
	int iIndex;
	int iReturn = GetInsertPosition( iScore );
	if ( iReturn != -1 )
	{
		// Insert into position
		// Everything iReturn and below must get moved down.
		// First thing's first, delete last position
		if ( m_szNames[9] )
		{
			delete [] m_szNames[9];
			m_szNames[9] = NULL;
		}
		m_iaScores[9] = 0;

		// Now, move the rest down one
		for( iIndex=iReturn; iIndex<9; iIndex++ )
		{
			m_szNames[iIndex+1] = m_szNames[iIndex];
			m_iaScores[iIndex+1] = m_iaScores[iIndex];
			m_szNames[iIndex] = NULL;
			m_iaScores[iIndex] = 0;
		}
		SetName( iReturn, szName );
		m_iaScores[iReturn] = iScore;
		m_iChanges++;
	}
	return iReturn;
}

int ReadFileString( HANDLE hFile, char * szBuffer, int iMaxChars )
{
	int iIndex = 0;
	DWORD dwRead;

	if (szBuffer && hFile && iMaxChars )
	{
		szBuffer[0] = 0;
		while( iMaxChars-- )
		{
			ReadFile( hFile, (LPVOID)&szBuffer[iIndex], 1, &dwRead, NULL);
			if ( dwRead == 0 )
			{
				szBuffer[iIndex] = 0;
			}
			if ( szBuffer[iIndex] == 0 )
			{
				break;
			}
			else
			{
				iIndex++;
			}
		}
	}
	return iIndex;
}

void CHighScores::LoadScores( char * szDifficulty )
{
	HANDLE hFile;
	char szFilePath[256];
	DWORD dwRead;
	int iIndex;

	sprintf( szFilePath, "T:\\Scores_%s.dat", szDifficulty );
	hFile = CreateFile( szFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		ClearList();
		// Save file!!
		ReadFile( hFile, &m_iaScores[0], sizeof(int)*10, &dwRead, NULL);
		for( iIndex=0; iIndex<10; iIndex++ )
		{
			if ( ReadFileString( hFile, szFilePath, 255 ) )
			{
				m_szNames[iIndex] = strdup(szFilePath);
			}
			else
			{
				m_szNames[iIndex] = strdup("???");
			}
		}
		if ( ReadFileString( hFile, szFilePath, 255 ) )
		{
			m_szLastName = strdup(szFilePath);
		}
		else
		{
			m_szLastName = strdup("???");
		}
		CloseHandle( hFile );
	}
	// Else, fresh list!
}

void CHighScores::SaveScores( char * szDifficulty )
{
//    char szSavePath[256];
//	DWORD dwSaveGame = XCreateSaveGame( "E:\\", L"X-Marbles",OPEN_ALWAYS, 0, szSavePath, 256 );
	HANDLE hFile;
	char szFilePath[256];
	DWORD dwWritten;
	int iIndex;

	sprintf( szFilePath, "T:\\Scores_%s.dat", szDifficulty );
	hFile = CreateFile( szFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		// Save file!!
		WriteFile( hFile, &m_iaScores[0], sizeof(int)*10, &dwWritten, NULL);
		for( iIndex=0; iIndex<10; iIndex++ )
		{
			if ( m_szNames[iIndex] )
			{
				WriteFile( hFile, m_szNames[iIndex], strlen( m_szNames[iIndex] )+1, &dwWritten, NULL);
			}
			else
			{
				szFilePath[0] = 0;
				WriteFile( hFile, szFilePath, 1, &dwWritten, NULL);
			}
		}
		if ( m_szLastName )
		{
			WriteFile( hFile, m_szLastName, strlen( m_szLastName )+1, &dwWritten, NULL);
		}
		else
		{
			szFilePath[0] = 0;
			WriteFile( hFile, szFilePath, 1, &dwWritten, NULL);
		}
		CloseHandle( hFile );
	}
}

void CHighScores::SetName( int iPos, char * szName )
{
	if ( m_szNames[iPos] )
	{
		delete [] m_szNames[iPos];
		m_szNames[iPos] = NULL;
	}
	// Finally, insert our new info.
	if ( szName )
	{
		m_szNames[iPos] = strdup( szName );
		SetLastName( szName );
	}
	else
	{
		m_szNames[iPos] = strdup( "..." );
	}
}

int  CHighScores::GetScore( int iPos )
{
	int iReturn = 0;

	if ( iPos >= 0 && iPos < 10 )
	{
		iReturn = m_iaScores[iPos];
	}
	return iReturn;
}

char * CHighScores::GetName( int iPos )
{
	char * szReturn = NULL;

	if ( iPos >= 0 && iPos < 10 )
	{
		szReturn = m_szNames[iPos];
	}
	return szReturn;
}


void CHighScores::ClearList( void )
{
	int iIndex;

	for( iIndex=0; iIndex<10; iIndex++ )
	{
		if ( m_szNames[iIndex] )
		{
			delete [] m_szNames[iIndex];
		}
		m_szNames[iIndex] = NULL;;
		m_iaScores[iIndex] = 0;
	}
}


void CHighScores::SetLastName( char * szName )
{
	if ( szName )
	{
		if ( m_szLastName )
		{
			delete [] m_szLastName;
			m_szLastName = strdup( szName );
		}
	}
}

char * CHighScores::GetLastName( void )
{
	return m_szLastName;
}

int CHighScores::GetInsertPosition( int iScore )
{
	int iIndex;
	int iReturn = -1;

	for( iIndex=0; iIndex<10; iIndex++ )
	{
		if ( ( m_szNames[iIndex] == NULL ) || (iScore > m_iaScores[iIndex]) )
		{
			// Insert score here!
			iReturn = iIndex;
			break;
		}
	}
	return iReturn;
}





/*


Tutorial Subjects:

  The XDK Framework and "Hello World!"
  Controller input
  Displaying a bitmap
  A simple menu
  Playing a sound
  Saving and loading configuration data
  Saving and loading save game data
  





  */


