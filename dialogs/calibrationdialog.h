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
    CalibrationDialog(const QString ip,QSharedPointer<ProjectDataModel> pdm,QWidget *parent = nullptr);
    explicit CalibrationDialog(QWidget *parent = nullptr);
    ~CalibrationDialog();

    void setProjectDataModel(QSharedPointer<ProjectDataModel> newProjectDataModel);

private:
    Ui::CalibrationDialog *ui;
    QSharedPointer<ProjectDataModel> _projectDataModel;
//    QString project_path;
    QMap<QString, float> value_group_;
    Inspector inspector;

private:
    void init();
    void initButton();
};

#endif // CALIBRATIONDIALOG_H
