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
	// https://ru.stackoverflow.com/questions/459154/�������-����-�-�������
	// http://it-negr.blogspot.com/2012/03/c.html
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno( stdin), _O_U16TEXT);
	_setmode(_fileno(stderr), _O_U16TEXT);

	if( argc > 1 )
	{
		myTagger app(argv[0]);

		std::wstring arg1 = argv[1], data, path;

		do {

			// The second param is a path where we start
			if( argc > 2 )
			{
				path = argv[2] + 6;

				std::wcout << " ---> Current path is: '" << path << "'" << std::endl;

				// -------------------------------------------------------------------------------

				if( arg1 == L"/Set" )
				{
					std::wcout << " ---> Input the tag(s) to set: ";
					std::getline(std::wcin, data);

					app.Set(path, data);
					break;
				}

				// -------------------------------------------------------------------------------

				if( arg1 == L"/Get" )
				{
					std::wcout << " ---> Tags found:" << std::endl;

					app.Get(path);
					break;
				}

				// -------------------------------------------------------------------------------

				if( arg1 == L"/Find" )
				{
					std::wcout << " ---> Input your tag(s) to find: ";
					std::getline(std::wcin, data);

					app.Find(data, path);
					break;
				}

				// -------------------------------------------------------------------------------

				if( arg1 == L"/Rem" )
				{
					std::wcout << " ---> Tags found:" << std::endl;

					if( app.Get(path) )
					{
						std::wcout << std::endl;

						std::wcout << " ---> Input tag name(s) to remove (OR input '*' to remove ALL tags): ";
						std::getline(std::wcin, data);

						app.Rem(data, path);
					}
					break;
				}

				// -------------------------------------------------------------------------------

				std::wcout << " ---> Missing the verb. Supported verbs are: [/Set, /Get, /Find, /Rem]." << std::endl;
				std::wcout << " ---> Exiting... " << std::endl;
				res = 1;
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
