

#include "NexusAsset.h"
#include "ui_NexusAsset.h"

NexusAsset::NexusAsset(
        NexusEnv const* nexus_env_,
        ads::CDockWidget* DockWidget_,
        std::string asset_id_,
        QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::NexusAsset),
    nexus_env(nexus_env_),
    DockWidget(DockWidget_)
{
    ui->setupUi(this);
    this->asset_id = asset_id_;

    // Retrieve the central widget from the UI
    QWidget* centralWidget = ui->centralwidget;

    // Create a layout for the central widget
    QHBoxLayout* layout = new QHBoxLayout(centralWidget);
    layout->setSpacing(10); // Set spacing between widgets
    layout->setContentsMargins(10, 10, 10, 10); // Set margins around the layout

    // Retrieve the NexusPlot widget from the UI
    NexusPlot* plotWidget = ui->widget;

    // Set the size policy to occupy 65% of the available width
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(60); // Set the horizontal stretch factor to 65
    plotWidget->setSizePolicy(sizePolicy);

    // Create a QTableWidget
    QTableView* tableView = new QTableView(this);

    // Add the NexusPlot widget and table widget to the layout
    layout->addWidget(plotWidget);
    layout->addWidget(tableView);

    // Set the layout for the central widget
    centralWidget->setLayout(layout);

}
