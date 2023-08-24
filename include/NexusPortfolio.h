#pragma once
#include <QMainWindow>
#include <QWidget>
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

    void load(std::shared_ptr<Hydra> hydra_, std::string portfolio_id_) {
        this->hydra = hydra_;
        this->portfolio_id = portfolio_id_;
    };

private slots:
    void contextMenuRequest(QPoint pos) override;
    
    void plot_nlv();

private:
    std::shared_ptr<Hydra> hydra = nullptr;
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
    NexusPortfolioPlot* nexus_plot;

    std::string get_portfolio_id() { return this->portfolio_id; }

public slots:
    void on_new_hydra_run();

private:
    NexusEnv const* nexus_env;
    std::string portfolio_id;
};
