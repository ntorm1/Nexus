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
#include "Order.h"

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

    void add_plot(std::string plot_name);

    /// <summary>
    /// List of columns currently plotted
    /// </summary>
    std::vector<std::string> plotted_graphs;

protected slots:
    void removeSelectedGraph() override;
    void removeAllGraphs() override;

private slots:
    void contextMenuRequest(QPoint pos) override;
    void new_plot(QString name);
    void plot_event_overlays();

private:
    NexusAsset* nexus_asset;
};

class NexusAsset : public QMainWindow
{
    Q_OBJECT

public slots:
    void on_new_hydra_run();

public:
    NexusAsset(
        NexusEnv const* nexus_env,
        ads::CDockWidget* DockWidget,
        AssetPtr asset,
        QWidget* parent = nullptr
    );

    void load_asset_data();
    void load_asset_event_data();

    void set_plotted_graphs(std::vector<std::string> const& graphs);
    std::vector<std::string> get_plotted_graphs() const { return this->nexus_plot->plotted_graphs; }
    std::string get_asset_id() const { return this->asset->get_asset_id(); }

    Ui::NexusAsset* ui;
    ads::CDockWidget* DockWidget;
    NexusAssetPlot* nexus_plot;
    QTabWidget* table_container;
    QTableView* table_view;
    QTableView* orders_table_view;
    QTableView* trades_table_view;
    QTableView* positions_table_view;

    NexusEnv const* nexus_env;
    AssetPtr asset;

    std::vector<std::string> column_names;
    std::vector<std::string> dt_index_str;
    StridedPointer<long long> dt_index;
    AgisMatrix <double> data;
};