#include "DataStructure.h"
#include<QMessageBox>
#include"back/DataInit.h"
#include"back/CountCompleteSubgraph.h"
#include<qscrollbar.h>
#include <QWebEngineView>
#include <QtWebEngineWidgets>
#include<QListView>
#include<QJsonDocument>
#include<qwebchannel.h>
using std::string;

DataStructure* DataStructure::window = nullptr;

DataStructure::DataStructure(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    //��ʼ���ж�
    window = this;
    fuzzInit = 0;
    if (mysearch::isCliqueInit()) {
        cliqueInit = 1;
    }
    else {
        cliqueInit = 0;
    }
    if (isInit()) {
        ui.cliqueButton->setEnabled(1);
        ui.rankButton->setEnabled(1);
        ui.seeingButton->setEnabled(1);
        ui.searchButton->setEnabled(1);
    }
    setFixedSize(this->width(), this->height());
    //���ò�����İ�ť����
    ui.searchButton->setIcon(QIcon(":/image/rce/search.png"));
    ui.searchButton->setIconSize(QSize(80, 100));
    ui.initButton->setIcon(QIcon(":/image/rce/init.png"));
    ui.initButton->setIconSize(QSize(80, 100));
    ui.cliqueButton->setIcon(QIcon(":/image/rce/clique.png"));
    ui.cliqueButton->setIconSize(QSize(80, 100));
    ui.rankButton->setIcon(QIcon(":/image/rce/rank.png"));
    ui.rankButton->setIconSize(QSize(80, 100));
    ui.seeingButton->setIcon(QIcon(":/image/rce/seeing.png"));
    ui.seeingButton->setIconSize(QSize(80, 100));

    btnGroup.addButton(ui.initButton, 0);
    btnGroup.addButton(ui.searchButton, 1);
    btnGroup.addButton(ui.seeingButton, 2);
    btnGroup.addButton(ui.rankButton, 3);
    btnGroup.addButton(ui.cliqueButton, 4);
    btnGroup.button(0)->setChecked(true);
    ui.stackedWidget->setCurrentIndex(0);

    //������ʾ
    ui.initButton->setToolTip(u8"�����ݽ��г�������");
    ui.searchButton->setToolTip(u8"ͨ�������������������֡����߲���������������Ӧ�����»�������");
    ui.seeingButton->setToolTip(u8"�������ߣ���ʾ���������߼�ͨ��ʲô���º�����");
    ui.rankButton->setToolTip(u8"����ȴ����С�����������������");
    ui.cliqueButton->setToolTip(u8"����ͼ�и�����ȫ��ͼ�ĸ���");

    ui.page1_line->setPlaceholderText(u8"��ѡ�����dblp.xml�ļ�...");
    ui.page1_choosefile->setIcon(QIcon(":/image/rce/file.png"));
    ui.page1_choosefile->setIconSize(QSize(30, 30));


    connect(&btnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &DataStructure::set_page);
    connect(this, &DataStructure::inited, this, &DataStructure::show_message);
    connect(this, &DataStructure::cliqueinited, this, &DataStructure::show_clique_message);

    connect(ui.page2_listWidget, &QListWidget::itemDoubleClicked, this, &DataStructure::onItemDoubleClicked);
    connect(ui.page2_listWidget, &QListWidget::itemClicked, this, &DataStructure::onItemClicked);
    //connect(ui.widget_web, &QWebEngineView::loadFinished, this, &DataStructure::run_js);

    
}

void DataStructure::on_page1_choosefile_clicked() {
    //ѡ���ļ�
    QString dir = QFileDialog::getOpenFileName(this, u8"ѡ���ļ�","./", "XML Files(*.xml)");
    string path = dir.toStdString();

    if(path.empty()) return;
    //������Ӧѡ��ʹ�ò�ͬ�ĳ�ʼ��ģʽ
    int ret = QMessageBox::question(this, u8"ѯ��", u8"��ռ��һ��CPU���ڴ�8G������ģʽ��Լ5G���ο�ʱ��Լ9���ӣ�����ͨģʽ��Լ8G���ο�ʱ��Լ5���ӣ���", u8"��������",u8"���е���ģʽ", u8"������ͨģʽ", 2);
    switch (ret) {
    case 0:
        QMessageBox::critical(this, u8"����", u8"δ��ʼ�������Խ�����������");
        break;
    case 1: {

        working = new std::thread(DataInit::data_init_low, path);
        ui.page1_choosefile->setDisabled(1);
        ofstream outfile("path.ini");
        outfile << path << endl;
        outfile.close();
        working->detach();
        ui.cliqueButton->setEnabled(0);
        ui.rankButton->setEnabled(0);
        ui.seeingButton->setEnabled(0);
        ui.searchButton->setEnabled(0);
    }
        break;
    case 2: {
        working = new std::thread(DataInit::data_init, path);
        ui.page1_choosefile->setDisabled(1);
        ofstream outfile("path.ini");
        outfile << path << endl;
        outfile.close();
        working->detach();
        ui.cliqueButton->setEnabled(0);
        ui.rankButton->setEnabled(0);
        ui.seeingButton->setEnabled(0);
        ui.searchButton->setEnabled(0);
    }
        break;
    }
}

void DataStructure::set_choosefile_en(time_t start,time_t end) {
    int use_time = end - start;
    emit inited(use_time);//���ͳ�ʼ������ź�
    ui.page1_choosefile->setEnabled(1);//����ѡ���ļ���ť�Ŀ�����
}
void DataStructure::set_cliqueButton(time_t start, time_t end) {
    //ͬ��
    int use_time = end - start;
    emit cliqueinited(use_time);
    ui.cliqueButton->setEnabled(1);
}
void DataStructure::show_message(int use_time) {
    //��ʾ��Ϣ�����ò�߰�ť�Ŀ�����
    QMessageBox::information(this, u8"����", u8"���ݳ�ʼ������������ʱ" + QString::number(use_time) + u8"��", u8"�õ�");
    ui.cliqueButton->setEnabled(1);
    ui.rankButton->setEnabled(1);
    ui.seeingButton->setEnabled(1);
    ui.searchButton->setEnabled(1);
}

void DataStructure::set_page(int idx) {
    //������ĵ������
    ui.stackedWidget->setCurrentIndex(idx);//��ʾ��ѡҳ
    switch (idx) {
    case 0:
        break;
    case 1:
        //������ص�����
        ui.page2_search->setIcon(QIcon(":/image/rce/search.png"));
        searchGroup.addButton(ui.page2_title_search, 0);
        searchGroup.addButton(ui.page2_author_search, 1);
        searchGroup.addButton(ui.page2_fuzz_search, 2);
        searchGroup.addButton(ui.page2_search_authors, 3);
        ui.page2_title_search->setChecked(1);
        ui.page2_listWidget->setWordWrap(1);
        ui.page2_title_search->setToolTip(u8"�����������֣����������Ϣ(����Ҫ��ȫ��������ĩβ�ķ���)");
        ui.page2_author_search->setToolTip(u8"�����������֣���������������");
        ui.page2_fuzz_search->setToolTip(u8"�����������ֵĲ������ʣ������������к��йؼ��ʵ�����");
        ui.page2_search_authors->setToolTip(u8"�����������֣�������������������������");
        break;
    case 2: {
        //�ҵ�html�ļ�����·��������
        string web_path = _getcwd(0, 0);
        web_path += "/index.html";
        for (int i = 0; i < web_path.size(); ++i) {
            if (web_path[i] == '\\') {
                web_path[i] = '/';
            }
        }
        ui.widget_web->page()->setUrl(QString::fromStdString(web_path));
        //ui.widget_web->page()->load((QUrl)("qrc:/source/index.html"));

    }
        break;
    case 3: {
        //���ý����������
        ui.page4_tableWidget->setAlternatingRowColors(1);
        ui.yearInput->setPlaceholderText(u8"������1936~2023�����");

    }
        break;
    case 4: {
        //δ�о��ദ�������ļ������д���
        if (!cliqueInit) {
            int ret = QMessageBox::information(this, u8"��ʾ", u8"��һ�δ���Ҫ�ϳ�ʱ����м���,��ռ���ڴ�", QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
            if (ret == QMessageBox::No) {
                break;
            }
            else {
                working=new std::thread(clique::clique_analysis);
                ui.cliqueButton->setDisabled(1);
                cliqueInit = 1;
                working->detach();
                break;
            }
        }
        //��ȡ�������ļ�����ʾ
        ui.page5_tableWidget->setAlternatingRowColors(1);
        ui.page5_tableWidget->setWordWrap(1);
        QStringList mylist = mysearch::get_clique();
        ui.page5_tableWidget->clear();
        ui.page5_tableWidget->setColumnCount(2);
        ui.page5_tableWidget->setHorizontalHeaderLabels(QStringList() << u8"����" << u8"����");
        ui.page5_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui.page5_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        int rowSz = mylist.size() / 2;
        ui.page5_tableWidget->setRowCount(rowSz);
        QTableWidgetItem* nameItem;
        QTableWidgetItem* cntItem;

        for (int i =  0; i <rowSz; ++i) {
            nameItem = new QTableWidgetItem(mylist.at(i * 2));
            cntItem = new QTableWidgetItem(mylist.at(i * 2 + 1));
            nameItem->setTextAlignment(Qt::AlignCenter);
            cntItem->setTextAlignment(Qt::AlignCenter);
            cntItem->setFlags(Qt::ItemIsEnabled);
            nameItem->setFlags(Qt::ItemIsEnabled);

            ui.page5_tableWidget->setItem(i, 0, nameItem);
            ui.page5_tableWidget->setItem(i, 1, cntItem);
        }
    }
        break;
    }

}

void DataStructure::on_page2_search_clicked() {
    //���ݶ�Ӧ�İ�ť��ѡ��,�ò�ͬ������������
    int idx = searchGroup.checkedId();
    ui.page2_listWidget->clear();
    switch (idx) {
    case 0: {
        //��������
        string name = ui.inputName->text().toStdString();
        show_titleInfo(name);
    }
        break;
    case 1: {
        //��������
        ui.page2_listWidget->setWordWrap(1);
        string name = ui.inputName->text().toStdString();
        QStringList mylist = mysearch::get_author_titles(name);
        if (mylist.empty()) {
            QListWidgetItem* item = new QListWidgetItem;
            item->setText(u8"û���ҵ����");
            item->setFont(QFont("����", 10, QFont::Bold));
            item->setFlags(Qt::ItemIsEnabled);
            ui.page2_listWidget->addItem(item);
            return;
        }
        show_authors_tiitles(mylist);
    }
        break;
    case 2: {
        //ģ������
        ui.page2_listWidget->setAlternatingRowColors(1);

        string name = ui.inputName->text().toStdString();
        if (!fuzzInit) {
            int ret = QMessageBox::information(this, u8"��ʾ", u8"��һ�δ���Ҫ�����ӽ��г�ʼ��,��ռ���ڴ�,�����»��п�������",QMessageBox::No|QMessageBox::Yes,QMessageBox::Yes);
            if (ret == QMessageBox::No) {
                break;
            }
        }
        //���������Ϣ
        QStringList mylist = fuzz::fuzz_search(name);
        fuzzInit = 1;
        QListWidgetItem* item = new QListWidgetItem;
        QString buf;
        ui.page2_listWidget->setWordWrap(0);

        ui.pageNum->setText(u8"�����������: " + QString::number(mylist.size()));
        ui.pageNum->setWordWrap(1);
        //���Ϊ�յĴ���
        if (mylist.empty()) {
            item->setText(u8"û���ҵ����");
            item->setFont(QFont("����", 10, QFont::Bold));
            item->setFlags(Qt::ItemIsEnabled);
            ui.page2_listWidget->addItem(item);
            return;
        }
        //����������
        for (int i = 0; i < mylist.size(); ++i) {
            buf = mylist.at(i);
            item = new QListWidgetItem;
            item->setText(buf);
            item->setFont(QFont("����", 10, QFont::Bold));
            item->setFlags(Qt::ItemIsEnabled);
            
            ui.page2_listWidget->addItem(item);
        }
    }
        break;
    case 3: {
        
        ui.page2_listWidget->setWordWrap(1);
        string name = ui.inputName->text().toStdString();
        QStringList mylist = mysearch::get_author_authors(name);
        //�մ���
        if (mylist.empty()) {
            QListWidgetItem* item = new QListWidgetItem;
            item->setText(u8"û���ҵ����");
            item->setFont(QFont("����", 10, QFont::Bold));
            item->setFlags(Qt::ItemIsEnabled);
            ui.page2_listWidget->addItem(item);
            return;
        }
        show_authors_tiitles(mylist);
    }
        break;
    }
}
void DataStructure::onItemDoubleClicked(QListWidgetItem* item) {
    //˫����ת�¼�
    int idx = searchGroup.checkedId();
    //���ݲ�ͬ��ťѡ�񣬴�����ת
    if (idx == 1 || idx == 2) {
        //��ת��������Ϣ
        string name = item->text().toStdString();
        ui.page2_listWidget->clear();
        show_titleInfo(name);
    }
    if (idx == 3) {
        //��ת�����ߵ�����
        ui.page2_author_search->setChecked(1);

        ui.page2_listWidget->setWordWrap(1);

        string name = item->text().toStdString();
        ui.page2_listWidget->clear();
        QStringList mylist = mysearch::get_author_titles(name);
        show_authors_tiitles(mylist);
    }
}
//��ʾ������Ϣ
void DataStructure::show_titleInfo(string name) {
    ui.page2_title_search->setChecked(1);
    ui.page2_listWidget->setAlternatingRowColors(0);

    QStringList mylist = mysearch::get_articleInfo(name);
    //�մ���
    if (mylist.empty()) {
        QListWidgetItem* item = new QListWidgetItem;
        item->setText(u8"û���ҵ����");
        item->setFont(QFont("����", 10, QFont::Bold));
        item->setFlags(Qt::ItemIsEnabled);
        ui.page2_listWidget->addItem(item);
        return;
    }
    ui.page2_listWidget->setWordWrap(1);
    //��ʾ����
    for (int i = 0; i < mylist.size();++i) {
        QListWidgetItem* item = new QListWidgetItem;
        QString buf = mylist.at(i);
        int listIdx = buf.lastIndexOf("<");
        buf = buf.mid(listIdx);
        item->setText(u8"���ԣ�");
        item->setBackgroundColor(Qt::gray);
        item->setFont(QFont("����", 10, QFont::Bold));
        item->setFlags(Qt::ItemIsEnabled);
        ui.page2_listWidget->addItem(item);
        item = new QListWidgetItem;
        item->setText(buf);
        item->setFont(QFont("����", 10));
        item->setFlags(Qt::ItemIsEnabled);
        ui.page2_listWidget->addItem(item);
        QString first_name = "";
        for (++i; i<mylist.size(); ++i) {
            if (isStart(mylist.at(i).toStdString())) {
                i--;
                break;
            }
            buf = mylist.at(i);
            if (buf.mid(0, buf.indexOf(">") + 1) != first_name) {
                first_name = buf.mid(0, buf.indexOf(">") + 1);
                item = new QListWidgetItem;
                item->setText(first_name);
                item->setBackgroundColor(Qt::gray);
                item->setFont(QFont("����", 10, QFont::Bold));
                item->setFlags(Qt::ItemIsEnabled);
                ui.page2_listWidget->addItem(item);
            }
            item = new QListWidgetItem;
            int l = buf.indexOf(">");
            int r = buf.lastIndexOf("<");
            item->setText(buf.mid(l + 1, r - l - 1));
            item->setFont(QFont("����", 10));
            item->setFlags(Qt::ItemIsEnabled);
            ui.page2_listWidget->addItem(item);
        }
    }
}

void DataStructure::show_authors_tiitles(const QStringList &mylist) {
    //��ʾ����
    ui.page2_listWidget->setAlternatingRowColors(1);

    ui.pageNum->setText(u8"�����������: " + QString::number(mylist.size()));
    ui.pageNum->setWordWrap(1);
    QListWidgetItem* item = new QListWidgetItem;
    QString buf;
    for (int i = 0; i < mylist.size(); ++i) {
        buf = mylist.at(i);
        item = new QListWidgetItem;
        item->setText( buf);
        item->setFont(QFont("����", 10, QFont::Bold));
        item->setFlags(Qt::ItemIsEnabled);
        ui.page2_listWidget->addItem(item);
    }
}
void DataStructure::on_authorRank_clicked() {
    //�����ƣ�������
    QStringList mylist = mysearch::get_author_rank();
    ui.page4_tableWidget->clear();
    ui.page4_tableWidget->setColumnCount(2);
    ui.page4_tableWidget->setHorizontalHeaderLabels(QStringList() << u8"����" << u8"����");
    ui.page4_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    int rowSz = mylist.size() / 2;
    ui.page4_tableWidget->setRowCount(rowSz);
    QTableWidgetItem* nameItem;
    QTableWidgetItem* cntItem;
    //����ʱ�ǵ���������һ��j��ȷ�����λ��
    for (int i = rowSz - 1, j = 0; i >= 0; --i, ++j) {
        nameItem = new QTableWidgetItem(mylist.at(i * 2));
        cntItem = new QTableWidgetItem(mylist.at(i * 2 + 1));
        nameItem->setTextAlignment(Qt::AlignCenter);
        cntItem->setTextAlignment(Qt::AlignCenter);
        ui.page4_tableWidget->setItem(j, 0, nameItem);
        ui.page4_tableWidget->setItem(j, 1, cntItem);
    }
}
void DataStructure::on_keywordRank_clicked() {
    //ͬ��
    string name = ui.yearInput->text().toStdString();
    if (name.empty()|| stoi(name) > 2023 || stoi(name) < 1936) {
        QMessageBox::critical(this, u8"����", u8"�����Ч����������û������!");
        return;
    }

        QStringList mylist = keyword::rank_keyword(name);
        ui.page4_tableWidget->clear();
        ui.page4_tableWidget->setColumnCount(2);
        ui.page4_tableWidget->setHorizontalHeaderLabels(QStringList() << u8"�ȴ�" << u8"����");
        ui.page4_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        int rowSz = mylist.size() / 2;
        ui.page4_tableWidget->setRowCount(rowSz);
        QTableWidgetItem* nameItem;
        QTableWidgetItem* cntItem;

        for (int i = rowSz - 1, j = 0; i >= 0; --i, ++j) {
            nameItem = new QTableWidgetItem(mylist.at(i * 2));
            cntItem = new QTableWidgetItem(mylist.at(i * 2 + 1));
            nameItem->setTextAlignment(Qt::AlignCenter);
            cntItem->setTextAlignment(Qt::AlignCenter);
            cntItem->setFlags(Qt::ItemIsEnabled);
            nameItem->setFlags(Qt::ItemIsEnabled);
            ui.page4_tableWidget->setItem(j, 0, nameItem);
            ui.page4_tableWidget->setItem(j, 1, cntItem);
        }

}

void DataStructure::on_page3_search_clicked() {
    if (ui.page3_input->text().isEmpty()) {
        QMessageBox::critical(this, u8"����", u8"����������������!");
        return;
    }
    string name = ui.page3_input->text().toStdString();
    //���ÿ��ӻ���Ϣ
    QString js = mysearch::get_js(name);
    
    //js = mysearch::test_js();
    QString setname = QString("setName(\"%1\")").arg(ui.page3_input->text());
    ui.widget_web->page()->runJavaScript(setname);//����ͼƬ����

    ui.widget_web->page()->runJavaScript(js);//����ͼƬ
    
}

void DataStructure::onItemClicked(QListWidgetItem* item) {
    //������ʾ��ǰ�������λ��
    int idxSearch = searchGroup.checkedId();
    if (!idxSearch) return;
    int idx=ui.page2_listWidget->row(item);
    ui.pageCurr->setText(u8"��ǰ��: " + QString::number(idx+1));
}

//void DataStructure::run_js(bool) {
//    QString js = mysearch::test_js();
//    if (js.isEmpty()) { return; }
//    ui.widget_web->page()->runJavaScript(js);
//}
//���¼���Ϊ������ص���ʾ��Ϣ������html.
void DataStructure::on_helpTips_triggered() {

    QString html = u8"<h3>��ʼ��</h3><p>ʹ��˵��</p><ol><li>���ѡ���ļ���ť,ѡ�񱾵ص�dblp.xml�ļ�</li>\
        <li>�������ʿ�����ѡ����Ӧģʽ</li>\
        <li>�ȴ���ʼ��,Լ5����,�������˽���ع��ܼ�֪ʶ������������ʾ�������ĵȴ�</li>\
        </ol><h3>��������</h3><p>���ܽ���</p>\
        < ul > <li>������������������ȫ������������ص�������Ϣ��</li>\
        <li>������������������ȫ������������봴�������£��ڽ���б�˫��������ת��������Ϣ������������������</li>\
        <li>ģ������������ؼ��ʣ��������д˹ؼ��ʵ����£��ڽ���б�˫��������ת��������Ϣ������������������</li>\
        <li>�����������������ȫ�������Եõ�������ߣ��ڽ���б�˫��������ת�����ߵ�������Ϣ������������������</li></ul>\
        <p>ʹ�ò���</p>\
        <ol><li>ѡ����Ӧ����������</li>\
        <li>�������������Ӧ����Ϣ</li>\
        <li>�����ť< /li>\
        <li>������Ŀ��������Ҳ࿴��ǰ�ǵڼ���Լ���Ŀ����< /li>\
        <li>����������Ϣ��������Ϣ����ͨ��˫����Ŀ������ת</li></ol>\
        <h3>���ӻ�����</h3><p>ʹ��˵��< /p>\
        <ol><li>��������������ߵ�����< /li>\
        <li>���������ť< /li>\
        <li>�·���ʾ��ǰ���ߵĺ�����ϵͼ, �ڵ�������, �������еĺ�����Ʒ< /li>\
        <li>���������ڱ���, ���ߵ���߻���ʾ���������еĺ�������(֧�����Ź��ܣ��������ּ��ɣ�����Ч�����Ե���ڵ���ٹ���)< /li>< /ol>\
        <h3>���й���< /h3><p>ʹ��˵��< /p>\
        <ul><li>��������<ol>\
        <li>����������еİ�ť< /li>\
        <li>��ʾ��Ӧ��ǰ100�����ߵ�����< /li>< /ol>< /li>\
        <li>����ȴ�<ol>\
        <li>��������������, 1936~2023, ��Ϊֻ����Щ������Ʒ< /li>\
        <li>�������ȴʰ�ť< /li>\
        <li>��ʾ��������ݵ�ǰ10���ȴ�< /li>< /ol>< /li>< /ul>\
        <h3>���ŷ���< /h3><p>����ʾ������Ϣ, ����ҵҪ������ < /p>";
    QMessageBox MyBox(QMessageBox::Information, u8"ʹ����ʾ", "", QMessageBox::Yes);
    MyBox.setText(html);
    MyBox.exec();
}
void DataStructure::on_tagTips_triggered() {
    QString html = u8"<table>\
        < caption align='center'> ��ǩ˵��< /caption>\
        <tr><th>��ǩ< /th><th>����< /th>< /tr>\
        <tr><th>article< / th><th>�ڿ�����־�ϵ�һƪ����< / th>< / tr>\
        <tr><th>book< / th><th>��ȷ����������鼮< / th>< / tr>\
        <tr><th>inclollection< / th><th>һ���������Լ���Ŀ��һ����< / th>< / tr>\
        <tr><th>inproceedings< / th><th>�������ļ��е�һƪ����< / th>< / tr>\
        <tr><th>proceedings< / th><th>�������ļ�< / th>< / tr>\
        <tr><th>mastersthesis< / th><th>˶ʿ����< / th>< / tr>\
        <tr><th>phdthesis< / th><th>��ʿ����< / th>< / tr>\
        <tr><th>title< / th><th>������Ŀ, ��¼��ΨһԪ��.�ñ�ǩ���ܴ��������ӱ�ǩ{ sub,sup,I,tt,ref }< / th>< / tr>\
        <tr><th>sup< / th><th>�ϱ�< / th>< / tr>\
        <tr><th>sub< / th><th>�±�< / th>< / tr>\
        <tr><th>i< / th><th>б��< / th>< / tr>\
        <tr><th>tt< / th><th>�ȿ���ʾ< / th>< / tr>\
        <tr><th>ref< / th><th>����< / th>< / tr>\
        <tr><th>author< / th><th>���ĵ�����, ��ʽ�����ߵ�˳�������Ŀ�ͷ���ߵ�˳��һ��< / th>< / tr>\
        <tr><th>editor< / th><th>�༭��< / th>< / tr>\
        <tr><th>booktitle< / th><th>����������ֻ�ļ��< / th>< / tr>\
        <tr><th>year< / th><th>�������ڣ���ʽΪ4����< / th>< / tr>\
        <tr><th>crossref< / th><th>������һ�����ӹ�ϵ��ͨ��crossref�����ҵ���¼�����ĵ����ļ�< / th>< / tr>\
        <tr><th>journal< / th><th>�ڿ�����< / th>< / tr>\
        <tr><th>school< / th><th>����ѧУ< / th>< / tr>\
        <tr><th>publisher< / th><th>������< / th>< / tr>\
        <tr><th>series< / th><th>������ϵ�вο�< / th>< / tr>\
        <tr><th>ee< / th><th>���Ӱ�����< / th>< / tr>\
        <tr><th>url< / th><th>DBLP��ҳ����< / th>< / tr>\
        <tr><th>page< / th><th>���ĵ�ҳ��< / th>< / tr>\
        <tr><th>volume< / th><th>�����﷢���ص�ԭʼ��< / th>< / tr>\
        <tr><th>number< / th><th>����������Դ�ı��< / th>< / tr>\
        <tr><th>month< / th><th>�����·�< / th>< / tr>\
        <tr><th>cdrom< / th><th>PDF���ӳ�����< / th>< / tr>\
        <tr><th>note< / th><th>�������ļ��е�һƪ���µıʼ�< / th>< / tr>\
        <tr><th>chapter< / th><th>incollection���½�< / th>< / tr>\
        < / table>";
    QMessageBox MyBox(QMessageBox::Information,u8"��ǩ��ʾ", "", QMessageBox::Yes);
    MyBox.setText(html);
    MyBox.exec();
}
void DataStructure::on_page1_knowtag_clicked() {
    QString html = u8"<table>\
        < caption align='center'> ��ǩ˵��< /caption>\
        <tr><th>��ǩ< /th><th>����< /th>< /tr>\
        <tr><th>article< / th><th>�ڿ�����־�ϵ�һƪ����< / th>< / tr>\
        <tr><th>book< / th><th>��ȷ����������鼮< / th>< / tr>\
        <tr><th>inclollection< / th><th>һ���������Լ���Ŀ��һ����< / th>< / tr>\
        <tr><th>inproceedings< / th><th>�������ļ��е�һƪ����< / th>< / tr>\
        <tr><th>proceedings< / th><th>�������ļ�< / th>< / tr>\
        <tr><th>mastersthesis< / th><th>˶ʿ����< / th>< / tr>\
        <tr><th>phdthesis< / th><th>��ʿ����< / th>< / tr>\
        <tr><th>title< / th><th>������Ŀ, ��¼��ΨһԪ��.�ñ�ǩ���ܴ��������ӱ�ǩ{ sub,sup,I,tt,ref }< / th>< / tr>\
        <tr><th>sup< / th><th>�ϱ�< / th>< / tr>\
        <tr><th>sub< / th><th>�±�< / th>< / tr>\
        <tr><th>i< / th><th>б��< / th>< / tr>\
        <tr><th>tt< / th><th>�ȿ���ʾ< / th>< / tr>\
        <tr><th>ref< / th><th>����< / th>< / tr>\
        <tr><th>author< / th><th>���ĵ�����, ��ʽ�����ߵ�˳�������Ŀ�ͷ���ߵ�˳��һ��< / th>< / tr>\
        <tr><th>editor< / th><th>�༭��< / th>< / tr>\
        <tr><th>booktitle< / th><th>����������ֻ�ļ��< / th>< / tr>\
        <tr><th>year< / th><th>�������ڣ���ʽΪ4����< / th>< / tr>\
        <tr><th>crossref< / th><th>������һ�����ӹ�ϵ��ͨ��crossref�����ҵ���¼�����ĵ����ļ�< / th>< / tr>\
        <tr><th>journal< / th><th>�ڿ�����< / th>< / tr>\
        <tr><th>school< / th><th>����ѧУ< / th>< / tr>\
        <tr><th>publisher< / th><th>������< / th>< / tr>\
        <tr><th>series< / th><th>������ϵ�вο�< / th>< / tr>\
        <tr><th>ee< / th><th>���Ӱ�����< / th>< / tr>\
        <tr><th>url< / th><th>DBLP��ҳ����< / th>< / tr>\
        <tr><th>page< / th><th>���ĵ�ҳ��< / th>< / tr>\
        <tr><th>volume< / th><th>�����﷢���ص�ԭʼ��< / th>< / tr>\
        <tr><th>number< / th><th>����������Դ�ı��< / th>< / tr>\
        <tr><th>month< / th><th>�����·�< / th>< / tr>\
        <tr><th>cdrom< / th><th>PDF���ӳ�����< / th>< / tr>\
        <tr><th>note< / th><th>�������ļ��е�һƪ���µıʼ�< / th>< / tr>\
        <tr><th>chapter< / th><th>incollection���½�< / th>< / tr>\
        < / table>";
    ui.textBrowser->setHtml(html);
}
void DataStructure::on_page1_knowdblp_clicked() {
    QString html = u8"<p style='text-indent:2em;'>������Ŀ������ͼ�����Ŀ��Ӣ�Digital Bibliography & Library Project�����DBLP��\
        �ṩ����������ѧ���׵�����������ֻ������Щ���׵����Ԫ���ݣ�����⣬���ߣ��������ڵȡ�\
        �����DBLPֻ�������ݿ�ϵͳ���߼������ط�������£�����DBLP֮ǰҲ������DataBase systems and Logic Programming����д��\
        ���Ÿ���������������������ݵļ��룬��չ�ɽ����DBLP��DBLP��Ŀ�ɵ¹��������ѧ��Michael Ley���𿪷���ά����\
        ����2018���Ѿ��г���3, 600, 000���ס�< / p>\
        <p style = 'text-indent:2em;'>��һ�����е������ͬ��DBLP��û��ʹ�����ݿ����ʹ��XML�洢Ԫ���ݡ�����ÿ�춼��ר���ֹ����룬���и��¡�< / p>\
        <p style = 'text-indent:2em;'>���������Ϣ���������<a href = 'https://dblp.org' target = '_blank'>https://dblp.org/</a></p>";
    ui.textBrowser->setHtml(html);
}
void DataStructure::on_page1_knowapp_clicked() {
    QString html = u8"<h3>��ʼ��</h3><p>ʹ��˵��</p><ol><li>���ѡ���ļ���ť,ѡ�񱾵ص�dblp.xml�ļ�</li>\
        <li>�������ʿ�����ѡ����Ӧģʽ</li>\
        <li>�ȴ���ʼ��,Լ5����,�������˽���ع��ܼ�֪ʶ������������ʾ�������ĵȴ�</li>\
        </ol><h3>��������</h3><p>���ܽ���</p>\
        < ul > <li>������������������ȫ������������ص�������Ϣ��</li>\
        <li>������������������ȫ������������봴�������£��ڽ���б�˫��������ת��������Ϣ������������������</li>\
        <li>ģ������������ؼ��ʣ��������д˹ؼ��ʵ����£��ڽ���б�˫��������ת��������Ϣ������������������</li>\
        <li>�����������������ȫ�������Եõ�������ߣ��ڽ���б�˫��������ת�����ߵ�������Ϣ������������������</li></ul>\
        <p>ʹ�ò���</p>\
        <ol><li>ѡ����Ӧ����������</li>\
        <li>�������������Ӧ����Ϣ</li>\
        <li>�����ť< /li>\
        <li>������Ŀ��������Ҳ࿴��ǰ�ǵڼ���Լ���Ŀ����< /li>\
        <li>����������Ϣ��������Ϣ����ͨ��˫����Ŀ������ת</li></ol>\
        <h3>���ӻ�����</h3><p>ʹ��˵��< /p>\
        <ol><li>��������������ߵ�����< /li>\
        <li>���������ť< /li>\
        <li>�·���ʾ��ǰ���ߵĺ�����ϵͼ, �ڵ�������, �������еĺ�����Ʒ< /li>\
        <li>���������ڱ���, ���ߵ���߻���ʾ���������еĺ�������(֧�����Ź��ܣ��������ּ��ɣ�����Ч�����Ե���ڵ���ٹ���)< /li>< /ol>\
        <h3>���й���< /h3><p>ʹ��˵��< /p>\
        <ul><li>��������<ol>\
        <li>����������еİ�ť< /li>\
        <li>��ʾ��Ӧ��ǰ100�����ߵ�����< /li>< /ol>< /li>\
        <li>����ȴ�<ol>\
        <li>��������������, 1936~2023, ��Ϊֻ����Щ������Ʒ< /li>\
        <li>�������ȴʰ�ť< /li>\
        <li>��ʾ��������ݵ�ǰ10���ȴ�< /li>< /ol>< /li>< /ul>\
        <h3>���ŷ���< /h3><p>����ʾ������Ϣ, ����ҵҪ������ < /p>";
    ui.textBrowser->setHtml(html);

}


void DataStructure::show_clique_message(int use_time) {
    //��ʾ������ص���Ϣ
    QMessageBox::information(this, u8"����", u8"���ŷ�������������ʱ" + QString::number(use_time) + u8"��", u8"�õ�");
    ui.page5_tableWidget->setAlternatingRowColors(1);
    ui.page5_tableWidget->setWordWrap(1);
    QStringList mylist = mysearch::get_clique();
    ui.page5_tableWidget->clear();
    ui.page5_tableWidget->setColumnCount(2);
    ui.page5_tableWidget->setHorizontalHeaderLabels(QStringList() << u8"����" << u8"����");
    ui.page5_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui.page5_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    int rowSz = mylist.size() / 2;
    ui.page5_tableWidget->setRowCount(rowSz);
    QTableWidgetItem* nameItem;
    QTableWidgetItem* cntItem;

    for (int i = 0; i < rowSz; ++i) {
        nameItem = new QTableWidgetItem(mylist.at(i * 2));
        cntItem = new QTableWidgetItem(mylist.at(i * 2 + 1));
        nameItem->setTextAlignment(Qt::AlignCenter);
        cntItem->setTextAlignment(Qt::AlignCenter);
        cntItem->setFlags(Qt::ItemIsEnabled);
        nameItem->setFlags(Qt::ItemIsEnabled);

        ui.page5_tableWidget->setItem(i, 0, nameItem);
        ui.page5_tableWidget->setItem(i, 1, cntItem);
    }
}
DataStructure::~DataStructure()
{}
