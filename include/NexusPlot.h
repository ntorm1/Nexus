#pragma once

#include "NexusPch.h"
#include <QMainWindow>
#include <optional>
#include <qwidget.h>
#include <QInputDialog>
#include <QVector>
#include <QTableView>

#include "qcustomplot.h"

#include "Trade.h"

struct Point
{
	long long datetime_index;
	double value;
};


class NexusPlot : public QCustomPlot
{
	Q_OBJECT

public:
	explicit NexusPlot(QWidget* parent = 0);
	~NexusPlot() = default;

	void set_title(std::string title);
	void plot(
		std::span<long long> x,
		std::span<double const> y,
		std::string name
	);

	//void scatter_plot();

protected:
	std::optional<std::string> selected_line = std::nullopt;

protected slots:
	//void titleDoubleClick(QMouseEvent* event);
	//void axisLabelDoubleClick(QCPAxis* axis, QCPAxis::SelectablePart part);
	//void legendDoubleClick(QCPLegend* legend, QCPAbstractLegendItem* item);
	void addRandomGraph();
	void selectionChanged();
	void mousePress();
	void mouseWheel();
	virtual void removeSelectedGraph();
	virtual void remove_graph_by_name(std::string const& name);
	virtual void removeAllGraphs();
	virtual void contextMenuRequest(QPoint pos);
	void moveLegend();
	void graphClicked(QCPAbstractPlottable* plottable, int dataIndex);
};
