/*********************************************************************************\
 * ColorCycle.h
 * Class used to cycle through RGB bvalues
 * (C) 2002 MasterMind
\*********************************************************************************/

#ifndef __COLORCYCLE_H__
#define __COLORCYCLE_H__



class CColorCycle
{
public:
    CColorCycle( DWORD dwInitColor = 0xff000000 );
    BYTE m_byRed;
    BYTE m_byGreen;
    BYTE m_byBlue;
    int  m_iRedVector;
    int  m_iGreenVector;
    int  m_iBlueVector;
    DWORD GetColor( void );
};


#endif //  __COLORCYCLE_H__