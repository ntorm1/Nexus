#pragma once
#include "NexusPch.h"
#include <QMainWindow>
#include <QWidget>
#include <QStringList>
#include <QString>
#include <QFrame>

#include "DockWidget.h"
#include "NexusHelpers.h"
#include "NexusEnv.h"
#include "NexusPlot.h"
#include "Order.h"

namespace Agis {
    class Asset;
}

typedef std::shared_ptr<Agis::Asset> AssetPtr;

namespace Ui {
    class NexusAsset;
}

class NexusAsset;

class NexusAssetPlot : public NexusPlot
{
     Q_OBJECT
         friend class NexusAsset;
public:
    explicit NexusAssetPlot(QWidget* parent);
    ~NexusAssetPlot() = default;

    void load_asset(NexusAsset* asset);

    void add_plot(std::string plot_name);
    void plot_trades(std::vector<SharedTradePtr> const& trades);
    void plot_orders(std::vector<SharedOrderPtr> const& orders);

    /// <summary>
    /// List of columns currently plotted
    /// </summary>
    std::vector<std::string> plotted_graphs;

    /// <summary>
    /// Vector of QCPGraphs that are used to plot the trades
    /// </summary>
    std::vector<QCPGraph*> trade_segments;

protected slots:
    void removeSelectedGraph() override;
    void removeAllGraphs() override;

private slots:
    void contextMenuRequest(QPoint pos) override;
    void new_plot(QString name);

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
    void init_asset_selection();
    void load_asset_data();
    void load_asset_order_data();
    void load_asset_trade_data();

    void set_plotted_graphs(std::vector<std::string> const& graphs);
    std::vector<std::string> get_plotted_graphs() const { return this->nexus_plot->plotted_graphs; }
    std::string get_asset_id() const noexcept;

    Ui::NexusAsset* ui;
    ads::CDockWidget* DockWidget;
    NexusAssetPlot* nexus_plot;
    QComboBox* asset_selection;
    QTabWidget* table_container;
    QTableView* table_view;
    QTableView* orders_table_view;
    QTableView* trades_table_view;
    QTableView* positions_table_view;

    NexusEnv const* nexus_env;
    std::vector<std::string> asset_ids;
    AssetPtr asset;

    std::vector<SharedTradePtr> trades;
    std::vector<SharedOrderPtr> orders;

    std::vector<std::string> column_names;
    std::vector<std::string> dt_index_str;
    std::span<const long long> dt_index;
    std::vector<double> data;
};


//============================================================================
template <typename T>
void event_data_loader(
    std::vector<T> events,
    QStringList const& q_columns,
    QStandardItemModel* model,
    HydraPtr hydra
)
{
    model->setRowCount(events.size());
    model->setColumnCount(q_columns.size());
    model->setHorizontalHeaderLabels(q_columns);

    // Set the data in the model
    int row = 0;
    auto column_names = qlist_to_str_vec(q_columns);
    for (auto& new_event : events) {
        int col = 0;
        for (auto& column_name : column_names) {
            std::expected<rapidjson::Document, AgisException> object_json_expected = new_event->serialize(hydra);
            if (!object_json_expected.has_value()) {
				AGIS_THROW(object_json_expected.error().what());
			}
            rapidjson::Document object_json = std::move(object_json_expected.value());
            const rapidjson::Value& value = object_json[column_name.c_str()];
            std::string str_value;

            // test if column_name is in nexus_datetime_columns
            if (std::find(nexus_datetime_columns.begin(), nexus_datetime_columns.end(), column_name) != nexus_datetime_columns.end()) {
                long long epoch_time = object_json[column_name.c_str()].GetUint64();
                auto res = epoch_to_str(epoch_time, NEXUS_DATETIME_FORMAT);
                if (res.is_exception())
                {
                    AGIS_THROW(res.get_exception());
                }
                str_value = res.unwrap();
            }
            // parse any other value
            else {
                str_value = json_val_to_string(value);
            }
            QStandardItem* item = new QStandardItem(QString::fromStdString(str_value));
            model->setItem(row, col, item);
            col++;
        }
        row++;
    }
}
