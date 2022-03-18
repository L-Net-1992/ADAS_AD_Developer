#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include <QDialog>
#include <QChartView>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileDialog>
#include <QDebug>
#include <QFile>
#include <project/modelsproject.hpp>


namespace Ui {
class CalibrationDialog;
}

class CalibrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalibrationDialog(QWidget *parent = nullptr);
    ~CalibrationDialog();

    void setProjectDataModel(ProjectDataModel *newProjectDataModel);

private:
    Ui::CalibrationDialog *ui;
    ProjectDataModel *_projectDataModel;
//    QString project_path;

private:
    void init();
    void initButton();
};

#endif // CALIBRATIONDIALOG_H
