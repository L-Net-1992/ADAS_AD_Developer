#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QObject>
#include <QTableWidget>
#include <QSizePolicy>

#include "controllers/aicctoolbutton.hpp"

namespace Ui {
class TestDialog;
}

class TestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TestDialog(QWidget *parent = nullptr);
    ~TestDialog();

private:
    void initEvent();

private:
    Ui::TestDialog *ui;
};

#endif // TESTDIALOG_H
