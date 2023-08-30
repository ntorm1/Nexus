#pragma once
#include <unordered_map>
#include <QMainWindow>
#include <QWidget>
#include <qaction.h>

#include "DockWidget.h"

#include "NexusEnv.h"
#include "NexusPlot.h"
#include "Hydra.h"

namespace Ui {
    class NexusPortfolio;
}

class NexusPortfolio;
class NexusPortfolioPlot;


class NexusPortfolioPlot : public NexusPlot
{
    Q_OBJECT
public:
    explicit NexusPortfolioPlot(QWidget* parent_);
    ~NexusPortfolioPlot() = default;

    void load(HydraPtr hydra_, std::string portfolio_id_) {
        this->hydra = hydra_;
        this->portfolio_id = portfolio_id_;
    };

    void load_portfolio(NexusPortfolio* portfolio) {
        this->nexus_portfolio = portfolio;
    }

    /// <summary>
    /// List of columns currently plotted
    /// </summary>
    std::vector<std::string> plotted_graphs;

protected slots:
    void removeAllGraphs() override;
    void removeSelectedGraph() override;

private slots:
    void contextMenuRequest(QPoint pos) override;
    void add_plot(QString const& name);

private:
    HydraPtr hydra = nullptr;
    NexusPortfolio* nexus_portfolio = nullptr;
    std::string portfolio_id;
};


class NexusPortfolio : public QMainWindow
{
    Q_OBJECT
public:
    NexusPortfolio(
        NexusEnv const* nexus_env,
        ads::CDockWidget* DockWidget,
        std::string portfolio_id,
        QWidget* parent = nullptr
    );

    Ui::NexusPortfolio* ui;
    ads::CDockWidget* DockWidget;

    QTableView* table_view;
    QWidget* stats_widget;
    QMenuBar* menu_bar;

    NexusPortfolioPlot* nexus_plot;

    std::vector<std::string> get_plotted_graphs() const { return this->nexus_plot->plotted_graphs; }
    std::string get_portfolio_id() { return this->portfolio_id; }
    std::vector<std::string> get_selected_strategies() const;

public slots:
    void on_new_hydra_run();

private:
    std::unordered_map<std::string, QAction*> strategies_checkboxes;

    void set_up_strategies_menu();
    NexusEnv const* nexus_env;
    std::string portfolio_id;
};
