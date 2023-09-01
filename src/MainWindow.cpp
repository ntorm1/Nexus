#include <fstream>

#include <QTime>
#include <QLabel>
#include <QLocale>
#include <QTextEdit>
#include <QCalendarWidget>
#include <QFrame>
#include <QTreeView>
#include <QFileSystemModel>
#include <QBoxLayout>
#include <QSettings>
#include <QDockWidget>
#include <QDebug>
#include <QResizeEvent>
#include <QAction>
#include <QWidgetAction>
#include <QComboBox>
#include <QInputDialog>
#include <QRubberBand>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QScreen>
#include <QStyle>
#include <QMessageBox>
#include <QMenu>
#include <QToolButton>
#include <QToolBar>
#include <QPointer>
#include <QMap>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QDateTime>
#include "AgisPointers.h"

#include "MainWindow.h"
#include "ui_MainWindow.h"

// Adavance Docking System
#include "AutoHideDockContainer.h"
#include "DockAreaWidget.h"
#include "DockAreaTitleBar.h"
#include "DockAreaTabBar.h"
#include "FloatingDockContainer.h"
#include "DockComponentsFactory.h"
#include "NexusDockManager.h"
#include "NexusTree.h"
#include "NexusNode.h"
#include "NexusErrors.h"
#include "NexusPortfolio.h"

// Octave Win32 Terminal 
#include "QTerminalImpl.h"



using namespace ads;

MainWindow::~MainWindow()
{
    delete ui;
}

//============================================================================

static QIcon svgIcon(const QString& File)
{
    // This is a workaround, because in item views SVG icons are not
    // properly scaled and look blurry or pixelate
    QIcon SvgIcon(File);
    SvgIcon.addPixmap(SvgIcon.pixmap(92));
    return SvgIcon;
}

//============================================================================
class CCustomComponentsFactory : public ads::CDockComponentsFactory
{
public:
    using Super = ads::CDockComponentsFactory;
    ads::CDockAreaTitleBar* createDockAreaTitleBar(ads::CDockAreaWidget* DockArea) const override
    {
        auto TitleBar = new ads::CDockAreaTitleBar(DockArea);
        auto CustomButton = new QToolButton(DockArea);
        CustomButton->setToolTip(QObject::tr("Help"));
        CustomButton->setIcon(svgIcon("./images/help_outline.svg"));
        CustomButton->setAutoRaise(true);
        int Index = TitleBar->indexOf(TitleBar->button(ads::TitleBarButtonTabsMenu));
        TitleBar->insertWidget(Index + 1, CustomButton);
        return TitleBar;
    }
};

//============================================================================
MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qDebug() << "INIT MAIN WINDOW";

    QString exePath = QCoreApplication::applicationDirPath();
    this->nexus_env.set_env_name(exePath.toStdString(), "default");
    qDebug() << "INIT MAIN WINDOW UI";  
    this->ui->setupUi(this);
    this->ProgressBar = new QProgressBar(this);
    this->ProgressBar->setFixedWidth(100);
    this->ProgressBar->setVisible(true);
    qDebug() << "INIT MAIN WINDOW UI COMPLETE";
    ads::CDockComponentsFactory::setFactory(new CCustomComponentsFactory());

    // Create the dock manager. Because the parent parameter is a QMainWindow
    // the dock manager registers itself as the central widget.
    CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
    CDockManager::setConfigFlag(CDockManager::AllTabsHaveCloseButton, true);
    CDockManager::setConfigFlag(CDockManager::DragPreviewIsDynamic, true);
    CDockManager::setConfigFlag(CDockManager::DragPreviewShowsContentPixmap, false);
    CDockManager::setConfigFlag(CDockManager::DragPreviewHasWindowFrame, false);
    CDockManager::setAutoHideConfigFlags(CDockManager::DefaultAutoHideConfig);

    qDebug() << "INIT MAIN WINDOW DOCK MANAGER";
    DockManager = new NexusDockManager(this, this);
    connect(DockManager, &CDockManager::focusedDockWidgetChanged,
        this, &MainWindow::on_widget_focus);
    qDebug() << "INIT MAIN WINDOW DOCK MANAGER COMPLETE";

    setWindowTitle(tr("Nexus"));
    setup_help_menu();

    setup_toolbar();
    create_perspective_ui();
    setup_command_bar();

    // create file system widget
    qDebug() << "INIT BASE WIDGETS";
    auto FileSystemWidget = create_file_system_tree_widget();
    FileSystemWidget->setFeature(ads::CDockWidget::DockWidgetFloatable, false);
    //FileSystemWidget->setFeature(ads::CDockWidget::DockWidgetMovable, false);
    FileSystemWidget->setFeature(ads::CDockWidget::DockWidgetClosable, false);
    auto container = this->DockManager->addAutoHideDockWidget(ads::SideBarLeft, FileSystemWidget);
    container->setSize(300);

    // create exchanges widget
    auto ExchangesWidget = create_exchanges_widget();
    ExchangesWidget->setFeature(ads::CDockWidget::DockWidgetFloatable, false);
    //ExchangesWidget->setFeature(ads::CDockWidget::DockWidgetMovable, false);
    ExchangesWidget->setFeature(ads::CDockWidget::DockWidgetClosable, false);
    container = this->DockManager->addAutoHideDockWidget(ads::SideBarLeft, ExchangesWidget);
    container->setSize(200);

    // create exchanges widget
    auto PortfoliosWidget = create_portfolios_widget();
    PortfoliosWidget->setFeature(ads::CDockWidget::DockWidgetFloatable, false);
    //ExchangesWidget->setFeature(ads::CDockWidget::DockWidgetMovable, false);
    PortfoliosWidget->setFeature(ads::CDockWidget::DockWidgetClosable, false);
    container = this->DockManager->addAutoHideDockWidget(ads::SideBarLeft, PortfoliosWidget);
    container->setSize(200);
    qDebug() << "INIT BASE WIDGETS COMPLETE";

    applyVsStyle();
    qDebug() << "INIT MAIN WINDOW COMPLETE";
}


//============================================================================
void MainWindow::about()
{
    QMessageBox::about(this, tr("About Syntax Highlighter"),
        tr("<p>The <b>Syntax Highlighter</b> example shows how " \
            "to perform simple syntax highlighting by subclassing " \
            "the QSyntaxHighlighter class and describing " \
            "highlighting rules using regular expressions.</p>"));
}


//============================================================================
ads::CDockWidget* MainWindow::create_console_widget()
{
    static int console_count = 0;
    
    ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Console %1").arg(console_count++));
    QWinTerminalImpl* terminal = new QWinTerminalImpl(DockWidget);

    // optional style options
    DockWidget->setWidget(terminal);

    this->DockManager->addDockWidget(ads::TopDockWidgetArea, DockWidget);

    return DockWidget;
}


//============================================================================
ads::CDockWidget* MainWindow::create_portfolios_widget()
{
    static int exchanges_widgets_counter = 0;

    PortfolioTree* w = new PortfolioTree(this, this->nexus_env.get_hydra());
    this->nexus_env.new_tree(w);
    this->portfolio_tree = w;

    // Signal that requests new strategy
    QObject::connect(
        w,
        SIGNAL(new_strategy_requested(QModelIndex, QString, QString, QString, AgisStrategyType)),
        this,
        SLOT(on_new_strategy_requested(QModelIndex, QString, QString, QString, AgisStrategyType))
    );
    // Signal that requests to remove a strategy
    QObject::connect(
        w,
        SIGNAL(strategy_remove_requested(QModelIndex, QString)),
        this,
        SLOT(on_strategy_remove_requested(QModelIndex, QString))
    );
    // Signal to request removal of portfolio
    QObject::connect(
        w,
        SIGNAL(remove_item_requested(QString, QModelIndex)),
        this,
        SLOT(on_remove_portfolio_request(QString, QModelIndex))
    );
    // Signal that requests new portfolio
    QObject::connect(
        w,
        SIGNAL(new_item_requested(QModelIndex, QString, QString)),
        this,
        SLOT(on_new_portfolio_request(QModelIndex, QString, QString))
    );
    // Signal that accepets new strategy
    QObject::connect(
        this,
        SIGNAL(new_strategy_accepeted(QModelIndex, QString)),
        w,
        SLOT(new_item_accepted(QModelIndex, QString))
    );
    // Signal to accept new portfolio
    QObject::connect(
        this,
        SIGNAL(new_portfolio_accepeted(QModelIndex, QString)),
        w,
        SLOT(new_item_accepted(QModelIndex, QString))
    );
    // Signal to accept removal of portfolio
    QObject::connect(
        this,
        SIGNAL(remove_portfolio_accepted(QModelIndex)),
        w,
        SLOT(remove_item_accepeted(QModelIndex))
    );
    // Signal that accepts to remove a strategy
    QObject::connect(
        this,
        SIGNAL(remove_strategy_accepted(QModelIndex)),
        w,
        SLOT(remove_item_accepeted(QModelIndex))
    );
    // Signal to create new node editor window
    QObject::connect(
        w,
        SIGNAL(strategy_double_clicked(QString)),
        this,
        SLOT(on_new_node_editor_request(QString))
    );
    // Signal to create new portfolio window
    QObject::connect(
        w,
        SIGNAL(portfolio_double_clicked(QString)),
        this,
        SLOT(on_new_portfolio_window_request(QString))
    );
    // Signal to toggle strategy
    QObject::connect(
        w,
        SIGNAL(strategy_toggled(QString, bool)),
        this,
        SLOT(on_strategy_toggle(QString, bool))
    );

    ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Portfolios")
        .arg(exchanges_widgets_counter++));
    w->setFocusPolicy(Qt::NoFocus);
    DockWidget->setWidget(w);
    DockWidget->setIcon(svgIcon("./images/piechart.png"));

    DockWidget->set_widget_type(WidgetType::Portfolios);
    return DockWidget;
}


//============================================================================
ads::CDockWidget* MainWindow::create_exchanges_widget()
{
    static int exchanges_widgets_counter = 0;

    ExchangeTree* w = new ExchangeTree(this, &this->nexus_env);
    this->nexus_env.new_tree(w);
    this->exchange_tree = w;

    // Signal that requests new exchanges
    QObject::connect(
        w, 
        SIGNAL(new_item_requested(QModelIndex, QString, QString, QString, QString, std::optional<MarketAsset>)),
        this, 
        SLOT(on_new_exchange_request(QModelIndex, QString, QString, QString, QString, std::optional<MarketAsset>))
    );
    // Signal to accept  new exchanges
    QObject::connect(
        this,
        SIGNAL(new_exchange_accepted(QModelIndex, QString)),
        w,
        SLOT(new_item_accepted(QModelIndex, QString))
    );
    // Signal to request removal of exchange
    QObject::connect(
        w,
        SIGNAL(remove_item_requested(QString, QModelIndex)),
        this,
        SLOT(on_remove_exchange_request(QString, QModelIndex))
    );
    // Signal to accept removal of exchange
    QObject::connect(
        this,
        SIGNAL(remove_exchange_accepted(QModelIndex)),
        w,
        SLOT(remove_item_accepeted(QModelIndex))
    );
    // Signal to create new asset window
    QObject::connect(
        w,
        SIGNAL(asset_double_click(QString)),
        this,
        SLOT(on_new_asset_window_request(QString))
    );

    ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Exchanges")
        .arg(exchanges_widgets_counter++));
    DockWidget->setWidget(w);
    DockWidget->setIcon(svgIcon("./images/exchange.png"));
    DockWidget->set_widget_type(WidgetType::Exchanges);

    w->setFocusPolicy(Qt::NoFocus);
    return DockWidget;
}

//============================================================================
ads::CDockWidget* MainWindow::create_file_system_tree_widget()
{
    static int file_system_count = 0;

    QTreeView* w = new QTreeView();
    w->setFrameShape(QFrame::NoFrame);
    QFileSystemModel* m = new QFileSystemModel(w);
    m->setRootPath(QDir::currentPath());
    w->setModel(m);
    w->setRootIndex(m->index(QDir::currentPath()));

    ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Files")
        .arg(file_system_count++));
    DockWidget->setWidget(w);
    DockWidget->set_widget_type(WidgetType::FileTree);
    DockWidget->setIcon(svgIcon(".images/folder_open.svg"));

    ui->menuView->addAction(DockWidget->toggleViewAction());
    // We disable focus to test focus highlighting if the dock widget content
    // does not support focus
    w->setFocusPolicy(Qt::NoFocus);

    connect(w, &QAbstractItemView::doubleClicked, this, &MainWindow::onFileDoubleClicked);

    return DockWidget;
}

//============================================================================
ads::CDockWidget* MainWindow::create_editor_widget()
{
    // Build the new TextEdit widget
    static int editor_count = 0;

    // Add the new TextEdit widget to a DockWidget
    ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Editor %1").arg(editor_count++));
    
    TextEdit* w = new TextEdit(&this->nexus_env, DockWidget, DockWidget);
    this->nexus_env.new_editor(w);

    DockWidget->setWidget(w);
    DockWidget->set_widget_type(WidgetType::Editor);
    DockWidget->setIcon(svgIcon("./images/edit.svg"));
    DockWidget->setFeature(ads::CDockWidget::CustomCloseHandling, true);


    connect(DockWidget, SIGNAL(closeRequested()), SLOT(on_editor_close_requested()));

    return DockWidget;
}

//============================================================================
ads::CDockWidget* MainWindow::create_asset_widget(const QString& asset_id)
{
    ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Asset: %1").arg(asset_id));

    auto asset = this->nexus_env.get_asset(asset_id.toStdString());
    if (asset.is_exception()) {
        QMessageBox::critical(this, "Error", asset.get_exception().c_str());
    }
    
    NexusAsset* w = new NexusAsset(
        &this->nexus_env,
        DockWidget,
        asset.unwrap(),
        DockWidget
    );

    // Signal for new hydra run complete
    QObject::connect(
        this,
        SIGNAL(new_hydra_run()),
        w,
        SLOT(on_new_hydra_run())
    );

    DockWidget->setWidget(w);
    DockWidget->setIcon(svgIcon("./images/stock.png"));
    DockWidget->set_widget_type(WidgetType::Asset);
    return DockWidget;
}

ads::CDockWidget* MainWindow::create_portfolio_widget(const QString& portfolio_id)
{
    ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Portfolio: %1").arg(portfolio_id));
    NexusPortfolio* w = new NexusPortfolio(
        &this->nexus_env,
        DockWidget,
        portfolio_id.toStdString(),
        DockWidget
    );

    // Signal for new hydra run complet
    QObject::connect(
        this,
        SIGNAL(new_hydra_run()),
        w,
        SLOT(on_new_hydra_run())
    );

    DockWidget->setWidget(w);
    DockWidget->setIcon(svgIcon("./images/piechart.png"));
    DockWidget->set_widget_type(WidgetType::Portfolio);
    return DockWidget;
}

ads::CDockWidget* MainWindow::create_node_editor_widget(const QString& strategy_id)
{
    ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Strategy: %1").arg(strategy_id));
    auto strategy_opt = this->nexus_env.__get_strategy(strategy_id.toStdString());
    // make sure the strategy is real
    if (!strategy_opt.has_value()) {

        QMessageBox::critical(this, "Error", "Failed to find strategy listed");
        delete DockWidget;
        return nullptr;
    }
    auto strategy = strategy_opt.value();
    // make sure the strategy is abstract
    if (!strategy->__is_abstract_class())
    {
		QMessageBox::critical(this, "Error", "Strategy is not abstract");
		delete DockWidget;
		return nullptr;
    }
    // make sure the strategy is not already open 
    auto res = this->nexus_env.new_node_editor(strategy_id.toStdString());
    if(res.is_exception())
	{
		QMessageBox::critical(this, "Error", "Strategy editor is already open");
		delete DockWidget;
		return nullptr;
	}

    NexusNodeEditor* w = new NexusNodeEditor(
        &this->nexus_env,
        DockWidget,
        strategy,
        DockWidget
    );

    // intercept the close request to modify open node editor state
    DockWidget->setFeature(ads::CDockWidget::CustomCloseHandling, true);
    connect(DockWidget, SIGNAL(closeRequested()), SLOT(on_node_editor_close_request()));

    DockWidget->setWidget(w);
    DockWidget->setIcon(svgIcon("./images/flow.png"));
    DockWidget->set_widget_type(WidgetType::NodeEditor);
    return DockWidget;
}

void MainWindow::place_widget(ads::CDockWidget* dock_widget, ads::CDockAreaWidget* dock_area)
{
    this->DockManager->addDockWidget(ads::RightDockWidgetArea, dock_widget, dock_area);
}

//============================================================================
void MainWindow::place_widget(ads::CDockWidget* DockWidget, QObject* Sender)
{
    QVariant vFloating = Sender->property("Floating");
    bool Floating = vFloating.isValid() ? vFloating.toBool() : true;
    QVariant vTabbed = Sender->property("Tabbed");
    bool Tabbed = vTabbed.isValid() ? vTabbed.toBool() : true;

    DockWidget->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    DockWidget->setFeature(ads::CDockWidget::DockWidgetForceCloseWithArea, true);

    // Creating a new floating dock widget
    if (Floating)
    {
        auto FloatingWidget = this->DockManager->addDockWidgetFloating(DockWidget);
        this->LastCreatedFloatingEditor = DockWidget;
        this->LastDockedEditor.clear();
        return;
    }

    // Get the editor area, either a docked editor, floating editor, or none
    ads::CDockAreaWidget* EditorArea;
    if (this->LastDockedEditor)
    {
        EditorArea = this->LastDockedEditor->dockAreaWidget();
    }
    else if (this->LastCreatedFloatingEditor)
    {
        EditorArea = this->LastCreatedFloatingEditor->dockAreaWidget();
    }
    else
    {
        EditorArea = nullptr;
    }
    if (EditorArea)
    {
        if (Tabbed)
        {
            // Test inserting the dock widget tab at a given position instead
            // of appending it. This function inserts the new dock widget as
            // first tab
            this->DockManager->addDockWidgetTabToArea(DockWidget, EditorArea, 0);
        }
        else
        {
            this->DockManager->setConfigFlag(ads::CDockManager::EqualSplitOnInsertion, true);
            this->DockManager->addDockWidget(ads::RightDockWidgetArea, DockWidget, EditorArea);
        }
    }
    else
    {
        if (this->LastCreatedFloatingEditor)
        {
            this->DockManager->addDockWidget(ads::RightDockWidgetArea, DockWidget, this->LastCreatedFloatingEditor->dockAreaWidget());
        }
        else
        {
            this->DockManager->addDockWidget(ads::TopDockWidgetArea, DockWidget);
        }
    }
    this->LastDockedEditor = DockWidget;
}

//============================================================================
void MainWindow::create_editor()
{
    auto DockWidget = this->create_editor_widget();
    this->place_widget(DockWidget, sender());
}

//============================================================================
void MainWindow::setup_toolbar()
{
    qDebug() << "INIT TOOL BAR";
    ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->toolBar->addAction(ui->actionSaveState);
    ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->actionSaveState->setIcon(svgIcon("./images/save.svg"));
    ui->toolBar->addAction(ui->actionRestoreState);
    ui->actionRestoreState->setIcon(svgIcon("./images/restore.svg"));
    ui->toolBar->addSeparator();

    //QAction* a = new QAction("New Console", ui->toolBar);
    //a->setProperty("Floating", true);
    //a->setToolTip("Creates a new Nexus Env console window");
    //a->setIcon(svgIcon("./images/console.png"));
    //connect(a, &QAction::triggered, this, &MainWindow::create_console_widget);
    //ui->toolBar->addAction(a);

    QAction* a = new QAction("Create Floating Editor", ui->toolBar);
    a->setProperty("Floating", true);
    a->setToolTip("Creates a docked editor windows that are deleted on close");
    a->setIcon(svgIcon("./images/note_add.svg"));
    connect(a, &QAction::triggered, this, &MainWindow::create_editor);
    ui->toolBar->addAction(a);

    a = new QAction("Create Docked Editor", ui->toolBar);
    a->setProperty("Floating", false);
    a->setProperty("Tabbed", false);
    a->setToolTip("Creates a docked editor windows that are deleted on close");
    a->setIcon(svgIcon("./images/docked_editor.svg"));
    connect(a, &QAction::triggered, this, &MainWindow::create_editor);
    ui->toolBar->addAction(a);

    a = new QAction("Create Editor Tab", ui->toolBar);
    a->setProperty("Floating", false);
    a->setProperty("Tabbed", true);
    a->setToolTip("Creates a docked editor windows that are deleted on close");
    a->setIcon(svgIcon("./images/tab.svg"));
    connect(a, &QAction::triggered, this, &MainWindow::create_editor);
    ui->toolBar->addAction(a);
    qDebug() << "INIT TOOL BAR COMPLETE";
}

//============================================================================
void MainWindow::setup_help_menu()
{
    QMenu* helpMenu = new QMenu(tr("&Help"), this);
    menuBar()->addMenu(helpMenu);

    helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
}

void MainWindow::setup_command_bar()
{
    qDebug() << "INIT COMMAND BAR";
    QWidget* spacerWidget = new QWidget(ui->toolBar);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->toolBar->addWidget(spacerWidget);


    QAction* a = new QAction("Compile", ui->toolBar);
    a->setProperty("Floating", true);
    a->setToolTip("Compile Agis strategies");
    a->setIcon(svgIcon("./images/console.png"));
    connect(a, &QAction::triggered, this, &MainWindow::__run_compile);
    ui->toolBar->addAction(a);

    a = new QAction("Link", ui->toolBar);
    a->setProperty("Floating", true);
    a->setToolTip("Link Agis strategies");
    a->setIcon(svgIcon("./images/link.png"));
    connect(a, &QAction::triggered, this, &MainWindow::__run_link);
    ui->toolBar->addAction(a);

    a = new QAction("Run", ui->toolBar);
    a->setProperty("Floating", false);
    a->setProperty("Tabbed", true);
    a->setToolTip("Executes Hyda instance run");
    a->setIcon(svgIcon("./images/run.png"));
    connect(a, &QAction::triggered, this, &MainWindow::__run_lambda);
    ui->toolBar->addAction(a);
    ui->toolBar->addWidget(this->ProgressBar);
    qDebug() << "INIT COMMAND BAR COMPLETE";
}

//============================================================================
void MainWindow::save_perspective()
{
    QString PerspectiveName = QInputDialog::getText(this, "Save Perspective", "Enter unique name:");
    if (PerspectiveName.isEmpty())
    {
        return;
    }

    DockManager->addPerspective(PerspectiveName);
    QSignalBlocker Blocker(PerspectiveComboBox);
    PerspectiveComboBox->clear();
    PerspectiveComboBox->addItems(DockManager->perspectiveNames());
    PerspectiveComboBox->setCurrentText(PerspectiveName);

    QSettings Settings("Settings.ini", QSettings::IniFormat);
    DockManager->savePerspectives(Settings);
}

//============================================================================
void MainWindow::create_perspective_ui()
{
    SavePerspectiveAction = new QAction("Create Perspective", this);
    connect(SavePerspectiveAction, SIGNAL(triggered()), SLOT(save_perspective()));
    PerspectiveListAction = new QWidgetAction(this);
    PerspectiveComboBox = new QComboBox(this);
    PerspectiveComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    PerspectiveComboBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(PerspectiveComboBox, SIGNAL(currentTextChanged(const QString&)),
        DockManager, SLOT(openPerspective(const QString&)));
    PerspectiveListAction->setDefaultWidget(PerspectiveComboBox);

    ui->toolBar->addSeparator();
    ui->toolBar->addAction(PerspectiveListAction);
    ui->toolBar->addAction(SavePerspectiveAction);
}

//============================================================================
AgisResult<bool> MainWindow::save_state()
{
    // allow to save to any env, i.e. a folder in the envs folder
    QString exePath = QCoreApplication::applicationDirPath();
    fs::path exe_parent_dir_path(exePath.toStdString());
    auto env_parent_path = exe_parent_dir_path / "envs";

    fs::path env_path;
    QString dirPath = QFileDialog::getExistingDirectory(this,
        tr("Select Directory"),
        QString::fromStdString(env_parent_path.string()));
    if (!dirPath.isEmpty()) {
        auto str_path = dirPath.toStdString();
        env_path = fs::path(str_path);

        // make sure it is valid dir
        if (!fs::is_directory(str_path))
        {
            QMessageBox::critical(this, tr("Error"), tr("Invalid directory selected."));
            return AgisResult<bool>(AGIS_EXCEP("Invalid directory selected."));
        }

        // Check to see if the directory is empty
        bool isDirectoryEmpty = fs::is_empty(str_path);
        if (!isDirectoryEmpty) {
            // Prompt the user
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("Are you sure you want to overwrite previous env?");
            msgBox.setInformativeText("This will overwrite env json and all abstract flow graphs.");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            if (msgBox.exec() == QMessageBox::No) {
                return AgisResult<bool>(true);
            }
        }
    }
    else {
        QMessageBox::critical(this, "Error", "no folder selected");
        return AgisResult<bool>(AGIS_EXCEP("no folder selected"));
    }
    // extract the name of the selected env
    fs::path lastDir = env_path.filename();
    this->nexus_env.set_env_name(exePath.toStdString(), lastDir.string());

    auto settings_path = this->nexus_env.get_env_path() / "Settings.ini";
    QSettings Settings(QString::fromStdString(settings_path.string()), QSettings::IniFormat);
    Settings.setValue("mainWindow/Geometry", this->saveGeometry());
    Settings.setValue("mainWindow/State", this->saveState());
    Settings.setValue("mainWindow/DockingState", DockManager->saveState());
    
    json j;
    // Save the open widgets and the Nexus env state
    j["widgets"] = this->DockManager->save_widgets();
    this->nexus_env.save_env(j);
    return AgisResult<bool>(true);
}


//============================================================================
std::optional<fs::path> get_editor_by_id(nlohmann::json const& open_editors, int id)
{
    for (const auto& editor : open_editors)
    {
        if (editor["widget_id"] == id)
        {
            std::string p = editor["open_file"];
            return fs::path(p);
        }
    }
    return std::nullopt;
}


//============================================================================
AgisResult<bool> MainWindow::restore_exchanges(json const& j)
{
    // Restore Nexus env from the given json
    auto startTime = std::chrono::high_resolution_clock::now();
    AGIS_DO_OR_RETURN(this->nexus_env.restore_exchanges(j), bool);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    qDebug() << "EXCHANGES RESTORE COMPLETE IN " << QString::number(durationMs) << " Ms";
    return AgisResult<bool>(true);
}


//============================================================================
AgisResult<bool> MainWindow::restore_portfolios(json const& j)
{
    // Restore Nexus env from the given json
    auto startTime = std::chrono::high_resolution_clock::now();
    AGIS_DO_OR_RETURN(this->nexus_env.restore_portfolios(j), bool);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    qDebug() << "PORTFOLIO RESTORE COMPLETE IN " << QString::number(durationMs) << " Ms";
    return AgisResult<bool>(true);
}


//============================================================================
AgisResult<bool> MainWindow::restore_editors(json const & j)
{
    auto& editors = j["open_editors"];

    // Open files for the text edit widgets
    for (auto DockWidget : this->DockManager->get_widgets().values())
    {
        if (DockWidget->get_widget_type() != WidgetType::Editor)
        {
            continue;
        }
        auto editor = qobject_cast<TextEdit*>(DockWidget->widget());
        auto editor_id = DockWidget->get_id();
        auto open_file = get_editor_by_id(editors, editor_id);

        if (open_file.has_value())
        {
            auto q_open_file = QString::fromStdString(open_file.value().string());
            if (q_open_file == "")
            {
                continue;
            }
            editor->load(q_open_file);
        }
    }
    return AgisResult<bool>(true);
}


//============================================================================
void MainWindow::restore_state()
{
    auto startTime = std::chrono::high_resolution_clock::now();
    //this->center_progress_bar();
    ProgressBar->setMinimum(0);
    ProgressBar->setMaximum(9);
    ProgressBar->setValue(0);
    qDebug() << "==== Restoring state ====";
    // Load in env settings
    auto env_settings = this->nexus_env.get_env_settings_path();
    std::ifstream env_settings_file(env_settings.string());

    if (!env_settings_file.is_open()) {
        QMessageBox::critical(nullptr, "Error", "Failed to find env state");
        return;
    }
    ProgressBar->setValue(1);
    std::string jsonString((std::istreambuf_iterator<char>(env_settings_file)), std::istreambuf_iterator<char>());
    json j;
    try {
         j = nlohmann::json::parse(jsonString);
    }
    catch (std::exception& e) {
		QMessageBox::critical(nullptr, "Error", "Failed to parse env state");
		return;
	}
    ProgressBar->setValue(2);

    // Reset window geometry
    auto gui_settings_path = this->nexus_env.get_env_path() / "Settings.ini";
    QSettings Settings(QString::fromStdString(gui_settings_path.string()), QSettings::IniFormat);

    qDebug() << "==== Restoring geometry ====";
    this->restoreGeometry(Settings.value("mainWindow/Geometry").toByteArray());
    ProgressBar->setValue(3);
    qDebug() << "==== Restoring state ====";
    this->restoreState(Settings.value("mainWindow/State").toByteArray());
    ProgressBar->setValue(4);
    

    // restore the Nexus hydra env
    this->nexus_env.clear();
    NEXUS_DO_OR_INTERUPT(this->restore_exchanges(j));
    ProgressBar->setValue(5);
    NEXUS_DO_OR_INTERUPT(this->restore_portfolios(j));
    ProgressBar->setValue(6);
    NEXUS_DO_OR_INTERUPT(this->nexus_env.restore_strategies(j));
    ProgressBar->setValue(7);
  
    // Restore widgets
    NEXUS_DO_OR_INTERUPT(this->restore_editors(j));
    ProgressBar->setValue(8);
    this->DockManager->restore_widgets(j);
    ProgressBar->setValue(9);


    // Restore dock manager state
    qDebug() << "==== Restoring docking manager ====";
    bool dock_res = DockManager->restoreState(Settings.value("mainWindow/DockingState").toByteArray());
    if (!dock_res)
    {
        QMessageBox::critical(nullptr, "Error", "Failed to restore state");
        return;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
    qDebug() << "STATE RESTORE COMPLETE IN " << QString::number(durationMs) << " Second";
    qDebug() << "==== State Restored ====";
}


//============================================================================
void MainWindow::onViewVisibilityChanged(bool visible)
{
    auto DockWidget = qobject_cast<ads::CDockWidget*>(sender());
    if (!DockWidget)
    {
        return;
    }
}


//============================================================================
void MainWindow::onViewToggled(bool open)
{
    auto DockWidget = qobject_cast<ads::CDockWidget*>(sender());
    if (!DockWidget)
    {
        return;
    }
    //qDebug() << DockWidget->objectName() << " viewToggled(" << Open << ")";
}

//============================================================================
void MainWindow::onFileDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    QAbstractItemModel const* model = index.model();

    if (QFileSystemModel const* fileSystemModel = dynamic_cast<QFileSystemModel const*>(model))
    {
        // Get the previous editor, if we fail to create the new one restore to this
        auto last_editor = this->LastDockedEditor;

        QString file_path = fileSystemModel->filePath(index);
        std::filesystem::path pathToCheck(file_path.toStdString());
        if (std::filesystem::is_directory(pathToCheck)) {
            return;
        }
        if (this->nexus_env.get_editor(file_path))
        {
            QMessageBox::critical(this, "Error", "File is already open");
            return;
        }

        auto a = new QAction("Create Docked Editor");
        a->setProperty("Floating", false);
        if (!LastDockedEditor)
        {
            a->setProperty("Tabbed", false);
        }
        connect(a, &QAction::triggered, this, &MainWindow::create_editor);
        a->trigger();

        // Get the editor that was just created and attempt to fload the file that was clicked
        auto editor = dynamic_cast<TextEdit*>(LastDockedEditor->widget());
        if (!editor->load(file_path))
        {
            LastDockedEditor->closeDockWidget();
            this->LastDockedEditor = last_editor;
            this->nexus_env.remove_editor(file_path);
        }
    }
}

//============================================================================
void MainWindow::on_editor_close_requested()
{
    auto DockWidget = qobject_cast<ads::CDockWidget*>(sender());
    auto text_edit = qobject_cast<TextEdit*>(DockWidget->widget());

    // Allow text edit to save if any changes made. If returned true close widget, save successful
    if (text_edit->maybeSave())
    {
        auto file_name = text_edit->get_file_name();
        this->nexus_env.remove_editor(file_name);
        DockWidget->closeDockWidget();
    }
}


//============================================================================
void MainWindow::on_node_editor_close_request()
{
    auto DockWidget = qobject_cast<ads::CDockWidget*>(sender());
    auto node_editor = qobject_cast<NexusNodeEditor*>(DockWidget->widget());
    auto strategy = node_editor->get_strategy_id();
    this->nexus_env.remove_node_editor(strategy);
    DockWidget->closeDockWidget();
}

//============================================================================
void MainWindow::on_widget_focus(ads::CDockWidget* old, ads::CDockWidget* now)
{
}

//============================================================================
void MainWindow::on_actionSaveState_triggered(bool)
{
    save_state();
}


//============================================================================
void MainWindow::on_actionRestoreState_triggered(bool)
{
    restore_state();
}


//============================================================================
void MainWindow::on_new_portfolio_request(
    const QModelIndex& parentIndex,
    const QString& portfolio_id,
    const QString& starting_cash)
{
    auto res = this->nexus_env.new_portfolio(
        portfolio_id.toStdString(),
        starting_cash.toStdString()
    );
    if (res != NexusStatusCode::Ok){
        QMessageBox::critical(this, "Error", "Failed to create portfolio");
    }
    else
    {
        emit new_portfolio_accepeted(parentIndex, portfolio_id);
    }
}


//============================================================================
void MainWindow::on_new_strategy_requested(
    const QModelIndex & parentIndex,
    const QString& portfolio_id,
    const QString& strategy_id,
    const QString& allocation,
    AgisStrategyType strategy_type
)
{
    qDebug() << "NEW STRATEGY REQUESTED";
    NexusStatusCode res;
    try {
        res = this->nexus_env.new_strategy(
            portfolio_id.toStdString(),
            strategy_id.toStdString(),
            allocation.toStdString(),
            strategy_type
        );
    }
    catch (std::exception& e)
	    {
		QMessageBox::critical(this, "Error", e.what());
		return;
	}
    if (res != NexusStatusCode::Ok) {
        QMessageBox::critical(this, "Error", "Failed to create strategy");
    }
    else
    {
        qDebug() << "NEW STRATEGY ACCEPETED";

        // by adding a new strategy we invalidated existing pointers to strategies.
        // reset all pointers to strategies 
        for (auto nexus_widget : this->DockManager->get_widgets())
        {
            if (nexus_widget->get_widget_type() == WidgetType::NodeEditor)
            {
                auto child = nexus_widget->widget();
                NexusNodeEditor* asset_child = static_cast<NexusNodeEditor*>(child);
                
                auto strategy_id = asset_child->get_strategy_id();
                auto strategy_ref = this->nexus_env.__get_strategy(strategy_id).value();
                asset_child->__set_strategy(strategy_ref);
            }
        }
        emit new_strategy_accepeted(parentIndex, strategy_id);
    }
}


//============================================================================
void MainWindow::on_strategy_remove_requested(const QModelIndex& parentIndex, const QString& strategy_id)
{
	auto res = this->nexus_env.remove_strategy(strategy_id.toStdString());
	if (res != NexusStatusCode::Ok) {
		QMessageBox::critical(this, "Error", "Failed to remove strategy");
	}
	else
	{
        // check which strategy type being removed
        auto strategy = this->nexus_env.__get_strategy(strategy_id.toStdString());

        if (strategy.value()->get_strategy_type() == AgisStrategyType::FLOW) {
            this->nexus_env.remove_node_editor(strategy_id.toStdString());
            qDebug() << "STRATEGY" << strategy_id << " REMOVED";
            emit remove_strategy_accepted(parentIndex);
        }
	}
}


//============================================================================
void MainWindow::on_new_exchange_request(const QModelIndex& parentIndex, 
    const QString& exchange_id,
    const QString& source,
    const QString& freq,
    const QString& dt_format,
    std::optional<MarketAsset> market_asset)
{
    auto res = this->nexus_env.new_exchange(
        exchange_id.toStdString(),
        source.toStdString(),
        freq.toStdString(),
        dt_format.toStdString(),
        market_asset);
    if (res.is_exception())
    {
        QMessageBox::critical(this, "Error", QString::fromStdString(res.get_exception()));
    }
    else
    {
        emit new_exchange_accepted(parentIndex, exchange_id);
    }
}


//============================================================================
void MainWindow::on_remove_portfolio_request(const QString& name, const QModelIndex& parentIndex)
{
    auto res = this->nexus_env.remove_portfolio(name.toStdString());
    if (res != NexusStatusCode::Ok)
    {
        QMessageBox::critical(this, "Error", "Failed to remove portfolio");
    }
    else { emit remove_portfolio_accepted(parentIndex); }
}

//============================================================================
void MainWindow::on_remove_exchange_request(const QString& name, const QModelIndex& parentIndex)
{
    auto res = this->nexus_env.remove_exchange(name.toStdString());
    if (res != NexusStatusCode::Ok)
    {
        QMessageBox::critical(this, "Error", "Failed to remove exchange");
    }
    else { emit remove_exchange_accepted(parentIndex);}
}

//============================================================================
void MainWindow::on_new_asset_window_request(const QString& name)
{
    auto _sender = sender();
    _sender->setProperty("Floating", false);
    _sender->setProperty("Tabbed", false);

    auto DockWidget = this->create_asset_widget(name);
    this->place_widget(DockWidget, sender());
}

//============================================================================
void MainWindow::on_new_portfolio_window_request(const QString& name)
{
    auto _sender = sender();
    _sender->setProperty("Floating", false);
    _sender->setProperty("Tabbed", false);

    auto DockWidget = this->create_portfolio_widget(name);
    this->place_widget(DockWidget, sender());
}


//============================================================================
void MainWindow::on_new_node_editor_request(const QString& name)
{
    auto _sender = sender();
    _sender->setProperty("Floating", false);
    _sender->setProperty("Tabbed", false);

    // verify double clicked on a flow strategy
    auto strategy = this->nexus_env.__get_strategy(name.toStdString());
    if(!strategy.has_value()) NEXUS_INTERUPT("failed to find strategy being toggled");
    if (strategy.value()->get_strategy_type() != AgisStrategyType::FLOW) {
        QMessageBox::critical(this, "Error", "Only flow strategies can be edited");
		return;
    }

    auto DockWidget = this->create_node_editor_widget(name);
    if (!DockWidget) { return; }
    this->place_widget(DockWidget, sender());
}


//============================================================================
void MainWindow::on_strategy_toggle(const QString& name, bool toggle)
{
    //check if strategy exists
    auto strategy = this->nexus_env.__get_strategy(name.toStdString());
    if (!strategy.has_value()) NEXUS_INTERUPT("failed to find strategy being toggled");

    //check if strategy is already running
    if (strategy.value()->__is_live() == toggle) NEXUS_INTERUPT("attempted mismatch toggle");

    strategy.value()->set_is_live(toggle);
    qDebug() << "Strategy" << name << "toggled " << toggle;
}


//============================================================================
void MainWindow::closeEvent(QCloseEvent* event)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Closing Nexus");
    msgBox.setText("Save State?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel); // Set default to Cancel

    int Result = msgBox.exec();
    if (QMessageBox::Yes == Result)
    {
        auto res = this->save_state();
        if (res.is_exception()) event->ignore();
    }
    else if (Result == QMessageBox::No) {
        // Delete dock manager here to delete all floating widgets. This ensures
        // that all top level windows of the dock manager are properly closed
        DockManager->deleteLater();
        QMainWindow::closeEvent(event);
    }
    else {
        event->ignore();
        return;
    }
}

//============================================================================ 
void MainWindow::extract_flow_graphs()
{
    // for all open node editors, extract the current lambda
    for (auto nexus_widget : this->DockManager->get_widgets())
    {
        if (nexus_widget->get_widget_type() == WidgetType::NodeEditor)
        {
            auto child = nexus_widget->widget();
            NexusNodeEditor* asset_child = static_cast<NexusNodeEditor*>(child);
            auto strategy_id = asset_child->get_strategy_id();

            auto strategy_ptr = this->nexus_env.__get_strategy(strategy_id).value();
            AbstractAgisStrategy* strategy = static_cast<AbstractAgisStrategy*>(strategy_ptr);
            strategy->extract_ev_lambda();
        }
    }
}


//============================================================================
void MainWindow::__run_lambda()
{
    this->ProgressBar->setValue(0);
    this->ProgressBar->setMaximum(6);

    this->extract_flow_graphs();
    QEventLoop eventLoop;
    ProgressBar->setValue(1);
    QFuture<std::variant<long long, std::string>> future = QtConcurrent::run([this, &eventLoop]() -> std::variant<long long, std::string> {
        try {
            // reset hydra to start of sim
            this->nexus_env.__reset();

            qDebug() << "BEGINNING HYDRA RUN" << QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss.zzzzzz");
            auto startTime = std::chrono::high_resolution_clock::now();

            // Long-running operation that may block the CPU
            auto res = this->nexus_env.__run();
            if (res.is_exception()) {
                throw std::runtime_error(res.get_exception().c_str());
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            qDebug() << "HYDRA RUN COMPLETE" << QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss.zzzzzz");

            // Unblock the event loop when the operation is completed
            eventLoop.quit();
            return durationMs;
        }
        catch (const std::exception& ex) {
            return std::string(ex.what());
        }
        });
    // Block the event loop until the long-running operation is completed
    //NEXUS_TRY(future.waitForFinished();) // Wait for the future to finish before getting the result
    
    long long durationMs;
    std::variant<long long, std::string> res = future.result();
    ProgressBar->setValue(3);
    if (std::holds_alternative<std::string>(res))
    {
        NEXUS_INTERUPT(std::get<std::string>(res));
    }
    else
    {
        durationMs = std::get<long long>(res);
    }

    double cps = this->nexus_env.get_candle_count() / (durationMs / 1000.0f);
    size_t rows = this->nexus_env.get_hydra()->__get_dt_index().size();
    double ms_per_row = durationMs / static_cast<double>(rows);

    // Create a QLocale instance with the desired number formatting settings
    QLocale locale(QLocale::English);

    // Format cps with commas and a decimal point
    QString cpsFormatted = locale.toString(cps, 'f', 2);
    QString ms_per_row_formatted = locale.toString(ms_per_row, 'f', 5);

    auto msg = "Execution time: " + QString::number(durationMs) + " ms\n";
    msg += "Candles per second: " + cpsFormatted + "\n";
    msg += "Ms per row: " + ms_per_row_formatted + "\n";

    // save the history and notify the UI that new hydra run has completed then
    // analyze the portfolio historys
    ProgressBar->setValue(4);
    this->nexus_env.__save_history();
    ProgressBar->setValue(5);
    emit new_hydra_run();
    ProgressBar->setValue(6);
    QMessageBox::information(nullptr, "Execution Time", msg, QMessageBox::Ok);
}


//============================================================================
void MainWindow::__run_compile()
{
    NEXUS_TRY(this->nexus_env.__compile());
}


//============================================================================
void MainWindow::__run_link()
{
    NEXUS_TRY(this->nexus_env.__link());
    auto portfolio_ids = this->nexus_env.get_portfolio_ids();
    this->portfolio_tree->relink_tree(portfolio_ids);
}


//============================================================================
void MainWindow::applyVsStyle()
{
    qDebug() << "INIT STYLE APPLY";
    QFile file("./style/vs_light.qss");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        QString stylesheet = stream.readAll();
        file.close();

        // Apply the stylesheet to the main window
        DockManager->setStyleSheet(stylesheet);
    }
    else
    { 
        throw std::runtime_error("Failed to load style sheet");
    }
    qDebug() << "INIT STYLE COMPLETE";
}
