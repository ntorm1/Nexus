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

#include "NexusEnv.h"
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



namespace Ui {
    class NexusNodeEditor;
}


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
    bool __extract_abstract_strategy();

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

    QMenuBar* createSaveRestoreMenu(BasicGraphicsScene* scene);

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