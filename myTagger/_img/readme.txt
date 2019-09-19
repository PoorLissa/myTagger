
	Supported Parameters:
	---------------------

	1. Set ["\myTagger.exe" /Set /path="!\\" /files=!&]
	Sets new tags for the selected files. If no files are selected, the file under cursor is affected.

	2. Get ["\myTagger.exe" /Get /path="!\!.!"]
	Retrieves tags for the file under cursor.

	3. Find ["\myTagger.exe" /Find /path="!\\"]
	Recursively finds files with the given set of tags, starting with the current path.
	* means "find all files with any tags"
	a b | c means "find files with tags a AND b OR c"

	4. Remove ["\myTagger.exe" /Rem /path="!\\" /files=!&]
	Removes tags from the selected files. If no files are selected, the file under cursor is affected.

	5. Import ["\myTagger.exe" /Imp /path="!\\"]
	Finds all the files with tags, starting with the current path.
	Exports the tags for the each file into the file file_name.mytag.stream.

	6. Export ["\myTagger.exe" /Exp /path="!\\"]
	Finds all the files with exported tags, starting with the current path.	
	Imports the tags for the corresponding files.


	ToDo:
	-----
	- when doing Import: if the *.mytag.stream file exists, but the original file does not, the original file is created -- we don't need that!


	Far Manager's F2 menu commands:
	-------------------------------
    T:   myTagger
    {
    s:   Set
         "\myTagger.exe" /Set /path="!\\" /files=!&
    g:   Get
         "\myTagger.exe" /Get /path="!\!.!"
    f:   Find
         "\myTagger.exe" /Find /path="!\\"
         tmp:"\myTagger.exe.temp"
         lua: Keys("AltF7 * . * TAB DEL ENTER")
    r:   Remove
         "\myTagger.exe" /Rem /path="!\\" /files=!&
    l:   Show Last Search
         tmp:"\myTagger.exe.temp"
    :    Copy
         "\myTagger.exe" /Copy /path="!\!.!"
    :    Import
         "\myTagger.exe" /Imp /path="!\\"
    :    Export
         "\myTagger.exe" /Exp /path="!\\"
    }
