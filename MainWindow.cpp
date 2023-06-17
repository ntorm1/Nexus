#include <QAction>
#include <QtWidgets>
#include <QWidgetAction>
#include <QLabel>
#include <QCalendarWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QTableWidget>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QInputDialog>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QToolBar>
#include "MainWindow.h"

#include "ui_MainWindow.h"

#include "AutoHideDockContainer.h"
#include "DockAreaWidget.h"
#include "DockAreaTitleBar.h"
#include "DockAreaTabBar.h"
#include "FloatingDockContainer.h"
#include "DockComponentsFactory.h"

using namespace ads;

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * Helper function to create an SVG icon
 */
static QIcon svgIcon(const QString& File)
{
    // This is a workaround, because in item views SVG icons are not
    // properly scaled and look blurry or pixelate
    QIcon SvgIcon(File);
    SvgIcon.addPixmap(SvgIcon.pixmap(92));
    return SvgIcon;
}


MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create the dock manager. Because the parent parameter is a QMainWindow
    // the dock manager registers itself as the central widget.
    CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
    CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    CDockManager::setAutoHideConfigFlags(CDockManager::DefaultAutoHideConfig);

    DockManager = new CDockManager(this);
    connect(DockManager, &CDockManager::focusedDockWidgetChanged,
        this, &MainWindow::on_widget_focus);

    setWindowTitle(tr("Hyperion"));
    setup_help_menu();
    setup_toolbar();

    //CDockWidget* CentralDockWidget = create_editor_widget();
    //auto* CentralDockArea = DockManager->setCentralWidget(CentralDockWidget);
    //CentralDockArea->setAllowedAreas(DockWidgetArea::OuterDockAreas);

    // create other dock widgets
    QTableWidget* table = new QTableWidget();
    table->setColumnCount(3);
    table->setRowCount(10);
    CDockWidget* TableDockWidget = new CDockWidget("Table 1");
    TableDockWidget->setWidget(table);
    TableDockWidget->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
    TableDockWidget->setMinimumSize(200, 150);
    const auto autoHideContainer = DockManager->addAutoHideDockWidget(SideBarLocation::SideBarLeft, TableDockWidget);
    autoHideContainer->setSize(480);
    ui->menuView->addAction(TableDockWidget->toggleViewAction());

    table = new QTableWidget();
    table->setColumnCount(5);
    table->setRowCount(1020);
    TableDockWidget = new CDockWidget("Table 2");
    TableDockWidget->setWidget(table);
    TableDockWidget->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
    TableDockWidget->resize(250, 150);
    TableDockWidget->setMinimumSize(200, 150);
    DockManager->addAutoHideDockWidget(SideBarLocation::SideBarLeft, TableDockWidget);
    ui->menuView->addAction(TableDockWidget->toggleViewAction());

    QTableWidget* propertiesTable = new QTableWidget();
    propertiesTable->setColumnCount(3);
    propertiesTable->setRowCount(10);
    CDockWidget* PropertiesDockWidget = new CDockWidget("Properties");
    PropertiesDockWidget->setWidget(propertiesTable);
    PropertiesDockWidget->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
    PropertiesDockWidget->resize(250, 150);
    PropertiesDockWidget->setMinimumSize(200, 150);
    //DockManager->addDockWidget(DockWidgetArea::RightDockWidgetArea, PropertiesDockWidget, CentralDockArea);
    //ui->menuView->addAction(PropertiesDockWidget->toggleViewAction());

    create_perspective_ui();
    applyVsStyle();
}


void MainWindow::about()
{
    QMessageBox::about(this, tr("About Syntax Highlighter"),
        tr("<p>The <b>Syntax Highlighter</b> example shows how " \
            "to perform simple syntax highlighting by subclassing " \
            "the QSyntaxHighlighter class and describing " \
            "highlighting rules using regular expressions.</p>"));
}

ads::CDockWidget* MainWindow::create_editor_widget()
{
    static int EditorCount = 0;
    TextEdit* w = new TextEdit();
    ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Editor %1").arg(EditorCount++));
    DockWidget->setWidget(w);
    DockWidget->setIcon(svgIcon("./images/edit.svg"));
    DockWidget->setFeature(ads::CDockWidget::CustomCloseHandling, true);

    return DockWidget;
}

void MainWindow::create_editor()
{
    QObject* Sender = sender();
    QVariant vFloating = Sender->property("Floating");
    bool Floating = vFloating.isValid() ? vFloating.toBool() : true;
    QVariant vTabbed = Sender->property("Tabbed");
    bool Tabbed = vTabbed.isValid() ? vTabbed.toBool() : true;

    auto DockWidget = this->create_editor_widget();
    DockWidget->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    DockWidget->setFeature(ads::CDockWidget::DockWidgetForceCloseWithArea, true);
    
    connect(DockWidget, SIGNAL(closeRequested()), SLOT(on_editor_close_requested()));

    // Creating a new floating dock widget
    if (Floating)
    {
        auto FloatingWidget = this->DockManager->addDockWidgetFloating(DockWidget);
        return;
    }

    // Get the editor area, either a docked editor, floating editor, or none
    ads::CDockAreaWidget* EditorArea;
    if (this->LastDockedEditor)
    {
        EditorArea= this->LastDockedEditor->dockAreaWidget();
    } 
    else if (this->LastCreatedFloatingEditor)
    {
        EditorArea= this->LastCreatedFloatingEditor->dockAreaWidget();
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
}

void MainWindow::setup_toolbar()
{
    ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);


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
}

void MainWindow::setup_help_menu()
{
    QMenu* helpMenu = new QMenu(tr("&Help"), this);
    menuBar()->addMenu(helpMenu);

    helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
}

void MainWindow::save_perspective()
{
    QString PerspectiveName = QInputDialog::getText(this, "Save Environment", "Enter unique name:");
    if (PerspectiveName.isEmpty())
    {
        return;
    }

    DockManager->addPerspective(PerspectiveName);
    QSignalBlocker Blocker(PerspectiveComboBox);
    PerspectiveComboBox->clear();
    PerspectiveComboBox->addItems(DockManager->perspectiveNames());
    PerspectiveComboBox->setCurrentText(PerspectiveName);
}

void MainWindow::create_perspective_ui()
{
    SavePerspectiveAction = new QAction("Create Environment", this);
    connect(SavePerspectiveAction, SIGNAL(triggered()), SLOT(savePerspective()));
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
void MainWindow::onViewVisibilityChanged(bool visible)
{
    auto DockWidget = qobject_cast<ads::CDockWidget*>(sender());
    if (!DockWidget)
    {
        return;
    }

    //qDebug() << DockWidget->objectName() << " visibilityChanged(" << Visible << ")";
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
void MainWindow::on_editor_close_requested()
{
    auto DockWidget = qobject_cast<ads::CDockWidget*>(sender());
    int Result = QMessageBox::question(this, "Close Editor", QString("Editor %1 "
        "contains unsaved changes? Would you like to close it?")
        .arg(DockWidget->windowTitle()));
    if (QMessageBox::Yes == Result)
    {
        DockWidget->closeDockWidget();
    }
}

//============================================================================
void MainWindow::on_widget_focus(ads::CDockWidget* old, ads::CDockWidget* now)
{
    if (now->floatingDockContainer())
    {
        this->LastCreatedFloatingEditor = now;
        this->LastDockedEditor.clear();
    }
    else
    {
        this->LastDockedEditor = now;
        this->LastCreatedFloatingEditor.clear();
    }
}

//============================================================================
void MainWindow::closeEvent(QCloseEvent* event)
{
    // Delete dock manager here to delete all floating widgets. This ensures
    // that all top level windows of the dock manager are properly closed
    DockManager->deleteLater();
    QMainWindow::closeEvent(event);
}

//============================================================================
void MainWindow::applyVsStyle()
{
    QFile file("vs_light.qss");
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
}
