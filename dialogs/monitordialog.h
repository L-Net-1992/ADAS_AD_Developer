#ifndef MONITORDIALOG_H
#define MONITORDIALOG_H

#include <QDialog>
#include <QtCharts/QValueAxis>
#include <QCheckBox>
#include <QTimer>
#include <QList>
#include <QLineSeries>
#include <QtCharts/QSplineSeries>
#include <string>
#include <map>

using namespace QtCharts;

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

    void init_button();
    void init_chart();
    void init_table();
    void update_table_content(int number);

private:

    Ui::Dialog *ui;
    QValueAxis *axisX_;
    QValueAxis *axisY_;
    QTimer *timer1;
    QTimer *timer2 ;
    QLineSeries *myseries;
    std::vector<QColor> color_group_;
    std::vector<QLineSeries*> series_group_;
    std::vector<std::string> signal_name_list_;
    std::map<std::string, std::vector<int> > signals_data_int_;
    std::map<std::string, std::vector<float> > signals_data_float_;

    unsigned int x_index_ = 0;
    size_t signal_num_ = 0; // total
    bool replay_running_ = 0;
    size_t count_ = 0;
};
#endif // DIALOG_H
