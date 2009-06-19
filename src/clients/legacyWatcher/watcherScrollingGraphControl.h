#ifndef GRAPH_DATA_WATHCER_FOOBAR_OH_YEAH_ITS_NOW_ACTULLAY_CALLED_WATCHERSCROLLINGRAPHCONTROL_H
#define GRAPH_DATA_WATHCER_FOOBAR_OH_YEAH_ITS_NOW_ACTULLAY_CALLED_WATCHERSCROLLINGRAPHCONTROL_H

#include <map>
#include <QWidget>

#include "watcherGraphDialog.h"
#include "libwatcher/dataPointMessage.h"
#include "logger.h"

namespace watcher
{
    class GraphPlot; 

    class GraphMenuItem : public QObject
    {
        Q_OBJECT

        public: 
            GraphMenuItem(const QString &str_);

       public slots:
                void showGraph(bool b) { emit showGraph(str, b); }

       signals:
            void showGraph(QString str, bool show); 

        private:
            const QString str;
    };

    class WatcherScrollingGraphControl : public QWidget
    {
        Q_OBJECT 

        public:

            // singleton class.
            static WatcherScrollingGraphControl *getWatcherScrollingGraphControl(); 

            // Add the data in the incoming watcherGraph message into the appropriate
            // place. Create a new scrolling grpah dialog if needed. 
            //
            void handleDataPointMessage(const event::DataPointMessagePtr &message); 

            // Give the control a menu on which it can add actions. 
            void setMenu(QMenu *m) { menu=m; }

       public slots:

                // General case of show dialog.
                void showGraphDialog(QString graphName, bool show);

            // Specific case of showDialog, just calls the general case with the appropriate string.
            void showBandwidthGraphDialog(bool show);
            void showLoadAverageGraphDialog(bool show);

            void showScrollingGraph(QString graphName);

            void showNodeDataInGraphs(unsigned int nodeId, bool show);
            void toggleNodeDataInGraphs(unsigned int nodeId);

        signals:

            void bandwidthDialogShowed(bool show);
            void loadAverageDialogShowed(bool show);
            void graphDialogShowed(QString graphName, bool show);

            void nodeDataInGraphsShowed(unsigned int nodeId, bool show);
            void nodeDataInGraphsToggled(unsigned int nodeId); 

        protected:

            // Don't create or copy.
            WatcherScrollingGraphControl();
            ~WatcherScrollingGraphControl();
            WatcherScrollingGraphControl(const WatcherScrollingGraphControl &);
            WatcherScrollingGraphControl &operator=(const WatcherScrollingGraphControl &);

            // add a new dialog to this control, with graph labeled/indexed by 'label'
            void createDialog(const std::string &label);

        private:

            // Where we put our graph actions.
            QMenu *menu; 

            // A place to store graphMeny items. Wee just have to 
            // keep them alive until dtor is called.
            std::vector<GraphMenuItem*> graphMenuItems;

            // Oor own private idaho^Wlogger
            DECLARE_LOGGER(); 

            // typedef std::map<std::string, boost::shared_ptr<GraphPlot> > GraphPlotMap;
            typedef std::map<std::string, GraphPlot*> GraphPlotMap;
            GraphPlotMap graphPlotMap;

            // typedef std::map<std::string, boost::shared_ptr<QDialog> > GraphDialogMap;
            typedef std::map<std::string, QWatcherGraphDialog*> GraphDialogMap;
            GraphDialogMap graphDialogMap;
    };

}  // end namesapce

#endif //  GRAPH_DATA_WATHCER_FOOBAR_OH_YEAH_ITS_NOW_ACTULLAY_CALLED_WATCHERSCROLLINGRAPHCONTROL_H
