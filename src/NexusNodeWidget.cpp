#pragma once

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


AssetLambdaNode::AssetLambdaNode(
    QWidget* parent_)
    : QWidget(parent_)
    , layout(nullptr)
{
    setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);

    this->layout = new QVBoxLayout(this);

    // column type
    QHBoxLayout* name_layout = new QHBoxLayout(this);
    QLabel* name_label = new QLabel("Column: ");
    this->column = new QLineEdit(this);
    name_layout->addWidget(name_label);
    name_layout->addWidget(this->column);
    layout->addLayout(name_layout);

    // row value
    QHBoxLayout* row_layout = new QHBoxLayout(this);
    QLabel* row_label = new QLabel("Row: ");
    this->row = new QSpinBox(this);
    this->row->setMinimum(-1e6); // Set the minimum value to the minimum possible integer value (most negative value)
    this->row->setMaximum(0); // Set the maximum value to 0
    row_layout->addWidget(row_label);
    row_layout->addWidget(this->row);
    layout->addLayout(row_layout);

    // operation type
    QHBoxLayout* rowLayout = new QHBoxLayout(this);
    this->opperation = new QComboBox();
    for (const auto& item : agis_function_strings) {
        opperation->addItem(QString::fromStdString(item));
    }
    QLabel* label = new QLabel("Operation: ");
    rowLayout->addWidget(label);
    rowLayout->addWidget(this->opperation);
    layout->addLayout(rowLayout);


    this->setFixedSize(layout->sizeHint());
}
