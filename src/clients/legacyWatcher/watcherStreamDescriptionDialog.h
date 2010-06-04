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
 * @file watcherStreamDescriptionDialog.h
 * @date 2010-06-04
 */
#ifndef watcherStreamDescriptionDialog_h
#define watcherStreamDescriptionDialog_h

#include <QtGui/QDialog>
#include "ui_streamdesc.h"
#include "declareLogger.h"

namespace watcher
{
    class WatcherStreamDescriptionDialog : public QDialog, public Ui::StreamDescription
    {
        Q_OBJECT

        public:

            explicit WatcherStreamDescriptionDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
            ~WatcherStreamDescriptionDialog(); 

	public slots:
	
	    void accept();

        signals:

	    void streamDescriptionChanged(const std::string&);

        private:

                DECLARE_LOGGER();
    };
}

#endif
