#include "DataStructure.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DataStructure w;
    w.setWindowTitle("DBLP PARSE");//��������
    w.setWindowIcon(QIcon(":/image/rce/icon.png"));//����ͼ��
    w.show();
    return a.exec();
}
