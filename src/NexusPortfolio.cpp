#include "AgisOverloads.h"
#include "NexusPortfolio.h"
#include "ui_NexusPortfolio.h"


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
    this->menu_bar = new QMenuBar(this);
	this->portfolio_id = portfolio_id_;

	// Retrieve the central widget from the UI as well as the stats widget
	QWidget* centralWidget = ui->centralwidget;
    auto stats_layout = ui->stats;
    this->stats_widget = new QWidget(this);
    
    // setup the stats widget
    stats_widget->setLayout(stats_layout);

    // Create a layout for the central widget
    QHBoxLayout* layout = new QHBoxLayout(centralWidget);
    layout->setSpacing(10); // Set spacing between widgets
    layout->setContentsMargins(10, 10, 10, 10); // Set margins around the layout

    // Create a QTableWidget
    //this->table_view = new QTableView(this);
    //this->table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //QScrollArea* scrollArea = new QScrollArea(this);
    //scrollArea->setWidgetResizable(true);
    //scrollArea->setWidget(this->table_view);
    //this->load_asset_data();

    // Retrieve the NexusPlot widget from the UI
    this->nexus_plot = ui->widget;
    auto hydra = this->nexus_env->get_hydra();
    this->nexus_plot->load(hydra, portfolio_id);

    QSplitter* splitter = new QSplitter(Qt::Horizontal, centralWidget);
    layout->addWidget(splitter);

    // Add the NexusPlot widget and table widget to the splitter
    splitter->addWidget(this->nexus_plot);
    splitter->addWidget(stats_widget);

    // Calculate the initial size for the NexusPlot widget (e.g., 65% of the total size)
    int initialNexusPlotWidth = centralWidget->width() * 0.85;

    // Set the sizes for the widgets in the splitter
    splitter->setSizes({ initialNexusPlotWidth, centralWidget->width() - initialNexusPlotWidth });

    this->nexus_plot->plotLayout()->insertRow(0);
    QCPTextElement* title = new QCPTextElement(this->nexus_plot, portfolio_id.c_str(), QFont("sans", 17, QFont::Bold));
    this->nexus_plot->plotLayout()->addElement(0, 0, title);

    // Set the layout for the central widget
    centralWidget->setLayout(layout);

    // load in the NexusPortfolio to the plot
    this->set_up_strategies_menu();
    this->nexus_plot->load_portfolio(this);
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
    this->menu_bar->addAction(settingsAction);

    PortfolioPtr portfolio = this->nexus_env->get_hydra()->get_portfolio(this->portfolio_id);
    auto strategy_ids = portfolio->get_strategy_ids();

    this->strategies_checkboxes.clear();
    auto new_action = new QAction("AGGREGATE", this);
    new_action->setCheckable(true);
    new_action->setChecked(true);
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
}


//============================================================================
void NexusPortfolio::on_new_hydra_run()
{
    // get the absolute and pct returns for the portfolio
    auto hydra = this->nexus_env->get_hydra();
    auto& nlv = hydra->get_portfolio(this->portfolio_id)->get_nlv_history_vec();
    auto dt_index = hydra->__get_dt_index();

    // calculate total returns 
    QString formattedPct = QString::number(get_stats_total_pl(nlv), 'f', 2);
    this->ui->total_return->setText("$" + formattedPct);

    // calculate pct returns
    formattedPct = QString::number(get_stats_pct_returns(nlv));
    this->ui->pct_return->setText(formattedPct + "%");

    // calculate annualized returns
    formattedPct = QString::number(get_stats_annualized_pct_returns(nlv), 'f', 2);
    this->ui->annualized_return->setText(formattedPct + "%");

    // calculate annualized volatility
    formattedPct = QString::number(get_stats_annualized_volatility(nlv), 'f', 2);
    this->ui->annualized_volatility->setText(formattedPct + "%");

    // calculate annualized sharpe
    formattedPct = QString::number(get_stats_sharpe_ratio(nlv), 'f', 2);
    this->ui->sharpe_ratio->setText(formattedPct);

    for (int row = 0; row < ui->stats->rowCount(); ++row) {
        for (int col = 0; col < ui->stats->columnCount(); ++col) {
            QWidget* widget = ui->stats->itemAtPosition(row, col)->widget();
            if (widget) {
                QLabel* label = qobject_cast<QLabel*>(widget);
                if (label) {
                    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
                    label->adjustSize(); // This will force the label to resize to fit the new text
                }
            }
        }
    }
    // update the stats widget label
    this->stats_widget->update();

    // remove all graphs from the plot
    this->nexus_plot->clearGraphs();
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

        QAction* action = moveSubMenu->addAction("CASH");
        connect(action, &QAction::triggered, this, [this]() {
            this->add_plot("CASH");
            });

        action = moveSubMenu->addAction("NET BETA DOLLARS / NLV");
        connect(action, &QAction::triggered, this, [this]() {
            this->add_plot("NET BETA DOLLARS / NLV");
            });

        action = moveSubMenu->addAction("NET BETA DOLLARS");
        connect(action, &QAction::triggered, this, [this]() {
            this->add_plot("NET BETA DOLLARS");
            });

        action = moveSubMenu->addAction("NLV");
        connect(action, &QAction::triggered, this, [this]() {
            this->add_plot("NLV");
            });

        action = moveSubMenu->addAction("UNDERWATER");
        connect(action, &QAction::triggered, this, [this]() {
            this->add_plot("UNDERWATER");
            });


        if (this->selectedGraphs().size() > 0)
            menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
        if (this->graphCount() > 0)
            menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
    }

    menu->popup(this->mapToGlobal(pos));
}


//============================================================================
std::vector<double> NexusPortfolioPlot::get_data(
    const std::variant<AgisStrategyRef, PortfolioPtr>& entity,
    const std::string& name)
{
    if (name == "CASH") {
        if (std::holds_alternative<AgisStrategyRef>(entity)) {
            return std::get<AgisStrategyRef>(entity).get()->get_cash_history();
        }
        else {
            return std::get<PortfolioPtr>(entity).get()->get_cash_history();
        }
    }
    else if (name == "NLV") {
        if (std::holds_alternative<AgisStrategyRef>(entity)) {
            return std::get<AgisStrategyRef>(entity).get()->get_nlv_history();
        }
        else {
            return std::get<PortfolioPtr>(entity).get()->get_nlv_history();
        }
    }
    else if (name == "NET BETA DOLLARS / NLV") {
        if (std::holds_alternative<AgisStrategyRef>(entity)) {
            AgisStrategyRef entity_ptr = std::get<AgisStrategyRef>(entity);
            return entity_ptr.get()->get_beta_history() / entity_ptr.get()->get_nlv_history();
        }
        else {
            PortfolioPtr entity_ptr = std::get<PortfolioPtr>(entity);
            return entity_ptr->get_beta_history() / entity_ptr->get_nlv_history();
        }
    }
    else if (name == "NET BETA DOLLARS") {
        if (std::holds_alternative<AgisStrategyRef>(entity)) {
            return std::get<AgisStrategyRef>(entity).get()->get_beta_history();
        }
        else {
            return std::get<PortfolioPtr>(entity).get()->get_beta_history();
        }
    }
    else if (name == "UNDERWATER") {
        std::span<double const> y_span;
        if (std::holds_alternative<AgisStrategyRef>(entity)) {
            y_span = std::get<AgisStrategyRef>(entity).get()->get_nlv_history();
        }
        else {
            y_span = std::get<PortfolioPtr>(entity).get()->get_nlv_history();
        }
        return get_stats_underwater_plot(y_span);

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
        std::variant<AgisStrategyRef, PortfolioPtr> entity = nullptr;
        if (this->hydra->strategy_exists(strategy_id)) {
            entity = portfolio->__get_strategy(strategy_id);
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
