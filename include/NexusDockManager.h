#pragma once

#include <QSettings>

#include "NexusPch.h"
#include "DockManager.h"

//#include "MainWindow.h"

class MainWindow;


// Qt Advanced Docking System dock manager extension
// because the Qt Advanced Docking System is not designed to e.g. regenerate closed widgets when restoring a perspective
class NexusDockManager : public ads::CDockManager {
public:
	NexusDockManager(MainWindow* main_window, QWidget* parent = nullptr);

	json save_widgets();
	void restore_widgets(json const& j);

private: 
	MainWindow* main_window;
};