#pragma once

#include <QMainWindow>
#include <QWidget>

#include "DockWidget.h"

#include "NexusEnv.h"


namespace Ui {
    class NexusAsset;
}


class NexusAsset : public QMainWindow
{
    Q_OBJECT
public:
    NexusAsset(
        NexusEnv const* nexus_env,
        ads::CDockWidget* DockWidget,
        std::string asset_id,
        QWidget* parent = nullptr
    );

    std::string get_asset_id() const { return this->asset_id; }

private:
    Ui::NexusAsset* ui;
    ads::CDockWidget* DockWidget;
    
    NexusEnv const* nexus_env;

    std::string asset_id;
};