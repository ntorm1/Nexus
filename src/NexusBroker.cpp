#pragma once
#include "NexusPch.h"
#include "NexusBroker.h"
#include "ui_NexusBroker.h"

NexusBroker::NexusBroker(
	NexusEnv const* nexus_env_,
	ads::CDockWidget* DockWidget_,
	QWidget* parent_) :
	QMainWindow(parent_),
	nexus_env(nexus_env_),
	ui(new Ui::NexusBroker),
	DockWidget(DockWidget_)
{
}