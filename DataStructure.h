#pragma once

#include <QtWidgets/QMainWindow>
#include<QButtonGroup>
#include<string>
#include<qdebug.h>
#include<qfile.h>
#include<qfiledialog.h>
#include "ui_DataStructure.h"
#include<thread>
#include<qthread.h>
#include"frontFunc.h"
#include"back/FuzzSearch.h"
#include"back/KeywordRank.h"


class DataStructure : public QMainWindow
{
    Q_OBJECT

public:
    DataStructure(QWidget *parent = nullptr);
    ~DataStructure();

private slots:
    //各种按键点击的槽函数
    void on_page1_choosefile_clicked();
    void on_page2_search_clicked();
    void on_authorRank_clicked();
    void on_keywordRank_clicked();
    void on_page3_search_clicked();
    void on_helpTips_triggered();
    void on_tagTips_triggered();
    void on_page1_knowapp_clicked();
    void on_page1_knowdblp_clicked();
    void on_page1_knowtag_clicked();



private:
    Ui::DataStructureClass ui;
    //单选的按钮组
    QButtonGroup btnGroup;//侧边按键的
    QButtonGroup searchGroup;//搜索界面的




    bool fuzzInit;//模糊搜索初始化标记
    bool cliqueInit;//聚类分析初始化标记
    void show_titleInfo(string name);//输出文章信息
    void show_authors_tiitles(const QStringList &list);//输出作者的所有文章或合作者的信息
    std::thread* working;//多线程，用于在后台运行


signals:
    void inited(int);//初始化结束的信号
    void cliqueinited(int);//聚类分析结束的信号


private slots:
    void show_message(int use_time);//输出所用时间的槽函数
    void set_page(int idx);//侧边按键的槽函数
    void onItemClicked(QListWidgetItem* item);//搜索结果的点击事件，跳转
    void onItemDoubleClicked(QListWidgetItem* item);//搜索结果的双击事件，跳转
    void show_clique_message(int use_time);//聚类分析结束提醒及设置
    //void run_js(bool);
public:
    //用于后台函数调用mainwindow
    static DataStructure* window;
    //用于后台函数对mainwindow的设置
    void set_choosefile_en(time_t start, time_t end);
    void set_cliqueButton(time_t start, time_t end);
};
