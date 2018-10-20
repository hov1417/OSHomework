#pragma once


#define CloseHandleSafe(handle) if(!CloseHandle(handle)) { \
	PrintError();\
	return FALSE;\
	}

#define handleFileError(handle1,handle2)  PrintError(); \
	CloseHandleSafe(handle1); \
	CloseHandleSafe(handle2); \
	return FALSE;

enum COPY_ELEMENT_TYPE {
	ELEMENT_BYTE,
	ELEMENT_WORD,
	ELEMENT_LINE
};

#define GeneralWrite(isFile, h, B, R, W) (isFile ? WriteFile(h, B, R, W, NULL) : WriteConsole(h, B, R, W, NULL))



void PrintError();
void PrintInfo(TCHAR **envp);
BOOL copyFile(TCHAR* name1, TCHAR* name2);
BOOL copyFileReverse(TCHAR* name1, TCHAR* name2);
BOOL firstNElements(TCHAR* name1, TCHAR* name2, UINT n, COPY_ELEMENT_TYPE elemType);
BOOL lastNElements(TCHAR* name1, TCHAR* name2, UINT n, COPY_ELEMENT_TYPE elemType);
BOOL copyFileToStdout(TCHAR* name);
BOOL copyStdinToFile(TCHAR* name);
BOOL copyStdinToStdout();
BOOL copyHTH(HANDLE handle1, HANDLE handle2, bool isFile = true);
BOOL cpyAsciiToUnicode(TCHAR* name1, TCHAR* name2);
BOOL printEnvironmentVariable(int argc, TCHAR* argv[], TCHAR *envp[]);
BOOL printLastNLines(UINT argc, TCHAR* argv[]);
BOOL changeTime(int argc, TCHAR* argv[]);
BOOL addToPATH(int argc, TCHAR* argv[]);
BOOL calculateWords(int argc, TCHAR* argv[]);
BOOL printFile(int argc, TCHAR* argv[]);



BOOL printLastNLines(TCHAR* name, UINT n);