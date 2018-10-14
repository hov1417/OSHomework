#include "StdAfx.h"
#include "functions1.h"

#define handleFileError(handle1,handle2)  PrintError(); \
	CloseHandle(handle1); \
	CloseHandle(handle2); \
	return FALSE;

//Problem 1
void PrintError() {
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError(); 

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, dw, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf, 0, NULL);

	_tprintf(_T("%s\n"), lpMsgBuf);

	LocalFree(lpMsgBuf);
	ExitProcess(dw); 
}

//Problem 2
void PrintInfo(TCHAR **envp) {
	const DWORD BufferSize = 2500;
	TCHAR Buffer[BufferSize];

	GetCurrentDirectory(BufferSize, Buffer);
	_tprintf(_T("Current Directory: %s\n"), Buffer);

	TCHAR** current = envp;
	_tprintf(_T("Envirement variables:\n"));
	while(*current) {
		_tprintf(_T("%s\n"), *current);
		current++;
	}

	if(!SetEnvironmentVariable(_T("test"), _T("test"))) {
		PrintError();
		return;
	}

	DWORD result = GetEnvironmentVariable(_T("test"), Buffer, BufferSize);
	if (!result) {
		PrintError();
		return;
	} else {
		_tprintf(_T("test: %s\n"), Buffer);
	}

	_tprintf(_T("\nOS info\n"));
	OSVERSIONINFOW versionInfo;
	versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);

	if(!GetVersionEx(&versionInfo)) {
		PrintError();
		return;
	}
	_tprintf(_T("BuildNumber: %u\n"), versionInfo.dwBuildNumber);
	_tprintf(_T("MajorVersion: %u\n"), versionInfo.dwMajorVersion);
	_tprintf(_T("MinorVersion: %u\n"), versionInfo.dwMinorVersion);
	_tprintf(_T("OSVersionInfoSize: %u\n"), versionInfo.dwOSVersionInfoSize);
	_tprintf(_T("PlatformId: %u\n"), versionInfo.dwPlatformId);
	_tprintf(_T("CSDVersion: %s\n"), versionInfo.szCSDVersion);
}

//Problem 3
BOOL copyFile(TCHAR* name1, TCHAR* name2) {

	const DWORD BufferSize = 10;
	TCHAR Buffer[BufferSize];

	DWORD BytesRead, BytesWritten;

	HANDLE file1 = CreateFile(name1, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file1 == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}

	HANDLE file2 = CreateFile(name2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file2 == INVALID_HANDLE_VALUE) {
		CloseHandle(file1);
		PrintError();
		return FALSE;
	}

	while(ReadFile(file1, Buffer, BufferSize, &BytesRead, NULL) && BytesRead > 0) {
		if(!WriteFile(file2, Buffer, BytesRead, &BytesWritten, NULL)
			|| BytesWritten != BytesRead) {
				handleFileError(file1,file2);
		}
	}
	if (BytesRead > 0) {
		handleFileError(file1,file2);
	}
	CloseHandle(file1);
	CloseHandle(file2);
	return TRUE;
}

//Problem 4
BOOL copyFileReverse(TCHAR* name1, TCHAR* name2) {

	const DWORD BufferSize = 2000;
	TCHAR Buffer[BufferSize];

	DWORD BytesRead, BytesWritten;

	HANDLE file1 = CreateFile(name1, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file1 == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}

	HANDLE file2 = CreateFile(name2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file2 == INVALID_HANDLE_VALUE) {
		CloseHandle(file1);
		PrintError();
		return FALSE;
	}

	SetFilePointer(file1, 0, NULL, FILE_END);
	DWORD currentPointer = SetFilePointer(file1, 0, NULL, FILE_CURRENT);
	DWORD BytesToRead = BufferSize;
	while(currentPointer != 0) {
		if(SetFilePointer(file1, -BufferSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER) {
			if(GetLastError() == ERROR_NEGATIVE_SEEK) { //seting pointer before begining
				BytesToRead = currentPointer;
				SetFilePointer(file1, 0, NULL, FILE_BEGIN);
			} else {
				handleFileError(file1,file2);
			}
		}
		if(!ReadFile(file1, Buffer, BytesToRead, &BytesRead, NULL)) {
			handleFileError(file1,file2);
		}
		//returning before readed
		if(SetFilePointer(file1, -BytesRead, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER) {
			handleFileError(file1,file2);
		}
		//reversing
		int count = BytesRead/sizeof(TCHAR);
		for (int i = 0; i < count/2; i++) {
			TCHAR tmp = Buffer[i];
			Buffer[i] = Buffer[count - i - 1];
			Buffer[count - i - 1] = tmp;
		}

		currentPointer = SetFilePointer(file1, 0, NULL, FILE_CURRENT);
		if(!WriteFile(file2, Buffer, BytesRead, &BytesWritten, NULL)
			|| BytesRead < 0 || BytesRead != BytesWritten) {
				handleFileError(file1,file2);
		}
	}
	CloseHandle(file1);
	CloseHandle(file2);
	return TRUE;
}

//Problem 5
BOOL copyElements(TCHAR* name1, TCHAR* name2, int elemCount, COPY_ELEMENT_TYPE elemType) {
	const DWORD BufferSize = 10;
	TCHAR Buffer[BufferSize];

	HANDLE file1 = CreateFile(name1, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file1 == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}

	HANDLE file2 = CreateFile(name2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file2 == INVALID_HANDLE_VALUE) {
		CloseHandle(file1);
		PrintError();
		return FALSE;
	}

	BOOL ReadResponse;
	DWORD OverallWritten = 0, BytesRead, BytesWritten;
	if (elemType == ELEMENT_BYTE) {
		while((ReadResponse = ReadFile(file1, Buffer, BufferSize, &BytesRead, NULL)) && BytesRead > 0) {
			if (OverallWritten + BytesRead > elemCount) {
				if (!WriteFile(file2, Buffer, elemCount - OverallWritten, &BytesWritten, NULL) 
					|| BytesWritten != elemCount - OverallWritten) {
						handleFileError(file1,file2);
				}
				break;
			}
			if (!WriteFile(file2, Buffer, BytesRead, &BytesWritten, NULL) 
				|| BytesWritten != BytesRead) {
					handleFileError(file1,file2);
			}
			OverallWritten += BytesWritten;
		}
	} else if (elemType == ELEMENT_WORD || elemType == ELEMENT_LINE) {
		while((ReadResponse = ReadFile(file1, Buffer, BufferSize, &BytesRead, NULL)) && BytesRead > 0) {
			unsigned int i = 0;
			for(; i < BytesRead / sizeof(TCHAR) && OverallWritten < elemCount; i++) {
				if (Buffer[i] == '\n' || 
					(elemType == ELEMENT_WORD && Buffer[i] == ' ')) {
						OverallWritten++;
				}
			}
			if (!WriteFile(file2, Buffer, i * sizeof(TCHAR), &BytesWritten, NULL) 
				|| BytesWritten != i * sizeof(TCHAR)) {
					handleFileError(file1,file2);
			}
			if (OverallWritten == elemCount) {
				break;
			}
		}
	} else {
		return FALSE;
	}

	if (!ReadResponse) {
		handleFileError(file1,file2);
	}

	CloseHandle(file1);
	CloseHandle(file2);
	return TRUE;
}

//Problem 7
BOOL copyFileToStdout(TCHAR* name) {

	HANDLE file = CreateFile(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}

	HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdoutHandle == INVALID_HANDLE_VALUE) {
		CloseHandle(file);
		PrintError();
		return FALSE;
	}

	BOOL result = copyFHTH(file, stdoutHandle);
	CloseHandle(file);
	return result;
}

//Problem 8 not fully implemented
BOOL copyStdinToFile(TCHAR* name) {
	//DWORD cNumRead, fdwMode; 
	//INPUT_RECORD irInBuf[128];

	//HANDLE file = CreateFile(name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//if (file == INVALID_HANDLE_VALUE) {
	//	PrintError();
	//	return FALSE;
	//}

	//HANDLE stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
	//if (stdinHandle == INVALID_HANDLE_VALUE) {
	//	CloseHandle(file);
	//	PrintError();
	//	return FALSE;
	//}

	//if (!ReadConsoleInput(stdinHandle, irInBuf, 128, &cNumRead))
	//	PrintError();

	//BOOL result = copyFHTH(stdinHandle, file);
	//CloseHandle(file);
	//return result;
	return TRUE;
}

// Problem 10
// copy from handle1 to handle 2
BOOL copyFHTH(HANDLE handle1, HANDLE handle2) {
	const DWORD BufferSize = 10;
	TCHAR Buffer[BufferSize];
	DWORD BytesRead, BytesWritten;

	while(ReadFile(handle1, Buffer, BufferSize, &BytesRead, NULL) && BytesRead > 0) {
		if(!WriteFile(handle2, Buffer, BytesRead, &BytesWritten, NULL)
			|| BytesWritten != BytesRead) {
				PrintError();
				return FALSE;
		}
	}
	if (BytesRead > 0) {
		PrintError();
		return FALSE;
	}
	return TRUE;
}

// Problem 11
BOOL cpyAsciiToUnicode(TCHAR* name1, TCHAR* name2) {
	const DWORD BufferSize = 100;
	char Buffer[BufferSize];

	DWORD BytesRead, BytesWritten;

	HANDLE file1 = CreateFile(name1, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file1 == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}

	HANDLE file2 = CreateFile(name2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file2 == INVALID_HANDLE_VALUE) {
		CloseHandle(file1);
		PrintError();
		return FALSE;
	}

	while(ReadFile(file1, Buffer, BufferSize, &BytesRead, NULL) && BytesRead > 0) {
		TCHAR unistring[BufferSize];
		for(int i = 0;i < BytesRead; i++) {
			unistring[i] = Buffer[i];
		}
		char* a = (char*) unistring;

		if(!WriteFile(file2, unistring, sizeof(TCHAR) * BytesRead, &BytesWritten, NULL)
			|| BytesWritten != sizeof(TCHAR) * BytesRead) {
				handleFileError(file1,file2);
		}
	}
	if (BytesRead > 0) {
		handleFileError(file1,file2);
	}
	CloseHandle(file1);
	CloseHandle(file2);
	return TRUE;
}

// Problem 12
// arguments without executable's name
BOOL printEnvironmentVariable(int argc, TCHAR* argv[], TCHAR *envp[]) {
	if (argc == 0) {
		while(*envp) {
			_tprintf(_T("%s\n"), *envp);
			envp++;
		}
		return TRUE;
	}

	HANDLE file = CreateFile(argv[0], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}
	DWORD BytesWritten;
	while(*envp) {
		DWORD len = _tcslen(*envp);
		len+=2;
		TCHAR* buffer = new TCHAR[len + 2];
		_tcscpy_s(buffer, len + 2, *envp);
		_tcscat_s(buffer, len + 2, _T("\r\n"));
		if(!WriteFile(file, buffer, len * sizeof(TCHAR), &BytesWritten, NULL)
			|| BytesWritten != len* sizeof(TCHAR)) {
				CloseHandle(file);
				PrintError();
				delete [] buffer;
				return FALSE;
		}
		delete [] buffer;
		envp++;
	}
	return TRUE;
}

//Problem 14
BOOL changeTime(TCHAR* fileName) {
	HANDLE handle = CreateFile(fileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
	if (handle == INVALID_HANDLE_VALUE){
		PrintError();
		return FALSE;
	}
	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		CloseHandle(handle);
		return TRUE;
	}
	SYSTEMTIME now;
	GetSystemTime(&now);
	FILETIME nowFile;
	if (!SystemTimeToFileTime(&now, &nowFile)){
		CloseHandle(handle);
		PrintError();
		return FALSE;
	}
	if(!SetFileTime(handle, &nowFile, NULL, NULL)){
		CloseHandle(handle);
		PrintError();
		return FALSE;
	}
	CloseHandle(handle);
	return TRUE;
}

//Problem 15
// arguments without executable's name
BOOL addToPATH(int argc, TCHAR* argv[]) {
	DWORD BufferSize = 2500;
	TCHAR* Buffer = new TCHAR[BufferSize];
	if (argc == 0) {
		_tprintf(_T("Directory name not specified\n"));
		return FALSE;
	}
	DWORD length = GetEnvironmentVariable(_T("PATH"), Buffer, BufferSize);
	if (!length) {
		PrintError();
		delete[] Buffer;
		return FALSE;
	}
	if (length > BufferSize) {
		delete[] Buffer;
		Buffer = new TCHAR[length + 1];
		BufferSize = length + 1;
		length = GetEnvironmentVariable(_T("PATH"), Buffer, BufferSize);
		if (!length) {
			PrintError();
			delete[] Buffer;
			return FALSE;
		}
	}
	DWORD DirectoryLength = _tcslen(argv[0]);
	DWORD newLen = length + DirectoryLength + 10;
	TCHAR* newBuffer = new TCHAR[newLen];
	_tcscpy_s(newBuffer, newLen, Buffer);
	_tcscat_s(newBuffer, newLen, _T(";"));
	_tcscat_s(newBuffer, newLen, argv[0]);
	delete[] Buffer;

	if(!SetEnvironmentVariable(_T("PATH"), newBuffer)) {
		PrintError();
		delete[] newBuffer;
		return FALSE;
	}

	delete[] newBuffer;
	return TRUE;
}


//Problem 16
// arguments without executable's name
BOOL calculateWords(int argc, TCHAR* argv[]) {
	const DWORD BufferSize = 1000;
	TCHAR Buffer[BufferSize];
	if (argc == 0) {
		_tprintf(_T("File name not specified\n"));
		return FALSE;
	}
	HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdoutHandle == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}
	DWORD BytesRead, BytesWritten;
	for(int i = 0; i < argc; i++) {
		HANDLE file = CreateFile(argv[i], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file == INVALID_HANDLE_VALUE) {
			PrintError();
			return FALSE;
		}
		int wordsNumber = 1;
		while(ReadFile(file, Buffer, BufferSize, &BytesRead, NULL) && BytesRead > 0) {
			for(int j = 0; j < BytesRead / sizeof(TCHAR); j++) {
				if (Buffer[j] == _T(' ') || Buffer[j] == _T('\n')) {
					wordsNumber++;
				}
			}
			DWORD len = _stprintf_s(Buffer, BufferSize, _T("%s %d\n\n"), argv[i], wordsNumber) - 1; //idk why, but 1 endline doesn't work
			if (!WriteFile(stdoutHandle, Buffer, len * sizeof(TCHAR), &BytesWritten, NULL) 
				|| BytesWritten != len * sizeof(TCHAR)) {
					CloseHandle(file);
					return FALSE;
			}
		}
		if (BytesRead > 0) {
			PrintError();
			CloseHandle(file);
			return FALSE;
		}
		CloseHandle(file);
	}
	return TRUE;
}


//Problem 17
// arguments without executable's name
BOOL printFile(int argc, TCHAR* argv[]) {
	const DWORD BufferSize = 1000;
	TCHAR Buffer[BufferSize];
	if (argc == 0) {
		_tprintf(_T("File name not specified\n"));
		return FALSE;
	}
	
	HANDLE file = CreateFile(argv[0], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}
	DWORD BytesRead;
	while(ReadFile(file, Buffer, BufferSize, &BytesRead, NULL) && BytesRead > 0) {
		_tprintf(Buffer);
	}
	if (BytesRead > 0) {
		PrintError();
		CloseHandle(file);
		return FALSE;
	}
	CloseHandle(file);
	return TRUE;
}