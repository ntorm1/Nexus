#pragma once

#include <vector>
#include <filesystem>

#include <json.hpp>

#include "CodeEditor.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

class NexusEnv
{
private:
	/// <summary>
	/// Vector of pointers to text editors currently open.
	/// </summary>
	std::vector<TextEdit*> open_editors;

	std::string env_name;
	fs::path env_path;

public:
	NexusEnv() {};

	fs::path get_env_settings_path() const { return this->env_path / "env_settings.json"; }

	bool save_env();
	void load_env(std::string const & exe_path, std::string const & env_name);

	void new_editor(TextEdit* new_editor);
	std::optional<TextEdit*> get_editor(QString const & file_name) const;
	void remove_editor(QString const& file_name);
	void remove_editors() { this->open_editors.clear(); }
	bool editor_open(QString const & file_name);

};