// myTagger.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <iostream>
#include <SYS\STAT.H>

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <Windows.h>
#include <algorithm>
#include <map>
#include <fstream>
#include <vector>
#include <iomanip>

#include <codecvt>
#include <cvt/wstring>
#include <iostream>
#include <locale>
#include <string>
#include <codecvt>

const wchar_t *exeName = nullptr;

// -----------------------------------------------------------------------------------------------------------------------

bool isDir(const wchar_t *path)
{
	struct _stat s;

	if( _wstat(path, &s) == 0 )
	{
		if( s.st_mode & S_IFDIR )
			return true;

		if( s.st_mode & S_IFREG )
			return false;
	}

	return false;
}
// -----------------------------------------------------------------------------------------------------------------------

void fixFileName_Set(std::wstring &str)
{
	size_t len = str.length();

	if( len > 3 && str[len-1] == '.' && str[len-2] == '.' && str[len-3] == '\\' )
		str = str.substr(0, len -3);

	if( isDir(str.c_str()) && str.back() != '\\' )
		str += '\\';
}
// -----------------------------------------------------------------------------------------------------------------------

void fixTags(std::wstring &str)
{
	wchar_t chars[] = { '[', ']', '\n', '\t' }, ch_old = ' ';

	size_t N = sizeof(chars)/sizeof(chars[0]);

	std::wstring tmp;

	for(size_t i = 0u; i < str.length(); i++)
	{
		wchar_t ch = str[i];

		for(size_t j = 0u; j < N; j++)
		{
			if( ch == chars[j] )
			{
				ch = L' ';
				break;
			}
		}

		if( ch_old == ' ' && ch == ' ' )
		{
			continue;
		}
		else
		{
			tmp += ch;
			ch_old = ch;
		}
	}

	if( tmp.back() == ' ' )
		tmp.pop_back();

	str = tmp;
}
// -----------------------------------------------------------------------------------------------------------------------

bool fileContainsPath(std::wstring &fileName, const std::wstring &path)
{
	std::wstring line;
	std::wfstream file;

	file.open(fileName, std::fstream::in);

	if( !file.is_open() )
	{
		return false;
	}
	else
	{
		while( std::getline(file, line) )
		{
			if( line == path )
			{
				file.close();
				return true;
			}
		}
	}

	file.close();
	return false;
}
// -----------------------------------------------------------------------------------------------------------------------

void set(std::wstring fName, std::wstring fData)
{
	std::wfstream	file;
	std::wstring	fileName(exeName);
					fileName += L".db";

	fixFileName_Set(fName);

	if( fileContainsPath(fileName, fName) )
	{
		std::map<std::wstring, std::wstring> mMap;

		file.open(fileName, std::fstream::in);
		{
			std::wstring line, line1;

			while( std::getline(file, line) )
			{
				if( line[0] == '[' )
					mMap[line1] = line;
				else
					line1 = line;
			}
		}
		file.close();

		file.open(fileName, std::fstream::out);
		{
			for(auto iter = mMap.begin(); iter != mMap.end(); ++iter)
			{
				file.write(iter->first.c_str(), iter->first.length());
				file.write(L"\n", 1);

				if( iter->first == fName )
				{
					std::wstring *str = &iter->second;
					str->pop_back();
					fixTags(fData);
					*str += L" " + fData + L"]";
				}

				file.write(iter->second.c_str(), iter->second.length());
				file.write(L"\n", 1);
			}
		}
		file.close();
	}
	else
	{
		// If the file does not exist, we create it
		file.open(fileName, std::fstream::in | std::fstream::out | std::fstream::app);

		std::wstring str = fName + L"\n[" + fData + L"]\n";

		file.write(str.c_str(), str.length());
		file.close();
	}
}
// -----------------------------------------------------------------------------------------------------------------------

void view(const std::wstring &path)
{
	std::wfstream	file;
	std::wstring	line;
	std::wstring	fileName(exeName);
					fileName += L".db";

	file.open(fileName, std::fstream::in);

	if( file.is_open() )
	{
		while( std::getline(file, line) )
		{
			if( line == path )
			{
				std::getline(file, line);
				file.close();

				std::wcout << "\t" << line << std::endl;
				return;
			}
		}
	}

	std::wcout << "\t" << "Not tags found" << std::endl;
	return;
}
// -----------------------------------------------------------------------------------------------------------------------

void get(std::wstring data, std::wstring path)
{
	fixTags(data);

	std::vector<std::wstring> vec;
	std::wstring str;

	for(size_t i = 0; i < data.length(); i++)
	{
		wchar_t ch = data[i];

		if( ch == L' ' )
		{
			vec.push_back(str);
			str.clear();
		}
		else
		{
			str.push_back(ch);
		}
	}
	vec.push_back(str);


	std::wfstream fileTmp;
	std::wstring fileNameTmp(exeName), resStr;
	fileNameTmp += L".temp";

	fileTmp.open(fileNameTmp, std::fstream::out);

	if( fileTmp.is_open() )
	{
		std::wfstream file;
		std::wstring fileName(exeName), line_path, line;
		fileName += L".db";

		file.open(fileName, std::fstream::in);

		while( std::getline(file, line) )
		{
			if( line[0] != '[' )
			{
				line_path = line;
			}
			else
			{
				bool found = true;

				for(size_t i = 0; i < vec.size(); i++)
				{
					if( line.find(vec[i]) == std::string::npos )
					{
						found = false;
						break;
					}
				}

				if( found )
				{
					std::wcout << " == path = " << path << std::endl;
					std::wcout << " == line = " << line_path << std::endl;

					if( line_path.find(path) != std::string::npos )
					{
						resStr += line_path;
						resStr += L"\n";
					}
				}
			}
		}

		file.close();

		fileTmp.write(resStr.c_str(), resStr.length());
	}

	fileTmp.close();
}

// =======================================================================================================================

int _tmain(int argc, _TCHAR* argv[])
{
	int res = 0;

	exeName = argv[0];

	if( argc > 1 )
	{
		std::wstring arg1 = argv[1];
		std::wstring data;

		do {

			if( arg1 == L"/set" && argc > 2 )
			{
				std::wstring fileName(argv[2] + 6);

				std::wcout << " ---> Selected target is: '" << fileName << "'" << std::endl;
				std::wcout << " ---> Input your tag(s) to set: ";
				std::getline(std::wcin, data);

				set(fileName, data);
				break;
			}

			if( arg1 == L"/get" && argc > 2 )
			{
				std::wstring dirName(argv[2] + 6);

				std::wcout << " ---> Current path is: '" << dirName << "'" << std::endl;
				std::wcout << " ---> Input your tag(s) to find: ";
				std::getline(std::wcin, data);

				get(data, dirName);
				break;
			}

			if( arg1 == L"/view" && argc > 2 )
			{
				std::wstring dirName(argv[2] + 6);

				fixFileName_Set(dirName);

				std::wcout << " ---> Current path is: '" << dirName << "'" << std::endl;
				std::wcout << " ---> Tags found:" << std::endl;;

				view(dirName);
				break;
			}

			std::wcout << " ---> Missing the first verb. Supported verbs are: [/set, /get]. Exiting... " << std::endl;;
			res = 1;

		}
		while( false );

	}
	else
	{
		res = 1;
		std::wcout << " ---> Too few parameters. Exiting... " << std::endl;
	}

	return res;
}

