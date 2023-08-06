#include <fstream>
#include <cstdlib>
#include <direct.h> // For Windows chdir
#include "NexusEnv.h"
#include "NexusNode.h"
#include "NexusNodeModel.h"

//============================================================================
NexusEnv::NexusEnv()
{
	this->hydra = std::make_shared<Hydra>();
}


//============================================================================
void NexusEnv::load_env(std::string const& exe_path_, std::string const & env_name_)
{
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
}


//============================================================================
void NexusEnv::new_editor(TextEdit* new_editor)
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
std::optional<TextEdit*> NexusEnv::get_editor(QString const& file_name) const
{
	auto it = std::find_if(open_editors.begin(), open_editors.end(), [&](TextEdit* w) {
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
	auto it = std::find_if(open_editors.begin(), open_editors.end(), [&](TextEdit* w) {
		return w->get_file_name() == file_name;
		});
	if (it == this->open_editors.end())
	{
		throw std::runtime_error("attempting to remove non existing editor");
	}
	else
	{
		this->open_editors.erase(it);
	}
}


//============================================================================
std::optional<std::shared_ptr<Asset>> const NexusEnv::get_asset(std::string const& asset_id)
{
	return this->hydra->get_asset(asset_id);
}


//============================================================================
std::optional<AgisStrategyRef const> NexusEnv::get_strategy(std::string const& strategy_id)
{
	if (!this->hydra->strategy_exists(strategy_id)) { return std::nullopt; }
	auto strategy = this->hydra->get_strategy(strategy_id);
	return strategy;
}


//============================================================================
NexusStatusCode NexusEnv::new_exchange(
	const std::string& exchange_id,
	const std::string& source,
	const std::string& freq,
	const std::string& dt_format)
{
	qDebug() << "Building new exchange: " << exchange_id;
	auto res = this->hydra->new_exchange(
		exchange_id,
		source,
		string_to_freq(freq),
		dt_format
		);
	return res;
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
	this->hydra->new_portfolio(portfolio_id, result);
	return NexusStatusCode::Ok;
}


//============================================================================
NexusStatusCode NexusEnv::new_strategy(
	const std::string& portfolio_id,
	const std::string& strategy_id,
	const std::string& allocation)
{
	double result;
	try {
		result = std::stod(allocation);
	}
	catch (const std::invalid_argument& e) {
		return NexusStatusCode::InvalidArgument;
	}
	
	auto& portfolio = this->hydra->get_portfolio(portfolio_id);
	auto strategy = std::make_unique<AbstractAgisStrategy>(
		portfolio,
		strategy_id,
		result
	);
	this->hydra->register_strategy(std::move(strategy));

	return NexusStatusCode::Ok;
}


//============================================================================
NexusStatusCode NexusEnv::remove_exchange(const std::string& name)
{
	qDebug() << "Removing exchange: " << name;
	return this->hydra->remove_exchange(name);
}

NexusStatusCode NexusEnv::remove_portfolio(const std::string& name)
{
	qDebug() << "Removing exchange: " << name;
	return this->hydra->remove_portfolio(name);
}


//============================================================================
void NexusEnv::__run()
{
	AGIS_TRY(
		this->hydra->__reset();
		this->hydra->__run();
	);
}


//============================================================================
void NexusEnv::__compile()
{
	qDebug() << "============================================================================";
	qDebug() << "Compiling strategies...";
	auto strat_folder = this->env_path / "strategies";
	auto& strategies = this->hydra->__get_strategy_map().__get_strategies();

	// generate code for all abstract strategies
	for (auto& strategy_pair : strategies)
	{
		auto& strategy = strategy_pair.second;
		auto strat_path = strat_folder / strategy->get_strategy_id();
		if (!strategy->__is_abstract_class()) { continue; }

		auto* abstract_strategy = dynamic_cast<AbstractAgisStrategy*>(strategy.get());
		AGIS_TRY(abstract_strategy->code_gen(strat_path);)
	}

	// create build folder
	auto build_folder = this->env_path / "build";

	if (!fs::exists(build_folder))
	{
		fs::create_directories(build_folder);
	}

	// AgisCore dll file
	fs::path output_dir = this->env_path.parent_path().parent_path();
	fs::path agis_dll = output_dir / "AgisCore.lib";
	std::string agis_dll_str = agis_dll.string();
	std::replace(agis_dll_str.begin(), agis_dll_str.end(), '\\', '/');

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

# Set the path to the AgisCore DLL
set(AGIS_CORE_PATH "{AGIS_CORE_PATH}")

# Set the path to the adjacent folder
set(ADJACENT_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/strategies")

# Gather source files from the 'adjacent_folder' and its subdirectories
file(GLOB_RECURSE SOURCE_FILES "${ADJACENT_FOLDER}/*.cpp")

# Gather header files from the 'adjacent_folder' and its subdirectories
file(GLOB_RECURSE HEADER_FILES "${ADJACENT_FOLDER}/*.h")

# Create the shared library (DLL)
add_library(AgisStrategy SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_compile_definitions(AgisStrategy PRIVATE AGISSTRATEGY_EXPORTS)

# Include AgisCore header files 
target_include_directories(AgisStrategy PUBLIC
    "C:/Users/natha/OneDrive/Desktop/C++/AgisCore"
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

# Specify the output directory for the strategy DLL
set_target_properties(AgisStrategy PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "{OUTPUT_DIR}"
)
)";

	// Replace the placeholder with the BUILD_METHOD
	auto pos = cmake_content.find("{AGIS_CORE_PATH}");
	cmake_content.replace(pos, 16, agis_dll_str);

	// Replace the output dir
	pos = cmake_content.find("{OUTPUT_DIR}");
	cmake_content.replace(pos, 12, output_dir.string());

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

	// generate cmake build files
	// Define the specific folder where you want to build the CMake files
	std::string build_folder_str = build_folder.string();
	std::replace(build_folder_str.begin(), build_folder_str.end(), '\\', '/');
	auto cd_command = "cd " + build_folder_str;

	// Concatenate the commands using the && operator
	std::string full_command = cd_command + " && cmake -G \"Visual Studio 17 2022\" .. && cmake --build .";

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
void NexusEnv::clear()
{
	this->remove_editors();
	this->reset_trees();
	this->hydra->clear();
}


//============================================================================
void NexusEnv::restore(json const& j)
{
	this->hydra->clear();
	AGIS_TRY(this->hydra->restore(j);)

	this->remove_editors();
	for (auto& tree : this->open_trees)
	{
		tree->restore_tree(j);
	}

	// restore abstract strategy tree
	auto strat_folder = this->env_path / "strategies";
	auto& strategy_map = this->hydra->__get_strategy_map();
	auto& strategies = strategy_map.__get_strategies();

	// node currently widgets are created for each one in order to restore the strategy
	// probably way to this without creating them.
	for (auto& strategy_pair : strategies)
	{
		auto& strategy = strategy_pair.second;
		auto strat_path = strat_folder / strategy->get_strategy_id() / "graph.flow";
		
		// use node model to load in flow graph to set absract strategy lambda
		std::shared_ptr<NodeDelegateModelRegistry> registry = registerDataModels();
		DataFlowGraphModel dataFlowGraphModel = DataFlowGraphModel(registry);
		ExchangeModel::hydra = this->get_hydra();

		QFile file(strat_path);
		if (!file.open(QIODevice::ReadOnly)) {
			AGIS_THROW("Failed to open the strategy flow file: " + strat_path.string());
		}

		QByteArray const wholeFile = file.readAll();
		file.close();

		QJsonParseError error;
		QJsonDocument jsonDocument = QJsonDocument::fromJson(wholeFile, &error);
		if (error.error != QJsonParseError::NoError) {
			AGIS_THROW("Failed to parse JSON in the strategy flow file: " + error.errorString().toStdString());
		}

		if (!jsonDocument.isObject()) {
			AGIS_THROW("Invalid JSON format in the strategy flow file.");
		}
		dataFlowGraphModel.load(jsonDocument.object());

		// case to abstract strategy and extract strategy
		auto abstract_strategy = dynamic_cast<AbstractAgisStrategy*>(strategy.get());
		abstract_strategy->set_abstract_ev_lambda([&dataFlowGraphModel]() {
			return NexusNodeEditor::__extract_abstract_strategy(&dataFlowGraphModel);
		});
		abstract_strategy->extract_ev_lambda();
	}
}


//============================================================================
bool NexusEnv::save_env(json& j)
{
	qDebug() << "Saving environemnt...";

	// Save the current open editors and the files they have open
	json open_editors;
	for (const auto& editor : this->open_editors)
	{
		json editor_json;
		auto open_file = editor->get_file_name().toStdString();
		if (open_file == "empty")
		{
			continue;
		}

		editor_json["widget_id"] = editor->get_id();
		editor_json["open_file"] = open_file;
		open_editors.push_back(editor_json);
	}
	j["open_editors"] = open_editors;

	// Save the current hydra sate;
	qDebug() << "Serializing hydra state...";
	this->hydra->save_state(j);
	qDebug() << "Hydra state serialized";

	// Save the current state of the trees
	json trees;
	for (const auto& tree : this->open_trees)
	{
		trees[tree->objectName().toStdString()] = tree->to_json();
	}
	j["trees"] = trees;

	// Dump ths json output to a file
	std::string jsonString = j.dump(4);
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