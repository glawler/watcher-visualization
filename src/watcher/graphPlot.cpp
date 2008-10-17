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
    setBrush(c);
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

    {
        static QVector<double> tmpData;
        tmpData.push_back(1.2);
        tmpData.push_back(2.3);
        tmpData.push_back(3.4);
        tmpData.push_back(4.5);
        tmpData.push_back(5.6);
        tmpData.push_back(6.7);

        GraphCurve *c=new GraphCurve("bogus"); 
        c->setColor(Qt::blue);
        c->attach(this);
        c->setData(timeData, tmpData); 

        QwtSymbol sym;
        sym.setStyle(QwtSymbol::Cross);
        sym.setPen(QColor(Qt::black));
        sym.setSize(5);
        c->setSymbol(sym);
        c->setPen(QColor(Qt::darkGreen));
        c->setStyle(QwtPlotCurve::Lines);
        c->setCurveAttribute(QwtPlotCurve::Fitted);

    }

    LOG_DEBUG("Starting 1 second timer for graph plot " << title.toStdString()); 
    (void)startTimer(1000); // 1 second

    connect(this, SIGNAL(legendChecked(QwtPlotItem *, bool)), SLOT(showCurve(QwtPlotItem *, bool)));
    
    TRACE_EXIT();
}

void GraphPlot::addDataPoint(unsigned int curveId, float dataPoint)
{
    TRACE_ENTER();
    PlotData::iterator data=plotData.find(curveId);

    // Create a new curve, if we've never seen this curveId before.
    if (data==plotData.end())
    {
        string curveTitle=boost::lexical_cast<string>((unsigned int)(0xFF & curveId));
        // boost::shared_ptr<GraphCurve> curve(new GraphCurve(curveTitle.c_str())); 
        GraphCurve *curve=new GraphCurve(curveTitle.c_str());
        plotCurves[curveId]=curve;

        curve->setColor(Qt::red);
        // curve->setZ(curve->z()-plotCurves.size()-1);
        curve->attach(this);

        showCurve(curveId, true); 

        LOG_DEBUG("Created new curve " << (0xFF & curveId)); 
    }

    LOG_DEBUG("Pushing back data point " << dataPoint << " for curve id " << (0xFF & curveId));
    plotData[curveId].push_back(dataPoint);

    // we only want timeDataSize data points, so trim front if longer than that.
    if (plotData[curveId].size()>timeDataSize)
        plotData[curveId].pop_front();      

    TRACE_EXIT();
}

void GraphPlot::timerEvent(QTimerEvent * /*event*/)
{
    TRACE_ENTER();
    for (int i=0;i<timeDataSize;i++)
        timeData[i]++;

    setAxisScale(QwtPlot::xBottom, timeData[timeDataSize-1], timeData[0]);

    for (PlotCurves::iterator c=plotCurves.begin(); c!=plotCurves.end(); c++)
        c->second->setData(timeData, plotData[c->first]);

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

    if (plotCurves.end()==plotCurves.find(curveId))
    {
        LOG_DEBUG("User wants to show curve for a node we've never seen");
        // GTL - throw up a notification box here? 
        TRACE_EXIT();
    }

    showCurve(plotCurves[curveId], on);     
    replot();
    TRACE_EXIT();
}

