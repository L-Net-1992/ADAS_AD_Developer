#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qsciapis.h>
#include <QMessageBox>
#include <QFileDialog>

namespace Ui {
class EditorWindow;
}

class EditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditorWindow(QWidget *parent = nullptr);
    ~EditorWindow();

public:
    void openTextFile(const QString pathName);

private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();

private:
    void initEditor(QWidget *parent = nullptr);
    void initAction();
    bool maybeSave();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

private:
    Ui::EditorWindow *ui;
    QsciScintilla *_editor;
    QString curFile;
};

#endif // EDITORWINDOW_H
