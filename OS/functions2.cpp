#include "StdAfx.h"
#include "functions1.h"
#include "functions2.h"


// Problem 1
BOOL PrintCurrentDirectory() {
	WIN32_FIND_DATA fileData;
	HANDLE find = FindFirstFile(_T(".\\*"), &fileData);
	if (find == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}
	do {
		if(!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			SYSTEMTIME systemTime;
			if(!FileTimeToSystemTime(&fileData.ftCreationTime, &systemTime)) {
				PrintError();
				return FALSE;
			}
			const DWORD bufferSize = 255;
			TCHAR dateFormatted[bufferSize];
			TCHAR timeFormatted[bufferSize];

			LCID ENGLISH = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), 0);

			GetDateFormat(ENGLISH, DATE_SHORTDATE, &systemTime, NULL, dateFormatted, bufferSize);
			GetTimeFormat(ENGLISH,             0, &systemTime, NULL, timeFormatted, bufferSize);


			_tprintf(_T("%s\t\t%ll bytes\t created on %s %s\n"), fileData.cFileName,
				((DWORDLONG)fileData.nFileSizeHigh * ((DWORDLONG)MAXDWORD + 1)) +
				(DWORDLONG)fileData.nFileSizeLow,
				dateFormatted, timeFormatted);
		}
	} while(FindNextFile(find, &fileData) != NULL);

	if (GetLastError() != ERROR_NO_MORE_FILES) {
		PrintError();
		return FALSE;
	}
	FindCloseSafe(find);
	return TRUE;
}


// Problem 2
// give only filtered (without executable's name etc.) arguments
BOOL CopyOldToNew(int argc, TCHAR* argv[]) {
	if (argc != 1) {
		_tprintf(_T("Directory name not specified\n"));
		return FALSE;
	}
	if(!SetCurrentDirectory(argv[0])) {
		PrintError();
		return FALSE;
	}
	return copyFile(_T("old.txt"), _T("new.txt"));
}


// Problem 3
BOOL PrintDirectories() {
	bool HasParent;
	HANDLE find;
	WIN32_FIND_DATA fileData;
	do {
		HasParent = false;
		find = FindFirstFile(_T(".\\*"), &fileData);
		if (find == INVALID_HANDLE_VALUE) {
			PrintError();
			return FALSE;
		}
		do {
			if(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (_tcscmp(fileData.cFileName, _T("..")) == 0) {
					HasParent = true;
				} else if (_tcscmp(fileData.cFileName, _T(".")) != 0) {
					_tprintf(_T("%s\n"), fileData.cFileName);
				}
			}
		} while(FindNextFile(find, &fileData) != NULL);
		if(!SetCurrentDirectory(_T(".."))) {
			FindCloseSafe(find);
			PrintError();
			return FALSE;
		}
		_tprintf(_T("\n"));
		if (GetLastError() != ERROR_NO_MORE_FILES) {
			FindCloseSafe(find);
			PrintError();
			return FALSE;
		}
		FindCloseSafe(find);
	} while(HasParent);
	return TRUE;
}


BOOL hasSubCategory(bool * hasSubCategory) {
	WIN32_FIND_DATA fileData;
	HANDLE find = FindFirstFile(_T(".\\*"), &fileData);
	if (find == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}
	*hasSubCategory = false;
	do {
		if(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
			&& _tcscmp(fileData.cFileName, _T(".")) != 0 
			&& _tcscmp(fileData.cFileName, _T("..")) != 0) {
				*hasSubCategory = true;
				break;
		}
	} while(FindNextFile(find, &fileData) != NULL);
	FindCloseSafe(find);
	return TRUE;
}

// Problem 4
BOOL LeafSubDirectories() {
	WIN32_FIND_DATA fileData;
	HANDLE find = FindFirstFile(_T(".\\*"), &fileData);
	if (find == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}
	do {
		if(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
			&& _tcscmp(fileData.cFileName, _T(".")) != 0
			&& _tcscmp(fileData.cFileName, _T("..")) != 0) {
				if(!SetCurrentDirectory(fileData.cFileName)) {
					PrintError();
					return FALSE;
				}
				bool subCategory;
				if(!hasSubCategory(&subCategory)) {
					return FALSE;
				}
				if (!subCategory) {
					_tprintf(_T("%s\n"), fileData.cFileName);
				}
				if(!SetCurrentDirectory(_T(".."))) {
					PrintError();
					return FALSE;
				}
		}
	} while(FindNextFile(find, &fileData) != NULL);
	if(!SetCurrentDirectory(_T(".."))) {
		PrintError();
		return FALSE;
	}
	_tprintf(_T("\n"));
	if (GetLastError() != ERROR_NO_MORE_FILES) {
		PrintError();
		return FALSE;
	}
	if(!FindClose(find)) {
		PrintError();
		return FALSE;
	}
	return TRUE;
}


// Problem 6
// give only filtered (without executable's name etc.) arguments
BOOL UniteFiles(int argc, TCHAR* argv[]) {

	if(!CreateDirectory(_T("D:\\Dir"), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
		PrintError();
		return FALSE;
	}

	HANDLE file = CreateFile(_T("D:\\Dir\\Text.txt"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}

	HANDLE file2;
	for(int i = 0; i < argc; i++) {
		file2 = CreateFile(argv[i], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file2 == INVALID_HANDLE_VALUE) {
			PrintError();
			CloseHandleSafe(file);
			return FALSE;
		}
		if(!copyHTH(file2, file)) {
			CloseHandleSafe(file);
			CloseHandleSafe(file2);
			return FALSE;
		}
		CloseHandleSafe(file2);
	}
	CloseHandleSafe(file);
	return TRUE;
}


// Problem 7
// give only filtered (without executable's name etc.) arguments
BOOL LastDirectory(int argc, TCHAR* argv[]) {
	if (argc != 1) {
		_tprintf(_T("Directory name not specified\n"));
		return FALSE;
	}
	FILETIME fileTime;
	fileTime.dwHighDateTime = 0;
	fileTime.dwLowDateTime = 0;
	
	const DWORD BufferSize = 1000;
	TCHAR Buffer[BufferSize];

	WIN32_FIND_DATA fileData;
	_stprintf_s(Buffer, _T("%s\\*"), argv[0]);
	HANDLE find = FindFirstFile(Buffer, &fileData);
	if (find == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}
	_tcscpy_s(Buffer, _T(""));
	do {
		if(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
			&& _tcscmp(fileData.cFileName, _T(".")) != 0
			&& _tcscmp(fileData.cFileName, _T("..")) != 0) {
				fileData.ftCreationTime;
				if(fileData.ftCreationTime.dwHighDateTime > fileTime.dwHighDateTime
					|| (fileData.ftCreationTime.dwHighDateTime == fileTime.dwHighDateTime 
					&& fileData.ftCreationTime.dwLowDateTime > fileTime.dwLowDateTime)) {
						fileTime = fileData.ftCreationTime;
						_tcscpy_s(Buffer, fileData.cFileName);
				}
		}
	} while(FindNextFile(find, &fileData) != NULL);
	if (GetLastError() != ERROR_NO_MORE_FILES) {
		PrintError();
		return FALSE;
	}
	FindCloseSafe(find);
	if(_tcscmp(Buffer, _T("")) == 0) {
		_tprintf(_T("Can't find subdirectory"));
		return FALSE;
	}
	
	if(!SetCurrentDirectory(Buffer)) {
		PrintError();
		return FALSE;
	}

	find = FindFirstFile(_T(".\\*"), &fileData);
	if (find == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}
	do {
		if(!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			_tprintf(_T("%s\n"), fileData.cFileName);
		}
	} while(FindNextFile(find, &fileData) != NULL);

	if (GetLastError() != ERROR_NO_MORE_FILES) {
		PrintError();
		return FALSE;
	}
	if(!FindClose(find)) {
		PrintError();
		return FALSE;
	}

	return TRUE;
}


// Problem 8
// give only filtered (without executable's name etc.) arguments
BOOL LastNLines(int argc, TCHAR* argv[]) {
	if(argc == 0) {
		_tprintf(_T("Not enough arguments are given\n"));
		return FALSE;
	}
	int n = _tstoi(argv[0]);
	if(argc >= 2) {
		if(!SetCurrentDirectory(argv[1])) {
			PrintError();
			return FALSE;
		}
	}

	WIN32_FIND_DATA fileData;
	HANDLE find = FindFirstFile(_T(".\\*"), &fileData);
	if (find == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}
	do {
		if(!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			if(!printLastNLines(fileData.cFileName, n)) {
				return FALSE;
			}
			_tprintf(_T("\n"));
		}
	} while(FindNextFile(find, &fileData));
	FindCloseSafe(find);
	return TRUE;	
}

// Problem 9
// give only filtered (without executable's name etc.) arguments
BOOL UniteFilesInDirectory(int argc, TCHAR* argv[]) {
	if(argc == 0) {
		_tprintf(_T("Not enough arguments are given\n"));
		return FALSE;
	}
	if(argc >= 1) {
		if(!SetCurrentDirectory(argv[0])) {
			PrintError();
			return FALSE;
		}
	}

	HANDLE file = CreateFile(_T("Text.txt"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}
	
	WIN32_FIND_DATA fileData;
	HANDLE find = FindFirstFile(_T(".\\*.txt"), &fileData);
	if (find == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}
	
	HANDLE file2;
	do {
		if(!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
			&& _tcscmp(fileData.cFileName, _T("Text.txt")) != 0) { // not equal to the writing file
			file2 = CreateFile(fileData.cFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (file2 == INVALID_HANDLE_VALUE) {
				PrintError();
				CloseHandleSafe(file);
				return FALSE;
			}
			if(!copyHTH(file2, file)) {
				CloseHandleSafe(file);
				CloseHandleSafe(file2);
				return FALSE;
			}
			CloseHandleSafe(file2);
		}
	} while(FindNextFile(find, &fileData));
	CloseHandleSafe(file);
	FindCloseSafe(find);
	return TRUE;
}


int compareFileData(const void* a, const void* b) {
	WIN32_FIND_DATA x = *((WIN32_FIND_DATA*) a),  
		y = *((WIN32_FIND_DATA*) b);
	if(x.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { // don't sort directories
		return -1;
	}
	if(y.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		return 1;
	}
	if(x.nFileSizeHigh != y.nFileSizeHigh) {
		return x.nFileSizeHigh - y.nFileSizeHigh;
	}
	if(x.nFileSizeLow != y.nFileSizeLow) {
		return x.nFileSizeLow - y.nFileSizeLow;
	}
	return 0;
}
// Problem 10
BOOL FilesSorted() {
	WIN32_FIND_DATA files[100];
	int n = 0;
	
	WIN32_FIND_DATA fileData;
	HANDLE find = FindFirstFile(_T(".\\*"), &fileData);
	if (find == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}

	do {
		if(_tcscmp(fileData.cFileName, _T(".")) != 0
			&& _tcscmp(fileData.cFileName, _T("..")) != 0) {
				files[n++] = fileData;
		}
	} while(FindNextFile(find, &fileData));
	qsort(files, n, sizeof(WIN32_FIND_DATA), compareFileData);
	_tprintf(_T("(0 if directory)\n"));
	for(int i = 0; i < n; i++) {
		_tprintf(_T("%s\t\t\t%d\n"),files[i].cFileName, 
			((DWORDLONG)files[i].nFileSizeHigh * ((DWORDLONG)MAXDWORD + 1)) +
			(DWORDLONG)files[i].nFileSizeLow);
	}
	return TRUE;
}


int compareFileDataByNames(const void* a, const void* b) {
	return _tcscmp(((WIN32_FIND_DATA*)a)->cFileName, ((WIN32_FIND_DATA*)b)->cFileName);
}
// Problem 11
BOOL FilesSortedByNames() {
	WIN32_FIND_DATA files[100];
	int n = 0;
	
	WIN32_FIND_DATA fileData;
	HANDLE find = FindFirstFile(_T(".\\*"), &fileData);
	if (find == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}

	do {
		if(!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			&& _tcscmp(fileData.cFileName, _T(".")) != 0
			&& _tcscmp(fileData.cFileName, _T("..")) != 0) {
				files[n++] = fileData;
		}
	} while(FindNextFile(find, &fileData));
	qsort(files, n, sizeof(WIN32_FIND_DATA), compareFileDataByNames);

	for(int i = 0; i < n; i++) {
		_tprintf(_T("%s\t\t\t\n"),files[i].cFileName);
	}
	return TRUE;
}