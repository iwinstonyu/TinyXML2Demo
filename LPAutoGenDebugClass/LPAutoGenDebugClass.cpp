// LPAutoGenDebugClass.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#pragma comment(lib, "User32.lib")

void DisplayErrorBox(LPTSTR lpszFunction);
int DisplayFiles(const TCHAR* pszPath, const TCHAR* pszPrefix);

int _tmain(int argc, TCHAR *argv[])
{
	size_t length_of_arg;
	DWORD dwError = 0;

	// If the directory is not specified as a command-line argument,
	// print usage.

	if (argc != 2)
	{
		_tprintf(TEXT("\nUsage: %s <directory name>\n"), argv[0]);
		return (-1);
	}

	// Check that the input path plus 3 is not longer than MAX_PATH.
	// Three characters are for the "\*" plus NULL appended below.

	StringCchLength(argv[1], MAX_PATH, &length_of_arg);

	if (length_of_arg > (MAX_PATH - 3))
	{
		_tprintf(TEXT("\nDirectory path is too long.\n"));
		return (-1);
	}

	_tprintf(TEXT("\nTarget directory is %s\n\n"), argv[1]);

	DisplayFiles(argv[1], TEXT(""));

	system("pause");
	return 0;
}




void DisplayErrorBox(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and clean up

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40)*sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

int DisplayFiles(const TCHAR* pszPath, const TCHAR* pszPrefix)
{
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	// Prepare string for use with FindFile functions.  First, copy the
	// string to a buffer, then append '\*' to the directory name.

	StringCchCopy(szDir, MAX_PATH, pszPath);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

	// Find the first file in the directory.

	hFind = FindFirstFile(szDir, &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		DisplayErrorBox(TEXT("FindFirstFile"));
		return dwError;
	}

	// List all the files in the directory with some info about them.

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			_tprintf(TEXT("%s%s   <DIR>\n"), pszPrefix, ffd.cFileName);

			if ( _tcsnccmp(ffd.cFileName, TEXT("."), 1) )
			{
				TCHAR szNextPrefix[MAX_PATH];
				StringCchCopy(szNextPrefix, MAX_PATH, pszPrefix);
				StringCchCat(szNextPrefix, MAX_PATH, TEXT("------"));

				TCHAR szNextDir[MAX_PATH];
				StringCchCopy(szNextDir, MAX_PATH, pszPath);
				StringCchPrintf(szNextDir + lstrlen(szNextDir), MAX_PATH, "\\%s", ffd.cFileName);

				DisplayFiles(szNextDir, szNextPrefix);
			}
		}
		else
		{
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
			//_tprintf(TEXT("%s%s   %lld bytes\n"), pszPrefix, ffd.cFileName, filesize.QuadPart);


			const TCHAR* pszExt = _tcsrchr(ffd.cFileName, '.');
			if (pszExt && !_tcsnccmp(pszExt, TEXT(".xml"), 4))
			{
				_tprintf(TEXT("%s%s   %lld bytes\n"), pszPrefix, ffd.cFileName, filesize.QuadPart);
			}
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
	{
		DisplayErrorBox(TEXT("FindFirstFile"));
	}

	FindClose(hFind);
	
	return dwError;
}
