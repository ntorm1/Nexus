#pragma once

#include <memory.h>
#include <vector>
#include <filesystem>
#include <unordered_map>

#include <json.hpp>

#include "CodeEditor.h"
#include "NexusTree.h"

#include "AgisPointers.h"
#include "AgisErrors.h"
#include "Hydra.h"

namespace fs = std::filesystem;
using json = nlohmann::json;


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

public:
	NexusEnv();

	void clear();
	void restore(json const & j);

	//============================================================================
	fs::path get_env_settings_path() const { return this->env_path / "env_settings.json"; }
	bool save_env();
	void load_env(std::string const & exe_path, std::string const & env_name);

	//============================================================================
	void new_editor(TextEdit* new_editor);
	std::optional<TextEdit*> get_editor(QString const & file_name) const;
	void remove_editor(QString const& file_name);
	void remove_editors() { this->open_editors.clear(); }
	bool editor_open(QString const & file_name);

	//============================================================================
	void new_tree(NexusTree* new_tree);
	void reset_trees();

	std::shared_ptr<Hydra> const get_hydra() { return this->hydra; }

	NexusStatusCode new_exchange(
		const std::string& exchange_id,
		const std::string& source,
		const std::string& freq
	);
	NexusStatusCode remove_exchange(const std::string& name);

};