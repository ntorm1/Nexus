#pragma once
#include <QComboBox>
#include <QLabel>

#include "NexusNodeModel.h"
#include "NexusNodeWidget.h"

#include "Hydra.h"

ExchangeNode::ExchangeNode(
    std::shared_ptr<Hydra> const hydra_,
    QWidget* parent_)
        : QWidget(parent_)
        , hydra(hydra_)
        , layout(nullptr)
{
    setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);

    this->layout = new QHBoxLayout(this);

    this->exchange_id = new QComboBox();
    auto& exchanges = this->hydra->get_exchanges();
    auto exchange_ids = exchanges.get_exchange_ids();
    for (const auto& item : exchange_ids) {
        // Convert each std::string to QString before adding to the QComboBox
        exchange_id->addItem(QString::fromStdString(item));
    }

    QLabel* label = new QLabel("Exchange ID: ");
    layout->addWidget(label);
    layout->addWidget(exchange_id);
    this->setFixedSize(layout->sizeHint());
}
