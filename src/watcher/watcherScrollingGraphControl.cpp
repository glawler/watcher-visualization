#include <QtGui>
#include <QDialog>

#include <deque>

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

#include "watcherScrollingGraphControl.h"
#include "ui_WatcherScrollingGraphDialogGUI.h"
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
    // No dynamically allocated data here.
    for (GraphPlotMap::iterator g=graphPlotMap.begin(); g!=graphPlotMap.end(); g++)
        delete g->second;
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
    {
        // boost::shared_ptr<QDialog> theDialog(new QDialog);
        // boost::shared_ptr<GraphPlot> thePlot(new GraphPlot(theDialog.get(), label.c_str()));
        
        QDialog *theDialog=new QDialog(this);
        theDialog->resize(474, 353);

        GraphPlot * thePlot=new GraphPlot(theDialog, label.c_str());
        thePlot->setMargin(5);

        graphDialogMap[label]=theDialog;
        graphPlotMap[label]=thePlot;

        QVBoxLayout *layout = new QVBoxLayout(theDialog);
        layout->addWidget(thePlot);


        // GUI stuff stolen from desginer-qt4 generated code.
        // {
        //     // QDialog  *d=theDialog.get();
        //     // QwtPlot *qwtPlot=thePlot.get();
        //     QDialog  *d=theDialog;
        //     QwtPlot *qwtPlot=thePlot;
        //     QGridLayout *gridLayout;
        //     QSpacerItem *spacerItem;
        //     QDialogButtonBox *buttonBox;

        //     d->resize(474, 353);
        //     QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
        //     sizePolicy.setHorizontalStretch(0);
        //     sizePolicy.setVerticalStretch(0);
        //     sizePolicy.setHeightForWidth(d->sizePolicy().hasHeightForWidth());
        //     d->setSizePolicy(sizePolicy);
        //     d->setAutoFillBackground(false);
        //     gridLayout = new QGridLayout(d);
        //     gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        //     // qwtPlot = new QwtPlot(d);
        //     qwtPlot->setObjectName(QString::fromUtf8("qwtPlot"));
        //     qwtPlot->setProperty("thisIsABool", QVariant(false));
        //     gridLayout->addWidget(qwtPlot, 0, 0, 1, 2);
        //     spacerItem = new QSpacerItem(157, 29, QSizePolicy::Expanding, QSizePolicy::Minimum);
        //     gridLayout->addItem(spacerItem, 1, 0, 1, 1);
        //     buttonBox = new QDialogButtonBox(d);
        //     buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        //     QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        //     sizePolicy1.setHorizontalStretch(0);
        //     sizePolicy1.setVerticalStretch(0);
        //     sizePolicy1.setHeightForWidth(buttonBox->sizePolicy().hasHeightForWidth());
        //     buttonBox->setSizePolicy(sizePolicy1);
        //     buttonBox->setOrientation(Qt::Horizontal);
        //     buttonBox->setStandardButtons(QDialogButtonBox::Close|QDialogButtonBox::NoButton|QDialogButtonBox::Save);
        //     gridLayout->addWidget(buttonBox, 1, 1, 1, 1);
        // }
    }

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

void WatcherScrollingGraphControl::showDialogGraph(bool show)
{
    TRACE_ENTER();

    LOG_DEBUG("show dialog graph -> " << (show?"true":"false")); 

    const char *graphName="Bandwidth"; 
    GraphPlotMap::const_iterator gp=graphPlotMap.find(graphName);
    if (gp==graphPlotMap.end())
    {
        LOG_DEBUG("User wants to show bandwidth graph - but we don't have any testnode bandwidth data\n"); 
        QMessageBox::information(this, tr("We are dataless"), QString("There is not yet any bandwidth data to show"));
        TRACE_EXIT();
        return;
    }
    else
    {
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
    }

    TRACE_EXIT();
}


void WatcherScrollingGraphControl::showDialogCPUUsage(bool /*show*/)
{
    TRACE_ENTER();
    QMessageBox::information(this, tr("Not implemented"), QString("The CPU usage graph is not yet implemented")); 
    TRACE_EXIT();
}
