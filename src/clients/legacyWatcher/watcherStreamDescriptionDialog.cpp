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
 * @file watcherStreamDescriptionDialog.cpp
 * @date 2010-06-03
 */
#include "watcherStreamDescriptionDialog.h"
#include "logger.h"

using namespace watcher;
using namespace std;

INIT_LOGGER(WatcherStreamDescriptionDialog, "WatcherStreamDescriptionDialog"); 

WatcherStreamDescriptionDialog::WatcherStreamDescriptionDialog(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f)
{
    TRACE_ENTER();
    setupUi(this);
    TRACE_EXIT();
}

WatcherStreamDescriptionDialog::~WatcherStreamDescriptionDialog()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void WatcherStreamDescriptionDialog::accept()
{
    TRACE_ENTER();
    emit streamDescriptionChanged(lineEdit->text().toStdString());
    close();
    TRACE_EXIT();
}
