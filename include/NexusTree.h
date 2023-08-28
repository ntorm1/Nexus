#pragma once
#include "NexusPch.h"

#include <QMouseEvent>
#include <QApplication>
#include <QTreeView>
#include <QMenu>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <NexusPopups.h>

#include "Hydra.h"


class ToggleButtonDelegate : public QStyledItemDelegate {
public:
    ToggleButtonDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QStyleOptionViewItem adjustedOption = option;
        // Draw the checkbox and the text conditionally
        if (index.column() == 0 && index.parent().isValid() && index.parent().parent().isValid()) {
            adjustedOption.showDecorationSelected = false;  // Prevent text highlighting
            QStyleOptionButton buttonOption;
            buttonOption.rect = option.rect;
            buttonOption.state = index.data(Qt::CheckStateRole).toInt() == Qt::Checked ? QStyle::State_On : QStyle::State_Off;

            // Calculate the rect for the checkbox and the text
            QRect checkBoxRect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &buttonOption, nullptr);
            QRect textRect = adjustedOption.rect.adjusted(checkBoxRect.width() + 2, 0, 0, 0); // Adjust for spacing

            // Draw the checkbox and text
            QApplication::style()->drawControl(QStyle::CE_CheckBox, &buttonOption, painter);
            painter->drawText(textRect, Qt::AlignVCenter, index.data().toString());
        }
        else {
            QStyledItemDelegate::paint(painter, adjustedOption, index);
        }
    }

    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton && index.column() == 0) {
                Qt::CheckState currentState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
                model->setData(index, currentState == Qt::Unchecked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
                return true;
            }
        }
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }
};


class NexusTree : public QTreeView
{
    Q_OBJECT

public:
    explicit NexusTree(QWidget* parent = nullptr);
    void reset_tree();
    virtual void restore_tree(json const& j) = 0;
    virtual json to_json() const;
    QStandardItemModel* get_model() { return this->model; }

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
    explicit PortfolioTree(QWidget* parent, HydraPtr hydra);
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
    /// Delete a new strategy underneath a portfolio
    /// </summary>
    /// <param name="parentIndex"></param>
    void delete_strategy(const QModelIndex& parentIndex);

    /// <summary>
    /// Override context menu event to prevent nested portfolios
    /// </summary>
    /// <param name="event"></param>
    void contextMenuEvent(QContextMenuEvent* event) override;

    /// <summary>
    /// Clear all elements from the tree except for the root
    /// </summary>
    void clear();

    /// <summary>
    /// Parent hydra instance
    /// </summary>
    HydraPtr hydra;

    /// <summary>
    /// strategy toggle button delegate
    /// </summary>
    ToggleButtonDelegate toggleButtonDelegate;

signals:
    void strategy_double_clicked(const QString& asset_id);
    void strategy_toggled(const QString& strategy_id, bool checked);
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
    void strategy_remove_requested(const QModelIndex& parentIndex,
        const QString& strategy_id);


public slots:
    void new_item_accepted(const QModelIndex& parentIndex, const QString& name) override;
    void strategy_remove_accepted(const QModelIndex& parentIndex, const QString& strategy_id);
};

//============================================================================
class ExchangeTree : public NexusTree
{
    Q_OBJECT

public:
    explicit ExchangeTree(QWidget* parent, HydraPtr  hydra);
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
    /// Clear all elements from the tree except for the root
    /// </summary>
    void clear();

    /// <summary>
    /// Parent hydra instance
    /// </summary>
    HydraPtr hydra;

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