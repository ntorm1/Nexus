#pragma once
#include "NexusPch.h"
#include <QMainWindow>
#include <QWidget>

#include "DockWidget.h"

#include "AgisPointers.h"
#include "NexusEnv.h"
#include "NexusPlot.h"
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

    void init_plot();

    std::string get_asset_id() const { return this->asset->read().unwrap()->get_asset_id(); }

private:
    Ui::NexusAsset* ui;
    ads::CDockWidget* DockWidget;
    NexusPlot* nexus_plot;

    NexusEnv const* nexus_env;
    SharedAssetLockPtr asset;

    StridedPointer<long long> dt_index;
};