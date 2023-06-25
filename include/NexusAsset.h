#pragma once
#pragma once

#include <QMainWindow>

#include "DockWidget.h"

#include "NexusEnv.h"

class NexusAsset : public QMainWindow
{
    Q_OBJECT
public:
    NexusAsset(
        NexusEnv const* nexus_env,
        ads::CDockWidget* DockWidget,
        QWidget* parent = nullptr
    );

private:
    ads::CDockWidget* DockWidget;
    NexusEnv const* nexus_env;
};