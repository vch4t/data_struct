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
    //初始化判断
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
    //设置侧边栏的按钮和组
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

    //设置提示
    ui.initButton->setToolTip(u8"对数据进行初步处理");
    ui.searchButton->setToolTip(u8"通过输入姓名、文章名字、或者部分名词搜索出相应的文章或者作者");
    ui.seeingButton->setToolTip(u8"输入作者，显示作者与作者间通过什么文章合作过");
    ui.rankButton->setToolTip(u8"年度热词排行、作者文章数量排行");
    ui.cliqueButton->setToolTip(u8"整个图中各阶完全子图的个数");

    ui.page1_line->setPlaceholderText(u8"请选择你的dblp.xml文件...");
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
    //选择文件
    QString dir = QFileDialog::getOpenFileName(this, u8"选择文件","./", "XML Files(*.xml)");
    string path = dir.toStdString();

    if(path.empty()) return;
    //根据相应选择使用不同的初始化模式
    int ret = QMessageBox::question(this, u8"询问", u8"会占用一定CPU及内存8G。低速模式（约5G，参考时间约9分钟），普通模式（约8G，参考时间约5分钟）。", u8"不运行了",u8"运行低速模式", u8"运行普通模式", 2);
    switch (ret) {
    case 0:
        QMessageBox::critical(this, u8"警告", u8"未初始化不可以进行其他功能");
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
    emit inited(use_time);//发送初始化完成信号
    ui.page1_choosefile->setEnabled(1);//设置选择文件按钮的可用性
}
void DataStructure::set_cliqueButton(time_t start, time_t end) {
    //同上
    int use_time = end - start;
    emit cliqueinited(use_time);
    ui.cliqueButton->setEnabled(1);
}
void DataStructure::show_message(int use_time) {
    //提示信息并设置侧边按钮的可用性
    QMessageBox::information(this, u8"结束", u8"数据初始化结束，总用时" + QString::number(use_time) + u8"秒", u8"好的");
    ui.cliqueButton->setEnabled(1);
    ui.rankButton->setEnabled(1);
    ui.seeingButton->setEnabled(1);
    ui.searchButton->setEnabled(1);
}

void DataStructure::set_page(int idx) {
    //侧边栏的点击处理
    ui.stackedWidget->setCurrentIndex(idx);//显示所选页
    switch (idx) {
    case 0:
        break;
    case 1:
        //界面相关的设置
        ui.page2_search->setIcon(QIcon(":/image/rce/search.png"));
        searchGroup.addButton(ui.page2_title_search, 0);
        searchGroup.addButton(ui.page2_author_search, 1);
        searchGroup.addButton(ui.page2_fuzz_search, 2);
        searchGroup.addButton(ui.page2_search_authors, 3);
        ui.page2_title_search->setChecked(1);
        ui.page2_listWidget->setWordWrap(1);
        ui.page2_title_search->setToolTip(u8"根据文章名字，搜索相关信息(文章要求全名，包含末尾的符号)");
        ui.page2_author_search->setToolTip(u8"根据作者名字，搜索其参与的文章");
        ui.page2_fuzz_search->setToolTip(u8"根据文章名字的部分名词，搜索出标题中含有关键词的文章");
        ui.page2_search_authors->setToolTip(u8"根据作者名字，搜索出与其合作过的相关作者");
        break;
    case 2: {
        //找到html文件所在路径，并打开
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
        //设置界面相关内容
        ui.page4_tableWidget->setAlternatingRowColors(1);
        ui.yearInput->setPlaceholderText(u8"请输入1936~2023的年份");

    }
        break;
    case 4: {
        //未有聚类处理结果的文件，进行处理
        if (!cliqueInit) {
            int ret = QMessageBox::information(this, u8"提示", u8"第一次打开需要较长时间进行计算,并占用内存", QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
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
        //读取处理结果文件并显示
        ui.page5_tableWidget->setAlternatingRowColors(1);
        ui.page5_tableWidget->setWordWrap(1);
        QStringList mylist = mysearch::get_clique();
        ui.page5_tableWidget->clear();
        ui.page5_tableWidget->setColumnCount(2);
        ui.page5_tableWidget->setHorizontalHeaderLabels(QStringList() << u8"阶数" << u8"数量");
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
    //根据对应的按钮组选择,用不同函数处理搜索
    int idx = searchGroup.checkedId();
    ui.page2_listWidget->clear();
    switch (idx) {
    case 0: {
        //文章搜索
        string name = ui.inputName->text().toStdString();
        show_titleInfo(name);
    }
        break;
    case 1: {
        //作者搜索
        ui.page2_listWidget->setWordWrap(1);
        string name = ui.inputName->text().toStdString();
        QStringList mylist = mysearch::get_author_titles(name);
        if (mylist.empty()) {
            QListWidgetItem* item = new QListWidgetItem;
            item->setText(u8"没有找到结果");
            item->setFont(QFont("宋体", 10, QFont::Bold));
            item->setFlags(Qt::ItemIsEnabled);
            ui.page2_listWidget->addItem(item);
            return;
        }
        show_authors_tiitles(mylist);
    }
        break;
    case 2: {
        //模糊搜索
        ui.page2_listWidget->setAlternatingRowColors(1);

        string name = ui.inputName->text().toStdString();
        if (!fuzzInit) {
            int ret = QMessageBox::information(this, u8"提示", u8"第一次打开需要数秒钟进行初始化,并占用内存,正常下会有卡顿现象",QMessageBox::No|QMessageBox::Yes,QMessageBox::Yes);
            if (ret == QMessageBox::No) {
                break;
            }
        }
        //设置相关信息
        QStringList mylist = fuzz::fuzz_search(name);
        fuzzInit = 1;
        QListWidgetItem* item = new QListWidgetItem;
        QString buf;
        ui.page2_listWidget->setWordWrap(0);

        ui.pageNum->setText(u8"搜索结果数量: " + QString::number(mylist.size()));
        ui.pageNum->setWordWrap(1);
        //结果为空的处理
        if (mylist.empty()) {
            item->setText(u8"没有找到结果");
            item->setFont(QFont("宋体", 10, QFont::Bold));
            item->setFlags(Qt::ItemIsEnabled);
            ui.page2_listWidget->addItem(item);
            return;
        }
        //正常输出结果
        for (int i = 0; i < mylist.size(); ++i) {
            buf = mylist.at(i);
            item = new QListWidgetItem;
            item->setText(buf);
            item->setFont(QFont("宋体", 10, QFont::Bold));
            item->setFlags(Qt::ItemIsEnabled);
            
            ui.page2_listWidget->addItem(item);
        }
    }
        break;
    case 3: {
        
        ui.page2_listWidget->setWordWrap(1);
        string name = ui.inputName->text().toStdString();
        QStringList mylist = mysearch::get_author_authors(name);
        //空处理
        if (mylist.empty()) {
            QListWidgetItem* item = new QListWidgetItem;
            item->setText(u8"没有找到结果");
            item->setFont(QFont("宋体", 10, QFont::Bold));
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
    //双击跳转事件
    int idx = searchGroup.checkedId();
    //根据不同按钮选择，处理跳转
    if (idx == 1 || idx == 2) {
        //跳转到文章信息
        string name = item->text().toStdString();
        ui.page2_listWidget->clear();
        show_titleInfo(name);
    }
    if (idx == 3) {
        //跳转到作者的文章
        ui.page2_author_search->setChecked(1);

        ui.page2_listWidget->setWordWrap(1);

        string name = item->text().toStdString();
        ui.page2_listWidget->clear();
        QStringList mylist = mysearch::get_author_titles(name);
        show_authors_tiitles(mylist);
    }
}
//显示文章信息
void DataStructure::show_titleInfo(string name) {
    ui.page2_title_search->setChecked(1);
    ui.page2_listWidget->setAlternatingRowColors(0);

    QStringList mylist = mysearch::get_articleInfo(name);
    //空处理
    if (mylist.empty()) {
        QListWidgetItem* item = new QListWidgetItem;
        item->setText(u8"没有找到结果");
        item->setFont(QFont("宋体", 10, QFont::Bold));
        item->setFlags(Qt::ItemIsEnabled);
        ui.page2_listWidget->addItem(item);
        return;
    }
    ui.page2_listWidget->setWordWrap(1);
    //显示设置
    for (int i = 0; i < mylist.size();++i) {
        QListWidgetItem* item = new QListWidgetItem;
        QString buf = mylist.at(i);
        int listIdx = buf.lastIndexOf("<");
        buf = buf.mid(listIdx);
        item->setText(u8"属性：");
        item->setBackgroundColor(Qt::gray);
        item->setFont(QFont("宋体", 10, QFont::Bold));
        item->setFlags(Qt::ItemIsEnabled);
        ui.page2_listWidget->addItem(item);
        item = new QListWidgetItem;
        item->setText(buf);
        item->setFont(QFont("宋体", 10));
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
                item->setFont(QFont("宋体", 10, QFont::Bold));
                item->setFlags(Qt::ItemIsEnabled);
                ui.page2_listWidget->addItem(item);
            }
            item = new QListWidgetItem;
            int l = buf.indexOf(">");
            int r = buf.lastIndexOf("<");
            item->setText(buf.mid(l + 1, r - l - 1));
            item->setFont(QFont("宋体", 10));
            item->setFlags(Qt::ItemIsEnabled);
            ui.page2_listWidget->addItem(item);
        }
    }
}

void DataStructure::show_authors_tiitles(const QStringList &mylist) {
    //显示处理
    ui.page2_listWidget->setAlternatingRowColors(1);

    ui.pageNum->setText(u8"搜索结果数量: " + QString::number(mylist.size()));
    ui.pageNum->setWordWrap(1);
    QListWidgetItem* item = new QListWidgetItem;
    QString buf;
    for (int i = 0; i < mylist.size(); ++i) {
        buf = mylist.at(i);
        item = new QListWidgetItem;
        item->setText( buf);
        item->setFont(QFont("宋体", 10, QFont::Bold));
        item->setFlags(Qt::ItemIsEnabled);
        ui.page2_listWidget->addItem(item);
    }
}
void DataStructure::on_authorRank_clicked() {
    //表格设计，输出表格
    QStringList mylist = mysearch::get_author_rank();
    ui.page4_tableWidget->clear();
    ui.page4_tableWidget->setColumnCount(2);
    ui.page4_tableWidget->setHorizontalHeaderLabels(QStringList() << u8"作者" << u8"数量");
    ui.page4_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    int rowSz = mylist.size() / 2;
    ui.page4_tableWidget->setRowCount(rowSz);
    QTableWidgetItem* nameItem;
    QTableWidgetItem* cntItem;
    //保存时是倒序，所以用一个j来确定添加位置
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
    //同上
    string name = ui.yearInput->text().toStdString();
    if (name.empty()|| stoi(name) > 2023 || stoi(name) < 1936) {
        QMessageBox::critical(this, u8"警告", u8"年份无效或输入的年份没有文章!");
        return;
    }

        QStringList mylist = keyword::rank_keyword(name);
        ui.page4_tableWidget->clear();
        ui.page4_tableWidget->setColumnCount(2);
        ui.page4_tableWidget->setHorizontalHeaderLabels(QStringList() << u8"热词" << u8"数量");
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
        QMessageBox::critical(this, u8"警告", u8"作者名字输入有误!");
        return;
    }
    string name = ui.page3_input->text().toStdString();
    //设置可视化信息
    QString js = mysearch::get_js(name);
    
    //js = mysearch::test_js();
    QString setname = QString("setName(\"%1\")").arg(ui.page3_input->text());
    ui.widget_web->page()->runJavaScript(setname);//设置图片名字

    ui.widget_web->page()->runJavaScript(js);//设置图片
    
}

void DataStructure::onItemClicked(QListWidgetItem* item) {
    //单击显示当前所在项的位置
    int idxSearch = searchGroup.checkedId();
    if (!idxSearch) return;
    int idx=ui.page2_listWidget->row(item);
    ui.pageCurr->setText(u8"当前项: " + QString::number(idx+1));
}

//void DataStructure::run_js(bool) {
//    QString js = mysearch::test_js();
//    if (js.isEmpty()) { return; }
//    ui.widget_web->page()->runJavaScript(js);
//}
//以下几个为设置相关的提示信息，设置html.
void DataStructure::on_helpTips_triggered() {

    QString html = u8"<h3>初始化</h3><p>使用说明</p><ol><li>点击选择文件按钮,选择本地的dblp.xml文件</li>\
        <li>根据提问框内容选择相应模式</li>\
        <li>等待初始化,约5分钟,可以先了解相关功能及知识，结束会有提示，请耐心等待</li>\
        </ol><h3>搜索功能</h3><p>功能介绍</p>\
        < ul > <li>文章搜索（输入文章全名，搜索出相关的文章信息）</li>\
        <li>作者搜索（输入作者全名，搜索其参与创作的文章）在结果列表双击可以跳转到文章信息，类似文章搜索功能</li>\
        <li>模糊搜索（输入关键词，搜索含有此关键词的文章）在结果列表双击可以跳转到文章信息，类似文章搜索功能</li>\
        <li>相关搜索（输入作者全名，可以得到其合作者）在结果列表双击可以跳转到作者的文章信息，类似作者搜索功能</li></ul>\
        <p>使用步骤</p>\
        <ol><li>选择相应的搜索功能</li>\
        <li>在输入框输入相应的信息</li>\
        <li>点击按钮< /li>\
        <li>单击条目后可以在右侧看当前是第几项，以及条目总量< /li>\
        <li>除了文章信息，其他信息可以通过双击条目进行跳转</li></ol>\
        <h3>可视化功能</h3><p>使用说明< /p>\
        <ol><li>在输入框输入作者的名字< /li>\
        <li>点击搜索按钮< /li>\
        <li>下方显示当前作者的合作关系图, 节点是作者, 边是所有的合作作品< /li>\
        <li>将鼠标放置在边上, 或者点击边会显示两作者所有的合作文章(支持缩放功能，滚动滚轮即可，如无效果可以点击节点后再滚动)< /li>< /ol>\
        <h3>排行功能< /h3><p>使用说明< /p>\
        <ul><li>作者排行<ol>\
        <li>点击作者排行的按钮< /li>\
        <li>显示相应的前100个作者的排行< /li>< /ol>< /li>\
        <li>年度热词<ol>\
        <li>在输入框输入年份, 1936~2023, 因为只有这些年有作品< /li>\
        <li>点击年度热词按钮< /li>\
        <li>显示所输入年份的前10个热词< /li>< /ol>< /li>< /ul>\
        <h3>聚团分析< /h3><p>会显示聚团信息, 大作业要求内容 < /p>";
    QMessageBox MyBox(QMessageBox::Information, u8"使用提示", "", QMessageBox::Yes);
    MyBox.setText(html);
    MyBox.exec();
}
void DataStructure::on_tagTips_triggered() {
    QString html = u8"<table>\
        < caption align='center'> 标签说明< /caption>\
        <tr><th>标签< /th><th>含义< /th>< /tr>\
        <tr><th>article< / th><th>期刊或杂志上的一篇文章< / th>< / tr>\
        <tr><th>book< / th><th>有确定出版社的书籍< / th>< / tr>\
        <tr><th>inclollection< / th><th>一本书中有自己题目的一部分< / th>< / tr>\
        <tr><th>inproceedings< / th><th>会议论文集中的一篇文章< / th>< / tr>\
        <tr><th>proceedings< / th><th>会议论文集< / th>< / tr>\
        <tr><th>mastersthesis< / th><th>硕士论文< / th>< / tr>\
        <tr><th>phdthesis< / th><th>博士论文< / th>< / tr>\
        <tr><th>title< / th><th>论文题目, 记录的唯一元素.该标签可能存在以下子标签{ sub,sup,I,tt,ref }< / th>< / tr>\
        <tr><th>sup< / th><th>上标< / th>< / tr>\
        <tr><th>sub< / th><th>下标< / th>< / tr>\
        <tr><th>i< / th><th>斜体< / th>< / tr>\
        <tr><th>tt< / th><th>等宽显示< / th>< / tr>\
        <tr><th>ref< / th><th>引用< / th>< / tr>\
        <tr><th>author< / th><th>论文的作者, 格式中作者的顺序与论文开头作者的顺序一致< / th>< / tr>\
        <tr><th>editor< / th><th>编辑者< / th>< / tr>\
        <tr><th>booktitle< / th><th>会议或者研讨会的简称< / th>< / tr>\
        <tr><th>year< / th><th>发行日期，格式为4数字< / th>< / tr>\
        <tr><th>crossref< / th><th>它代表一种链接关系，通过crossref可以找到收录该论文的论文集< / th>< / tr>\
        <tr><th>journal< / th><th>期刊名称< / th>< / tr>\
        <tr><th>school< / th><th>作者学校< / th>< / tr>\
        <tr><th>publisher< / th><th>出版社< / th>< / tr>\
        <tr><th>series< / th><th>出版物系列参考< / th>< / tr>\
        <tr><th>ee< / th><th>电子版链接< / th>< / tr>\
        <tr><th>url< / th><th>DBLP网页链接< / th>< / tr>\
        <tr><th>page< / th><th>论文的页码< / th>< / tr>\
        <tr><th>volume< / th><th>出版物发布地的原始卷< / th>< / tr>\
        <tr><th>number< / th><th>发布发布的源的编号< / th>< / tr>\
        <tr><th>month< / th><th>发行月份< / th>< / tr>\
        <tr><th>cdrom< / th><th>PDF电子出版物< / th>< / tr>\
        <tr><th>note< / th><th>会议论文集中的一篇文章的笔记< / th>< / tr>\
        <tr><th>chapter< / th><th>incollection的章节< / th>< / tr>\
        < / table>";
    QMessageBox MyBox(QMessageBox::Information,u8"标签提示", "", QMessageBox::Yes);
    MyBox.setText(html);
    MyBox.exec();
}
void DataStructure::on_page1_knowtag_clicked() {
    QString html = u8"<table>\
        < caption align='center'> 标签说明< /caption>\
        <tr><th>标签< /th><th>含义< /th>< /tr>\
        <tr><th>article< / th><th>期刊或杂志上的一篇文章< / th>< / tr>\
        <tr><th>book< / th><th>有确定出版社的书籍< / th>< / tr>\
        <tr><th>inclollection< / th><th>一本书中有自己题目的一部分< / th>< / tr>\
        <tr><th>inproceedings< / th><th>会议论文集中的一篇文章< / th>< / tr>\
        <tr><th>proceedings< / th><th>会议论文集< / th>< / tr>\
        <tr><th>mastersthesis< / th><th>硕士论文< / th>< / tr>\
        <tr><th>phdthesis< / th><th>博士论文< / th>< / tr>\
        <tr><th>title< / th><th>论文题目, 记录的唯一元素.该标签可能存在以下子标签{ sub,sup,I,tt,ref }< / th>< / tr>\
        <tr><th>sup< / th><th>上标< / th>< / tr>\
        <tr><th>sub< / th><th>下标< / th>< / tr>\
        <tr><th>i< / th><th>斜体< / th>< / tr>\
        <tr><th>tt< / th><th>等宽显示< / th>< / tr>\
        <tr><th>ref< / th><th>引用< / th>< / tr>\
        <tr><th>author< / th><th>论文的作者, 格式中作者的顺序与论文开头作者的顺序一致< / th>< / tr>\
        <tr><th>editor< / th><th>编辑者< / th>< / tr>\
        <tr><th>booktitle< / th><th>会议或者研讨会的简称< / th>< / tr>\
        <tr><th>year< / th><th>发行日期，格式为4数字< / th>< / tr>\
        <tr><th>crossref< / th><th>它代表一种链接关系，通过crossref可以找到收录该论文的论文集< / th>< / tr>\
        <tr><th>journal< / th><th>期刊名称< / th>< / tr>\
        <tr><th>school< / th><th>作者学校< / th>< / tr>\
        <tr><th>publisher< / th><th>出版社< / th>< / tr>\
        <tr><th>series< / th><th>出版物系列参考< / th>< / tr>\
        <tr><th>ee< / th><th>电子版链接< / th>< / tr>\
        <tr><th>url< / th><th>DBLP网页链接< / th>< / tr>\
        <tr><th>page< / th><th>论文的页码< / th>< / tr>\
        <tr><th>volume< / th><th>出版物发布地的原始卷< / th>< / tr>\
        <tr><th>number< / th><th>发布发布的源的编号< / th>< / tr>\
        <tr><th>month< / th><th>发行月份< / th>< / tr>\
        <tr><th>cdrom< / th><th>PDF电子出版物< / th>< / tr>\
        <tr><th>note< / th><th>会议论文集中的一篇文章的笔记< / th>< / tr>\
        <tr><th>chapter< / th><th>incollection的章节< / th>< / tr>\
        < / table>";
    ui.textBrowser->setHtml(html);
}
void DataStructure::on_page1_knowdblp_clicked() {
    QString html = u8"<p style='text-indent:2em;'>数字书目索引与图书馆项目（英语：Digital Bibliography & Library Project，简称DBLP）\
        提供计算机领域科学文献的搜索服务，它只储存这些文献的相关元数据，如标题，作者，发表日期等。\
        最早的DBLP只包含数据库系统和逻辑编程相关方面的文章，所以DBLP之前也可以是DataBase systems and Logic Programming的缩写。\
        随着更多的其他计算机领域的内容的加入，发展成今天的DBLP。DBLP项目由德国特里尔大学的Michael Ley负责开发和维护。\
        截至2018年已经有超过3, 600, 000文献。< / p>\
        <p style = 'text-indent:2em;'>和一般流行的情况不同，DBLP并没有使用数据库而是使用XML存储元数据。几乎每天都有专人手工输入，进行更新。< / p>\
        <p style = 'text-indent:2em;'>更多相关信息请浏览官网<a href = 'https://dblp.org' target = '_blank'>https://dblp.org/</a></p>";
    ui.textBrowser->setHtml(html);
}
void DataStructure::on_page1_knowapp_clicked() {
    QString html = u8"<h3>初始化</h3><p>使用说明</p><ol><li>点击选择文件按钮,选择本地的dblp.xml文件</li>\
        <li>根据提问框内容选择相应模式</li>\
        <li>等待初始化,约5分钟,可以先了解相关功能及知识，结束会有提示，请耐心等待</li>\
        </ol><h3>搜索功能</h3><p>功能介绍</p>\
        < ul > <li>文章搜索（输入文章全名，搜索出相关的文章信息）</li>\
        <li>作者搜索（输入作者全名，搜索其参与创作的文章）在结果列表双击可以跳转到文章信息，类似文章搜索功能</li>\
        <li>模糊搜索（输入关键词，搜索含有此关键词的文章）在结果列表双击可以跳转到文章信息，类似文章搜索功能</li>\
        <li>相关搜索（输入作者全名，可以得到其合作者）在结果列表双击可以跳转到作者的文章信息，类似作者搜索功能</li></ul>\
        <p>使用步骤</p>\
        <ol><li>选择相应的搜索功能</li>\
        <li>在输入框输入相应的信息</li>\
        <li>点击按钮< /li>\
        <li>单击条目后可以在右侧看当前是第几项，以及条目总量< /li>\
        <li>除了文章信息，其他信息可以通过双击条目进行跳转</li></ol>\
        <h3>可视化功能</h3><p>使用说明< /p>\
        <ol><li>在输入框输入作者的名字< /li>\
        <li>点击搜索按钮< /li>\
        <li>下方显示当前作者的合作关系图, 节点是作者, 边是所有的合作作品< /li>\
        <li>将鼠标放置在边上, 或者点击边会显示两作者所有的合作文章(支持缩放功能，滚动滚轮即可，如无效果可以点击节点后再滚动)< /li>< /ol>\
        <h3>排行功能< /h3><p>使用说明< /p>\
        <ul><li>作者排行<ol>\
        <li>点击作者排行的按钮< /li>\
        <li>显示相应的前100个作者的排行< /li>< /ol>< /li>\
        <li>年度热词<ol>\
        <li>在输入框输入年份, 1936~2023, 因为只有这些年有作品< /li>\
        <li>点击年度热词按钮< /li>\
        <li>显示所输入年份的前10个热词< /li>< /ol>< /li>< /ul>\
        <h3>聚团分析< /h3><p>会显示聚团信息, 大作业要求内容 < /p>";
    ui.textBrowser->setHtml(html);

}


void DataStructure::show_clique_message(int use_time) {
    //显示聚类相关的信息
    QMessageBox::information(this, u8"结束", u8"聚团分析结束，总用时" + QString::number(use_time) + u8"秒", u8"好的");
    ui.page5_tableWidget->setAlternatingRowColors(1);
    ui.page5_tableWidget->setWordWrap(1);
    QStringList mylist = mysearch::get_clique();
    ui.page5_tableWidget->clear();
    ui.page5_tableWidget->setColumnCount(2);
    ui.page5_tableWidget->setHorizontalHeaderLabels(QStringList() << u8"阶数" << u8"数量");
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
