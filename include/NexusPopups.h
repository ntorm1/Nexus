#pragma once

#include <QVboxlayout>
#include <qfile.h>
#include <QDialog>
#include <QtUiTools/QUiLoader>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>

class Exchange;
typedef std::shared_ptr<Exchange> ExchangePtr;

QT_BEGIN_NAMESPACE
namespace Ui {
    class NewExchangePopup;
    class NewPortfolioPopup;
    class NewStrategyPopup;
}
QT_END_NAMESPACE

class NewPortfolioPopup : public QDialog
{
    Q_OBJECT
public:
    explicit NewPortfolioPopup(QWidget* parent = nullptr);
    ~NewPortfolioPopup();

    QString get_portfolio_id() const;
    QString get_starting_cash() const;

private:
    void on_submit();
    Ui::NewPortfolioPopup* ui;

};

class NewStrategyPopup : public QDialog
{
    Q_OBJECT
public:
    explicit NewStrategyPopup(QWidget* parent = nullptr);
    ~NewStrategyPopup();

    QString get_strategy_id() const;
    QString get_allocation() const;

private:
    void on_submit();
    Ui::NewStrategyPopup* ui;

};

class NewExchangePopup : public QDialog
{
    Q_OBJECT

public:
    explicit NewExchangePopup(
        QWidget* parent = nullptr,
        std::optional<ExchangePtr> exchange = std::nullopt
    );
    ~NewExchangePopup();

    QString get_source() const;
    QString get_exchange_id() const;
    QString get_freq() const;
    QString get_dt_format() const;
    QString get_market_asset_id() const;
    QString get_beta_lookback() const;

    Ui::NewExchangePopup* ui;

private slots:
    void selectFolder();

private:
    void on_submit();
    
};
