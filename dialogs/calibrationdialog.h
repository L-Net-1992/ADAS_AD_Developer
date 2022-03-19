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
#include "nodeparser/inspector.hpp"

namespace Ui {
class CalibrationDialog;
}

class CalibrationDialog : public QDialog
{
    Q_OBJECT

public:
    CalibrationDialog(ProjectDataModel *pdm,QWidget *parent = nullptr);
    explicit CalibrationDialog(QWidget *parent = nullptr);
    ~CalibrationDialog();

    void setProjectDataModel(ProjectDataModel *newProjectDataModel);

private:
    Ui::CalibrationDialog *ui;
    ProjectDataModel *_projectDataModel;
//    QString project_path;
    QMap<QString, float> value_group_;
    Inspector inspector{"127.0.0.1"};

private:
    void init();
    void initButton();
};

#endif // CALIBRATIONDIALOG_H
