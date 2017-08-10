// AcgDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include "NodeAcg.h"
#include "tinyxml2.h"
using namespace std;

//void testthrow() throw()
//{
//	throw 1;
//}

int main()
{
	printf("acg demo\n");

	__int64 a = 11111111111111111;
	printf("%lld\n", a);
	printf("%I64d\n", a);

	long long b = 22222222222222222;
	printf("%lld\n", b);
	printf("%I64d\n", b);

	MsItemData item;
	item.idItemType = 1;
	item.amount = 2;
	cout << "item: " << DebugClass(item) << endl;

	MsClanInfo clan;
	clan.clanId = 1;
	clan.clanName = "123";
	cout << "clan: " << DebugClass(clan) << endl;

	MsUserInfoLite userInfoLite;
	userInfoLite.roleId = 1;
	userInfoLite.roleName = "123";
	cout << "userInfoLite: " << DebugClass(userInfoLite) << endl;

	MsUserInfo userInfo;
	static_cast<MsUserInfoLite&>(userInfo) = userInfoLite;
	userInfo.vipLev = 1;
	userInfo.lastLogin = 1;
	userInfo.clanInfo = clan;

	for (int i = 0; i < 5; ++i)
	{
		userInfo.numVec.push_back(i);
		userInfo.numMap[i] = i;

		MsItemData item;
		item.idItemType = i;
		item.amount = i * 100;
		userInfo.itemVec.push_back(item);

		userInfo.itemMap[item.idItemType] = item;
	}

	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j <= i; ++j)
		{
			userInfo.itemMapMap[i][j] = userInfo.itemMap[j];
		}
	}

	cout << "userInfo: " << DebugClass(userInfo) << endl;

	tinyxml2::XMLDocument document;
	tinyxml2::XMLHandle docHandle(&document);
	tinyxml2::XMLElement* child2 = docHandle.FirstChildElement("Document").FirstChildElement("Element").FirstChildElement().NextSiblingElement().ToElement();
	if (child2)
	{
	}

	system("pause");
    return 0;
}

