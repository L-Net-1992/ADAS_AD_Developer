#include "editordialog.h"
#include "ui_editordialog.h"
//#include

EditorDialog::EditorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditorDialog)
{
    ui->setupUi(this);

    //定义QsciScintilla编辑器
    _editor = new QsciScintilla(parent);

    //设置语法
    QsciLexerCPP *textLexer = new QsciLexerCPP(parent);
    _editor->setLexer(textLexer);
    textLexer->setPaper(QColor(250,250,250));                                       //底色
    textLexer->setColor(QColor(0,170,0),QsciLexerCPP::Comment);         //注释行色

    //代码提示
    QsciAPIs *apis = new QsciAPIs(textLexer);
    apis->add(QString("class"));
    apis->add(QString("api"));
    apis->add(QString("main"));
    apis->prepare();

    //行号提示
    _editor->SendScintilla(QsciScintilla::SCI_SETCODEPAGE,QsciScintilla::SC_CP_UTF8);
    QFont line_font;
    line_font.setFamily("SimSun");
    line_font.setPointSize(11);
    _editor->setFont(line_font);
    _editor->setWrapMode(QsciScintilla::WrapWord);
    _editor->setEolMode(QsciScintilla::EolWindows);
    _editor->setTabWidth(4);
    _editor->setAutoIndent(true);
    _editor->setBackspaceUnindents(true);
    _editor->setTabIndents(true);
    _editor->setBraceMatching(QsciScintilla::SloppyBraceMatch);
    //当前行
    _editor->setCaretWidth(2);
    _editor->setCaretForegroundColor(QColor("darkCyan"));            //光标颜色
    _editor->setCaretLineVisible(true);                                                  //高亮显示光标所在行
    _editor->setCaretLineBackgroundColor(QColor(255,255,255));   //光标所在行背景色
    //选中颜色
    _editor->setSelectionBackgroundColor(Qt::black);                     //选中文本背景色
    _editor->setSelectionForegroundColor(Qt::white);                      //选中文本前景色

    //页边设置
    QFont margin_font;
    margin_font.setFamily("SimSun");
    margin_font.setPointSize(11);
    _editor->setMarginsFont(margin_font);
    _editor->setMarginType(0,QsciScintilla::NumberMargin);       //设置标号为0的页边显示行号
    _editor->setMarginLineNumbers(0,true);                                 //设置第0个边栏为行号边栏，True表示显示
    _editor->setMarginWidth(0,15);                                                 //设置0边栏宽度
    _editor->setMarginsBackgroundColor(Qt::gray);
    _editor->setMarginsForegroundColor(Qt::white);
    _editor->setFolding(QsciScintilla::BoxedTreeFoldStyle);          //折叠样式
    _editor->setFoldMarginColors(Qt::gray,Qt::lightGray);            //折叠栏颜色

    _editor->setAutoCompletionSource(QsciScintilla::AcsAll);     //对于所有Ascii字符自动补全
    _editor->setAutoCompletionThreshold(1);                                //每输入一个字符就会出现自动补全的提示

    ui->verticalLayout->addWidget(_editor);

//    setCentralwidget(editor);

}

void EditorDialog::openTextFile(const QString pathName){
    QFile file(pathName);
    if(file.open(QIODevice::ReadOnly|QIODevice::Text)){
        _editor->clear();
        _editor->append(file.readAll());
    }
    else
        qDebug() <<"The file open failed!";
}

EditorDialog::~EditorDialog()
{
    delete ui;
}
