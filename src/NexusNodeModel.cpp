#include "NexusNodeModel.h"
#include "NexusNodeWidget.h"


#include <QVBoxLayout>

std::shared_ptr<Hydra> ExchangeDataModel::hydra = nullptr;



QWidget* ExchangeDataModel::embeddedWidget()
{
	if (!this->exchange_node) {
		this->exchange_node = new ExchangeNode(
			ExchangeDataModel::hydra,
			nullptr
		);

		connect(
			exchange_node->exchange_id,
			QOverload<int>::of(&QComboBox::currentIndexChanged),
			this,
			&ExchangeDataModel::on_exchange_change
		);
	}

	return this->exchange_node;
}

void ExchangeDataModel::on_exchange_change()
{

	Q_EMIT dataUpdated(0);
}
