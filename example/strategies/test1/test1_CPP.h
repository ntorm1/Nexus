#pragma once

#ifdef AGISSTRATEGY_EXPORTS // This should be defined when building the DLL
#  define AGIS_STRATEGY_API __declspec(dllexport)
#else
#  define AGIS_STRATEGY_API __declspec(dllimport)
#endif

// the following code is generated from an abstract strategy flow graph.
// EDIT IT AT YOUR OWN RISK 
#include "AgisStrategy.h"

class test1_CPP : public AgisStrategy {
public:
	AGIS_STRATEGY_API test1_CPP (
        PortfolioPtr const portfolio_
    ) : AgisStrategy("test1_CPP", portfolio_, 1.000000) {
		this->strategy_type = AgisStrategyType::CPP;
		this->trading_window = std::nullopt;
	};

    AGIS_STRATEGY_API inline static std::unique_ptr<AgisStrategy> create_instance(
        PortfolioPtr const& portfolio_
    ) 
	{
        return std::make_unique<test1_CPP>(portfolio_);
    }

	AGIS_STRATEGY_API inline void reset() override {}

	AGIS_STRATEGY_API void build() override;

	AGIS_STRATEGY_API void next() override;

private:
	ExchangeViewOpp ev_opp_type = ExchangeViewOpp::UNIFORM;
	ExchangePtr exchange = nullptr;
	size_t warmup = 1;
};
