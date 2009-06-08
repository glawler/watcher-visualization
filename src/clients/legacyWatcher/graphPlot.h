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
            // showCurve() will call addCurve() if the curve does not yet exist. 
            void showCurve(unsigned int curveId, bool on);

            void toggleCurveAndLegendVisible(unsigned int curveId);
            void curveAndLegendVisible(unsigned int curveId, bool visible);

        signals:

            void curveAndLegendToggled(bool on);

        protected:
            QString title;
            QwtArray<double> timeData;
            QwtArray<double> zeroData;
            const int timeDataSize;

            typedef struct
            {
                QwtArray<double> data;
                boost::shared_ptr<GraphCurve> curve;
                bool pointSet;     
            } CurveData;

            typedef std::map<unsigned int, boost::shared_ptr<CurveData> > GraphData;
            GraphData graphData;

            void timerEvent(QTimerEvent *event);    

        private:
            DECLARE_LOGGER();

            void addCurve(unsigned int curveId);

        private slots:

            void showCurve(QwtPlotItem *, bool on);

    };
}

#endif // CRAZYGRAPHPLOT_THINGY_THAT_PALS_AROUND_WITH_TERRORISTS_YOU_BETCHA_H
