#ifndef NODEPARAMETERSMILDIALOG_H
#define NODEPARAMETERSMILDIALOG_H

#include <QDialog>
#include <QFileDialog>

namespace Ui {
class NodeParametersMILDialog;
}

class NodeParametersMILDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NodeParametersMILDialog(QWidget *parent = nullptr);
    ~NodeParametersMILDialog();

private:
    void initToolButton();

private:
    Ui::NodeParametersMILDialog *ui;
};

#endif // NODEPARAMETERSMILDIALOG_H
