#include <qmessagebox.h>
#include <qfiledialog.h>
#include <filesystem>

#include "Hydra.h"
#include "ExchangeMap.h"
#include "NexusEnv.h"
#include "NexusPopups.h"

#include "ui_NewExchangePopup.h"
#include "ui_NewPortfolioPopup.h"
#include "ui_NewStrategyPopup.h"
#include "ui_NexusSettings.h"
#include "ui_ExchangesPopup.h"

#include "Asset/Asset.h"

using namespace Agis;


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

QString NewStrategyPopup::get_strategy_type() const
{
    return ui->source_type->currentText();
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



//============================================================================
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

        auto frequency = FrequencyToString(exchange.value()->get_frequency());
        this->ui->freq_combo->setCurrentText(QString::fromStdString(frequency));

        this->ui->dt_format_combo->setCurrentText(QString::fromStdString(
			exchange.value()->get_dt_format())
        		);

        auto vol_lookback = std::to_string(exchange.value()->__get_vol_lookback());
        this->ui->vol_lookback->setText(QString::fromStdString(vol_lookback));

        auto market_asset = exchange.value()->__get_market_asset_struct();
        if (market_asset.has_value())
        {
            this->ui->market_asset->setText(
                QString::fromStdString(market_asset.value()->market_id)
            );
            if (market_asset.value()->beta_lookback.has_value())
            {
                auto v = market_asset.value()->beta_lookback.value();
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


//============================================================================
NewExchangePopup::~NewExchangePopup()
{
    delete ui;
}


//============================================================================
void NewExchangePopup::on_submit()
{
    // validate id
    if (this->get_exchange_id().isEmpty())
    {
        QMessageBox::critical(this, "Error", "Missing exchange ID");
        return;
    }
    // validate source 
    if (this->get_source().isEmpty())
    {
        QMessageBox::critical(this, "Error", "Missing source");
        return;
    }
    // validate beta lookback
    if (this->get_beta_lookback().isEmpty() && !this->get_market_asset_id().isEmpty())
    {
        QMessageBox::critical(this, "Error", "Missing beta");
        return;
    }
    // validate vol lookback
    try {
        if (!this->get_vol_lookback().isEmpty()) {
            size_t vol_lookback = std::stoul(this->get_vol_lookback().toStdString());
        }
    }
    catch (const std::exception& e)
    {
		QMessageBox::critical(this, "Error", "Invalid vol lookback");
		return;
	}

    QDialog::accept();
}


//============================================================================
QString NewExchangePopup::get_vol_lookback() const
{
    return ui->vol_lookback->text();
}

//============================================================================
QString NewExchangePopup::get_source() const
{
    return ui->folder_path_label->text();
}


//============================================================================
QString NewExchangePopup::get_exchange_id() const
{
    return ui->exchange_id_edit->text();
}


//============================================================================
QString NewExchangePopup::get_freq() const
{
    return ui->freq_combo->currentText();
}


//============================================================================
QString NewExchangePopup::get_dt_format() const
{
    return ui->dt_format_combo->currentText();
}


//============================================================================
QString NewExchangePopup::get_market_asset_id() const
{
    return ui->market_asset->text();
}


//============================================================================
QString NewExchangePopup::get_beta_lookback() const
{
    return ui->beta_lookback->text();
}


//============================================================================
void NewExchangePopup::selectFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"));
    if (!folderPath.isEmpty())
        ui->folder_path_label->setText(folderPath);
}


//============================================================================
NexusSettings::NexusSettings(
    NexusEnv const* nexs_env_,
    QWidget* parent
) :
    QDialog(parent),
    nexs_env(nexs_env_),
    ui(new Ui::NexusSettings)
{
    ui->setupUi(this);

    this->ui->agis_include_path->setText(QString::fromStdString(
        nexs_env->get_agis_include_path())
    );
    this->ui->agis_lib_path->setText(QString::fromStdString(
        nexs_env->get_agis_dll_path())
    );    
    this->ui->agis_pyd_path->setText(QString::fromStdString(
        nexs_env->get_agis_pyd_path())
    );
    auto agis_build_method = nexs_env->get_agis_build_method();
    // remove first and last char, they are quatations used to pass arg to command line 
    agis_build_method = agis_build_method.substr(1, agis_build_method.size() - 2);
    this->ui->vs_version->setText(QString::fromStdString(
        agis_build_method)
    );

    connect(ui->select_agis_include_path, &QPushButton::clicked, this, [this]() {
        this->select_folder("include");
        });

    connect(ui->select_agis_lib_path, &QPushButton::clicked, this, [this]() {
        this->select_folder("dll");
        });

    connect(ui->select_agis_pyd_path, &QPushButton::clicked, this, [this]() {
        this->select_folder("pyd");
        });
    connect(ui->save_button, &QPushButton::clicked, this, &NexusSettings::on_submit);
}


//============================================================================
void NexusSettings::select_folder(std::string dest)
{
    QString folderPath;
    QStringList filters;

    if (dest == "dll") {
        filters << tr("Dynamic Link Libraries (*.dll)");
    }
    else if (dest == "pyd") {
        filters << tr("Python Extension Modules (*.pyd)");
    }

    if (dest == "include")
    {
        folderPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"));
    }
    else {
        folderPath = QFileDialog::getOpenFileName(this, tr("Select File"), QString(), filters.join(";;"));

    }  
    

    if (!folderPath.isEmpty()) {
        if (dest == "include") {
            ui->agis_include_path->setText(folderPath);
        }
        else if (dest == "dll") {
            ui->agis_lib_path->setText(folderPath);
        }
        else if (dest == "pyd") {
            ui->agis_pyd_path->setText(folderPath);
        }
    }
}

//============================================================================
NexusSettings::~NexusSettings()
{
    delete ui;
}


//============================================================================
QString NexusSettings::get_agis_include_path() const
{
    return ui->agis_include_path->text();
}


//============================================================================
QString NexusSettings::get_agis_lib_path() const
{
    return ui->agis_lib_path->text();
}


//============================================================================
QString NexusSettings::get_agis_pyd_path() const
{
    return ui->agis_pyd_path->text();
}


//============================================================================
QString NexusSettings::get_vs_version() const
{
    return ui->vs_version->text();
}


//============================================================================
void NexusSettings::on_submit()
{
    emit this->settings_changed();
}


//============================================================================
ExchangesPopup::ExchangesPopup(QWidget* parent, std::optional<HydraPtr> hydra_) : 
    QDialog(parent),
    hydra(hydra_),
    ui(new Ui::ExchangesPopup)
{
    ui->setupUi(this);

    if (this->hydra.has_value()) {
        auto& exchanges = this->hydra.value()->get_exchanges();
        auto cov_matrix = exchanges.get_covariance_matrix();
        if (!cov_matrix.is_exception()) {
            auto lookback = cov_matrix.unwrap()->get_lookback();
            this->ui->cov_lookback->setText(QString::fromStdString(std::to_string(lookback)));
        }
    }

    connect(ui->save_button, &QPushButton::clicked, this, &ExchangesPopup::on_submit);
}


//============================================================================
ExchangesPopup::~ExchangesPopup()
{
    delete ui;
}


//============================================================================
bool ExchangesPopup::get_cov_enabled() const
{
    return this->ui->cov_enabled->isChecked();
}

//============================================================================
QString ExchangesPopup::get_cov_lookback() const
{
    return this->ui->cov_lookback->text();
}


//============================================================================
QString ExchangesPopup::get_cov_step() const
{
    return this->ui->cov_step->text();
}


//============================================================================
void ExchangesPopup::on_submit()
{
    // validate vol lookback
    try {
        if (!this->get_cov_lookback().isEmpty()) {
            this->cov_lookback = std::stoul(this->get_cov_lookback().toStdString());
        }
        if (!this->get_cov_lookback().isEmpty()) {
            this->cov_step_size = std::stoul(this->get_cov_step().toStdString());
        }
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, "Error", "Invalid cov lookback");
        return;
    }

    QDialog::accept();
}
