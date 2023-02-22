

#pragma once
#include "StdString.h"
#include <list>

#define CADTYPE_IMAGE	1
#define CADTYPE_MSGBOX	2
#define CADTYPE_BOX		3
#define CADTYPE_TEXT	4
#define CADTYPE_LINE	5
#define CADTYPE_POLYGON	6

class CActionDraw
{
	int m_iType;
public:
	CActionDraw(int iType);
	CActionDraw(const CActionDraw &src );
	~CActionDraw();
	CActionDraw * GetPtr( void );
	virtual void Draw( void ) = 0;
};

typedef list<CActionDraw *> TActionDrawList;

void ClearActionDrawList( TActionDrawList & actionDrawList );
void DrawActionDrawList( TActionDrawList & actionDrawList );
void AddActionDrawList( TActionDrawList & actionDrawList, CActionDraw * pActionDraw, bool bCritical = true );

//#define CADTYPE_MSGBOX	2
//#define CADTYPE_BOX	3
//#define CADTYPE_TEXT	4
//#define CADTYPE_LINE	5
//#define CADTYPE_POLYGON	6


class CADText : public CActionDraw
{
	int m_iX;
	int m_iY;
	int m_dwJustify;
	DWORD m_dwColor;
	CStdString m_sMessage;
public:
	CADText( void );
	CADText( const CADText & src );
	void SetPos( int iX, int iY, DWORD dwJustify );
	void SetColor( DWORD dwColor );
	void SetText( LPCTSTR szMessage );
	virtual void Draw( void );
};


class CADBox : public CActionDraw
{
	int m_iX;
	int m_iY;
	int m_iW;
	int m_iH;
	DWORD m_dwColor;
	DWORD m_dwBorderColor;
public:
	CADBox( void );
	CADBox( const CADBox & src );
	void SetPos( int iX, int iY, int iW, int iH );
	void SetColor( DWORD dwColor, DWORD dwBorder = 0 );
	virtual void Draw( void );
};

class CADMsgBox : public CActionDraw
{
	CStdString m_sMessage;
public:
	CADMsgBox( void );
	CADMsgBox( const CADMsgBox & src );
	virtual void Draw( void );
	void SetMessage( LPCTSTR szMessage );
};

class CADImage : public CActionDraw
{
	LPDIRECT3DTEXTURE8    m_pTexture;
	bool           m_bTextureAttached;
	CImageSource * m_pImageSource;
	bool           m_bImageAttached;
	int            m_iX;
	int            m_iY;
	int            m_iW;
	int            m_iH;
public:
	CADImage( void );
	void SetImage( LPDIRECT3DTEXTURE8 pTexture = NULL, bool bAttach = true );
	void SetPos( int iX, int iY, int iW, int iH );
	void ClearResources( void );
	void SetImage( CImageSource * pImageSource = NULL );
	~CADImage();
	CADImage( const CADImage & src );
	virtual void Draw( void );
};





