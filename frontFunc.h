#pragma once
#include"./back/utils.h"
#include<qdebug.h>
#include<qstring.h>
#include<QJsonObject>
#include<qjsonarray.h>
#include<QJsonDocument>
#include<qjsonvalue.h>
namespace mysearch {
	//��Щ������Ϊ��mainwindow��ʾ�����
	QStringList get_articleInfo(string title);//�õ�������Ϣ
	QStringList get_author_titles(string author);//�õ����ߵ���������
	QStringList get_author_authors(string author);//�õ�������
	QStringList get_author_rank();//�õ���������
	bool isCliqueInit();//�жϾ����Ƿ��ʼ��
	QStringList get_clique();//��ȡ�����ļ�������
	QString get_js(string name);//�õ����ӻ���Ϣ
	//QString test_js();
}
