
#include "DockAreaWidget.h"
#include "NexusDockManager.h"
#include "MainWindow.h"
#include "DockWidget.h"

NexusDockManager::NexusDockManager(MainWindow* main_window_, QWidget* parent) :
	main_window(main_window_),
	ads::CDockManager(parent) 
{
}

json NexusDockManager::save_widgets()
{
	json widgets;
	for (auto const& dock_widget : this->get_widgets())
	{
		json widget;
		widget["widget_id"] = dock_widget->get_id();
		widget["widget_type"] = dock_widget->get_widget_type();

		if (dock_widget->get_widget_type() == WidgetType::Asset)
		{
			auto child = dock_widget->widget();
			NexusAsset* asset_child = static_cast<NexusAsset*>(child);
			widget["asset_id"] = asset_child->get_asset_id();
		}
		else if (dock_widget->get_widget_type() == WidgetType::NodeEditor)
		{
			auto child = dock_widget->widget();
			NexusNodeEditor* asset_child = static_cast<NexusNodeEditor*>(child);
			asset_child->__save();
			widget["strategy_id"] = asset_child->get_strategy_id();
		}

		widgets[dock_widget->objectName().toStdString()] = widget;
	}
	return widgets;
}

void NexusDockManager::restore_widgets(json const& j)
{
	auto const& widgets = j["widgets"];

	// Store the exchange items in a vector for parallel processing
	int max_widget_id = -1;
	for (const auto& widget_pair : widgets.items())
	{
		std::string widget_name = widget_pair.key();
		auto widget_json = widget_pair.value();
		int widget_id = widget_json["widget_id"];
		WidgetType widget_type = widget_json["widget_type"];

		ads::CDockWidget* widget = nullptr;
		switch (widget_type) {
			case WidgetType::Editor:{
				widget = this->main_window->create_editor_widget();
				break;
			}
			case WidgetType::Asset: {
				auto asset_id = QString::fromStdString(widget_json["asset_id"]);
				widget = this->main_window->create_asset_widget(asset_id);
				break;
			}
			case WidgetType::NodeEditor:{
				auto strategy_id = QString::fromStdString(widget_json["strategy_id"]);
				widget = this->main_window->create_node_editor_widget(strategy_id);
				break;
			}
			case WidgetType::Exchanges: {
				break;
			}
			case WidgetType::FileTree: {
				break;
			}
		}
		if (widget_id > max_widget_id) max_widget_id = widget_id;

		if (!widget) { continue; }
		widget->set_id(widget_id);
		widget->set_widget_type(widget_type);
		this->addDockWidget(ads::TopDockWidgetArea, widget);
	}
	ads::CDockWidget::counter = max_widget_id + 1;
}