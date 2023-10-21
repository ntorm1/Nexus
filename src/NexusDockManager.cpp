
#include "DockAreaWidget.h"
#include "NexusDockManager.h"
#include "MainWindow.h"
#include "DockWidget.h"
#include "NexusPortfolio.h"

NexusDockManager::NexusDockManager(MainWindow* main_window_, QWidget* parent) :
	main_window(main_window_),
	ads::CDockManager(parent) 
{
}

Document NexusDockManager::save_widgets()
{
	Document widgets;
	widgets.SetObject();  // Create a JSON object to store the data.

	auto& allocator = widgets.GetAllocator();
	for (auto const& dock_widget : this->get_widgets())
	{
		rapidjson::Value widget(rapidjson::kObjectType);
		widget.AddMember("widget_id", dock_widget->get_id(), allocator);

		// Add widget_type as a member
		rapidjson::Value widgetTypeValue;
		widgetTypeValue.SetUint64(static_cast<uint64_t>(dock_widget->get_widget_type()));
		widget.AddMember("widget_type", widgetTypeValue, allocator);

		if (dock_widget->get_widget_type() == WidgetType::Asset)
		{
			auto child = dock_widget->widget();
			NexusAsset* asset_child = static_cast<NexusAsset*>(child);
			widget.AddMember("asset_id", rapidjson::Value(asset_child->get_asset_id().c_str(), allocator), allocator);

			rapidjson::Value plottedGraphs(rapidjson::kArrayType);
			const std::vector<std::string> graphs = asset_child->get_plotted_graphs();
			for (const std::string& graph : graphs)
			{
				rapidjson::Value graphValue(graph.c_str(), allocator);
				plottedGraphs.PushBack(graphValue, allocator);
			}
			widget.AddMember("plotted_graphs", plottedGraphs, allocator);
		}
		else if (dock_widget->get_widget_type() == WidgetType::NodeEditor)
		{
			auto child = dock_widget->widget();
			NexusNodeEditor* asset_child = static_cast<NexusNodeEditor*>(child);
			asset_child->__save();
			widget.AddMember("strategy_id", rapidjson::Value(asset_child->get_strategy_id().c_str(), allocator), allocator);

		}
		else if (dock_widget->get_widget_type() == WidgetType::Portfolio)
		{
			auto child = dock_widget->widget();
			NexusPortfolio* p = static_cast<NexusPortfolio*>(child);
			widget.AddMember("portfolio_id", rapidjson::Value(p->get_portfolio_id().c_str(), allocator), allocator);

			auto graphs = p->get_plotted_graphs();
			if (graphs.size() > 0) {
				rapidjson::Value plottedGraphs(rapidjson::kArrayType);
				for (const std::string& graph : graphs)
				{
					rapidjson::Value graphValue(graph.c_str(), allocator);
					plottedGraphs.PushBack(graphValue, allocator);
				}
				widget.AddMember("plotted_graphs", plottedGraphs, allocator);
			}
		}

		// Use dock_widget's objectName() as the key
		rapidjson::Value key(dock_widget->objectName().toStdString().c_str(), allocator);

		// Add the widget object with the custom key to the main JSON object
		widgets.AddMember(key.Move(), widget, allocator);
	}
	return widgets;
}

void NexusDockManager::restore_widgets(Document const& j)
{
	auto const& widgets = j["widgets"];

	// Store the exchange items in a vector for parallel processing
	int max_widget_id = -1;
	size_t widget_id = 0;
	for (rapidjson::Value::ConstMemberIterator itr = widgets.MemberBegin(); itr != widgets.MemberEnd(); ++itr) {
		//auto widget_id_str = itr->name.GetString();  
		//auto widget_id = static_cast<size_t>(std::stoi(widget_id_str));
		const rapidjson::Value& widget_json = itr->value; // Get the value
		
		size_t widget_type_uint = widget_json["widget_type"].GetUint64();
		WidgetType widget_type = static_cast<WidgetType>(widget_type_uint);

		ads::CDockWidget* widget = nullptr;
		switch (widget_type) {
			case WidgetType::Editor:{
				widget = this->main_window->create_editor_widget();
				break;
			}
			case WidgetType::Asset: {
				auto asset_id = QString::fromStdString(widget_json["asset_id"].GetString());
				widget = this->main_window->create_asset_widget(asset_id);
				
				std::vector<std::string> plots;
				if (widget_json.HasMember("plotted_graphs") && widget_json["plotted_graphs"].IsArray()) {
					const rapidjson::Value& plottedGraphsArray = widget_json["plotted_graphs"];

					for (rapidjson::SizeType i = 0; i < plottedGraphsArray.Size(); i++) {
						if (plottedGraphsArray[i].IsString()) {
							plots.push_back(plottedGraphsArray[i].GetString());
						}
					}
				}

				auto child = widget->widget();
				NexusAsset* asset_child = static_cast<NexusAsset*>(child);
				asset_child->set_plotted_graphs(plots);
				break;
			}
			case WidgetType::NodeEditor:{
				auto strategy_id = QString::fromStdString(widget_json["strategy_id"].GetString());
				widget = this->main_window->create_node_editor_widget(strategy_id);
				break;
			}
			case WidgetType::Portfolio: {
				auto portfolio_id = QString::fromStdString(widget_json["portfolio_id"].GetString());
				widget = this->main_window->create_portfolio_widget(portfolio_id);
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
		widget_id++;
	}
	ads::CDockWidget::counter = max_widget_id + 1;
}