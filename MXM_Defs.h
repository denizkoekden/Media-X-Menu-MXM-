/*********************************************************************************\
 * MXM_Defs.h
 * Class used to hold images and fonts
 * (C) 2002 MasterMind
\*********************************************************************************/

#if !defined(__MXM_DEFS_H__)
#define __MXM_DEFS_H__

#include <map>
#include <iostream>
#include <string>
#include <list>
#include "XBFont_mxm.h"
#include "StdString.h"
#include "ImageSource.h"

using namespace std;

const LPCTSTR c_szEmpty = _T("");

typedef list<CStdString> TListStrings;
typedef map<string, string> TMapStrings;
typedef map<string, TMapStrings> TMapSections;

typedef map<int, CImageSource *>	TMapImages;
typedef map<int, CXBFont *>			TMapFonts;
typedef map<int, CStdString>		TMapNames;

#define WORKER_THREAD_STACK_SIZE 12 * 1024


// Keep time on button push to see button being held down
__forceinline ULONGLONG ReadTimeStampCounter()
{
   _asm rdtsc
}




#endif