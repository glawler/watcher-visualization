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

#include <QtGui/qapplication.h>
#include "watcherMainWindow.h"
#include "watcherConfig.h"
#include "ui_mainwindow.h"

int main(int argc, char **argv)
{
    watcher::config::initialize(argc, argv);
    QApplication app(argc, argv);
    watcher::ui::MainWindow* window = new watcher::ui::MainWindow();
    window->setup();
    window->show();
    return app.exec();
}

// vim:sw=4
