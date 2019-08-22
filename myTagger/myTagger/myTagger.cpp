// myTagger.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include "myApp.h"
#include "myAppOld.h"

const wchar_t *exeName = nullptr;

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

			if( arg1 == L"/Set" && argc > 2 )
			{
				myTagger app;

				std::wstring fileName(argv[2] + 6);

				std::wcout << " ---> Selected target is: '";
				doPrint(fileName);
				std::wcout << "'" << std::endl;
				std::wcout << " ---> Input your tag(s) to set: ";
				std::getline(std::wcin, data);

				app.Set(fileName, data);
				break;
			}

			if( arg1 == L"/Get" && argc > 2 )
			{
				myTagger app;

				std::wstring dirName(argv[2] + 6);

				std::wcout << " ---> Current path is: '";
				doPrint(dirName);
				std::wcout << "'" << std::endl;
				std::wcout << " ---> Tags found:" << std::endl;;

				app.Get(dirName);
				break;
			}

			if( arg1 == L"/Find" && argc > 2 )
			{
				myTagger app;

				std::wstring dirName(argv[2] + 6);

				std::wcout << " ---> Current path is: '";
				doPrint(dirName);
				std::wcout << "'" << std::endl;
				std::wcout << " ---> Input your tag(s) to find: ";
				std::getline(std::wcin, data);

				app.Find(data, dirName);
				break;
			}

			if( arg1 == L"/Rem" && argc > 2 )
			{
				myTagger app;

				std::wstring dirName(argv[2] + 6);

				std::wcout << " ---> Current path is: '";
				doPrint(dirName);
				std::wcout << "'" << std::endl;
				std::wcout << " ---> Tags found:" << std::endl;

				if( app.Get(dirName) )
				{
					std::wcout << std::endl;

					std::wcout << " ---> Input your tag(s) to remove (OR input '*' to remove ALL tags): ";
					std::getline(std::wcin, data);

					app.Rem(data, dirName);
				}
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

