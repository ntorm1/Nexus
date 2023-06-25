#pragma once
#include "NexusPch.h"
#include "DockManager.h"

#include <QSettings>

// Qt Advanced Docking System dock manager extension
// because the Qt Advanced Docking System is not designed to e.g. regenerate closed widgets when restoring a perspective
class NexusDockManager : public ads::CDockManager {
public:
	NexusDockManager(QWidget* parent = nullptr);

	json save_widgets();
	void restore_widgets(json const& j);
};