#include <qmessagebox.h>
#include <qfiledialog.h>
#include <filesystem>

#include "NexusPopups.h"
#include "ui_NewExchangePopup.h"
#include "ui_NewPortfolioPopup.h"

bool isValidDirectory(const std::string& path)
{
    std::filesystem::path directoryPath(path);
    return std::filesystem::is_directory(directoryPath);
}


NewPortfolioPopup::NewPortfolioPopup(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::NewPortfolioPopup)
{
    ui->setupUi(this);

    connect(ui->submit_button, &QPushButton::clicked, this, &NewPortfolioPopup::on_submit);

}


NewPortfolioPopup::~NewPortfolioPopup()
{
    delete ui;
}


QString NewPortfolioPopup::get_portfolio_id() const
{
    return ui->portfolio_id->text();
}

QString NewPortfolioPopup::get_starting_cash() const
{
    return ui->starting_cash->text();
}

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

NewExchangePopup::NewExchangePopup(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::NewExchangePopup)
{
    ui->setupUi(this);

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
    //if (!isValidDirectory(this->get_source().toStdString()))
    //{
    //    QMessageBox::critical(this, "Error", "Invlaid Dir");
    //    return;
    //}
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

void NewExchangePopup::selectFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"));
    if (!folderPath.isEmpty())
        ui->folder_path_label->setText(folderPath);
}
