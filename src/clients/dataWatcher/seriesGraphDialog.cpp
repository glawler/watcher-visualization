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
#include <qwt_legend.h>
#include <cstdlib>

#include "logger.h"

#include "libwatcher/watcherTypes.h"
#include "seriesGraphDialog.h"

namespace watcher {
namespace ui {

extern watcher::Timestamp EpochTS;

INIT_LOGGER(SeriesGraphDialog, "SeriesGraphDialog");

class NodeInfo {
    QString id;
    std::vector<double> xdata;
    std::vector<double> ydata;
    QwtPlotCurve *curve;

    public:
    NodeInfo(const QString& id_, QwtPlot *plot) : id(id_) {
	curve = new QwtPlotCurve(id_);
	curve->attach(plot);
	curve->setPen(QPen(QColor(random() % 256, random() % 256, random() % 256)));
    }

    void data_point(qlonglong when, double value) {
	/* the x-axis (time) is number of seconds elapsed since the first event in the stream */
	xdata.push_back((double)(when - EpochTS) / 1000.0);
	ydata.push_back(value);
	curve->setRawData(&xdata[0], &ydata[0], xdata.size());
    }
};

SeriesGraphDialog::SeriesGraphDialog(const QString& name)
{
    TRACE_ENTER();
    setupUi(this);
    qwtPlot->insertLegend(new QwtLegend);
    qwtPlot->setAxisTitle(QwtPlot::xBottom, QString::fromUtf8("time (s)"));
    qwtPlot->setAxisTitle(QwtPlot::yLeft, name);
    TRACE_EXIT();
}

SeriesGraphDialog::~SeriesGraphDialog()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void SeriesGraphDialog::data_point(const QString& fromID, qlonglong when, double value)
{
    TRACE_ENTER();

    NodeInfoPtr nodeInfo;
    NodeMap::iterator it = nodeMap.find(fromID);
    if (it == nodeMap.end()) {
	// not found
	nodeInfo = NodeInfoPtr(new NodeInfo(fromID, qwtPlot));
	nodeMap[fromID] = nodeInfo;
	listWidget->addItem(fromID);
    } else {
	nodeInfo = it->second;
    }
    nodeInfo->data_point(when, value);
    qwtPlot->replot();

    TRACE_EXIT();
}

} // namespace
} // namespace

// vim:sw=4
