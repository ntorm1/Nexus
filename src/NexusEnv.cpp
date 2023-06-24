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

NexusStatusCode NexusEnv::new_exchange(
	const std::string& exchange_id,
	const std::string& source,
	const std::string& freq)
{
	qDebug() << "Building new exchange: " << exchange_id;
	return this->hydra->new_exchange(exchange_id, source, string_to_freq(freq));
}

NexusStatusCode NexusEnv::remove_exchange(const std::string& name)
{
	qDebug() << "Removing exchange: " << name;
	return this->hydra->remove_exchange(name);
}

bool NexusEnv::save_env()
{
	qDebug() << "Saving environemnt...";
	json j;

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

	// Dump ths json output to a file
	std::string jsonString = j.dump(4);
	auto json_path = this->env_path / "env_settings.json";
	std::ofstream outputFile(json_path.string());
	if (outputFile.is_open()) {
		outputFile << jsonString;
		outputFile.close();
		return true;
	}
	else {
		return false;
	}
	qDebug() << "Environemnt saved";
}