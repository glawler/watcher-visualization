/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
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
 * @file watcherAbourDialog.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2010-02-07
 */
#ifndef SOTHEN_WHATS_ALL_THIS_ABOUT_THE_WATCHER_H
#define SOTHEN_WHATS_ALL_THIS_ABOUT_THE_WATCHER_H

#include <QtGui/QDialog>
#include "ui_aboutDialog.h"
#include "declareLogger.h"

namespace watcher
{
    class WatcherAboutDialog : public QDialog, public Ui::AboutDialog
    {
        Q_OBJECT

        public:

            explicit WatcherAboutDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
            ~WatcherAboutDialog(); 

            public slots:

            signals:

        protected:

        private:

                DECLARE_LOGGER();
    };
}

#endif // SOTHEN_WHATS_ALL_THIS_ABOUT_THE_WATCHER_H
