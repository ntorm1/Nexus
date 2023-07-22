#pragma once

#include <QtCore/QObject>

#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModel>

#include "NexusNodeWidget.h"
#include "AgisStrategyHelpers.h"

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
#define NEXUS_UNWRAP_DIALOG(expression, additionalInfo)                       \
    {                                                                         \
        try {                                                                 \
            expression;                                                       \
        }                                                                     \
        catch (const std::exception& e) {                                     \
            QString errorMessage = "Error in " + QString(__FILE__) + " at line " + QString::number(__LINE__) + ": " + QString(e.what()); \
            QString fullErrorMessage = errorMessage + "\n" + QString(additionalInfo); \
            QMessageBox::critical(nullptr, "Critical Error", fullErrorMessage, QMessageBox::Ok); \
            throw CustomException(errorMessage.toStdString(), additionalInfo.toStdString()); \
        }                                                                     \
    }

#define NEXUS_THROW(msg)                            \
    do {                                                \
        std::ostringstream oss;                         \
        oss << "Error in " << __FILE__                  \
            << " at line " << __LINE__ << ": " << msg;  \
        throw std::runtime_error(oss.str());            \
    } while (false)


/// Class to encapsulate exchange so it can be send through nodes
class ExchangeData : public NodeData
{
public:
    ExchangeData() = default;
    ExchangeData(ExchangePtr const exchange_) : exchange_ptr(exchange_) {};

    NodeDataType type() const override { return NodeDataType{ "Exchange", "Exchange" }; }
    
    ExchangePtr const exchange_ptr = nullptr;
};


/// Class to encapsulate asset lambda chain so it can be send through nodes
class AssetLambdaData : public NodeData
{
public:
    AssetLambdaData() = default;
    AssetLambdaData(AgisAssetLambdaChain lambda_chain_) : lambda_chain(lambda_chain_) {};

    NodeDataType type() const override { return NodeDataType{ "Asset Lambda", "Asset Lambda" }; }

    AgisAssetLambdaChain lambda_chain;
};


class ExchangeViewData : public NodeData
{
public:
    ExchangeViewData() = default;
    ExchangeViewData(ExchangeViewLambdaStruct my_struct) { exchange_view_lambda = my_struct; };

    NodeDataType type() const override { return NodeDataType{ "Exchange View", "Exchange View" }; }

    ExchangeViewLambdaStruct exchange_view_lambda;
};

/// Asset Lambda data mdoel
class AssetLambdaModel : public NodeDelegateModel
{
    Q_OBJECT

public:
    AssetLambdaModel() = default;
    virtual ~AssetLambdaModel() {}

public:
    QString caption() const override { return QString("Asset Lambda"); }

    QString name() const override { return QString("Asset Lambda"); }

    QWidget* embeddedWidget() override;

public:
    unsigned int nPorts(PortType const portType) const override
    {
        unsigned int result = 1;

        switch (portType) {
        case PortType::In:
            result = 1;
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
            switch (portIndex)
            {
            case 0:
                return AssetLambdaData().type();
            }
            break;

        case PortType::In:
            switch (portIndex)
            {
            case 0:
                return AssetLambdaData().type();
            }
            break;

        case PortType::None:
            break;
        }
        // FIXME: control may reach end of non-void function [-Wreturn-type]
        return NodeDataType();
    }

    std::shared_ptr<NodeData> outData(PortIndex const port) override;
    void setInData(std::shared_ptr<NodeData> data, PortIndex const port) override;

    QJsonObject save() const override;
    void load(QJsonObject const &p) override;

    void on_lambda_change();

private:

    AssetLambdaNode* asset_lambda_node = nullptr;
    AgisAssetLambdaChain lambda_chain;

};

/// Exchange model
class ExchangeModel : public NodeDelegateModel
{
    Q_OBJECT

public:
    ExchangeModel() = default;
    virtual ~ExchangeModel() {}

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

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

    static std::shared_ptr<Hydra> hydra;


private:

    ExchangeNode* exchange_node = nullptr;

};

/// Exchange view data mdoel
class ExchangeViewModel : public NodeDelegateModel
{
    Q_OBJECT

public:
    ExchangeViewModel() = default;
    virtual ~ExchangeViewModel() {}

public:
    QString caption() const override { return QString("Exchange View"); }

    QString name() const override { return QString("Exchange View"); }

    QWidget* embeddedWidget() override;

public:
    unsigned int nPorts(PortType const portType) const override
    {
        unsigned int result = 1;

        switch (portType) {
        case PortType::In:
            result = 2;
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
            switch (portIndex)
            {
            case 0:
                return ExchangeViewData().type();
            }
            break;

        case PortType::In:
            switch (portIndex)
            {
            case 0:
                return AssetLambdaData().type();
            case 1:
                return ExchangeData().type();
            }
            break;

        case PortType::None:
            break;
        }
        // FIXME: control may reach end of non-void function [-Wreturn-type]
        return NodeDataType();
    }

    std::shared_ptr<NodeData> outData(PortIndex const port) override;

    void setInData(std::shared_ptr<NodeData> data, PortIndex const port) override;

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

    void on_exchange_view_change();


private:
    ExchangePtr exchange;
    AgisAssetLambdaChain lambda_chain;
    ExchangeViewNode* exchange_view_node = nullptr;

};



/// Strategy Allocation Model
class StrategyAllocationModel : public NodeDelegateModel
{
    Q_OBJECT

public:
    StrategyAllocationModel() = default;
    virtual ~StrategyAllocationModel() {}

public:
    QString caption() const override { return QString("Strategy Allocation"); }

    QString name() const override { return QString("Strategy Allocation"); }

    QWidget* embeddedWidget() override;

public:
    unsigned int nPorts(PortType const portType) const override
    {
        unsigned int result = 1;

        switch (portType) {
        case PortType::In:
            result = 1;
            break;

        case PortType::Out:
            result = 0;
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
            break;

        case PortType::In:
            switch (portIndex)
            {
            case 0:
                return ExchangeViewData().type();
            }
            break;

        case PortType::None:
            break;
        }
        // FIXME: control may reach end of non-void function [-Wreturn-type]
        return NodeDataType();
    }

    std::shared_ptr<NodeData> outData(PortIndex const port) override { return nullptr; };
    void setInData(std::shared_ptr<NodeData> data, PortIndex const port);

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

    std::optional<ExchangeViewLambdaStruct> ev_lambda_struct;

private:
    StrategyAllocationNode* strategy_allocation_node = nullptr;
    
};
