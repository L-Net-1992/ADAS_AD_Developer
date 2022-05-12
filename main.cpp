#include "dialogs/mainwindow.h"
#include "splashscreen/splashscreen.h"
#include <QApplication>
//#include <QtPlugin>

#define SPLASHSCREEN 1

MainWindow *mw = Q_NULLPTR;
void myMessageOutput(QtMsgType type,const QMessageLogContext &context,const QString &msg){
    QString message;
    switch(type){
    case QtDebugMsg:
        message = QString("Debug:");
        break;
    case QtInfoMsg:
        message = QString("Info:");
        break;
    case QtWarningMsg:
        message = QString("Warning:");
        break;
    case QtCriticalMsg:
        message = QString("Critical:");
        break;
    case QtFatalMsg:
        message = QString("Fatal:");
        break;
    }


    message.append(msg);
    mw->showMsg(message);

}

int main(int argc, char *argv[])
{

    //此处引入plugins文件夹中的库文件，当前用来支持Sqlite
    QApplication::addLibraryPath("./plugins");
    QApplication a(argc, argv);

    //
#if SPLASHSCREEN
    QPixmap pixmap(":/res/AADS4 .jpg");
    SplashScreen splash(pixmap, 10000);
#endif
    //加载不同应用的皮肤
//    QFile file(":/qss/pagefold.qss");
//        QFile file(":/qss/lightblue.css");
        QFile file(":/qss/lakeblue.css");
//            QFile file(":/qss/blacksoft.css");
//            QFile file(":/qss/flatgray.css");
    file.open(QFile::ReadOnly);
    QString styleSheet = QString::fromLatin1(file.readAll());
    a.setStyleSheet(styleSheet);

    MainWindow w;
    mw = &w;
    qInstallMessageHandler(myMessageOutput);

    w.show();

#if SPLASHSCREEN
    splash.finish(&w);
#endif

    return a.exec();
}

