/*********************************************************************************\
 * xmltree.h
 * Support for XML tree nodes and elements 
 * (C) 2002 MasterMind
\*********************************************************************************/


#ifndef __XMLTREE_H__
#define __XMLTREE_H__

#include <xtl.h>
#include <map>
#include <list>
#include "utils.h"
#include "StdString.h"

using namespace std;


class CXMLNode;

class CXMLNodeProxy
{
	CXMLNode * pNode;
public:
	CXMLNodeProxy(CXMLNode & node);
	CXMLNodeProxy(const CXMLNodeProxy & nodeproxy);
	CXMLNodeProxy(CXMLNode * pParent = NULL, LPCTSTR szName = NULL);
	~CXMLNodeProxy();
	CXMLNode * GetNodePtr( void ) { return pNode; };
	operator CXMLNode&();
	
};


class CXMLElement
{
	CXMLNode *	m_pParent;
public:
	bool	SaveElement( HANDLE hFile );
	CXMLElement( const CXMLElement& src );
	CXMLElement( CXMLNode * pParent = NULL, LPCTSTR szName = NULL, LPCTSTR szValue = NULL );
	CXMLElement * GetElementPtr( void ) { return this; };
	CStdString		GetBasicValue( LPCTSTR szLabel = NULL );

	CXMLNode *		GetParent( void ) { return m_pParent; };
	void			SetParent( CXMLNode *pParent ) { m_pParent = pParent; };
	int				GetIndex( bool bNamed );


	bool	HasValue( LPCTSTR szAttrKey = NULL );
	CStdString GetString( LPCTSTR szDefault, LPCTSTR szAttrKey = NULL, bool bTrim = false, bool bLower = false );
	BOOL	GetBool( BOOL bDefault, LPCTSTR szAttrKey = NULL );
	long	GetLong( long lDefault, LPCTSTR szAttrKey = NULL );
	DWORD	GetDWORD( DWORD dwDefault, LPCTSTR szAttrKey = NULL );

	void	SetString( LPCTSTR szValue, LPCTSTR szAttrKey = NULL );
	void	SetBool( BOOL bValue, LPCTSTR szAttrKey = NULL );
	void	SetLong( long lValue, LPCTSTR szAttrKey = NULL );
	void	SetDWORD( DWORD dwValue, LPCTSTR szAttrKey = NULL );


	CStdString		m_sName;
	CStdString		m_sValue;
	TMapStrings		m_msAttributes;
};

typedef list<CXMLNodeProxy> TXMLNodeList;
typedef list<CXMLElement> TXMLElementList;
//typedef map<CStdString, CXMLNodeProxy> TXMLNodes;

class CXMLNode
{
	CXMLNode *	m_pParent;
public:

	bool	SaveNode( LPCTSTR szFilename );
	bool	SaveNode( HANDLE hFile );
	CXMLNode(CXMLNode * pParent = NULL, LPCTSTR szName = NULL);
	CXMLNode(const CXMLNode& node);
	~CXMLNode();
	CXMLNode *		GetParent( void ) { return m_pParent; };
	void			SetParent( CXMLNode *pParent ) { m_pParent = pParent; };
	CXMLNode *		GetNode( LPCTSTR szName, int iIndex = 0 );
	CXMLNode *		GetNode( int iIndex );
	CXMLNode *		AddNode( LPCTSTR szName );
	CXMLNode *		CopyInNode( const CXMLNode& node, LPCTSTR szName = NULL );
	void			FixElementsParent( void );
	int				GetNodeCount( LPCTSTR szName = NULL );

//	CXMLNode *		DetachNode( CXMLNode * pNode );

	CXMLElement *	AddElement( LPCTSTR szName, LPCTSTR szValue = NULL );
	CXMLElement *	GetElement( LPCTSTR szName, int iIndex=0 );
	CXMLElement *	GetElement( int iIndex );
	CXMLElement *	CopyInElement( const CXMLElement& element );
	int				GetElementCount( LPCTSTR szName = NULL );
	void			DeleteNode( CXMLNode * pNode );
	void			DeleteElement( CXMLElement * pElement );
	CXMLNode *		GetRootNode( void );


	CStdString		GetBasicValue( LPCTSTR szKey );
	CStdString		GetSubValue( LPCTSTR szKey, LPCTSTR szSubKey );


	bool	HasValue( LPCTSTR szSection, LPCTSTR szKey );
	// "ini file" type access
	// No defaults here, though the section and keys could be nulled out
	// to be more direct.
	CStdString GetString( LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szDefault, bool bTrim = false, bool bLower = false );
	BOOL	GetBool( LPCTSTR szSection, LPCTSTR szKey, BOOL bDefault );
	long	GetLong( LPCTSTR szSection, LPCTSTR szKey, long lDefault );
	DWORD	GetDWORD( LPCTSTR szSection, LPCTSTR szKey, DWORD dwDefault );

	void	SetString( LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szValue, bool bAttr = false );
	void	SetBool( LPCTSTR szSection, LPCTSTR szKey, BOOL bValue, bool bAttr = false );
	void	SetLong( LPCTSTR szSection, LPCTSTR szKey, long lValue, bool bAttr = false );
	void	SetDWORD( LPCTSTR szSection, LPCTSTR szKey, DWORD dwValue, bool bAttr = false );

	int		GetStrings( LPCTSTR szKey, TListStrings & slStrings );
	int		SetStrings( LPCTSTR szKey, TListStrings & slStrings );
	int		GetIndex( bool bNamed );

	TMapStrings		m_msAttributes;
	TXMLNodeList	m_nlNodes;
	TXMLElementList	m_msElements;
	CStdString		m_sName;
};


class CXMLLoader : public XmlNotify
{
	bool			m_bAutoDelete;
	CXMLNode	*   m_pMainElementNode;
	CXMLNode	*   m_pCurrentElementNode;
	bool			m_bLowerIDs;
	int				m_iLevel;
public:
	CXMLLoader(CXMLNode	* pMainNode = NULL, bool bLowerIDs = true );
	~CXMLLoader();
	CXMLNode	*   LoadXMLNodes( LPCTSTR szFileName );
	CXMLNode	*   LoadXMLNodesFromBuffer( LPCSTR szBuffer );
	CXMLNode	*   GetRootNode( void );
	void			DetachMainNode( void );
	virtual void foundNode		( string & name, string & attributes );
	virtual void terminateNode  ( string & name, string & attributes );
	virtual void foundElement	( string & name, string & value, string & attributes );
	virtual void startElement	( string & name, string & value, string & attributes );
	virtual void endElement		( string & name, string & value, string & attributes );
};




#endif //  __XMLTREE_H__
