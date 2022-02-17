#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include <QDialog>
#include <QChartView>
#include <QJsonObject>
#include <QJsonDocument>

namespace Ui {
class CalibrationDialog;
}

class CalibrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalibrationDialog(QWidget *parent = nullptr);
    ~CalibrationDialog();

private:
    Ui::CalibrationDialog *ui;

private:
    void init();
    void initButton();
};

#endif // CALIBRATIONDIALOG_H
