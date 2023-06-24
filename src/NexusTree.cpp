
#include <QContextMenuEvent>
#include <QInputDialog>

#include "NexusTree.h"

enum CustomRoles {
    ParentItemRole = Qt::UserRole + 1,
};

NexusTree::NexusTree(QWidget* parent) : QTreeView(parent)
{
    this->setObjectName("Exchanges");
    connect(this, &NexusTree::customContextMenuRequested, this, &NexusTree::handle_new_item_action);

    this->model = new QStandardItemModel(this);
    QStandardItem* rootItem = this->model->invisibleRootItem();
    
    QStandardItem* item1 = new QStandardItem("Exchanges");
    //item1->setFlags(item1->flags() & ~Qt::ItemIsSelectable); // Make item1 unselectable
    item1->setData(QVariant::fromValue(rootItem), ParentItemRole);  // Set the parent item using custom role
    rootItem->appendRow(item1);
    
    this->setModel(model);
}

void NexusTree::contextMenuEvent(QContextMenuEvent* event)
{
    QModelIndex index = this->indexAt(event->pos());
    if (index.isValid())
    {
        QStandardItemModel* model = static_cast<QStandardItemModel*>(this->model);
        QStandardItem* item = model->itemFromIndex(index);

        QMenu menu(this);
        QAction* addAction = new QAction("Add Item", this);
        connect(addAction, &QAction::triggered, [this, index]() { create_new_item(index); });
        menu.addAction(addAction);


        QAction* removeAction = new QAction("Delete Item", this);
        connect(removeAction, &QAction::triggered, [this, index]() { remove_item(index); });
        menu.addAction(removeAction);
        menu.exec(event->globalPos());
    }
}

void NexusTree::handle_new_item_action()
{
    // This slot is connected to the triggered signal of the New Item action
    // It is invoked when the New Item action is triggered via the context menu
    QContextMenuEvent* event = new QContextMenuEvent(QContextMenuEvent::Mouse, QPoint());
    contextMenuEvent(event);
}

void NexusTree::new_item_accepted(const QModelIndex& parentIndex, const QString& name)
{
    // New item has been accepeted by the main window
    QStandardItemModel* model = static_cast<QStandardItemModel*>(this->model);
    QStandardItem* parentItem = model->itemFromIndex(parentIndex);
    QStandardItem* newItem = new QStandardItem(name);
    newItem->setData(QVariant::fromValue(parentItem), ParentItemRole);  // Set the parent item using custom role
    parentItem->appendRow(newItem);
}

void NexusTree::remove_item(const QModelIndex& index)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model);
    QStandardItem* item = model->itemFromIndex(index);
    if (item && item != model->invisibleRootItem())
    {
        emit remove_item_requested(item->text(), index);
    }
}

void NexusTree::remove_item_accepeted(const QModelIndex& index)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model);
    QStandardItem* item = model->itemFromIndex(index);

    QStandardItem* parentItem = item->parent();
    parentItem->removeRow(item->row());
}

void ExchangeTree::create_new_item(const QModelIndex& parentIndex)
{
    NewExchangePopup* popup = new NewExchangePopup();
    if (popup->exec() == QDialog::Accepted)
    {
        auto exchange_id = popup->get_exchange_id();
        auto source = popup->get_source();
        auto freq = popup->get_freq();
        // Send signal to main window asking to create the new item
        emit new_item_requested(parentIndex, exchange_id, source, freq);
    }
}
