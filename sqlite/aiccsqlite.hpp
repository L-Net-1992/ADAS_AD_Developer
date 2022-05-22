#ifndef AICCSQLITE_H
#define AICCSQLITE_H

#include <QSqlDatabase>
#include <QSql>
#include <QSqlQuery>
#include <QSqlError>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>


class AICCSqlite
{
    //    Q_OBJECT
public:
    AICCSqlite(){
        _sqlQuery = initDatabaseConnection();
    }
    ~AICCSqlite(){
        closeConnection();
    }

    ///初始化数据库链接
    QSqlQuery initDatabaseConnection(){
        if(QSqlDatabase::contains("qt_sql_default_connection"))
            _database = QSqlDatabase::database("qt_sql_default_connection");
        else {
            QSqlDatabase sqlDatabase;
            _database = sqlDatabase.addDatabase("QSQLITE");
            _database.setDatabaseName(QApplication::applicationDirPath()+"/sqlite/node.db3");
            if(!_database.open()){
                //            QMessageBox::warning(0,QObject::tr("Database Error"),database.lastError());
                qDebug() <<  _database.lastError();
                return _sqlQuery;
            }
        }
        return QSqlQuery(_database);
    }

    ///sql语句执行
    //    template <typename F>
    QSqlQuery query(const QString sql){
        if(_sqlQuery.exec(sql)){
            return _sqlQuery;
        }else{
            qDebug() << _sqlQuery.lastError();
            return _sqlQuery;
        }
    }


    ///sql语句执行后返回容器结果,赋值过程在函数参数中处理
    QVector<QMap<QString,QVariant>> query1(const QString sql,QMap<QString,QVariant> (*func_ptr)(QSqlQuery) ){
        QVector<QMap<QString,QVariant>> vector;
        try{
            if(_sqlQuery.exec(sql)){
                while(_sqlQuery.next()){
                    vector.append(func_ptr(_sqlQuery));
                }
            }else{
                qDebug() << _sqlQuery.lastError();
            }
        }catch(const std::exception &e){
            qDebug() << "exception:" << e.what();
        }
        return vector;

    }

    ///关闭数据库连接
    void closeConnection(){
        if(_database.isOpen())
            _database.close();
    }

    QSqlQuery &sqlQuery(){return _sqlQuery;};



private:
    QSqlDatabase _database;
    QSqlQuery _sqlQuery;
};

#endif // AICCSQLITE_H
