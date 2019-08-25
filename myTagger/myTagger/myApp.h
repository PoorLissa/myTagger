#ifndef MY_APP_H
#define MY_APP_H
#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <stdio.h>
#include <SYS\STAT.H>
#include <windows.h>
#include <locale>

// -----------------------------------------------------------------------------------------------

class myTagger {

	typedef std::map<std::wstring, size_t> streamMap;

	public:
		myTagger(const wchar_t *);

		void	Set					(std::wstring, std::wstring, bool = false);
		int		Get					(const std::wstring &);
		void	Find				(std::wstring, std::wstring);
		void	Rem					(std::wstring, std::wstring);
		void	doPrint				(const std::wstring);

	private:
		bool	isDir				(const wchar_t *);
		void	fixFileName			(std::wstring &);
		bool	fixTags				(std::wstring &);
		void	findFiles			(std::wstring, std::vector<std::wstring> &, std::wstring &);
		void	findFilesRecursive	(std::wstring, streamMap &, WIN32_FIND_DATA &, int level = 0);
		void	findStreams			(const std::wstring &, streamMap &, bool);
		void	getStreamData		(const std::wstring *, std::wstring &);
		bool	dataHasTags			(std::vector<std::wstring> &, std::wstring &);
		void	deleteStream		(std::wstring);
		void	buildTagCloud		(const std::wstring &, streamMap &);
		void	showTagCloud		(const streamMap &);

		template<class T>
		void	parseStr_toVec		(const std::basic_string<T> &, std::vector<std::basic_string<T>> &);

	private:
		std::wstring streamSuffix;
		const wchar_t *exeName;
};

// -----------------------------------------------------------------------------------------------

myTagger::myTagger(const wchar_t *exe) : exeName(exe), streamSuffix(L":mytag.stream")
{
	;
}
// -----------------------------------------------------------------------------------------------

void myTagger::doPrint(const std::wstring str)
{
	char bufFile[MAX_PATH];
	WideCharToMultiByte(CP_INSTALLED, 0, str.c_str(), -1, bufFile, MAX_PATH, NULL, NULL);
	std::cout << bufFile;
}
// -----------------------------------------------------------------------------------------------

template<class T>
void myTagger::parseStr_toVec(const std::basic_string<T> &data, std::vector<std::basic_string<T>> &vec)
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
// -----------------------------------------------------------------------------------------------

bool myTagger::isDir(const wchar_t *path)
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
// -----------------------------------------------------------------------------------------------

void myTagger::fixFileName(std::wstring &str)
{
	size_t len = str.length();

	if( len > 3 && str[len-1] == '.' && str[len-2] == '.' && str[len-3] == '\\' )
		str = str.substr(0, len -3);

	if( isDir(str.c_str()) && str.back() == '\\' )
		str.pop_back();
}
// -----------------------------------------------------------------------------------------------

// Appends or rewrites data in tags stream
void myTagger::Set(std::wstring fName, std::wstring fData, bool doRewriteAllTags /* = false*/)
{
	if( !fixTags(fData) )
	{
		std::wcout << " ---> Tag(s) contained illegal characters and were fixed. New set of tags is:\n\t[ " << fData << " ]" << std::endl;
		std::wcout << " ---> If you want to proceed with this new set of tags, say 'Y': ";

		std::wstring s;
		std::wcin >> s;

		if( s[0] != L'y' && s[0] != L'Y' )
		{
			std::wcout << " ---> Cancelled by user. Exiting..." << std::endl;
			return;
		}
	}

	if( fData.empty() )
	{
		std::wcout << " ---> New set of tags is empty. Exiting..." << std::endl;
		return;
	}

	DWORD dw;
	bool  isReadOnly = false;

	auto flagsRewrite = std::wfstream::out;
	auto flagsAppend  = std::wfstream::out | std::wfstream::app;

	std::wfstream file;
	std::wstring  oldData;

	fixFileName(fName);
	fName += streamSuffix;

	getStreamData(&fName, oldData);

	auto flags = (doRewriteAllTags || oldData.empty())
					? flagsRewrite
					: flagsAppend;

	file.open(fName, flags);

	if( !file.is_open() )
	{
		// Check attributes -- if the file is ReadOnly, remove this flag and restore it later
		dw = GetFileAttributesW(fName.c_str());

		if( dw & FILE_ATTRIBUTE_READONLY )
		{
			isReadOnly = true;

			if( SetFileAttributesW(fName.c_str(), dw ^ FILE_ATTRIBUTE_READONLY) )
			{
				file.open(fName, flags);
			}
		}
	}

	if( file.is_open() )
	{
		if( flags == flagsAppend )
			file.write(L" ", 1u);

		file.imbue(std::locale("rus_rus.866"));

		file.write(fData.c_str(), fData.length());
		file.close();

		if( isReadOnly )
			SetFileAttributesW(fName.c_str(), dw | FILE_ATTRIBUTE_READONLY);
	}
	else
	{
		std::wcout << " Error: Can't open stream '" << fName << "'" << std::endl;
	}

	return;
}
// -----------------------------------------------------------------------------------------------

int myTagger::Get(const std::wstring &path)
{
	int found = 0;

	std::wfstream file;
	std::wstring  fName(path), line;

	fixFileName(fName);
	fName += streamSuffix;

	file.open(fName, std::wfstream::in);

	if( file.is_open() )
	{
		file.imbue(std::locale("rus_rus.866"));

		while( std::getline(file, line) )
		{
			found += 1;
			std::wcout << L"\t[ " << line << " ]" << std::endl;
		}

		file.close();
	}

	if( !found )
		std::wcout << "\t" << "No tags found" << std::endl;

	return found;
}
// -----------------------------------------------------------------------------------------------

// Removes illegal characters from tags
bool myTagger::fixTags(std::wstring &str)
{
	wchar_t chars_illegal[] = { '[', ']', '(', ')', '*', '\n', '\t' }, old = ' ';

	bool res = true;
	std::wstring tmp;
	size_t i = 0;

	// Skip whitespaces and '|'
	while( str[i] == ' ' || str[i] == '|' )
		i++;

	for(; i < str.length(); i++)
	{
		wchar_t ch = str[i];

		for(size_t j = 0u; j < sizeof(chars_illegal)/sizeof(chars_illegal[0]); j++)
		{
			if( ch == chars_illegal[j] )
			{
				ch = L' ';
				res = false;
				break;
			}
		}

		if( old == ' ' && ch == ' ' )
		{
			continue;
		}
		else
		{
			if( ch == '|' )
			{
				if( tmp.empty() )
					continue;

				if( old == '|' )
				{
					tmp += ' ';
					continue;
				}

				if( old != ' ' )
					tmp += ' ';
			}
			else
			{
				if( old == '|' )
					tmp += ' ';
			}

			tmp += ch;
			old  = ch;
		}
	}

	// Clear all spaces and '|' from the tail, if any
	while( !tmp.empty() && (tmp.back() == ' ' || tmp.back() == '|'))
		tmp.pop_back();

	str = tmp;

	return res;
}
// -----------------------------------------------------------------------------------------------

// Find item(s) by tag(s) and put them into tmp file for Far Manager to open
void myTagger::Find(std::wstring data, std::wstring path)
{
	fixTags(data);

	std::wfstream				fileTmp;
	std::vector<std::wstring>	vec;
	std::wstring				str, fileNameTmp(exeName), resStr;
								fileNameTmp += L".temp";

	// parse the tags entered by the user into a vector
	for(size_t i = 0; i < data.length(); i++)
	{
		wchar_t ch = data[i];

		if( ch == L' ' )
		{
			if( !str.empty())
				vec.push_back(str);

			str.clear();
		}
		else
		{
			str.push_back(ch);
		}
	}
	vec.push_back(str);

	std::wcout << " ---> Search query is: [ ";
	for(size_t i = 0; i < vec.size(); i++)
		std::wcout << vec[i] << " ";
	std::wcout << "]" << std::endl;

	// open temp file and do the seach
	fileTmp.open(fileNameTmp, std::fstream::out);

	if( fileTmp.is_open() )
	{
		// wfstream won't write cyrillic symbols into the file unless we set up the locale.
		// "rus_rus.1251" stands for Far's OEM format, which is unexpected by Far.
		// So we use "rus_rus.866", which stands for Far's ANSI (which is just fine).
		fileTmp.imbue(std::locale("rus_rus.866"));

		findFiles(path, vec, resStr);
		fileTmp.write(resStr.c_str(), resStr.length());
		fileTmp.close();
	}
	else
	{
		std::wcout << " ---> Error: could not open tmp file '" << fileNameTmp << "'" << std::endl;
		std::wcout << " ---> Exiting..." << std::endl;
	}

	return;
}
// -----------------------------------------------------------------------------------------------

// Find alternate streams for a given object (dir or file)
void myTagger::findStreams(const std::wstring &path, streamMap &map, bool isDir)
{
	WIN32_FIND_STREAM_DATA wfsd;

	HANDLE h = FindFirstStreamW(path.c_str(), STREAM_INFO_LEVELS::FindStreamInfoStandard, &wfsd, NULL);

	if( h != INVALID_HANDLE_VALUE )
	{
		BOOL b = TRUE;

		// Directories do not have data streams, so the first found stream will be an alternate one
		// Files _do_ have a data stream, so we skip it
		if( isDir )
			map[path + wfsd.cStreamName] = size_t(wfsd.StreamSize.QuadPart);

		while( b )
		{
			b = FindNextStreamW(h, &wfsd);

			if( b )
			{
				map[path + wfsd.cStreamName] = size_t(wfsd.StreamSize.QuadPart);
			}
		}

		FindClose(h);
	}

	return;
}
// -----------------------------------------------------------------------------------------------

// 
void myTagger::findFiles(std::wstring path, std::vector<std::wstring> &vecTags, std::wstring &res)
{
	WIN32_FIND_DATA wfd;
	streamMap		mapStreams, mapTagCloud;
	size_t			cnt = 0u;


	// Recursively find all files starting with the current directory
	// Map all the alternate streams in these files
	findFilesRecursive(path, mapStreams, wfd);


	if( mapStreams.size() )
	{
		std::vector<const std::wstring *> vec;
		size_t size = 0u;

		// Put our tagged streams into separate vector
		for(auto iter = mapStreams.begin(); iter != mapStreams.end(); ++iter)
		{
//			std::wcout << " mapStreams: " << iter->first << "; size = " << iter->second << std::endl;

			size_t pos1 = iter->first.find(':', 2);
			size_t pos2 = iter->first.length() - 6;

			std::wstring str(iter->first.c_str() + pos1, pos2 - pos1);

			if( iter->first.substr(pos1, pos2 - pos1) == streamSuffix )
			{
				vec.push_back(&iter->first);
			}

			size += iter->second;
		}

		std::wcout << " ---> Found " << mapStreams.size() << " alternative NTFS stream(s); Total Size = " << size << " bytes" << std::endl;

		// Extract objects with tags matching the search criteria
		for(size_t i = 0; i < vec.size(); i++)
		{
			std::wstring data;
			const std::wstring *str = vec[i];

			getStreamData(str, data);

			buildTagCloud(data, mapTagCloud);

			if( dataHasTags(vecTags, data) )
			{
				size_t pos = str->find(':', 2);

				res += str->substr(0, pos) + L"\n";
				cnt++;
			}
		}

		std::wcout << " ---> Found " << cnt << " tagged object(s)" << std::endl;

		// Display the tag cloud
		if( cnt )
			showTagCloud(mapTagCloud);
	}
	else
	{
		std::wcout << " ---> Alternative NTFS streams not found" << std::endl;
	}

	return;
}
// -----------------------------------------------------------------------------------------------

// Recursive search for alternate file streams
// https://www.codeproject.com/Articles/13667/Enumerating-Alternate-Data-Streams
// https://ws680.nist.gov/publication/get_pdf.cfm?pub_id=50914
void myTagger::findFilesRecursive(std::wstring path, streamMap &mapStreams, WIN32_FIND_DATA &FindFileData, int level /* = 0*/)
{
	std::wstring searchPath = path + L"*";

	HANDLE h = FindFirstFileW(searchPath.c_str(), &FindFileData);

	if( h != INVALID_HANDLE_VALUE )
	{
		BOOL b = TRUE;
		bool b_isDir;
		std::wstring fileName;

		while( b )
		{
			fileName = FindFileData.cFileName;

			if( fileName != L"." && fileName != L".." )
			{
//				std::wcout << " found file: " << fileName << std::endl;

				fileName = path + fileName;

				b_isDir = isDir(fileName.c_str());

				findStreams(fileName, mapStreams, b_isDir);

				if( b_isDir )
				{
					fileName += L"\\";

					if( level == 0 )
						std::wcout << "\t" << fileName << std::endl;

					findFilesRecursive(fileName, mapStreams, FindFileData, level+1);
				}
			}

			b = FindNextFileW(h, &FindFileData);
		}

		FindClose(h);
	}

	return;
}
// -----------------------------------------------------------------------------------------------

// Get data from our tag stream
void myTagger::getStreamData(const std::wstring *path, std::wstring &data)
{
	std::wfstream file;
	std::wstring  line;

	file.open(*path, std::fstream::in);

	if( file.is_open() )
	{
		file.imbue(std::locale("rus_rus.866"));

		while( std::getline(file, line) )
		{
			data += line;
		}

		file.close();
	}

	return;
}
// -----------------------------------------------------------------------------------------------

// Checks if the data from the tag stream contains the tags we're searching for
bool myTagger::dataHasTags(std::vector<std::wstring> &tags, std::wstring &data)
{
	bool res = true;

	// First try, AND logic
#if 0
	for(size_t i = 0; i < tags.size(); i++)
	{
		if( data.find(tags[i]) == std::wstring::npos )
		{
			res = false;
			break;
		}
	}
#endif

	// Second try, AND / OR logic
	const size_t size = tags.size();

	for(size_t i = 0; i < size; i++)
	{
		wchar_t ch = tags[i][0];

		// if OR, just skip it
		if( ch == '|' )
			continue;

		// if tag is not found:
		if( data.find(tags[i]) == std::wstring::npos )
		{
			// if the next tag is OR => proceed, but set the result to false
			// if the next iteration succeeds, the result will be set to true
			if( i < (size - 1) )
			{
				ch = tags[i+1][0];

				if( ch == '|' )
				{
					if( i > 0 && tags[i-1][0] == '|' )
						continue;

					res = false;
					continue;
				}
			}

			// if the next is not OR, but the one before was OR => check if the tag was found
			// if it was, then procees to the next one
			// if it was not, then the search failed
			if( i > 0 )
			{
				ch = tags[i-1][0];

				if( ch == '|' )
				{
					if( res )
						continue;
				}
			}

			res = false;
			break;
		}

		res = true;
	}

	return res;
}
// -----------------------------------------------------------------------------------------------

// Removes tag(s) from alternative NTFS stream
void myTagger::Rem(std::wstring data, std::wstring path)
{
	std::wstring str_old, str_new, streamPath;
	std::vector<std::wstring> vecTags, vecToRemove;

	if( data == L"*" )
	{
		std::wcout << " ---> You are trying to remove ALL the tags from this object." << std::endl;
		std::wcout << " ---> If you want to proceed, say 'Y': ";

		std::wcin >> str_old;

		if( str_old[0] == L'y' || str_old[0] == L'Y' )
		{
			// Remove all tags
			deleteStream(path);
		}
	}
	else
	{
		streamPath = path + streamSuffix;
		getStreamData(&streamPath, str_old);

		parseStr_toVec(str_old, vecTags);
		parseStr_toVec(data,    vecToRemove);

		for(size_t i = 0; i < vecTags.size(); i++)
		{
			bool doRemove = false;

			for(size_t j = 0; j < vecToRemove.size(); j++)
			{
				if( vecTags[i] == vecToRemove[j] )
				{
					doRemove = true;
					break;
				}
			}

			if( !doRemove )
			{
				if( str_new.size() )
					str_new += ' ';

				str_new += vecTags[i];
			}
		}

		if( str_old != str_new )
		{
			if( str_new.length() )
			{
				// Change tags
				Set(path, str_new, true);
				std::wcout << " ---> Tag(s) removed. Tag(s) left:\n\t[ " << str_new << " ]" << std::endl;
			}
			else
			{
				// Remove all tags
				deleteStream(path);
			}
		}
		else
		{
			std::wcout << " ---> Tag(s) not found. Nothing has changed" << std::endl;
		}
	}

	return;
}
// -----------------------------------------------------------------------------------------------

// Physically deletes the stream
void myTagger::deleteStream(std::wstring path)
{
	std::wstring streamPath = path + streamSuffix;

	DeleteFileW(streamPath.c_str());

	std::wcout << " ---> All tag removed. No tags left:\n\t[ ]" << std::endl;

	return;
}
// -----------------------------------------------------------------------------------------------

// Map all the found tags and their quantity
void myTagger::buildTagCloud(const std::wstring &str, streamMap &map)
{
	std::vector<std::wstring> vec;

	parseStr_toVec(str, vec);

	for(size_t i = 0; i < vec.size(); i++)
	{
		size_t cnt = map.count(vec[i]);

		if( !cnt )
			map[vec[i]] = 1;
		else
			map[vec[i]] = map[vec[i]] + 1;
	}

	return;
}
// -----------------------------------------------------------------------------------------------

// Print tag cloud contents in a sorted manner
void myTagger::showTagCloud(const streamMap &map)
{
	size_t min = 0u;
	std::wcout << " ---> Tag Cloud:";
	std::multimap<size_t, const std::wstring *> mMap;

	for(auto iter = map.begin(); iter != map.end(); ++iter)
		mMap.insert( std::make_pair(iter->second, &iter->first) );

	for(auto iter = mMap.begin(); iter != mMap.end(); ++iter)
	{
		if( iter->first > min )
		{
			std::wcout << (min ? "]" : "") << std::endl;
			std::wcout.width(6);
			std::wcout << std::right << (min = iter->first) << " :: [ ";
		}

		std::wcout << *iter->second << " ";
	}

	std::wcout << "]" << std::endl;

	return;
}
// -----------------------------------------------------------------------------------------------

#endif
