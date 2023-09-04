#pragma once

#include "NexusNodeModel.h"
#include "NexusNodeWidget.h"


//============================================================================
ExchangeNode::ExchangeNode(
    HydraPtr hydra_,
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


//============================================================================
TradeExitNode::TradeExitNode(
    QWidget* parent_)
    : QWidget(parent_)
    , layout(nullptr)
{
    setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);

    this->layout = new QVBoxLayout(this);

    QHBoxLayout* row_layout = new QHBoxLayout(this);
    this->exit_type = new QComboBox();
    for (const auto& item : agis_trade_exit_strings) {
        // Convert each std::string to QString before adding to the QComboBox
        this->exit_type->addItem(QString::fromStdString(item));
    }
    QLabel* label = new QLabel("Exit Type: ");
    row_layout->addWidget(label);
    row_layout->addWidget(this->exit_type);
    this->layout->addLayout(row_layout);

    row_layout = new QHBoxLayout(this);
    this->extra_param = new QLineEdit(this);
    label = new QLabel("Extra Param: ");
    row_layout->addWidget(label);
    row_layout->addWidget(this->extra_param);
    this->layout->addLayout(row_layout);

    this->setFixedSize(layout->sizeHint());
}


//============================================================================
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

    // optional filter 
    QHBoxLayout* filter_layout = new QHBoxLayout(this);
    QLabel* filter_label = new QLabel("Filter Range: ");
    this->filter = new QLineEdit(this);
    filter_layout->addWidget(filter_label);
    filter_layout->addWidget(this->filter);
    layout->addLayout(filter_layout);


    this->setFixedSize(layout->sizeHint());
}


//============================================================================
ExchangeViewNode::ExchangeViewNode(
    QWidget* parent_)
    : QWidget(parent_)
    , layout(nullptr)
{
    setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);

    this->layout = new QVBoxLayout(this);

    // operation type
    QHBoxLayout* row_layout = new QHBoxLayout(this);
    this->query_type = new QComboBox();
    for (const auto& item : agis_query_strings) {
        query_type->addItem(QString::fromStdString(item));
    }
    QLabel* label = new QLabel("Query Type: ");
    row_layout->addWidget(label);
    row_layout->addWidget(this->query_type);
    layout->addLayout(row_layout);

    // row value
    row_layout = new QHBoxLayout(this);
    QLabel* row_label = new QLabel("Count: ");
    this->N = new QSpinBox(this);
    this->N->setMinimum(1); // Set the minimum value to the minimum possible integer value (most negative value)
    this->N->setMaximum(1e5); 
    row_layout->addWidget(row_label);
    row_layout->addWidget(this->N);
    layout->addLayout(row_layout);

    this->setFixedSize(layout->sizeHint());
}


//============================================================================
void StrategyAllocationNode::update_ev_opp_param_state() {
    auto val = this->ev_opp_type->currentText().toStdString();
    bool isEnabled = (val == "CONDITIONAL_SPLIT" || val == "CONSTANT");
    this->ev_opp_param->setEnabled(isEnabled);
}


//============================================================================
StrategyAllocationNode::StrategyAllocationNode(
    QWidget* parent_)
    : QWidget(parent_)
    , layout(nullptr)
{
    setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);

    this->layout = new QVBoxLayout(this);

    //  Allocation type
    QHBoxLayout* row_layout = new QHBoxLayout(this);
    this->alloc_type = new QComboBox();
    for (const auto& item : agis_strat_alloc_strings) {
        this->alloc_type->addItem(QString::fromStdString(item));
    }
    QLabel* label = new QLabel("Alloc Type: ");
    row_layout->addWidget(label);
    row_layout->addWidget(this->alloc_type);
    layout->addLayout(row_layout);

    // function to generate weights from the view
    row_layout = new QHBoxLayout(this);
    this->ev_opp_type = new QComboBox();
    for (const auto& item : exchange_view_opps) {
        this->ev_opp_type->addItem(QString::fromStdString(item));
    }
    connect(this->ev_opp_type, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &StrategyAllocationNode::update_ev_opp_param_state);
    label = new QLabel("Ev Opp Type: ");
    row_layout->addWidget(label);
    row_layout->addWidget(this->ev_opp_type);
    layout->addLayout(row_layout);

    // EV Opp Param
    row_layout = new QHBoxLayout(this);
    QLabel* row_label = new QLabel("EV Opp Param: ");
    this->ev_opp_param = new QLineEdit(this);
    QDoubleValidator* validator = new QDoubleValidator();
    this->ev_opp_param->setValidator(validator);
    this->ev_opp_param->setEnabled(false);
    row_layout->addWidget(row_label);
    row_layout->addWidget(this->ev_opp_param);

    layout->addLayout(row_layout);

    // row value
    row_layout = new QHBoxLayout(this);
    row_label = new QLabel("Epsilon: ");
    this->epsilon = new QLineEdit(this);
    validator = new QDoubleValidator(-1.0, 1.0, 3, epsilon); // 2 decimal places
    this->epsilon->setValidator(validator);
    this->epsilon->setText(".01");
    row_layout->addWidget(row_label);
    row_layout->addWidget(this->epsilon);
    layout->addLayout(row_layout);

    // target leverage
    row_layout = new QHBoxLayout(this);
    row_label = new QLabel("Target Lev: ");
    this->target_leverage = new QLineEdit(this);
    validator = new QDoubleValidator(-10.0, 10.0, 2, target_leverage); // 2 decimal places
    this->target_leverage->setValidator(validator);
    this->target_leverage->setText("1.00");
    row_layout->addWidget(row_label);
    row_layout->addWidget(this->target_leverage);
    layout->addLayout(row_layout);

    row_layout = new QHBoxLayout(this);
    this->clear_missing = new QCheckBox("Clear Missing: ");
    this->clear_missing->setChecked(true);
    layout->addWidget(this->clear_missing);
    layout->addLayout(row_layout);


    this->setFixedSize(layout->sizeHint());
}
