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
 * @file watcherGraphDialog.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#include "watcherGraphDialog.h"

using namespace watcher;

INIT_LOGGER(QWatcherGraphDialog, "QWatcherGraphDialog"); 

QWatcherGraphDialog::QWatcherGraphDialog(const QString &label_, QWidget * parent, Qt::WindowFlags f) : 
    QDialog(parent, f),
    label(label_)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void QWatcherGraphDialog::done(int)
{
    TRACE_ENTER();
    // Just hide it. We still want it around so the embedded GraphPlot can keep track of incoming data.  
    hide(); 
    emit dialogVisible(label, false); 
    TRACE_EXIT();
}

bool QWatcherGraphDialog::createDialog(GraphPlot *thePlot)
{
    TRACE_ENTER();
    resize(474, 353);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(thePlot);
    TRACE_EXIT_RET(true);
    return true;
}

