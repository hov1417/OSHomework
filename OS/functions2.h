#pragma once


#define FindCloseSafe(find) if(!FindClose(find)) { \
	PrintError();\
	return FALSE;\
	}


BOOL PrintCurrentDirectory();
BOOL CopyOldToNew(int argc, TCHAR* argv[]);
BOOL PrintDirectories();
BOOL LeafSubDirectories();

BOOL UniteFiles(int argc, TCHAR* argv[]);
BOOL LastDirectory(int argc, TCHAR* argv[]);
BOOL LastNLines(int argc, TCHAR* argv[]);
BOOL UniteFilesInDirectory(int argc, TCHAR* argv[]);
BOOL FilesSorted();
BOOL FilesSortedByNames();