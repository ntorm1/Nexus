
#include "NexusEnv.h"


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
	if (it != this->open_editors.end())
	{
		this->open_editors.erase(it);
	}
}