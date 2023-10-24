
#include "NexusWidgetFactory.h"
#include "MainWindow.h"


//============================================================================
NexusWidgetFactory::NexusWidgetFactory()
{
}


//============================================================================
NexusWidgetFactory::~NexusWidgetFactory()
{
}


//============================================================================
ads::CDockWidget* NexusWidgetFactory::create_portfolios_widget(MainWindow* window)
{
    PortfolioTree* w = new PortfolioTree(window, window->nexus_env.get_hydra());
    window->nexus_env.new_tree(w);
    window->portfolio_tree = w;

    // Signal that requests new strategy
    QObject::connect(
        w,
        SIGNAL(new_strategy_requested(QModelIndex, QString, QString, QString, AgisStrategyType)),
        window,
        SLOT(window->on_new_strategy_requested(QModelIndex, QString, QString, QString, AgisStrategyType))
    );
    // Signal that requests to remove a strategy
    QObject::connect(
        w,
        SIGNAL(strategy_remove_requested(QModelIndex, QString)),
        window,
        SLOT(window->on_strategy_remove_requested(QModelIndex, QString))
    );
    // Signal to request removal of portfolio
    QObject::connect(
        w,
        SIGNAL(remove_item_requested(QString, QModelIndex)),
        window,
        SLOT(window->on_remove_portfolio_request(QString, QModelIndex))
    );
    // Signal that requests new portfolio
    QObject::connect(
        w,
        SIGNAL(new_item_requested(QModelIndex, QString, QString)),
        window,
        SLOT(window->on_new_portfolio_request(QModelIndex, QString, QString))
    );
    // Signal that accepets new strategy
    QObject::connect(
        window,
        SIGNAL(window->new_strategy_accepeted(QModelIndex, QString)),
        w,
        SLOT(new_item_accepted(QModelIndex, QString))
    );
    // Signal to accept new portfolio
    QObject::connect(
        window,
        SIGNAL(window->new_portfolio_accepeted(QModelIndex, QString)),
        w,
        SLOT(new_item_accepted(QModelIndex, QString))
    );
    // Signal to accept removal of portfolio
    QObject::connect(
        window,
        SIGNAL(window->remove_portfolio_accepted(QModelIndex)),
        w,
        SLOT(remove_item_accepeted(QModelIndex))
    );
    // Signal that accepts to remove a strategy
    QObject::connect(
        window,
        SIGNAL(window->remove_strategy_accepted(QModelIndex)),
        w,
        SLOT(remove_item_accepeted(QModelIndex))
    );
    // Signal to create new node editor window
    QObject::connect(
        w,
        SIGNAL(strategy_double_clicked(QString)),
        window,
        SLOT(window->on_new_node_editor_request(QString))
    );
    // Signal to create new portfolio window
    QObject::connect(
        w,
        SIGNAL(portfolio_double_clicked(QString)),
        window,
        SLOT(window->on_new_portfolio_window_request(QString))
    );
    // Signal to toggle strategy
    QObject::connect(
        w,
        SIGNAL(strategy_toggled(QString, bool)),
        window,
        SLOT(window->on_strategy_toggle(QString, bool))
    );

    ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Portfolios")
        .arg(0));
    w->setFocusPolicy(Qt::NoFocus);
    DockWidget->setWidget(w);
    DockWidget->setIcon(svgIcon("./images/piechart.png"));

    DockWidget->set_widget_type(WidgetType::Portfolios);
    return DockWidget;
}


//============================================================================
ads::CDockWidget* NexusWidgetFactory::create_exchanges_widget(MainWindow* window)
{
    ExchangeTree* w = new ExchangeTree(window, &window->nexus_env);
    window->nexus_env.new_tree(w);
    window->exchange_tree = w;

    // Signal that requests new exchanges
    QObject::connect(
        w,
        SIGNAL(new_item_requested(QModelIndex, NewExchangePopup*)),
        window,
        SLOT(on_new_exchange_request(QModelIndex, NewExchangePopup*))
    );

    // Signal to accept new exchanges
    QObject::connect(
        window,
        SIGNAL(new_exchange_accepted(QModelIndex, QString)),
        w,
        SLOT(new_item_accepted(QModelIndex, QString))
    );

    // Signal to request removal of exchange
    QObject::connect(
        w,
        SIGNAL(remove_item_requested(QString, QModelIndex)),
        window,
        SLOT(on_remove_exchange_request(QString, QModelIndex))
    );

    // Signal to accept removal of exchange
    QObject::connect(
        window,
        SIGNAL(remove_exchange_accepted(QModelIndex)),
        w,
        SLOT(remove_item_accepted(QModelIndex))
    );

    // Signal to create a new asset window
    QObject::connect(
        w,
        SIGNAL(asset_double_click(QString)),
        window,
        SLOT(on_new_asset_window_request(QString))
    );

    ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Exchanges")
        .arg(0));
    DockWidget->setWidget(w);
    DockWidget->setIcon(svgIcon("./images/exchange.png"));
    DockWidget->set_widget_type(WidgetType::Exchanges);

    w->setFocusPolicy(Qt::NoFocus);
    return DockWidget;
}


//============================================================================
ads::CDockWidget* NexusWidgetFactory::create_file_system_tree_widget(MainWindow* window)
{
    QTreeView* w = new QTreeView();
    w->setFrameShape(QFrame::NoFrame);
    QFileSystemModel* m = new QFileSystemModel(w);
    m->setRootPath(QDir::currentPath());
    w->setModel(m);
    w->setRootIndex(m->index(QDir::currentPath()));

    ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Files")
        .arg(0));
    DockWidget->setWidget(w);
    DockWidget->set_widget_type(WidgetType::FileTree);
    DockWidget->setIcon(svgIcon(".images/folder_open.svg"));
    // We disable focus to test focus highlighting if the dock widget content
    // does not support focus
    w->setFocusPolicy(Qt::NoFocus);

    QObject::connect(
        w,
        &QAbstractItemView::doubleClicked,
        window,
        &MainWindow::onFileDoubleClicked
    );
    return DockWidget;
}