
#include "NexusAsset.h"
#include "NexusHelpers.h""
#include "ui_NexusAsset.h"
#include "Utils.h"
#include <QThread>

//============================================================================
NexusAsset::NexusAsset(
        NexusEnv const* nexus_env_,
        ads::CDockWidget* DockWidget_,
        AssetPtr asset_,
        QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::NexusAsset),
    nexus_env(nexus_env_),
    DockWidget(DockWidget_)
{
    ui->setupUi(this);
    this->asset = asset_;

    // Retrieve the central widget from the UI
    QWidget* centralWidget = ui->centralwidget;

    // Create a layout for the central widget
    QHBoxLayout* layout = new QHBoxLayout(centralWidget);
    layout->setSpacing(10); // Set spacing between widgets
    layout->setContentsMargins(10, 10, 10, 10); // Set margins around the layout

    // Create a QTableWidget
    this->table_container = new QTabWidget(this);
    this->table_view = new QTableView();
    this->table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->load_asset_data();
    this->table_container->addTab(this->table_view, QString("Data"));

    this->orders_table_view = new QTableView();
    this->orders_table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->table_container->addTab(this->orders_table_view, QString("Orders"));

    this->trades_table_view = new QTableView();
    this->trades_table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->table_container->addTab(this->trades_table_view, QString("Trades"));

    this->positions_table_view = new QTableView();
    this->positions_table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->table_container->addTab(this->positions_table_view, QString("Positions"));

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(this->table_container);

    // Retrieve the NexusPlot widget from the UI
    this->nexus_plot = ui->widget;
    this->nexus_plot->load_asset(this);

    QSplitter* splitter = new QSplitter(Qt::Horizontal, centralWidget);
    layout->addWidget(splitter);

    // Add the NexusPlot widget and table widget to the splitter
    splitter->addWidget(this->nexus_plot);
    splitter->addWidget(scrollArea);

    // Calculate the initial size for the NexusPlot widget (e.g., 65% of the total size)
    int initialNexusPlotWidth = centralWidget->width() * 0.85;

    // Set the sizes for the widgets in the splitter
    splitter->setSizes({ initialNexusPlotWidth, centralWidget->width() - initialNexusPlotWidth });

    this->nexus_plot->plotLayout()->insertRow(0);
    QCPTextElement* title = new QCPTextElement(this->nexus_plot, this->asset->get_asset_id().c_str(), QFont("sans", 17, QFont::Bold));
    this->nexus_plot->plotLayout()->addElement(0, 0, title);

    // Set the layout for the central widget
    centralWidget->setLayout(layout);

    // attempt to load in event data if Hydra has already run
    this->on_new_hydra_run();
}


//============================================================================
void NexusAsset::load_asset_data()
{
    this->dt_index = asset->__get_dt_index(false);
    this->dt_index_str = asset->__get_dt_index_str(false);
    this->data = asset->__get__data();
    this->column_names = asset->get_column_names();
    auto& headers = asset->get_headers();

    // Create the model
    QStandardItemModel* model = new QStandardItemModel(this);

    QStringList q_column_names, q_row_names;
    for (const std::string& str : this->column_names) {
        q_column_names.append(QString::fromStdString(str));
    }
    for (const std::string& str : this->dt_index_str) {
        q_row_names.append(QString::fromStdString(str));
    }

    // Set the number of rows and columns
    int rows = data.rows();
    int columns = data.columns();
    model->setRowCount(rows);
    model->setColumnCount(columns);
    model->setHorizontalHeaderLabels(q_column_names);
    model->setVerticalHeaderLabels(q_row_names);


    // Set the data in the model
    int q_index = 0;
    for (auto& column_name : this->column_names) {
        auto i = headers.at(column_name);
        auto col = this->data.column(i);
        for (int j = 0; j < rows; ++j) {
            double value = col[j];
            QStandardItem* item = new QStandardItem(QString::number(value));
            model->setItem(j, q_index, item);
        }
        q_index++;
    }

    // Set the model in the table view
    this->table_view->setModel(model);

    // Resize the columns to fit the content
    this->table_view->resizeColumnsToContents();
   
}

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
    json object_json;
    auto column_names = qlist_to_str_vec(q_columns);
    for (auto& new_event : events) {
        int col = 0;
        for (auto& column_name : column_names) {
            AGIS_TRY(new_event->serialize(object_json, hydra));
            const json& value = object_json[column_name];
            std::string str_value;

            // test if column_name is in nexus_datetime_columns
            if (std::find(nexus_datetime_columns.begin(), nexus_datetime_columns.end(), column_name) != nexus_datetime_columns.end()) {
                auto& value = object_json[column_name];
                long long epoch_time = value.get<long long>();
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

//============================================================================
void NexusAsset::load_asset_order_data()
{
    std::vector<SharedOrderPtr> const& events = this->nexus_env->get_order_history();
    this->orders = this->nexus_env->filter_event_history<Order>(
        events,
        this->asset->get_asset_id(),
        std::nullopt,
        std::nullopt
    );
    if (this->orders.size() == 0) {
        return;
    }

    QStandardItemModel* model = new QStandardItemModel(this);
    event_data_loader(this->orders, q_order_columns_names, model, this->nexus_env->get_hydra());

    // Set the model in the table view
    this->orders_table_view->reset();
    this->orders_table_view->setModel(model);
    this->orders_table_view->resizeColumnsToContents();
}


//============================================================================
void NexusAsset::load_asset_trade_data()
{
    std::vector<SharedTradePtr> const& all_trades = this->nexus_env->get_trade_history();
    this->trades = this->nexus_env->filter_event_history<Trade>(
        all_trades,
        this->asset->get_asset_id(),
        std::nullopt,
        std::nullopt
    );
    if(this->trades.size() == 0) {
		return;
	}

    QStandardItemModel* model = new QStandardItemModel(this);
    event_data_loader(this->trades, q_trade_column_names, model, this->nexus_env->get_hydra());

    // Set the model in the table view
    this->trades_table_view->reset();
    this->trades_table_view->setModel(model);
    this->trades_table_view->resizeColumnsToContents();
}


//============================================================================
void NexusAsset::set_plotted_graphs(std::vector<std::string> const& graphs)
{
    this->nexus_plot->plotted_graphs.clear();
    // plot each graph 
    for (auto& graph_name : graphs) {
        if (graph_name == "TRADES")
        {
            this->nexus_plot->plot_trades(this->trades);
        }
        else if (graph_name == "ORDERS")
        {
            this->nexus_plot->plot_orders(this->orders);
        }
        else
        {
            this->nexus_plot->add_plot(graph_name);
        }
	}
}


//============================================================================
void NexusAsset::on_new_hydra_run() {
    this->trades.clear();
    this->orders.clear();

    load_asset_order_data();
    load_asset_trade_data();
}

//============================================================================
NexusAssetPlot::NexusAssetPlot(QWidget* parent) : NexusPlot(parent)
{
}


//============================================================================
void NexusAssetPlot::load_asset(NexusAsset* asset_)
{
    this->nexus_asset = asset_;
}


//============================================================================
void NexusAssetPlot::add_plot(std::string plot_name)
{
    this->new_plot(QString::fromStdString(plot_name));
}


//============================================================================
void NexusAssetPlot::plot_orders(std::vector<SharedOrderPtr> const& orders)
{
    if (orders.size() == 0) { return; }

    // get count of orders that had positive units
    int num_buys = std::count_if(orders.begin(), orders.end(), [](SharedOrderPtr const& order) { return order->get_units() > 0; });
	int num_sells = orders.size() - num_buys;
    QVector<QCPGraphData> order_buys(num_buys);
    QVector<QCPGraphData> order_sells(num_sells);

    int num_buys_counter = 0;
    int num_sells_counter = 0;
    for (int i = 0; i < orders.size(); i++) {
        if (orders[i]->get_units() > 0)
        {
            order_buys[num_buys_counter].key = orders[i]->get_fill_time() / static_cast<double>(1000000000);
            order_buys[num_buys_counter].value = orders[i]->get_average_price();
        	num_buys_counter++;
        }
		else
		{
			order_sells[num_sells_counter].key = orders[i]->get_fill_time() / static_cast<double>(1000000000);
			order_sells[num_sells_counter].value = orders[i]->get_average_price();
		    num_sells_counter++;
        }
    }
    this->addGraph();
    this->graph()->setName("Buys");
    this->graph()->data()->set(order_buys, true);

    QPen graphPen;
    graphPen.setColor(QColor(144, 238, 144, 255));
    this->graph()->setPen(graphPen);
    this->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
    this->graph()->setLineStyle(QCPGraph::lsNone);

    this->addGraph();
    this->graph()->setName("Sells");
    this->graph()->data()->set(order_sells, true);
    graphPen.setColor(QColor(255, 182, 193, 255));
    this->graph()->setPen(graphPen);
    this->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
    this->graph()->setLineStyle(QCPGraph::lsNone);

    this->rescaleAxes();
    this->replot();
    this->plotted_graphs.push_back("ORDERS");
}


//============================================================================
void NexusAssetPlot::plot_trades(std::vector<SharedTradePtr> const& trades)
{
    // remove any existing trade segments from the graph
    if (this->trade_segments.size())
    {
        // remove trade segements
        for (auto trade_segment : this->trade_segments) {
            this->removeGraph(trade_segment);
        }
        this->trade_segments.clear();

        // remove the trade entry and exit points, iterate in reverse order
        // to prevent out of bonds access when removing graphs
        for (int i = this->graphCount() - 1; i >= 0; --i)
        {
            QCPGraph* graph = this->graph(i);
            if (graph->name() == "Trade Entires" || graph->name() == "Trade Exits")
            {
                this->removeGraph(graph);
            }
        }
    }

    if (trades.size() == 0) { return; }
    this->trade_segments.clear();

    QVector<QCPGraphData> trade_entries(trades.size());
    QVector<QCPGraphData> trade_exits(trades.size());
    
    QPen trade_up, trade_down;
    trade_up.setColor(QColor(0, 100, 0, 255));
    trade_up.setStyle(Qt::DotLine);
    trade_up.setWidthF(4);

    trade_down.setColor(QColor(139, 0, 0, 255));
    trade_down.setStyle(Qt::DotLine);
    trade_down.setWidthF(4);

    for (int i = 0; i < trades.size(); i++) {
        trade_entries[i].key = trades[i]->trade_open_time / static_cast<double>(1000000000);
        trade_entries[i].value = trades[i]->open_price;

        trade_exits[i].key = trades[i]->trade_close_time / static_cast<double>(1000000000);
        trade_exits[i].value = trades[i]->close_price;

        // connect entry and exit with line colored by the trade's profit
        QVector<QCPGraphData> trade_duration(2);
        trade_duration[0] = trade_entries[i];
        trade_duration[1] = trade_exits[i];

        auto graph = this->addGraph();
        graph->removeFromLegend();
        this->graph()->data()->set(trade_duration, true);
        if (trades[i]->realized_pl > 0)
            this->graph()->setPen(trade_up);
        else this->graph()->setPen(trade_down);
        this->graph()->setName("");
        this->trade_segments.push_back(graph);
	}

    this->addGraph();
    this->graph()->setName("Trade Entries");
    this->graph()->data()->set(trade_entries, true);
    
    QPen graphPen;
    graphPen.setColor(QColor(0, 100, 0, 255)); // Dark green color (RGB values)
    this->graph()->setPen(graphPen);
    this->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle, 10));
    this->graph()->setLineStyle(QCPGraph::lsNone);

    this->addGraph();
    this->graph()->setName("Trade Exits");
    this->graph()->data()->set(trade_entries, true);
    graphPen.setColor(QColor(139, 0, 0, 255));
    this->graph()->setPen(graphPen);
    this->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangleInverted, 10));
    this->graph()->setLineStyle(QCPGraph::lsNone);

    this->rescaleAxes();
    this->replot();
    this->plotted_graphs.push_back("TRADES");
}


//============================================================================
void NexusAssetPlot::removeSelectedGraph()
{
    auto line = selected_line.value_or("TRADES");
    // remove line from list of plotted graphs
    this->plotted_graphs.erase(std::remove(
        this->plotted_graphs.begin(),
        this->plotted_graphs.end(), line),
        this->plotted_graphs.end());

    // make sure selected_line is not nullopt otherwise remove trade segments
    if (!this->selected_line.has_value())
    {
        for (auto trade_segment : this->trade_segments)
        {
            this->removeGraph(trade_segment);
        }
        this->remove_graph_by_name("Trade Exits");
        this->remove_graph_by_name("Trade Entries");
        this->replot();
    }
    else {
        NexusPlot::removeSelectedGraph();
    }
}


//============================================================================
void NexusAssetPlot::removeAllGraphs()
{
    this->plotted_graphs.clear();
    NexusPlot::removeAllGraphs();
}


//============================================================================
void NexusAssetPlot::contextMenuRequest(QPoint pos)
{
    QMenu* menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (this->legend->selectTest(pos, false) >= 0) // context menu on legend requested
    {
        menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignLeft));
        menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignHCenter));
        menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignRight));
        menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignRight));
        menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignLeft));
    }
    else  // general context menu on graphs requested
    {
        QMenu* moveSubMenu = menu->addMenu("Plot");
        for (auto const& column_name : this->nexus_asset->column_names)
        {
            auto q_name = QString::fromStdString(column_name);
            QAction* action = moveSubMenu->addAction(q_name);
            connect(action, &QAction::triggered, this, [this, q_name]() {
                this->new_plot(q_name);
            });
        }

        moveSubMenu->addSeparator(); // Add a separator line
        QAction* action = moveSubMenu->addAction("TRADES");
        connect(action, &QAction::triggered, this, [this]() {
            this->plot_trades(this->nexus_asset->trades);
            });
        action = moveSubMenu->addAction("ORDERS");
        connect(action, &QAction::triggered, this, [this]() {
            this->plot_orders(this->nexus_asset->orders);
            });
        action = moveSubMenu->addAction("BETA");
        connect(action, &QAction::triggered, this, [this]() {
            this->add_plot("BETA");
            });


        if (this->selectedGraphs().size() > 0)
            menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
        if (this->graphCount() > 0)
            menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
    }

    menu->popup(this->mapToGlobal(pos));
}


//============================================================================
void NexusAssetPlot::new_plot(QString name)
{
    auto column_name = name.toStdString();
    this->plotted_graphs.push_back(column_name);

    std::span<double const> y_span;
    if (name == "BETA")
    {
        y_span = nexus_asset->asset->get_beta_column();
    }
    else{
        auto& headers = nexus_asset->asset->get_headers();
        auto column_index = headers.at(column_name);
        y_span = nexus_asset->asset->__get_column(column_index);
	}

    this->plot(
        nexus_asset->dt_index,
        y_span,
        name.toStdString()
    );
}