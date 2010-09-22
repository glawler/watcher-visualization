/* Copyright 2010 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_legend.h>
#include <qwt_plot_picker.h>
#include <cstdlib>

#include <logger.h>

#include <libwatcher/watcherTypes.h>

#include "seriesGraphDialog.h"

namespace watcher {
namespace ui {

extern Timestamp EpochTS;

} //namespace
} //namespace

namespace {

    const int MinDetail = 5; // the smallest x-range the detail graph can display

double tsToOffset(watcher::Timestamp t)
{
    return (t - watcher::ui::EpochTS) / 1000.0;
}

} //namespace

namespace watcher {
namespace ui {

INIT_LOGGER(SeriesGraphDialog, "SeriesGraphDialog");

class NodeInfo {
    QString id;
    std::vector<double> xdata;
    std::vector<double> ydata;
    QwtPlotCurve *detailCurve;
    QwtPlotCurve *globalCurve;

    public:
    NodeInfo(const QString& id_, QwtPlot *detailPlot, QwtPlot *globalPlot) : id(id_) {
	QPen pen(QColor(random() % 256, random() % 256, random() % 256));

	detailCurve = new QwtPlotCurve(id_);
	detailCurve->attach(detailPlot);
	detailCurve->setPen(pen);

	globalCurve = new QwtPlotCurve(id_);
	globalCurve->attach(globalPlot);
	globalCurve->setPen(pen);
    }

    void data_point(qlonglong when, double value) {
	/* the x-axis (time) is number of seconds elapsed since the first event in the stream */
	double t = (double)(when - EpochTS) / 1000.0;
	if (xdata.size() && t <= xdata[0]) {
	    xdata.insert(xdata.begin(), t);
	    ydata.insert(ydata.begin(), value);
	} else {
	    xdata.push_back(t);
	    ydata.push_back(value);
	}
	detailCurve->setRawData(&xdata[0], &ydata[0], xdata.size());
	globalCurve->setRawData(&xdata[0], &ydata[0], xdata.size());
    }
};

SeriesGraphDialog::SeriesGraphDialog(const QString& name) : firstEvent(-1), lastEvent(0), detailBegin(0), detailEnd(0)
{
    TRACE_ENTER();

    setupUi(this);

    globalPlot->insertLegend(new QwtLegend);
    globalPlot->setAxisTitle(QwtPlot::xBottom, QString::fromUtf8("time (s)"));
    globalPlot->setAxisTitle(QwtPlot::yLeft, name);

    detailPlot->insertLegend(new QwtLegend);
    detailPlot->setAxisTitle(QwtPlot::xBottom, QString::fromUtf8("time (s)"));
    detailPlot->setAxisTitle(QwtPlot::yLeft, name);

    detailTimeMarker = new QwtPlotMarker;
    detailTimeMarker->setLineStyle(QwtPlotMarker::VLine);
    detailTimeMarker->setLinePen(QPen(QColor(0, 255, 0)));
    detailTimeMarker->attach(detailPlot);

    detailBeginMarker = new QwtPlotMarker;
    detailBeginMarker->setLineStyle(QwtPlotMarker::VLine);
    detailBeginMarker->setLinePen(QPen(QColor(0, 0, 255)));
    detailBeginMarker->attach(globalPlot);

    detailEndMarker = new QwtPlotMarker;
    detailEndMarker->setLineStyle(QwtPlotMarker::VLine);
    detailEndMarker->setLinePen(QPen(QColor(0, 0, 255)));
    detailEndMarker->attach(globalPlot);

    globalTimeMarker = new QwtPlotMarker;
    globalTimeMarker->setLineStyle(QwtPlotMarker::VLine);
    globalTimeMarker->setLinePen(QPen(QColor(0, 255, 0)));
    globalTimeMarker->attach(globalPlot);

    QObject::connect(beginSlider, SIGNAL(valueChanged(int)), this, SLOT(setDetailBegin(int)));
    QObject::connect(endSlider, SIGNAL(valueChanged(int)), this, SLOT(setDetailEnd(int)));

    //beginSlider->setTickInterval(MinDetail);
    //beginSlider->setTickPosition(QSlider::TicksBelow);
    beginSlider->setTracking(true);

    //endSlider->setTickInterval(MinDetail);
    //endSlider->setTickPosition(QSlider::TicksBelow);
    endSlider->setTracking(true);

    detailPicker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, detailPlot->canvas());
    detailPicker->setSelectionFlags(QwtPicker::PointSelection);
    globalPicker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, globalPlot->canvas());
    globalPicker->setSelectionFlags(QwtPicker::PointSelection);
    QObject::connect(detailPicker, SIGNAL(selected(const QwtDoublePoint&)), this, SLOT(plotClicked(const QwtDoublePoint&)));
    QObject::connect(globalPicker, SIGNAL(selected(const QwtDoublePoint&)), this, SLOT(plotClicked(const QwtDoublePoint&)));

    TRACE_EXIT();
}

SeriesGraphDialog::~SeriesGraphDialog()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

/** Add a data point for a node to this series.
 * @param fromID id of the sender of this data point
 * @param when the timestamp when the data point was generated (x-axis)
 * @param value the value of the data point (y-axis)
 */
void SeriesGraphDialog::dataPoint(const QString& fromID, qlonglong when, double value)
{
    TRACE_ENTER();

    /* ignore events with timestamps less than the max received so far.  this is to avoid
     * duplicate values when seeking/rewinding.
     */
    if (when >= lastEvent) {
	NodeInfoPtr nodeInfo;
	NodeMap::iterator it = nodeMap.find(fromID);
	if (it == nodeMap.end()) {
	    // not found
	    nodeInfo = NodeInfoPtr(new NodeInfo(fromID, detailPlot, globalPlot));
	    nodeMap[fromID] = nodeInfo;
	    listWidget->addItem(fromID);
	} else {
	    nodeInfo = it->second;
	}
	nodeInfo->data_point(when, value);
    }

    if (when < firstEvent) {
	firstEvent = when;
	double start = tsToOffset(firstEvent);
	beginSlider->setMinimum(start);
	endSlider->setMinimum(start);
    }
    if (when > lastEvent) {
	lastEvent = when;
	double end = tsToOffset(lastEvent);
	beginSlider->setMaximum(end);
	if (detailEnd == endSlider->maximum()) {
	    // auto-scrolling
	    endSlider->setMaximum(end);
	    endSlider->setValue(end);
	} else {
	    endSlider->setMaximum(end);
	}
    }

    handleClock(when);
    TRACE_EXIT();
}

/** Updates the current time and redraws the plot.
 * @param t timestamp to use as the current time.
 */
void SeriesGraphDialog::handleClock(qlonglong t)
{
    TRACE_ENTER();

    globalTimeMarker->setXValue(tsToOffset(t));
    globalPlot->replot();

    detailTimeMarker->setXValue(tsToOffset(t));
    detailPlot->replot();

    TRACE_EXIT();
}

void SeriesGraphDialog::adjustDetail()
{
    // force a reasonable minimum width for the x-axis
    if (detailEnd - detailBegin < MinDetail)
	detailEnd = detailBegin + MinDetail;
    detailPlot->setAxisScale(QwtPlot::xBottom, detailBegin, detailEnd);
    detailPlot->replot();

    detailBeginMarker->setXValue(detailBegin);
    detailEndMarker->setXValue(detailEnd);
    globalPlot->replot();
}

/** Slot for receiving the value from the slider that controls the starting offset of the detail graph. */
void SeriesGraphDialog::setDetailBegin(int val)
{
    TRACE_ENTER();
    detailBegin = val;
    adjustDetail();
    TRACE_EXIT();
}

/** Slot for receiving the value from the slider that controls the ending offset of the detail graph. */
void SeriesGraphDialog::setDetailEnd(int val)
{
    TRACE_ENTER();
    detailEnd = val;
    adjustDetail();
    TRACE_EXIT();
}
 
/** Slot for receiving the coordinates when the user clicks on the plot. */
void SeriesGraphDialog::plotClicked(const QwtDoublePoint& p)
{
    TRACE_ENTER();
    LOG_INFO("user clicked on the point (" << p.x() << ", " << p.y() << ")");
    emit seekStream(EpochTS + p.x() * 1000.0);
    TRACE_EXIT();
}

} // namespace
} // namespace

// vim:sw=4
