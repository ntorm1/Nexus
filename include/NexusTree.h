#pragma once

#include <QTreeView>
#include <QMenu>
#include <QStandardItemModel>
#include <QPainter>

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
    void create_new_item(const QModelIndex& parentIndex);
    void remove_item(const QModelIndex& parentIndex);

    QStandardItemModel* model;

signals:
    void new_item_requested(const QString& name, const QModelIndex& parentInde);
    void remove_item_requested(const QString& name);
};