#pragma once

#include <QVboxlayout>
#include <qfile.h>
#include <QDialog>
#include <QtUiTools/QUiLoader>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <optional>

class Exchange;
class NexusEnv;
class Hydra;
typedef std::shared_ptr<Exchange> ExchangePtr;
typedef const Hydra * HydraPtr;

QT_BEGIN_NAMESPACE
namespace Ui {
    class NewExchangePopup;
    class NewPortfolioPopup;
    class NewStrategyPopup;
    class NexusSettings;
    class ExchangesPopup;
}
QT_END_NAMESPACE


/// <summary>
/// Poppup window when a new portfolio is requested.
/// </summary>
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


/// <summary>
/// Popup window when a new strategy is requested.
/// </summary>
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

/// <summary>
/// Pop up window presented when a new exchange is requested.
/// </summary>
class NewExchangePopup : public QDialog
{
    Q_OBJECT

public:
    explicit NewExchangePopup(
        QWidget* parent = nullptr,
        std::optional<ExchangePtr> exchange = std::nullopt
    );
    ~NewExchangePopup();

    QString get_vol_lookback() const;
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


/// <summary>
/// Pop up window presented when exchanges are requested.
/// </summary>
class ExchangesPopup : public QDialog
{
    Q_OBJECT

public:
    explicit ExchangesPopup(
        QWidget* parent = nullptr,
        std::optional<HydraPtr> hydra = std::nullopt
    );
    ~ExchangesPopup();

    QString get_cov_lookback() const;
    QString get_cov_step() const;

    Ui::ExchangesPopup* ui;

    size_t cov_lookback = 0;
    size_t cov_step_size = 1;

private:
    void on_submit();
    std::optional<HydraPtr> hydra;
};


/// <summary>
/// Popup window when Nexus settings ar
/// </summary>
class NexusSettings : public QDialog
{
    Q_OBJECT

signals:
    void settings_changed();

public:
    explicit NexusSettings(
        NexusEnv const * nexs_env,
        QWidget* parent = nullptr
    );
    ~NexusSettings();

    QString get_agis_include_path() const;
    QString get_agis_lib_path() const;
    QString get_agis_pyd_path() const;
    QString get_vs_version() const;

private slots:
    void select_folder(std::string dest);

private:
    void on_submit();
    Ui::NexusSettings* ui;
    NexusEnv const * nexs_env;
};