
#include <QVBoxLayout>

#include "NexusNodeModel.h"
#include "NexusNodeWidget.h"
#include "NexusErrors.h"


HydraPtr ExchangeModel::hydra = nullptr;


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
		connect(
			asset_lambda_node->filter,
			&QLineEdit::textChanged,
			this,
			&AssetLambdaModel::on_filter_change
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
QWidget* TradeExitModel::embeddedWidget()
{
	if (!this->trade_exit_node) {
		this->trade_exit_node = new TradeExitNode(
			nullptr
		);

		connect(
			trade_exit_node->exit_type,
			QOverload<int>::of(&QComboBox::currentIndexChanged),
			this,
			&TradeExitModel::on_exit_change
		);
		connect(
			trade_exit_node->extra_param,
			&QLineEdit::textChanged,
			this,
			&TradeExitModel::on_exit_change
		);
	}

	return this->trade_exit_node;
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
void AssetLambdaModel::on_filter_change()
{
	auto filter_text = this->asset_lambda_node->filter->text();
	if (filter_text.isEmpty()) {
		this->on_lambda_change();
		return;
	};

	// check to see if last char is ) or ], if it is then call on_lambda_change
	auto& last_char = filter_text[filter_text.size() - 1];
	if (last_char == ')' || last_char == ']') {
		try {
			auto filter = AssetFilterRange(filter_text.toStdString());
			// reset the style sheet background 
			this->asset_lambda_node->filter->setStyleSheet("QLineEdit { background: white; }");
			this->on_lambda_change();
		}
		catch (std::exception& e) {
			// change the color of the filter edit to red 
			this->asset_lambda_node->filter->setStyleSheet("QLineEdit { background: red; }");
		}
	}
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
void TradeExitModel::on_exit_change()
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
	modelJson["filter"] = this->asset_lambda_node->filter->text();
	return modelJson;
}



//============================================================================
QJsonObject TradeExitModel::save() const
{
	QJsonObject modelJson = NodeDelegateModel::save();
	modelJson["exit_type"] = this->trade_exit_node->exit_type->currentText();
	modelJson["extra_param"] = this->trade_exit_node->extra_param->text();
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

	if (this->strategy_allocation_node->ev_opp_param->isEnabled())
	{
		modelJson["ev_opp_param"] = this->strategy_allocation_node->ev_opp_param->text();
	}
	else
	{
		modelJson["ev_opp_param"] = "";
	}

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
	QJsonValue ev_opp_param = p["ev_opp_param"];

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
	if (!ev_opp_param.isUndefined()) {
		auto _ev_opp_param = ev_opp_param.toString();
		if (strategy_allocation_node)
		{
			if (_ev_opp_param == "")
			{
				strategy_allocation_node->ev_opp_param->setEnabled(false);
			}
			else
			{
				strategy_allocation_node->ev_opp_param->setEnabled(true);
				strategy_allocation_node->ev_opp_param->setText(_ev_opp_param);
			}
		}
	}
}


//============================================================================
void AssetLambdaModel::load(QJsonObject const& p)
{
	QJsonValue opp = p["opperation"];
	QJsonValue column = p["column"];
	QJsonValue row = p["row"];
	QJsonValue filter = p["filter"];

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
	if (!filter.isUndefined()) {
		QString str_filter = filter.toString();
		if (asset_lambda_node)
			asset_lambda_node->filter->setText(str_filter);
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
void TradeExitModel::load(QJsonObject const& p)
{
	QJsonValue exit_type = p["exit_type"];
	QJsonValue extra_param = p["extra_param"];


	if (!exit_type.isUndefined()) {
		QString str_exit_type = exit_type.toString();
		if (!trade_exit_node) { this->embeddedWidget(); }
		if (trade_exit_node)
		{
			trade_exit_node->exit_type->setCurrentText(str_exit_type);
		}
	}
	if (!extra_param.isUndefined()) {
		QString str_extra_param = extra_param.toString();
		if (trade_exit_node)
		{
			trade_exit_node->extra_param->setText(str_extra_param);
		}
	}
}

//============================================================================
std::shared_ptr<NodeData> AssetLambdaModel::outData(PortIndex const port)
{
	if (port == 0)
	{
		// extract information about the asset lambda operation
		auto op_str = this->asset_lambda_node->opperation->currentText().toStdString();
		AgisOperation op = agis_function_map.at(op_str);
		auto column_name = this->asset_lambda_node->column->text().toStdString();
		auto row = this->asset_lambda_node->row->value();
		this->warmup = abs(row);

		// build the asset lambda struct and push to the chain
		AssetLambda l = AssetLambda(op, [=](const AssetPtr& asset) {
			return asset->get_asset_feature(column_name, row);
		});
		AgisAssetLambdaChain new_chain = this->lambda_chain;
		AssetLambdaScruct asset_lambda_struct{ l, op, column_name, row };
		new_chain.push_back(asset_lambda_struct);

		// parse the filter if needed
		auto filter_str = this->asset_lambda_node->filter->text();
		if (!filter_str.isEmpty() && filter_str != "")
		{
			new_chain.push_back(AssetLambdaScruct(AssetFilterRange(filter_str.toStdString())));
		}

		return std::make_shared<AssetLambdaData>(std::move(new_chain), this->warmup);
	}
	NEXUS_THROW("unexpected out port");
}


//============================================================================
std::shared_ptr<NodeData> ExchangeModel::outData(PortIndex const port)
{
	if (port == 0)
	{
		auto exchange_id = this->exchange_node->exchange_id->currentText().toStdString();
		auto exchange = this->hydra->get_exchanges().get_exchange(exchange_id);
		return std::make_shared<ExchangeData>(exchange);
	}
	return nullptr;
}


//============================================================================
std::shared_ptr<NodeData> TradeExitModel::outData(PortIndex const port)
{
	if (port == 0)
	{

		auto exit_type = this->trade_exit_node->exit_type->currentText();
		auto extra_param = this->trade_exit_node->extra_param->text().toStdString();
		AgisResult<TradeExitPtr> trade_exit;
		try {
 			trade_exit = parse_trade_exit(
				trade_exit_type_map.at(exit_type.toStdString()),
				extra_param
			);
			this->trade_exit_node->extra_param->setStyleSheet("QLineEdit { background: white; }");
			return std::make_shared<TradeExitData>(trade_exit.unwrap());
		}
		catch (std::exception& e) {
			this->trade_exit_node->extra_param->setStyleSheet("QLineEdit { background: red; }");
			return nullptr;
		}
	}
	else {
		return nullptr;
	}
}


//============================================================================
std::shared_ptr<NodeData> ExchangeViewModel::outData(PortIndex const port)
{
	if (port == 0)
	{
		auto N = stoi(this->exchange_view_node->N->text().toStdString());
		auto query_string = this->exchange_view_node->query_type->currentText().toStdString();
		auto query_type = agis_query_map.at(query_string);
		auto warmup_copy = this->warmup;

		ExchangeViewLambda ev_chain = [=](
			AgisAssetLambdaChain const& lambda_opps,
			ExchangePtr const exchange,
			ExchangeQueryType query_type,
			int N) -> ExchangeView
		{
			// function that takes in serious of operations to apply to as asset and outputs
			// a double value that is result of said opps
			auto asset_chain = [&](AssetPtr const& asset) -> AgisResult<double> {
				return asset_feature_lambda_chain(
					asset,
					lambda_opps
				);
			};

			// function that takes an exchange an applys the asset chain to each element when 
			// generating the exchange view
			AGIS_TRY(
				auto exchange_view = exchange->get_exchange_view(
					asset_chain,
					query_type,
					N,
					false,
					warmup_copy
				);
				return exchange_view;
			);
		};

		ExchangeViewLambdaStruct my_struct = {
			N,
			this->warmup,
			this->lambda_chain,
			ev_chain,
			this->exchange,
			query_type,
			std::nullopt
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
		if (!data) { 
			this->lambda_chain.clear(); 
			Q_EMIT dataInvalidated(0);
			return; 
		}
		std::shared_ptr<AssetLambdaData> assetData = std::dynamic_pointer_cast<AssetLambdaData>(data);
		this->lambda_chain = assetData->lambda_chain;
		if (assetData->warmup > this->warmup) { this->warmup = assetData->warmup; }
		Q_EMIT dataUpdated(0);
	}
}


//============================================================================
void ExchangeViewModel::setInData(std::shared_ptr<NodeData> data, PortIndex const port)
{
	switch (port)
	{
		case 0: {
			if (!data) 
			{ 
				this->lambda_chain.clear(); 
				Q_EMIT dataInvalidated(0);
				return; 
			}

			// take in the asset lambda chain and convert the string columns to size_t indexes
			// to prevent map lookups at runtime 
			std::shared_ptr<AssetLambdaData> assetData = std::dynamic_pointer_cast<AssetLambdaData>(data);
			this->lambda_chain.clear();
			for(AssetLambdaScruct& lambda_struct : assetData->lambda_chain)
			{
				// if the asset lambda struct is a filter then just push it to the chain
				if (lambda_struct.is_filter()) {
					this->lambda_chain.emplace_back(lambda_struct);
					continue;
				}

				// parse column name, if it is invalid return. Needs to be improved
				auto& operation = lambda_struct.get_asset_operation_struct();
				auto& column_name = operation.column;
				auto column_index_res = this->exchange->get_column_index(column_name);
				if (column_index_res.is_exception())
				{
					this->lambda_chain.clear();
					Q_EMIT dataInvalidated(0);
					return;
				}
				// with the new column index create a new asset lambda struct and push to the chain
				auto column_index = column_index_res.unwrap();
				auto row = operation.row;
				AssetLambda lambda_op = AssetLambda(operation.asset_lambda.first, [=](const AssetPtr& asset) -> AgisResult<double> {
					return asset->get_asset_feature(column_index, row);
					});
				AssetLambdaScruct asset_lambda_struct{ lambda_op, operation.asset_lambda.first, column_name, row};
				this->lambda_chain.emplace_back(asset_lambda_struct);
			}

			// extract the warmup needed for the lambda chain
			int min_row = 0;
			for (auto& asset_lambda_struct : this->lambda_chain)
			{
				if (asset_lambda_struct.is_filter()) continue;
				auto& operation = asset_lambda_struct.get_asset_operation_struct();
				if (operation.row < min_row) { min_row = operation.row; }
			}
			this->warmup = abs(min_row);

			Q_EMIT dataUpdated(0);
			return;
		}
		
		case 1:{
			if (!data) 
			{
				this->exchange = nullptr; 
				Q_EMIT dataInvalidated(0);
				return; 
			}
			std::shared_ptr<ExchangeData> exchange_data = std::dynamic_pointer_cast<ExchangeData>(data);
			this->exchange = exchange_data->exchange_ptr;
			Q_EMIT dataUpdated(0);
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
			if (!data) 
			{
				this->ev_lambda_struct = std::nullopt;
				Q_EMIT dataInvalidated(0);
				return;
			}
			std::shared_ptr<ExchangeViewData> ev_ptr = std::dynamic_pointer_cast<ExchangeViewData>(data);
			this->ev_lambda_struct = ev_ptr->exchange_view_lambda;
			return;
		}
		case 1: {
			if (!data)
			{
				this->trade_exit = std::nullopt;
				Q_EMIT dataInvalidated(1);
				return;
			}
			std::shared_ptr<TradeExitData> trade_exit_data = std::dynamic_pointer_cast<TradeExitData>(data);
			this->trade_exit = trade_exit_data->trade_exit;
			return;
		}
	}
	NEXUS_THROW("unexpected out port");
}
