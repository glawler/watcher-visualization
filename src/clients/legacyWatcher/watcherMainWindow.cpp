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
