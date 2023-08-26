
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
	this->portfolio_id = portfolio_id_;

	// Retrieve the central widget from the UI
	QWidget* centralWidget = ui->centralwidget;

    // Create a layout for the central widget
    QHBoxLayout* layout = new QHBoxLayout(centralWidget);
    layout->setSpacing(10); // Set spacing between widgets
    layout->setContentsMargins(10, 10, 10, 10); // Set margins around the layout

    // Create a QTableWidget
    this->table_view = new QTableView(this);
    this->table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(this->table_view);
    //this->load_asset_data();

    // Retrieve the NexusPlot widget from the UI
    this->nexus_plot = ui->widget;
    auto hydra = this->nexus_env->get_hydra();
    this->nexus_plot->load(hydra, portfolio_id);

    QSplitter* splitter = new QSplitter(Qt::Horizontal, centralWidget);
    layout->addWidget(splitter);

    // Add the NexusPlot widget and table widget to the splitter
    splitter->addWidget(this->nexus_plot);
    splitter->addWidget(scrollArea);

    // Calculate the initial size for the NexusPlot widget (e.g., 65% of the total size)
    int initialNexusPlotWidth = centralWidget->width() * 0.65;

    // Set the sizes for the widgets in the splitter
    splitter->setSizes({ initialNexusPlotWidth, centralWidget->width() - initialNexusPlotWidth });

    this->nexus_plot->plotLayout()->insertRow(0);
    QCPTextElement* title = new QCPTextElement(this->nexus_plot, portfolio_id.c_str(), QFont("sans", 17, QFont::Bold));
    this->nexus_plot->plotLayout()->addElement(0, 0, title);

    // Set the layout for the central widget
    centralWidget->setLayout(layout);
}


//============================================================================
void NexusPortfolio::on_new_hydra_run()
{
    qDebug() << "HERE";
}


//============================================================================
NexusPortfolioPlot::NexusPortfolioPlot(QWidget* parent_)
{
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
        QAction* action = moveSubMenu->addAction("NLV");
        connect(action, &QAction::triggered, this, [this]() {
            this->plot_nlv();
            });

        if (this->selectedGraphs().size() > 0)
            menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
        if (this->graphCount() > 0)
            menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
    }

    menu->popup(this->mapToGlobal(pos));
}

//============================================================================
void NexusPortfolioPlot::plot_nlv()
{
    auto& portfolio = this->hydra->get_portfolio(this->portfolio_id);
    auto y = portfolio->get_nlv_history();
    std::span<double> y_sp = std::span<double>(y.data(), y.size());

    this->plot(
        hydra->__get_dt_index(),
        y_sp,
        "NLV"
    );
}
