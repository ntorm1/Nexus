
#include <QtNodes/ConnectionStyle>

#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/NodeData>
#include <QSpacerItem>
#include <QAction>
#include <QFileDialog>
#include <QScreen>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>

#include "NexusPch.h"
#include "NexusErrors.h"
#include "NexusNodeModel.h"
#include "NexusEnv.h"
#include "NexusNode.h"

#include "ui_NexusNodeEditor.h"
#include <qlabel.h>


using QtNodes::ConnectionStyle;
using QtNodes::DataFlowGraphicsScene;


size_t NexusNodeEditor::counter(0);


//============================================================================
std::shared_ptr<NodeDelegateModelRegistry> registerDataModels()
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
	auto openAction = menu->addAction("Open Scene");
	auto loadAction = menu->addAction("Load Scene");

	QObject::connect(saveAction, &QAction::triggered, scene, [this] {
		this->__save();
		});

	QObject::connect(loadAction, &QAction::triggered, scene, [this, scene] {
		RUN_WITH_ERROR_DIALOG(this->__load(scene));
		});

	QObject::connect(openAction, &QAction::triggered, scene, [this, scene] {
		auto strat_dir = this->nexus_env->get_env_path() / "strategies";
		QString filePath = QFileDialog::getOpenFileName(this,
			tr("Open File"),
			QString::fromStdString(strat_dir.string()),
			tr("Flow Files (*.flow);;All Files (*.*)"));
		if (!filePath.isEmpty()) {
			auto flow_path = fs::path(filePath.toStdString());
			RUN_WITH_ERROR_DIALOG(this->__load(scene, flow_path));
		}
		});

	return menuBar;
}


//============================================================================
void NexusNodeEditor::on_tw_change(int index)
{
	auto str_tw = trading_window->currentText().toStdString();
	if (str_tw == "") {
		this->strategy.get()->set_trading_window(std::nullopt);
	}
	else {
		TradingWindow tw = agis_trading_window_map.at(str_tw);
		this->strategy.get()->set_trading_window(tw);
	}
}


//============================================================================
void NexusNodeEditor::on_alloc_change(double allocation)
{
	this->strategy.get()->__set_allocation(allocation);
}


//============================================================================
void NexusNodeEditor::create_strategy_tab(QVBoxLayout* l)
{
	// strat allocation
	QHBoxLayout* row_layout = new QHBoxLayout(this);
	QLabel* row_label = new QLabel("Allocation: ");
	this->allocation = new QLineEdit(this);
	QDoubleValidator* validator = new QDoubleValidator(0.0, 2.0, 3, this->allocation); // 2 decimal places
	allocation->setValidator(validator);
	row_layout->addWidget(row_label);
	row_layout->addWidget(this->allocation);
	l->addLayout(row_layout);

	// listen to allocation change
	connect(
		allocation,
		&QLineEdit::textChanged,
		[this](const QString& newText) {
			bool ok;
			double allocationValue = newText.toDouble(&ok);
			if (ok) {
				on_alloc_change(allocationValue);
			}
		}
	);

	// trading window
	row_layout = new QHBoxLayout(this);
	this->trading_window = new QComboBox();
	for (const auto& item : agis_trading_windows) {
		this->trading_window->addItem(QString::fromStdString(item));
	}
	QLabel* label = new QLabel("Trading Window: ");
	row_layout->addWidget(label);
	row_layout->addWidget(this->trading_window);
	l->addLayout(row_layout);
	
	// strategy beta settings
	this->beta_scale = new QCheckBox("Beta Scale Positions");
	this->beta_hedge = new QCheckBox("Beta Hedge Positions");
	beta_hedge->setChecked(this->strategy.get()->__is_beta_hedged());
	beta_scale->setChecked(this->strategy.get()->__is_beta_scaling());
	l->addWidget(beta_hedge);
	l->addWidget(beta_scale);
	
	connect(beta_scale, &QCheckBox::stateChanged, [this](int state) {
		AgisResult<bool> res;
		if (state == Qt::Checked) {
			res = this->strategy.get()->set_beta_scale_positions(true);
		}
		else {
			res = this->strategy.get()->set_beta_scale_positions(false);
		}
		if (res.is_exception())
		{
			QMessageBox::critical(this, "Error", QString::fromStdString(res.get_exception()));
			beta_scale->setChecked(false);
		}
	});
	connect(beta_hedge, &QCheckBox::stateChanged, [this](int state) {
		AgisResult<bool> res;
		if (state == Qt::Checked) {
			res = this->strategy.get()->set_beta_hedge_positions(true);
		}
		else {
			res = this->strategy.get()->set_beta_hedge_positions(false);
		}
		if (res.is_exception())
		{
			QMessageBox::critical(this, "Error", QString::fromStdString(res.get_exception()));
			beta_hedge->setChecked(false);
		}
	});

	// listen to changes in strategy settings widgets
	connect(
		trading_window,
		QOverload<int>::of(&QComboBox::currentIndexChanged),
		this,
		&NexusNodeEditor::on_tw_change);
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
	this->strategy_id = this->strategy.get()->get_strategy_id();

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
	QHBoxLayout* h = new QHBoxLayout(centralWidget);

	// node editor layout
	QVBoxLayout* l = new QVBoxLayout(centralWidget);

	std::shared_ptr<NodeDelegateModelRegistry> registry = registerDataModels();
	this->dataFlowGraphModel = new DataFlowGraphModel(registry);

	DataFlowGraphicsScene* scene = new DataFlowGraphicsScene(*dataFlowGraphModel);
	this->view = new GraphicsView(scene);

	l->addWidget(createSaveRestoreMenu(scene));
	l->addWidget(view);
	h->addLayout(l);

	//strategy settings layout
	l = new QVBoxLayout(centralWidget);
	l->setAlignment(Qt::AlignTop); // Set the alignment to top
	QSpacerItem* spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Fixed);
	l->addItem(spacer);
	this->create_strategy_tab(l);
	h->addLayout(l);
	centralWidget->setLayout(h);

	// set the base hydra instance
	ExchangeModel::hydra = nexus_env->get_hydra();

	// attempt to load existing flow graph if it exists
	RUN_WITH_ERROR_DIALOG(this->__load(scene);)

	auto abstract_strategy = dynamic_cast<AbstractAgisStrategy*>(strategy.get().get());
	abstract_strategy->set_abstract_ev_lambda([this]() {
		return this->__extract_abstract_strategy(this->dataFlowGraphModel);
	});

	// set widget values to class values
	auto double_allocation = strategy.get()->get_allocation();
	this->allocation->setText(QString::fromStdString(std::to_string(double_allocation)));
	auto w = strategy.get()->get_trading_window();
	auto w_str = trading_window_to_key_str(w);
	int index = this->trading_window->findText(QString::fromStdString(w_str));
	this->trading_window->setCurrentIndex(index);
	
	this->id = counter++;
}


//============================================================================
NexusNodeEditor::~NexusNodeEditor()
{
	//auto abstract_strategy = dynamic_cast<AbstractAgisStrategy*>(strategy.get().get());
	//abstract_strategy->extract_ev_lambda();
	delete dataFlowGraphModel;
	delete view;
}


//============================================================================
void NexusNodeEditor::__set_strategy(AgisStrategyRef const strategy_)
{
	this->strategy = strategy_;
	this->strategy_id = this->strategy.get()->get_strategy_id();
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
	else {
		AGIS_THROW("Failed to open the strategy flow file: " + flow_path.string());
	}

	auto abstract_strategy = dynamic_cast<AbstractAgisStrategy*>(strategy.get().get());
	auto res = abstract_strategy->extract_ev_lambda();
	if (res.is_exception()) NEXUS_INTERUPT(res.get_exception());
}

//============================================================================
void NexusNodeEditor::__load(BasicGraphicsScene* scene, std::optional<fs::path> file_path)
{
	fs::path strat_path;
	if (!file_path.has_value())
	{
		strat_path = this->nexus_env->get_env_path()
			/ "strategies"
			/ this->strategy.get()->get_strategy_id()
			/ "graph.flow";
	}
	else {
		strat_path = file_path.value();
	}

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
std::optional<ExchangeViewLambdaStruct> NexusNodeEditor::__extract_abstract_strategy(DataFlowGraphModel* model)
{
	auto ids = model->allNodeIds();
	// probably better way to find the strategy node
	StrategyAllocationModel* node = nullptr;
	for (auto& id : ids)
	{
		node = model->delegateModel<StrategyAllocationModel>(id);
		if(node) break;
	}
	if (!node) { return std::nullopt; }
	if (!node->ev_lambda_struct.has_value()) { return std::nullopt; }
	if (node->ev_lambda_struct.value().asset_lambda.size() == 0)
	{
		NEXUS_THROW("Attempting to extract strategy with no asset lambdas");
	}

	auto epsilon = stod(node->strategy_allocation_node->epsilon->text().toStdString());
	auto target_leverage = stod(node->strategy_allocation_node->target_leverage->text().toStdString());
	auto clear_missing = node->strategy_allocation_node->clear_missing->isChecked();
	auto ev_opp_type = node->strategy_allocation_node->ev_opp_type->currentText().toStdString();
	auto str_alloc_type = node->strategy_allocation_node->alloc_type->currentText().toStdString();
	std::optional<double> ev_opp_param = std::nullopt;
	if (node->strategy_allocation_node->ev_opp_param->isEnabled())
	{
		auto val = node->strategy_allocation_node->ev_opp_param->text().toStdString();
		ev_opp_param = stod(val);
	}

	StrategyAllocLambdaStruct _struct{
		epsilon,
		target_leverage,
		ev_opp_param,
		clear_missing,
		ev_opp_type,
		agis_strat_alloc_map.at(str_alloc_type)
	};
	auto ev_lambda_struct = node->ev_lambda_struct;
	ev_lambda_struct.value().strat_alloc_struct = _struct;
	return ev_lambda_struct;
}
