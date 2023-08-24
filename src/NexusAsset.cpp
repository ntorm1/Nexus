
#include "NexusAsset.h"
#include "ui_NexusAsset.h"


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
    this->table_view = new QTableView(this);
    this->table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(this->table_view);
    this->load_asset_data();

    // Retrieve the NexusPlot widget from the UI
    this->nexus_plot = ui->widget;
    this->nexus_plot->load_asset(this);

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
    QCPTextElement* title = new QCPTextElement(this->nexus_plot, this->asset->get_asset_id().c_str(), QFont("sans", 17, QFont::Bold));
    this->nexus_plot->plotLayout()->addElement(0, 0, title);

    // Set the layout for the central widget
    centralWidget->setLayout(layout);
}


//============================================================================
void NexusAsset::load_asset_data()
{
    this->dt_index = asset->__get_dt_index();
    this->dt_index_str = asset->__get_dt_index_str();
    this->data = asset->__get__data();
    this->column_names = asset->get_column_names();

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
    for (int i = 0; i < columns; ++i) {
        auto col = this->data.column(i);
        for (int j = 0; j < rows; ++j) {
            double value = col[j];
            QStandardItem* item = new QStandardItem(QString::number(value));
            model->setItem(j, i, item);
        }
    }

    // Set the model in the table view
    this->table_view->setModel(model);

    // Resize the columns to fit the content
    this->table_view->resizeColumnsToContents();
   
}


//============================================================================
size_t NexusAsset::get_column_index(std::string const& column_name)
{
    auto it = std::find(this->column_names.begin(), this->column_names.end(), column_name);
    return std::distance(this->column_names.begin(), it);
}


//============================================================================
NexusAssetPlot::NexusAssetPlot(QWidget* parent) : NexusPlot(parent)
{
}


//============================================================================
void NexusAssetPlot::load_asset(NexusAsset* asset_)
{
    this->asset = asset_;
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
        for (auto const& column_name : this->asset->column_names)
        {
            auto q_name = QString::fromStdString(column_name);
            QAction* action = moveSubMenu->addAction(q_name);
            connect(action, &QAction::triggered, this, [this, q_name]() {
                this->new_plot(q_name);
            });
        }

        moveSubMenu->addSeparator(); // Add a separator line


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
    auto column_index = asset->get_column_index(column_name);
    auto y_vec = asset->data.column(column_index);
    this->plot(
        asset->dt_index,
        y_vec,
        name.toStdString()
    );
}


//============================================================================
void NexusAssetPlot::plot_event_overlays()
{
}

