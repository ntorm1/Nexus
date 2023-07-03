#pragma once
#include "NexusPch.h"
#include <QMainWindow>
#include <QWidget>
#include <QStringList>
#include <QString>
#include <QFrame>
#include "DockWidget.h"

#include "AgisPointers.h"
#include "NexusEnv.h"
#include "NexusPlot.h"
#include "Asset.h"

namespace Ui {
    class NexusAsset;
}

class NexusAsset;

class NexusAssetPlot : public NexusPlot
{
     Q_OBJECT
public:
    explicit NexusAssetPlot(QWidget* parent);
    ~NexusAssetPlot() = default;

    void load_asset(NexusAsset* asset);

private slots:
    void contextMenuRequest(QPoint pos) override;
    void new_plot(QString name);

private:
    NexusAsset* asset;
};

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

    void load_asset_data();

    std::string get_asset_id() const { return this->asset->read().unwrap()->get_asset_id(); }
    size_t get_column_index(std::string const& column_name);

    Ui::NexusAsset* ui;
    ads::CDockWidget* DockWidget;
    NexusAssetPlot* nexus_plot;
    QTableView* table_view;

    NexusEnv const* nexus_env;
    SharedAssetLockPtr asset;

    std::vector<std::string> column_names;
    std::vector<std::string> dt_index_str;
    StridedPointer<long long> dt_index;
    AgisMatrix <double> data;
};