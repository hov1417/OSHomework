#pragma once

#define handleFileError(handle1,handle2)  PrintError(); \
	CloseHandle(handle1); \
	CloseHandle(handle2); \
	return FALSE;

enum COPY_ELEMENT_TYPE {
	ELEMENT_BYTE,
	ELEMENT_WORD,
	ELEMENT_LINE
};

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
BOOL changeTime(TCHAR* fileName);
BOOL addToPATH(int argc, TCHAR* argv[]);
BOOL calculateWords(int argc, TCHAR* argv[]);
BOOL printFile(int argc, TCHAR* argv[]);