#if !defined(XmlParser_H)
#define XmlParser_H

#include "XmlUtil.h"

#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>
using namespace std;



//////////////////////////////////////////////////////////////////////////////
// XmlParserr
//
// Purpose: used to parse a buffer that has xml formatted data

class XmlParser
{
public:

	static long s_lParseID;
	
	// id of parser (for debugging)
	long		m_lID;

	// provides string interface to buffer
	string		m_sParse;

	// parse buffer and offsets of where we are parsing
	char *		m_szBuffer;
	long		m_lParseLength;

	// current position in parse buffer
	long		m_lCurrent;

	// tag, attributes and data position information
	long		m_lFirstTagStart;
	long		m_lFirstTagEnd;

	long		m_lLastTagStart;
	long		m_lLastTagEnd;

	long		m_lNameStart;
	long		m_lNameEnd;
	long		m_lAttrStart;
	long		m_lAttrEnd;
	long		m_lValueStart;
	long		m_lValueEnd;


	XmlParser();

	// Create/Release
	bool Create( const char * szBuffer, long lParseStart, long lParseEnd );
	bool Create( const char * szBuffer, long lLength );
	void Release();

	// cur position methods
	long GetCurrent() {	return m_lCurrent; };
	long GetParseLength(){	return m_lParseLength; };
	long GetCurLength()	 {	return GetOffsetLength(m_lCurrent);	};
	long GetOffsetLength( long lOffset ) { return CalcLength( lOffset, m_lParseLength - 1); };
	char * GetBufferPos() {	return m_szBuffer;	};
	char * GetLastBufferPos() {	return m_szBuffer + m_lParseLength;	};
	char * GetCurPos();
	char * GetParseState( long & lParseLength );

	// get ref to parse buffer
	string & str()	{ return m_sParse; };

	// state methods
	void Reset();
	bool isValid();
	void ResetTagPositions( long start=-1 );
	void Clear();
	int GetTagType( string & sTag, string & sName );
	bool FindNextTag( long & lStart, long & lEnd, int & iType, string & sName, string & sTag );

	// parse methods
	bool parse( const char * buffer, long parseStart, long parseEnd )
	{
		// if Create fails stop
		if ( !Create(buffer,parseStart,parseEnd) )
			return false;

		return parse();
	};

	bool parse( const char * buffer, long parseLength )
	{
		// if Create fails stop
		if ( !Create(buffer,parseLength) )
			return false;

		return parse();
	};

	
	bool parse();




	bool parse ( string & name,
				 string & value,
				 string & attributes,
				 long & current )
	{

		return true;
	}


	// tag search methods
	bool hasNullTag()  
	{    
		// get beginning of first tag    
		char * firstTagBegin = m_szBuffer + m_lFirstTagStart;    
		// get end of first tag    
		char * firstTagEnd = m_szBuffer + m_lFirstTagEnd - 1;    

		// if null tag marker    
		if (( *firstTagBegin == '<' && *(firstTagBegin+1) == '?' ) ||        
			( *firstTagBegin == '<' && *(firstTagBegin+1) == '!' ) ||        
			( *firstTagEnd == '/' && *(firstTagEnd+1) == '>' ))      
				return true;    
		else      
			return false;  
	}
#if 0
	bool hasNullTag()
	{
		// get end of first tag
		char * buffer = m_szBuffer + m_lFirstTagEnd - 1;

		// if null tag marker
		if ( *buffer == '/' && *(buffer+1) == '>' )
			return true;
		else
			return false;
	}
#endif
	/////////////////////////////////////////////////////////
	// these methods are protected because the state
	// of parsing might not be properly setup, and
	// if that were so then calling these methods
	// would cause errors

protected:

	// name search methods
	bool parseName()
	{
		// if first tag search failed show failed
		if ( m_lFirstTagStart < 0 ||  m_lFirstTagEnd < 0 ||
			 m_lFirstTagEnd <= m_lFirstTagStart )
		{
			m_lNameStart = -1;
			m_lNameEnd   = -1;
			return false;
		}

		// init name start/end position
		m_lNameStart = m_lFirstTagStart + idTagLeftLength;
		m_lNameEnd   = m_lFirstTagEnd - 1;

		// if null tag then backup before
		// null tag marker
		if ( hasNullTag() )
			m_lNameEnd -= 1;

		// check for separator (i.e. there are attributes)
		long last = find(' ',m_lNameStart, getNameLength());
		if ( last != -1 )
		{
			// there are attributes so backup
			// before attributes
			m_lNameEnd = last - 1;
		}

		return true;
	}

	bool parseName( string & name )
	{
		// set name state
		if ( !parseName() )
			return false;

		name = getName();

		return true;
	}

	// attribute search methods
	bool parseAttributes()
	{
		// init name start/end position
		m_lAttrStart = -1;
		m_lAttrEnd   = -1;

		// if tag or name length invalid stop
		long tagLength  = getTagLength();
		long nameLength = getNameLength();
		if ( tagLength <= 0 || nameLength <= 0 )
			return 0;

		// if the difference in the lengths is
		// less than the length of the left/right marker
		// then no attributes
		long diff = getTagLength() - getNameLength();

		switch ( diff )
		{
			// no attribute case
			case 0:
			case 1:
			case 2:
				return false;

			// no attribute case but has null tag
			case 3:
				return false;
		}

		// init attributes start, move past space after name
		m_lAttrStart = m_lNameEnd + 2;

		// init attribute end move before right tag marker
		// if null tag move before null tag marker
		m_lAttrEnd = m_lFirstTagEnd - 1;
//		if ( hasNullTag() )
//			m_lAttrEnd -= -1;
		if ( hasNullTag() )
			m_lAttrEnd -= 1;

		// Special case - we'd never have "valid" attributes of len 1 anyway
		string sTemp = getAttributes();

		if ( sTemp.size() == 1 )
		{
			if ( sTemp[0] == '>' )
			{
				m_lAttrEnd--;
			}
		}
		return true;
	}

	bool parseAttributes( string & attributes )
	{
		// set name state
		if ( !parseAttributes() )
			return false;

		attributes = getAttributes();

		return true;
	}


	// data search methods
	bool parseValue()
	{
		// if first tag search failed show failed
		if ( m_lFirstTagStart < 0 ||  m_lLastTagEnd < 0 ||
			 m_lLastTagEnd <= m_lFirstTagStart )
		{
			m_lValueStart = -1;
			m_lValueEnd   = -1;
			return false;
		}

		// init value start/end positions
		m_lValueStart = m_lFirstTagEnd + 1;
		m_lValueEnd   = m_lLastTagStart - 1;

		return true;
	}


	bool parseValue( string & value )
	{
		// set name state
		if ( !parseValue() )
			return false;

		value = getValue();

		return true;
	}

public:

	// name access methods
	char * getNamePos()
	{
		if ( hasName() )
			return m_szBuffer + m_lNameStart;
		else
			return NULL;
	}

	bool hasName()
	{
		if ( getNameLength() > 0 )
			return true;
		else
			return false;
	}

	long getNameLength()
	{
		long length = CalcLength(m_lNameStart,m_lNameEnd);
		return length;
	}


	string getName()
	{
		// get name length
		long length = getNameLength();

		// if length invalid show null string
		// else get string
		if ( length <= 0 )
			return string("");
		else
			return substr(m_lNameStart,length);
	}


	// attribute access methods
	char * getAttributesPos()
	{
		if ( hasAttributes() )
			return m_szBuffer + m_lAttrStart;
		else
			return NULL;
	}

	bool hasAttributes()
	{
		if ( getValueLength() > 0 )
			return true;
		else
			return false;
	}

	long getAttributesLength()
	{
		long length = CalcLength(m_lAttrStart,m_lAttrEnd);
		return length;
	}

	string getAttributes()
	{
		// get attribute length
		long length = getAttributesLength();

		// if length invalid show null string
		// else get string
		if ( length <= 1 )
			return string("");
		else
			return substr(m_lAttrStart,length);
	}

	// value access methods
	char * getValuePos()
	{
		if ( hasValue() )
			return m_szBuffer + m_lValueStart;
		else
			return NULL;
	}

	bool hasValue()
	{
		if ( getValueLength() > 0 )
			return true;
		else
			return false;
	}

	long getValueLength ()
	{
		long length = CalcLength(m_lValueStart,m_lValueEnd);
		return length;
	}

	string getValue ()
	{
		// get tag data length
		long length = getValueLength();

		// if length invalid show null string
		// else get string
		if ( length <= 0 )
			return string("");
		else
			return substr(m_lValueStart,length);
	}

	char * getValueState ( long & valueLength )
	{
		// get value state
		valueLength = getValueLength();

		// return value buffer pos
		return m_szBuffer + m_lValueStart;
	}

	bool valueHasTag ()
	{
		bool bReturn = false;
		// if find end tag
		if ( getValueLength() )
		{
			long pos = find( idTagLeft, m_lValueStart, getValueLength() );

			// if found tag
			if ( pos != -1 )
				bReturn = true;
			else
				bReturn = false;
		}
		return bReturn;
	}

	// tag access methods
	long getTagLength ()
	{
		long length = CalcLength( m_lFirstTagStart, m_lFirstTagEnd );
		return length;
	}

	long getLastTagLength ()
	{
		long length = CalcLength( m_lLastTagStart, m_lLastTagEnd );
		return length;
	}

	bool hasTag ()
	{
		if ( getTagLength() > 0 )
			return true;
		else
			return false;
	}

	bool hasLastTag ()
	{
		if ( getLastTagLength() > 0 )
			return true;
		else
			return false;
	}

	char * getTagPos ()
	{
		if ( hasTag() )
			return m_szBuffer + m_lFirstTagStart;
		else
			return NULL;
	}

	char * getLastTagPos ()
	{
		if ( hasTag() )
			return m_szBuffer + m_lLastTagStart;
		else
			return NULL;
	}

	string getTag ()
	{
		// get tag data length
		long length = getTagLength();
		return substr(m_lFirstTagStart,length);
	}

	// string utility methods
	long CalcLength( long lStartPos,
			         long lEndPos )
	{
		long lReturn = 0;

		// if positions invalid show no length
		if ( lStartPos >= 0 && lEndPos >= 0  ||
			 lEndPos >= lStartPos )
		{
			// get length
			lReturn = lEndPos - lStartPos + 1;
		}
		return lReturn;
	}

	string ::iterator begin ()
	{
		string::iterator buf = m_szBuffer;
		return string::iterator(buf);
	}

	string ::iterator end ()
	{
		string::iterator buf = m_szBuffer + m_lParseLength;
		return string::iterator(buf);
	}

	long find ( char srchChar, long offset, long length = -1 )
	{
		// if no length set to length to
		// end of parse buffer
		if ( length == -1 )
			length = GetOffsetLength(offset);

		// set start and end of search 
		string::iterator start = m_szBuffer + offset;
		string::iterator end   = m_szBuffer + (offset + length);

		// search for it
		string::iterator found = std::find( start, end, srchChar );

		// if at end did not find it
		if ( found >= end )
		{
			return -1;
		}
		else
		{
			// as a last check make sure found is valid
			if ( found < start )
				return -1;

			// set position
			long pos = (found - start);
			pos += offset;

			return pos;
		}
	}

	long find ( char * srchStr, long offset, long length = -1 )
	{
		// if no length set to length to
		// end of parse buffer
		if ( length == -1 )
			length = GetOffsetLength(offset);

		// set start and end of search 
		string::iterator start = m_szBuffer + offset;
		string::iterator end   = m_szBuffer + (offset + length);


		string::iterator srchStart = srchStr;
		string::iterator srchEnd   = srchStr + strlen(srchStr);

		// search for it
		string::iterator found = std::search( start, end, srchStart, srchEnd );

		// if at end did not find it
		if ( found >= end )
		{
			return -1;
		}
		else
		{
			// as a last check make sure found is valid
			if ( found < start )
				return -1;

			// set position
			long pos = (found - start);
			pos += offset;

			return pos;
		}
	}

	long find ( string & srchStr, long offset, long length = -1 )
	{
		// if no length set to length to
		// end of parse buffer
		if ( length == -1 )
			length = GetOffsetLength(offset);

		// set start and end of search 
		string::iterator start = m_szBuffer + offset;
		string::iterator end   = m_szBuffer + (offset + length);


		string::iterator srchStart = srchStr.begin();
		string::iterator srchEnd   = srchStr.end();

		// search for it
		string::iterator found = std::search( start, end, srchStart, srchEnd );

		// if at end did not find it
		if ( found >= end )
		{
			return -1;
		}
		else
		{
			// as a last check make sure found is valid
			if ( found < start )
				return -1;

			// set position
			long pos = (found - start);
			pos += offset;

			return pos;
		}
	}

	long rfind ( char srchChar, long offset, long length )
	{
		// setup srch string
		char srchStr[2];
		srchStr[0] = srchChar;
		srchStr[1] = '\0';

		return rfind(srchStr,offset,length);
	}

	long rfind ( char * srchStr, long offset, long length )
	{
		/*
		// set start and end of search 
		string::reverse_iterator revStart = m_szBuffer + (offset + length)
		string::reverse_iterator revEnd   = m_szBuffer + offset;

		// search for it
		string::reverse_iterator found = std::find( start, end, srchStr );

		// get position 
		long pos = found - revStart;
		*/
		long pos = 0;

		return pos;
	}


	string substr ( long offset, long length )
	{
		// get start of sub string
		char * ptr = m_szBuffer + offset;

		// Create string for it
		string str;
		str.assign( ptr, length );

		return string(str);
	}


};


#endif