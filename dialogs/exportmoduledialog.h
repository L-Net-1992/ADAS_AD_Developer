#ifndef EXPORTMODULEDIALOG_H
#define EXPORTMODULEDIALOG_H

#include <QDialog>
#include "model/modelsproject.hpp"
#include <QDir>
#include <QDebug>
#include <QFileDialog>
#include "utils.h"

namespace Ui {
class ExportModuleDialog;
}

class ExportModuleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportModuleDialog(QSharedPointer<ProjectDataModel>pdm,QWidget *parent = nullptr);
    ~ExportModuleDialog();
private:
    void getSubsystemData();
    void initConnect();

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::ExportModuleDialog *ui;
    QSharedPointer<ProjectDataModel>pDataModel;
};

#endif // EXPORTMODULEDIALOG_H
