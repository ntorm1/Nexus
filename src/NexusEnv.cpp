#include "NexusPch.h"
#include <fstream>
#include <cstdlib>
#include "NexusEnv.h"
#include "NexusNode.h"
#include "NexusNodeModel.h"
#include <AgisStrategyRegistry.h>

#ifdef USE_LUAJIT
#include "AgisLuaStrategy.h"
#endif

#ifdef _DEBUG
std::string build_method = "debug";
#else
std::string build_method = "release";
#endif

#include "Portfolio.h"
#include "Asset/Asset.h"

using namespace Agis;

//============================================================================
const std::vector<std::string> nexus_datetime_columns = {
	"Order Create Time",
	"Order Fill Time",
	"Order Cancel Time",
	"Trade Open Time",
	"Trade Close Time"
};


//============================================================================
NexusEnv::NexusEnv() : hydra(Hydra())
{
	this->hydra.new_broker("test");
}


//============================================================================
NexusEnv::~NexusEnv()
{
	if (this->agis_strategy_dll_loaded)
	{
		FreeLibrary(this->AgisStrategyDLL);
	}
}


//============================================================================
void NexusEnv::set_env_name(std::string const& exe_path_, std::string const & env_name_)
{
	qDebug() << "LOADING ENV: " + env_name_;
	this->env_name = env_name_;
	fs::path exe_parent_dir_path(exe_path_);

	// Check to see of env master folder exists yet
	auto env_parent_path = exe_parent_dir_path / "envs";
	if (!fs::exists(env_parent_path))
	{
		fs::create_directory(env_parent_path);
	}

	// Create folder for the actual env being created
	this->env_path = env_parent_path / env_name;
	if (!fs::exists(this->env_path))
	{
		fs::create_directory(this->env_path);
	}
	qDebug() << "LOADING ENV: " + env_name_ + " COMPLETE";
}


//============================================================================
AgisResult<bool> NexusEnv::new_node_editor(std::string strategy_id)
{
	// if strategy id is in open_node_editors, return false
	auto it = std::find_if(open_node_editors.begin(), open_node_editors.end(), [&](std::string const& id) {
		return id == strategy_id;
	});
	if (it != this->open_node_editors.end())
	{
		return AgisResult<bool>(AGIS_EXCEP("strategy node editor is already open"));
	}
	this->open_node_editors.push_back(strategy_id);
	return AgisResult<bool>(true);
}

//============================================================================
void NexusEnv::new_editor(QScintillaEditor* new_editor)
{
	this->open_editors.push_back(new_editor);
}


//============================================================================
void NexusEnv::new_tree(NexusTree* new_tree)
{
	this->open_trees.push_back(new_tree);
}


//============================================================================
void NexusEnv::reset_trees()
{
	for (auto& tree : this->open_trees)
	{
		tree->reset_tree();
	}
}


//============================================================================
bool NexusEnv::editor_open(QString const& file)
{
	for (auto editor : this->open_editors)
	{
		if (editor->get_file_name() == file)
		{
			return true;
		}
	}
	return false;
}


//============================================================================
std::optional<QScintillaEditor*> NexusEnv::get_editor(QString const& file_name) const
{
	auto it = std::find_if(open_editors.begin(), open_editors.end(), [&](QScintillaEditor* w) {
		return w->get_file_name() == file_name;
		});
	if (it != this->open_editors.end())
	{
		return *it;
	}
	else {
		return std::nullopt;
	}
}


//============================================================================
void NexusEnv::remove_editor(QString const& file_name)
{
	auto it = std::find_if(open_editors.begin(), open_editors.end(), [&](QScintillaEditor* w) {
		return w->get_file_name() == file_name;
		});
	if (it == this->open_editors.end())
	{
		AGIS_THROW("attempting to remove non existing editor");
	}
	else
	{
		this->open_editors.erase(it);
	}
}


//============================================================================
void NexusEnv::remove_node_editor(std::string const& id)
{
	// remove id from open_node_editors
	auto it = std::find_if(open_node_editors.begin(), open_node_editors.end(), [&](std::string const& s) {
		return s == id;
	});	
	if (it == this->open_node_editors.end())
	{
		AGIS_THROW("attempting to remove non existing node editor");
	}
	else
	{
		this->open_node_editors.erase(it);
	}
}


const Hydra* NexusEnv::get_hydra() const
{
	auto ret = &this->hydra; 
	return ret;
}

//============================================================================
AgisResult<AssetPtr> const NexusEnv::get_asset(std::string const& asset_id)
{
	return this->hydra.get_asset(asset_id);
}


//============================================================================
std::optional<AgisStrategy*> NexusEnv::__get_strategy(std::string const& strategy_id)
{
	if (!this->hydra.strategy_exists(strategy_id)) { return std::nullopt; }
	auto strategy = this->hydra.__get_strategy(strategy_id);
	return strategy;
}

std::vector<std::string> NexusEnv::get_portfolio_ids()
{
	auto& portfolios = this->hydra.get_portfolios();
	return portfolios.get_portfolio_ids();
}



//============================================================================
AgisResult<bool> NexusEnv::new_exchange(
	const std::string& exchange_id,
	const std::string& source,
	const std::string& freq,
	const std::string& dt_format,
	std::optional<std::shared_ptr<MarketAsset>> market_asset
)
{
	qDebug() << "Building new exchange: " << exchange_id;
	return this->hydra.new_exchange(
		AssetType::US_EQUITY, // TODO fix this
		exchange_id,
		source,
		StringToFrequency(freq),
		dt_format,
		std::nullopt,
		market_asset
		);
}


//============================================================================
NexusStatusCode NexusEnv::new_portfolio(const std::string& portfolio_id, const std::string& starting_cash)
{
	double result;
	try {
		result = std::stod(starting_cash);
	}
	catch (const std::invalid_argument& e) {
		return NexusStatusCode::InvalidArgument;
	}
	this->hydra.new_portfolio(portfolio_id, result);
	return NexusStatusCode::Ok;
}


//============================================================================
NexusStatusCode NexusEnv::new_strategy(
	const std::string& portfolio_id,
	const std::string& strategy_id,
	const std::string& allocation,
	AgisStrategyType strategy_type
)
{
	auto broker = this->hydra.get_broker("test").value();

	double allocation_double;
	try {
		allocation_double = std::stod(allocation);
	}
	catch (const std::invalid_argument& e) {
		return NexusStatusCode::InvalidArgument;
	}
	
	auto& portfolio = this->hydra.get_portfolio(portfolio_id);
	// if new flow strategy was requested create a new abstract agis strategy
	if (strategy_type == AgisStrategyType::FLOW) {

		auto strategy = std::make_unique<AbstractAgisStrategy>(
			portfolio,
			broker,
			strategy_id,
			allocation_double
		);
		AGIS_TRY(this->hydra.register_strategy(std::move(strategy)));
	}
	else if (strategy_type == AgisStrategyType::LUAJIT) {
		auto parent_dir = this->get_env_path() / "strategies" / strategy_id;
		auto script_path = parent_dir / (strategy_id + ".lua");
		if (!fs::exists(script_path)) {
			if (!fs::exists(parent_dir))
			{
				fs::create_directory(parent_dir);
			}
			std::ofstream fileStream(script_path.string());
			auto script = AgisLuaStrategy::get_script_template(strategy_id);
			// Check if the file was opened successfully
			if (fileStream.is_open()) {
				// Write the script into the file
				fileStream << script;
				// Close the file after writing
				fileStream.close();
			}
			else {
				AGIS_THROW("Failed to open file: " + script_path.string());
			}
		}
		auto strategy = std::make_unique<AgisLuaStrategy>(
			portfolio,
			broker,
			strategy_id,
			allocation_double,
			script_path
		);
		AGIS_TRY(this->hydra.register_strategy(std::move(strategy)));
	}
	// if benchmark strategy was request create a new agis benchmark strategy for the portfolio.
	// note that this strategy won't affect the portfolio's values or holdings
	else if (strategy_type == AgisStrategyType::BENCHMARK) {

		auto strategy = std::make_unique<BenchMarkStrategy>(
			portfolio,
			broker,
			strategy_id
		);
		AGIS_TRY(this->hydra.register_strategy(std::move(strategy)));
	}

	return NexusStatusCode::Ok;
}


//============================================================================
NexusStatusCode NexusEnv::remove_exchange(const std::string& name)
{
	qDebug() << "Removing exchange: " << name;
	return this->hydra.remove_exchange(name);
}


//============================================================================
NexusStatusCode NexusEnv::remove_portfolio(const std::string& name)
{
	qDebug() << "Removing exchange: " << name;
	return this->hydra.remove_portfolio(name);
}


//============================================================================
NexusStatusCode NexusEnv::remove_strategy(const std::string& name)
{
	if (!this->hydra.strategy_exists(name)) return NexusStatusCode::InvalidArgument;
	this->hydra.remove_strategy(name);
	return NexusStatusCode::Ok;
}


//============================================================================
[[nodiscard]] AgisResult<bool> NexusEnv::set_market_asset(
	std::string const& exchange_id, 
	std::string const& asset_id,
	bool disable,
	std::optional<size_t> beta_lookback)
{
	// forward all arguments to this->exchange_map.set_market_asset
	return this->hydra.set_market_asset(exchange_id, asset_id, disable, beta_lookback);
}


//============================================================================
AgisResult<bool> NexusEnv::__run()
{
	auto res = this->hydra.__run();
	if (!res.has_value()) {
		return AgisResult<bool>(res.error());
	}
	return AgisResult<bool>(true);
}


//============================================================================
void NexusEnv::__save_history()
{
	this->order_history.clear();
	this->trade_history.clear();
	this->position_history.clear();

	// load in the orders, trades, positions
	auto& order_history = this->hydra.get_order_history();
	for (auto& order : order_history)
	{
		this->order_history.push_back(order);
	}
	PortfolioMap const& portfolios = this->hydra.get_portfolios();
	for (auto& portfolio_id : portfolios.get_portfolio_ids())
	{
		auto portfolio_ptr = portfolios.get_portfolio(portfolio_id);
		auto& trades = portfolio_ptr.get()->get_trade_history();
		for (auto& trade : trades)
		{
			this->trade_history.push_back(trade);
		}
		auto& position_history = portfolio_ptr.get()->get_position_history();
		for (auto& position : position_history)
		{
			this->position_history.push_back(position);
		}
	}
}


//============================================================================
void NexusEnv::__compile()
{
	qDebug() << "============================================================================";
	qDebug() << "Compiling strategies...";
	auto strat_folder = this->env_path / "strategies";
	auto& strategies = this->hydra.__get_strategy_map().__get_strategies();

	// generate code for all abstract strategies
	for (auto& strategy_pair : strategies)
	{
		auto& strategy = strategy_pair.second;
		auto strat_path = strat_folder / strategy->get_strategy_id();
		if (!strategy->__is_abstract_class()) { continue; }

		auto* abstract_strategy = dynamic_cast<AbstractAgisStrategy*>(strategy.get());
		AGIS_TRY(abstract_strategy->code_gen(strat_path);)
	}

	// include strategy classes
	std::string strategy_include = R"()";
	std::string strategy_create = R"(
    [](PortfolioPtr const& p) {
        return std::make_unique<{STRAT}>(p);
    }
)";
	// add include paths for pch.h
	for (auto& strategy_pair : strategies)
	{
		// skip non-live strategies
		if (!strategy_pair.second->__is_live()) continue;
		auto t = strategy_pair.second->get_strategy_type();
		if (t == AgisStrategyType::BENCHMARK) continue;
		if (t == AgisStrategyType::LUAJIT) continue;
		bool is_abstract = strategy_pair.second->__is_abstract_class();

		auto strategy_id = strategy_pair.second->get_strategy_id();
		// if strategy_id ends in the characters "_CPP", remove them

		if (strategy_id.size() >= 4 && strategy_id.substr(strategy_id.size() - 4) == "_CPP")
		{
			strategy_id = strategy_id.substr(0, strategy_id.size() - 4);
		}
		std::string strat_include_mid = "#include \"strategies/" + strategy_id + "/"
			+ strategy_pair.second->get_strategy_id();
		if (is_abstract) strat_include_mid += +"_CPP.h\"\n";
		else strat_include_mid += +".h\"\n";
		
		// register the strategy to the registry
		strat_include_mid += "static bool registered_{STRAT} = StrategyRegistry::registerStrategy(\"{STRAT}\"," + \
			strategy_create + ", \"{PORTFOLIO}\");\n";
		std::string strategy_class = strategy_pair.second->get_strategy_id();
		if (is_abstract) strategy_class += "_CPP";
		str_replace_all(strat_include_mid, "{STRAT}", strategy_class);

		// Set the static portfolio id
		auto pos = strat_include_mid.find("{PORTFOLIO}");
		strat_include_mid.replace(pos, 11, strategy_pair.second->get_portfolio_id());

		strategy_include += strat_include_mid;
	}

	std::string pch_header = R"(// the following code is generated in order build the static strategy register from realized abstract strategies.
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

{STRATEGY}


// Wrapper function to return the RegistryMap
extern "C" AGIS_STRATEGY_API StrategyRegistry::RegistryMap& getRegistryWrapper() {
    return StrategyRegistry::getRegistry();
};


// Wrapper function to return the ID RegistryMap
extern "C" AGIS_STRATEGY_API StrategyRegistry::PortfolioIdMap& getIDRegistryWrapper() {
    return StrategyRegistry::getIDMap();
};

#endif //PCH_H
)";
	auto pos = pch_header.find("{STRATEGY}");
	pch_header.replace(pos, 10, strategy_include);
	std::string dll_main = R"(#include "pch.h"


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        break;
    }

    return TRUE;
}
)";
	AGIS_TRY(code_gen_write(this->env_path / "dllmain.cpp", dll_main);)
	AGIS_TRY(code_gen_write(this->env_path / "pch.h", pch_header);)

	// create build folder
	auto build_folder = this->env_path / "build";

	if (!fs::exists(build_folder))
	{
		fs::create_directories(build_folder);
	}

	// CMake file contents
	// cmake -G "Visual Studio 17 2022" ..
	std::string cmake_content = R"(
cmake_minimum_required(VERSION 3.26)

# Enable C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(AgisStrategies)

# Include the Vcpkg toolchain file
set(VCPKG_TOOLCHAIN_FILE "C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
include_directories("C:/dev/vcpkg/installed/x64-windows/include")

# Set the path to the AgisCore lib
set(AGIS_CORE_PATH "{AGIS_CORE_PATH}")

# Set the path to the adjacent folder
set(ADJACENT_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/strategies")

# Gather source files from the 'adjacent_folder' and its subdirectories
file(GLOB_RECURSE SOURCE_FILES "${ADJACENT_FOLDER}/*.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/dllmain.cpp")

# Gather header files from the 'adjacent_folder' and its subdirectories
file(GLOB_RECURSE HEADER_FILES "${ADJACENT_FOLDER}/*.h")
list(APPEND HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

# Create the shared library (DLL)
add_library(AgisStrategy SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_compile_definitions(AgisStrategy PRIVATE AGISSTRATEGY_EXPORTS)

# Include AgisCore header files 
target_include_directories(AgisStrategy PUBLIC
    "{AGIS_CORE_INCLUDE}"
)
target_include_directories(AgisStrategy PUBLIC
    "{AGIS_CORE_INCLUDE}/external/include"
)

# Windows-specific configurations
if (WIN32)
    # Export symbols to create a .def file (needed for Windows)
    target_compile_definitions(AgisStrategy PRIVATE MYDLL_EXPORTS)
	
	# Link the AgisCore DLL to the strategy DLL (use .lib for MSVC)
	if (MSVC)
		target_link_libraries(AgisStrategy PRIVATE "${AGIS_CORE_PATH}")
	else ()
		message(FATAL_ERROR "AgisStrategy can only be built with MSVC")
	endif()

else ()
    # Throw an error if the target platform is not Windows (WIN32)
    message(FATAL_ERROR "AgisStrategy requires a Windows (WIN32) platform.")
endif()

# Installation to the build directory
install(TARGETS AgisStrategy
    LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/lib
    ARCHIVE DESTINATION ${CMAKE_BINARY_DIR}/lib
    RUNTIME DESTINATION ${CMAKE_BINARY_DIR}/bin
)

)";

	// Replace the placeholder with the BUILD_METHOD
	pos = cmake_content.find("{AGIS_CORE_PATH}");
	cmake_content.replace(pos, 16, this->agis_lib_path);

	// Replace the adis include path
	str_replace_all(cmake_content, "{AGIS_CORE_INCLUDE}", this->agis_include_path);


	// create cmake file
	auto cmake_file = this->env_path / "CmakeLists.txt";

	std::ofstream file(cmake_file);
	if (file.is_open()) {
		file << cmake_content;
		file.close();
	}
	else {
		AGIS_THROW("Failed to generate CMake file");
	}

	// if agis_strategy_dll_loaded is true, unload the dll
	if (this->agis_strategy_dll_loaded) {
		FreeLibrary(this->AgisStrategyDLL);
	}

	// generate cmake build files
	// Define the specific folder where you want to build the CMake files
	std::string build_folder_str = build_folder.string();
	std::replace(build_folder_str.begin(), build_folder_str.end(), '\\', '/');
	auto cd_command = "cd " + build_folder_str;

	// Concatenate the commands using the && operator
	std::string full_command = cd_command + " && \
		cmake -G " + this->agis_build_method + " .. && \
		cmake --build . --config " + build_method;

	qDebug() << "==== Building AgisStrategy CMake ====";
	qDebug() << full_command;

	int result = std::system(full_command.c_str());

	if (result != 0) {
		AGIS_THROW("Failed to generate CMake build files or build AgisStrategy dll");
	}
	qDebug() << "========================";

	qDebug() << "Compiling strategies complete";
	qDebug() << "============================================================================";
}


//============================================================================
LPCWSTR StringToLPCWSTR(const std::string& str) {
	// Calculate the required buffer size for the wide character string
	int bufferSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

	// Allocate memory for the wide character string
	wchar_t* wideCharString = new wchar_t[bufferSize];

	// Convert the multi-byte string to wide character string
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wideCharString, bufferSize);

	// Return the wide character string
	return wideCharString;
}


//============================================================================
void NexusEnv::__link(bool assume_live)
{
	// before attempting to link reset hydra to reset the cash levels of the portfolio 
	this->__reset();

	qDebug() << "============================================================================";
	qDebug() << "Linking strategies...";
	// AgisCore dll file
	fs::path output_dir = this->env_path / "build" / build_method;
	fs::path agis_strategy_dll = output_dir / "AgisStrategy.dll";

	this->AgisStrategyDLL = LoadLibrary(StringToLPCWSTR(agis_strategy_dll.string()));
	if (!AgisStrategyDLL) AGIS_THROW("Failed to locate AgisStrategy.dll");

	// Get the function pointer for getRegistry
	using GetRegistryWrapperFunc = StrategyRegistry::RegistryMap & (*)();
	using GetIDRegistryWrapperFunc = StrategyRegistry::PortfolioIdMap& (*)();

	GetRegistryWrapperFunc getRegistryWrapperFunc = reinterpret_cast<GetRegistryWrapperFunc>(GetProcAddress(AgisStrategyDLL, "getRegistryWrapper"));
	GetIDRegistryWrapperFunc getIDRegistryWrapperFunc = reinterpret_cast<GetIDRegistryWrapperFunc>(GetProcAddress(AgisStrategyDLL, "getIDRegistryWrapper"));
	if (!getRegistryWrapperFunc || !getIDRegistryWrapperFunc)
	{
		AGIS_THROW("Failed to get function pointer for strategy registries");
	}

	StrategyRegistry::RegistryMap& registryMap = getRegistryWrapperFunc();
	StrategyRegistry::PortfolioIdMap& IDRegistryMap = getIDRegistryWrapperFunc();
	// Now you can access all the classes in the registryMap and do whatever you need with them
	for (const auto& entry : registryMap)
	{
		const std::string& strategy_id = entry.first;
		qDebug() << "Linking strategy: " + strategy_id;
		
		std::string portfolio_id = IDRegistryMap.at(strategy_id);
		// make sure the portfolio that strategy tries to link to exists
		if (!this->hydra.portfolio_exists(portfolio_id))
		{
			AGIS_THROW("Attempting to link strategy to portfolio: " + portfolio_id + " doesn't eixst");
		}
		// replace existing strategies with the new class
		if (this->hydra.strategy_exists(strategy_id))
		{
			this->hydra.remove_strategy(strategy_id);
		}

		// build the new strategy class
		auto& portfolio = this->hydra.get_portfolio(portfolio_id);
		auto strategy = entry.second(portfolio);

		// set the strategy to live if assume_live is true
		if (!assume_live) {
			strategy->set_is_live(false);
		}

		AGIS_TRY(this->hydra.register_strategy(std::move(strategy)));

		// check if the linked strategy is replacing abstract strategy
		std::string sub_string = "_CPP";
		if (strategy_id.length() >= sub_string.length() &&
			strategy_id.compare(strategy_id.length() - sub_string.length(), sub_string.length(), sub_string) == 0) {
			// Extract the "test" part
			std::string abstract_strategy_id = strategy_id.substr(0, strategy_id.length() - sub_string.length());
			
			if (this->hydra.strategy_exists(abstract_strategy_id))
			{
				hydra.__set_strategy_is_live(abstract_strategy_id, false);
				qDebug() << "Disabling abstract strategy: " + abstract_strategy_id;
			}
		}

		qDebug() << "Strategy: " + strategy_id + " linked";
	}
	this->agis_strategy_dll_loaded = true;
	qDebug() << "Linking strategies complete";
	qDebug() << "============================================================================";
}


//============================================================================
void NexusEnv::__reset()
{
	this->hydra.__reset();
}


//============================================================================
void NexusEnv::clear()
{
	this->remove_editors();
	this->reset_trees();
	this->hydra.clear();
}


//============================================================================
AgisResult<bool> NexusEnv::restore_strategies(const rapidjson::Document& j)
{
	// Restore CPP strategy tree by linking to all strats if the AgisStrategy library
	fs::path output_dir = this->env_path / "build" / build_method;
	fs::path agis_strategy_dll = output_dir / "AgisStrategy.dll";

	// If agis_strategy_dll exists, call __link
	if (fs::exists(agis_strategy_dll))
	{
		AGIS_TRY_RESULT(this->__link(false), bool);
	}

	// For all portfolios loaded, check if their strategies are live
	const rapidjson::Value& portfolios = j["portfolios"];
	for (rapidjson::Value::ConstMemberIterator portfolio = portfolios.MemberBegin(); portfolio != portfolios.MemberEnd(); ++portfolio)
	{
		const rapidjson::Value& portfolio_json = portfolio->value;
		const rapidjson::Value& strategies = portfolio_json["strategies"];
		for (rapidjson::Value::ConstValueIterator strategy = strategies.Begin(); strategy != strategies.End(); ++strategy)
		{
			bool is_live = (*strategy)["is_live"].GetBool();
			const char* strategy_id = (*strategy)["strategy_id"].GetString();

			if (!this->hydra.strategy_exists(strategy_id))
			{
				continue;
			}

			auto strategy_ptr = this->hydra.get_strategy(strategy_id);

			// If strategy was linked but it is not live, remove it and force it to be re-linked
			// if we actually want to load it.
			if (!is_live && !strategy_ptr->__is_abstract_class())
			{
				this->hydra.remove_strategy(strategy_id);
			}
			else
			{
				hydra.__set_strategy_is_live(strategy_id, is_live);
			}
		}
	}

	// restore abstract strategy tree
	auto strat_folder = this->env_path / "strategies";
	auto& strategy_map = this->hydra.__get_strategy_map();
	auto& strategies = strategy_map.__get_strategies();

	// node currently widgets are created for each one in order to restore the strategy
	// probably way to this without creating them.
	for (auto& strategy_pair : strategies)
	{
		auto& strategy = strategy_pair.second;
		if (!strategy->__is_abstract_class()) { continue; }
		auto strat_path = strat_folder / strategy->get_strategy_id() / "graph.flow";
		
		// use node model to load in flow graph to set absract strategy lambda
		std::shared_ptr<NodeDelegateModelRegistry> registry = registerDataModels();
		DataFlowGraphModel dataFlowGraphModel = DataFlowGraphModel(registry);
		ExchangeModel::hydra = this->get_hydra();

		QFile file(strat_path);
		if (!file.open(QIODevice::ReadOnly)) {
			auto msg = "Failed to open the strategy flow file: " + strat_path.string();
			return AgisResult<bool>(AGIS_EXCEP(msg));
		}

		QByteArray const wholeFile = file.readAll();
		file.close();

		QJsonParseError error;
		QJsonDocument jsonDocument = QJsonDocument::fromJson(wholeFile, &error);
		if (error.error != QJsonParseError::NoError) {
			auto msg = "Failed to parse JSON in the strategy flow file: " + error.errorString().toStdString();
			return AgisResult<bool>(AGIS_EXCEP(msg));
		}

		if (!jsonDocument.isObject()) {
			return AgisResult<bool>(AGIS_EXCEP("Invalid JSON format in the strategy flow file."));
		}
		dataFlowGraphModel.load(jsonDocument.object());

		// case to abstract strategy and extract strategy
		auto abstract_strategy = dynamic_cast<AbstractAgisStrategy*>(strategy.get());
		abstract_strategy->set_abstract_ev_lambda([&dataFlowGraphModel]() {
			return NexusNodeEditor::__extract_abstract_strategy(&dataFlowGraphModel);
		});
		auto res = abstract_strategy->extract_ev_lambda();
		if (res.is_exception()) abstract_strategy->set_is_live(false);
		qDebug() << "Disabling abstract strategy, invalid flow graph: " + strategy->get_strategy_id();
	}

	// build the hydra instance to allow for the strategy map to get populated
	auto res = this->hydra.build();
	if (!res.has_value()) return AgisResult<bool>(res.error());

	this->remove_editors();
	for (auto& tree : this->open_trees)
	{
		tree->restore_tree(j);
	}
	
	return AgisResult<bool>(true);
}


//============================================================================
AgisResult<bool> NexusEnv::restore_settings(rapidjson::Document const& j)
{
	if (j.HasMember("agis_include_path") && j["agis_include_path"].IsString())
	{
		this->agis_include_path = j["agis_include_path"].GetString();
	}

	if (j.HasMember("agis_lib_path") && j["agis_lib_path"].IsString())
	{
		this->agis_lib_path = j["agis_lib_path"].GetString();
	}

	if (j.HasMember("agis_pyd_path") && j["agis_pyd_path"].IsString())
	{
		this->agis_pyd_path = j["agis_pyd_path"].GetString();
	}

	return AgisResult<bool>(true);
}

//============================================================================
AgisResult<bool> NexusEnv::set_settings(NexusSettings* nexus_settings)
{
	// ===== agis core include =====
	auto q_string = nexus_settings->get_agis_include_path();
	if (q_string.isEmpty() || !fs::exists(q_string.toStdString()))
	{
		return AgisResult<bool>(AGIS_EXCEP("Invalid AGIS include path: " + q_string.toStdString()));
	}
	else this->agis_include_path = q_string.toStdString();

	// ===== agis dll file =====
	q_string = nexus_settings->get_agis_lib_path();
	if (q_string.isEmpty() || !fs::exists(q_string.toStdString()))
	{
		return AgisResult<bool>(AGIS_EXCEP("Invalid AGIS DLL path: " + q_string.toStdString()));
	}
	else if (q_string.right(4) != ".dll")
	{
		return AgisResult<bool>(AGIS_EXCEP("Invalid AGIS DLL path: " + q_string.toStdString()));
	}
	else this->agis_lib_path = q_string.toStdString();

	// ===== agis pyd file =====
	q_string = nexus_settings->get_agis_pyd_path();
	if (q_string.isEmpty() || !fs::exists(q_string.toStdString()))
	{
		return AgisResult<bool>(AGIS_EXCEP("Invalid AGIS PYD path: " + q_string.toStdString()));
	}
	else if (q_string.right(4) != ".pyd")
	{
		return AgisResult<bool>(AGIS_EXCEP("Invalid AGIS PYD path: " + q_string.toStdString()));
	}
	else this->agis_pyd_path = q_string.toStdString();

	// ===== build method =====
	q_string = nexus_settings->get_vs_version();
	if (q_string.isEmpty())
	{
		return AgisResult<bool>(AGIS_EXCEP("Invalid VS version: " + q_string.toStdString()));
	}
	else this->agis_build_method = "\"" + q_string.toStdString() + "\"";

	return AgisResult<bool>(true);
}


//============================================================================
bool NexusEnv::save_env(rapidjson::Document &j)
{
	rapidjson::Document::AllocatorType& allocator = j.GetAllocator();

	// Save the current open editors and the files they have open
	rapidjson::Value open_editors(rapidjson::kArrayType);
	for (const auto& editor : this->open_editors)
	{
		if (editor->get_file_name() != "empty")
		{
			rapidjson::Value editor_json(rapidjson::kObjectType);
			editor_json.AddMember("widget_id", editor->get_id(), allocator);
			editor_json.AddMember("open_file", rapidjson::StringRef(editor->get_file_name().toStdString().c_str()), allocator);
			open_editors.PushBack(editor_json, allocator);
		}
	}
	j.AddMember("open_editors", open_editors, allocator);

	// Serialize hydra state
	Document hydra_state;
	qDebug() << "Serializing hydra state...";
	this->hydra.save_state(hydra_state);
	qDebug() << "Hydra state serialized";
	j.AddMember("hydra_state", hydra_state, allocator);

	// Save the current state of the trees
	rapidjson::Value trees(rapidjson::kObjectType);
	for (const auto& tree : this->open_trees)
	{
		trees.AddMember(rapidjson::StringRef(tree->objectName().toStdString().c_str()), tree->to_json(), allocator);
	}
	j.AddMember("trees", trees, allocator);

	// Save the nexus settings
	j.AddMember("agis_include_path", rapidjson::StringRef(this->agis_include_path.c_str()), allocator);
	j.AddMember("agis_lib_path", rapidjson::StringRef(this->agis_lib_path.c_str()), allocator);
	j.AddMember("agis_pyd_path", rapidjson::StringRef(this->agis_pyd_path.c_str()), allocator);

	// Dump the JSON output to a file
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	j.Accept(writer);

	std::string jsonString = buffer.GetString();
	auto json_path = this->env_path / "env_settings.json";
	std::ofstream outputFile(json_path.string());
	if (outputFile.is_open()) {
		outputFile << jsonString;
		outputFile.close();
		qDebug() << "Environemnt saved";
		return true;
	}
	else {
		qDebug() << "Environemnt failed to saved";
		return false;
	}
}