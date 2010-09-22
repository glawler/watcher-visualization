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

#include <tr1/memory>

#include <libwatcher/watcherTypes.h>
#include <declareLogger.h>

#include "ui_graph.h"

class QwtPlotMarker; // forward decl
class QwtPlotPicker; // forward decl

namespace watcher {
namespace ui {

class NodeInfo; //forward decl
typedef std::tr1::shared_ptr<NodeInfo> NodeInfoPtr;

/** Dialog box for plotting data from multiple nodes for a single data series.  */
class SeriesGraphDialog : public QWidget, Ui::Form {
    private:
	Q_OBJECT
	DECLARE_LOGGER();

	SeriesGraphDialog(const SeriesGraphDialog&);
	SeriesGraphDialog& operator=(const SeriesGraphDialog&);
	void adjustDetail();
	void replot();

	typedef std::map<QString, NodeInfoPtr> NodeMap;
	NodeMap nodeMap; // set of all nodes with data for this series

	QwtPlotMarker* globalTimeMarker;
	QwtPlotMarker* detailTimeMarker; // vline indicating current simulation time
	QwtPlotMarker* detailBeginMarker; // marker showing the starting offset of the detail graph in the global plot
	QwtPlotMarker* detailEndMarker; // marker showing the ending offset of the detail graph in the global plot

	// used to allow the user to click on a plot to seek
	QwtPlotPicker* detailPicker;
	QwtPlotPicker* globalPicker;
	
	Timestamp firstEvent; // lowest timestamp received
	Timestamp lastEvent; // highest timestamp received
	int detailBegin; // lower bound on timestamp for detail graph
	int detailEnd; // upper bound on timestamp for detail graph

    public:
	explicit SeriesGraphDialog(const QString&);
	~SeriesGraphDialog();
	void dataPoint(const QString&, qlonglong, double);

    public slots:
	void handleClock(qlonglong);
	void plotClicked(const QwtDoublePoint&);
	void selectionChanged();
	void setDetailBegin(int val);
	void setDetailEnd(int val);
	
    signals:
	void seekStream(qlonglong);
};

} // namespace
} // namespace

// vim:sw=4
