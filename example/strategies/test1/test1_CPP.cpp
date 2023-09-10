
// the following code is generated from an abstract strategy flow graph.
// EDIT IT AT YOUR OWN RISK 

#include "test1_CPP.h"

std::vector<AssetLambdaScruct> operations = { AssetLambdaScruct(AssetLambda(agis_init, [&](const AssetPtr& asset) {
			return asset->get_asset_feature("Close", 0);
		}),agis_init, "Close", 0)
, AssetLambdaScruct(AssetLambda(agis_divide, [&](const AssetPtr& asset) {
			return asset->get_asset_feature("Close", -1);
		}),agis_divide, "Close", -1)
};

void test1_CPP::build(){
	// set the strategies target exchanges
	this->exchange_subscribe("SPY_DAILY");
	this->exchange = this->get_exchange();
	
	this->set_beta_trace(true);
	this->set_beta_scale_positions(false);
	this->set_beta_hedge_positions(false);
	this->set_net_leverage_trace(true);
};

void test1_CPP::next(){
	if (this->exchange->__get_exchange_index() < this->warmup) { return; }

    auto& operationsRef = operations; // Create a reference to operations

	// define the lambda function the strategy will apply
	auto next_lambda = [&operationsRef](const AssetPtr& asset) -> AgisResult<double> {			
		return asset_feature_lambda_chain(
			asset, 
			operationsRef
		);
	};
		
	auto ev = this->exchange->get_exchange_view(
		next_lambda, 
		ExchangeQueryType::Default,
		5
	);

	ev.uniform_weights(1.000000);

	this->strategy_allocate(
		ev,
		0.000000,
		1,
		std::nullopt,
		AllocType::PCT
	);

	
};
