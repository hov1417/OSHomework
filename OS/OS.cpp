#include "stdafx.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))

#define prepareHandlesIfError(handle1,handle2)  PrintError(); \
	CloseHandle(handle1); \
	CloseHandle(handle2); \
	return FALSE; 



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
void PrintInfo(_TCHAR **envp) {
	const DWORD BufferSize = 2500;
	TCHAR Buffer[BufferSize];

	GetCurrentDirectory(BufferSize, Buffer);
	_tprintf(_T("Current Directory: %s\n"), Buffer);

	_TCHAR** current = envp;
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
		WriteFile(file2, Buffer, BytesRead, &BytesWritten, NULL);
		if (BytesWritten != BytesRead) {
			prepareHandlesIfError(file1,file2);
		}
	}
	if (BytesRead > 0) {
		prepareHandlesIfError(file1,file2);
	}
	CloseHandle(file1);
	CloseHandle(file2);
	return TRUE;
}


//Problem 4
BOOL cpyFileReverse(TCHAR* name1, TCHAR* name2) {

	const DWORD BufferSize = 2000;
	TCHAR Buffer[BufferSize];

	DWORD FileSize, BytesRead, BytesWritten;


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

	GetFileSize(file1, &FileSize);
	BOOL readResponse;

	SetFilePointer(file1, 0, NULL, FILE_END);
	DWORD currentPointer = SetFilePointer(file1, 0, NULL, FILE_CURRENT);
	DWORD BytesToRead = BufferSize;
	while(currentPointer != 0) {
		if(SetFilePointer(file1, -BufferSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER) {
			if(GetLastError() == ERROR_NEGATIVE_SEEK) { //seting pointer before begining
				BytesToRead = currentPointer;
				SetFilePointer(file1, 0, NULL, FILE_BEGIN);
			} else {
				prepareHandlesIfError(file1,file2);
			}
		}
		readResponse = ReadFile(file1, Buffer, BytesToRead, &BytesRead, NULL);
		//reverse readed
		if(SetFilePointer(file1, -BytesRead, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER) {
			prepareHandlesIfError(file1,file2);
		}
		//reversing
		int count = BytesRead/sizeof(TCHAR);
		for (int i = 0; i < count/2; i++) {
			TCHAR tmp = Buffer[i];
			Buffer[i] = Buffer[count - i - 1];
			Buffer[count - i - 1] = tmp;
		}

		currentPointer = SetFilePointer(file1, 0, NULL, FILE_CURRENT);
		if(!readResponse || !WriteFile(file2, Buffer, BytesRead, &BytesWritten, NULL)
			|| BytesRead < 0 || BytesRead != BytesWritten) {
				prepareHandlesIfError(file1,file2);
		}
	}
	CloseHandle(file1);
	CloseHandle(file2);
	return TRUE;
}
#pragma endregion


int _tmain(int argc, _TCHAR* argv[], _TCHAR **envp) {
	//Tests
	if (argc == 1) {
		_tprintf(_T("No arguments are given\n"));
		return 0;
	}
	if (_tcscmp(argv[1], _T("test")) == 0 && argc == 3) {
		switch(_tstoi(argv[2])) {
		case 1:
			SetLastError(3);
			PrintError();
			break;
		case 2:
			PrintInfo(envp);
			break;
		case 3:
			_tprintf(_T("coping file 1.txt to 2.txt\n"));
			if(!cpyFile(_T("1.txt"), _T("2.txt"))) {
				_tprintf(_T("\nERROR!!\n"));
			}
			break;
		case 4:
			_tprintf(_T("coping file 1.txt to 2.txt reversed\n"));
			if(!cpyFileReverse(_T("1.txt"), _T("2.txt"))) {
				_tprintf(_T("\nERROR!!\n"));
			}
			break;
		}
	}
	return 0;
}

