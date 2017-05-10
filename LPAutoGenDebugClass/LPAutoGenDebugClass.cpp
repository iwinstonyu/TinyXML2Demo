// LPAutoGenDebugClass.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include <iostream>
#include <set>
#include <vector>
#include <string>
#include "tinyxml2.h"
#pragma comment(lib, "User32.lib")
using namespace tinyxml2;
using namespace std;

#ifdef _UNICODE
typedef wstring tstring;
#else
typedef string tstring;
#endif

const char* LP_AGDC_H = "..\\..\\Src\\Server\\Acg\\LPAgdc.h";
const char* DEBUG_CLASS_XML = "DebugClass.xml";

void DisplayErrorBox(LPTSTR lpszFunction);
int DisplayFiles(const TCHAR* pszPath, const TCHAR* pszPrefix);

void DebugElement(FILE* fp, const char* pszSpace, const char* pszPrefix, const char* pszName, const char* pszType, bool bPrintName)
{
	if (bPrintName)
		fprintf(fp, "%soss << \"%s:\"", pszSpace, pszName);
	else
		fprintf(fp, "%soss", pszSpace);

	if (!strncmp(pszType, "Ms", 2))
	{
		fprintf(fp, " << DebugClass(%s%s);\n", pszPrefix, pszName);
	}
	else if (!strncmp(pszType, "vector", 6))
	{
		fprintf(fp, " << \"[ \";\n");

		char pszVecType[64] = { 0 };
		const char* pszLT = strchr(pszType, '<');
		const char* pszGT = strrchr(pszType, '>');
		strncpy_s(pszVecType, 64, pszLT + 1, pszGT - pszLT - 1);
		fprintf(fp, "%sfor_each(%s%s.begin(), %s%s.end(), [&oss](const %s& data)->void\n", pszSpace, pszPrefix, pszName, pszPrefix, pszName, pszVecType);
		fprintf(fp, "%s{\n", pszSpace);
		char pszNewSpace[1024] = { 0 };
		strncpy_s(pszNewSpace, 1024, pszSpace, strlen(pszSpace));
		strcat_s(pszNewSpace, 1024, "	");
		DebugElement(fp, pszNewSpace, "", "data", pszVecType, false);
		fprintf(fp, "%s});\n", pszSpace);
		fprintf(fp, "%soss << \"] \";\n", pszSpace);
	}
	else if (!strncmp(pszType, "map", 3))
	{
		fprintf(fp, " << \"{ \";\n");

		char keyType[64] = { 0 };
		char valType[64] = { 0 };
		char pairType[64] = { 0 };
		const char* pszLT = strchr(pszType, '<');
		const char* pszComma = strchr(pszType, ',');
		const char* pszGT = strrchr(pszType, '>');
		strncpy_s(keyType, 64, pszLT + 1, pszComma - pszLT - 1);
		strncpy_s(valType, 64, pszComma + 1, pszGT - pszComma - 1);
		strncpy_s(pairType, 64, pszLT + 1, pszGT - pszLT - 1);
		fprintf(fp, "%sfor_each(%s%s.begin(), %s%s.end(), [&oss](const pair<%s>& data)->void\n", pszSpace, pszPrefix, pszName, pszPrefix, pszName, pairType);
		fprintf(fp, "%s{\n", pszSpace);
		char pszNewSpace[1024] = { 0 };
		strncpy_s(pszNewSpace, 1024, pszSpace, strlen(pszSpace));
		strcat_s(pszNewSpace, 1024, "	");
		fprintf(fp, "%soss << \"[ \";\n", pszNewSpace);
		DebugElement(fp, pszNewSpace, "", "data.first", keyType, false);
		DebugElement(fp, pszNewSpace, "", "data.second", valType, false);
		fprintf(fp, "%soss << \"]\";\n", pszNewSpace);
		fprintf(fp, "%s});\n", pszSpace);
		fprintf(fp, "%soss << \" } \";\n", pszSpace);
	}
	else
	{
		fprintf(fp, " << %s%s << \" \";\n", pszPrefix, pszName);
	}
}

void GenDebugClass(FILE* fp, tstring& file, set<string>& needDebugClass)
{
	tinyxml2::XMLDocument *pDoc = new tinyxml2::XMLDocument();
	XMLError errorId = pDoc->LoadFile(file.c_str());
	if (errorId != XML_SUCCESS)
	{
		printf("error load xml: %d %s\n", errorId, file.c_str());
		pDoc->PrintError();
		return;
	}

	XMLElement* pRoot = pDoc->RootElement();

	XMLElement* pMessage = pRoot->FirstChildElement();
	while (pMessage)
	{
		if (!strncmp(pMessage->Name(), "Message", 7) || !strncmp(pMessage->Name(), "Struct", 6))
		{
			const char* pszName = pMessage->Attribute("name");
			const char* pszBase = pMessage->Attribute("base");

			if (!needDebugClass.count(pszName))
			{
				pMessage = pMessage->NextSiblingElement();
				continue;
			}

			printf("generating debug class %s\n", pszName);

			fprintf(fp, "string DebugClass(const class Ms%s& mb)\n", pszName);
			fprintf(fp, "{\n");
			char pszSpace[1024] = "	";
			fprintf(fp, "%sostringstream oss;\n", pszSpace);
			fprintf(fp, "%soss << \"{ \";\n", pszSpace);

			if (pszBase)
			{
				fprintf(fp, "%soss << \"Ms%s: \" << DebugClass(static_cast<const Ms%s&>(mb));\n", pszSpace, pszBase, pszBase);
			}

			XMLElement* pVar = pMessage->FirstChildElement("Var");
			while (pVar)
			{
				const char* pszName = pVar->Attribute("name");
				const char* pszType = pVar->Attribute("type");

				DebugElement(fp, pszSpace, "mb.", pszName, pszType, true);

				pVar = pVar->NextSiblingElement();
			}

			fprintf(fp, "%soss << \"} \";\n", pszSpace);
			fprintf(fp, "%sreturn oss.str();\n", pszSpace);
			fprintf(fp, "}\n");
		}

		pMessage = pMessage->NextSiblingElement();
	}

	delete pDoc;
}

void LoadDebugClassXml()
{
	std::set<string> needDebugClass;

	FILE* fp;
	fopen_s(&fp, LP_AGDC_H, "a");
	if (!fp)
	{
		printf("error open file: %s\n", LP_AGDC_H);
		return;
	}

	tinyxml2::XMLDocument *pDoc = new tinyxml2::XMLDocument();
	XMLError errorId = pDoc->LoadFile(DEBUG_CLASS_XML);
	if (errorId != XML_SUCCESS)
	{
		fclose(fp);
		printf("error load xml: %d %s\n", errorId, DEBUG_CLASS_XML);
		return;
	}

	XMLElement* pRoot = pDoc->RootElement();

	XMLElement* pMessage = pRoot->FirstChildElement("DebugClass");
	printf("Loading classes to generate debug info...\n");
	if (pMessage)
	{
		XMLElement* pVar = pMessage->FirstChildElement("Var");
		while (pVar)
		{
			const char* pszName = pVar->Attribute("name");
			if (pszName)
			{
				needDebugClass.insert(pszName);
				printf("load: %s\n", pszName);
			}

			pVar = pVar->NextSiblingElement();
		}
	}



	fclose(fp);
}

bool LoadNeedDebugClass(FILE* fp, set<string>& needDebugClass)
{
	tinyxml2::XMLDocument *pDoc = new tinyxml2::XMLDocument();
	XMLError errorId = pDoc->LoadFile(DEBUG_CLASS_XML);
	if (errorId != XML_SUCCESS)
	{
		printf("error load xml: %d %s\n", errorId, DEBUG_CLASS_XML);
		return false;
	}

	XMLElement* pRoot = pDoc->RootElement();

	XMLElement* pCppTextBeg = pRoot->FirstChildElement("CppTextBeg");
	fprintf(fp, "%s\n", pCppTextBeg->GetText());

	XMLElement* pMessage = pRoot->FirstChildElement("DebugClass");
	printf("==================================\n");
	printf("Loading need debug class from %s\n", DEBUG_CLASS_XML);
	if (pMessage)
	{
		XMLElement* pVar = pMessage->FirstChildElement("Var");
		while (pVar)
		{
			const char* pszName = pVar->Attribute("name");
			if (pszName)
			{
				needDebugClass.insert(pszName);
				printf("load: %s\n", pszName);
			}

			pVar = pVar->NextSiblingElement();
		}
	}
	printf("==================================\n");

	return true;
}

int LoadXmlFiles(vector<tstring>& xmlFiles, const TCHAR* pszPath)
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
			//_tprintf(TEXT("%s%s   <DIR>\n"), pszPrefix, ffd.cFileName);

			if (_tcsnccmp(ffd.cFileName, TEXT("."), 1))
			{
				TCHAR szNextDir[MAX_PATH];
				StringCchCopy(szNextDir, MAX_PATH, pszPath);
				StringCchPrintf(szNextDir + lstrlen(szNextDir), MAX_PATH, TEXT("\\%s"), ffd.cFileName);

				LoadXmlFiles(xmlFiles, szNextDir);
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
				tstring strNextPath = pszPath;
				strNextPath += TEXT("\\");
				strNextPath += ffd.cFileName;
				xmlFiles.push_back(strNextPath);
				_tprintf(TEXT("load %s\n"), strNextPath.c_str());
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

void AutoGenDebugClass()
{
	std::set<string> needDebugClass;

	FILE* fp;
	fopen_s(&fp, LP_AGDC_H, "w");
	if (!fp)
	{
		printf("error open file: %s\n", LP_AGDC_H);
		return;
	}

	if (!LoadNeedDebugClass(fp, needDebugClass))
	{
		fclose(fp);
		return;
	}

	std::vector<tstring> xmlFiles;
	TCHAR szDir[MAX_PATH] = { 0 };
	//GetCurrentDirectory(MAX_PATH, szDir);
	StringCchCat(szDir, MAX_PATH, TEXT("..\\..\\Src"));
	_tprintf(TEXT("==================================\n"));
	_tprintf(TEXT("Loading xmlFiles from %s\n"), szDir);
	if (LoadXmlFiles(xmlFiles, szDir) != ERROR_NO_MORE_FILES)
	{
		fclose(fp);
		return;
	}
	_tprintf(TEXT("==================================\n"));


	_tprintf(TEXT("==================================\n"));
	for (auto it = xmlFiles.begin(); it != xmlFiles.end(); ++it)
	{
		auto& file = *it;

		_tprintf(TEXT("Parsing %s\n"), file.c_str());
		GenDebugClass(fp, file, needDebugClass);
	}
	_tprintf(TEXT("==================================\n"));

	fprintf(fp, "\n}");

	fclose(fp);
}


int _tmain(int argc, TCHAR *argv[])
{
	//size_t length_of_arg;
	//DWORD dwError = 0;

	//// If the directory is not specified as a command-line argument,
	//// print usage.

	//if (argc != 2)
	//{
	//	_tprintf(TEXT("\nUsage: %s <directory name>\n"), argv[0]);
	//	return (-1);
	//}

	//// Check that the input path plus 3 is not longer than MAX_PATH.
	//// Three characters are for the "\*" plus NULL appended below.

	//StringCchLength(argv[1], MAX_PATH, &length_of_arg);

	//if (length_of_arg > (MAX_PATH - 3))
	//{
	//	_tprintf(TEXT("\nDirectory path is too long.\n"));
	//	return (-1);
	//}

	//_tprintf(TEXT("\nTarget directory is %s\n\n"), argv[1]);

	//DisplayFiles(argv[1], TEXT(""));

	AutoGenDebugClass();

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
			//_tprintf(TEXT("%s%s   <DIR>\n"), pszPrefix, ffd.cFileName);

			if ( _tcsnccmp(ffd.cFileName, TEXT("."), 1) )
			{
				TCHAR szNextPrefix[MAX_PATH];
				StringCchCopy(szNextPrefix, MAX_PATH, pszPrefix);
				StringCchCat(szNextPrefix, MAX_PATH, TEXT(""));

				TCHAR szNextDir[MAX_PATH];
				StringCchCopy(szNextDir, MAX_PATH, pszPath);
				StringCchPrintf(szNextDir + lstrlen(szNextDir), MAX_PATH, TEXT("\\%s"), ffd.cFileName);

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
				_tprintf(TEXT("%s%s\\%s   %lld bytes\n"), pszPrefix, pszPath, ffd.cFileName, filesize.QuadPart);
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