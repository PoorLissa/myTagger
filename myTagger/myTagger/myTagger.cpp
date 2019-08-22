// myTagger.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "myApp.h"

const wchar_t *exeName = nullptr;

// =======================================================================================================================

int _tmain(int argc, _TCHAR* argv[])
{
	int res = 0;
	exeName = argv[0];

	if( argc > 1 )
	{
		myTagger app;

		std::wstring arg1 = argv[1], data, path;

		do {

			// The second param is a path where we start
			if( argc > 2 )
			{
				path = argv[2] + 6;

				std::wcout << " ---> Current path is: '";
				app.doPrint(path);
				std::wcout << "'" << std::endl;

				// -------------------------------------------------------------------------------

				if( arg1 == L"/Set" )
				{
					std::wcout << " ---> Input your tag(s) to set: ";
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

				std::wcout << " ---> Missing the first verb. Supported verbs are: [/Set, /Get, /Find, /Rem]." << std::endl;
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
