#ifndef MY_APP_OLD_H
#define MY_APP_OLD_H
#pragma once

#include <vector>
#include <SYS\STAT.H>
#include <Windows.h>

// -----------------------------------------------------------------------------------------------------------------------

extern const wchar_t *exeName;

// -----------------------------------------------------------------------------------------------------------------------

#if 0

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

	if( arg1 == L"/rem" && argc > 2 )
	{
		std::wstring dirName(argv[2] + 6);

		fixFileName_Set(dirName);

		std::wcout << " ---> Current path is: '";
		doPrint(dirName);
		std::wcout << "'" << std::endl;
		std::wcout << " ---> Tags found:" << std::endl;;

		if( view(dirName) )
		{
			std::wcout << std::endl;

			std::wcout << " ---> Input your tag(s) to remove: ";
			std::getline(std::wcin, data);

			rem(data, dirName);
		}
		break;
	}
#endif

// -----------------------------------------------------------------------------------------------------------------------

template<class T>
void parseStr_toVec(const std::basic_string<T> &data, std::vector<std::basic_string<T>> &vec)
{
	std::basic_string<T> str;
	vec.clear();

	for(size_t i = 0; i < data.length(); i++)
	{
		T ch = data[i];

		if( ch == T(' ') )
		{
			if( str.length() )
				vec.push_back(str);

			str.clear();
		}
		else
		{
			str.push_back(ch);
		}
	}
	vec.push_back(str);

	return;
}
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

void set(std::wstring fName, std::wstring fData, bool doRewriteAllTags = false)
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

					if( doRewriteAllTags )
					{
						*str = "[";
					}
					else
					{
						str->pop_back();
						fixTags(fData);
						*str += " ";
					}

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

int view(const std::wstring &path)
{
	int found = 0;

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
				found = 1;
				std::getline(file, line);
				std::cout << "\t" << line << std::endl;
				break;
			}
		}

		file.close();
	}

	if( !found )
		std::cout << "\t" << "No tags found" << std::endl;

	return found;
}
// -----------------------------------------------------------------------------------------------------------------------

// Find item(s) by tag(s) and put them into tmp file for Far Manager to open
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
// -----------------------------------------------------------------------------------------------------------------------

void rem(std::wstring data, std::wstring path)
{
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
				std::getline(file, line);
				break;
			}
		}

		file.close();
	}

	if( line.length() )
	{
		fixTags(data);

		line = line.substr(1, line.length() - 2);

		std::vector<std::string> vecTags, vecTagsToRemove;
		parseStr_toVec(line, vecTags);

		char *buf = new char[data.length() + 1];
		WideCharToMultiByte(CP_INSTALLED, 0, data.c_str(), -1, buf, data.length() + 1, NULL, NULL);

		line = buf;

		delete [] buf;

		parseStr_toVec(line, vecTagsToRemove);

		line.clear();

		for(size_t i = 0; i < vecTags.size(); i++)
		{
			bool doAdd = true;

			for(size_t j = 0; j < vecTagsToRemove.size(); j++)
			{
				if( vecTags[i] == vecTagsToRemove[j] )
				{
					doAdd = false;
					break;
				}
			}

			if( doAdd )
			{
				if( line.size() )
					line += ' ';

				line += vecTags[i];
			}
		}

		// rewrite the whole line in file
		if( line.length() )
		{
			wchar_t *buf = new wchar_t[line.length() + 1];

			MultiByteToWideChar(CP_OEMCP, 0, line.c_str(), -1, buf, line.length() + 1);

			std::wstring wData(buf);
			delete [] buf;

			set(path, wData, true);
		}
	}
}

#endif
