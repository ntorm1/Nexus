#pragma once
#include "NexusPch.h"
#include <QPushButton>
#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QDoubleValidator>

#include <QtNodes/Definitions>

#include <QHBoxLayout>
#include <QComboBox>

class Hydra;

using QtNodes::NodeId;
using QtNodes::PortIndex;
using QtNodes::PortType;



class ExchangeNode : public QWidget
{
    Q_OBJECT
public:
    ExchangeNode(
        HydraPtr hydra,
        QWidget* parent = nullptr);

    ~ExchangeNode() { delete layout; };

    HydraPtr hydra;

    QHBoxLayout* layout;
    QComboBox* exchange_id;
};

class AssetLambdaNode : public QWidget
{
    Q_OBJECT
public:
    AssetLambdaNode(
        QWidget* parent = nullptr);

    ~AssetLambdaNode() { delete layout; };

    QVBoxLayout* layout;
    QComboBox* opperation;
    QSpinBox* row;
    QLineEdit* column;
    QLineEdit* filter;
};

class ExchangeViewNode : public QWidget
{
    Q_OBJECT
public:
    ExchangeViewNode(
        QWidget* parent = nullptr);

    ~ExchangeViewNode() { delete layout; };

    QVBoxLayout* layout;
    QComboBox* query_type;
    QSpinBox* N;
};

class TradeExitNode : public QWidget
{
    Q_OBJECT
public:
    TradeExitNode(
        QWidget* parent = nullptr);

    ~TradeExitNode() { delete layout; };

    QVBoxLayout* layout;
    QComboBox* exit_type;
    QLineEdit* extra_param;
};

class StrategyAllocationNode : public QWidget
{
    Q_OBJECT
public:
    StrategyAllocationNode(
        QWidget* parent = nullptr);

    ~StrategyAllocationNode() { delete layout; };

    void update_ev_opp_param_state();

    QVBoxLayout* layout;
    QLineEdit* epsilon;
    QLineEdit* target_leverage;
    QCheckBox* clear_missing;
    QComboBox* alloc_type;
    QComboBox* ev_opp_type;
    QLineEdit* ev_opp_param;

};