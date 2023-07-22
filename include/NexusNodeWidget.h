#pragma once

#include <QPushButton>
#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

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
        std::shared_ptr<Hydra> const hydra,
        QWidget* parent = nullptr);

    ~ExchangeNode() { delete layout; };

    std::shared_ptr<Hydra> const hydra;

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