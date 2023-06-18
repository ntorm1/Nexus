#pragma once

#include <vector>

#include "CodeEditor.h"

class NexusEnv
{
private:
	/// <summary>
	/// Vector of pointers to text editors currently open.
	/// </summary>
	std::vector<TextEdit*> open_editors;

public:
	/// <summary>
	/// Add a new TextEdit widget to the open editors vector
	/// </summary>
	/// <param name="new_editor">pointer to new TextEdit widget</param>
	void new_editor(TextEdit* new_editor);

	/// <summary>
	/// Get a editor from the open editors vector by file name
	/// </summary>
	/// <param name="file_name">File name of the editor to get</param>
	/// <returns>Pointer to the TextEdit widget if exists</returns>
	std::optional<TextEdit*> get_editor(QString const & file_name) const;
	
	/// <summary>
	/// Remove a TextEdit widget by file name
	/// </summary>
	/// <param name="file_name"></param>
	void remove_editor(QString const& file_name);

	/// <summary>
	/// File is already open in a text editor
	/// </summary>
	/// <param name="file"></param>
	/// <returns></returns>
	bool editor_open(QString const & file_name);
};