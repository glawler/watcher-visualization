/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * @file watcherGraphDialog.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
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
