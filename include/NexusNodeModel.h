#pragma once

#include <QtCore/QObject>

#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModel>

#include "NexusNodeWidget.h"
#include "Hydra.h"

#include <memory>


using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDelegateModel;
using QtNodes::PortIndex;
using QtNodes::PortType;


class ExchangeNode;

// Custom exception class to store the error message
class CustomException : public std::exception {
public:
    CustomException(const std::string& message) : errorMessage(message) {}
    const char* what() const noexcept override {
        return errorMessage.c_str();
    }

private:
    std::string errorMessage;
};
// Custom macro to handle exceptions and display error messages
#define NEXUS_UNWRAP_DIALOG(expression)                               \
    {                                                              \
        try {                                                      \
            expression;                                            \
        }                                                          \
        catch (const std::exception& e) {                          \
            QString errorMessage = "Error: " + QString(e.what());  \
            QMessageBox::critical(nullptr, "Critical Error", errorMessage, QMessageBox::Ok); \
            throw CustomException(errorMessage.toStdString());    \
        }                                                          \
    }


/// The class can potentially incapsulate any user data which
/// need to be transferred within the Node Editor graph
class ExchangeData : public NodeData
{
public:
    ExchangeData() = default;
    ExchangeData(ExchangePtr const exchange_) : exchange_ptr(exchange_) {};

    NodeDataType type() const override { return NodeDataType{ "Exchange", "Exchange" }; }
    
    ExchangePtr const exchange_ptr = nullptr;
};


/// Exchange data mdoel
class ExchangeDataModel : public NodeDelegateModel
{
    Q_OBJECT

public:
    ExchangeDataModel() = default;
    virtual ~ExchangeDataModel() {}

public:
    QString caption() const override { return QString("Exchange"); }

    QString name() const override { return QString("Exchange"); }

    QWidget* embeddedWidget() override;

public:
    unsigned int nPorts(PortType const portType) const override
    {
        unsigned int result = 1;

        switch (portType) {
        case PortType::In:
            result = 0;
            break;

        case PortType::Out:
            result = 1;
            break;
        case PortType::None:
            break;
        }

        return result;
    }

    NodeDataType dataType(PortType const portType, PortIndex const portIndex) const override
    {
        switch (portType) {
        case PortType::Out:
            switch (portIndex) {
            case 0:
                return ExchangeData().type();
            }
            break;

        case PortType::None:
            break;
        }
        // FIXME: control may reach end of non-void function [-Wreturn-type]
        return NodeDataType();
    }

    std::shared_ptr<NodeData> outData(PortIndex const port) override
    {
        if (port == 0)
        {
            auto exchange_id = this->exchange_node->exchange_id->currentText().toStdString();
            auto exchange = this->hydra->get_exchanges().get_exchange(exchange_id);
            return std::make_shared<ExchangeData>(exchange);
        }
    }

    void setInData(std::shared_ptr<NodeData>, PortIndex const) override
    {
    }

    void on_exchange_change();

    static std::shared_ptr<Hydra> hydra;


private:

    ExchangeNode* exchange_node = nullptr;

};

