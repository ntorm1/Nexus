#include <qmessagebox.h>
#include <qfiledialog.h>
#include <filesystem>

#include "Exchange.h"
#include "NexusPopups.h"
#include "ui_NewExchangePopup.h"
#include "ui_NewPortfolioPopup.h"
#include "ui_NewStrategyPopup.h"


//============================================================================
bool isValidDirectory(const std::string& path)
{
    std::filesystem::path directoryPath(path);
    return std::filesystem::is_directory(directoryPath);
}


//============================================================================
NewPortfolioPopup::NewPortfolioPopup(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::NewPortfolioPopup)
{
    ui->setupUi(this);

    connect(ui->submit_button, &QPushButton::clicked, this, &NewPortfolioPopup::on_submit);

}


//============================================================================
NewStrategyPopup::NewStrategyPopup(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::NewStrategyPopup)
{
    ui->setupUi(this);

    connect(ui->submit_button, &QPushButton::clicked, this, &NewStrategyPopup::on_submit);

}


//============================================================================
QString NewStrategyPopup::get_allocation() const
{
    return ui->allocation->text();
}


//============================================================================
QString NewStrategyPopup::get_strategy_id() const
{
    return ui->strategy_id->text();
}


//============================================================================
void NewStrategyPopup::on_submit()
{
    if (this->get_strategy_id().isEmpty())
    {
        QMessageBox::critical(this, "Error", "Missing strategy ID");
        return;
    }
    if (this->get_allocation().isEmpty())
    {
        QMessageBox::critical(this, "Error", "Missing starting allocation");
        return;
    }
    QDialog::accept();
}


//============================================================================
NewStrategyPopup::~NewStrategyPopup()
{
    delete ui;
}


//============================================================================
NewPortfolioPopup::~NewPortfolioPopup()
{
    delete ui;
}


//============================================================================
QString NewPortfolioPopup::get_portfolio_id() const
{
    return ui->portfolio_id->text();
}


//============================================================================
QString NewPortfolioPopup::get_starting_cash() const
{
    return ui->starting_cash->text();
}


//============================================================================
void NewPortfolioPopup::on_submit()
{
    if (this->get_portfolio_id().isEmpty())
    {
        QMessageBox::critical(this, "Error", "Missing portfolio ID");
        return;
    }
    if (this->get_starting_cash().isEmpty())
    {
        QMessageBox::critical(this, "Error", "Missing starting cash");
        return;
    }
    
    QDialog::accept();
}

NewExchangePopup::NewExchangePopup(
    QWidget* parent,
    std::optional<ExchangePtr> exchange
) :
    QDialog(parent),
    ui(new Ui::NewExchangePopup)
{
    ui->setupUi(this);

    // load current exchange settings into the popup
    if (exchange.has_value())
    {
        this->ui->exchange_id_edit->setText(QString::fromStdString(
            exchange.value()->get_exchange_id())
        );

        this->ui->folder_path_label->setText(QString::fromStdString(
			exchange.value()->get_source())
        );

        auto frequency = freq_to_string(exchange.value()->get_frequency());
        this->ui->freq_combo->setCurrentText(QString::fromStdString(frequency));

        this->ui->dt_format_combo->setCurrentText(QString::fromStdString(
			exchange.value()->get_dt_format())
        		);

        auto market_asset = exchange.value()->__get_market_asset_struct();
        if (market_asset.has_value())
        {
            this->ui->market_asset->setText(
                QString::fromStdString(market_asset.value().market_id)
            );
            if (market_asset.value().beta_lookback.has_value())
            {
                auto v = market_asset.value().beta_lookback.value();
                this->ui->beta_lookback->setText(
                    QString::fromStdString(std::to_string(v))
                );
            }
        }
    }

    connect(ui->folder_button, &QPushButton::clicked, this, &NewExchangePopup::selectFolder);
    connect(ui->submit_button, &QPushButton::clicked, this, &NewExchangePopup::on_submit);
    connect(ui->cancel_button, &QPushButton::clicked, this, &NewExchangePopup::reject);
}

NewExchangePopup::~NewExchangePopup()
{
    delete ui;
}

void NewExchangePopup::on_submit()
{
    if (this->get_exchange_id().isEmpty())
    {
        QMessageBox::critical(this, "Error", "Missing exchange ID");
        return;
    }
    if (this->get_source().isEmpty())
    {
        QMessageBox::critical(this, "Error", "Missing source");
        return;
    }
    if (this->get_beta_lookback().isEmpty() || this->get_market_asset_id().isEmpty())
    {
        QMessageBox::critical(this, "Error", "Missing beta");
        return;
    }
    QDialog::accept();
}

QString NewExchangePopup::get_source() const
{
    return ui->folder_path_label->text();
}

QString NewExchangePopup::get_exchange_id() const
{
    return ui->exchange_id_edit->text();
}

QString NewExchangePopup::get_freq() const
{
    return ui->freq_combo->currentText();
}

QString NewExchangePopup::get_dt_format() const
{
    return ui->dt_format_combo->currentText();
}

QString NewExchangePopup::get_market_asset_id() const
{
    return ui->market_asset->text();
}

QString NewExchangePopup::get_beta_lookback() const
{
    return ui->beta_lookback->text();
}

void NewExchangePopup::selectFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"));
    if (!folderPath.isEmpty())
        ui->folder_path_label->setText(folderPath);
}
