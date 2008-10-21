#include <QtGui>
#include <QDialog>

#include <deque>

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

#include "watcherScrollingGraphControl.h"
#include "graphPlot.h"
#include "marshal.h"

using namespace std;
using namespace watcher;

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
    for (GraphPlotMap::iterator p=graphPlotMap.begin(); p!=graphPlotMap.end(); p++)
        delete p->second;
    for (GraphDialogMap::iterator d=graphDialogMap.begin(); d!=graphDialogMap.end(); d++)
        delete d->second;
    TRACE_EXIT();
}

void WatcherScrollingGraphControl::unmarshalWatcherGraphMessage(const unsigned int nodeAddress, const unsigned char *payload)
{
    TRACE_ENTER();

    char labelBuf[260];     // meh. 
    unsigned int numOfPoints;

    UNMARSHALSTRINGSHORT(payload, labelBuf);
    string label(labelBuf);
    UNMARSHALLONG(payload, numOfPoints);

    LOG_DEBUG("Got Watcher graph message from node " << (0xFF & nodeAddress) << " with "  
           <<  numOfPoints << " data points."); 

    GraphPlotMap::iterator gp=graphPlotMap.find(label);
    if (gp==graphPlotMap.end())
        createDialog(label);

    for (unsigned int i=0; i < numOfPoints; i++)
    {
        char floatBuf[64];
        UNMARSHALSTRINGSHORT(payload, floatBuf);
        try
        {
            float tmpFloat=boost::lexical_cast<float>(static_cast<const char*>(floatBuf));
            graphPlotMap[label]->addDataPoint(nodeAddress, tmpFloat);

            LOG_DEBUG("New data point: " << label << ":" << nodeAddress << " ("  
                    << (0xFF & nodeAddress) << "):" << tmpFloat);
        }
        catch (boost::bad_lexical_cast &e)
        {
            LOG_ERROR("Unable to parse incoming graph floating point value from testnode " 
                    << (0xFF & nodeAddress) << ". Error: " << e.what());
        }
    }

    TRACE_EXIT();
}

void WatcherScrollingGraphControl::createDialog(const std::string &label)
{
    TRACE_ENTER();
    // boost::shared_ptr<QDialog> theDialog(new QDialog);
    QDialog *theDialog=(new QDialog);
    theDialog->resize(474, 353);

    // boost::shared_ptr<GraphPlot> thePlot(new GraphPlot(theDialog.get(), label.c_str()));
    GraphPlot *thePlot=new GraphPlot(theDialog, label.c_str());
    thePlot->setMargin(5);

    graphDialogMap[label]=theDialog;
    graphPlotMap[label]=thePlot;

    QVBoxLayout *layout = new QVBoxLayout(theDialog);
    layout->addWidget(thePlot);
    TRACE_EXIT();
}

void WatcherScrollingGraphControl::showDialogGraph(bool show)
{
    TRACE_ENTER();

    LOG_DEBUG("show dialog graph -> " << (show?"true":"false")); 

    const char *graphName="Bandwidth"; 
    GraphPlotMap::const_iterator gp=graphPlotMap.find(graphName);
    if (gp==graphPlotMap.end())
    {
        LOG_DEBUG("User wants to show bandwidth graph - but we don't have any testnode bandwidth data. Creating empty dialog and graph.\n"); 
        // QMessageBox::information(this, tr("We are dataless"), QString("There is not yet any bandwidth data to show"));
        createDialog(graphName);
    }
    if (show)
    {
        graphDialogMap[graphName]->show();
        emit showDialog(show);
    }
    else
    {
        graphDialogMap[graphName]->hide();
        emit showDialog(show);
    }

    TRACE_EXIT();
}


void WatcherScrollingGraphControl::showDialogCPUUsage(bool /*show*/)
{
    TRACE_ENTER();
    QMessageBox::information(this, tr("Not implemented"), QString("The CPU usage graph is not yet implemented")); 
    TRACE_EXIT();
}

void WatcherScrollingGraphControl::showNodeDataInGraphs(unsigned int nodeId, bool show)
{
    TRACE_ENTER();
    for (GraphPlotMap::iterator g=graphPlotMap.begin(); g!=graphPlotMap.end(); g++)
        g->second->curveAndLegendVisible(nodeId, show);
    TRACE_EXIT();
}

void WatcherScrollingGraphControl::toggleNodeDataInGraphs(unsigned int nodeId)
{
    TRACE_ENTER();
    for (GraphPlotMap::iterator g=graphPlotMap.begin(); g!=graphPlotMap.end(); g++)
        g->second->toggleCurveAndLegendVisible(nodeId);
    TRACE_EXIT();
}
