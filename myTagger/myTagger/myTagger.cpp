// myTagger.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "myApp.h"
#include <io.h>
#include <fcntl.h>

// =======================================================================================================================

int _tmain(int argc, _TCHAR* argv[])
{
	int res = 0;

	// Add support for Russian lang in console
	// The second thing to do is to imbue fstreams when writing data to the file (see 'myTagger::Get').
	// https://ru.stackoverflow.com/questions/459154/Русский-язык-в-консоли
	// http://it-negr.blogspot.com/2012/03/c.html
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno( stdin), _O_U16TEXT);
	_setmode(_fileno(stderr), _O_U16TEXT);

	if( argc > 1 )
	{
		myTagger app(argv[0]);

		std::wstring verb = argv[1], data, path, tmp;

		do {

			// The second param is a path where we start
			if( argc > 2 )
			{
				tmp = argv[2];

				if( tmp.substr(0, 6) == L"/path=" && tmp.length() > 8 )
				{
					path = argv[2] + 6;
					std::wcout << " ---> Current path is: '" << path << "'" << std::endl;

					if( app.checkFileSystem(path) )
					{
						// -------------------------------------------------------------------------------

						// Set tags for a single file or multiple files
						// "myTagger.exe" /Set /path="!\\" /files=!&
						if( verb == L"/Set" )
						{
							// get the list of files to process
							std::vector<std::wstring> vec;
							app.extractFiles(vec, argc, argv, path);

							if( vec.size() )
							{
								std::wcout << " ---> Input the tag(s) to set: ";
								std::getline(std::wcin, data);

								for(size_t i = 0; i < vec.size(); i++)
									app.Set(vec[i].c_str(), data);
							}
							else
							{
								std::wcout << " ---> No files selected." << std::endl;;
							}

							break;
						}

						// -------------------------------------------------------------------------------

						// Get tags for a single file
						// "myTagger.exe" /Get /path="!\!.!"
						if( verb == L"/Get" )
						{
							std::wcout << " ---> Tags found:" << std::endl;

							app.Get(path);
							break;
						}

						// -------------------------------------------------------------------------------

						// Copies tag data from a single file to clipboard
						// "myTagger.exe" /Copy /path="!\!.!"
						if( verb == L"/Copy" )
						{
							std::wcout << " ---> Copying tags:" << std::endl;

							app.Copy(path);
							break;
						}

						// -------------------------------------------------------------------------------

						// Find files containing tag(s)
						// "myTagger.exe" /Find /path="!\\"
						if( verb == L"/Find" )
						{
							std::wcout << " ---> Input your tag(s) to find: ";
							std::getline(std::wcin, data);

							app.Find(data, path);
							break;
						}

						// -------------------------------------------------------------------------------

						// Remove tags from a single file or multiple files
						// "myTagger.exe" /Rem /path="!\\" /files=!&
						if( verb == L"/Rem" )
						{
							// get the list of files to process
							std::vector<std::wstring> vec;
							app.extractFiles(vec, argc, argv, path);

							if( vec.size() )
							{
								std::wcout << " ---> Tags found:" << std::endl;

								if( app.Get(vec[0]) )
								{
									bool doProceed = true;

									std::wcout << "\n ---> Input tag name(s) to remove (OR input '*' to remove ALL tags): ";
									std::getline(std::wcin, data);

									if( data == L"*" )
									{
										doProceed = false;

										std::wcout << " ---> You are trying to remove ALL the tags from selected object(s)." << std::endl;
										std::wcout << " ---> If you want to proceed, say 'Y': ";
										std::wcin >> tmp;

										if( tmp[0] == L'y' || tmp[0] == L'Y' )
											doProceed = true;
									}

									if( doProceed )
									{
										for(size_t i = 0; i < vec.size(); i++)
											app.Rem(data, vec[i].c_str());
									}
									else
									{
										std::wcout << " ---> Cancelled." << std::endl;
									}
								}
							}
							else
							{
								std::wcout << " ---> No files selected." << std::endl;;
							}

							break;
						}

						// -------------------------------------------------------------------------------

						// Export all found tags into file(s)
						// "myTagger.exe" /Exp /path="!\\"
						if( verb == L"/Exp" )
						{
							app.Export(path);
							break;
						}

						// -------------------------------------------------------------------------------

						// Import all found tags into file(s)
						// "myTagger.exe" /Imp /path="!\\"
						if( verb == L"/Imp" )
						{
							app.Import(path);
							break;
						}

						// -------------------------------------------------------------------------------

						std::wcout << " ---> Missing the verb. Supported verbs are: [/Set, /Get, /Copy, /Find, /Rem, /Exp, /Imp]." << std::endl;
						std::wcout << " ---> Exiting... " << std::endl;
						res = 1;
					}
				}
				else
				{
					std::wcout << " ---> Missing the path parameter." << std::endl;
					std::wcout << " ---> Exiting... " << std::endl;
					res = 1;
				}

			}

		}
		while( false );

	}
	else
	{
		res = 1;
		std::wcout << " ---> Too few parameters." << std::endl;
		std::wcout << " ---> Exiting... " << std::endl;
	}

	if( !res )
	{
		std::wcout << " ---> Ok" << std::endl;
	}

	return res;
}
// =======================================================================================================================
