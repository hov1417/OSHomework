#include "stdafx.h"
#include "functions1.h"
#include "functions2.h"

int _tmain(int argc, TCHAR* argv[], TCHAR *envp[]) {
	//Tests
	if (argc == 1) {
		_tprintf(_T("No arguments are given\n%s test [test_no [subtest_no | argv]]"), argv[0]);
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
			copyFile(_T("a/1.txt"), _T("2.txt"));
			break;
		case 4:
			_tprintf(_T("copying file 1.txt to 2.txt reversed\n"));
			copyFileReverse(_T("1.txt"), _T("2.txt"));
			break;
		case 5: {
			DWORD subtest = (argc == 4) ? _tstoi(argv[3]) : 1;
			switch(subtest) {
			case 1:
				_tprintf(_T("copying 120 bytes from 1.txt to 2.txt\n"));
				firstNElements(_T("1.txt"), _T("2.txt"), 120, ELEMENT_BYTE);
				break;
			case 2:
				_tprintf(_T("copying 10 words from 1.txt to 2.txt\n"));
				firstNElements(_T("1.txt"), _T("2.txt"), 10, ELEMENT_WORD);
				break;
			case 3:
				_tprintf(_T("copying 3 lines from 1.txt to 2.txt\n"));
				firstNElements(_T("1.txt"), _T("2.txt"), 3, ELEMENT_LINE);
				break;
			}
			break;
				}

		case 6: {
			DWORD subtest = (argc == 4) ? _tstoi(argv[3]) : 1;
			switch(subtest) {
			case 1:
				_tprintf(_T("copying last 120 bytes from 1.txt to 2.txt\n"));
				lastNElements(_T("1.txt"), _T("2.txt"), 120, ELEMENT_BYTE);
				break;
			case 2:
				_tprintf(_T("copying last 10 words from 1.txt to 2.txt\n"));
				lastNElements(_T("1.txt"), _T("2.txt"), 10, ELEMENT_WORD);
				break;
			case 3:
				_tprintf(_T("copying last 3 lines from 1.txt to 2.txt\n"));
				lastNElements(_T("1.txt"), _T("2.txt"), 3, ELEMENT_LINE);
				break;
			}
			break;
				}
		case 7:
			_tprintf(_T("copying file 3.txt to stdout\n"));
			copyFileToStdout(_T("3.txt"));
			_tprintf(_T("\n"));
			break;

		case 8:
			_tprintf(_T("copying stdin to 3.txt\n"));
			copyStdinToFile(_T("3.txt"));
			_tprintf(_T("\n"));
			break;

		case 9:
			_tprintf(_T("copying stdin to stdout\ninput text and press enter\n"));
			copyStdinToStdout();
			_tprintf(_T("\n"));
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
			_tprintf(_T("Problem 12\n"));
			printEnvironmentVariable(argc - 3, argv + 3, envp);
			_tprintf(_T("\n"));
			break;
		case 13:
			_tprintf(_T("Last n lines\n"));
			printLastNLines(argc - 3, argv + 3);
			break;
		case 14:
			_tprintf(_T("changing file creation date\n"));
			changeTime(argv[3]);
			_tprintf(_T("\n"));
			break;
		case 15:
			_tprintf(_T("adding to PATH\n"));
			addToPATH(argc - 3, argv + 3);
			_tprintf(_T("\n"));
			break;
		case 16:
			_tprintf(_T("calculating words number\n"));
			calculateWords(argc - 3, argv + 3);
			_tprintf(_T("\n"));
			break;
		case 17:
			_tprintf(_T("printing file\n"));
			printFile(argc - 3, argv + 3);
			_tprintf(_T("\n"));
			break;


		default:
			_tprintf(_T("Unknown test\n"));
		}

	}
	
	if (_tcscmp(argv[1], _T("test2")) == 0 && argc >= 3) {
		switch(_tstoi(argv[2])) {
		case 1:
			_tprintf(_T("Current directory info\n"));
			PrintCurrentDirectory();
			break;
		case 2:
			_tprintf(_T("copy old.txt to new.txt, given directory\n"));
			CopyOldToNew(argc - 3, argv + 3);
			break;
		case 3:
			_tprintf(_T("print Directories\n"));
			PrintDirectories();
			break;
		case 4:
			_tprintf(_T("print Leaf Subdirectories\n"));
			LeafSubDirectories();
			break;
		case 5:
			_tprintf(_T("see problem 16 (test 16)\n"));
			break;
		case 6:
			_tprintf(_T("Unite Files\n"));
			UniteFiles(argc - 3, argv + 3);
			break;
		case 7:
			_tprintf(_T("Last directory files\n"));
			LastDirectory(argc - 3, argv + 3);
			break;
		default:
			_tprintf(_T("Unknown test\n"));
		}

	}
	return 0;
}
