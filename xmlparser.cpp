
#include <xtl.h>
#include <string>
#include "MXM.h"
#include "CommDebug.h"
#include "xmlparser.h"


XmlParser::XmlParser() :
	m_sParse()
{ 
	Clear(); 

	// get parser id
	m_lID = s_lParseID;

	// next id
	s_lParseID++;
}

bool XmlParser::Create( const char * szBuffer, long lParseStart, long lParseEnd )
{
	// if invalid stop
	if ( !szBuffer || lParseStart < 0 || lParseEnd < lParseStart )
		return false;

	return Create( szBuffer + lParseStart, CalcLength( lParseStart, lParseEnd ) );
}	


bool XmlParser::Create( const char * szBuffer, long lLength )
{
	bool bReturn = false;

	if ( szBuffer && lLength > 0 )
	{
		// parse buffer and offsets of where we are parsing
		m_szBuffer      = (char *) szBuffer;
		m_lParseLength = lLength;

		// current position in parse buffer
		m_lCurrent    = 0;

		bReturn = true;
	}

	return bReturn;
}	

void XmlParser::Release()
{
}

char * XmlParser::GetCurPos()	
{	
	if ( isValid() )
		return m_szBuffer + m_lCurrent;
	else
		return NULL;
}


char * XmlParser::GetParseState( long & lParseLength )
{
	// if not valid stop
	if ( !isValid() )
	{
		lParseLength = 0;
		return GetCurPos();
	}

	/*
	// if value has length
	if ( getValueLength() > 0 )
	{
		// if value has a tag it must be parsed
		if ( valueHasTag() )
		{
			// get parse length
			lParseLength = getValueLength();

			// return ptr to start of value
			return m_szBuffer + m_lValueStart;
		}
	}
	*/

	// get parse state
	lParseLength = GetCurLength();

	// get current buffer position
	char * buffer = GetCurPos();

	// if last tag is valid
	/*
	if ( hasLastTag() )
	{
		// if current position is in last tag
		// pos then show stop
		if ( GetCurPos() >= getLastTagPos() )
		{
			lParseLength = 0;
			return GetLastBufferPos();
		}
	}
	*/

	return GetCurPos();

}

void XmlParser::Reset()
{
	ResetTagPositions();
}

bool XmlParser::isValid()
{
	// if buffer state not valid
	if ( !m_szBuffer || m_lParseLength <= 0 )
		return false;

	// if cur position not valid
	if ( m_lCurrent < 0 || m_lCurrent > m_lParseLength )
		return false;

	return true;
}

void XmlParser::ResetTagPositions( long start /*=-1*/ )
{
	// set tag positions
	m_lFirstTagStart  = start;
	m_lFirstTagEnd	= start;

	m_lLastTagStart   = start;
	m_lLastTagEnd	    = start;

	m_lNameStart	    = start;
	m_lNameEnd		= start;

	m_lAttrStart      = start;
	m_lAttrEnd        = start;

	m_lValueStart     = start;
	m_lValueEnd       = start;
}

void XmlParser::Clear()
{
	// parse buffer and offsets of where we are parsing
	m_szBuffer		 = 0;
	m_lParseLength = 0;

	// current position in parse buffer
	m_lCurrent	= 0;

	// Reset tag positions
	Reset();
}

int XmlParser::GetTagType( string & sTag, string & sName )
{
	int iReturn = 0; // 0=Start Tag, 1= End Tag, 2=Null Tag
	long lEnd, lStart, lCount = 0;

	if ( sTag.size() < 3 )
	{
		sName = "";
	}
	else
	{
		lStart = 1;
		if ( sTag[1] == '/' )
		{
			iReturn = 1;
			lStart = 2;
		}
		else
		{
			iReturn = 0;
		}
		while( sTag[lStart] )
		{
			if( isspace( sTag[lStart] ) )
			{
				lStart++;
			}
			else 
			{
				break;
			}
		}
		lEnd = lStart;
		while( sTag[lEnd] )
		{
			if( isspace( sTag[lEnd] ) )
			{
				break;
			}
			else if( sTag[lEnd] == '/' )
			{
				if ( iReturn == 0 )
				{
					iReturn = 2; // Null Tag
				}
				break;
			}
			else if( sTag[lEnd] == '>' )
			{
				break;
			}
			else
			{
				lCount++;
				lEnd++;
			}
		}
		if ( lCount )
		{
			sName = sTag.substr( lStart, lCount );
		}
		else
		{
			sName = "";
		}
	}
	return iReturn;
}

bool XmlParser::FindNextTag( long & lStart, long & lEnd, int & iType, string & sName, string & sTag )
{
	bool bReturn = false;
	long lPos;

	lPos = find( idTagLeft, lStart );
	if ( lPos != -1 )
	{
		lStart = lPos;
		lPos = find( idTagRight, lPos );
		if ( lPos != -1 )
		{
			lEnd = lPos;
			sTag = string( &m_szBuffer[lStart], (lEnd-lStart)+1 );
			bReturn = true;
		}
		else
		{
			// End of buffer?
			sTag = string( &m_szBuffer[lStart] );
			lEnd = (lStart + sTag.size())-1;
			bReturn = true;
		}
		iType = GetTagType( sTag, sName );
	}
	return bReturn;
}

// LAL / Problem: If a nested node has the same name, we lose the cursor pos.
//       The parser should be smart enough to spot subnodes and skip over them!
//       So we need to track an instance count, if we encounter something between our 
//       suspected and our current beginning

// Better logic? Get each tag, check to see what type it is. If self-terminated tag, 
// Skip it
bool XmlParser::parse()
{
	// init tag position
	m_lFirstTagStart = m_lCurrent;
	m_lFirstTagEnd   = m_lCurrent;

	m_lLastTagStart  = m_lCurrent;
	m_lLastTagEnd    = m_lCurrent;

				    
	// find first tag
	long first = find( idTagLeft, m_lCurrent );
	if ( first == -1 )
		return false;


	// if find right tag
	long last  = find( idTagRight, first );
	if ( last == -1  )
		return false;

	// set first tag start/end
	m_lFirstTagStart = first;
	m_lFirstTagEnd   = last;

	// now parse name
	if ( !parseName() )
		return false;

	// parse attributes
	parseAttributes();

	// if null tag no data or last tag
	if ( hasNullTag() )
	{
		// update cur position
		long lPos = find( '>', m_lCurrent );
		if ( lPos != -1 )
		{
			m_lCurrent = lPos+1;
		}
		else
		{
			m_lCurrent  = (m_lFirstTagEnd + idTagRightLength)+1;
		}
		m_lFirstTagEnd  = m_lCurrent;
		m_lLastTagStart = m_lCurrent;
		m_lLastTagEnd   = m_lCurrent;
		m_lValueStart = m_lValueEnd = m_lCurrent;
		m_lValueEnd--; // Make it so! ZERO length item

		// DEBUG_FORMAT( _T("Found Null Tag: %s (%s)"), getName().c_str(), getAttributes().c_str() );

		// done so show success
		return true;
	}		

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// This is bad code!!!! IT will find the first end tage after the start tag,
// but ignore intervening start tags that are nested!!!!
// BAD BAD BAD!!!!!! LAL
	string possibleTagA;
	string possibleTagB;

	possibleTagA = idTagLeft;
	possibleTagA += getName();
	possibleTagB += possibleTagA;
	possibleTagA += " ";
	possibleTagB += "/";

	bool bFoundEnd = false;
	long lRefCount = 0;
	long lTagPos = last;
	long lTagEndPos;

	string sTagCheck;
	int iType;

	// form end tag
	string endTag;
	endTag  = idTagEnd;
	endTag += getName();
	endTag += idTagRight;

	string sTagName(getName());
	string sTag;
	string sName;

	while( !bFoundEnd && (lTagPos!=-1) )
	{
		if ( FindNextTag( lTagPos, lTagEndPos, iType, sName, sTag ) )
		{
			if ( iType == 0 ) // Start Tag
			{
				if ( strcmpi( sName.c_str(), sTagName.c_str() ) == 0 )
				{
					lRefCount++;
				}
				lTagPos = lTagEndPos;
			}
			else if ( iType == 1 ) // End Tag
			{
				if ( strcmpi( sName.c_str(), sTagName.c_str() ) == 0 )
				{
					if ( lRefCount == 0 )
					{
						bFoundEnd = true;
					}
					else
					{
						lRefCount--;
						lTagPos = lTagEndPos;
					}
				}
				else
				{
					lTagPos = lTagEndPos;
				}
			}
			else // Null Tag - Ignore
			{
				lTagPos = lTagEndPos;
			}
		}
		else
		{
			lTagPos = -1;
		}
//			lTagPos = find( idTagLeft, lTagPos );
//			if ( lTagPos != -1 )
//			{
//				lTagEndPos = find ( idTagRight, lTagPos );
//				sTagCheck
//			}
	}

	if ( bFoundEnd )
	{
		first = lTagPos;
	}
	else
	{

		// find last tag
		first = find( endTag, last );
		if ( first == -1 )
			return false;
	}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


	// set last tag start/end
	m_lLastTagStart = first;
	m_lLastTagEnd   = first + endTag.size()-1;
//		m_lLastTagEnd   = first + endTag.size();

	// parse the value if not a null tag
	if ( !hasNullTag() )
		parseValue();

	// update cur position
	// we have parsed a tag so look for the start
	// of a new tag, if found set current position
	// to it, else set to last tag
	long pos = find( idTagLeft, m_lLastTagEnd );
	if ( pos != -1 )
		m_lCurrent = pos;
	else
		m_lCurrent = m_lLastTagEnd;


	return true;
}
