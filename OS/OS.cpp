#include "stdafx.h"

#define handleFileError(handle1,handle2)  PrintError(); \
	CloseHandle(handle1); \
	CloseHandle(handle2); \
	return FALSE;

BOOL cpyFHTH(HANDLE, HANDLE);
#pragma region section 1

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

	BOOL r = SetEnvironmentVariable(_T("test"), _T("test"));
	DWORD result = GetEnvironmentVariable(_T("test"), Buffer, BufferSize);
	if (!result) {
		PrintError();
	} else if (result > BufferSize) {
		_tprintf(_T("Buffer is small\n"));
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
BOOL cpyFile(TCHAR* name1, TCHAR* name2) {

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
BOOL cpyFileReverse(TCHAR* name1, TCHAR* name2) {

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


enum COPY_ELEMENT_TYPE {
	ELEMENT_BYTE,
	ELEMENT_WORD,
	ELEMENT_LINE
};


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
BOOL cpyFileToStdout(TCHAR* name) {

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
	BOOL result = cpyFHTH(file, stdoutHandle);
	CloseHandle(file);
	return result;
}

//
////Problem 8
//BOOL cpyStdinToFile(TCHAR* name) {
//    DWORD cNumRead, fdwMode; 
//    INPUT_RECORD irInBuf[128];
//
//	HANDLE file = CreateFile(name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
//	if (file == INVALID_HANDLE_VALUE) {
//		PrintError();
//		return FALSE;
//	}
//
//	HANDLE stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
//	if (stdinHandle == INVALID_HANDLE_VALUE) {
//		CloseHandle(file);
//		PrintError();
//		return FALSE;
//	}
//
//	if (!ReadConsoleInput(stdinHandle, irInBuf, 128, &cNumRead))
//			PrintError();
//
//	BOOL result = cpyFHTH(stdinHandle, file);
//	CloseHandle(file);
//	return result;
//}


// Problem 10
// copy from handle1 to handle 2
BOOL cpyFHTH(HANDLE handle1, HANDLE handle2) {
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
		int converted = MultiByteToWideChar(CP_OEMCP, 0, Buffer, -1, unistring, BufferSize);

		if(!WriteFile(file2, unistring, BytesRead, &BytesWritten, NULL)
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


// Problem 12
// arguments without executable's name
BOOL printEnvironmentVariable(int argc, TCHAR* argv[], TCHAR **envp) {
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
		if(!WriteFile(file, *envp, len, &BytesWritten, NULL)
			|| BytesWritten != len) {
			CloseHandle(file);
			PrintError();
			return FALSE;
		}
		envp++;
	}
	return TRUE;
}
#pragma endregion

int _tmain(int argc, TCHAR* argv[], TCHAR **envp) {
	//Tests
	if (argc == 1) {
		_tprintf(_T("No arguments are given\nOS test [test_no [subtest_no | argv]]"));
		return 0;
	}
	if (_tcscmp(argv[1], _T("test")) == 0 && argc >= 3) {
		switch(_tstoi(argv[2])) {
		case 1:
			SetLastError(3);
			_tprintf(_T("Error code 3: "));
			PrintError();
			break;
		case 2:
			PrintInfo(envp);
			break;
		case 3:
			_tprintf(_T("copying file 1.txt to 2.txt\n"));
			cpyFile(_T("1.txt"), _T("2.txt"));
			break;
		case 4:
			_tprintf(_T("copying file 1.txt to 2.txt reversed\n"));
			cpyFileReverse(_T("1.txt"), _T("2.txt"));
			break;
		case 5: {
			DWORD subtest = (argc == 4) ? _tstoi(argv[3]) : 1;
			switch(subtest) {
			case 1:
				_tprintf(_T("copying 120 bytes from 1.txt to 2.txt\n"));
				copyElements(_T("1.txt"), _T("2.txt"), 120, ELEMENT_BYTE);
				break;
			case 2:
				_tprintf(_T("copying 10 words from 1.txt to 2.txt\n"));
				copyElements(_T("1.txt"), _T("2.txt"), 10, ELEMENT_WORD);
				break;
			case 3:
				_tprintf(_T("copying 3 lines from 1.txt to 2.txt\n"));
				copyElements(_T("1.txt"), _T("2.txt"), 3, ELEMENT_LINE);
				break;
			}
			break;
				}

		case 6:
			_tprintf(_T("Not implemented yet\n"));
			break;

		case 7:
			_tprintf(_T("copying file 3.txt to stdout\n"));
			cpyFileToStdout(_T("3.txt"));
			_tprintf(_T("\n"));
			break;

		case 8:
			_tprintf(_T("Not implemented yet\n"));
			//_tprintf(_T("copying stdin to 3.txt\n"));
			//cpyStdinToFile(_T("3.txt"));
			//_tprintf(_T("\n"));
			break;

		case 9:
			_tprintf(_T("Not implemented yet\n"));
			break;

		case 10:
			_tprintf(_T("For testing the 'Problem 10' implementation run the 'test 7'\n"));
			break;

		case 11:
			_tprintf(_T("copying ASCII.txt to Unicode.txt\n"));
			cpyAsciiToUnicode(_T("ASCII.txt"), _T("Unicode.txt"));
			_tprintf(_T("\n"));
			break;
		case 12:
			printEnvironmentVariable(argc - 3, argv + 3, envp);
			_tprintf(_T("\n"));
			break;

		default:
			_tprintf(_T("Unknown test\n"));
		}

	}
	return 0;
}

