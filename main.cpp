#include "dialogs/mainwindow.h"

#include <QApplication>
//#include <QtPlugin>

int main(int argc, char *argv[])
{
    //此处引入plugins文件夹中的库文件，当前用来支持Sqlite
    QApplication::addLibraryPath("./plugins");
    QApplication a(argc, argv);
    QFont font;
    font.setPointSize(9);
    font.setFamily("黑体");
    a.setFont(font);

    MainWindow w;
    w.show();
    return a.exec();
}

