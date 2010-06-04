/* Copyright 2010 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * @file watcherStreamListDialog.h
 * @date 2010-06-03
 */
#ifndef watcherStreamListDialog_h
#define watcherStreamListDialog_h

#include <QtGui/QDialog>
#include "ui_streamlist.h"
#include "declareLogger.h"

namespace watcher
{
    class WatcherStreamListDialog : public QDialog, public Ui::StreamListDialog
    {
        Q_OBJECT

        public:

            explicit WatcherStreamListDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
            ~WatcherStreamListDialog(); 

	    void addStream(unsigned long uid, const std::string& desc);

        public slots:

	    // user double clicks on a stream
	    void selectStream(QTreeWidgetItem *item, int column);

	    // select the current stream
	    void selectStream();

        signals:

	    void streamChanged(unsigned long uid);

        protected:

        private:

                DECLARE_LOGGER();
    };
}

#endif
