#pragma once
#include <QMainWindow>
#include "NexusPch.h"
#include "DockWidget.h"

namespace Ui {
	class NexusBroker;
}

class NexusEnv;

class NexusBroker : public QMainWindow
{
public:
	NexusBroker(
		NexusEnv const* nexus_env_,
		ads::CDockWidget* DockWidget,
		QWidget* parent = 0
	);

	ads::CDockWidget* DockWidget;

private:
	Ui::NexusBroker* ui;
	NexusEnv const* nexus_env;

};