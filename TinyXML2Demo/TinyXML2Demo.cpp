// TinyXML2Demo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "tinyxml2.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <time.h>
using namespace std;
using namespace tinyxml2;

const char* NODE_XML = "Node.xml";
const char* NODE_ACG_H = "../AcgDemo/NodeAcg.h";

template<class TData>
string DebugData(const TData& data)
{
	ostringstream oss;

	oss << data;

	return oss.str();
}

template<class TData>
string DebugData(const vector<TData>& dataVec)
{
	ostringstream oss;

	for each (const TData& data in dataVec)
	{
		oss << data << " ";
	}

	return oss.str();
}

template<class TData1, class TData2>
string DebugData(const map<TData1, TData2>& dataMap)
{
	ostringstream oss;

	for each (pair<TData1, TData2> data in dataMap)
	{
		oss << "[" << DebugData(data.first) << " " << DebugData(data.second) << "]";
	}

	return oss.str();
}

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

//string DebugClass(MsUserInfoLite& mb)
//{
//	ostringstream oss;
//
//	oss << DebugData(mb.roleId) << " " << DebugData(mb.roleName);
//
//	return oss.str();
//}

void AutoGenClass()
{
	FILE* fp;
	fopen_s(&fp, NODE_ACG_H, "w");
	if (!fp)
	{
		printf("error open file %s\n", NODE_ACG_H);
		return;
	}

	XMLDocument *pDoc = new XMLDocument();
	XMLError errorId = pDoc->LoadFile(NODE_XML);
	if (errorId != XML_SUCCESS)
	{
		fclose(fp);
		printf("error load xml: %d %s\n", errorId, NODE_XML);
		return;
	}

	XMLElement* pRoot = pDoc->RootElement();

	XMLElement* pCppText = pRoot->FirstChildElement("CppText");
	fprintf(fp, "%s\n", pCppText->GetText());

	XMLElement* pMessage = pRoot->FirstChildElement("Message");
	while (pMessage)
	{
		const char* pszName = pMessage->Attribute("name");
		const char* pszBase = pMessage->Attribute("base");
		fprintf(fp, "class Ms%s", pszName);
		if (pszBase)
			fprintf(fp, " : public Ms%s", pszBase);
		fprintf(fp, "\n");
		fprintf(fp, "{\n");
		fprintf(fp, "public:\n");
		fprintf(fp, "	Ms%s(){}\n", pszName);
		fprintf(fp, "\npublic:\n");

		XMLElement* pVar = pMessage->FirstChildElement("Var");
		while (pVar)
		{
			const char* pszName = pVar->Attribute("name");
			const char* pszType = pVar->Attribute("type");

			fprintf(fp, "	%s %s;\n", pszType, pszName);

			pVar = pVar->NextSiblingElement();
		}

		fprintf(fp, "};\n\n\n");

		pMessage = pMessage->NextSiblingElement();
	}

	fclose(fp);
}

void AutoGenDebugClass()
{
	FILE* fp;
	fopen_s(&fp, NODE_ACG_H, "a");
	if (!fp)
	{
		printf("error open file: %s\n", NODE_ACG_H);
		return;
	}

	XMLDocument *pDoc = new XMLDocument();
	XMLError errorId = pDoc->LoadFile(NODE_XML);
	if (errorId != XML_SUCCESS)
	{
		fclose(fp);
		printf("error load xml: %d %s\n", errorId, NODE_XML);
		return;
	}

	XMLElement* pRoot = pDoc->RootElement();
	XMLElement* pMessage = pRoot->FirstChildElement("Message");
	while (pMessage)
	{
		const char* pszName = pMessage->Attribute("name");
		const char* pszBase = pMessage->Attribute("base");
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

		pMessage = pMessage->NextSiblingElement();
	}

	fclose(fp);
}

int main()
{
	//printf("/\\");
	//printf("\n");

	AutoGenClass();
	AutoGenDebugClass();

	//FILE* fp;
	//fopen_s(&fp, "../AcgDemo/NodeAcg.h", "a");
	//XMLDocument *pDoc = new XMLDocument();
	//XMLError errorId = pDoc->LoadFile(NODE_XML);
	//if (errorId != XML_SUCCESS)
	//{
	//	fclose(fp);
	//	printf("error load xml: %d %s\n", errorId, NODE_XML);
	//	system("pause");
	//	return 0;
	//}

	//XMLElement* pRoot = pDoc->RootElement();
	//XMLElement* pMessage = pRoot->FirstChildElement("Message");
	//while (pMessage)
	//{
	//	const char* pszName = pMessage->Attribute("name");
	//	const char* pszBase = pMessage->Attribute("base");
	//	fprintf(fp, "string DebugClass(class Ms%s& mb)\n", pszName);
	//	fprintf(fp, "{\n");
	//	fprintf(fp, "	ostringstream oss;\n");

	//	XMLElement* pVar = pMessage->FirstChildElement("Var");
	//	while (pVar)
	//	{
	//		const char* pszName = pVar->Attribute("name");
	//		const char* pszType = pVar->Attribute("type");

	//		DebugElement(fp, "mb.", pszName, pszType);

	//		//if( !strncmp(pszType, "Ms", 2) )
	//		//	printf("	oss << DebugClass(mb.%s);\n", pszName);
	//		//else if( !strncmp(pszType, "vector", 6))

	//		//else
	//		//	printf("	oss << DebugData(mb.%s);\n", pszName);

	//		pVar = pVar->NextSiblingElement();
	//	}

	//	fprintf(fp, "	return oss.str();\n");
	//	fprintf(fp, "}\n");
	//	
	//	pMessage = pMessage->NextSiblingElement();
	//}

	//while (pMessage)
	//{
	//	const char* pszName = pMessage->Attribute("name");
	//	const char* pszBase = pMessage->Attribute("base");

	//	XMLElement* pVar = pMessage->FirstChildElement("Var");
	//	while (pVar)
	//	{
	//		printf("var name:%s type:%s\n", pVar->Attribute("name"), pVar->Attribute("type"));

	//		pVar = pVar->NextSiblingElement();
	//	}


	//	pMessage = pMessage->NextSiblingElement();
	//}

	//int nNum = 1;
	//printf("debug 1: %s\n", DebugData(nNum).c_str());

	//vector<int> numVec;
	//numVec.push_back(1);
	//numVec.push_back(1);
	//numVec.push_back(3);
	//printf("debug vector: %s\n", DebugData(numVec).c_str());

	//map<int, int> numMap;
	//numMap[1] = 1;
	//numMap[2] = 2;
	//numMap[3] = 3;
	//printf("debug map: %s\n", DebugData(numMap).c_str());

	//fclose(fp);

	system("pause");
    return 0;
}

