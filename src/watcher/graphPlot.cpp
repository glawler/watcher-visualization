#include <boost/lexical_cast.hpp>

#include <qwt_symbol.h>
#include "graphPlot.h"

using namespace std;
using namespace watcher;

INIT_LOGGER(TimeScaleDraw, "TimeScaleDraw"); 
INIT_LOGGER(GraphCurve, "GraphCurve"); 
INIT_LOGGER(GraphPlot, "GraphPlot"); 

TimeScaleDraw::TimeScaleDraw(const QTime &base): baseTime(base)
{
    TRACE_ENTER();
    TRACE_EXIT();
}
QwtText TimeScaleDraw::label(double v) const
{
    TRACE_ENTER();
    QTime upTime = baseTime.addSecs((int)v);
    return upTime.toString();
    TRACE_EXIT();
}

GraphCurve::GraphCurve(const QString &title): QwtPlotCurve(title)
{
    TRACE_ENTER();
    setRenderHint(QwtPlotItem::RenderAntialiased);
    TRACE_EXIT();
}

void GraphCurve::setColor(const QColor &color)
{
    TRACE_ENTER();
    QColor c = color;
    c.setAlpha(150);
    setPen(c);
    // setBrush(c); // Don't fill below curve.
    TRACE_EXIT();
}

GraphPlot::GraphPlot(QWidget *parent, const QString &title_) : 
    QwtPlot(parent), title(title_), timeDataSize(60)
{
    TRACE_ENTER();
    timeData.reserve(timeDataSize); 

    // Initialize time data to 1, 2, ... sizeof(timeData)-1
    for (int i=timeDataSize; i>0; i--)
        timeData.push_back(i);

    setTitle(title);

    setAutoReplot(false);
    plotLayout()->setAlignCanvasToScales(true);

    QwtLegend *legend = new QwtLegend;
    legend->setItemMode(QwtLegend::CheckableItem);
    insertLegend(legend, QwtPlot::RightLegend);

    setAxisTitle(QwtPlot::xBottom, "Time"); 
    setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(QTime::currentTime()));
    setAxisScale(QwtPlot::xBottom, 0, timeDataSize);
    setAxisLabelRotation(QwtPlot::xBottom, -50.0);
    setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

    // add space for right aligned label
    QwtScaleWidget *scaleWidget = axisWidget(QwtPlot::xBottom);
    const int fmh = QFontMetrics(scaleWidget->font()).height();
    scaleWidget->setMinBorderDist(0, fmh / 2);

    setAxisTitle(QwtPlot::yLeft, title); 
    // setAxisScale(QwtPlot::yLeft, 0, 100);

    LOG_DEBUG("Starting 1 second timer for graph plot " << title.toStdString()); 
    (void)startTimer(1000); // 1 second

    connect(this, SIGNAL(legendChecked(QwtPlotItem *, bool)), SLOT(showCurve(QwtPlotItem *, bool)));
    
    TRACE_EXIT();
}

void GraphPlot::addDataPoint(unsigned int curveId, float dataPoint)
{
    TRACE_ENTER();
    GraphData::iterator data=graphData.find(curveId);

    // Create a new curve, if we've never seen this curveId before.
    if (data==graphData.end())
        addCurve(curveId);

    LOG_DEBUG("Pushing back data point " << dataPoint << " for curve id " << (0xFF & curveId));
    graphData[curveId]->data.push_front(dataPoint);
    graphData[curveId]->pointSet=true; 

    // we only want timeDataSize data points, so trim front if longer than that.
    if (graphData[curveId]->data.size()>timeDataSize)
        graphData[curveId]->data.pop_back();      

    TRACE_EXIT();
}

void GraphPlot::timerEvent(QTimerEvent * /*event*/)
{
    TRACE_ENTER();
    for (int i=0;i<timeDataSize;i++)
        timeData[i]++;

    setAxisScale(QwtPlot::xBottom, timeData[timeDataSize-1], timeData[0]);

    for (GraphData::iterator d=graphData.begin(); d!=graphData.end(); d++)
    {
        if (!d->second->pointSet)           // Didn't hear from the node for last second, set data to zero.
        {
            LOG_DEBUG("Didn't hear from node " << (0xFF & d->first) << " setting this second's data to 0"); 

            d->second->data.push_front(0);

            if (d->second->data.size()>timeDataSize)
                d->second->data.pop_back();      
        }
        else
            d->second->pointSet=false;  // reset for next go around.

        d->second->curve->setData(timeData, d->second->data);
    }

    replot(); 
    TRACE_EXIT();
}

void GraphPlot::showCurve(QwtPlotItem *curve, bool on)
{
    TRACE_ENTER();

    LOG_DEBUG("Setting curve " << (on?"":"in") << "visible.")

    curve->setVisible(on);

    QWidget *w = legend()->find(curve);
    if ( w && w->inherits("QwtLegendItem") )
        ((QwtLegendItem *)w)->setChecked(on);

    TRACE_EXIT();
}

void GraphPlot::showCurve(unsigned int curveId, bool on)
{
    TRACE_ENTER();

    if (graphData.end()==graphData.find(curveId))
    {
        LOG_DEBUG("User wants to show curve for a node we've never seen, adding empty curve with id " << (0xFF & curveId));
        addCurve(curveId);
    }

    showCurve(graphData[curveId]->curve.get(), on);     
    replot();

    TRACE_EXIT();
}

void GraphPlot::toggleCurveAndLegendVisible(unsigned int curveId)
{
    TRACE_ENTER();
    if (graphData.end()==graphData.find(curveId))
    {
        LOG_DEBUG("User wants to toggle visibility for a  curve we've never seen, adding empty curve with id " << (0xFF & curveId));
        addCurve(curveId);
        graphData[curveId]->curve->setVisible(false); 
    }

    curveAndLegendVisible(curveId, !graphData[curveId]->curve->isVisible());

    TRACE_EXIT();
}

void GraphPlot::curveAndLegendVisible(unsigned int curveId, bool visible)
{
    TRACE_ENTER();

    if (graphData.end()==graphData.find(curveId))
    {
        LOG_DEBUG("User wants to set/unset visibility for a  curve we've never seen, adding empty curve with id " << (0xFF & curveId));
        addCurve(curveId);
    }

    showCurve(graphData[curveId]->curve.get(), visible);

    QWidget *w = legend()->find(graphData[curveId]->curve.get());
    if ( w && w->inherits("QwtLegendItem") )
        ((QwtLegendItem *)w)->setVisible(visible);

    emit curveAndLegendToggled(visible);

    TRACE_EXIT();
}

void GraphPlot::addCurve(unsigned int curveId)
{
    TRACE_ENTER();
    boost::shared_ptr<CurveData> data(new CurveData);
    for (int i=0; i<timeDataSize; i++)
        data->data.push_back(0);
    graphData[curveId]=data;

    string curveTitle=boost::lexical_cast<string>((unsigned int)(0xFF & curveId));
    data->curve=boost::shared_ptr<GraphCurve>(new GraphCurve(curveTitle.c_str()));

    data->curve->setColor(QColor::fromHsv(qrand() % 256, 255, 190)); 
    data->curve->setStyle(QwtPlotCurve::Lines);
    data->curve->attach(this);

    LOG_DEBUG("Created new curve " << (0xFF & curveId)); 
    TRACE_EXIT();
}

