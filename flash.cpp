
#include <xtl.h>
#include "flash.h"


#define FlashBaseAddress( ID )  ((ULONG_PTR)(0xFFFFFFFF-FlashSize(ID)+1))
#define FLASH_BASE_ADDRESS      0xFFF00000
#define FLASH_REGION_SIZE       (0xFFFFFFFF-FLASH_BASE_ADDRESS-1)

#define FlashWriteByte(a, d) \
	(*(PBYTE)((ULONG_PTR)KernelRomBase+(ULONG_PTR)(a)-FLASH_BASE_ADDRESS) = d)

#define FlashReadByte(a) \
	(*(PBYTE)((ULONG_PTR)KernelRomBase+(ULONG_PTR)(a)-FLASH_BASE_ADDRESS))



TFlashInfo g_flashInfo[] =
{
	{ "Am29F080B - AMD",		0x01d5, 0x100000 },
	{ "MBM29F080A - FUJITSU",	0x04d5, 0x100000 },
	{ "HY29F080 - Hynix",		0xadd5, 0x100000 },
	{ "M29F080A - ST",			0x20f1, 0x100000 },
	{ "A29002 - AMIC",			0x378c, 0x40000 },
	{ "Am29F080B - AMD",		0xbf61, 0x40000 },
	{ "W29f020 - Winbond",		0xda8c, 0x40000 },
	{ "M29F002BT70N1 - ST",		0x20b0, 0x40000 },
	{ "SST49LF040 - SST",		0xbf53, 0x80000 },
	{ "Protected BIOS",			0x0900,	0x40000 },
	{ "Unknown",				0,		0x40000 }
};


// 0xBF 0x53  SST SST_49LF040 FlashSST49LF040

//	0x01 0xD5 AMD GENERIC_29F080  FlashGeneric29F080
//	0x04 0xD5 Fujitsu GENERIC_29F080  FlashGeneric29F080
//	0xAD 0xD5 HYUNDAI_ID GENERIC_29F080  FlashGeneric29F080

// 0x20 0xF1  ST M29F080A   FlashGeneric29F080

#ifdef FLASH_TIME

			 /*++

			 Routine Description:

			 This routine reads processor's time-stamp counter.  The time-stamp counter
			 is contained in a 64-bit MSR.  The high-order of 32 bits MSR are loaded
			 into the EDX register, and the low-order 32 bits are loaded into the EAX
			 register.  The processor increments the time-stamp counter MSR every
			 clock cycle and resets it to 0 whenever the processor reset.

			 Arguments:

			 None

			 Return Value:

			 64-bit MSR of time-stamp counter

			 --*/
UINT64 FlashReadTSC( VOID )
{
	__asm 
	{
		rdtsc
	}
}

#endif // FLASH_TIME


#if 0
					  /*++

					  Routine Description:

					  This routine searches for ROM cache setting in MTRR and disable it.  It is
					  necessary to disable and flash cache before changing MTRR.  The following
					  steps are recommended by Intel in order to change MTRR settings. Save CR4,
					  disable and flush processor cache, flush TLB, disable MTRR, change MTRR
					  settings, flush cache and TLB, enable MTRR and restore CR4

					  Arguments:

					  EnableCache - TRUE to enable caching, FALSE to disable

					  Return Value:

					  None

					  --*/


VOID FlashChangeRomCaching( BOOL EnableCache )
{
	ULONG MTRR;
	UINT64 v, MTRRdeftype;
	ULONG Base, Type;

	__asm {
		push    ecx
		push    edx

		_emit   0fh                 ; mov  eax, cr4
		_emit   20h
		_emit   0e0h

		push    eax                 ; save content of cr4

		mov     eax, cr0            ; disable and flush cache
		push    eax                 ; save content of cr0
		or      eax, 060000000H
		mov     cr0, eax
		wbinvd

		mov     eax, cr3            ; flush TLB
		mov     cr3, eax
	}

	//
	// Save the content of MTRR deftype and disable MTRR
	//

	MTRRdeftype = FlashReadMSR( 0x2FF );
	FlashWriteMSR( 0x2FF, 0 );

	for ( MTRR=0x200; MTRR<0x20F; MTRR+=2 )
	{
		v = FlashReadMSR( MTRR );
		Base = (ULONG)((v >> 12) & 0xFFFFFF);
		Type = (BYTE)v;

		//
		// Set or reset valid bit according to cache enable flag
		//

		if ( Base >= (FLASH_BASE_ADDRESS >> 12) && Type != 0 )
		{
			v = FlashReadMSR( MTRR+1 );
			v = EnableCache ? (v | 0x800) : (v & (~0x800));
			FlashWriteMSR( MTRR+1, v );
		}
	}

	__asm {
		wbinvd                      ; flush cache
		mov     eax, cr3            ; flush TLB
		mov     cr3, eax
	}

	//
	// Restore content of MTRR deftype, MTRR should be re-enabled
	//

	FlashWriteMSR( 0x2FF, MTRRdeftype );

	__asm {
		pop     eax                 ; restore cr0
			mov     cr0, eax

			pop     eax                 ; restore cr4

			_emit   0fh                 ; mov  cr4, eax
			_emit   22h
			_emit   0e0h

			pop     edx
			pop     ecx
	}
}



HRESULT FlashKernelImage( PVOID  ImageBuffer, SIZE_T ImageSize, LPSTR  szResp, DWORD  cchResp, ULONG  writeOffset )
{
#ifdef FLASH_TIME
	UINT64 ClockTick;
#endif

	FlashDeviceID ID;
	HRESULT hr = S_OK;

#ifdef FLASH_TIME
	ClockTick = FlashReadTSC();
#endif

	if (	IsBadReadPtr(ImageBuffer, ImageSize) ||
			IsBadWritePtr(szResp, cchResp) ) 
	{
		return HRESULT_FROM_WIN32( ERROR_NOACCESS );
	}

	//
	// Map top 1MB of physical memory of ROM region (FFF00000-FFFFFFFF)
	//

	KernelRomBase = MmMapIoSpace( FLASH_BASE_ADDRESS, FLASH_REGION_SIZE, PAGE_READWRITE | PAGE_NOCACHE );

	if ( !KernelRomBase )
	{
		_snprintf( szResp, cchResp, "unable to map i/o space" );
		return E_FAIL;
	}

	//		__asm cli

	OutputDebugString("FLASH: interrupts are now disabled\n" );

	//
	// Disable ROM caching
	//

	FlashChangeRomCaching( FALSE );

	ID = FlashDetectDevice();

	if ( ID == FlashUnknownDevice )
	{
		_snprintf( szResp, cchResp, "Unknown flash device id" );
		hr = E_FAIL;
		goto cleanup;
	}

	/*		if ( FlashSize(ID) != ImageSize ) {
		_snprintf( szResp, cchResp, "Invalid image size" );
		hr = E_FAIL;
		goto cleanup;
		}
	*/
	OutputDebugString("FLASH: erasing and blank checking...\n" );

	bool result;

	result = FlashErase(ID, writeOffset, ImageSize);

	if (!result)
	{
		_snprintf( szResp, cchResp, "Failed to erase flash chip" );
		hr = E_FAIL;
		goto cleanup;
	}

	OutputDebugString("FLASH: programming...\n" );

	if ( !FlashProgramImage(ID, ImageBuffer, writeOffset, ImageSize) )
	{
		_snprintf( szResp, cchResp, "Failed to program kernel image (verify failed)" );
		hr = E_FAIL;
		goto cleanup;
	}

	OutputDebugString("FLASH: done\n" );

	if ( SUCCEEDED(hr) )
	{
		_snprintf( szResp, cchResp, "Done, new image flashed" );
	}

	hr = S_OK;

cleanup:

//		__asm sti

	OutputDebugString("FLASH: interrupts are now enabled" );

	MmUnmapIoSpace( KernelRomBase, FLASH_REGION_SIZE );

	//
	// Re-enable ROM caching as needed
	//

	FlashChangeRomCaching( TRUE );

#ifdef FLASH_TIME
	ClockTick = FlashReadTSC() - ClockTick;
	DbgPrint( "FLASH: elapsed time %I64u seconds\n", ClockTick / 733000000UI64 );
#endif

	return hr;
}


bool FlashErase(
				FlashDeviceID ID,
				ULONG address,
				ULONG size
				)
{
	ULONG i;
	bool result;
	char temp[128];
/*	if (size == ROM_SIZE)
		return FlashEraseChip(ID);
	else*/
	{
		for (i = 0; i < size; i += FLASH_SECTOR_SIZE)
		{
			result = FlashEraseSector(ID, address+i);

			sprintf(temp,"Erasing.....");
			RenderProgressBar( (100*(i))/size ,1, 0xa0ff0000, 0xa0000000, temp );

		}
		RenderProgressBar( 100 ,1, 0xa0ff0000, 0xa0000000, "Erase success");
	}
	return result;
}

#endif