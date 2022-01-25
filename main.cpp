#include "dialogs/mainwindow.h"

#include <QApplication>
//#include <QtPlugin>


int main(int argc, char *argv[])
{
    //此处引入plugins文件夹中的库文件，当前用来支持Sqlite
    QApplication::addLibraryPath("./plugins");
    QApplication a(argc, argv);

    //load app style file
//    QFile file(":/qss/pagefold.qss");
        QFile file(":/qss/lightblue.css");
//            QFile file(":/qss/blacksoft.css");
//            QFile file(":/qss/flatgray.css");
    file.open(QFile::ReadOnly);
    QString styleSheet = QString::fromLatin1(file.readAll());
    a.setStyleSheet(styleSheet);

    MainWindow w;
    w.show();

    return a.exec();
}

