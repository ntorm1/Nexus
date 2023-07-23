
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


//============================================================================
static std::shared_ptr<NodeDelegateModelRegistry> registerDataModels()
{
	auto ret = std::make_shared<NodeDelegateModelRegistry>();

	ret->registerModel<ExchangeModel>();
	ret->registerModel<ExchangeViewModel>();
	ret->registerModel<AssetLambdaModel>();
	ret->registerModel<StrategyAllocationModel>();

	return ret;
}


//============================================================================
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
QMenuBar* NexusNodeEditor::createSaveRestoreMenu(BasicGraphicsScene* scene)
{
	auto menuBar = new QMenuBar();
	QMenu* menu = menuBar->addMenu("File");
	auto saveAction = menu->addAction("Save Scene");
	auto loadAction = menu->addAction("Load Scene");

	QObject::connect(saveAction, &QAction::triggered, scene, [this] {
		this->__save();
		});

	QObject::connect(loadAction, &QAction::triggered, scene, [this, scene] {
		RUN_WITH_ERROR_DIALOG(this->__load(scene));
		});

	return menuBar;
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

	l->addWidget(createSaveRestoreMenu(scene));
	l->addWidget(view);
	centralWidget->setLayout(l);

	// set the base hydra instance
	ExchangeModel::hydra = nexus_env->get_hydra();

	// attempt to load existing flow graph if it exists
	RUN_WITH_ERROR_DIALOG(this->__load(scene);)

	auto abstract_strategy = dynamic_cast<AbstractAgisStrategy*>(strategy.get().get());
	abstract_strategy->set_abstract_ev_lambda([this]() {
		return this->__extract_abstract_strategy();
	});

	this->id = counter++;
}


//============================================================================
NexusNodeEditor::~NexusNodeEditor()
{
	auto abstract_strategy = dynamic_cast<AbstractAgisStrategy*>(strategy.get().get());
	abstract_strategy->extract_ev_lambda();

	delete dataFlowGraphModel;
	delete view;
}


//============================================================================
void NexusNodeEditor::__save()
{
	auto base_path = this->nexus_env->get_env_path() / "strategies";
	if (!fs::exists(base_path)) { fs::create_directory(base_path); }


	auto strat_path = base_path / this->strategy.get()->get_strategy_id();
	if (!fs::exists(strat_path)) { fs::create_directory(strat_path); }

	auto flow_path = strat_path / "graph.flow";
	QFile file(flow_path);
	if (file.open(QIODevice::WriteOnly)) {
		file.write(QJsonDocument(dataFlowGraphModel->save()).toJson());
	}

	auto abstract_strategy = dynamic_cast<AbstractAgisStrategy*>(strategy.get().get());
	abstract_strategy->extract_ev_lambda();
}

//============================================================================
void NexusNodeEditor::__load(BasicGraphicsScene* scene)
{
	auto strat_path = this->nexus_env->get_env_path()
		/ "strategies"
		/ this->strategy.get()->get_strategy_id()
		/ "graph.flow";

	if (!fs::exists(strat_path)) {
		throw std::runtime_error("Missing strategy flow file: " + strat_path.string());
	}

	scene->clearScene();

	QFile file(strat_path);
	if (!file.open(QIODevice::ReadOnly)) {
		throw std::runtime_error("Failed to open the strategy flow file: " + strat_path.string());
	}

	QByteArray const wholeFile = file.readAll();
	file.close();

	QJsonParseError error;
	QJsonDocument jsonDocument = QJsonDocument::fromJson(wholeFile, &error);
	if (error.error != QJsonParseError::NoError) {
		throw std::runtime_error("Failed to parse JSON in the strategy flow file: " + error.errorString().toStdString());
	}

	if (!jsonDocument.isObject()) {
		throw std::runtime_error("Invalid JSON format in the strategy flow file.");
	}

	this->dataFlowGraphModel->load(jsonDocument.object());
	view->centerScene();
}


//============================================================================
std::optional<ExchangeViewLambdaStruct> NexusNodeEditor::__extract_abstract_strategy()
{
	auto ids = this->dataFlowGraphModel->allNodeIds();
	// probably better way to find the strategy node
	StrategyAllocationModel* node = nullptr;
	for (auto& id : ids)
	{
		auto node = this->dataFlowGraphModel->delegateModel<StrategyAllocationModel>(id);
		if(node) break;
	}
	if (!node) { return std::nullopt; }
	if (!node->ev_lambda_struct.has_value()) { return std::nullopt; }

	auto epsilon = stod(node->strategy_allocation_node->epsilon->text().toStdString());
	auto target_leverage = stod(node->strategy_allocation_node->target_leverage->text().toStdString());
	auto clear_missing = node->strategy_allocation_node->clear_missing->isChecked();
	auto ev_opp_type = node->strategy_allocation_node->ev_opp_type->currentText().toStdString();
	auto str_alloc_type = node->strategy_allocation_node->alloc_type->currentText().toStdString();


	StrategyAllocLambdaStruct _struct{
		epsilon,
		target_leverage,
		clear_missing,
		ev_opp_type,
		agis_strat_alloc_map.at(str_alloc_type)
	};
	auto ev_lambda_struct = node->ev_lambda_struct;
	ev_lambda_struct.value().strat_alloc_struct = _struct;
	return ev_lambda_struct;
}
