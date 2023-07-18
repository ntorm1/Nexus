#pragma once
#include "NexusNode.h"

#include "ui_NexusNodeEditor.h"

size_t NexusNodeEditor::counter(0);

NexusNodeEditor::NexusNodeEditor(
		NexusEnv const* nexus_env_,
		ads::CDockWidget* DockWidget_,
		QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::NexusNodeEditor),
	nexus_env(nexus_env_),
	DockWidget(DockWidget_)
{
	ui->setupUi(this);

	this->id = counter++;
}
