/*********************************************************************************\
 * ColorCycle.cpp
 * Class used to cycle through RGB bvalues
 * (C) 2002 MasterMind
\*********************************************************************************/



#include <xtl.h>
#include "ColorCycle.h"
#include "utils.h"


CColorCycle::CColorCycle( DWORD dwInitColor )
{
    m_byRed = (BYTE)((dwInitColor>>16)&0xff);
    m_byGreen = (BYTE)((dwInitColor>>8)&0xff);
    m_byBlue = (BYTE)((dwInitColor)&0xff);
    // Randomly choose our vectors
	int iVectorize;
	m_iRedVector = 0;
	m_iGreenVector = 0;
	m_iBlueVector = 0;
	iVectorize = RandomRange(0,7); // rand()%8;
	if ( iVectorize == 0 )
	{
		iVectorize = 1<<(RandomRange(0,2)); // rand()%3);
	}
	if( iVectorize&0x01)
	{
		if ( m_byRed > 127 ) m_iRedVector = -1;
		else m_iRedVector = 1;
	}
	if( iVectorize&0x02)
	{
		if ( m_byGreen > 127 ) m_iGreenVector = -1;
		else m_iGreenVector = 1;
	}
	if( iVectorize&0x04)
	{
		if ( m_byBlue > 127 ) m_iBlueVector = -1;
		else m_iBlueVector = 1;
	}
    
}

DWORD CColorCycle::GetColor( void )
{
	DWORD dwReturn;
	bool  bResetVectors = false;

	dwReturn = 0xff;
	dwReturn <<=8;
	dwReturn |= m_byRed;
	dwReturn <<=8;
	dwReturn |= m_byGreen;
	dwReturn <<=8;
	dwReturn |= m_byBlue;
	if ( !( m_iRedVector || m_iGreenVector || m_iBlueVector ) )
	{
		// Problem.... nobody has a vector! Give somebody a vector.
		bResetVectors = true;    
	}
	if ( m_iRedVector && ((m_byRed == 0)||(m_byRed == 255 )) )
	{
		bResetVectors = true;    
		if ( m_byRed == 255 )
		{
			m_byRed = 254;
		}
		else
		{
			m_byRed = 1;
		}
	}
	else
	{
		m_byRed += m_iRedVector;
	}
	if ( m_iGreenVector && ((m_byGreen == 0)||(m_byGreen == 255 )) )
	{
		bResetVectors = true;    
		if ( m_byGreen == 255 )
		{
			m_byGreen = 254;
		}
		else
		{
			m_byGreen = 1;
		}
	    
	}
	else
	{
	    m_byGreen += m_iGreenVector;
	}
	if ( m_iBlueVector && ((m_byBlue == 0)||(m_byBlue == 255 )) )
	{
		bResetVectors = true;    
		if ( m_byBlue == 255 )
		{
			m_byBlue = 254;
		}
		else
		{
			m_byBlue = 1;
		}
	}
	else
	{
		m_byBlue += m_iBlueVector;
	}
	if ( bResetVectors )
	{
		int iVectorize;
		m_iRedVector = 0;
		m_iGreenVector = 0;
		m_iBlueVector = 0;
		iVectorize = RandomRange(0,7); // rand()%8;
		if ( iVectorize == 0 )
		{
			iVectorize = 1<<(RandomRange(0,2)); // rand()%3);
		}
		if( iVectorize&0x01)
		{
			if ( m_byRed > 127 ) m_iRedVector = -1;
			else m_iRedVector = 1;
		}
		if( iVectorize&0x02)
		{
			if ( m_byGreen > 127 ) m_iGreenVector = -1;
			else m_iGreenVector = 1;
		}
		if( iVectorize&0x04)
		{
			if ( m_byBlue > 127 ) m_iBlueVector = -1;
			else m_iBlueVector = 1;
		}
    }
    return dwReturn;
}


