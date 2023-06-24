#pragma once

#include <QTreeView>
#include <QMenu>
#include <QStandardItemModel>
#include <QPainter>
#include <NexusPopups.h>

class NexusTree : public QTreeView
{
    Q_OBJECT

public:
    explicit NexusTree(QWidget* parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void handle_new_item_action();

private:
    virtual void create_new_item(const QModelIndex& parentIndex) = 0;
    void remove_item(const QModelIndex& parentIndex);

    QStandardItemModel* model;

public slots:
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

signals:
    void new_item_requested(const QModelIndex& parentIndex, 
        const QString& exchange_id,
        const QString& source,
        const QString& freq
    );
};