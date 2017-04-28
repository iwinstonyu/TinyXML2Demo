// TinyXML2Demo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "tinyxml2.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
using namespace std;
using namespace tinyxml2;

const char* NODE_XML = "Node.xml";

template<class TData>
string DebugData(TData& data)
{
	ostringstream oss;

	oss << data;

	return oss.str();
}

template<class TData>
string DebugData(const vector<TData>& dataVec)
{
	ostringstream oss;

	for each (TData data in dataVec)
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

void DebugData(const char* pszName, const char* pszType)
{
	if (!strncmp(pszType, "Ms", 2))
	{
		printf("	oss << DebugClass(mb.%s);\n", pszName);
	}
	else if (!strncmp(pszType, "vector", 6))
	{
		printf("	for each (auto& data in %s) \n", pszName);
		{
			oss << data << " ";
		}
	}

	else
		printf("	oss << DebugData(mb.%s);\n", pszName);

}

//string DebugClass(MsUserInfoLite& mb)
//{
//	ostringstream oss;
//
//	oss << DebugData(mb.roleId) << " " << DebugData(mb.roleName);
//
//	return oss.str();
//}



int main()
{
	XMLDocument *pDoc = new XMLDocument();
	XMLError errorId = pDoc->LoadFile(NODE_XML);
	if (errorId != XML_SUCCESS)
	{
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
		printf("string DebugClass(class %s& mb)\n", pszName);
		printf("{\n");
		printf("	ostringstream oss;\n");

		XMLElement* pVar = pMessage->FirstChildElement("Var");
		while (pVar)
		{
			const char* pszName = pVar->Attribute("name");
			const char* pszType = pVar->Attribute("type");
			if( !strncmp(pszType, "Ms", 2) )
				printf("	oss << DebugClass(mb.%s);\n", pszName);
			else if( !strncmp(pszType, "vector"))
			else
				printf("	oss << DebugData(mb.%s);\n", pszName);

			pVar = pVar->NextSiblingElement();
		}

		printf("	return oss.str();\n");
		printf("}\n");
		


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

	printf("debug 1: %s\n", DebugData(1).c_str());

	vector<int> numVec;
	numVec.push_back(1);
	numVec.push_back(1);
	numVec.push_back(3);
	printf("debug vector: %s\n", DebugData(numVec).c_str());

	map<int, int> numMap;
	numMap[1] = 1;
	numMap[2] = 2;
	numMap[3] = 3;
	printf("debug map: %s\n", DebugData(numMap).c_str());

	system("pause");
    return 0;
}

