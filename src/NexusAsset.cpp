#include <QSplitter>

#include "NexusAsset.h"
#include "ui_NexusAsset.h"

NexusAsset::NexusAsset(
        NexusEnv const* nexus_env_,
        ads::CDockWidget* DockWidget_,
        SharedAssetLockPtr asset_,
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

    // Set the size policy to occupy 65% of the available width
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(55); // Set the horizontal stretch factor
    this->nexus_plot->setSizePolicy(sizePolicy);

    // Add the NexusPlot widget and table widget to the layout
    layout->addWidget(this->nexus_plot);
    layout->addWidget(scrollArea);

    // Set the layout for the central widget
    centralWidget->setLayout(layout);
}

void NexusAsset::load_asset_data()
{
    auto asset_lock = this->asset->read().unwrap();
    this->dt_index = asset_lock->__get_dt_index();
    this->dt_index_str = asset_lock->__get_dt_index_str();
    this->data = asset_lock->__get__data();
    this->column_names = asset_lock->get_column_names();

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
    for (int i = 0; i < rows; ++i) {
        auto row = this->data.row(i);
        for (int j = 0; j < columns; ++j) {
            double value = row[j];
            QStandardItem* item = new QStandardItem(QString::number(value));
            model->setItem(i, j, item);
        }
    }

    // Set the model in the table view
    this->table_view->setModel(model);

    // Resize the columns to fit the content
    this->table_view->resizeColumnsToContents();
   
}

size_t NexusAsset::get_column_index(std::string const& column_name)
{
    auto it = std::find(this->column_names.begin(), this->column_names.end(), column_name);

    if (it != this->column_names.end()) {
        return std::distance(this->column_names.begin(), it);

    }
}

NexusAssetPlot::NexusAssetPlot(QWidget* parent) : NexusPlot(parent)
{
}

void NexusAssetPlot::load_asset(NexusAsset* asset_)
{
    this->asset = asset_;
}

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

        if (this->selectedGraphs().size() > 0)
            menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
        if (this->graphCount() > 0)
            menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
    }

    menu->popup(this->mapToGlobal(pos));
}

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

