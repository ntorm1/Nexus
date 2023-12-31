
#include <qmessagebox.h>
#include <QContextMenuEvent>
#include <QInputDialog>

#include "NexusTree.h"
#include "NexusEnv.h"

#include "Asset/Asset.h"
#include "ExchangeMap.h"
#include "Exchange.h"
#include "Portfolio.h"

using namespace Agis;

enum CustomRoles {
    ParentItemRole = Qt::UserRole + 1,
};

NexusTree::NexusTree(QWidget* parent) : QTreeView(parent)
{
    this->model = new QStandardItemModel(this);
}

void NexusTree::reset_tree()
{
    // Retrieve the model associated with the tree view
    QAbstractItemModel* model = this->model;

    // Get the root index
    QModelIndex rootIndex = model->index(0, 0);

    // Get the number of children under the root
    int numChildren = model->rowCount(rootIndex);

    // Remove the children of the root item
    model->removeRows(0, numChildren, rootIndex);

    // Reset the model to update the view
    this->reset();
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


//============================================================================
void NexusTree::handle_new_item_action()
{
    // This slot is connected to the triggered signal of the New Item action
    // It is invoked when the New Item action is triggered via the context menu
    QContextMenuEvent* event = new QContextMenuEvent(QContextMenuEvent::Mouse, QPoint());
    contextMenuEvent(event);
}


//============================================================================
void NexusTree::new_item_accepted(const QModelIndex& parentIndex, const QString& name)
{
    // New item has been accepeted by the main window
    QStandardItemModel* model = static_cast<QStandardItemModel*>(this->model);
    QStandardItem* parentItem = model->itemFromIndex(parentIndex);
    QStandardItem* newItem = new QStandardItem(name);
    newItem->setData(Qt::Checked, Qt::CheckStateRole);

    newItem->setEditable(false);
    newItem->setData(QVariant::fromValue(parentItem), ParentItemRole);  // Set the parent item using custom role
    parentItem->appendRow(newItem);
}


//============================================================================
void NexusTree::remove_item(const QModelIndex& index)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model);
    QStandardItem* item = model->itemFromIndex(index);
    if (item && item != model->invisibleRootItem())
    {
        emit remove_item_requested(item->text(), index);
    }
}


//============================================================================
void NexusTree::remove_item_accepeted(const QModelIndex& index)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model);
    QStandardItem* item = model->itemFromIndex(index);

    QStandardItem* parentItem = item->parent();
    parentItem->removeRow(item->row());
}


//============================================================================
rapidjson::Value
NexusTree::to_json(rapidjson::Document::AllocatorType& allocator) const
{
    // Save the state of the tree to json
    Value j(kObjectType);
    rapidjson::Value rootExpanded(this->isExpanded(this->root->index()));
    j.AddMember("root_expanded", rootExpanded, allocator);

    auto root_index = this->root->index();
    int rowCount = root_index.model()->rowCount(root_index);

    for (int i = 0; i < rowCount; i++)
    {
        QModelIndex childIndex = root_index.model()->index(i, 0, root_index);
        QModelIndex parentIndex = childIndex.parent();
        const QAbstractItemModel* abstractModel = parentIndex.model();
        const QStandardItemModel* parent_model = static_cast<const QStandardItemModel*>(abstractModel);
        QStandardItem* childItem = parent_model->itemFromIndex(childIndex);
        rapidjson::Value childExpanded(this->isExpanded(childIndex));
        rapidjson::Value key(childItem->text().toStdString().c_str(), allocator);
        j.AddMember(key.Move(), childExpanded, allocator);
    }
    return j;
}


//============================================================================
PortfolioTree::PortfolioTree(QWidget* parent, HydraPtr hydra_) :
    NexusTree(parent),
    hydra(hydra_)
{
    this->setObjectName("Portfolios");
    QStandardItem* rootItem = this->model->invisibleRootItem();
    this->root = new QStandardItem("Portfolios");
    this->root->setEditable(false);
    this->root->setData(QVariant::fromValue(rootItem), ParentItemRole);  // Set the parent item using custom role
    rootItem->appendRow(this->root);

    this->setModel(model);

    // listen the check box of a strategy
    QObject::connect(model, &QStandardItemModel::dataChanged, [&](const QModelIndex& topLeft, const QModelIndex& bottomRight) {
        // Check if the changed data role is Qt::CheckStateRole
        if (topLeft.column() == 0 && model->data(topLeft, Qt::CheckStateRole).isValid()) {
            QModelIndex index = topLeft;  // Use the top left index for simplicity
            // Check if the checkbox item has two parents
            if (index.parent().isValid() && index.parent().parent().isValid()) {
                Qt::CheckState newState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
                
                QVariant itemData = index.data(Qt::DisplayRole);
                QString strategy_id = itemData.toString();

                if (newState == Qt::Checked) {
                    qDebug() << strategy_id << "was checked, attempting to set strategy live";
                    emit strategy_toggled(strategy_id, true);
                }
                else {
                    qDebug() << strategy_id << "was unchecked, attempting to disable strategy";
                    emit strategy_toggled(strategy_id, false);
                }
            }
        }
        });

    // strategy toggle
    this->setItemDelegateForColumn(0, &toggleButtonDelegate);
}


//============================================================================
void PortfolioTree::restore_tree(const rapidjson::Document& doc)
{
    this->clear();
    if (!doc.HasMember("hydra_state") || !doc["hydra_state"].HasMember("portfolios")) { return; }
    const rapidjson::Value& portfolios = doc["hydra_state"]["portfolios"];
    const rapidjson::Value& portfolios_expanded = doc["trees"][this->objectName().toStdString().c_str()];

    // Is the root exchanges element expanded?
    QModelIndex itemIndex = root->index();
    if (portfolios_expanded["root_expanded"].GetBool())
    {
        this->setExpanded(itemIndex, true);
    }

    for (rapidjson::Value::ConstMemberIterator portfolio = portfolios.MemberBegin(); portfolio != portfolios.MemberEnd(); ++portfolio)
    {
        QString id = QString::fromStdString(portfolio->name.GetString());
        QStandardItem* newItem = new QStandardItem(id);

        // Set exchange tree item flags. Can not edit or create child exchanges (TODO)
        Qt::ItemFlags rootFlags = newItem->flags();
        rootFlags &= ~Qt::ItemIsDropEnabled;  // Remove the drop enabled flag
        newItem->setFlags(rootFlags);
        newItem->setEditable(false);
        newItem->setData(QVariant::fromValue(root), ParentItemRole);  // Set the parent item using custom role

        root->appendRow(newItem);
        this->restore_strategies(newItem, id);

        // Is the element expanded?
        if (portfolios_expanded.HasMember(id.toStdString().c_str()) && portfolios_expanded[id.toStdString().c_str()].GetBool())
        {
            QModelIndex itemIndex = newItem->index();
            this->setExpanded(itemIndex, true);
        }
    }
}


//============================================================================
void PortfolioTree::clear()
{
    // clear previous tree
    this->model->clear();
    QStandardItem* rootItem = this->model->invisibleRootItem();
    this->root = new QStandardItem("Portfolios");
    this->root->setEditable(false);
    this->root->setData(QVariant::fromValue(rootItem), ParentItemRole);  // Set the parent item using custom role
    rootItem->appendRow(this->root);
}


//============================================================================
void ExchangeTree::clear()
{
    // clear previous tree
    this->model->clear();
    QStandardItem* rootItem = this->model->invisibleRootItem();
    this->root = new QStandardItem("Exchanges");
    this->root->setEditable(false);
    this->root->setData(QVariant::fromValue(rootItem), ParentItemRole);  // Set the parent item using custom role
    rootItem->appendRow(this->root);
}


//============================================================================
void PortfolioTree::relink_tree(std::vector<std::string> const& portfolios)
{
    this->clear();
    QModelIndex itemIndex = root->index();
    this->setExpanded(itemIndex, true);
    
    for (auto& id_str : portfolios)
    {
        QString id = QString::fromStdString(id_str);
        QStandardItem* newItem = new QStandardItem(id);

        // Set exchange tree item flags. Can not edit or create child exchanges (TODO)
        Qt::ItemFlags rootFlags = newItem->flags();
        rootFlags &= ~Qt::ItemIsDropEnabled;  // Remove the drop enabled flag
        newItem->setFlags(rootFlags);
        newItem->setEditable(false);
        newItem->setData(QVariant::fromValue(root), ParentItemRole);  // Set the parent item using custom role

        root->appendRow(newItem);
        this->restore_strategies(newItem, id);

        // Is the element expanded?
        QModelIndex itemIndex = newItem->index();
        this->setExpanded(itemIndex, true);
    }
}


//============================================================================
void PortfolioTree::create_new_item(const QModelIndex& parentIndex)
{
    NewPortfolioPopup* popup = new NewPortfolioPopup();
    if (popup->exec() == QDialog::Accepted)
    {
        auto portfolio_id = popup->get_portfolio_id();
        auto starting_cash = popup->get_starting_cash();

        // Send signal to main window asking to create the new item
        emit new_item_requested(parentIndex, portfolio_id, starting_cash);
    }
}


//============================================================================
void PortfolioTree::create_new_strategy(const QModelIndex& parentIndex)
{
    QVariant itemData = parentIndex.data(Qt::DisplayRole);
    QString portfolio_id = itemData.toString();

    NewStrategyPopup* popup = new NewStrategyPopup();
    if (popup->exec() == QDialog::Accepted)
    {
        auto strategy_id = popup->get_strategy_id();
        auto allocation = popup->get_allocation();
        auto strategy_type = popup->get_strategy_type();

        AgisStrategyType type;
        if (strategy_type.isEmpty()) {
            QMessageBox::warning(this, "Warning", "Invalid strategy type");
            return;
		}
        else if (strategy_type == "FLOW") {
			type = AgisStrategyType::FLOW;
		}
        else if (strategy_type == "LUAJIT") {
			type = AgisStrategyType::LUAJIT;
		}
        else if (strategy_type == "CPP") {
            type = AgisStrategyType::CPP;
            throw std::runtime_error("CPP strategy type not implemented");
        }
        else {
			QMessageBox::warning(this, "Warning", "Invalid strategy type");
			return;
		}

        // Send signal to main window asking to create the new item
        emit new_strategy_requested(parentIndex, portfolio_id, strategy_id, allocation, type);
    }
}


//============================================================================
void PortfolioTree::set_benchmark_strategy(const QModelIndex& parentIndex)
{
    QVariant itemData = parentIndex.data(Qt::DisplayRole);
    QString portfolio_id = itemData.toString();

    auto portfolio = this->hydra->get_portfolio(portfolio_id.toStdString());
    auto& exchanges = this->hydra->get_exchanges();
    auto market_asset = exchanges.__get_market_asset(portfolio->get_frequency());

    if(market_asset.is_exception()) {
		QMessageBox::warning(this, "Warning", "No market asset found for portfolio frequency");
		return;
	}

    auto benchmark_strategy_id = portfolio_id + " Benchmark";
    emit new_strategy_requested(parentIndex, portfolio_id, benchmark_strategy_id, "0.0", AgisStrategyType::BENCHMARK);
}

//============================================================================
void PortfolioTree::delete_strategy(const QModelIndex& parentIndex)
{
    QVariant itemData = parentIndex.data(Qt::DisplayRole);
    QString itemName = itemData.toString();

    emit strategy_remove_requested(parentIndex, itemName);
}


//============================================================================
void PortfolioTree::contextMenuEvent(QContextMenuEvent* event)
{
    QModelIndex index = this->indexAt(event->pos());
    if (index.isValid())
    {
        QStandardItemModel* model = static_cast<QStandardItemModel*>(this->model);
        QStandardItem* item = model->itemFromIndex(index);
        if (item != this->root) {
            QMenu menu(this);
            // item selected is a strategy
            if (index.parent().isValid() && index.parent().parent().isValid()) 
            {
                QAction* addAction = new QAction("Remove Strategy", this);
                connect(addAction, &QAction::triggered, [this, index]() {delete_strategy(index); });
                menu.addAction(addAction);
            }
            // item selected is a portfolio
            else
            {
                QAction* addAction = new QAction("Add Strategy", this);
                connect(addAction, &QAction::triggered, [this, index]() { create_new_strategy(index); });
                menu.addAction(addAction);
                
                addAction = new QAction("Add Benchmark Strategy", this);
                connect(addAction, &QAction::triggered, [this, index]() { set_benchmark_strategy(index); });
                menu.addAction(addAction);
            }
            menu.exec(event->globalPos());
            return;
        }
        else {
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
}


//============================================================================
void PortfolioTree::new_item_accepted(const QModelIndex& parentIndex, const QString& name)
{
    NexusTree::new_item_accepted(parentIndex, name);

    QStandardItem* parentItem = model->itemFromIndex(parentIndex);
    QStandardItem* addedItem = parentItem->child(parentItem->rowCount() - 1);

    if (!this->hydra->portfolio_exists(name.toStdString())) { return; }
    this->restore_strategies(addedItem, name);
}

void PortfolioTree::strategy_remove_accepted(const QModelIndex& parentIndex, const QString& strategy_id)
{
}


//============================================================================
void PortfolioTree::restore_strategies(QStandardItem* addedItem, QString id)
{
    auto& portfolio = this->hydra->get_portfolio(id.toStdString());
    auto strategy_ids = portfolio->get_strategy_ids();

    // Generate tree item for each asset listed on the exchange
    for (auto const& strategy_id : strategy_ids)
    {
        auto q_id = QString::fromStdString(strategy_id);
        QStandardItem* newItem = new QStandardItem(q_id);
        Qt::ItemFlags rootFlags = newItem->flags();
        rootFlags &= ~Qt::ItemIsDropEnabled;  // Remove the drop enabled flag
        newItem->setFlags(rootFlags);
        newItem->setEditable(false);
        newItem->setData(QVariant::fromValue(addedItem), ParentItemRole);  // Set the parent item using custom role
        
        auto strategy = this->hydra->get_strategy(strategy_id);
        if (strategy->__is_live()) {
			newItem->setData(Qt::Checked, Qt::CheckStateRole);
		}
        else {
			newItem->setData(Qt::Unchecked, Qt::CheckStateRole);
		}
        addedItem->appendRow(newItem);
    }
}


//============================================================================
void PortfolioTree::mouseDoubleClickEvent(QMouseEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid())
    {
        QVariant itemData = index.data(Qt::DisplayRole);
        if (itemData.canConvert<QString>())
        {
            QString itemName = itemData.toString();
            if (this->hydra->strategy_exists(itemName.toStdString()))
            {
                emit strategy_double_clicked(itemName);
            }
            else if (this->hydra->portfolio_exists(itemName.toStdString()))
            {
                emit portfolio_double_clicked(itemName);
            }
        }
    }
    QTreeView::mouseDoubleClickEvent(event);
}

//============================================================================
ExchangeTree::ExchangeTree(QWidget* parent, NexusEnv* nexus_env_) :
    NexusTree(parent),
    hydra(nexus_env_->get_hydra()),
    nexus_env(nexus_env_)
{
    this->setObjectName("Exchanges");
    connect(this, &NexusTree::customContextMenuRequested, this, &NexusTree::handle_new_item_action);

    QStandardItem* rootItem = this->model->invisibleRootItem();
    this->root = new QStandardItem("Exchanges");
    this->root->setEditable(false);
    this->root->setData(QVariant::fromValue(rootItem), ParentItemRole);  // Set the parent item using custom role
    rootItem->appendRow(this->root);

    this->setModel(model);
}


//============================================================================
void ExchangeTree::contextMenuEvent(QContextMenuEvent* event) {
    // If index is an exchange disable the context menu from popping up
    QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        QStandardItem* item = model->itemFromIndex(index);
        if (item != this->root) {
            return;
        }
    }

    // Proceed with the default context menu event handling
    NexusTree::contextMenuEvent(event);
}


//============================================================================
void ExchangeTree::new_item_accepted(const QModelIndex& parentIndex, const QString& name)
{
    NexusTree::new_item_accepted(parentIndex, name);

    QStandardItem* parentItem = model->itemFromIndex(parentIndex);
    QStandardItem* addedItem = parentItem->child(parentItem->rowCount() - 1);

    this->restore_ids(addedItem, name);
}


//============================================================================
AgisResult<bool> ExchangeTree::edit_exchange_instance(QString const& exchange_id)
{
    auto exchange_opt = this->hydra->get_exchanges()
        .get_exchange(exchange_id.toStdString());
    if (!exchange_opt.has_value()) {
		return AgisResult<bool>(AGIS_EXCEP("Exchange not found"));
	}
    auto & exchange = exchange_opt.value();

    NewExchangePopup* popup = new NewExchangePopup(this, exchange);
    
    if (popup->exec() == QDialog::Accepted)
    {
        auto market_asset_id = popup->get_market_asset_id();
        auto beta_lookback = popup->get_beta_lookback();
        if (!market_asset_id.isEmpty())
        {
            auto market_asset = std::make_shared<MarketAsset>(
                market_asset_id.toStdString(),
                stoi(beta_lookback.toStdString())
            );

            //test to see if market asset is the same
            auto market_asset_struct = exchange->__get_market_asset_struct();
            if (market_asset_struct.has_value())
            {
                if (market_asset_struct.value() == market_asset)
                {
					return AgisResult<bool>(true);
				}
            }

            auto res = this->nexus_env->set_market_asset(
                exchange_id.toStdString(),
                market_asset_id.toStdString(),
                true, 
                stoi(beta_lookback.toStdString())
            );
            return res;
        }

        // update vol lookback if needed
        auto vol_lookback = popup->get_vol_lookback();
        size_t vol_lookback_size_t = std::stoul(vol_lookback.toStdString());
        ExchangePtr exchange = this->hydra->get_exchanges()
            .get_exchange(popup->get_exchange_id().toStdString())
            .value();
        if (exchange->__get_vol_lookback() != vol_lookback_size_t) {
            exchange->__set_volatility_lookback(vol_lookback_size_t);
        }

    }

    return AgisResult<bool>(true);
}


//============================================================================
AgisResult<bool> ExchangeTree::edit_exchanges_instance()
{
    auto* popup = new ExchangesPopup(this, this->hydra);
    if (popup->exec() == QDialog::Accepted)
    {
        // covariance lookback set and matrix tracing enabled
        auto cov_lookback = popup->cov_lookback;
        if (cov_lookback != 0 && popup->get_cov_enabled()) {
            return this->nexus_env->init_covariance_matrix(cov_lookback, popup->cov_step_size);
        }
    }

    return AgisResult<bool>(true);
}


//============================================================================
void ExchangeTree::create_new_item(const QModelIndex& parentIndex)
{
    NewExchangePopup* popup = new NewExchangePopup();
    if (popup->exec() == QDialog::Accepted)
    {
        auto market_asset_id = popup->get_market_asset_id();
        auto beta_lookback = popup->get_beta_lookback();
        std::optional<std::shared_ptr<MarketAsset>> market_asset = std::nullopt;
        if (!market_asset_id.isEmpty())
        {
            market_asset = std::make_shared<MarketAsset>(
                market_asset_id.toStdString(),
                stoi(beta_lookback.toStdString())
            );
        }

        // Send signal to main window asking to create the new item
        emit new_item_requested(parentIndex, popup);
    }
}


//============================================================================
void ExchangeTree::mouseDoubleClickEvent(QMouseEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid())
    {
        QVariant itemData = index.data(Qt::DisplayRole);
        if (itemData.canConvert<QString>())
        {
            QString itemName = itemData.toString();
            AgisResult<bool> res(true);
            // open asset view window
            if (this->hydra->asset_exists(itemName.toStdString()))
            {
                emit asset_double_click(itemName);
            }
            // open exchange settings popup
            else if (this->hydra->get_exchanges().exchange_exists(itemName.toStdString()))
            {
                res = this->edit_exchange_instance(itemName);

            }
            else {
                res = this->edit_exchanges_instance();
            }
            if (res.is_exception())
            {
                QMessageBox::critical(this, "Error", QString::fromStdString(res.get_exception()));
                return;
            }
        }
    }
    QTreeView::mouseDoubleClickEvent(event);
}


//============================================================================
void ExchangeTree::restore_ids(QStandardItem* addedItem, QString exchange_id)
{
    auto asset_ids = this->hydra->get_asset_ids(exchange_id.toStdString());

    // Generate tree item for each asset listed on the exchange
    for (auto const& asset_id : asset_ids)
    {
        auto q_id = QString::fromStdString(asset_id);
        QStandardItem* newItem = new QStandardItem(q_id);
        Qt::ItemFlags rootFlags = newItem->flags();
        rootFlags &= ~Qt::ItemIsDropEnabled;  // Remove the drop enabled flag
        newItem->setFlags(rootFlags);
        newItem->setEditable(false);
        newItem->setData(QVariant::fromValue(addedItem), ParentItemRole);  // Set the parent item using custom role
        addedItem->appendRow(newItem);
    }
}


//============================================================================
void ExchangeTree::restore_tree(const rapidjson::Document& doc)
{
    this->clear();
    if (!doc.HasMember("hydra_state") || !doc["hydra_state"].HasMember("exchanges")) { return; }
    const rapidjson::Value& exchanges = doc["hydra_state"]["exchanges"];
    const rapidjson::Value& exchanges_expanded = doc["trees"][this->objectName().toStdString().c_str()];

    // Is the root exchanges element expanded?
    QModelIndex itemIndex = root->index();
    if (exchanges_expanded["root_expanded"].GetBool())
    {
        this->setExpanded(itemIndex, true);
    }

    for (rapidjson::Value::ConstMemberIterator exchange = exchanges.MemberBegin(); exchange != exchanges.MemberEnd(); ++exchange)
    {
        QString id = QString::fromStdString(exchange->name.GetString());
        QStandardItem* newItem = new QStandardItem(id);

        // Set exchange tree item flags. Cannot edit or create child exchanges (TODO)
        Qt::ItemFlags rootFlags = newItem->flags();
        rootFlags &= ~Qt::ItemIsDropEnabled;  // Remove the drop enabled flag
        newItem->setFlags(rootFlags);
        newItem->setEditable(false);
        newItem->setData(QVariant::fromValue(root), ParentItemRole);  // Set the parent item using a custom role

        root->appendRow(newItem);
        this->restore_ids(newItem, id);

        // Is the exchange element expanded?
        if (exchanges_expanded.HasMember(id.toStdString().c_str()) && exchanges_expanded[id.toStdString().c_str()].GetBool())
        {
            QModelIndex itemIndex = newItem->index();
            this->setExpanded(itemIndex, true);
        }
    }
}
