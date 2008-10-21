#ifndef GRAPH_DATA_WATHCER_FOOBAR_OH_YEAH_ITS_NOW_ACTULLAY_CALLED_WATCHERSCROLLINGRAPHCONTROL_H
#define GRAPH_DATA_WATHCER_FOOBAR_OH_YEAH_ITS_NOW_ACTULLAY_CALLED_WATCHERSCROLLINGRAPHCONTROL_H

#include <map>
#include <vector>
#include <QWidget>

// #include <boost/shared_ptr.hpp>  Qt doesn't like shared_ptrs, apparently.

#include "logger.h"

namespace watcher
{
    class GraphPlot; 

    class WatcherScrollingGraphControl : public QWidget
    {
        Q_OBJECT 

        public:

            // singleton class.
            static WatcherScrollingGraphControl *getWatcherScrollingGraphControl(); 

            // Add the data in the incoming watcherGraph message into the appropriate
            // place. Create a new scrolling grpah dialog if needed. 
            //
            // The WatcherGraph message has to be of the form:
            // byte labelsize;
            // char label[labelsize]
            // unsigned int datanum;
            // unsigned int data[datanum], where data[i] is a (int)(float(x)/1000000.0)
            //
            void unmarshalWatcherGraphMessage(const unsigned int nodeId, const unsigned char *payload); 


            public slots:

                void showDialogGraph(bool show);
                void showDialogCPUUsage(bool show);

                void showNodeDataInGraphs(unsigned int nodeId, bool show);
                void toggleNodeDataInGraphs(unsigned int nodeId);

            signals:

                void showDialog(bool show);

        protected:

            // Don't create or copy.
            WatcherScrollingGraphControl();
            ~WatcherScrollingGraphControl();
            WatcherScrollingGraphControl(const WatcherScrollingGraphControl &);
            WatcherScrollingGraphControl &operator=(const WatcherScrollingGraphControl &);

            // add a new dialog to this control, with graph labeled/indexed by 'label'
            void createDialog(const std::string &label);

        private:

            // Oor own private idaho^Wlogger
            DECLARE_LOGGER(); 

            // typedef std::map<std::string, boost::shared_ptr<GraphPlot> > GraphPlotMap;
            typedef std::map<std::string, GraphPlot*> GraphPlotMap;
            GraphPlotMap graphPlotMap;

            // typedef std::map<std::string, boost::shared_ptr<QDialog> > GraphDialogMap;
            typedef std::map<std::string, QDialog*> GraphDialogMap;
            GraphDialogMap graphDialogMap;
    };

}  // end namesapce

#endif //  GRAPH_DATA_WATHCER_FOOBAR_OH_YEAH_ITS_NOW_ACTULLAY_CALLED_WATCHERSCROLLINGRAPHCONTROL_H
