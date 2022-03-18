#ifndef EXPORTMODULEDIALOG_H
#define EXPORTMODULEDIALOG_H

#include <QDialog>
#include "project/modelsproject.hpp"
#include <QDir>
#include <QDebug>

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
    void initSubsystemData();

private:
    Ui::ExportModuleDialog *ui;
    ProjectDataModel *pDataModel;
};

#endif // EXPORTMODULEDIALOG_H
