
#ifndef XBOXFLASH_H
#define XBOXFLASH_H

#include <stdio.h>
#include <stdarg.h>
//#include "debug.h"

#define	FLASH_STATUS_DQ7			0x80
#define FLASH_STATUS_TOGGLE			0x40
#define FLASH_STATUS_ERROR			0x20
#define FLASH_STATUS_ERASE_TIMER	0x08
#define FLASH_STATUS_TOGGLE_ALT		0x04

#define FLASH_BASE_ADDRESS			0xff000000
#define	FLASH_UNLOCK_ADDR_1			(FLASH_BASE_ADDRESS+0x0555)
#define	FLASH_UNLOCK_ADDR_2			(FLASH_BASE_ADDRESS+0x02aa)

#define FLASH_UNLOCK_DATA_1			0xaa
#define FLASH_UNLOCK_DATA_2			0x55

#define FLASH_COMMAND_RESET			0xf0
#define FLASH_COMMAND_AUTOSELECT	0x90
#define FLASH_COMMAND_PROGRAM		0xa0
#define FLASH_COMMAND_UNLOCK_BYPASS	0x20
#define FLASH_COMMAND_ERASE			0x80

#define FLASH_COMMAND_ERASE_BLOCK	0x30
#define FLASH_COMMAND_ERASE_ALL		0x10

#define FLASH_BLOCK_COUNT			0x10
#define FLASH_BLOCK_MASK			(FLASH_BLOCK_COUNT-1)

class CXBoxFlash
{
public:
	CXBoxFlash()
	{
	}
	
	~CXBoxFlash(void)
	{
	}

//	CXBoxDebug *mpDebug;

	inline void Write(DWORD address,BYTE data)
	{
		volatile BYTE *ptr=(BYTE*)address;
		*ptr=data;
	}

	inline BYTE Read(DWORD address)
	{
		volatile BYTE *ptr=(BYTE*)address;
		return *ptr;
	}

	void SetReadMode(void)
	{
		// 	Unlock stage 1
		Write(FLASH_UNLOCK_ADDR_1,FLASH_UNLOCK_DATA_1); 
		Write(FLASH_UNLOCK_ADDR_2,FLASH_UNLOCK_DATA_2);
		// Issue the reset
		Write(FLASH_UNLOCK_ADDR_1,FLASH_COMMAND_RESET);

		// Leave it in a read mode to avoid any buss contention issues
		BYTE dummy=Read(FLASH_BASE_ADDRESS);
	}

	DWORD CheckID(void)
	{
		DWORD dwReturn = 0;
		BYTE manuf,code;

		// 	Unlock stage 1
		Write(FLASH_UNLOCK_ADDR_1,FLASH_UNLOCK_DATA_1);
		Write(FLASH_UNLOCK_ADDR_2,FLASH_UNLOCK_DATA_2);
		// Issue the autroselect
		Write(FLASH_UNLOCK_ADDR_1,FLASH_COMMAND_AUTOSELECT);

		manuf=Read(FLASH_BASE_ADDRESS);
		code=Read(FLASH_BASE_ADDRESS+1);
		dwReturn = manuf;
		dwReturn <<= 8;
		dwReturn |= code;
//		if(mpDebug) mpDebug->Message(L"Flash Manufacturer = %02x",manuf);
//		if(mpDebug) mpDebug->Message(L"Flash Device code  = %02x",code);
		
		// All done
		SetReadMode();

		// Fix for Hyundai & ST flash.
		if((manuf==0xad && code==0xd5) || (manuf==0x20 && code==0xf1)) 
			return dwReturn |= 0x80000000;
//		if(manuf==0x20 && code==0xf1) return true; else return false;

		return dwReturn;
	}

	bool EraseBlock(int block)
	{
		bool retval;

		// 	Unlock stage 1
		Write(FLASH_UNLOCK_ADDR_1,FLASH_UNLOCK_DATA_1);
		Write(FLASH_UNLOCK_ADDR_2,FLASH_UNLOCK_DATA_2);
		// Issue the erase
		Write(FLASH_UNLOCK_ADDR_1,FLASH_COMMAND_ERASE);
		// Unlock stage 2
		Write(FLASH_UNLOCK_ADDR_1,FLASH_UNLOCK_DATA_1);
		Write(FLASH_UNLOCK_ADDR_2,FLASH_UNLOCK_DATA_2);
		// Now set the block
		Write((FLASH_UNLOCK_ADDR_1+(block&FLASH_BLOCK_MASK)),FLASH_COMMAND_ERASE_BLOCK);
		
		// Now poll for a result
		retval=WaitOnToggle();
		
		// All done		
		SetReadMode();

		return retval;
	}

	bool EraseDevice(void)
	{
		bool retval;

		// 	Unlock stage 1
		Write(FLASH_UNLOCK_ADDR_1,FLASH_UNLOCK_DATA_1);
		Write(FLASH_UNLOCK_ADDR_2,FLASH_UNLOCK_DATA_2);
		// Issue the erase
		Write(FLASH_UNLOCK_ADDR_1,FLASH_COMMAND_ERASE);
		// Unlock stage 2
		Write(FLASH_UNLOCK_ADDR_1,FLASH_UNLOCK_DATA_1);
		Write(FLASH_UNLOCK_ADDR_2,FLASH_UNLOCK_DATA_2);
		// Now set the block
		Write(FLASH_UNLOCK_ADDR_1,FLASH_COMMAND_ERASE_ALL);
		
		// Now poll for a result
		retval=WaitOnToggle();

		// All done
		SetReadMode();

		// Check it all 0xFF
		for(DWORD address=FLASH_BASE_ADDRESS;address<(FLASH_BASE_ADDRESS+0x100000);address++)
		{
			if(Read(address)!=0xff)
			{
//				if(mpDebug) mpDebug->Message(L"EraseDevice() - Flash Verifyfailure at address %08x",address);
//				if(mpDebug) mpDebug->Message(L"EraseDevice() - Expected 0xff, Read %02x",Read(address));
				return false;
			}
		}

		return retval;
	}

	bool ProgramDevice(char *filename)
	{
		FILE *fp;
		BYTE data;
		volatile BYTE dummy;
		DWORD twiddle=0;
		DWORD address=FLASH_BASE_ADDRESS;

		if((fp=fopen(filename,"rb"))==NULL) return false;

		while(fread(&data,1,sizeof(BYTE),fp)==1)
		{
			// Check address bound
			if(address>=FLASH_BASE_ADDRESS+0x100000)
			{
//				if(mpDebug) mpDebug->Message(L"Aborting ProgramDevice() - File too large");
				fclose(fp);
				SetReadMode();
				return false;
			}

			// 	Unlock stage 1
			Write(FLASH_UNLOCK_ADDR_1,FLASH_UNLOCK_DATA_1);
			Write(FLASH_UNLOCK_ADDR_2,FLASH_UNLOCK_DATA_2);
			// Issue the program command
			Write(FLASH_UNLOCK_ADDR_1,FLASH_COMMAND_PROGRAM);
			// Program Byte
			Write(address,data);

			// Do the Data polling test
			while(1)
			{
				dummy=Read(address);

				if((data&FLASH_STATUS_DQ7)==(dummy&FLASH_STATUS_DQ7)) break;

				if(dummy&FLASH_STATUS_ERROR)
				{
					dummy=Read(address);
					if((data&FLASH_STATUS_DQ7)==(dummy&FLASH_STATUS_DQ7))
					{
						break;
					}
					else
					{
//						if(mpDebug) mpDebug->Message(L"Aborting ProgramDevice() - Flash Write failure at address %08x",address);
						fclose(fp);
						SetReadMode();
						return false;
					}
				}
			}
			Write(FLASH_UNLOCK_ADDR_1,FLASH_COMMAND_RESET);

			dummy=Read(address);
			// Verify the written byte
			if(dummy!=data)
			{
//				if(mpDebug) mpDebug->Message(L"Aborting ProgramDevice() - Flash Verifyfailure at address %08x",address);
//				if(mpDebug) mpDebug->Message(L"Aborting ProgramDevice() - Wrote %02x, Read %02x",data,dummy);
				fclose(fp);
				SetReadMode();
				return false;
			}

			// Next byte
			address++;

			// User information
			if((address&0xffff)==0x0000)
			{
//				if(mpDebug) mpDebug->MessageInstant(L"ProgramDevice() - Wrote block %02d",(address>>16)&0xff);
			}
		}
//		if(mpDebug) mpDebug->MessageInstant(L"ProgramDevice() - Wrote %06x Bytes",address-FLASH_BASE_ADDRESS);

		// Verify written code
		int count=0;
		fseek(fp,0,SEEK_SET);
		address=FLASH_BASE_ADDRESS;
		while(fread(&data,1,sizeof(BYTE),fp)==1)
		{
			if(data!=Read(address))
			{
//				if(mpDebug) mpDebug->Message(L"Aborting ProgramDevice() - Flash Verify Failure at address %08x",address);
//				if(mpDebug) mpDebug->Message(L"Aborting ProgramDevice() - Wrote %02x, Read %02x",data,Read(address));
				if(count++>8) return false;
			}
			address++;
		}
		if(count)
		{
//			if(mpDebug) mpDebug->MessageInstant(L"ProgramDevice() - Failed Verify");
		}
		else
		{
//			if(mpDebug) mpDebug->MessageInstant(L"ProgramDevice() - Verified %06x Bytes",address-FLASH_BASE_ADDRESS);
		}
		
		// All done
		fclose(fp);

		return true;
	}

	bool WaitOnToggle(void)
	{
		BYTE last,current;
		last=Read(FLASH_BASE_ADDRESS);
		while(1)
		{
			// We wait for the toggle bit to stop toggling
			current=Read(FLASH_BASE_ADDRESS);
			// Check for an end to toggling
			if((last&FLASH_STATUS_TOGGLE)==(current&FLASH_STATUS_TOGGLE)) break;
			last=current;

			// We're still in command mode so its OK to check for Error condition
			if(current&FLASH_STATUS_ERROR)
			{
				last=Read(FLASH_BASE_ADDRESS);
				current=Read(FLASH_BASE_ADDRESS);
				if((last&FLASH_STATUS_TOGGLE)==(current&FLASH_STATUS_TOGGLE)) break; else return false;
			}
		}
		return true;
	}

};

struct _FlashInfo
{
	LPCTSTR m_szDescription;
	WORD	m_wFlashID;
	DWORD	m_dwSize;
};

typedef struct _FlashInfo TFlashInfo;

extern TFlashInfo g_flashInfo[];


#endif