#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QCheckBox>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

    QCheckBox * getCheckBox(int row, int column);

private:
    Ui::Dialog *ui;
};
#endif // DIALOG_H
