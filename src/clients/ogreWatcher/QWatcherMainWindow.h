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
 * @file QWatcherMainWindow.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2010-11-15
 */
#ifndef QWATCHER_MAIN_WINDOW_H_SOMETHING_FUNNY
#define QWATCHER_MAIN_WINDOW_H_SOMETHING_FUNNY

#include <string>
#include <QtGui/QMainWindow>
#include "declareLogger.h"
#include "ui_ogreWatcher.h"      // the UI class/file for this window: Ui_WatcherMainWindow

namespace watcher
{
    class QWatcherMainWindow : public QMainWindow, public Ui_WatcherMainWindow
    {
        Q_OBJECT

        public:

            explicit QWatcherMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
            ~QWatcherMainWindow(); 

            void loadUi(Ui_WatcherMainWindow &ui); 

        public slots:

        signals:

        protected:
            DECLARE_LOGGER();

            MessageStreamPtr messageStream;
            MessageStreamReactor messageStreamReactor;
            // GTL - could stick a manetGLView in here if we wanted. 

        private:


    };
}

#endif // QWATCHER_MAIN_WINDOW_H_SOMETHING_FUNNY
