#pragma once

#include "DockAreaWidget.h"
#include "DockWidget.h"

class MainWindow;

class NexusWidgetFactory
{
public:
	NexusWidgetFactory();
	~NexusWidgetFactory();

	static ads::CDockWidget* create_portfolios_widget(MainWindow* w);
	static ads::CDockWidget* create_exchanges_widget(MainWindow* w);
	static ads::CDockWidget* create_file_system_tree_widget(MainWindow* w);

};