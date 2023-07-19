
#include <QtNodes/ConnectionStyle>

#include <QtNodes/DataFlowGraphicsScene>

#include <QtNodes/NodeData>

#include <QAction>
#include <QFileDialog>
#include <QScreen>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QVBoxLayout>


#include "NexusNodeModel.h"
#include "NexusNode.h"

#include "ui_NexusNodeEditor.h"
#include <qlabel.h>


using QtNodes::ConnectionStyle;
using QtNodes::DataFlowGraphicsScene;

size_t NexusNodeEditor::counter(0);



static std::shared_ptr<NodeDelegateModelRegistry> registerDataModels()
{
	auto ret = std::make_shared<NodeDelegateModelRegistry>();

	ret->registerModel<NaiveDataModel>();

	/*
	 We could have more models registered.
	 All of them become items in the context meny of the scene.

	 ret->registerModel<AnotherDataModel>();
	 ret->registerModel<OneMoreDataModel>();

   */

	return ret;
}

static void setStyle()
{
	ConnectionStyle::setConnectionStyle(
		R"(
  {
    "ConnectionStyle": {
      "UseDataDefinedColors": true
    }
  }
  )");
}


//============================================================================
NexusNodeEditor::NexusNodeEditor(
		NexusEnv const* nexus_env_,
		ads::CDockWidget* DockWidget_,
		AgisStrategyRef strategy_,
		QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::NexusNodeEditor),
	nexus_env(nexus_env_),
	strategy(strategy_),
	DockWidget(DockWidget_)
{
	ConnectionStyle::setConnectionStyle(
	R"(
	  {
		"ConnectionStyle": {
		  "UseDataDefinedColors": true
		}
	  }
	 )");
	ui->setupUi(this);
	QWidget* centralWidget = ui->centralwidget;
	QVBoxLayout* l = new QVBoxLayout(centralWidget);

	std::shared_ptr<NodeDelegateModelRegistry> registry = registerDataModels();
	this->dataFlowGraphModel = new DataFlowGraphModel(registry);

	DataFlowGraphicsScene* scene = new DataFlowGraphicsScene(*dataFlowGraphModel);
	this->view = new GraphicsView(scene);

	l->addWidget(view);
	centralWidget->setLayout(l);

	this->id = counter++;
}

NexusNodeEditor::~NexusNodeEditor()
{
	delete dataFlowGraphModel;
	delete view;
}
