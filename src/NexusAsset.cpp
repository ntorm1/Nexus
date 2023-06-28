

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

    // Retrieve the NexusPlot widget from the UI
    this->nexus_plot = ui->widget;
    this->init_plot();

    // Set the size policy to occupy 65% of the available width
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(60); // Set the horizontal stretch factor
    this->nexus_plot->setSizePolicy(sizePolicy);

    // Create a QTableWidget
    QTableView* tableView = new QTableView(this);

    // Add the NexusPlot widget and table widget to the layout
    layout->addWidget(this->nexus_plot);
    layout->addWidget(tableView);

    // Set the layout for the central widget
    centralWidget->setLayout(layout);

}

void NexusAsset::init_plot()
{
    auto asset_lock = this->asset->read().unwrap();
    this->dt_index = asset_lock->__get_dt_index();
    auto y_vec = asset_lock->__get_column("CLOSE");
    this->nexus_plot->plot(this->dt_index, y_vec);
}