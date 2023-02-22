

#include "stdafx.h"
#include "BasicHandler.h"

int variables[26]=
{    /* 26 user variables,  A-Z */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0
};

#if 0
// xxxx - Float
// xxxx% - Int
// xxxx$ - String

#define BVDT_INT		0
#define BVDT_FLOAT		1
#define BVDT_STRING		2

class CBasicVarData
{
	int			m_iVarType;
	PVOID 		m_pData;
	int			m_iNumDims;
	int	 		m_iaDimVals[8];
	int			SizeOf( void );
	PVOID		GetPointer( int iIndex );
	PVOID		GetLocation( int iIdx1=0, int iIdx2=-1, int iIdx3=-1, int iIdx4=-1, int iIdx5=-1, int iIdx6=-1, int iIdx7=-1, int iIdx8=-1 );
public:
	~CBasicVarData();

	CBasicVarData( LPCTSTR szName, LPCTSTR szValue = NULL );
	int GetType( void );
	virtual bool IsNumeric( void );
	virtual CString GetString( void );
	virtual void SetValue( LPCTSTR szValue );
	bool	Dim(  int iIdx1=1, int iIdx2=-1, int iIdx3=-1, int iIdx4=-1, int iIdx5=-1, int iIdx6=-1, int iIdx7=-1, int iIdx8=-1 );
};

CBasicVarData::CBasicVarData() :
	m_pData(NULL),
	m_iVarType(0)
{
	int iIndex;
	m_iNumDims = 0;
	for( iIndex = 0; iIndex<8; iIndex++ )
	{
		m_iaDimVals[iIndex] = 0;
	}
}

PVOID CBasicVarData::GetPointer( int iIndex )
{
	PVOID pReturn = m_pData;

	switch( m_iVarType )
	{
		case BVDT_INT:
			pReturn = &((int *)m_pData)[iIndex];
			break;
		case BVDT_FLOAT:
			pReturn = &((double *)m_pData)[iIndex];
			break;
		case BVDT_STRING:
			pReturn = &((CString *)m_pData)[iIndex];
			break;
	}

	return pReturn;
}

bool CBasicVarData::Dim(  int iIdx1, int iIdx2, int iIdx3, int iIdx4, int iIdx5, int iIdx6, int iIdx7, int iIdx8 )
{
	bool bReturn = false;
	int iNumElements;

	if ( !m_pData && iIdx1 > 0 )
	{
		for( iIndex = 0; iIndex<8; iIndex++ )
		{
			m_iaDimVals[iIndex] = 0;
		}
		m_iaDimVals[0] = iIdx1;
		iNumElements = iIdx1;
		if ( iIdx2 > 0 )
		{
			m_iaDimVals[1] = iIdx2;
			iNumElements *= iIdx2;
			if ( iIdx3 > 0 )
			{
				m_iaDimVals[2] = iIdx3;
				iNumElements *= iIdx3;
				if ( iIdx4 > 0 )
				{
					m_iaDimVals[3] = iIdx4;
					iNumElements *= iIdx4;
					if ( iIdx5 > 0 )
					{
						m_iaDimVals[4] = iIdx5;
						iNumElements *= iIdx5;
						if ( iIdx6 > 0 )
						{
							m_iaDimVals[5] = iIdx6;
							iNumElements *= iIdx6;
							if ( iIdx7 > 0 )
							{
								m_iaDimVals[6] = iIdx7;
								iNumElements *= iIdx7;
								if ( iIdx8 > 0 )
								{
									m_iaDimVals[7] = iIdx8;
									iNumElements *= iIdx8;
								}
							}
						}
					}
				}
			}
		}
	}
	m_pData = NULL;
	switch( m_iVarType )
	{
		case BVDT_INT:
			m_pData = new int[iNumElements];
			break;
		case BVDT_FLOAT:
			m_pData = new double[iNumElements];
			break;
		case BVDT_STRING:
			m_pData = new CString[iNumElements];
			break;
	}
	if ( m_pData )
	{
		bReturn = true;
	}

	return bReturn;
}


int	CBasicVarData::SizeOf( void )
{
	int iReturn = 0;
	switch( m_iVarType )
	{
		case BVDT_INT:
			iReturn = sizeof( int );
			break;
		case BVDT_FLOAT:
			iReturn = sizeof( double );
			break;
		case BVDT_STRING:
			iReturn = sizeof( CString );
			break;
	}
	return iReturn;
}

PVOID CBasicVarData::GetLocation( int iIdx1, int iIdx2, int iIdx3, int iIdx4, int iIdx5, int iIdx6, int iIdx7, int iIdx8 )
{
	PVOID pReturn = m_pData;

	switch( m_iNumDims )
	{
		case 0: // Invalid!! Default to 0
			break;
		case 1: //
			pReturn = GetPointer( iIdx1 );
			break;
		case 2: //
			pReturn = GetPointer( (iIdx1*m_iaDimVals[0])+iIdx2 );
			break;
		case 3: //
			pReturn = GetPointer( (iIdx1*m_iaDimVals[0])+(iIdx2*m_iaDimVals[1])+(iIdex3) );
			break;
		case 4: //
			pReturn = GetPointer( (iIdx1*m_iaDimVals[0])+(iIdx2*m_iaDimVals[1])+(iIdex3*m_iaDimVals[2])+(iIdex4) );
			break;
		case 5: //
			pReturn = GetPointer( (iIdx1*m_iaDimVals[0])+(iIdx2*m_iaDimVals[1])+(iIdex3*m_iaDimVals[2])+(iIdex4*m_iaDimVals[3])+(iIdex5) );
			break;
		case 6: //
			pReturn = GetPointer( (iIdx1*m_iaDimVals[0])+(iIdx2*m_iaDimVals[1])+(iIdex3*m_iaDimVals[2])+(iIdex4*m_iaDimVals[3])+(iIdex5*m_iaDimVals[4])+(iIdex6) );
			break;
		case 7: //
			pReturn = GetPointer( (iIdx1*m_iaDimVals[0])+(iIdx2*m_iaDimVals[1])+(iIdex3*m_iaDimVals[2])+(iIdex4*m_iaDimVals[3])+(iIdex5*m_iaDimVals[4])+(iIdex6*m_iaDimVals[5])+(iIdex7) );
			break;
		case 8: //
			pReturn = GetPointer( (iIdx1*m_iaDimVals[0])+(iIdx2*m_iaDimVals[1])+(iIdex3*m_iaDimVals[2])+(iIdex4*m_iaDimVals[3])+(iIdex5*m_iaDimVals[4])+(iIdex6*m_iaDimVals[5])+(iIdex7*m_iaDimVals[6])+(iIdex8) );
			break;
	}
	return pReturn;
}


CBasicVarData::~CBasicVarData()
{
	if ( m_pData )
	{
		delete [] m_pData;
	}
}

class CBasicVariables
{
public:
	CBasicVariables();
	~CBasicVariables();
	CBasicVarData * GetVarData( LPCTSTR szToken );
	CBasicVarData * AddVarData( LPCTSTR szName, LPCTSTR szValue );
}

#endif

struct TCommands CBasicHandler::s_CmdTokenTable[] = 
{ /* Commands must be entered lowercase */
  { _T("print"),	KEYWORD_PRINT }, /* in this table. */
  { _T("input"),	KEYWORD_INPUT },
  { _T("if"),		KEYWORD_IF },
  { _T("then"),		KEYWORD_THEN },
  { _T("goto"),		KEYWORD_GOTO },
  { _T("for"),		KEYWORD_FOR },
  { _T("next"),		KEYWORD_NEXT },
  { _T("to"),		KEYWORD_TO },
  { _T("gosub"),	KEYWORD_GOSUB },
  { _T("return"),	KEYWORD_RETURN },
  { _T("end"),		KEYWORD_END },
  { _T(""),			KEYWORD_END  } /* mark end of table */
};



void CBasicHandler::RunScript( LPCTSTR szScript, bool bKeepLastData )
{
	m_iProgPos = 0;

	m_sProgram = szScript;

	ScanForLabels(); /* find the labels in the program */
	m_iForStackTop = 0; /* initialize the FOR stack index */
	m_iGoSubStackTop = 0; /* initialize the GOSUB stack index */
	do
	{
		m_iTokenType = GetToken();
		/* check for assignment statement */
		if(m_iTokenType==TOKEN_VARIABLE)
		{
			PutBack(); /* return the var to the input stream */
			DoAssignment(); /* must be assignment statement */
		}
		else /* is m_szCommand */
		{
			switch(m_iTokenValue)
			{
				case KEYWORD_PRINT:
					DoPrint();
					break;
				case KEYWORD_GOTO:
					DoGoto();
					break;
				case KEYWORD_IF:
					DoIf();
					break;
				case KEYWORD_FOR:
					DoFor();
					break;
				case KEYWORD_NEXT:
					DoNext();
					break;
				case KEYWORD_INPUT:
					DoInput();
					break;
				case KEYWORD_GOSUB:
					DoGosub();
					break;
				case KEYWORD_RETURN:
					DoReturn();
					break;
				case KEYWORD_END:
					DoEnd(0);
			}
		}
	} while ( m_iTokenValue != KEYWORD_FINISHED );

}

CBasicHandler::CBasicHandler( void )
{
	ZeroMemory( m_szToken, 80*sizeof(TCHAR) );
	m_iProgPos = 0;
	m_iForStackTop = 0;
	m_iGoSubStackTop = 0;
	m_iTokenType = 0;
	m_iTokenValue = 0;

}

void CBasicHandler::DoEnd( int iResult )
{
}

/* Return a token to input stream. */
void CBasicHandler::PutBack(void)
{
	TCHAR *t; 

	t = m_szToken; 
	for(; *t; t++)
	{
		m_iProgPos--;
	}
}

/* Get a token. */
int CBasicHandler::GetToken(void)
{
	TCHAR *temp;
	int iReturn = TOKEN_DELIMITER;

	m_iTokenType=0; m_iTokenValue=0;
	temp=m_szToken;

	if( m_iProgPos >= m_sProgram.GetLength() )
	{ 
		/* end of file */
		m_szToken[0]=0;
		m_iTokenValue = KEYWORD_FINISHED;
		m_iTokenType=TOKEN_DELIMITER;
	}
	else
	{
		while(IsWhite(m_sProgram[m_iProgPos]))
		{
			++m_iProgPos;  /* skip over white space */
		}

		if(m_sProgram[m_iProgPos]==_T('\r')) 
		{ 
			/* crlf */
			++m_iProgPos;
			++m_iProgPos;

			m_iTokenValue = KEYWORD_EOL; 

			m_szToken[0]=_T('\r');
			m_szToken[1]=_T('\n');
			m_szToken[2]=0;

			m_iTokenType = TOKEN_DELIMITER;
		}
		else if(_tcschr(_T("+-*^/%=;(),><"), m_sProgram[m_iProgPos]))
		{	
			/* delimiter */
			m_szToken[0] = m_sProgram[m_iProgPos];
			m_szToken[1] = 0; 

			m_iProgPos++; /* advance to next position */

			m_iTokenType = TOKEN_DELIMITER;
		}
    	else if(m_sProgram[m_iProgPos]==_T('"'))
		{	
			/* quoted string */
			m_iProgPos++;
			while( m_iProgPos<m_sProgram.GetLength() && m_sProgram[m_iProgPos]!=_T('"') && m_sProgram[m_iProgPos]!=_T('\r') )
			{
				*temp++ = m_sProgram[m_iProgPos++];
			}
			if( m_iProgPos>=m_sProgram.GetLength() || m_sProgram[m_iProgPos]==_T('\r') )
			{
				serror(1);
			}
			m_iProgPos++;
			*temp=0;
			m_iTokenType=TOKEN_QUOTE;
		}
  		else if( _istdigit(m_sProgram[m_iProgPos]) )
		{   
			/* number */
			while( m_iProgPos<m_sProgram.GetLength() && !IsDelim(m_sProgram[m_iProgPos]) )
			{
				*temp++=m_sProgram[m_iProgPos++];
			}
			*temp = _T('\0');
			m_iTokenType = TOKEN_NUMBER;
		}
		else 
		{
			if(_istalpha(m_sProgram[m_iProgPos]))
			{   
				/* var or command */
				while(!IsDelim(m_sProgram[m_iProgPos]))
				{
					*temp++=m_sProgram[m_iProgPos++];
				}
				m_iTokenType=TOKEN_STRING;

				*temp = _T('\0');

				/* see if a string is a command or a variable */
				if(m_iTokenType==TOKEN_STRING)
				{
					m_iTokenValue=LookUp(m_szToken); /* convert to internal rep */
					if(!m_iTokenValue)
					{
						m_iTokenType = TOKEN_VARIABLE;
					}
					else
					{
						m_iTokenType = TOKEN_COMMAND; /* is a command */
					}
				}
			}
		}
	}
	return m_iTokenType;
}


/* Assign a variable a value. */
void CBasicHandler::DoAssignment(void)
{
	int var, value;

	/* get the variable name */
	GetToken();
	if(!_istalpha(m_szToken[0]))
	{
		serror(4);
		return;
	}

	var = _totupper(m_szToken[0])-_T('A');
 
	/* get the equals sign */
	GetToken();
	if(m_szToken[0]!=_T('='))
	{
		serror(3);
		return;
	}

	/* get the value to assign to var */
	GetNumericExp(&value);

	/* assign the value */
	variables[var] = value;
}

/* Entry point into parser. */
void CBasicHandler::GetNumericExp(int *result)
{
	GetToken();
	if(!m_szToken[0])
	{
		serror(2);
		return;
	}
	CalcLevel2(result);
	PutBack(); /* return last token read to input stream */
}


/*  Add or subtract two terms. */
void CBasicHandler::CalcLevel2(int *result)
{
	TCHAR tcOp; 
	int hold; 

	CalcLevel3(result); 
	while((tcOp = m_szToken[0]) == _T('+') || tcOp == _T('-'))
	{
		GetToken(); 
		CalcLevel3(&hold); 
		PerformArith(tcOp, result, &hold);
	}
}

/* Multiply or divide two factors. */
void CBasicHandler::CalcLevel3(int *result)
{
	TCHAR  tcOp; 
	int hold;

	CalcLevel4(result); 
	while((tcOp = m_szToken[0]) == _T('*') || tcOp == _T('/') || tcOp == _T('%'))
	{
		GetToken(); 
		CalcLevel4(&hold); 
		PerformArith(tcOp, result, &hold); 
	}
}

/* Process integer exponent. */
void CBasicHandler::CalcLevel4(int *result)
{
	int hold; 

	CalcLevel5(result); 
	if(m_szToken[0]== _T('^'))
	{
		GetToken(); 
		CalcLevel4(&hold); 
		PerformArith(_T('^'), result, &hold); 
	}
}

/* Is a unary + or -. */
void CBasicHandler::CalcLevel5(int *result)
{
	TCHAR tcOp; 

	tcOp = 0; 
	if((m_iTokenType==TOKEN_DELIMITER) && m_szToken[0]==_T('+') || m_szToken[0]==_T('-'))
	{
		tcOp = m_szToken[0];
		GetToken(); 
	}
	CalcLevel6(result); 
	if(tcOp)
	{
		PerformUnary(tcOp, result); 
	}
}

/* Process parenthesized expression. */
void CBasicHandler::CalcLevel6(int *result)
{
	if((m_szToken[0] == _T('(') ) && (m_iTokenType == TOKEN_DELIMITER))
	{
		GetToken(); 
		CalcLevel2(result); 
		if(m_szToken[0] != _T(')') )
		{
			serror(1);
		}
		GetToken(); 
	}
	else
	{
		FetchPrimitive(result);
	}
}

/* Find value of number or variable. */
void CBasicHandler::FetchPrimitive(int *result)
{
	switch(m_iTokenType)
	{
		case TOKEN_VARIABLE:
			*result = FindVar(m_szToken);
			GetToken(); 
			return; 
		case TOKEN_NUMBER:
			*result = _ttoi(m_szToken);
			GetToken();
			return;
		default:
			serror(0);
	}
}

/* Perform the specified arithmetic. */
void CBasicHandler::PerformArith(TCHAR o, int *r, int *h)
{
	register int t, ex;

	switch(o) 
	{
		case _T('-'):
			*r = *r-*h; 
			break; 
		case _T('+'):
			*r = *r+*h; 
			break; 
		case _T('*'):  
			*r = *r * *h; 
			break; 
		case _T('/'):
			*r = (*r)/(*h);
			break; 
		case _T('%'):
			t = (*r)/(*h); 
			*r = *r-(t*(*h)); 
			break; 
		case _T('^'):
			ex = *r; 
			if(*h==0)
			{
				*r = 1; 
				break; 
			}
			for(t=*h-1; t>0; --t)
			{
				*r = (*r) * ex;
			}
			break;       
	}
}

/* Reverse the sign. */
void CBasicHandler::PerformUnary(TCHAR o, int *r)
{
	if(o==_T('-'))
	{
		*r = -(*r);
	}
}

/* Find the value of a variable. */
int CBasicHandler::FindVar(TCHAR *s)
{
	if(!_istalpha(*s))
	{
		serror(4); /* not a variable */
		return 0;
	}
	return variables[_totupper(m_szToken[0])-'A'];
}


/* Look up a a token's internal representation in the
   token table.
*/
int CBasicHandler::LookUp(TCHAR *s)
{
	int i; // ,j;
//	TCHAR *p;

	/* convert to lowercase */
//	p = s;
//	while(*p)
//	{ 
//		*p = _totlower(*p);
//		p++;
//	}

	/* see if token is in table */
	for(i=0; *s_CmdTokenTable[i].m_szCommand; i++)
	{
		if(!_tcsicmp(s_CmdTokenTable[i].m_szCommand, s)) 
		{
			return s_CmdTokenTable[i].m_iTokenID;
		}
	}
	return 0; /* unknown command */
}

/* Return true if c is a delimiter. */
bool CBasicHandler::IsDelim(TCHAR tc)
{
	if(_tcschr( _T(" ;,+-<>/*%^=()"), tc) || tc==9 || tc==_T('\r') || tc==0) 
	{
		return true;
	}
	return false;
}

/* Return 1 if c is space or tab. */
bool CBasicHandler::IsWhite( TCHAR tc)
{
	if( tc==_T(' ') || tc==_T('\t') )
	{
		return true;
	}
	else
	{
		return false;
	}
}


/* Find all labels. */

/* Find the start of the next line. */
void CBasicHandler::FindEOL(void)
{
	while(m_iProgPos<m_sProgram.GetLength() && m_sProgram[m_iProgPos]!=_T('\n'))
	{
		++m_iProgPos;
	}
	if(m_iProgPos<m_sProgram.GetLength() && m_sProgram[m_iProgPos])
	{
		m_iProgPos++;
	}
}

/* Return index of next free position in label array. 
   A -1 is returned if the array is full.
   A -2 is returned when duplicate label is found.
*/
int CBasicHandler::GetNextLabel(TCHAR *s)
{
	int t;

	for(t=0;t<NUM_LAB;++t)
	{
		if( label_table[t].m_sName.GetLength()==0)
		{
			return t;
		}
		if(!_tcsicmp(label_table[t].m_sName,s))
		{
			return -2; /* dup */
		}
	}

	return -1;
}

void		CBasicHandler::ScanForLabels( void )
{
	int addr;
	int iPlaceHolder = m_iProgPos;

	LabelInit();  /* zero all labels */
	// temp = prog;   /* save pointer to top of program */

	/* if the first token in the file is a label */
	GetToken();
	if(m_iTokenType==TOKEN_NUMBER)
	{
		label_table[0].m_sName = m_szToken;
		label_table[0].m_iLabelPos = m_iProgPos;
	}

	FindEOL();
	do
	{
		GetToken();
		if(m_iTokenType==TOKEN_NUMBER)
		{
			addr = GetNextLabel(m_szToken);
			if(addr==-1 || addr==-2)
			{
				(addr==-1) ?serror(5):serror(6);
			}
			label_table[addr].m_sName = m_szToken;
			label_table[addr].m_iLabelPos = m_iProgPos;  /* current point in program */
		}
		/* if not on a blank line, find next line */
		if(m_iTokenValue!=KEYWORD_EOL)
		{
			FindEOL();
		}
	} while(m_iTokenValue!=KEYWORD_FINISHED);
	m_iProgPos = iPlaceHolder;
}

void		CBasicHandler::DoPrint( void )
{
	int answer;
	int len=0, spaces;
	TCHAR last_delim;
	CString sTemp;

	do
	{
		GetToken(); /* get next list item */

		if(	m_iTokenValue == KEYWORD_EOL || m_iTokenValue == KEYWORD_FINISHED )
		{
			break;
		}
		if( m_iTokenType == TOKEN_QUOTE )
		{	/* is string */
			m_sResult += m_szToken;
			// printf(szToken);
			len += _tcslen(m_szToken);
			GetToken();
		}
		else
		{ /* is expression */
			PutBack();
			GetNumericExp(&answer);
			GetToken();
			sTemp.Format( _T("%d"), answer );
			m_sResult += sTemp;
			len += sTemp.GetLength();
		}
	    last_delim = m_szToken[0];

		if(m_szToken[0]==_T(';'))
		{
			/* compute number of spaces to move to next tab */
			spaces = 8 - (len % 8);
			len += spaces; /* add in the tabbing position */
			while(spaces)
			{
				m_sResult += _T(" ");
				// printf(" ");
				spaces--;
			}
		}
		else if(m_szToken[0]==_T(',')) /* do nothing */;
		else if(m_iTokenValue!=KEYWORD_EOL && m_iTokenValue!=KEYWORD_FINISHED)
		{
			serror(0);
		}
	} while (m_szToken[0]==_T(';') || m_szToken[0]==_T(','));

	if(m_iTokenValue==KEYWORD_EOL || m_iTokenValue==KEYWORD_FINISHED)
	{
		if(last_delim != _T(';') && last_delim!=_T(','))
		{
			m_sResult += _T("\r\n");
			// printf("\n");
		}
	}
	else
	{
		serror(0); /* error is not , or ; */
	}
}

void		CBasicHandler::DoGoto( void )
{
	// TCHAR *loc;
	int iPos;

	GetToken(); /* get label to go to */
	/* find the location of the label */
	iPos = FindLabel(m_szToken);
	if(m_sProgram[iPos]=='\0')
	{
		serror(7); /* label not defined */
	}
	else m_iProgPos = iPos;
	// else prog=loc;  /* start program running at that loc */
}

void		CBasicHandler::DoIf( void )
{
	int x , y, cond;
	char op;

	GetNumericExp(&x); /* get left expression */

	GetToken(); /* get the operator */
	if(!strchr("=<>", m_szToken[0]))
	{
		serror(0); /* not a legal operator */
		return;
	}
	op=m_szToken[0];

	GetNumericExp(&y); /* get right expression */

	/* determine the outcome */
	cond = 0;
	switch(op)
	{
		case '<':
			if(x<y) cond=1;
			break;
		case '>':
			if(x>y) cond=1;
			break;
		case '=':
			if(x==y) cond=1;
			break;
	}
	if(cond)
	{ /* is true so process target of IF */
		GetToken();
		if(m_iTokenValue!=KEYWORD_THEN)
		{
			serror(8);
			return;
		}/* else program execution starts on next line */
	}
	else FindEOL(); /* find start of next line */


}

void		CBasicHandler::DoFor( void )
{
	struct for_stack i;
	int value;

	GetToken(); /* read the control variable */
	if(!_istalpha(m_szToken[0]))
	{
		serror(4);
		return;
	}

	i.var=_totupper(m_szToken[0])-'A'; /* save its index */

	GetToken(); /* read the equals sign */
	if(m_szToken[0]!='=')
	{
		serror(3);
		return;
	}

	GetNumericExp(&value); /* get initial value */

	variables[i.var]=value;

	GetToken();
	if(m_iTokenValue!=KEYWORD_TO) serror(9); /* read and discard the TO */

	GetNumericExp(&i.target); /* get target value */

	/* if loop can execute at least once, push info on stack */
	if(value>=variables[i.var])
	{
		i.m_iPos = m_iProgPos;
		// i.loc = m_sProgram[m_iProgPos];
		fpush(i);
	}
	else  /* otherwise, skip loop code altogether */
	{
		while(m_iTokenValue!=KEYWORD_NEXT)
		{
			GetToken();
		}
	}


}

/* Push function for the FOR stack. */
void CBasicHandler::fpush(struct for_stack i)
{
	if(m_iForStackTop>FOR_NEST)
	{
		serror(10);
	}

	m_fstack[m_iForStackTop]=i;
	m_iForStackTop++;
}

struct for_stack CBasicHandler::fpop()
{
	m_iForStackTop--;
	if(m_iForStackTop<0)
	{
		serror(11);
	}
	return(m_fstack[m_iForStackTop]);
}


void		CBasicHandler::DoNext( void )
{
	struct for_stack i;

	i = fpop(); /* read the loop info */

	variables[i.var]++; /* increment control variable */
	if(variables[i.var]>i.target)
	{
		return;  /* all done */
	}
	fpush(i);  /* otherwise, restore the info */
	m_iProgPos = i.m_iPos;
	// prog = i.loc;  /* loop */
}

void		CBasicHandler::DoInput( void )
{
}

void		CBasicHandler::DoGosub( void )
{
//	char *loc;
	int iPos;

	GetToken();
	/* find the label to call */
	iPos = FindLabel(m_szToken);
	// loc = find_label(szToken);
	if(m_sProgram[iPos]=='\0')
	{
		serror(7); /* label not defined */
	}
	else
	{
		gpush(m_iProgPos);
		// gpush(prog); /* save place to return to */
		m_iProgPos = iPos;
		// prog = loc;  /* start program running at that loc */
	}
}

/* GOSUB stack push function. */
void CBasicHandler::gpush(int iPos)
{
	m_iGoSubStackTop++;

	if(m_iGoSubStackTop==SUB_NEST)
	{
		serror(12);
		return;
	}

	m_gstack[m_iGoSubStackTop]=iPos;
}


/* GOSUB stack pop function. */
int CBasicHandler::gpop(void)
{
	if(m_iGoSubStackTop==0)
	{
		serror(13);
		return 0;
	}

	return(m_gstack[m_iGoSubStackTop--]);
}

void		CBasicHandler::DoReturn( void )
{
	m_iProgPos = gpop();
}



/* Find location of given label.  A null is returned if
   label is not found; otherwise a pointer to the position
   of the label is returned.
*/
int CBasicHandler::FindLabel(TCHAR *s)
{
	int t;

	for(t=0; t<NUM_LAB; ++t)
	{
		if(!_tcsicmp(label_table[t].m_sName,s))
		{
			return label_table[t].m_iLabelPos;
		}
	}
	return '\0'; /* error condition */
}
