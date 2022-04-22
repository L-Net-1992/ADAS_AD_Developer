//
// Created by liudian on 2022/3/8.
//

#ifndef NODEDRIVING_NEW_SUBSYSTEM_DIALOG_H
#define NODEDRIVING_NEW_SUBSYSTEM_DIALOG_H

#include <QDialog>
#include "ui_new_subsystem_dialog.h"
#include <optional>
#include <utility>
#include <string>
QT_BEGIN_NAMESPACE
namespace Ui { class NewSubsystemDialog; }
QT_END_NAMESPACE

class NewSubsystemDialog : public QDialog {
Q_OBJECT
public:
    using SubsystemNameType = std::pair<std::string, std::string>;
public:
    explicit NewSubsystemDialog(QWidget *parent = nullptr);

    ~NewSubsystemDialog() override;

    SubsystemNameType getSubsystemName();


private:
    Ui::NewSubsystemDialog *ui;
};


#endif //NODEDRIVING_NEW_SUBSYSTEM_DIALOG_H
