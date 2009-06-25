#ifndef WATCHER_GRAPH_DIALOG_H_YOU_BETCHA
#define WATCHER_GRAPH_DIALOG_H_YOU_BETCHA

#include <Qt/qdialog.h>

#include "graphPlot.h"
#include "logger.h"

namespace watcher
{
    class QWatcherGraphDialog : public QDialog
    {
        Q_OBJECT

        public:
            QWatcherGraphDialog(const QString &label, QWidget * parent = 0, Qt::WindowFlags f = 0);
            bool createDialog(GraphPlot *thePlot);

            void done(int result);  // intercept dialog close

            signals:

            void dialogVisible(QString label, bool isVisible);

        protected:
        private:

            QString label;

            DECLARE_LOGGER();

    };
}

#endif // WATCHER_GRAPH_DIALOG_H_YOU_BETCHA
