#pragma once

#include "NexusPch.h"
#include <QMainWindow>
#include <qwidget.h>
#include <QInputDialog>
#include <QVector>
#include <QTableView>

#include "qcustomplot.h"

class NexusPlot : public QCustomPlot
{
	Q_OBJECT

public:
	explicit NexusPlot(QWidget* parent = 0);
	~NexusPlot() = default;

	void set_title(std::string title) {};
	void plot(
		StridedPointer<long long> x,
		StridedPointer<double> y,
		std::string name
	);

protected slots:
	//void titleDoubleClick(QMouseEvent* event);
	//void axisLabelDoubleClick(QCPAxis* axis, QCPAxis::SelectablePart part);
	//void legendDoubleClick(QCPLegend* legend, QCPAbstractLegendItem* item);
	void addRandomGraph();
	void selectionChanged();
	void mousePress();
	void mouseWheel();
	void removeSelectedGraph();
	void removeAllGraphs();
	virtual void contextMenuRequest(QPoint pos);
	void moveLegend();
	void graphClicked(QCPAbstractPlottable* plottable, int dataIndex);
};
