

#include "StdAfx.h"
#include "FlashData.h"


TFlashDefs flashDefs[] =
{
	{ 0xbf, 0x61, _T("SST49LF020"), 		0x40000  }, 
	{ 0x01, 0xd5, _T("Am29F080B"), 			0x100000 }, 
	{ 0x04, 0xd5, _T("Fujitsu MBM29F080A"), 0x100000 }, 
	{ 0xad, 0xd5, _T("Hynix HY29F080"), 	0x100000 },
	{ 0x20, 0xf1, _T("ST M29F080A"), 		0x100000 },
	{ 0x89, 0xa6, _T("Sharp LHF08CH1"),		0x100000 },
	{ 0xda, 0x8c, _T("Winbond W49F020"),	0x40000  },
	{ 0x01, 0xa4, _T("AMD - Am29F040B"),	0x80000  },
	{ 0xda, 0x0b, _T("Winbond - W49F002U"),	0x40000  },
	{ 0xc2, 0x36, _T("MACRONIX - MX29F022NTPC"),0x40000 },
	{ 0x20, 0xb0, _T("ST M29f002BT"),		0x40000  },
	
	{ 0, 0, NULL, 0 } // terminator
};

bool CFlashData::GetDescriptor( void )
{
	bool bReturn = false;
	BYTE baNormalModeFirstTwoBytes[2];
	int iTries=0;
	int nPos=0;

	m_bWriteable = true;

	baNormalModeFirstTwoBytes[0] = m_pbMMStartAddr[0];
	baNormalModeFirstTwoBytes[1] = m_pbMMStartAddr[1];

	while(iTries++ <2)
	{ // first we try 29xxx method, then 28xxx if that failed

		// no ISRs should touch flash while we do the stuff
		__asm__ __volatile__ ( "pushf ; cli ");


		// 29xxx protocol
		if ( iTries != 1 )
		{ 	

			// make sure the flash state machine is reset

			m_pbMMStartAddr[0x5555]=0xf0;
			m_pbMMStartAddr[0x5555]=0xaa;
			m_pbMMStartAddr[0x2aaa]=0x55;
			m_pbMMStartAddr[0x5555]=0xf0;

				// read flash ID

			m_pbMMStartAddr[0x5555]=0xaa;
			m_pbMMStartAddr[0x2aaa]=0x55;
			m_pbMMStartAddr[0x5555]=0x90;

			m_byMfgID=m_pbMMStartAddr[0];
			m_byDevID=m_pbMMStartAddr[1];

			m_pbMMStartAddr[0x5555]=0xf0;

			m_bDetectedVia28=false; // mark the flash object as representing a 28xxx job


		} 
		else
		{ 
			// 28xxx protocol, seen on Sharp
			// make sure the flash state machine is reset

			m_pbMMStartAddr[0x5555]=0xff;

				// read flash ID

			m_pbMMStartAddr[0x5555]=0x90;
			m_byMfgID=m_pbMMStartAddr[0];

			m_pbMMStartAddr[0x5555]=0x90;
			m_byDevID=m_pbMMStartAddr[1];

			m_pbMMStartAddr[0x5555]=0xff;

			m_bDetectedVia28=true; // mark the flash object as representing a 28xxx job

		}

		__asm__ __volatile__ ( "popf ");


		if(
			(baNormalModeFirstTwoBytes[0]!=m_byMfgID) ||
			(baNormalModeFirstTwoBytes[1]!=m_pbMMStartAddr[1])
			) 
		{
			nTries=2;  // don't try any more if we got some result the first time
		}

	} // while


	// interpret device ID info
	{
		bool fMore=true;
		while(fMore)
		{
			if(!pkft->m_bManufacturerId)
			{
				fMore=false; continue;
			}
			if((pkft->m_bManufacturerId == m_byMfgID) &&
				(pkft->m_bDeviceId == m_byDevID) )
			{
				bReturn=true;
				fMore=false;
				CStdString sTemp;
				
				sTemp.Format( _T(" %s (%ldK) "), pkft->m_szFlashDescription, pkft->m_dwLengthInBytes/1024);
				m_sFlashDescription += sTemp;
				
				m_dwLength = pkft->m_dwLengthInBytes;

				if(m_bDetectedVia28)
				{
					int n=0;
						// detect master lock situation

					m_pbMMStartAddr[0x5555]=0x90;
					if ( m_pbMMStartAddr[3] != 0 )
					{ // master lock bit is set, no erases or writes are going to happen
						m_bWriteable=false;
						
						m_sFlashDescription += _T("Master Lock SET  ");
					}

						// detect block lock situation

					m_sFlashDescription += _T("Block Locks: ");
					while(n<m_dwLength)
					{
						m_pbMMStartAddr[0x5555]=0x90;
						sTemp.Format("%u", m_pbMMStartAddr[n|0x0002]&1 );
						
						m_sFlashDescription += sTemp;
						// nPos+=sprintf(&pof->m_szFlashDescription[nPos], "%u", m_pbMMStartAddr[n|0x0002]&1);
						n+=0x10000;
					}
					m_sFlashDescription += _T("  ");;
					// nPos+=sprintf(&pof->m_szFlashDescription[nPos], "  ");
					m_pbMMStartAddr[0x5555]=0x50;
					m_pbMMStartAddr[0x5555]=0xff;

				}
			}
			pkft++;
		}
	}


	if(!bReturn) 
	{
		if(
			(baNormalModeFirstTwoBytes[0]==m_byMfgID) &&
			(baNormalModeFirstTwoBytes[1]==m_pbMMStartAddr[1])
			)
		{ // we didn't get anything worth reporting
			m_sFlashDescription.Format( _T("Read Only??? manf=0x%02X, dev=0x%02X"), m_byMfgID, m_byDevID);
		} 
		else
		{ // we got what is probably an unknown flash type
			m_sFlashDescription.Format( _T("manf=0x%02X, dev=0x%02X"), m_byMfgID, m_byDevID);
		}
	}
	return bReturn;
}

