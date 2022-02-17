#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include <QDialog>
#include <QChartView>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileDialog>
#include <QDebug>


namespace Ui {
class CalibrationDialog;
}

class CalibrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalibrationDialog(QWidget *parent = nullptr,QString pp = nullptr);
    ~CalibrationDialog();

private:
    Ui::CalibrationDialog *ui;
    QString project_path;

private:
    void init();
    void initButton();
};

#endif // CALIBRATIONDIALOG_H
