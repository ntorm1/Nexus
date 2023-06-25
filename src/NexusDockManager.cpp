
#include "DockAreaWidget.h"
#include "NexusDockManager.h"

NexusDockManager::NexusDockManager(QWidget* parent) :
	ads::CDockManager(parent) 
{
}

json NexusDockManager::save_widgets()
{
	json widgets;
	for (auto const& dock_widget : this->get_widgets())
	{
		json widget;
		widget["dock_area_id"] = dock_widget->dockAreaWidget()->get_id();
		widget["widget_id"] = dock_widget->get_id();
		widget["widget_type"] = dock_widget->get_widget_type();
		widgets[dock_widget->objectName().toStdString()] = widget;
	}
	return widgets;
}

void NexusDockManager::restore_widgets(json const& j)
{
	auto const& widgets = j["widgets"];
}