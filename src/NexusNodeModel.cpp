#include "NexusNodeModel.h"
#include "NexusNodeWidget.h"


#include <QVBoxLayout>

std::shared_ptr<Hydra> ExchangeDataModel::hydra = nullptr;


//============================================================================
QWidget* ExchangeDataModel::embeddedWidget()
{
	if (!this->exchange_node) {
		this->exchange_node = new ExchangeNode(
			ExchangeDataModel::hydra,
			nullptr
		);

		connect(
			exchange_node->exchange_id,
			QOverload<int>::of(&QComboBox::currentIndexChanged),
			this,
			&ExchangeDataModel::on_exchange_change
		);
	}

	return this->exchange_node;
}


//============================================================================
QWidget* AssetLambdaModel::embeddedWidget()
{
	if (!this->asset_lambda_node) {
		this->asset_lambda_node = new AssetLambdaNode(
			nullptr
		);

		connect(
			asset_lambda_node->opperation,
			QOverload<int>::of(&QComboBox::currentIndexChanged),
			this,
			&AssetLambdaModel::on_lambda_change
		);
		connect(
			asset_lambda_node->row,
			&QSpinBox::valueChanged,
			this,
			&AssetLambdaModel::on_lambda_change
		);
		connect(
			asset_lambda_node->column,
			&QLineEdit::textChanged,
			this,
			&AssetLambdaModel::on_lambda_change
		);
	}

	return this->asset_lambda_node;
}



//============================================================================
QWidget* ExchangeViewDataModel::embeddedWidget()
{
	if (!this->exchange_view_node) {
		this->exchange_view_node = new ExchangeViewNode(
			nullptr
		);
	}
	return this->exchange_view_node;
}


//============================================================================
void AssetLambdaModel::on_lambda_change()
{
	Q_EMIT dataUpdated(0);
}


//============================================================================
void ExchangeDataModel::on_exchange_change()
{

	Q_EMIT dataUpdated(0);
}


//============================================================================
QJsonObject AssetLambdaModel::save() const
{
	QJsonObject modelJson = NodeDelegateModel::save();

	modelJson["opperation"] = this->asset_lambda_node->opperation->currentText();
	modelJson["column"] = this->asset_lambda_node->column->text();
	modelJson["row"] = QString::number(this->asset_lambda_node->row->value());

	return modelJson;
}


//============================================================================
void AssetLambdaModel::load(QJsonObject const& p)
{
	QJsonValue opp = p["opperation"];
	QJsonValue column = p["column"];
	QJsonValue row = p["row"];

	if (!opp.isUndefined()) {
		QString str_opp = opp.toString();
		if (asset_lambda_node)
		{
			asset_lambda_node->opperation->setCurrentText(str_opp);
		}
	}
	if (!row.isUndefined()) {
		auto row_num = row.toString().toInt();
		if (asset_lambda_node)
		{
			asset_lambda_node->row->setValue(row_num);
		}
	}
	if (!column.isUndefined()) {
		QString str_column = column.toString();
		if (asset_lambda_node)
		{
			asset_lambda_node->column->setText(str_column);
		}
	}
}


//============================================================================
QJsonObject ExchangeViewDataModel::save() const
{
	QJsonObject modelJson = NodeDelegateModel::save();
	modelJson["query_type"] = this->exchange_view_node->query_type->currentText();
	modelJson["N"] = QString::number(this->exchange_view_node->N->value());
	return modelJson;
}


//============================================================================
void ExchangeViewDataModel::load(QJsonObject const& p)
{
	QJsonValue query_type = p["query_type"];
	QJsonValue N = p["N"];

	if (!query_type.isUndefined()) {
		QString str_opp = query_type.toString();
		if (exchange_view_node)
		{
			exchange_view_node->query_type->setCurrentText(str_opp);
		}
	}
	if (!N.isUndefined()) {
		auto N_num = N.toString().toInt();
		if (exchange_view_node)
		{
			exchange_view_node->N->setValue(N_num);
		}
	}
}

//============================================================================
QJsonObject ExchangeDataModel::save() const
{
	QJsonObject modelJson = NodeDelegateModel::save();
	modelJson["exchange_id"] = this->exchange_node->exchange_id->currentText();
	return modelJson;
}


//============================================================================
void ExchangeDataModel::load(QJsonObject const& p)
{
	QJsonValue exchange_id = p["exchange_id"];

	if (!exchange_id.isUndefined()) {
		QString str_exchange_id = exchange_id.toString();
		if (exchange_node)
		{
			exchange_node->exchange_id->setCurrentText(str_exchange_id);
		}
	}
}

//============================================================================
std::shared_ptr<NodeData> AssetLambdaModel::outData(PortIndex const port)
{
	if (port == 0)
	{
		auto op_str = this->asset_lambda_node->opperation->currentText().toStdString();
		AgisOperation op = agis_function_map.at(op_str);
		auto column_name = this->asset_lambda_node->column->text().toStdString();
		auto row = this->asset_lambda_node->row->value();

		AssetLambda l = AssetLambda(op, [&](const AssetPtr& asset) {
			return asset_feature_lambda(asset, column_name, row);
		});
		if (this->lambda_chain.size()) { this->lambda_chain.back() = l; }
		else { this->lambda_chain.push_back(l); }
		return std::make_shared<AssetLambdaData>(this->lambda_chain);
	}
}


//============================================================================
void AssetLambdaModel::setInData(std::shared_ptr<NodeData> data, PortIndex const port)
{
	if (port == 0)
	{
		if (!data) { this->lambda_chain.clear(); return; }
		std::shared_ptr<AssetLambdaData> assetData = std::dynamic_pointer_cast<AssetLambdaData>(data);
		this->lambda_chain = assetData->lambda_chain;
	}
}


