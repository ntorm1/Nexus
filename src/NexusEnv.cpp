#include <fstream>

#include "NexusEnv.h"


NexusEnv::NexusEnv()
{
	this->hydra = std::make_shared<Hydra>();
}

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

void NexusEnv::new_editor(TextEdit* new_editor)
{
	this->open_editors.push_back(new_editor);
}

void NexusEnv::new_tree(NexusTree* new_tree)
{
	this->open_trees.push_back(new_tree);
}

void NexusEnv::reset_trees()
{
	for (auto& tree : this->open_trees)
	{
		tree->reset_tree();
	}
}

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

std::optional<SharedAssetLockPtr> const NexusEnv::get_asset(std::string const& asset_id)
{
	return this->hydra->get_asset(asset_id);
}

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
	if (res != NexusStatusCode::Ok) return res;

}

NexusStatusCode NexusEnv::remove_exchange(const std::string& name)
{
	qDebug() << "Removing exchange: " << name;
	return this->hydra->remove_exchange(name);
}

void NexusEnv::clear()
{
	this->remove_editors();
	this->reset_trees();
	this->hydra->clear();
}

void NexusEnv::restore(json const& j)
{
	this->hydra->restore(j);
	this->remove_editors();
	for (auto& tree : this->open_trees)
	{
		tree->restore_tree(j);
	}
}

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