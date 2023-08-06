#pragma once
#include "NexusPch.h"

#include <QMouseEvent>
#include <QApplication>
#include <QTreeView>
#include <QMenu>
#include <QStandardItemModel>
#include <QPainter>
#include <NexusPopups.h>

#include "Hydra.h"


class NexusTree : public QTreeView
{
    Q_OBJECT

public:
    explicit NexusTree(QWidget* parent = nullptr);
    void reset_tree();
    virtual void restore_tree(json const& j) = 0;
    virtual json to_json() const;

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void create_new_item(const QModelIndex& parentIndex) = 0;
    void remove_item(const QModelIndex& parentIndex);

    QStandardItemModel* model;
    QStandardItem* root;

public slots:
    void handle_new_item_action();
    virtual void new_item_accepted(const QModelIndex& parentIndex, const QString& name);
    void remove_item_accepeted(const QModelIndex& index);

signals:
    void remove_item_requested(const QString& name, const QModelIndex& parentIndex);
};


class PortfolioTree : public NexusTree
{
    Q_OBJECT

public:
    explicit PortfolioTree(QWidget* parent, std::shared_ptr<Hydra> const hydra);
    void restore_tree(json const& j) override;
    void relink_tree(std::vector<std::string> const& portfolios);
    void restore_strategies(QStandardItem* addedItem, QString portfolio_id);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;


private:
    /// <summary>
    /// Create a new Portfolio from params using the popup window
    /// </summary>
    /// <param name="parentIndex"></param>
    virtual void create_new_item(const QModelIndex& parentIndex) override;

    /// <summary>
    /// Create a new strategy underneath a portfolio
    /// </summary>
    /// <param name="parentIndex"></param>
    void create_new_strategy(const QModelIndex& parentIndex);

    /// <summary>
    /// Override context menu event to prevent nested portfolios
    /// </summary>
    /// <param name="event"></param>
    void contextMenuEvent(QContextMenuEvent* event) override;


    /// <summary>
    /// Parent hydra instance
    /// </summary>
    std::shared_ptr<Hydra> const hydra;

signals:
    void strategy_double_clicked(const QString& asset_id);
    void portfolio_double_clicked(const QString& portfolio_id);

    void new_item_requested(const QModelIndex& parentIndex,
        const QString& portfolio_id,
        const QString& starting_cash
    );
    void new_strategy_requested(const QModelIndex& parentIndex,
        const QString& portfolio_id,
        const QString& strategy_id,
        const QString& starting_cash
    );

public slots:
    void new_item_accepted(const QModelIndex& parentIndex, const QString& name) override;
};

//============================================================================
class ExchangeTree : public NexusTree
{
    Q_OBJECT

public:
    explicit ExchangeTree(QWidget* parent, std::shared_ptr<Hydra> const hydra);
    void restore_tree(json const& j) override;
    void restore_ids(QStandardItem* newItem, QString exchange_id);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    /// <summary>
    /// Create a new exchange from params using the popup window
    /// </summary>
    /// <param name="parentIndex"></param>
    virtual void create_new_item(const QModelIndex& parentIndex) override;
    
    /// <summary>
    /// Override context menu event method to prevent nested exchange
    /// </summary>
    /// <param name="event"></param>
    void contextMenuEvent(QContextMenuEvent* event) override;

    /// <summary>
    /// Parent hydra instance
    /// </summary>
    std::shared_ptr<Hydra> const hydra;

signals:
    void asset_double_click(const QString& asset_id);
    void new_item_requested(const QModelIndex& parentIndex, 
        const QString& exchange_id,
        const QString& source,
        const QString& freq,
        const QString& dt_format
    );

public slots:
    void new_item_accepted(const QModelIndex& parentIndex, const QString& name) override;
};