#include <fstream>
#include "AgisOverloads.h"
#include "NexusAsset.h"
#include "NexusPortfolio.h"
#include "NexusHelpers.h"
#include "ui_NexusPortfolio.h"

#include "Portfolio.h"

//============================================================================
NexusPortfolio::NexusPortfolio(
		NexusEnv const* nexus_env_,
		ads::CDockWidget* DockWidget_,
		std::string portfolio_id_,
		QWidget* parent_):
	QMainWindow(parent_),
	ui(new Ui::NexusPortfolio),
	nexus_env(nexus_env_),
	DockWidget(DockWidget_)
{
	ui->setupUi(this);
	this->portfolio_id = portfolio_id_;

	// Retrieve the central widget from the UI as well as the stats widget
	QWidget* centralWidget = ui->centralwidget;
    
    // Create a layout for the central widget
    QHBoxLayout* layout = new QHBoxLayout(centralWidget);
    layout->setSpacing(20); // Set spacing between widgets
    layout->setContentsMargins(20, 20, 20, 20); // Set margins around the layout

    // Retrieve the NexusPlot widget from the UI
    this->nexus_plot = ui->widget;
    auto hydra = this->nexus_env->get_hydra();
    this->nexus_plot->load(hydra, portfolio_id);

    QSplitter* splitter = new QSplitter(Qt::Horizontal, centralWidget);
    layout->addWidget(splitter);

    this->table_container = new QTabWidget(this);
    this->stats_table_view = new QTableView(this);
    this->stats_table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->table_container->addTab(this->stats_table_view, QString("Results"));
    
    this->portfolio_treeview = new QTreeView(this);
    //QPushButton download_portfolio("Click Me");
    this->table_container->addTab(this->portfolio_treeview, QString("Portfolio"));

    // Add the NexusPlot widget and table widget to the splitter
    splitter->addWidget(this->nexus_plot);
    splitter->addWidget(this->table_container);

    // Calculate the initial size for the NexusPlot widget (e.g., 65% of the total size)
    int initialNexusPlotWidth = centralWidget->width() * 0.85;

    // Set the sizes for the widgets in the splitter
    splitter->setSizes({ initialNexusPlotWidth, centralWidget->width() - initialNexusPlotWidth });

    this->nexus_plot->set_title(portfolio_id.c_str());

    // Set the layout for the central widget
    centralWidget->setLayout(layout);

    // load in the NexusPortfolio to the plot 
    this->set_up_toolbar();
    this->set_up_strategies_menu();
    this->set_up_portfolio_table();
    this->nexus_plot->load_portfolio(this);
}


void NexusPortfolio::set_up_toolbar()
{
    ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->toolBar->addAction(ui->actionSaveState);
    ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->actionSaveState->setIcon(svgIcon("./images/json.png"));
    auto a = ui->actionSaveState;
    connect(a, &QAction::triggered, this, &NexusPortfolio::on_portfolio_download);
}

//============================================================================
std::vector<std::string> NexusPortfolio::get_selected_strategies() const
{
    // loop over strategy checkboxes and get the selected ones
    std::vector<std::string> selected_strategies;
    for (auto& [id, checkbox] : this->strategies_checkboxes)
    {
        if (checkbox->isChecked())
        {
            selected_strategies.push_back(id);
        }
    }
    return selected_strategies;
}

//============================================================================
void NexusPortfolio::set_up_strategies_menu()
{
    // Create a new QAction for the dropdown with checkboxes for the strats
    QAction* settingsAction = new QAction("Strategies", this);
    QMenu* settingsMenu = new QMenu(this);
    settingsAction->setMenu(settingsMenu);

    PortfolioPtr portfolio = this->nexus_env->get_hydra()->get_portfolio(this->portfolio_id);
    auto strategy_ids = portfolio->get_strategy_ids();

    this->strategies_checkboxes.clear();
    auto new_action = new QAction("AGGREGATE", this);
    new_action->setCheckable(true);
    new_action->setChecked(false);
    this->strategies_checkboxes.insert({ "AGGREGATE",new_action });
    settingsMenu->addAction(new_action);

    for (auto& id : strategy_ids)
    {
        new_action = new QAction(QString::fromStdString(id), this);
        new_action->setCheckable(true);
        new_action->setChecked(true);
        this->strategies_checkboxes.insert({ id, new_action });
        settingsMenu->addAction(new_action);
    }
    settingsAction->setIcon(svgIcon("./images/flow.png"));
    ui->toolBar->addAction(settingsAction);
}


//============================================================================
void NexusPortfolio::set_up_portfolio_table()
{
    auto portfolio_model = new QStandardItemModel(this);

    // Set the headers for the columns (optional)
    portfolio_model->setHorizontalHeaderLabels(q_trade_column_names);
    auto portfolio = this->nexus_env->get_hydra()->get_portfolio(this->portfolio_id);
    auto& positions = portfolio->__get_positions();

    for (auto& [id, postion] : positions) {
        // load in the trade data for the positions
        auto& trades = postion->__get_trades();
        std::vector<SharedTradePtr> trade_ptrs;
        for (auto& [id, trade] : trades) trade_ptrs.push_back(trade);

        // parse the trades in to a standard item model
        auto position_model = new QStandardItemModel(this);
        event_data_loader(trade_ptrs, q_trade_column_names, position_model, this->nexus_env->get_hydra());
        
        // add a new run for the position
        auto position_id = this->nexus_env->get_hydra()->asset_index_to_id(id).unwrap();
        QStandardItem* position_item = new QStandardItem(QString::fromStdString(position_id));

        // Iterate through the rows in position_model and add them to portfolio_model
        for (int row = 0; row < position_model->rowCount(); ++row) {
            QList<QStandardItem*> items;
            for (int col = 0; col < position_model->columnCount(); ++col) {
                auto item = position_model->takeItem(row, col);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                items.append(item);
            }
            position_item->appendRow(items);
        }
        portfolio_model->appendRow(position_item);
    }
    this->portfolio_treeview->reset();
    this->portfolio_treeview->setModel(portfolio_model);
    this->portfolio_treeview->expandAll();
    for (int column = 0; column < portfolio_model->columnCount(); ++column) {
        this->portfolio_treeview->resizeColumnToContents(column);
    }
    // Show the treeView
    this->show();
}


//============================================================================
void populate_stats_model(
    std::vector<double> const& nlv, 
    QStandardItemModel* model, 
    size_t i,
    std::optional<std::vector<double>> benchmark_nlv = std::nullopt
)
{
    // calculate total returns 
    QString x = "$" + QString::number(get_stats_total_pl(nlv), 'f', 2);
    QStandardItem* item = new QStandardItem(x);
    model->setItem(0, i, item);

    // calculate pct returns
    x = QString::number(get_stats_pct_returns(nlv)) + "%";
    item = new QStandardItem(x);
    model->setItem(1, i, item);

    // calculate annualized returns
    x = QString::number(get_stats_annualized_pct_returns(nlv), 'f', 2) + "%";
    item = new QStandardItem(x);
    model->setItem(2, i, item);

    // calculate annualized volatility
    x = QString::number(get_stats_annualized_volatility(nlv), 'f', 2) + "%";
    item = new QStandardItem(x);
    model->setItem(3, i, item);

    // calculate annualized sharpe
    x = QString::number(get_stats_sharpe_ratio(nlv), 'f', 2);
    item = new QStandardItem(x);
    model->setItem(4, i, item);

    // calculate beta
    if(benchmark_nlv.has_value()){
        //auto beta = get_stats_beta(nlv, benchmark_nlv.value()).unwrap_or(AGIS_NAN);
        x = QString::number(0.0f, 'f', 2);
    }
	else{
        x = QString::number(AGIS_NAN, 'f', 2);
    }
    item = new QStandardItem(x);
    model->setItem(5, i, item);
}


//============================================================================
void NexusPortfolio::on_portfolio_download()
{
    auto hydra = this->nexus_env->get_hydra();
    auto portfolio = hydra->get_portfolio(this->portfolio_id);
    auto& positions = portfolio->__get_positions();

    // Create a RapidJSON Document
    rapidjson::Document j;
    j.SetObject();
    rapidjson::Document::AllocatorType& allocator = j.GetAllocator();

    for (auto& [asset_index, position] : positions)
    {
        auto& trades = position->__get_trades();

        // Create an array for each position
        rapidjson::Value position_j(rapidjson::kArrayType);

        for (auto& [strategy_index, trade] : trades)
        {
            // Create an object for each trade
            auto trade_json = trade->serialize(this->nexus_env->get_hydra());

            // Add the trade object to the position array
            if (!trade_json.has_value()) {
				AGIS_THROW(trade_json.error().what());
			}
            position_j.PushBack(trade_json.value(), allocator);
        }

        auto asset_id = hydra->asset_index_to_id(asset_index).unwrap();

        // Add the position array to the main JSON object
        j.AddMember(rapidjson::StringRef(asset_id.c_str()), position_j, allocator);
    }

    auto ext = this->portfolio_id + ".json";
    auto json_path = this->nexus_env->get_env_path() / ext;

    // Serialize the RapidJSON Document to a file
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    j.Accept(writer);

    std::ofstream file(json_path.string());
    file << buffer.GetString();
}


//============================================================================
void NexusPortfolio::on_new_hydra_run()
{
    // set up the table view
    QStandardItemModel* model = new QStandardItemModel(this);
    static QStringList q_columns = { "Total P/L", "Pct Return", 
        "Annualized Return", "Annualized Volatility", "Sharpe Ratio","Beta"};

    auto selected_strategies = this->get_selected_strategies();
    model->setRowCount(q_columns.size());
    model->setColumnCount(selected_strategies.size());
    model->setVerticalHeaderLabels(q_columns);
    model->setHorizontalHeaderLabels(str_vec_to_qlist(selected_strategies));

    // get the benchmark nlv history if it exists
    std::optional<std::vector<double>> benchmark_nlv = std::nullopt;
    auto portfolio = this->nexus_env->get_hydra()->get_portfolio(this->portfolio_id);
    auto benchmark = portfolio->__get_benchmark_strategy();
    if(benchmark) benchmark_nlv = benchmark->get_nlv_history();

    size_t i = 0;
    for (const auto& id : selected_strategies)
    {
        // stats for the overall portfolio
        if (id == "AGGREGATE") {
            auto& nlv = this->nexus_env->get_hydra()->get_portfolio(this->portfolio_id)->get_nlv_history_vec();
            populate_stats_model(nlv, model, i, benchmark_nlv);
        }
        // check if bench mark strategy by looking for a space in the id (only allowed for benchmark
        else if (id.find(" ") != std::string::npos) {
            auto portfolio = this->nexus_env->get_hydra()->get_portfolio(this->portfolio_id);
            auto nlv = portfolio->__get_benchmark_strategy()->get_nlv_history();
            populate_stats_model(nlv, model, i, benchmark_nlv);
		}
        // stats for a specific strategy
		else {
            // TODO listen for strategy delete event and remove from selected_strategies
            if (!this->nexus_env->get_hydra()->strategy_exists(id)) {
                // remove from selected strategies vector 
                selected_strategies.erase(std::remove(
					selected_strategies.begin(),
					selected_strategies.end(), id),
                    					selected_strategies.end());
                this->set_up_strategies_menu();
                continue;
            }
			auto nlv = this->nexus_env->get_hydra()->get_strategy(id)->get_nlv_history();
            populate_stats_model(nlv, model, i, benchmark_nlv);
        }
        i++;
    }

    // get the absolute and pct returns for the portfolio
    auto hydra = this->nexus_env->get_hydra();
    auto& nlv = hydra->get_portfolio(this->portfolio_id)->get_nlv_history_vec();
    auto dt_index = hydra->__get_dt_index();

    // reload stats table
    this->stats_table_view->reset();
    this->stats_table_view->setModel(model);
    this->stats_table_view->resizeColumnsToContents();

    // remove any plotted graphs from previous run 
    this->nexus_plot->clearGraphs();

    // replot the portfolio table
    this->set_up_portfolio_table();
}


//============================================================================
NexusPortfolioPlot::NexusPortfolioPlot(QWidget* parent_)
{
}


//============================================================================
void NexusPortfolioPlot::removeAllGraphs()
{
    this->plotted_graphs.clear();
    NexusPlot::removeAllGraphs();
}


//============================================================================
void NexusPortfolioPlot::removeSelectedGraph()
{
    // remove line from list of plotted graphs
    this->plotted_graphs.erase(std::remove(
        this->plotted_graphs.begin(),
        this->plotted_graphs.end(), selected_line.value()),
        this->plotted_graphs.end());

    NexusPlot::removeSelectedGraph();
}


//============================================================================
void NexusPortfolioPlot::contextMenuRequest(QPoint pos)
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

        std::vector<std::string> menu_cols = { 
            "CASH", "NET BETA DOLLARS / NLV", "NET BETA DOLLARS","NET LEVERAGE",
            "NLV","UNDERWATER", "FORWARD VOLATILIY", "REALIZED VOLATILITY"
        };
        for (auto& col : menu_cols)
		{
			QAction* action = moveSubMenu->addAction(QString::fromStdString(col));
			connect(action, &QAction::triggered, this, [this, col]() {
				this->add_plot(QString::fromStdString(col));
				});
		}

        if (this->selectedGraphs().size() > 0)
            menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
        if (this->graphCount() > 0)
            menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
    }

    menu->popup(this->mapToGlobal(pos));
}


//============================================================================
std::vector<double> NexusPortfolioPlot::get_data(
    const std::variant<AgisStrategy *, PortfolioPtr>& entity,
    const std::string& name)
{
    if (name == "CASH") {
        if (std::holds_alternative<AgisStrategy *>(entity)) {
            return std::get<AgisStrategy *>(entity)->get_cash_history();
        }
        else {
            return std::get<PortfolioPtr>(entity)->get_cash_history();
        }
    }
    else if (name == "NLV") {
        if (std::holds_alternative<AgisStrategy *>(entity)) {
            return std::get<AgisStrategy *>(entity)->get_nlv_history();
        }
        else {
            return std::get<PortfolioPtr>(entity)->get_nlv_history();
        }
    }
    else if (name == "NET BETA DOLLARS / NLV") {
        if (std::holds_alternative<AgisStrategy *>(entity)) {
            AgisStrategy * entity_ptr = std::get<AgisStrategy *>(entity);
            if(!entity_ptr->__is_beta_trace()) return std::vector<double>();
            return entity_ptr->get_beta_history() / entity_ptr->get_nlv_history();
        }
        else {
            PortfolioPtr entity_ptr = std::get<PortfolioPtr>(entity);
            if (!entity_ptr->__is_beta_trace()) return std::vector<double>();
            return entity_ptr->get_beta_history() / entity_ptr->get_nlv_history();
        }
    }
    else if (name == "NET BETA DOLLARS") {
        if (std::holds_alternative<AgisStrategy *>(entity)) {
            return std::get<AgisStrategy *>(entity)->get_beta_history();
        }
        else {
            return std::get<PortfolioPtr>(entity)->get_beta_history();
        }
    }
    else if (name == "NET LEVERAGE") {
        if (std::holds_alternative<AgisStrategy *>(entity)) {
            return std::get<AgisStrategy *>(entity)->get_net_leverage_ratio_history();
        }
        else {
            //return std::get<PortfolioPtr>(entity).get()->get_ne();
        }
    }
    else if (name == "UNDERWATER") {
        std::vector<double> y_span;
        if (std::holds_alternative<AgisStrategy *>(entity)) {
            y_span = std::get<AgisStrategy *>(entity)->get_nlv_history();
        }
        else {
            y_span = std::get<PortfolioPtr>(entity)->get_nlv_history();
        }
        return get_stats_underwater_plot(y_span);

    }
    else if (name == "FORWARD VOLATILIY") {
        if (std::holds_alternative<AgisStrategy*>(entity)) {
			return std::get<AgisStrategy *>(entity)->get_portfolio_vol_vec();
		}
        else {
			return std::vector<double>();
		}
	}
	else if (name == "REALIZED VOLATILITY") {
        std::vector<double> y_span;
        if (std::holds_alternative<AgisStrategy*>(entity)) {
            y_span = std::get<AgisStrategy*>(entity)->get_nlv_history();
        }
        else {
            y_span = std::get<PortfolioPtr>(entity)->get_nlv_history();
        }
        return rolling_volatility(y_span, 252);
	}
    // Return an empty span if the name doesn't match any condition
    return std::vector<double>();
}


//============================================================================
void NexusPortfolioPlot::add_plot(QString const& name)
{
    this->plotted_graphs.push_back(name.toStdString());
    auto& portfolio = this->hydra->get_portfolio(this->portfolio_id);
    auto x = this->hydra->__get_dt_index();

    // get a vector of strategy ids that are currently selected by the portfolio widget
    auto selected_strategies = this->nexus_portfolio->get_selected_strategies();
    for (auto& strategy_id : selected_strategies)
    {
        std::variant<AgisStrategy*, PortfolioPtr> entity = nullptr;
        if (this->hydra->strategy_exists(strategy_id)) {
            entity = this->hydra->__get_strategy(strategy_id);
        }
        else {
            entity = portfolio;
        }

        // extract the data column from the entity
        auto y = get_data(entity, name.toStdString());

        if (x.size() != y.size())
        {
            QMessageBox::critical(nullptr, "Error", 
                "Failed find " + name + " history for " + QString::fromStdString(strategy_id)
            );
            return;
        }

        this->plot(
            x,
            y,
            strategy_id + " " + name.toStdString()
        );
        this->plotted_graphs.push_back(strategy_id + " " + name.toStdString());
    }
}
