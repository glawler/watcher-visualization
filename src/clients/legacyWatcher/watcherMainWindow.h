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
 * @file watcherMainWindow.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#ifndef WATRCHER_AMINE_WAINFODW_YOU_BETCHA_H
#define WATRCHER_AMINE_WAINFODW_YOU_BETCHA_H

#include <string>
#include <QtGui/QMainWindow>
#include "logger.h"

namespace watcher
{
    class WatcherMainWindow : public QMainWindow
    {
        Q_OBJECT

        public:

            explicit WatcherMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
            ~WatcherMainWindow(); 

            public slots:

            signals:

        protected:

        private:

                DECLARE_LOGGER();
    };
}

#endif // WATRCHER_AMINE_WAINFODW_YOU_BETCHA_H
