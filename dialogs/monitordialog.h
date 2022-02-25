#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QTimer>
#include <map>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

    QCheckBox * getCheckBox(int row, int column);

    void set_table_content(int number);

private:
    Ui::Dialog *ui;
    QTimer *timer1;
    QTimer *timer2 ;
    std::vector<std::string> signal_name_list_;
    std::map<std::string, std::vector<int> > signals_data_int;
    std::map<std::string, std::vector<float> > signals_data_float;

};
#endif // DIALOG_H
