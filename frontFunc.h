#pragma once
#include"./back/utils.h"
#include<qdebug.h>
#include<qstring.h>
#include<QJsonObject>
#include<qjsonarray.h>
#include<QJsonDocument>
#include<qjsonvalue.h>
namespace mysearch {
	//这些函数是为了mainwindow显示服务的
	QStringList get_articleInfo(string title);//得到文章信息
	QStringList get_author_titles(string author);//得到作者的所有文章
	QStringList get_author_authors(string author);//得到合作者
	QStringList get_author_rank();//得到作者排行
	bool isCliqueInit();//判断聚类是否初始化
	QStringList get_clique();//读取聚类文件并返回
	QString get_js(string name);//得到可视化信息
	//QString test_js();
}
