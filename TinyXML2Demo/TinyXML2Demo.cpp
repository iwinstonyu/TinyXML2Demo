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

void DebugElement(FILE* fp, const char* pszPrefix, const char* pszName, const char* pszType)
{
	if (!strncmp(pszType, "Ms", 2))
	{
		fprintf(fp, "	oss << DebugClass(%s%s);\n", pszPrefix, pszName);
	}
	else if (!strncmp(pszType, "vector", 6))
	{
		char pszVecType[64] = { 0 };
		const char* pszLT = strchr(pszType, '<');
		const char* pszGT = strrchr(pszType, '>');
		strncpy_s(pszVecType, 64, pszLT + 1, pszGT - pszLT - 1);
		fprintf(fp, "	for each (%s data in %s%s) \n", pszVecType, pszPrefix, pszName);
		fprintf(fp, "	{\n");
		DebugElement(fp, "", "data", pszVecType);
		fprintf(fp, "	}\n");
	}
	else if (!strncmp(pszType, "map", 3))
	{
		char keyType[64] = { 0 };
		char valType[64] = { 0 };
		const char* pszLT = strchr(pszType, '<');
		const char* pszComma = strchr(pszType, ',');
		const char* pszGT = strrchr(pszType, '>');
		strncpy_s(keyType, 64, pszLT + 1, pszComma - pszLT - 1);
		strncpy_s(valType, 64, pszComma + 1, pszGT - pszComma - 1);
		fprintf(fp, "	for each (pair<%s, %s> data in %s%s) \n", keyType, valType, pszPrefix, pszName);
		fprintf(fp, "	{\n");
		DebugElement(fp, "", "data.first", keyType);
		DebugElement(fp, "", "data.second", valType);
		fprintf(fp, "	}\n");
	}
	else
		fprintf(fp, "	oss << %s%s;\n", pszPrefix, pszName);
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
	fopen_s(&fp, "../AcgDemo/NodeAcg.h", "w");
	if (!fp)
	{
		printf("error open NodeAcg.h");
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

	fprintf(fp, "#include <stdio.h>\n");
	fprintf(fp, "#include <iostream>\n");
	fprintf(fp, "#include <string>\n");
	fprintf(fp, "#include <map>\n");
	fprintf(fp, "#include <set>\n");
	fprintf(fp, "#include <vector>\n");
	fprintf(fp, "#include <sstream>\n");
	fprintf(fp, "using namespace std;\n\n\n");
	fprintf(fp, "typedef unsigned int uint32;\n");
	fprintf(fp, "typedef string tstring;\n\n\n");

	XMLElement* pRoot = pDoc->RootElement();
	XMLElement* pMessage = pRoot->FirstChildElement();
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

int main()
{
	//printf("/\\");
	//printf("\n");

	AutoGenClass();

	FILE* fp;
	fopen_s(&fp, "../AcgDemo/NodeAcg.h", "a");
	XMLDocument *pDoc = new XMLDocument();
	XMLError errorId = pDoc->LoadFile(NODE_XML);
	if (errorId != XML_SUCCESS)
	{
		fclose(fp);
		printf("error load xml: %d %s\n", errorId, NODE_XML);
		system("pause");
		return 0;
	}

	XMLElement* pRoot = pDoc->RootElement();
	XMLElement* pMessage = pRoot->FirstChildElement();
	while (pMessage)
	{
		const char* pszName = pMessage->Attribute("name");
		const char* pszBase = pMessage->Attribute("base");
		fprintf(fp, "string DebugClass(class Ms%s& mb)\n", pszName);
		fprintf(fp, "{\n");
		fprintf(fp, "	ostringstream oss;\n");

		XMLElement* pVar = pMessage->FirstChildElement("Var");
		while (pVar)
		{
			const char* pszName = pVar->Attribute("name");
			const char* pszType = pVar->Attribute("type");

			DebugElement(fp, "mb.", pszName, pszType);

			//if( !strncmp(pszType, "Ms", 2) )
			//	printf("	oss << DebugClass(mb.%s);\n", pszName);
			//else if( !strncmp(pszType, "vector", 6))

			//else
			//	printf("	oss << DebugData(mb.%s);\n", pszName);

			pVar = pVar->NextSiblingElement();
		}

		fprintf(fp, "	return oss.str();\n");
		fprintf(fp, "}\n");
		
		pMessage = pMessage->NextSiblingElement();
	}

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

	vector<int> numVec;
	numVec.push_back(1);
	numVec.push_back(1);
	numVec.push_back(3);
	printf("debug vector: %s\n", DebugData(numVec).c_str());

	//map<int, int> numMap;
	//numMap[1] = 1;
	//numMap[2] = 2;
	//numMap[3] = 3;
	//printf("debug map: %s\n", DebugData(numMap).c_str());

	fclose(fp);

	system("pause");
    return 0;
}

