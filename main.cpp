#include "DataStructure.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DataStructure w;
    w.setWindowTitle("DBLP PARSE");//设置名字
    w.setWindowIcon(QIcon(":/image/rce/icon.png"));//设置图标
    w.show();
    return a.exec();
}
