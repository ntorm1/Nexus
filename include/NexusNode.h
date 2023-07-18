#pragma once

#include <QtCore/QObject>
#include <QMainWindow>

#include "DockWidget.h"

#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModel>

#include "NexusEnv.h"
#include <memory>

using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDelegateModel;
using QtNodes::PortIndex;
using QtNodes::PortType;

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
        QWidget* parent = nullptr
    );

private:
    static size_t counter;
    size_t id;

    NexusEnv const* nexus_env;
    Ui::NexusNodeEditor* ui;
    ads::CDockWidget* DockWidget;

};


