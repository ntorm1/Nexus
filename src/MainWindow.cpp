#include <QTime>
#include <QLabel>
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



MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->nexus_env = NexusEnv();
    this->ui->setupUi(this);
    ads::CDockComponentsFactory::setFactory(new CCustomComponentsFactory());

    // Create the dock manager. Because the parent parameter is a QMainWindow
    // the dock manager registers itself as the central widget.
    CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    CDockManager::setAutoHideConfigFlags(CDockManager::DefaultAutoHideConfig);

    DockManager = new CDockManager(this);
    connect(DockManager, &CDockManager::focusedDockWidgetChanged,
        this, &MainWindow::on_widget_focus);

    setWindowTitle(tr("Hyperion"));
    setup_help_menu();
    setup_toolbar();

    // create other dock widgets
    auto FileSystemWidget = create_file_system_tree_widget();
    FileSystemWidget->setFeature(ads::CDockWidget::DockWidgetFloatable, false);
    FileSystemWidget->setFeature(ads::CDockWidget::DockWidgetMovable, false);
    FileSystemWidget->setFeature(ads::CDockWidget::DockWidgetClosable, false);
    this->DockManager->addAutoHideDockWidget(ads::SideBarLeft, FileSystemWidget);

    auto TextEditWidget = create_editor_widget();
    this->DockManager->addDockWidget(ads::RightDockWidgetArea, TextEditWidget);
    this->LastDockedEditor = TextEditWidget;

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

ads::CDockWidget* MainWindow::create_file_system_tree_widget()
{
    static int FileSystemCount = 0;
    QTreeView* w = new QTreeView();
    w->setFrameShape(QFrame::NoFrame);
    QFileSystemModel* m = new QFileSystemModel(w);
    m->setRootPath(QDir::currentPath());
    w->setModel(m);
    w->setRootIndex(m->index(QDir::currentPath()));
    ads::CDockWidget* DockWidget = new ads::CDockWidget(QString("Files")
        .arg(FileSystemCount++));
    DockWidget->setWidget(w);
    DockWidget->setIcon(svgIcon(".images/folder_open.svg"));
    ui->menuView->addAction(DockWidget->toggleViewAction());
    // We disable focus to test focus highlighting if the dock widget content
    // does not support focus
    w->setFocusPolicy(Qt::NoFocus);


    connect(w, &QAbstractItemView::doubleClicked, this, &MainWindow::onFileDoubleClicked);

    return DockWidget;
}


ads::CDockWidget* MainWindow::create_editor_widget()
{
    // Build the new TextEdit widget
    static int EditorCount = 0;
    TextEdit* w = new TextEdit(&this->nexus_env);

    // Register the new editor with the current nexus env
    this->nexus_env.new_editor(w);

    // Add the new TextEdit widget to a DockWidget
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
        this->LastCreatedFloatingEditor = DockWidget;
        this->LastDockedEditor.clear();
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
    this->LastDockedEditor = DockWidget;
}

void MainWindow::setup_toolbar()
{
    ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->toolBar->addAction(ui->actionSaveState);
    ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->actionSaveState->setIcon(svgIcon("./images/save.svg"));
    ui->toolBar->addAction(ui->actionRestoreState);
    ui->actionRestoreState->setIcon(svgIcon("./images/restore.svg"));
    ui->toolBar->addSeparator();

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

//============================================================================
void MainWindow::setup_help_menu()
{
    QMenu* helpMenu = new QMenu(tr("&Help"), this);
    menuBar()->addMenu(helpMenu);

    helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
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
void MainWindow::save_state()
{
    QSettings Settings("C:\\Users\\natha\\OneDrive\\Desktop\\C++\\Nexus\\x64\\Debug\\Settings.ini", QSettings::IniFormat);
    Settings.setValue("mainWindow/Geometry", this->saveGeometry());
    Settings.setValue("mainWindow/State", this->saveState());
    Settings.setValue("mainWindow/DockingState", DockManager->saveState());
}


//============================================================================
void MainWindow::restore_state()
{
    QSettings Settings("C:\\Users\\natha\\OneDrive\\Desktop\\C++\\Nexus\\x64\\Debug\\Settings.ini", QSettings::IniFormat);
    this->restoreGeometry(Settings.value("mainWindow/Geometry").toByteArray());
    this->restoreState(Settings.value("mainWindow/State").toByteArray());

    bool res = DockManager->restoreState(Settings.value("mainWindow/DockingState").toByteArray());
    if (!res)
    {
        QMessageBox::critical(nullptr, "Error", "Failed to restore state");
        return;
    }
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
void MainWindow::onFileDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    QAbstractItemModel const* model = index.model();

    if (QFileSystemModel const* fileSystemModel = dynamic_cast<QFileSystemModel const*>(model))
    {
        // Get the previous editor, if we fail to create the new one restore to this
        auto last_editor = this->LastDockedEditor;

        //Create a new TextEdit widget and load in the file that was clicked
        QString file_path = fileSystemModel->filePath(index);
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
