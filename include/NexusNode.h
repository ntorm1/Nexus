#pragma once

#include <QtCore/QObject>
#include <QMainWindow>

#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <stdexcept>

#include "DockWidget.h"
#include <QtNodes/BasicGraphicsScene>
#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeDelegateModelRegistry>
#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/GraphicsView>

#include "AgisStrategy.h"
#include <memory>

using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDelegateModel;
using QtNodes::PortIndex;
using QtNodes::PortType;
using QtNodes::NodeDelegateModelRegistry;
using QtNodes::DataFlowGraphModel;
using QtNodes::GraphicsView;
using QtNodes::BasicGraphicsScene;

class NexusEnv;

namespace Ui {
    class NexusNodeEditor;
}

std::shared_ptr<NodeDelegateModelRegistry> registerDataModels();

class NexusNodeEditor : public QMainWindow
{
    Q_OBJECT

public:

    NexusNodeEditor(
        NexusEnv const* nexus_env,
        ads::CDockWidget* DockWidget,
        AgisStrategyRef strategy,
        QWidget* parent = nullptr
    );
    ~NexusNodeEditor();

    void __save();
    void __load(BasicGraphicsScene* scene);
    static std::optional<ExchangeViewLambdaStruct>  __extract_abstract_strategy(DataFlowGraphModel* dataFlowGraphModel);

    std::string get_strategy_id() { return this->strategy.get()->get_strategy_id(); }

private:
    static size_t counter;
    size_t id;

    DataFlowGraphModel* dataFlowGraphModel;
    GraphicsView* view;

    NexusEnv const* nexus_env;
    AgisStrategyRef strategy;
    Ui::NexusNodeEditor* ui;
    ads::CDockWidget* DockWidget;

    QLineEdit* allocation;
    QComboBox* trading_window;

    QMenuBar* createSaveRestoreMenu(BasicGraphicsScene* scene);

    void on_tw_change(int index);

};


// Macro to run a function and display QMessageBox if an exception is thrown
#define RUN_WITH_ERROR_DIALOG(function)                                          \
    try {                                                                       \
        function;                                                               \
    }                                                                           \
    catch (const std::exception& e) {                                           \
        QString errorMessage = "Error: " + QString(e.what());                   \
        QMessageBox::critical(nullptr, "Critical Error", errorMessage, QMessageBox::Ok); \
    }                                                                           \
    catch (...) {                                                               \
        QString errorMessage = "Unknown error occurred.";                        \
        QMessageBox::critical(nullptr, "Critical Error", errorMessage, QMessageBox::Ok); \
    }