#pragma once

#include <QMainWindow>
#include <QtWidgets/QDockWidget>
#include <QComboBox>
#include <QWidgetAction>
#include <QPointer>

#include "DockManager.h"
#include "DockAreaWidget.h"
#include "DockWidget.h"

#include "CodeEditor.h"
#include "NexusEnv.h"


QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = 0);
    ~MainWindow();

public slots:
    void about();

private slots:
    void save_perspective();
    void create_editor();
    void on_editor_close_requested();
    void on_widget_focus(ads::CDockWidget* old, ads::CDockWidget* now);
    void on_actionSaveState_triggered(bool);
    void on_actionRestoreState_triggered(bool);

protected:
    virtual void closeEvent(QCloseEvent* event) override;

private:
    static int widget_counter;
    void restore_state();
    void save_state();

    void setup_toolbar();
    void setup_help_menu();
    void create_perspective_ui();

    ads::CDockWidget* create_console_widget();
    ads::CDockWidget* create_editor_widget();
    ads::CDockWidget* create_file_system_tree_widget();

    void onViewVisibilityChanged(bool open);
    void onViewToggled(bool open);
    void onFileDoubleClicked(const QModelIndex& index);

    void applyVsStyle();

    QAction*        SavePerspectiveAction = nullptr;
    QWidgetAction*  PerspectiveListAction = nullptr;
    QComboBox*      PerspectiveComboBox = nullptr;

    QPointer<ads::CDockWidget> LastDockedEditor;
    QPointer<ads::CDockWidget> LastCreatedFloatingEditor;

    NexusEnv                nexus_env;
    Ui::MainWindow*         ui;

    ads::CDockManager*      DockManager;
    ads::CDockAreaWidget*   StatusDockArea;
    ads::CDockWidget*       TimelineDockWidget;

};