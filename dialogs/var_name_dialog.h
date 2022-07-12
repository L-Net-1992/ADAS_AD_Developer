//
// Created by liudian on 2022/3/13.
//

#ifndef NODEDRIVING_VAR_NAME_DIALOG_H
#define NODEDRIVING_VAR_NAME_DIALOG_H

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class VarNameDialog; }
QT_END_NAMESPACE

class VarNameDialog : public QDialog {
Q_OBJECT

public:
    explicit VarNameDialog(QWidget *parent = nullptr);

    ~VarNameDialog() override;

    QString varName() const;
    void setVarName(const QString & var_name);
private:
    Ui::VarNameDialog *ui;
};


#endif //NODEDRIVING_VAR_NAME_DIALOG_H
