#include "StdAfx.h"
#include "functions1.h"

// Problem 1
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

// Problem 2
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

	if (!GetEnvironmentVariable(_T("test"), Buffer, BufferSize)) {
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

// Problem 3
//file with name2 must be UTF-16 without BOM
BOOL copyFile(TCHAR* name1, TCHAR* name2) {
	HANDLE file1 = CreateFile(name1, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file1 == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}

	HANDLE file2 = CreateFile(name2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file2 == INVALID_HANDLE_VALUE) {
		CloseHandleSafe(file1);
		PrintError();
		return FALSE;
	}

	BOOL result = copyHTH(file1, file2);
	CloseHandleSafe(file1);
	CloseHandleSafe(file2);
	return result;
}

// Problem 4
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
		CloseHandleSafe(file1);
		PrintError();
		return FALSE;
	}

	if(SetFilePointer(file1, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER) {
		handleFileError(file1,file2);
	}
	DWORD currentPointer = SetFilePointer(file1, 0, NULL, FILE_CURRENT);
	if(currentPointer == INVALID_SET_FILE_POINTER) {
		handleFileError(file1,file2);
	}
	DWORD BytesToRead = BufferSize;
	while(currentPointer != 0) {
		if(SetFilePointer(file1, -(LONG)BufferSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER) {
			if(GetLastError() == ERROR_NEGATIVE_SEEK) { //seting pointer before begining
				BytesToRead = currentPointer;
				if(SetFilePointer(file1, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
					handleFileError(file1,file2);
				}
			} else {
				handleFileError(file1,file2);
			}
		}
		if(!ReadFile(file1, Buffer, BytesToRead, &BytesRead, NULL)) {
			handleFileError(file1,file2);
		}
		//returning before read
		if(SetFilePointer(file1, -(LONG)BytesRead, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER) {
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
		if(currentPointer == INVALID_SET_FILE_POINTER) {
			handleFileError(file1,file2);
		}
		if(!WriteFile(file2, Buffer, BytesRead, &BytesWritten, NULL)
			|| BytesRead < 0 || BytesRead != BytesWritten) {
				handleFileError(file1,file2);
		}
	}
	CloseHandleSafe(file1);
	CloseHandleSafe(file2);
	return TRUE;
}

// Problem 5
BOOL firstNElements(TCHAR* name1, TCHAR* name2, UINT n, COPY_ELEMENT_TYPE elemType) {
	const DWORD BufferSize = 100;
	TCHAR Buffer[BufferSize];

	HANDLE file1 = CreateFile(name1, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file1 == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}

	HANDLE file2 = CreateFile(name2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file2 == INVALID_HANDLE_VALUE) {
		CloseHandleSafe(file1);
		PrintError();
		return FALSE;
	}

	BOOL ReadResponse;
	DWORD OverallWritten = 0, BytesRead, BytesWritten;
	if (elemType == ELEMENT_BYTE) {
		while((ReadResponse = ReadFile(file1, Buffer, BufferSize, &BytesRead, NULL)) && BytesRead > 0) {
			if (OverallWritten + BytesRead > n) {
				if (!WriteFile(file2, Buffer, n - OverallWritten, &BytesWritten, NULL) 
					|| BytesWritten != n - OverallWritten) {
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
			UINT i = 0;
			for(; i < BytesRead / sizeof(TCHAR) && OverallWritten < n; i++) {
				if (Buffer[i] == '\n' || 
					(elemType == ELEMENT_WORD && Buffer[i] == ' ')) {
						OverallWritten++;
				}
			}
			if (!WriteFile(file2, Buffer, i * sizeof(TCHAR), &BytesWritten, NULL) 
				|| BytesWritten != i * sizeof(TCHAR)) {
					handleFileError(file1,file2);
			}
			if (OverallWritten == n) {
				break;
			}
		}
	} else {
		return FALSE;
	}

	if (!ReadResponse) {
		handleFileError(file1,file2);
	}

	CloseHandleSafe(file1);
	CloseHandleSafe(file2);
	return TRUE;
}

BOOL lastNElements(HANDLE handle1, HANDLE handle2, UINT n, COPY_ELEMENT_TYPE elemType, bool isHandle2File) {
	const DWORD BufferSize = 100;
	TCHAR Buffer[BufferSize];

	DWORD OverallWritten = 0, BytesRead = -1;
	DWORD BytesToRead = BufferSize;
	if (elemType == ELEMENT_BYTE) {
		if(SetFilePointer(handle1, -(LONG)n, NULL, FILE_END) == INVALID_SET_FILE_POINTER){
			if(GetLastError() == ERROR_NEGATIVE_SEEK) { //seting pointer before begining
				if(SetFilePointer(handle1, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
					return FALSE;
				}
			} else {
				return FALSE;
			}
		}
	} else if (elemType == ELEMENT_WORD || elemType == ELEMENT_LINE) {
		if(SetFilePointer(handle1, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER) {
			return FALSE;
		}
		UINT elemCount = 0;
		while(BytesRead != 0) {
			if(SetFilePointer(handle1, -(LONG)BufferSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER) {
				if(GetLastError() == ERROR_NEGATIVE_SEEK) { //seting pointer before begining
					BytesToRead = SetFilePointer(handle1, 0, NULL, FILE_CURRENT);
					if(SetFilePointer(handle1, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
						return FALSE;
					}
				} else {
					return FALSE;
				}
			}
			if(!ReadFile(handle1, Buffer, BytesToRead, &BytesRead, NULL)) {
				return FALSE;
			}
			//returning before read
			if(SetFilePointer(handle1, -(LONG)BytesRead, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER) {
				return FALSE;
			}
			//counting
			UINT countTCHAR = BytesRead/sizeof(TCHAR);
			int i = countTCHAR - 1;
			for(; i >= 0 && elemCount != n; i--) {
				if(Buffer[i] == '\n' || 
					(elemType == ELEMENT_WORD && Buffer[i] == ' ')) {
					elemCount++;
				}
			}
			if (elemCount == n) {
				if(SetFilePointer(handle1, (LONG)((i + 1) * sizeof(TCHAR)), NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER) {
					return FALSE;
				}
				break;
			}
		}
	} else {
		return FALSE;
	}

	return copyHTH(handle1, handle2, isHandle2File);
}

// Problem 6
BOOL lastNElements(TCHAR* name1, TCHAR* name2, UINT n, COPY_ELEMENT_TYPE elemType) {
	HANDLE file1 = CreateFile(name1, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file1 == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}

	HANDLE file2 = CreateFile(name2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file2 == INVALID_HANDLE_VALUE) {
		CloseHandleSafe(file1);
		PrintError();
		return FALSE;
	}


	BOOL result = lastNElements(file1, file2, n, elemType, true);
	if(!result) {
		PrintError();
	}
	CloseHandleSafe(file1);
	CloseHandleSafe(file2);
	return result;
}

// Problem 7
BOOL copyFileToStdout(TCHAR* name) {
	HANDLE file = CreateFile(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}

	HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdoutHandle == INVALID_HANDLE_VALUE) {
		CloseHandleSafe(file);
		PrintError();
		return FALSE;
	}

	BOOL result = copyHTH(file, stdoutHandle, false);
	CloseHandleSafe(file);
	return result;
}

// Problem 8
BOOL copyStdinToFile(TCHAR* name) {
	HANDLE file = CreateFile(name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}

	HANDLE stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
	if (stdinHandle == INVALID_HANDLE_VALUE) {
		CloseHandleSafe(file);
		PrintError();
		return FALSE;
	}
	DWORD BytesWritten, EventNumber;
	INPUT_RECORD Buffer[100];
	while(true) {
		if (!ReadConsoleInput(stdinHandle, Buffer, 128, &EventNumber)) { //for breaking on "new line" event
			PrintError();
			CloseHandleSafe(file);
			return FALSE;
		}
		bool breakThis = false;
		for (UINT i = 0; i < EventNumber; i++) {
            if(Buffer[i].EventType == KEY_EVENT && Buffer[i].Event.KeyEvent.bKeyDown) {
				KEY_EVENT_RECORD keyEvent = Buffer[i].Event.KeyEvent;
				if(keyEvent.wVirtualKeyCode == 13) {
					breakThis = true;
					break;
				}
				printf(&keyEvent.uChar.AsciiChar);
				if(!WriteFile(file, &keyEvent.uChar.AsciiChar, 1, &BytesWritten, NULL) || BytesWritten != 1) {
					PrintError();
					CloseHandleSafe(file);
					return FALSE;
				}
            }
        }
		if (breakThis) {
			break;
		}
	}

	CloseHandleSafe(file);
	return TRUE;
}

// Problem 9
BOOL copyStdinToStdout() {
	HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdoutHandle == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}
	HANDLE stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
	if (stdinHandle == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}

	DWORD BytesWritten, EventNumber;
	INPUT_RECORD Buffer[128];
	const int inputBufferSize = 10000;
	char inputBuffer[inputBufferSize];
	int index = 0;
	while(true) {
		if (!ReadConsoleInput(stdinHandle, Buffer, 128, &EventNumber)) { //for breaking on "new line" event
			PrintError();
			return FALSE;
		}
		for (UINT i = 0; i < EventNumber; i++) {
            if(Buffer[i].EventType == KEY_EVENT && Buffer[i].Event.KeyEvent.bKeyDown) {
				KEY_EVENT_RECORD keyEvent = Buffer[i].Event.KeyEvent;
				if(keyEvent.wVirtualKeyCode == 13) {
					if(!WriteFile(stdoutHandle, inputBuffer, index, &BytesWritten, NULL) 
						|| BytesWritten !=  index) {
						PrintError();
						return FALSE;
					}
					return TRUE;
				}
				if(index == inputBufferSize) {
					_tprintf(_T("buffer is small"));
					return FALSE;
				}
				inputBuffer[index++] = keyEvent.uChar.AsciiChar;
            }
        }
	}

	return TRUE;
}

// Problem 10
// copy from handle1 to handle 2
BOOL copyHTH(HANDLE handle1, HANDLE handle2, bool isFile) {
	const DWORD BufferSize = 100;
	TCHAR Buffer[BufferSize];
	DWORD Read, Written;

	while(ReadFile(handle1, Buffer, BufferSize, &Read, NULL) && Read > 0) {
		DWORD readedElements = Read / (isFile ? 1 : sizeof(TCHAR));
		if(!GeneralWrite(isFile, handle2, Buffer, readedElements, &Written)
			|| Written != readedElements) {
				PrintError();
				return FALSE;
		}
	}
	if (Read > 0) {
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
		CloseHandleSafe(file1);
		PrintError();
		return FALSE;
	}

	while(ReadFile(file1, Buffer, BufferSize, &BytesRead, NULL) && BytesRead > 0) {
		TCHAR unistring[BufferSize];
		for(UINT i = 0;i < BytesRead; i++) {
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
	CloseHandleSafe(file1);
	CloseHandleSafe(file2);
	return TRUE;
}

// Problem 12
// give only filtered (without executable's name etc.) arguments
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
				CloseHandleSafe(file);
				PrintError();
				delete [] buffer;
				return FALSE;
		}
		delete [] buffer;
		envp++;
	}
	CloseHandleSafe(file);
	return TRUE;
}


BOOL printLastNLines(TCHAR* name, UINT n) {
	const DWORD BufferSize = 2000;
	TCHAR Buffer[BufferSize];

	DWORD BytesRead = -1;

	HANDLE file = CreateFile(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		PrintError();
		return FALSE;
	}
	if(SetFilePointer(file, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER) {
		CloseHandleSafe(file);
		return FALSE;
	}
	DWORD BytesToRead = BufferSize;
	UINT newlineCount = 0;
	while(BytesRead != 0) {
		if(SetFilePointer(file, -(LONG)BufferSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER) {
			if(GetLastError() == ERROR_NEGATIVE_SEEK) { //seting pointer before begining
				BytesToRead = SetFilePointer(file, 0, NULL, FILE_CURRENT);
				if(SetFilePointer(file, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
					CloseHandleSafe(file);
					return FALSE;
				}
			} else {
				CloseHandleSafe(file);
				return FALSE;
			}
		}
		if(!ReadFile(file, Buffer, BytesToRead, &BytesRead, NULL)) {
			CloseHandleSafe(file);
			return FALSE;
		}
		//returning before read
		if(SetFilePointer(file, -(LONG)BytesRead, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER) {
			CloseHandleSafe(file);
			return FALSE;
		}
		//counting
		UINT countTCHAR = BytesRead/sizeof(TCHAR);
		int i = countTCHAR - 1;
		for(; i >= 0 && newlineCount != n; i--) {
			if(Buffer[i] == '\n') {
				newlineCount++;
			}
		}
		if (newlineCount == n) {
			if(SetFilePointer(file, (LONG)((i + 1) * sizeof(TCHAR)), NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER) {
				CloseHandleSafe(file);
				return FALSE;
			}
			break;
		}
	}
	HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdoutHandle == INVALID_HANDLE_VALUE) {
		CloseHandleSafe(file);
		PrintError();
		return FALSE;
	}
	BOOL result = copyHTH(file, stdoutHandle, false);

	CloseHandleSafe(file);
	return result;
}

// Problem 13
// give only filtered (without executable's name etc.) arguments
BOOL printLastNLines(UINT argc, TCHAR* argv[]) {
	if (argc < 2) {
		_tprintf(_T("Not enough arguments are given\n"));
		return FALSE;
	}
	UINT n = _tstoi(argv[0]);
	for(UINT i = 1; i < argc; i++) {
		if(!printLastNLines(argv[i], n)) {
			return FALSE;
		}
		_tprintf(_T("\n"));
	}
	return TRUE;
}


// Problem 14
// give only filtered (without executable's name etc.) arguments
BOOL changeTime(int argc, TCHAR* argv[]) {
	if (argc == 0) {
		_tprintf(_T("Not enough arguments are given\n"));
		return FALSE;
	}
	TCHAR* fileName = argv[0];
	HANDLE handle = CreateFile(fileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
	if (handle == INVALID_HANDLE_VALUE){
		PrintError();
		return FALSE;
	}
	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		CloseHandleSafe(handle);
		return TRUE;
	}
	SYSTEMTIME now;
	GetSystemTime(&now);
	FILETIME nowFile;
	if (!SystemTimeToFileTime(&now, &nowFile)){
		CloseHandleSafe(handle);
		PrintError();
		return FALSE;
	}
	if(!SetFileTime(handle, &nowFile, NULL, NULL)){
		CloseHandleSafe(handle);
		PrintError();
		return FALSE;
	}
	CloseHandleSafe(handle);
	return TRUE;
}

// Problem 15
// give only filtered (without executable's name etc.) arguments
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


// Problem 16
// give only filtered (without executable's name etc.) arguments
// (also Section 2 Problem 5)
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
	
	if(!GetCurrentDirectory(BufferSize, Buffer)) {
		PrintError();
		return FALSE;
	}
	_tprintf(_T("Directory: %s\n"), Buffer);


	for(int i = 0; i < argc; i++) {
		HANDLE file = CreateFile(argv[i], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file == INVALID_HANDLE_VALUE) {
			PrintError();
			return FALSE;
		}
		int wordsNumber = 1;
		while(ReadFile(file, Buffer, BufferSize, &BytesRead, NULL) && BytesRead > 0) {
			for(UINT j = 0; j < BytesRead / sizeof(TCHAR); j++) {
				if (Buffer[j] == _T(' ') || Buffer[j] == _T('\n')) {
					wordsNumber++;
				}
			}
			DWORD len = _stprintf_s(Buffer, BufferSize, _T("%s %d\n\n"), argv[i], wordsNumber) - 1; //idk why, but 1 endline doesn't work
			if (!GeneralWrite(false, stdoutHandle, Buffer, len, &BytesWritten) 
				|| BytesWritten != len) {
					CloseHandleSafe(file);
					return FALSE;
			}
		}
		if (BytesRead > 0) {
			PrintError();
			CloseHandleSafe(file);
			return FALSE;
		}
		CloseHandleSafe(file);
	}
	return TRUE;
}

BOOL WINAPI CtrlHandler(DWORD ctrlType) {
    if (ctrlType ==CTRL_C_EVENT) {
        _tprintf(_T("\nCtrl-C exit\n\n"));
        return FALSE;
	} else {
        return TRUE;
    }
}

// Problem 17
// give only filtered (without executable's name etc.) arguments
BOOL printFile(int argc, TCHAR* argv[]) {
	
    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
		PrintError();
		return FALSE;
	}

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
		CloseHandleSafe(file);
		return FALSE;
	}
	CloseHandleSafe(file);
	return TRUE;
}