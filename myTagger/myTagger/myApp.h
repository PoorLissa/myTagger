#include <vector>
#include <SYS\STAT.H>
#include <windows.h>
#include <stdio.h>

// -----------------------------------------------------------------------------------------------

extern const wchar_t *exeName;

// -----------------------------------------------------------------------------------------------

class myTagger {

	typedef std::map<std::wstring, size_t> streamMap;

	public:
		myTagger();

		void	Set					(std::wstring, std::wstring, bool = false);
		int		Get					(const std::wstring &);
		void	Find				(std::wstring, std::wstring);
		void	Rem					(std::wstring, std::wstring);

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

		template<class T>
		void	parseStr_toVec		(const std::basic_string<T> &, std::vector<std::basic_string<T>> &);

	private:
		std::wstring streamSuffix;
};

// -----------------------------------------------------------------------------------------------

myTagger::myTagger() : streamSuffix(L":mytag.stream")
{
	;
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

		char ch = getchar();

		if( ch != 'y' && ch != 'Y' )
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

	auto flagsRewrite = std::fstream::out;
	auto flagsAppend  = std::fstream::out | std::fstream::app;

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

	std::wstring fName(path);
	fixFileName(fName);
	fName += streamSuffix;

	std::wfstream	file;
	std::wstring	line;

	file.open(fName, std::fstream::in);

	if( file.is_open() )
	{
		while( std::getline(file, line) )
		{
			found += 1;
			std::wcout << L"\t[ " << line << " ]" << std::endl;
		}

		file.close();
	}

	if( !found )
		std::cout << "\t" << "No tags found" << std::endl;

	return found;
}
// -----------------------------------------------------------------------------------------------

// Removes illegal characters from tags
bool myTagger::fixTags(std::wstring &str)
{
	bool res = true;

	wchar_t chars[] = { '[', ']', '*', '\n', '\t' }, ch_old = ' ';

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
				res = false;
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

	if( !tmp.empty() && tmp.back() == ' ' )
		tmp.pop_back();

	str = tmp;

	return res;
}
// -----------------------------------------------------------------------------------------------

// Find item(s) by tag(s) and put them into tmp file for Far Manager to open
void myTagger::Find(std::wstring data, std::wstring path)
{
	fixTags(data);

	std::vector<std::wstring> vec;
	std::wstring str;

	// parse the tags user entered into a vector
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


	// open temp file
	std::wfstream fileTmp;
	std::wstring fileNameTmp(exeName), resStr;
	fileNameTmp += L".temp";

	fileTmp.open(fileNameTmp, std::fstream::out);

	if( fileTmp.is_open() )
	{
		findFiles(path, vec, resStr);
		fileTmp.write(resStr.c_str(), resStr.length());
	}

	fileTmp.close();

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
		// Files do have a data stream, we will skip it
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
	WIN32_FIND_DATA FindFileData;
	streamMap		mapStreams;

	// Recursively find all files starting from the current directory and map all the alternate streams in these files
	findFilesRecursive(path, mapStreams, FindFileData);

	if( mapStreams.size() )
	{
		std::wcout << "   found " << mapStreams.size() << " alternative streams " << std::endl;

		std::vector<const std::wstring *> vec;

		for(auto iter = mapStreams.begin(); iter != mapStreams.end(); ++iter)
		{
			size_t pos1 = iter->first.find(':', 3);
			size_t pos2 = iter->first.length() - 6;

			std::wstring str(iter->first.c_str() + pos1, pos2 - pos1);

			if( iter->first.substr(pos1, pos2 - pos1) == streamSuffix )
			{
				vec.push_back(&iter->first);
			}
		}

		size_t cnt = 0u;

		for(size_t i = 0; i < vec.size(); i++)
		{
			std::wstring data;
			const std::wstring *str = vec[i];

			getStreamData(str, data);

			if( dataHasTags(vecTags, data) )
			{
				size_t pos = str->find(':', 3);

				res += str->substr(0, pos) + L"\n";
				cnt++;
			}
		}

		std::wcout << "   found " << cnt << " tagged files " << std::endl;
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

		while( b )
		{
			std::wstring fileName(FindFileData.cFileName);

			if( fileName != L"." && fileName != L".." )
			{
				fileName = path + fileName;

				bool b_isDir = isDir(fileName.c_str());

				findStreams(fileName, mapStreams, b_isDir);

				if( b_isDir )
				{
					fileName += L"\\";

					if( level == 0 )
						std::wcout << "\t" << fileName << std::endl;;

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
	std::wfstream	file;
	std::wstring	line;

	file.open(*path, std::fstream::in);

	if( file.is_open() )
	{
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

	for(size_t i = 0; i < tags.size(); i++)
	{
		if( data.find(tags[i]) == std::wstring::npos )
		{
			res = false;
			break;
		}
	}

	return res;
}
// -----------------------------------------------------------------------------------------------

// Removes tag(s) from alt stream
void myTagger::Rem(std::wstring data, std::wstring path)
{
	std::wstring str_old, str_new, streamPath;
	std::vector<std::wstring> vecTags, vecToRemove;

	if( data == L"*" )
	{
		std::wcout << " ---> You are trying to remove ALL the tags from this object." << std::endl;
		std::wcout << " ---> If you want to proceed, say 'Y': ";

		char ch = getchar();

		if( ch == 'y' || ch == 'Y' )
		{
			// Remove all tags
			deleteStream(path);
			return;
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

	std::wcout << " ---> All tag(s) removed. No tags left:\n\t[ ]" << std::endl;

	return;
}
// -----------------------------------------------------------------------------------------------
