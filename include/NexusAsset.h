#pragma once
#include "NexusPch.h"
#include <QMainWindow>
#include <QWidget>

#include "DockWidget.h"

#include "AgisPointers.h"
#include "NexusEnv.h"
#include "Asset.h"

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
        SharedAssetLockPtr asset,
        QWidget* parent = nullptr
    );

    std::string get_asset_id() const { return this->asset->read().unwrap().get_asset_id(); }

private:
    Ui::NexusAsset* ui;
    ads::CDockWidget* DockWidget;
    
    NexusEnv const* nexus_env;

    SharedAssetLockPtr asset;
};