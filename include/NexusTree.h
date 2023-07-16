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
    virtual json to_json() const = 0;

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void create_new_item(const QModelIndex& parentIndex) = 0;
    void remove_item(const QModelIndex& parentIndex);

    QStandardItemModel* model;

public slots:
    void handle_new_item_action();
    virtual void new_item_accepted(const QModelIndex& parentIndex, const QString& name);
    void remove_item_accepeted(const QModelIndex& index);

signals:
    void remove_item_requested(const QString& name, const QModelIndex& parentIndex);
};


class DoubleClickEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit DoubleClickEventFilter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    bool eventFilter(QObject* obj, QEvent* event) override
    {
        if (event->type() == QEvent::MouseButtonDblClick)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                // Double-click occurred, emit signal with the item name
                emit itemDoubleClicked(obj->objectName());
                return true;  // Consume the event
            }
        }
        return false;  // Let the event propagate further
    }

signals:
    void itemDoubleClicked(const QString& itemName);
};


//============================================================================
class ExchangeTree : public NexusTree
{
    Q_OBJECT

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    /// <summary>
    /// Create a new exchange from params using the popup window
    /// </summary>
    /// <param name="parentIndex"></param>
    virtual void create_new_item(const QModelIndex& parentIndex) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    
    QStandardItem* root;
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

public:
    explicit ExchangeTree(QWidget* parent, std::shared_ptr<Hydra> const hydra);
    void restore_tree(json const& j) override;
    json to_json() const;
    void restore_ids(QStandardItem* newItem, QString exchange_id);
};