#ifndef SCANFOLDER_H
#define SCANFOLDER_H

#include <windows.h>

//#define BORLAND_C

#if defined BORLAND_C
#include <dir.h>
/*
 Параметр Attr определяет атрибуты файла (см. Help про структуру ffblk).
 Пользоваться этим классом очень просто:
 Например, вы хотите опросить текущий каталог и считать имена всех файлов с расширением *.cpp в ListBox1.

#include "__ScanFolder.h"
AnsiString Mask = "*.cpp";
for(CScanFolder SF((char*)Mask.c_str(), 0); SF; SF++)
	ListBox1->Items->Add( AnsiString(SF) ); */
class CScanFolder : public ffblk{
 public:
	__fastcall CScanFolder(const char *fileName = "*.*", int Attr = 0) {
		Find = findfirst(fileName, this, Attr);
	}

	operator const char*() {
		return Find ? 0 : ff_name;
	}

	int operator++(int) {
		// returns 0 if OK, -1 if no matching files found or on error
		return( Find = findnext(this) );
	}

	int operator++() {
		return operator++(0);
	}

	__fastcall ~CScanFolder(void) {
		findclose(this);
	}

 private:
	int Find;
};
#endif

// --------------------------------------------------------------------------------------------------

/*
 Параметр Attr определяет атрибуты файла (см. Help про структуру ffblk).
 Пользоваться этим классом очень просто:
 Например, вы хотите опросить текущий каталог и считать имена всех файлов с расширением *.cpp:

#include "__ScanFolder.h"
QString Mask = "*.cpp";
for(CScanFolder SF(Mask, 0); SF; SF++)
	QString res = QString::fromWCharArray(SF);
*/
class CScanFolder : WIN32_FIND_DATA {
 public:
	 __fastcall CScanFolder(const WCHAR *fileName = TEXT("*.*"), int Attr = 0) : found(false) {
		 if (hFind = FindFirstFile(fileName, this))
			 found = true;
	 }

	 __fastcall CScanFolder(const QString fileName = "*.*", int Attr = 0) : found(false) {
		 WCHAR* str = (WCHAR*)(fileName.data());
		 if( hFind = FindFirstFile(str, this) )
			found = true;
	 }

	 operator const WCHAR* () {
		 return found ? cFileName : 0;
	 }

	 void operator++ (int) {
		 found = FindNextFile(hFind, this);
	 }

	 void operator++ () {
		 operator++(0);
	 }

	 __fastcall ~CScanFolder(void) {
		 FindClose(hFind);
	 }

 private:
	 HANDLE hFind;
	 bool found;
};
#endif

/*
findnext()               Fetch Files That Match findfirst()

#include   <dir.h>

int           findnext(ffblk);
struct ffblk  *findnext;

findnext() is used to fetch subsequent files that match the
'pathname' specified in findfirst().  'ffblk' still contains all the
necessary information about the file for continuing the search. One
file name for each call to findnext() will be returned until no more
matching files are found in the directory. The format of the
structure 'ffblk' is as follows:

struct ffblk  {
	char ff_reserved[21];         // Reserved by DOS
	char ff_attrib;               // Attribute found
	int ff_ftime;                 // File time
	int ff_fdate;                 // File date
	long ff_fsize;                // File size
	char ff_name[13];             // Found file name
};

Returns: 0 is returned if a file matching 'pathname' is found.
If no more files can be found or on error, a - 1 is returned
and 'errno' (defined in <stdlib.h>) is set to either :

ENOENT          Path or file name not found
ENMFILE         No more files.

*/

