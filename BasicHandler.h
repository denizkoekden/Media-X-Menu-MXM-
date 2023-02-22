

#ifndef __BASICHANDLER_H__
#define __BASICHANDLER_H__

// #include "StdString.h"

#define NUM_LAB 100
#define LAB_LEN 10 
#define FOR_NEST 25
#define SUB_NEST 25
#define PROG_SIZE 10000

#define TOKEN_DELIMITER		1
#define TOKEN_VARIABLE		2
#define TOKEN_NUMBER		3
#define TOKEN_COMMAND		4
#define TOKEN_STRING		5
#define TOKEN_QUOTE			6

#define KEYWORD_PRINT		1
#define KEYWORD_INPUT		2
#define KEYWORD_IF			3
#define KEYWORD_THEN		4
#define KEYWORD_FOR			5
#define KEYWORD_NEXT		6
#define KEYWORD_TO			7
#define KEYWORD_GOTO		8
#define KEYWORD_EOL			9
#define KEYWORD_FINISHED	10
#define KEYWORD_GOSUB		11
#define KEYWORD_RETURN		12
#define KEYWORD_END			13

struct for_stack
{
	int var; /* counter variable */
	int target;  /* target value */
	int m_iPos;
	// char *loc;
};

struct TCommands 
{ /* keyword lookup table */
  LPCTSTR	m_szCommand;
  int		m_iTokenID;
};


// Basic handler has several enhancements...
// 1. It has persistent storage. The variables formt he last are always available
//    to the next execution script - essentially, apps can be chained.
//    The mechanism will be a value fed in at run time
// 2. Handle basic strings
// 3. Add functions

struct label
{
	CString m_sName;
	int		m_iLabelPos;
//	char name[LAB_LEN];
//	char *p;  /* points to place to go in source file*/
};




class CBasicHandler
{
	TCHAR		m_szToken[80];
	CString		m_sProgram;
	int			m_iProgPos;
	int			m_iForStackTop;
	int			m_iGoSubStackTop;
	int			m_iTokenType;
	int			m_iTokenValue;
	struct label label_table[NUM_LAB];
	struct for_stack m_fstack[FOR_NEST]; /* stack for FOR/NEXT loop */
	int			m_gstack[SUB_NEST];
	struct	for_stack fpop();
	void		gpush(int iPos);
	int			gpop(void);
	void		fpush(struct for_stack i);

	int			GetToken( void );
	void		PutBack(void);
	void		ScanForLabels( void );
	void		DoPrint( void );
	void		DoGoto( void );
	void		DoIf( void );
	void		DoFor( void );
	void		DoNext( void );
	void		DoInput( void );
	void		DoGosub( void );
	void		DoReturn( void );
	void		DoEnd( int iResult );
	void 		DoAssignment(void);
	bool		IsWhite( TCHAR tc);
	bool		IsDelim(TCHAR tc);
	int			LookUp(TCHAR *s);
	void		GetNumericExp(int *result);
	void		CalcLevel2(int *result);
	void		CalcLevel3(int *result);
	void		CalcLevel4(int *result);
	void		CalcLevel5(int *result);
	void		CalcLevel6(int *result);
	void		FetchPrimitive(int *result);
	void		PerformArith(TCHAR o, int *r, int *h);
	void		PerformUnary(TCHAR o, int *r);
	int			FindVar(TCHAR *s);
	void		FindEOL(void);
	int			GetNextLabel(TCHAR *s);
	int			FindLabel(TCHAR *s);

	void		serror( int iError ) {};
	void		LabelInit( void ) {};
public:
	CBasicHandler();
	~CBasicHandler() {};
//	CBasicHandler( LPCTSTR szScript, bool bKeepLastData );
	void RunScript( LPCTSTR szScript, bool bKeepLastData );
	CString m_sResult;

	static struct TCommands s_CmdTokenTable[];
//	char token[80];
//	char token_type, tok;
};


#endif //  __BASICHANDLER_H__
