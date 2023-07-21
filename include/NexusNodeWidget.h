#pragma once

#include <QPushButton>
#include <QWidget>

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