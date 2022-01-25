#ifndef EDITORDIALOG_H
#define EDITORDIALOG_H

#include <QDialog>
#include <QDebug>

namespace Ui {
class EditorDialog;
}

class EditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditorDialog(QWidget *parent = nullptr);
    ~EditorDialog();

public:
    void openTextFile(const QString pathName);

private:
    Ui::EditorDialog *ui;
};

#endif // EDITORDIALOG_H
