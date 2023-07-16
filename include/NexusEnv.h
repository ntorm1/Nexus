#pragma once
#include "NexusPch.h"
#include <filesystem>
#include <unordered_map>

#include "CodeEditor.h"
#include "NexusTree.h"

#include "AgisPointers.h"
#include "AgisErrors.h"
#include "Hydra.h"
#include "Asset.h"

namespace fs = std::filesystem;

class NexusEnv
{
private:
	/// <summary>
	/// Vector of pointers to text editors currently open.
	/// </summary>
	std::vector<TextEdit*> open_editors;

	/// <summary>
	/// Vector of pointers to trees that are currently open
	/// </summary>
	std::vector<NexusTree*> open_trees;

	/// <summary>
	/// Shared pointer to a hydra instance
	/// </summary>
	std::shared_ptr<Hydra> hydra;

	std::string env_name;
	fs::path env_path;

	void remove_editors() { this->open_editors.clear(); }

public:
	NexusEnv();

	void clear();
	void restore(json const & j);

	//============================================================================
	fs::path get_env_settings_path() const { return this->env_path / "env_settings.json"; }
	bool save_env(json &j);
	void load_env(std::string const & exe_path, std::string const & env_name);

	//============================================================================
	void new_editor(TextEdit* new_editor);
	std::optional<TextEdit*> get_editor(QString const & file_name) const;
	void remove_editor(QString const& file_name);
	bool editor_open(QString const & file_name);

	//============================================================================
	void new_tree(NexusTree* new_tree);
	void reset_trees();

	std::shared_ptr<Hydra> const get_hydra() { return this->hydra; }
	std::optional<AssetPtr> const get_asset(std::string const& asset_id);

	NexusStatusCode new_exchange(
		const std::string& exchange_id,
		const std::string& source,
		const std::string& freq,
		const std::string& dt_format
	);
	NexusStatusCode remove_exchange(const std::string& name);

};