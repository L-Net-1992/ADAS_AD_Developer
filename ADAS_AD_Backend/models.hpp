#pragma once

#include <QtCore/QObject>

#include <nodes/NodeData>
#include <nodes/NodeDataModel>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <memory>
#include <utility>
#include <vector>
#include <string>
#include "invocable.hpp"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


class InvocableDataModel : public NodeDataModel {
Q_OBJECT
private:
    Invocable _invocable;
    QWidget *_input{nullptr};
    QLabel *_var_name_label{nullptr};

public:
    const Invocable &invocable() const {
        return _invocable;
    }

private Q_SLOTS:

    void paramTextEdited(const QString &text) {
        auto *edit = qobject_cast<QLineEdit *>(sender());
        if (edit) {
            const auto &name = edit->objectName().toStdString();
            if (_invocable.hasParam(name))
                _invocable.setParamValue(name, edit->text().toStdString());
        }

    };
public Q_SLOTS:

public:
    InvocableDataModel(Invocable invocable) : _invocable(std::move(invocable)) {
        _input = new QWidget;
        _input->setAttribute(Qt::WA_NoSystemBackground);
        auto *il = new QVBoxLayout;
        il->setContentsMargins(16, 16, 16, 16);
        il->setSpacing(16);
        _input->setLayout(il);
        _var_name_label = new QLabel("");
        _var_name_label->setMinimumWidth(80);
        _var_name_label->setAlignment(Qt::AlignCenter);
        il->addWidget(_var_name_label);
        if (!_invocable.getParamList().empty()) {
            auto *form = new QWidget;
            form->setAttribute(Qt::WA_NoSystemBackground);
            auto *l = new QFormLayout;
            for (const Param &p: _invocable.getParamList()) {
                const auto &name = QString::fromStdString(p.getName());
                auto *edit = new QLineEdit;
                if (p.isInteger()) {
                    edit->setAlignment(Qt::AlignmentFlag::AlignRight);
                    auto *validator = new QIntValidator(edit);
                    if (p.getType() == "bool") {
                        validator->setBottom(0);
                        validator->setTop(1);
                    }
                    edit->setValidator(validator);
                    edit->setText(QString::fromStdString(p.getValue()));
                } else if (p.isFloat()) {
                    edit->setAlignment(Qt::AlignmentFlag::AlignRight);
                    edit->setValidator(new QDoubleValidator(edit));
                    edit->setText(QString::fromStdString(p.getValue()));
                } else if (p.isString()) {
                    edit->setAlignment(Qt::AlignmentFlag::AlignLeft);
                    edit->setText(QString::fromStdString(p.getValue()));
                } else {
                    edit->setReadOnly(true);
                    edit->setAlignment(Qt::AlignmentFlag::AlignCenter);
                    edit->setStyleSheet("color: #ff0000");
                    edit->setText("invalid type");
                }
                edit->setObjectName(name);
                connect(edit, &QLineEdit::textEdited, this, &InvocableDataModel::paramTextEdited);
                l->addRow(name, edit);
            }
            form->setLayout(l);
            il->addWidget(form);
        }

    }


public:

    QString
    caption() const override {
//        return QString::fromStdString(_invocable.getName());
        return QString::fromStdString(_invocable.caption());
    }

    void setCaption(QString c) {
        _invocable.setCaption(c.toStdString());
    }

    QString
    name() const override {
        return QString::fromStdString(_invocable.getName());
    }

    QString varName() const {
        return QString::fromStdString(_invocable.getVarName());
    }

    void setVarName(const QString &var_name) {
        _invocable.setVarName(var_name.toStdString());
        _var_name_label->setText(var_name);
    }


public:

    QJsonObject
    save() const override {
        QJsonObject modelJson;

        modelJson["name"] = name();
        modelJson["var_name"] = varName();
        if(_invocable.getType() == Invocable::SubsystemIn) {
            QJsonObject subsystemIn;
            subsystemIn["type"] = QString::fromStdString(_invocable.getOutputPort(0).getType());
            modelJson["subsystem_in"] = subsystemIn;

        } else if(_invocable.getType() == Invocable::SubsystemOut) {
            QJsonObject subsystemOut;
            subsystemOut["type"] = QString::fromStdString(_invocable.getInputPort(0).getType());
            modelJson["subsystem_out"] = subsystemOut;

        }

        if (!_invocable.getParamList().empty()) {
            QJsonObject paramsJson;
            for (const auto &p: _invocable.getParamList())
                paramsJson[QString::fromStdString(p.getName())] = QString::fromStdString(p.getValue());
            modelJson["params"] = paramsJson;


        }
        return modelJson;
    }

    void restore(const QJsonObject &object) override {
        QJsonValue v = object["var_name"];
        if(v.isString())
            setVarName(v.toString());
        v = object["params"];
        if (v.isObject()) {
            QJsonObject params = v.toObject();
            for (const QString &qname: params.keys()) {
                std::string name = qname.toStdString();
                QJsonValue pv = params[qname];
                if (_invocable.hasParam(name) && pv.isString()) {
                    _invocable.setParamValue(name, pv.toString().toStdString());
                    auto *edit = _input->findChild<QLineEdit *>(qname);
                    if (edit)
                        edit->setText(pv.toString());
                }
            }
        }
    }

public:

    unsigned int
    nPorts(PortType type) const override {
        switch (type) {

            case PortType::None:
                return 0;
            case PortType::In:
                return _invocable.getNumInput();
            case PortType::Out:
                return _invocable.getNumOutput();
        }
    }

    NodeDataType
    dataType(PortType type, PortIndex index) const override {
        QString port_name;
        QString type_name;
        switch (type) {

            case PortType::None:
                port_name = "<none>";
                break;
            case PortType::In:
                port_name = QString::fromStdString(_invocable.getInputPort(index).getName());
                type_name = QString::fromStdString(_invocable.getInputPort(index).getType());
                break;
            case PortType::Out:
                port_name = QString::fromStdString(_invocable.getOutputPort(index).getName());
                type_name = QString::fromStdString(_invocable.getOutputPort(index).getType());
                break;
        }
        return NodeDataType{type_name, port_name};
    }

    std::shared_ptr<NodeData>
    outData(PortIndex) override {
        return nullptr;
    }

    void
    setInData(std::shared_ptr<NodeData>, int) override {
        //
    }

    QWidget *
    embeddedWidget() override {

        return _input;
    }

};
