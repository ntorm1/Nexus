#include <qsharedpointer.h>

#include "NexusPlot.h"

NexusPlot::NexusPlot(QWidget* parent) :
	QCustomPlot(parent)
{
	std::srand(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0);
	//ui->setupUi(this);

	this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
		QCP::iSelectLegend | QCP::iSelectPlottables);
	this->xAxis->setRange(-8, 8);
	this->yAxis->setRange(-5, 5);
	this->axisRect()->setupFullAxesBox();

	this->xAxis->setLabel("Time");
	this->yAxis->setLabel("y Axis");
	this->legend->setVisible(true);
	QFont legendFont = font();
	legendFont.setPointSize(10);
	this->legend->setFont(legendFont);
	this->legend->setSelectedFont(legendFont);
	this->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

	// set locale to english, so we get english month names:
	this->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));

	// configure bottom axis to show date instead of number:
	QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
	dateTicker->setDateTimeFormat("d. MMMM\nyyyy");
	this->xAxis->setTicker(dateTicker);

	// connect slot that ties some axis selections together (especially opposite axes):
	connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
	// connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
	connect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
	connect(this, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

	// make bottom and left axes transfer their ranges to top and right axes:
	connect(this->xAxis, SIGNAL(rangeChanged(QCPRange)), this->xAxis2, SLOT(setRange(QCPRange)));
	connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)), this->yAxis2, SLOT(setRange(QCPRange)));

	// connect some interaction slots:
	connect(this, SIGNAL(axisDoubleClick(QCPAxis*, QCPAxis::SelectablePart, QMouseEvent*)), this, SLOT(axisLabelDoubleClick(QCPAxis*, QCPAxis::SelectablePart)));
	connect(this, SIGNAL(legendDoubleClick(QCPLegend*, QCPAbstractLegendItem*, QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*, QCPAbstractLegendItem*)));
//	connect(title, SIGNAL(doubleClicked(QMouseEvent*)), this, SLOT(titleDoubleClick(QMouseEvent*)));

	// connect slot that shows a message in the status bar when a graph is clicked:
	connect(this, SIGNAL(plottableClick(QCPAbstractPlottable*, int, QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*, int)));

	// setup policy and connect slot for context menu popup:
	this->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
}

void NexusPlot::mousePress()
{
	// if an axis is selected, only allow the direction of that axis to be dragged
	// if no axis is selected, both directions may be dragged

	if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
		this->axisRect()->setRangeDrag(this->xAxis->orientation());
	else if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
		this->axisRect()->setRangeDrag(this->yAxis->orientation());
	else
		this->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
}

void NexusPlot::mouseWheel()
{
	// if an axis is selected, only allow the direction of that axis to be zoomed
	// if no axis is selected, both directions may be zoomed

	if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
		this->axisRect()->setRangeZoom(this->xAxis->orientation());
	else if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
		this->axisRect()->setRangeZoom(this->yAxis->orientation());
	else
		this->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
}

void NexusPlot::removeAllGraphs()
{
	this->clearGraphs();
	this->replot();
}


void NexusPlot::removeSelectedGraph()
{
	if (this->selectedGraphs().size() > 0)
	{
		this->removeGraph(this->selectedGraphs().first());
		this->replot();
	}
}

void NexusPlot::selectionChanged()
{
	/*
	 normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
	 the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
	 and the axis base line together. However, the axis label shall be selectable individually.

	 The selection state of the left and right axes shall be synchronized as well as the state of the
	 bottom and top axes.

	 Further, we want to synchronize the selection of the graphs with the selection state of the respective
	 legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
	 or on its legend item.
	*/

	// make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
	if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || this->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
		this->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || this->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
	{
		this->xAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
		this->xAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
	}
	// make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
	if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || this->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
		this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || this->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
	{
		this->yAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
		this->yAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
	}

	// synchronize selection of graphs with selection of corresponding legend items:
	for (int i = 0; i < this->graphCount(); ++i)
	{
		QCPGraph* graph = this->graph(i);
		QCPPlottableLegendItem* item = this->legend->itemWithPlottable(graph);
		if (item->selected() || graph->selected())
		{
			item->setSelected(true);
			this->selected_line = item->plottable()->name().toStdString();
			graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
		}
	}
}

void NexusPlot::contextMenuRequest(QPoint pos)
{
	QMenu* menu = new QMenu(this);
	menu->setAttribute(Qt::WA_DeleteOnClose);

	if (this->legend->selectTest(pos, false) >= 0) // context menu on legend requested
	{
		menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignLeft));
		menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignHCenter));
		menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignRight));
		menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignRight));
		menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignLeft));
	}
	else  // general context menu on graphs requested
	{
		menu->addAction("Add random graph", this, SLOT(addRandomGraph()));
		if (this->selectedGraphs().size() > 0)
			menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
		if (this->graphCount() > 0)
			menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
	}

	menu->popup(this->mapToGlobal(pos));
}

void NexusPlot::moveLegend()
{
	if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
	{
		bool ok;
		int dataInt = contextAction->data().toInt(&ok);
		if (ok)
		{
			this->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
			this->replot();
		}
	}
}

void NexusPlot::graphClicked(QCPAbstractPlottable* plottable, int dataIndex)
{
	// since we know we only have QCPGraphs in the plot, we can immediately access interface1D()
	// usually it's better to first check whether interface1D() returns non-zero, and only then use it.
	double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
	QString message = QString("Clicked on graph '%1' at data point #%2 with value %3.").arg(plottable->name()).arg(dataIndex).arg(dataValue);
	//this->ui->statusbar->showMessage(message, 2500);
}

void NexusPlot::plot(StridedPointer<long long> x, StridedPointer<double> y, std::string name)
{
	this->addGraph();
	auto q_name = QString::fromStdString(name);
	this->graph()->setName(q_name);

	QVector<QCPGraphData> timeData(x.size());
	for (int i = 0; i < x.size(); i++)
	{
		timeData[i].key = x[i] / static_cast<double>(1000000000);
		//timeData[i].key = i;
		timeData[i].value = y[i];
	}

	this->graph()->data()->set(timeData, true);
	this->rescaleAxes();
	QPen graphPen;
	graphPen.setColor(QColor(std::rand() % 245 + 10, std::rand() % 245 + 10, std::rand() % 245 + 10));
	this->graph()->setPen(graphPen);
	this->replot();
}

void NexusPlot::addRandomGraph()
{
	int n = 50; // number of points in graph
	double xScale = (std::rand() / (double)RAND_MAX + 0.5) * 2;
	double yScale = (std::rand() / (double)RAND_MAX + 0.5) * 2;
	double xOffset = (std::rand() / (double)RAND_MAX - 0.5) * 4;
	double yOffset = (std::rand() / (double)RAND_MAX - 0.5) * 10;
	double r1 = (std::rand() / (double)RAND_MAX - 0.5) * 2;
	double r2 = (std::rand() / (double)RAND_MAX - 0.5) * 2;
	double r3 = (std::rand() / (double)RAND_MAX - 0.5) * 2;
	double r4 = (std::rand() / (double)RAND_MAX - 0.5) * 2;
	QVector<double> x(n), y(n);
	for (int i = 0; i < n; i++)
	{
		x[i] = (i / (double)n - 0.5) * 10.0 * xScale + xOffset;
		y[i] = (qSin(x[i] * r1 * 5) * qSin(qCos(x[i] * r2) * r4 * 3) + r3 * qCos(qSin(x[i]) * r4 * 2)) * yScale + yOffset;
	}

	this->addGraph();
	this->graph()->setName(QString("New graph %1").arg(this->graphCount() - 1));
	this->graph()->setData(x, y);
	this->graph()->setLineStyle((QCPGraph::LineStyle)(std::rand() % 5 + 1));
	if (std::rand() % 100 > 50)
		this->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(std::rand() % 14 + 1)));
	QPen graphPen;
	graphPen.setColor(QColor(std::rand() % 245 + 10, std::rand() % 245 + 10, std::rand() % 245 + 10));
	graphPen.setWidthF(std::rand() / (double)RAND_MAX * 2 + 1);
	this->graph()->setPen(graphPen);
	this->replot();
}