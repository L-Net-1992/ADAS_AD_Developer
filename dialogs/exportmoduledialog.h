#ifndef EXPORTMODULEDIALOG_H
#define EXPORTMODULEDIALOG_H

#include <QDialog>
#include "project/modelsproject.hpp"
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
    explicit ExportModuleDialog(ProjectDataModel *pdm,QWidget *parent = nullptr);
    ~ExportModuleDialog();
private:
    void getSubsystemData();
    void initConnect();

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::ExportModuleDialog *ui;
    ProjectDataModel *pDataModel;
};

#endif // EXPORTMODULEDIALOG_H
