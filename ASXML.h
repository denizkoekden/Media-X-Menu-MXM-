

#pragma once

#include <map>

class CASXmlInfo
{
	CXMLElement *  	m_pCurrElement;
	CXMLNode *  	m_pCurrNode;
	CXMLNode *  	m_pMainNode;
	bool			m_bAttached;
	CStdString		m_sFilename;
	bool CheckPointers( bool bCheckElement );
	
public:
	CASXmlInfo( void );
	~CASXmlInfo();
	CASXmlInfo( CXMLNode * pNode, bool bAttached = false );
	CASXmlInfo( const CASXmlInfo & src );
	CASXmlInfo( LPCTSTR szFileName, LPCTSTR szPath );
	bool Save( LPCTSTR szFilename, LPCTSTR szPath );
	bool SaveNode( LPCTSTR szFilename, LPCTSTR szPath );
	bool SetNodePtr( LPCTSTR szNodeIndicator );
	bool CreateNode( LPCTSTR szNodeIndicator );
	bool SetElementPtr( LPCTSTR szElementIndicator );
	bool CreateElement( LPCTSTR szElementIndicator );
	int GetElementCount( LPCTSTR szName );
	int GetNodeCount( LPCTSTR szName );
	int GetElementAttrCount( void );
	int GetNodeAttrCount( void );
	CXMLNode * GetNodeByLocation( LPCTSTR szLocation, CXMLNode * pNode = NULL );
	CXMLElement * GetElementByLocation( LPCTSTR szLocation );
	bool SetElementValue( LPCTSTR szLocation, LPCTSTR szValue );
	bool SetElementAttr( LPCTSTR szLocation, LPCTSTR szValue );
	bool SetNodeAttr( LPCTSTR szLocation, LPCTSTR szValue );
	bool SetValue( LPCTSTR szLocation, LPCTSTR szValue, bool bPrefAttr );
	CStdString GetValue( LPCTSTR szLocation, LPCTSTR szDefault );
	CStdString GetCurrentNodeLocation( void );
	CStdString GetCurrentElementLocation( void );
};


typedef map<CStdString, CASXmlInfo *> TXMLMap;

void ASXML_Create( LPCTSTR szHandleName, LPCTSTR szMainNodeName, TXMLMap & xmlMap );
void ASXML_Open( LPCTSTR szHandleName, LPCTSTR szFilename, LPCTSTR szPath, TXMLMap & xmlMap );
void ASXML_Close( LPCTSTR szHandleName, TXMLMap & xmlMap );
void ASXML_Save( LPCTSTR szHandleName, LPCTSTR szFilename, LPCTSTR szPath, TXMLMap & xmlMap );
void ASXML_SaveNode( LPCTSTR szHandleName, LPCTSTR szFilename, LPCTSTR szPath, TXMLMap & xmlMap );
CASXmlInfo * ASXML_GetInfo( LPCTSTR szHandleName, TXMLMap & xmlMap );
void ASXML_ClearList( TXMLMap & xmlMap );
CStdString GetLocationPart( LPCTSTR szLocation, int iPart, int & iIndex );


/*
XMLOpen <HandleName> <FileName>
- Opens an XML file or internal XML link (::MXM, ::PREFS, ::MENUCACHE, ::MENU, ::Internal)

XMLSave <HandleName> [<FileName>]
- Saves XML back out to a given file

XMLSaveNode <HandleName> <FileName>
- Save current selected node to file (File must not exist)


XMLSetNodePtr <HandleName> <Node> [OnFail <Label>]
- Set the current node pointer
  .Main -> <Config><Main>  ":config.main"
  The initial dot represents the root node.
  A colon can represent a fully qualified node.
  If the node does not exist, the pointer will not be changed. Use the OnFail or check lasterror!

XMLCreateNode <HandleName> <Node> [OnFail <Label>]

XMLCreateElement <HandleName> <Element> [OnFail <Label>]

XMLSetElementPtr <HandleName> <Element> [OnFail <Label>]
- Must be relative to currently set node, or this will also cause the current node to change

XMLSetElementValue <HandleName> <Value>

XMLSetElementAttr <HandleValue> <Attribute> <Value>

XMLSetNodeAttr <HandleValue> <Attribute> <Value>

XMLSetValue <HandleValue> <Location> <Value> [<AttrPref>]

XMLGetValue <HandleValue> <VarName> <Location> [<DefaultValue>]

XMLGetNodeCount <HandleName> <VarName> [<Name>]

XMLGetElementCount <HandleName> <VarName> [<Name>]

XMLGetNodeAttrCount <HandleName> <VarName>

XMLGetElementAttrCount <HandleName> <VarName>


:+ Would advance the pointer one element or node from current
name:+ Would advance to next named element or node from current


.:1 would be the first node or element (depending on what ptr was being set)

:Config.Item:1.Title


. == :Config


Special cases:
Root 
Main (May not be root) 
Current
Next
Prev

:Config:FTPServer:UserItem:
:.:FTPServer:UserItem

!:FTPServer.UserItem:1.Password (From Root, regardless of name)
@:UserItem:1.Password (From Main, might not be root)
+ (Increments index of item, if possible)
- (Decrements index of item, if possible)
: (Goes to first index item of current location)






*/