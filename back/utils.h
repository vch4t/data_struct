//���ļ���������������Ҫ���ļ�ͷ���Լ�һЩ���ܺ���
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
//��ϣ�������������ȷ�ɢ�����Ϣ
inline size_t ELFhash(const string& tar) {
	hash<string> h;
	return h(tar) & 0xfff;
}
bool isStart(const string &tempstr);//�ж��Ƿ���һƪ���µ���Ϣ��ͷ
bool isTitle(const string& tempstr);//�ж��Ƿ���һƪ���µı���
bool isAuthor(const string& tempstr);//�ж��Ƿ���������Ϣ
bool isTail(const string& tempstr);//�ж��Ƿ���һƪ���µ���Ϣ��β
bool isHomePage(const string& buf);//�ж��Ƿ���һƪ����Ψһ���ֵ�����
//�ж�xml�ļ��Ƿ񱻳�ʼ��
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
