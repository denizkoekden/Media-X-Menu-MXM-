
//------------------------------------------------------------------------------------------------
// Xbox Country / Video Mode Patcher - Cherry / CCS 2002.
//------------------------------------------------------------------------------------------------

#include	<xtl.h>
#include	"CherryPatch.h"

//------------------------------------------------------------------------------------------------

extern	"C"	int		__stdcall	MmQueryAddressProtect(void *Adr);
extern	"C"	void	__stdcall	MmSetAddressProtect(void *Adr, int Size, int Type);

//------------------------------------------------------------------------------------------------
// Original Code in Kernel that I want to patch. Stored because I must search for it (it moves!)
//------------------------------------------------------------------------------------------------

static	BYTE	OriginalData[57]=
{
	0x55,0x8B,0xEC,0x81,0xEC,0x04,0x01,0x00,0x00,0x8B,0x45,0x08,0x3D,0x04,0x01,0x00,
	0x00,0x53,0x75,0x32,0x8B,0x4D,0x18,0x85,0xC9,0x6A,0x04,0x58,0x74,0x02,0x89,0x01,
	0x39,0x45,0x14,0x73,0x0A,0xB8,0x23,0x00,0x00,0xC0,0xE9,0x59,0x01,0x00,0x00,0x8B,
	0x4D,0x0C,0x89,0x01,0x8B,0x45,0x10,0x8B,0x0D
};

//------------------------------------------------------------------------------------------------
// My Code that I'm going to patch the Kernel with.
//------------------------------------------------------------------------------------------------

static	BYTE	PatchData[70]=
{
	0x55,0x8B,0xEC,0xB9,0x04,0x01,0x00,0x00,0x2B,0xE1,0x8B,0x45,0x08,0x53,0x3B,0xC1,
	0x74,0x0C,0x49,0x3B,0xC1,0x75,0x2F,0xB8,0x00,0x03,0x80,0x00,0xEB,0x05,0xB8,0x04,
	0x00,0x00,0x00,0x50,0x8B,0x4D,0x18,0x6A,0x04,0x58,0x85,0xC9,0x74,0x02,0x89,0x01,
	0x8B,0x4D,0x0C,0x89,0x01,0x59,0x8B,0x45,0x10,0x89,0x08,0x33,0xC0,0x5B,0xC9,0xC2,
	0x14,0x00,0x00,0x00,0x00,0x00
};

//------------------------------------------------------------------------------------------------

void
PatchCountryVideo(F_COUNTRY Country, F_VIDEO Video)
{
	BYTE	*Kernel=(BYTE *)0x80010000;
	DWORD	i, j;
	DWORD	*CountryPtr;
	BYTE	CountryValues[4]={0, 1, 2, 4};
	BYTE	VideoTyValues[5]={0, 1, 2, 3, 3};
	BYTE	VideoFrValues[5]={0x00, 0x40, 0x40, 0x80, 0x40};

	// No Country or Video specified, do nothing.
	if((Country==0) && (Video==0))	return;

	// Video specified with no Country - select default Country for this Video Mode.
	if(Country==0)
	{
		Country=COUNTRY_EUR;
		if(Video==VIDEO_NTSCM)	Country=COUNTRY_USA;
		if(Video==VIDEO_NTSCJ)	Country=COUNTRY_JAP;
	}

	// Country specified with no Video - select default Video Mode for this Country.
	if(Video==0)
	{
		Video=VIDEO_PAL50;
		if(Country==COUNTRY_USA)	Video=VIDEO_NTSCM;
		if(Country==COUNTRY_JAP)	Video=VIDEO_NTSCJ;
	}

	// Search for the original code in the Kernel.
	// Searching from 0x80011000 to 0x80024000 in order that this will work on as many Kernels
	// as possible.

	for(i=0x1000; i<0x14000; i++)
	{
		if(Kernel[i]!=OriginalData[0])	continue;

		for(j=0; j<57; j++)
		{
			if(Kernel[i+j]!=OriginalData[j])	break;
		}
		if(j==57)	break;
	}

	if(j==57)
	{
		// Ok, found the code to patch. Get pointer to original Country setting.
		// This may not be strictly neccessary, but lets do it anyway for completeness.

		j=(Kernel[i+57])+(Kernel[i+58]<<8)+(Kernel[i+59]<<16)+(Kernel[i+60]<<24);
		CountryPtr=(DWORD *)j;
	}
	else
	{
		// Did not find code in the Kernel. Check if my patch is already there.

		for(i=0x1000; i<0x14000; i++)
		{
			if(Kernel[i]!=PatchData[0])	continue;

			for(j=0; j<25; j++)
			{
				if(Kernel[i+j]!=PatchData[j])	break;
			}
			if(j==25)	break;
		}

		if(j==25)
		{
			// Ok, found my patch. Get pointer to original Country setting.
			// This may not be strictly neccessary, but lets do it anyway for completeness.

			j=(Kernel[i+66])+(Kernel[i+67]<<8)+(Kernel[i+68]<<16)+(Kernel[i+69]<<24);
			CountryPtr=(DWORD *)j;
		}
		else
		{
			// Did not find my patch - so I can't work with this BIOS. Exit.
			return;
		}
	}

	// Patch in new code.

	j=MmQueryAddressProtect(&Kernel[i]);
	MmSetAddressProtect(&Kernel[i], 70, PAGE_READWRITE);

	memcpy(&Kernel[i], &PatchData[0], 70);

	// Patch Success. Fix up values.

	*CountryPtr=(DWORD)CountryValues[Country];
	Kernel[i+0x1f]=CountryValues[Country];
	Kernel[i+0x19]=VideoTyValues[Video];
	Kernel[i+0x1a]=VideoFrValues[Video];

	j=(DWORD)CountryPtr;
	Kernel[i+66]=(BYTE)(j&0xff);
	Kernel[i+67]=(BYTE)((j>>8)&0xff);
	Kernel[i+68]=(BYTE)((j>>16)&0xff);
	Kernel[i+69]=(BYTE)((j>>24)&0xff);

	MmSetAddressProtect(&Kernel[i], 70, j);

	// All Done!
}

//------------------------------------------------------------------------------------------------
