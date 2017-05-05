#include <stdio.h>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <sstream>
using namespace std;


typedef unsigned int uint32;
typedef string tstring;


class MsItemData
{
public:
	MsItemData(){}

public:
	uint32 idItemType;
	uint32 amount;
};


class MsClanInfo
{
public:
	MsClanInfo(){}

public:
	uint32 clanId;
	tstring clanName;
};


class MsUserInfoLite
{
public:
	MsUserInfoLite(){}

public:
	uint32 roleId;
	tstring roleName;
};


class MsUserInfo : public MsUserInfoLite
{
public:
	MsUserInfo(){}

public:
	uint32 vipLev;
	uint32 lastLogin;
	MsClanInfo clanInfo;
	vector<uint32> numVec;
	map<uint32,uint32> numMap;
	vector<MsItemData> itemVec;
	map<uint32,MsItemData> itemMap;
	map<uint32,map<uint32,MsItemData>> itemMapMap;
};


string DebugClass(class MsItemData& mb)
{
	ostringstream oss;
	oss << mb.idItemType;
	oss << mb.amount;
	return oss.str();
}
string DebugClass(class MsClanInfo& mb)
{
	ostringstream oss;
	oss << mb.clanId;
	oss << mb.clanName;
	return oss.str();
}
string DebugClass(class MsUserInfoLite& mb)
{
	ostringstream oss;
	oss << mb.roleId;
	oss << mb.roleName;
	return oss.str();
}
string DebugClass(class MsUserInfo& mb)
{
	ostringstream oss;
	oss << mb.vipLev;
	oss << mb.lastLogin;
	oss << DebugClass(mb.clanInfo);
	for each (uint32 data in mb.numVec) 
	{
	oss << data;
	}
	for each (pair<uint32, uint32> data in mb.numMap) 
	{
	oss << data.first;
	oss << data.second;
	}
	for each (MsItemData data in mb.itemVec) 
	{
	oss << DebugClass(data);
	}
	for each (pair<uint32, MsItemData> data in mb.itemMap) 
	{
	oss << data.first;
	oss << DebugClass(data.second);
	}
	for each (pair<uint32, map<uint32,MsItemData>> data in mb.itemMapMap) 
	{
	oss << data.first;
	for each (pair<uint32, MsItemData> data in data.second) 
	{
	oss << data.first;
	oss << DebugClass(data.second);
	}
	}
	return oss.str();
}
