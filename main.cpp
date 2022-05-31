#include "dialogs/mainwindow.h"
#include "splashscreen/splashscreen.h"
#include <QApplication>
//#include <QtPlugin>

//#define SPLASHSCREEN 1

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
    //设置全局字体大小
    QFont font = a.font();
    font.setPointSize(11);
    a.setFont(font);


    //开始界面
    QPixmap pixmap(":/res/AADS3.jpg");
    SplashScreen splash(pixmap);

    splash.show();
    a.processEvents();



    //加载不同应用的皮肤
//    QFile file(":/qss/pagefold.qss");
//    QFile file(":/qss/lightblue.css");
    QFile file(":/qss/lakeblue.css");
//    QFile file(":/qss/blacksoft.css");
//    QFile file(":/qss/flatgray.css");
    file.open(QFile::ReadOnly);
    QString styleSheet = QString::fromLatin1(file.readAll());
    a.setStyleSheet(styleSheet);

    MainWindow w;
    mw = &w;
    qInstallMessageHandler(myMessageOutput);

    //资源加载完毕后显示主界面，关闭开始窗口
    QObject::connect(&w,&MainWindow::scriptParserCompleted,&w,[&w,&splash](std::list<Invocable> parserResult){
        w.show();
        splash.stop_timer();
        splash.finish(&w);
    });

    return a.exec();
}

