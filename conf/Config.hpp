#ifndef CONFIG_H
#define CONFIG_H

#include <QVariantMap>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

class Config
{
public:
    Config(const QString &fileName):m_fileName(fileName){
    	open(fileName);
    }
    ~Config(){
        sync();
    }

    bool open(const QString &fileName){
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly)){
            return false;
        }
        QByteArray allData = file.readAll();
        file.close();

        QJsonParseError jsonError;
        jsonDoc = QJsonDocument::fromJson(allData,&jsonError);
        if(jsonError.error != QJsonParseError::NoError){
            return false;
        }


        root = jsonDoc.object();
        m_cache=root.toVariantMap();

        return true;
    }
    void sync()
    {
        QJsonObject root = QJsonObject::fromVariantMap(m_cache);
        QJsonDocument jsonDoc(root);
        QByteArray data = jsonDoc.toJson(QJsonDocument::Compact);
        QFile file(m_fileName);
        if(file.open(QIODevice::WriteOnly)){
            file.write(data);
            file.close();
        }
    }

    void write(const QString &key, const QVariant& value){
        m_cache.insert(key,value);
    }

    QString readString(const QString &key, const QString &def=""){
        if(m_cache.contains(key)){
            return m_cache.value(key).toString();
        }
        return def;
    }

    bool readBool(const QString &key, bool def=false){
        if(m_cache.contains(key)){
            return m_cache.value(key).toBool();
        }
        return def;
    }

    int readInt(const QString &key, int def=0){
        if(m_cache.contains(key)){
            return m_cache.value(key).toInt();
        }
        return def;
    }

    QJsonObject readJsonObject(const QString &key){
        if(m_cache.contains(key)){
            return m_cache.value(key).toJsonObject();
        }
    }

    QVariantMap readRootJsonObject(){
        return m_cache;
    }

    QJsonDocument getJsonDocument(){
        return jsonDoc;
    }

    QJsonObject getJsonRoot(){
        return root;
    }
private:
    QString m_fileName;
    QVariantMap m_cache;
    QJsonDocument jsonDoc;
    QJsonObject root;
};

//using json  = nlohmann::json;


class ModernJson {

public:
    ModernJson(const QString &fileName):m_fileName(fileName){
        open(fileName);
    }
    ~ModernJson(){
    }
public:
    bool open(const QString &fileName){
        std::ifstream jfile(fileName.toStdString());
        jfile >> m_cache;
        jfile.close();
        return true;
    }

    json readRootJsonObject(){
        return m_cache;
    }
private:
    QString m_fileName;
    ordered_json m_cache;
};

#endif // CONFIG_H

