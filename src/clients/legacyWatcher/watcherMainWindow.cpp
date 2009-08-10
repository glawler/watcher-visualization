/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
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

/** 
 * @file watcherMainWindow.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#include "watcherMainWindow.h"
// #include "watcherScrollingGraphControl.h"
// #include "ui_watcherScrollingGraphGUI.h"
#include "qwt_plot_curve.h"

using namespace watcher;
using namespace std;

INIT_LOGGER(WatcherMainWindow, "WatcherMainWindow"); 

WatcherMainWindow::WatcherMainWindow(QWidget *parent, Qt::WindowFlags flags) : 
    QMainWindow(parent, flags)
{
    TRACE_ENTER();

    // Make the testbed graph data object tell us where there is a new graph to be made.
    // TestbedGraphData *d=TestbedGraphData::getTestbedGraphData();
    // QObject::connect(d, SIGNAL(newGraph(QString)), this, SLOT(newGraph(QString)));

    TRACE_EXIT();
}

WatcherMainWindow::~WatcherMainWindow()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// void WatcherMainWindow::newGraph(QString graphName)
// {
//     TRACE_ENTER();
// 
//     WatcherScrollingGraph g((char*)graphName.data(), this);
//     Ui::WatcherScrollingGraphDialog ui; 
//     ui.setupUi(&g);
//     QObject::connect(&g, SIGNAL(newDataPoint(QString)), this, SLOT(newGraph(QString)));
// 
//     double ydata1[] = { 1, 2, 3, 2, 3, 4, 5, 3, 2, 12, 32,  4,  3,  3 }; 
//     double xdata[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
// 
//     QwtPlotCurve *cpuUsageCurveNode1 = new QwtPlotCurve("one"); 
//     cpuUsageCurveNode1->setData(xdata, ydata1, 13); 
//     cpuUsageCurveNode1->attach(ui.qwtPlot); 
// 
//     double ydata2[] = { 1, 3, 4, 7, 8, 8, 9, 1, 2, 22, 22, 1,  3,  3 }; 
// 
//     QwtPlotCurve *cpuUsageCurveNode2 = new QwtPlotCurve("two"); 
//     cpuUsageCurveNode2->setData(xdata, ydata2, 13);
//     cpuUsageCurveNode2->attach(ui.qwtPlot); 
//     ui.qwtPlot->replot(); 
// 
//     g.setModal(true);
//     g.setSizeGripEnabled(true);
//     g.exec();
// 
//     delete cpuUsageCurveNode1;
//     delete cpuUsageCurveNode2;
// 
//     TRACE_EXIT();
// }
