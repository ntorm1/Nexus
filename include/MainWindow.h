#pragma once
#include "NexusPch.h"
#include <unordered_map>
#include <QMainWindow>
#include <QtWidgets/QDockWidget>
#include <QComboBox>
#include <QWidgetAction>
#include <QPointer>
#include <QStandardItemModel>
#include <QThread>
#include <QObject>
#include <QProgressBar>
#include <QtConcurrent/QtConcurrent>

#include "DockManager.h"
#include "DockAreaWidget.h"
#include "DockWidget.h"

#include "QScintillaEditor.h"
#include "NexusEnv.h"
#include "NexusAsset.h"
#include "NexusNode.h"


class NexusDockManager;
class NexusSettings;
class NewExchangePopup;
class NexusWidgetFactory;

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class NexusWidgetFactory;
public:
    MainWindow(QWidget* parent = 0);
    ~MainWindow();

public slots:
    void about();

    void showErrorMessageBox(const QString& errorMessage)
    {
       
    }


signals:
    void new_hydra_run();
    void new_exchange_accepted(const QModelIndex& parentIndex, const QString name);
    void new_portfolio_accepeted(const QModelIndex& parentIndex, const QString name);
    void new_strategy_accepeted(const QModelIndex& parentIndex, const QString name);
    void remove_strategy_accepted(const QModelIndex& parentIndex);
    void remove_exchange_accepted(const QModelIndex& parentIndex);
    void remove_portfolio_accepted(const QModelIndex& parentIndex);


private slots:
    void save_perspective();
    void create_editor();
    void on_editor_close_requested();
    void on_node_editor_close_request();
    void on_widget_focus(ads::CDockWidget* old, ads::CDockWidget* now);
    void on_actionSaveState_triggered(bool);
    void on_actionRestoreState_triggered(bool);

    void on_new_portfolio_request(const QModelIndex& parentIndex,
        const QString& portfolio_id,
        const QString& starting_casj
    );
    void on_new_strategy_requested(const QModelIndex& parentIndex,
        const QString& portfolio_id,
        const QString& strategy_id,
        const QString& allocation,
        AgisStrategyType type
    );
    void on_strategy_remove_requested(const QModelIndex& parentIndex,
        const QString& strategy_id
    );
    void on_new_exchange_request(const QModelIndex& parentIndex,
        NewExchangePopup* popup
    );

    void on_remove_portfolio_request(const QString& name, const QModelIndex& parentIndex);
    void on_remove_exchange_request(const QString& name, const QModelIndex& parentIndex);
    void on_new_asset_window_request(const QString& name);
    void on_new_portfolio_window_request(const QString& name);
    void on_new_node_editor_request(const QString& name);
    void on_strategy_toggle(const QString& name, bool toggle);
    void on_settings_change(NexusSettings* settings);

protected:
    virtual void closeEvent(QCloseEvent* event) override;

private:
    void restore_state();
    AgisResult<bool> restore_editors(rapidjson::Document const& j);
    std::expected<bool, AgisException> restore_exchanges(rapidjson::Document const& j);
    std::expected<bool, AgisException> restore_portfolios(rapidjson::Document const& j);

    AgisResult<bool> save_state();

    void setup_toolbar();
    void setup_help_menu();
    void setup_command_bar();
    void create_perspective_ui();

    void place_widget(ads::CDockWidget* docket_widget, QObject* Sender);
    void place_widget(ads::CDockWidget* docket_widget, ads::CDockAreaWidget* dock_area);
    void on_settings_window_request();

    void onViewVisibilityChanged(bool open);
    void onViewToggled(bool open);
    void onFileDoubleClicked(const QModelIndex& index);
    void extract_flow_graphs();
    void applyVsStyle();

    QAction*        SavePerspectiveAction = nullptr;
    QWidgetAction*  PerspectiveListAction = nullptr;
    QComboBox*      PerspectiveComboBox = nullptr;
    QProgressBar*   ProgressBar = nullptr;

    QPointer<ads::CDockWidget> LastDockedEditor;
    QPointer<ads::CDockWidget> LastCreatedFloatingEditor;

    NexusEnv                nexus_env;
    Ui::MainWindow*         ui;
    ExchangeTree*           exchange_tree;
    PortfolioTree*          portfolio_tree;

    NexusDockManager*       DockManager;
    ads::CDockAreaWidget*   StatusDockArea;
    ads::CDockWidget*       TimelineDockWidget;

    void __run();
    void __run_lambda();
    void __run_compile();
    void __run_link();

public:
    ads::CDockWidget* create_console_widget();
    ads::CDockWidget* create_editor_widget();
    ads::CDockWidget* create_asset_widget(const QString& asset_id);
    ads::CDockWidget* create_portfolio_widget(const QString& portfolio_id);
    ads::CDockWidget* create_node_editor_widget(const QString& strategy_id);
};
