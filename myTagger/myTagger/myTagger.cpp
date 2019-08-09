// myTagger.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <Windows.h>
#include <SYS\STAT.H>

const wchar_t *exeName = nullptr;

// -----------------------------------------------------------------------------------------------------------------------

void doPrint(const std::wstring str)
{
	char bufFile[MAX_PATH];
	WideCharToMultiByte(CP_INSTALLED, 0, str.c_str(), -1, bufFile, MAX_PATH, NULL, NULL);
	std::cout << bufFile;
}
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
	std::string line;
	std::fstream file;

	file.open(fileName, std::fstream::in);

	if( !file.is_open() )
	{
		return false;
	}
	else
	{
		char buf[MAX_PATH];
		WideCharToMultiByte(CP_INSTALLED, 0, path.c_str(), -1, buf, MAX_PATH, NULL, NULL);

		while( std::getline(file, line) )
		{
			if( line == buf )
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
	std::fstream	file;
	std::wstring	fileName(exeName);
					fileName += L".db";

	fixFileName_Set(fName);

	if( fileContainsPath(fileName, fName) )
	{
		std::map<std::string, std::string> mMap;

		file.open(fileName, std::fstream::in);
		{
			std::string line, line1;

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
			char buf[MAX_PATH];
			WideCharToMultiByte(CP_INSTALLED, 0, fName.c_str(), -1, buf, MAX_PATH, NULL, NULL);

			for(auto iter = mMap.begin(); iter != mMap.end(); ++iter)
			{
				file.write(iter->first.c_str(), iter->first.length());
				file.write("\n", 1);

				if( iter->first == buf )
				{
					char bufData[MAX_PATH];
					WideCharToMultiByte(CP_INSTALLED, 0, fData.c_str(), -1, bufData, MAX_PATH, NULL, NULL);

					std::string *str = &iter->second;
					str->pop_back();
					fixTags(fData);
					*str += " ";
					*str += bufData;
					*str += "]";
				}

				file.write(iter->second.c_str(), iter->second.length());
				file.write("\n", 1);
			}
		}
		file.close();
	}
	else
	{
		std::wstring str = fName + L"\n[" + fData + L"]\n";

		file.open(fileName, std::fstream::in | std::fstream::out | std::fstream::app);

		char bufFile[MAX_PATH];
		WideCharToMultiByte(CP_INSTALLED, 0, str.c_str(), -1, bufFile, MAX_PATH, NULL, NULL);
		file.write(bufFile, strlen(bufFile));

		file.close();
	}
}
// -----------------------------------------------------------------------------------------------------------------------

void view(const std::wstring &path)
{
	bool found = false;

	std::fstream	file;
	std::string		line;
	std::wstring	fileName(exeName);
					fileName += L".db";

	file.open(fileName, std::fstream::in);

	if( file.is_open() )
	{
		char buf[MAX_PATH];
		WideCharToMultiByte(CP_INSTALLED, 0, path.c_str(), -1, buf, MAX_PATH, NULL, NULL);

		while( std::getline(file, line) )
		{
			if( line == buf )
			{
				found = true;
				std::getline(file, line);
				std::cout << "\t" << line << std::endl;
				break;
			}
		}

		file.close();
	}

	if( !found )
		std::cout << "\t" << "No tags found" << std::endl;

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

				std::wcout << " ---> Selected target is: '";
				doPrint(fileName);
				std::wcout << "'" << std::endl;
				std::wcout << " ---> Input your tag(s) to set: ";
				std::getline(std::wcin, data);

				set(fileName, data);
				break;
			}

			if( arg1 == L"/get" && argc > 2 )
			{
				std::wstring dirName(argv[2] + 6);

				std::wcout << " ---> Current path is: '";
				doPrint(dirName);
				std::wcout << "'" << std::endl;
				std::wcout << " ---> Input your tag(s) to find: ";
				std::getline(std::wcin, data);

				get(data, dirName);
				break;
			}

			if( arg1 == L"/view" && argc > 2 )
			{
				std::wstring dirName(argv[2] + 6);

				fixFileName_Set(dirName);

				std::wcout << " ---> Current path is: '";
				doPrint(dirName);
				std::wcout << "'" << std::endl;
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

