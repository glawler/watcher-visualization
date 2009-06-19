#include <QtGui>
#include <QDialog>

#include <deque>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include "watcherScrollingGraphControl.h"
#include "graphPlot.h"
// #include "marshal.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(WatcherScrollingGraphControl, "WatcherScrollingGraphControl"); 

//static 
WatcherScrollingGraphControl *WatcherScrollingGraphControl::getWatcherScrollingGraphControl()
{
    TRACE_ENTER();
    static WatcherScrollingGraphControl theOneAndOlnyGraphControlYouBetcha;
    TRACE_EXIT();
    return &theOneAndOlnyGraphControlYouBetcha;
}

WatcherScrollingGraphControl::WatcherScrollingGraphControl()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

WatcherScrollingGraphControl::~WatcherScrollingGraphControl()
{
    TRACE_ENTER();
    
    // for (GraphDialogMap::iterator g=graphDialogMap.begin(); g!=graphDialogMap.end();g++)
    //     delete g->second;
    // for (GraphPlotMap::iterator p=graphPlotMap.begin(); p!=graphPlotMap.end(); p++)
    //     delete p->second;
    
    for (vector<StringIndexedMenuItem*>::iterator i=menuItems.begin(); i!=menuItems.end(); ++i)
        delete *i;

    TRACE_EXIT();
}

void WatcherScrollingGraphControl::handleDataPointMessage(const DataPointMessagePtr &message)
{
    TRACE_ENTER();

    LOG_DEBUG("Got Watcher graph message from node " << message->fromNodeID << " with "  
            <<  message->dataPoints.size() << " data points."); 

    bool newPlot=false;
    GraphPlotMap::iterator gp=graphPlotMap.find(message->dataName);
    if (gp==graphPlotMap.end())
    {
        createDialog(message->dataName);
        newPlot=true;
    }

    unsigned long curveId=message->fromNodeID.to_v4().to_ulong();
    BOOST_FOREACH(double point, message->dataPoints)
    {
        graphPlotMap[message->dataName]->addDataPoint(curveId, point);
    }

    if (newPlot)
    {
        graphPlotMap[message->dataName]->curveAndLegendVisible(curveId, false);  // new plots are invisible until clicked in the GUI
        if (menu)
        {
            QAction *action=new QAction(QString::fromStdString(message->dataName), (QObject*)this);
            action->setCheckable(true);

            // There is probably a better way to do this.
            // We create a class that just stores the string. Then we chain the signals bool->string->string.
            StringIndexedMenuItem *item = new StringIndexedMenuItem(QString::fromStdString(message->dataName)); 
            connect(action, SIGNAL(triggered(bool)), item, SLOT(showMenuItem(bool)));
            connect(item, SIGNAL(showMenuItem(QString, bool)), this, SLOT(showGraphDialog(QString, bool)));
            menuItems.push_back(item);     // We have to keep 'item' alive somewhere. 

            menu->addAction(action); 
        }
    }

    TRACE_EXIT();
}

void WatcherScrollingGraphControl::createDialog(const std::string &label)
{
    TRACE_ENTER();
    QWatcherGraphDialog *theDialog=new QWatcherGraphDialog(label.c_str());
    GraphPlot *thePlot=new GraphPlot(theDialog, label.c_str());

    theDialog->createDialog(thePlot); 

    graphDialogMap[label]=theDialog;
    graphPlotMap[label]=thePlot;

    connect(theDialog, SIGNAL(dialogVisible(QString, bool)), this, SLOT(showGraphDialog(QString, bool))); 

    TRACE_EXIT();
}

void WatcherScrollingGraphControl::showGraphDialog(QString label, const bool show)
{
    TRACE_ENTER();
    LOG_DEBUG("show dialog graph -> " << (show?"true":"false")); 

    string graphName(label.toStdString());

    GraphPlotMap::const_iterator gp=graphPlotMap.find(graphName);
    if (gp==graphPlotMap.end())
    {
        LOG_DEBUG("User wants to show " << graphName << " graph - but we don't have any testnode data for that graph. Creating empty dialog and graph.\n"); 
        createDialog(graphName);
    }

    if (show)
        graphDialogMap[graphName]->show();
    else
        graphDialogMap[graphName]->hide();

    // special cases for hardcoded graphs.
    // Need to get rid of these at some point.
    if (graphName=="Bandwidth")
        emit bandwidthDialogShowed(show);
    if (graphName=="Load Average")
        emit loadAverageDialogShowed(show);

    TRACE_EXIT();
}

void WatcherScrollingGraphControl::showBandwidthGraphDialog(bool show)
{
    TRACE_ENTER();
    showGraphDialog("Bandwidth", show); 
    TRACE_EXIT();
}

void WatcherScrollingGraphControl::showLoadAverageGraphDialog(bool show)
{
    TRACE_ENTER();
    showGraphDialog("Load Average", show); 
    TRACE_EXIT();
}

void WatcherScrollingGraphControl::showScrollingGraph(QString graphName)
{
    TRACE_ENTER();
    showGraphDialog(graphName, true); 
    TRACE_EXIT();
}

void WatcherScrollingGraphControl::showNodeDataInGraphs(unsigned int nodeId, bool show)
{
    TRACE_ENTER();
    for (GraphPlotMap::iterator g=graphPlotMap.begin(); g!=graphPlotMap.end(); g++)
    {
        g->second->curveAndLegendVisible(nodeId, show);
        emit nodeDataInGraphsShowed(nodeId, show);
    }
    TRACE_EXIT();
}

void WatcherScrollingGraphControl::toggleNodeDataInGraphs(unsigned int nodeId)
{
    TRACE_ENTER();
    for (GraphPlotMap::iterator g=graphPlotMap.begin(); g!=graphPlotMap.end(); g++)
    {
        g->second->toggleCurveAndLegendVisible(nodeId);
        emit nodeDataInGraphsToggled(nodeId);
    }
    TRACE_EXIT();
}
