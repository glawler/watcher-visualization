#ifndef CRAZYGRAPHPLOT_THINGY_THAT_PALS_AROUND_WITH_TERRORISTS_YOU_BETCHA_H
#define CRAZYGRAPHPLOT_THINGY_THAT_PALS_AROUND_WITH_TERRORISTS_YOU_BETCHA_H

#include <map>
#include <QtGui>
#include <boost/shared_ptr.hpp>

#include "qwt_plot_layout.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_widget.h"
#include "qwt_legend.h"
#include "qwt_legend_item.h"

#include "logger.h"

namespace watcher
{
    class TimeScaleDraw : public QwtScaleDraw
    {
        public:
            TimeScaleDraw(const QTime &base);
            virtual QwtText label(double v) const;
        private:
            DECLARE_LOGGER();
            QTime baseTime;
    };

    class GraphCurve : public QwtPlotCurve
    {
        public:
            GraphCurve(const QString &title);
            void setColor(const QColor &color);
        private:
            DECLARE_LOGGER();
    };

    class GraphPlot : public QwtPlot
    {
        Q_OBJECT

        public:
            GraphPlot(QWidget *parent, const QString &title_);
            void addDataPoint(unsigned int curveId, float dataPoint);

        public slots:
            void showCurve(unsigned int curveId, bool on);

        protected:
            QString title;
            QwtArray<double> timeData;
            const int timeDataSize;

            typedef std::map<unsigned int, QwtArray<double> > PlotData;
            PlotData plotData;

            typedef std::map<unsigned int, boost::shared_ptr<GraphCurve> > PlotCurves;
            PlotCurves plotCurves;

            void timerEvent(QTimerEvent * /*event*/);

        private:
            DECLARE_LOGGER();
    };
}

#endif // CRAZYGRAPHPLOT_THINGY_THAT_PALS_AROUND_WITH_TERRORISTS_YOU_BETCHA_H
