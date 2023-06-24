#pragma once

#include "json.hpp"
#include <QTreeView>
#include <QMenu>
#include <QStandardItemModel>
#include <QPainter>
#include <NexusPopups.h>

using json = nlohmann::json;


class NexusTree : public QTreeView
{
    Q_OBJECT

public:
    explicit NexusTree(QWidget* parent = nullptr);
    void reset_tree();
    virtual void restore_tree(json const& j) = 0;

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void create_new_item(const QModelIndex& parentIndex) = 0;
    void remove_item(const QModelIndex& parentIndex);

    QStandardItemModel* model;

public slots:
    void handle_new_item_action();
    void new_item_accepted(const QModelIndex& parentIndex, const QString& name);
    void remove_item_accepeted(const QModelIndex& index);

signals:
    void remove_item_requested(const QString& name, const QModelIndex& parentIndex);
};


class ExchangeTree : public NexusTree
{
    Q_OBJECT

private:
    virtual void create_new_item(const QModelIndex& parentIndex) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    QStandardItem* root;

signals:
    void new_item_requested(const QModelIndex& parentIndex, 
        const QString& exchange_id,
        const QString& source,
        const QString& freq
    );

public:
    explicit ExchangeTree(QWidget* parent = nullptr);
    void restore_tree(json const& j) override;

};