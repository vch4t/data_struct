//本文件是用来放置所需要的文件头，以及一些功能函数
#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<set>
#include<unordered_map>
#include<unordered_set>
#include <queue>
#include <thread>
#include<algorithm>
#include <functional>
#include<fstream>
#include<sstream>
#include<condition_variable>
#include<mutex>
#include<direct.h>
#include<Windows.h>
#include<qstring.h>
#include<qstringlist.h>


using namespace std;
//哈希函数，用来均匀分散相关信息
inline size_t ELFhash(const string& tar) {
	hash<string> h;
	return h(tar) & 0xfff;
}
bool isStart(const string &tempstr);//判断是否是一篇文章的信息开头
bool isTitle(const string& tempstr);//判断是否是一篇文章的标题
bool isAuthor(const string& tempstr);//判断是否是作者信息
bool isTail(const string& tempstr);//判断是否是一篇文章的信息结尾
bool isHomePage(const string& buf);//判断是否是一篇文章唯一名字的文章
//判断xml文件是否被初始化
inline bool isInit() {
	fstream infile("init.ini", ios::in);
	string flag;
	getline(infile, flag);
	if (flag == "1") {
		infile.close();
		return 1;
	}
	return 0;
}
