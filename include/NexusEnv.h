#pragma once
#include "NexusPch.h"
#include <windows.h>
#include <filesystem>
#include <unordered_map>

#include "CodeEditor.h"
#include "NexusTree.h"

#include "AgisPointers.h"
#include "AgisErrors.h"
#include "Hydra.h"
#include "Asset.h"

namespace fs = std::filesystem;

/// <summary>
/// Datetime format to use when displaying long long ns epoch times
/// </summary>
constexpr auto NEXUS_DATETIME_FORMAT = "%F %T";

/// <summary>
/// A list of columns to parse as datetime columns when loading ns epoch times
/// </summary>
extern const std::vector<std::string> nexus_datetime_columns;

class NexusEnv
{
private:

	void remove_editors() { this->open_editors.clear(); }

	/// <summary>
	/// Vector of pointers to text editors currently open.
	/// </summary>
	std::vector<TextEdit*> open_editors;

	/// <summary>
	/// Vector of pointers to trees that are currently open
	/// </summary>
	std::vector<NexusTree*> open_trees;

	/// <summary>
	/// Vector of names of strategies whose node editors are currently open
	/// </summary>
	std::vector<std::string> open_node_editors;

	/// <summary>
	/// Shared pointer to a hydra instance
	/// </summary>
	std::shared_ptr<Hydra> hydra;

	std::string env_name;
	fs::path env_path;


	/// <summary>
	/// AgisStrategy dll
	/// </summary>
	HINSTANCE AgisStrategyDLL;
	bool agis_strategy_dll_loaded = false;

	std::vector<SharedOrderPtr> order_history;
	std::vector<SharedPositionPtr> position_history;
	std::vector<SharedTradePtr> trade_history;

public:
	NexusEnv();
	~NexusEnv();

	[[nodiscard]] AgisResult<bool> __run();
	void __save_history();
	void __compile();
	void __link(bool assume_live = true);
	void __reset();
	void clear();
	void restore(json const & j);

	//============================================================================
	fs::path const& get_env_path() const { return this->env_path; }
	fs::path get_env_settings_path() const { return this->env_path / "env_settings.json"; }
	bool save_env(json &j);
	void load_env(std::string const & exe_path, std::string const & env_name);

	//============================================================================
	[[nodiscard]] AgisResult<bool> new_node_editor(std::string strategy_id);
	void new_editor(TextEdit* new_editor);
	std::optional<TextEdit*> get_editor(QString const & file_name) const;
	void remove_editor(QString const& file_name);
	void remove_node_editor(std::string const& id);
	bool editor_open(QString const & file_name);

	//============================================================================
	void new_tree(NexusTree* new_tree);
	void reset_trees();

	std::shared_ptr<Hydra> const get_hydra() const { return this->hydra; }
	AgisResult<AssetPtr> const get_asset(std::string const& asset_id);
	std::optional<AgisStrategyRef const> get_strategy(std::string const& strategy_id); 
	
	std::vector<std::string> get_portfolio_ids();
	size_t get_candle_count() { return this->hydra->get_candle_count(); }

	[[nodiscard]] AgisResult<bool> new_exchange(
		const std::string& exchange_id,
		const std::string& source,
		const std::string& freq,
		const std::string& dt_format
	);
	NexusStatusCode new_portfolio(
		const std::string& portfolio_id,
		const std::string& starting_cash
	);
	NexusStatusCode new_strategy(
		const std::string& portfolio_id,
		const std::string& strategy_id,
		const std::string& allocation
	);

	NexusStatusCode remove_exchange(const std::string& name);
	NexusStatusCode remove_portfolio(const std::string& name);
	NexusStatusCode remove_strategy(const std::string& name);

	auto const& get_order_history() const { return this->order_history; }
	auto const& get_trade_history() const { return this->trade_history; }
	auto const& get_position_history() const { return this->position_history; }

	//============================================================================
	template <typename T>
	std::vector<std::shared_ptr<T>> const filter_event_history(
		std::vector<std::shared_ptr<T>> const& events,
		std::optional<std::string> const& asset_id,
		std::optional<std::string> const& strategy_id,
		std::optional<std::string> const& portfolio_id) const
	{
		std::optional<size_t> asset_index = std::nullopt;
		if (asset_id.has_value()) asset_index = this->hydra->get_exchanges().get_asset_index(asset_id.value());

		std::optional<size_t> strategy_index = std::nullopt;
		if (strategy_id.has_value()) strategy_index = this->hydra->__get_strategy_map().__get_strategy_index(strategy_id.value());

		std::optional<size_t> portfolio_index = std::nullopt;
		if (portfolio_id.has_value()) portfolio_index = this->hydra->get_portfolios().__get_portfolio_index(portfolio_id.value());

		std::vector<std::shared_ptr<T>> return_vec;
		auto it = events.begin();
		while (it != events.end())
		{
			if (asset_index.has_value() && (*it)->get_asset_index() != asset_index.value())
			{
				++it;
				continue;
			}
			if (strategy_index.has_value() && (*it)->get_strategy_index() != strategy_index.value())
			{
				++it;
				continue;
				
			}
			if (portfolio_index.has_value() && (*it)->get_portfolio_index() != portfolio_index.value())
			{
				++it;
				continue;
			}
			return_vec.push_back(*it);
			++it;
		}
		return return_vec;
	}
};

LPCWSTR StringToLPCWSTR(const std::string& str);