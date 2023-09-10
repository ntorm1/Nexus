// the following code is generated in order build the static strategy register from realized abstract strategies.
// the register exposes strategies to Nexus and they can be incorperated by clicking on the Link button in the top right. 
// Doing so will load in the AgisStrategy.dll that was generated from the compiling of the abstract strategies.
// EDIT IT AT YOUR OWN RISK ANY
// ANY CHANGES WILL BE OVERWRITEN ON THE NEXT COMPILE

#ifndef PCH_H
#define PCH_H
#define NOMINMAX 
#ifdef AGISSTRATEGY_EXPORTS // This should be defined when building the DLL
#  define AGIS_STRATEGY_API __declspec(dllexport)
#else
#  define AGIS_STRATEGY_API __declspec(dllimport)
#endif

#include "framework.h"
#include "json.hpp"
using json = nlohmann::json;

#include "AgisStrategyRegistry.h"

#include "strategies/test1/test1_CPP.h"
static bool registered = StrategyRegistry::registerStrategy("test1_CPP",
    [](PortfolioPtr const& p) {
        return std::make_unique<test1_CPP>(p);
    }
, "Portfolio1");



// Wrapper function to return the RegistryMap
extern "C" AGIS_STRATEGY_API StrategyRegistry::RegistryMap& getRegistryWrapper() {
    return StrategyRegistry::getRegistry();
};


// Wrapper function to return the ID RegistryMap
extern "C" AGIS_STRATEGY_API StrategyRegistry::PortfolioIdMap& getIDRegistryWrapper() {
    return StrategyRegistry::getIDMap();
};

#endif //PCH_H
