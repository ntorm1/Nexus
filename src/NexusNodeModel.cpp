
#include <QVBoxLayout>

#include "NexusNodeModel.h"
#include "NexusNodeWidget.h"
#include "NexusErrors.h"


std::shared_ptr<Hydra> ExchangeModel::hydra = nullptr;


//============================================================================
QWidget* ExchangeModel::embeddedWidget()
{
	if (!this->exchange_node) {
		this->exchange_node = new ExchangeNode(
			ExchangeModel::hydra,
			nullptr
		);

		connect(
			exchange_node->exchange_id,
			QOverload<int>::of(&QComboBox::currentIndexChanged),
			this,
			&ExchangeModel::on_exchange_change
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
QWidget* ExchangeViewModel::embeddedWidget()
{
	if (!this->exchange_view_node) {
		this->exchange_view_node = new ExchangeViewNode(
			nullptr
		);
		connect(
			exchange_view_node->N,
			&QSpinBox::valueChanged,
			this,
			&ExchangeViewModel::on_exchange_view_change
		);
		connect(
			exchange_view_node->query_type,
			QOverload<int>::of(&QComboBox::currentIndexChanged),
			this,
			&ExchangeViewModel::on_exchange_view_change
		);
	}
	return this->exchange_view_node;
}


//============================================================================
QWidget* StrategyAllocationModel::embeddedWidget()
{
	if (!this->strategy_allocation_node) {
		this->strategy_allocation_node = new StrategyAllocationNode(
			nullptr
		);
	}
	return this->strategy_allocation_node;
}


//============================================================================
void AssetLambdaModel::on_lambda_change()
{
	Q_EMIT dataUpdated(0);
}


//============================================================================
void ExchangeModel::on_exchange_change()
{

	Q_EMIT dataUpdated(0);
}


//============================================================================
void ExchangeViewModel::on_exchange_view_change()
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
QJsonObject ExchangeViewModel::save() const
{
	QJsonObject modelJson = NodeDelegateModel::save();
	modelJson["query_type"] = this->exchange_view_node->query_type->currentText();
	modelJson["N"] = QString::number(this->exchange_view_node->N->value());
	return modelJson;
}



//============================================================================
QJsonObject ExchangeModel::save() const
{
	QJsonObject modelJson = NodeDelegateModel::save();
	modelJson["exchange_id"] = this->exchange_node->exchange_id->currentText();
	return modelJson;
}


//============================================================================
QJsonObject StrategyAllocationModel::save() const
{
	QJsonObject modelJson = NodeDelegateModel::save();

	modelJson["epsilon"] =this->strategy_allocation_node->epsilon->text();
	modelJson["target_leverage"] = this->strategy_allocation_node->target_leverage->text();
	modelJson["clear_missing"] = this->strategy_allocation_node->clear_missing->isChecked();
	modelJson["alloc_type"] = this->strategy_allocation_node->alloc_type->currentText();
	modelJson["ev_opp_type"] = this->strategy_allocation_node->ev_opp_type->currentText();

	return modelJson;
}


//============================================================================
void StrategyAllocationModel::load(QJsonObject const& p)
{
	QJsonValue epsilon = p["epsilon"];
	QJsonValue target_leverage = p["target_leverage"];
	QJsonValue clear_missing = p["clear_missing"];
	QJsonValue alloc_type = p["alloc_type"];
	QJsonValue ev_opp_type = p["ev_opp_type"];

	if (!epsilon.isUndefined()) {
		QString _epsilon = epsilon.toString();
		if (!strategy_allocation_node) { this->embeddedWidget(); }
		if (strategy_allocation_node)
			strategy_allocation_node->epsilon->setText(_epsilon);
	}
	if (!target_leverage.isUndefined()) {
		auto _target_leverage = target_leverage.toString();
		if (strategy_allocation_node)
			strategy_allocation_node->target_leverage->setText(_target_leverage);
	}
	if (!clear_missing.isUndefined()) {
		bool _clear_missing = clear_missing.toBool();
		if (strategy_allocation_node)
			strategy_allocation_node->clear_missing->setChecked(_clear_missing);
	}	
	if (!alloc_type.isUndefined()) {
		QString _alloc_type = alloc_type.toString();
		if (strategy_allocation_node)
			strategy_allocation_node->alloc_type->setCurrentText(_alloc_type);
	}
	if (!ev_opp_type.isUndefined()) {
		auto _ev_opp_type = ev_opp_type.toString();
		if (strategy_allocation_node)
			strategy_allocation_node->ev_opp_type->setCurrentText(_ev_opp_type);
	}
}


//============================================================================
void AssetLambdaModel::load(QJsonObject const& p)
{
	QJsonValue opp = p["opperation"];
	QJsonValue column = p["column"];
	QJsonValue row = p["row"];

	if (!opp.isUndefined()) {
		QString str_opp = opp.toString();
		if (!asset_lambda_node) { this->embeddedWidget(); }
		if (asset_lambda_node)
			asset_lambda_node->opperation->setCurrentText(str_opp);
	}
	if (!row.isUndefined()) {
		auto row_num = row.toString().toInt();
		if (asset_lambda_node)
			asset_lambda_node->row->setValue(row_num);
	}
	if (!column.isUndefined()) {
		QString str_column = column.toString();
		if (asset_lambda_node)
			asset_lambda_node->column->setText(str_column);
	}
}


//============================================================================
void ExchangeViewModel::load(QJsonObject const& p)
{
	QJsonValue query_type = p["query_type"];
	QJsonValue N = p["N"];

	if (!query_type.isUndefined()) {
		QString str_opp = query_type.toString();
		if (!exchange_view_node) { this->embeddedWidget(); }
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
void ExchangeModel::load(QJsonObject const& p)
{
	QJsonValue exchange_id = p["exchange_id"];

	if (!exchange_id.isUndefined()) {
		QString str_exchange_id = exchange_id.toString();
		if (!exchange_node) { this->embeddedWidget(); }
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
		this->warmup = abs(row);

		AssetLambda l = AssetLambda(op, [=](const AssetPtr& asset) {
			return asset_feature_lambda(asset, column_name, row);
		});
		AgisAssetLambdaChain new_chain = this->lambda_chain;
		new_chain.push_back({ l, column_name, row });
		return std::make_shared<AssetLambdaData>(new_chain, this->warmup);
	}
	NEXUS_THROW("unexpected out port");
}


//============================================================================
std::shared_ptr<NodeData> ExchangeViewModel::outData(PortIndex const port)
{
	if (port == 0)
	{
		auto N = stoi(this->exchange_view_node->N->text().toStdString());
		auto query_string = this->exchange_view_node->query_type->currentText().toStdString();
		auto query_type = agis_query_map.at(query_string);

		ExchangeViewLambda ev_chain = [](
			AgisAssetLambdaChain const& lambda_opps,
			ExchangePtr const exchange,
			ExchangeQueryType query_type,
			int N) -> ExchangeView
		{
			// function that takes in serious of operations to apply to as asset and outputs
			// a double value that is result of said opps
			auto asset_chain = [&](AssetPtr const& asset) -> double {
				double result = asset_feature_lambda_chain(
					asset,
					lambda_opps
				);
				return result;
			};

			// function that takes an exchange an applys the asset chain to each element when 
			// generating the exchange view
			auto exchange_view = exchange->get_exchange_view(
				asset_chain,
				query_type,
				N
			);
			return exchange_view;
		};

		ExchangeViewLambdaStruct my_struct = {
			N,
			this->warmup,
			this->lambda_chain,
			ev_chain,
			this->exchange,
			query_type
		};
		return std::make_shared<ExchangeViewData>(my_struct);
	}
	NEXUS_THROW("unexpected out port");
}


//============================================================================
void AssetLambdaModel::setInData(std::shared_ptr<NodeData> data, PortIndex const port)
{
	if (port == 0)
	{
		if (!data) { this->lambda_chain.clear(); return; }
		std::shared_ptr<AssetLambdaData> assetData = std::dynamic_pointer_cast<AssetLambdaData>(data);
		this->lambda_chain = assetData->lambda_chain;
		if (assetData->warmup > this->warmup) { this->warmup = assetData->warmup; }
	}
}


//============================================================================
void ExchangeViewModel::setInData(std::shared_ptr<NodeData> data, PortIndex const port)
{
	switch (port)
	{
		case 0: {
			if (!data) { this->lambda_chain.clear(); return; }
			std::shared_ptr<AssetLambdaData> assetData = std::dynamic_pointer_cast<AssetLambdaData>(data);
			this->lambda_chain = assetData->lambda_chain;
			this->warmup = assetData->warmup;
			return;
		}
		
		case 1:{
			if (!data) { this->exchange = nullptr; return; }
			std::shared_ptr<ExchangeData> exchange_data = std::dynamic_pointer_cast<ExchangeData>(data);
			this->exchange = exchange_data->exchange_ptr;
			return;
		}
	}
	NEXUS_THROW("unexpected out port");
}


//============================================================================
void StrategyAllocationModel::setInData(std::shared_ptr<NodeData> data, PortIndex const port)
{
	switch (port)
	{
		case 0: {
			if (!data) { this->ev_lambda_struct = std::nullopt; return; }
			std::shared_ptr<ExchangeViewData> ev_ptr = std::dynamic_pointer_cast<ExchangeViewData>(data);
			this->ev_lambda_struct = ev_ptr->exchange_view_lambda;
			return;
		}
	}
	NEXUS_THROW("unexpected out port");
}
