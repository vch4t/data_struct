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
    //���ְ�������Ĳۺ���
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
    //��ѡ�İ�ť��
    QButtonGroup btnGroup;//��߰�����
    QButtonGroup searchGroup;//���������




    bool fuzzInit;//ģ��������ʼ�����
    bool cliqueInit;//���������ʼ�����
    void show_titleInfo(string name);//���������Ϣ
    void show_authors_tiitles(const QStringList &list);//������ߵ��������»�����ߵ���Ϣ
    std::thread* working;//���̣߳������ں�̨����


signals:
    void inited(int);//��ʼ���������ź�
    void cliqueinited(int);//��������������ź�


private slots:
    void show_message(int use_time);//�������ʱ��Ĳۺ���
    void set_page(int idx);//��߰����Ĳۺ���
    void onItemClicked(QListWidgetItem* item);//��������ĵ���¼�����ת
    void onItemDoubleClicked(QListWidgetItem* item);//���������˫���¼�����ת
    void show_clique_message(int use_time);//��������������Ѽ�����
    //void run_js(bool);
public:
    //���ں�̨��������mainwindow
    static DataStructure* window;
    //���ں�̨������mainwindow������
    void set_choosefile_en(time_t start, time_t end);
    void set_cliqueButton(time_t start, time_t end);
};
