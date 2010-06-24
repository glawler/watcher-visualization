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
 * @file watcherStreamListDialog.cpp
 * @date 2010-06-03
 */
#include "watcherStreamListDialog.h"
#include "logger.h"

using namespace watcher;
using namespace std;

INIT_LOGGER(WatcherStreamListDialog, "WatcherStreamListDialog"); 

WatcherStreamListDialog::WatcherStreamListDialog(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f)
{
    TRACE_ENTER();
    setupUi(this);
    QObject::connect(treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(selectStream(QTreeWidgetItem*, int)));
    QObject::connect(ok_btn, SIGNAL(clicked()), this, SLOT(selectStream()));
    TRACE_EXIT();
}

WatcherStreamListDialog::~WatcherStreamListDialog()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void WatcherStreamListDialog::addStream(unsigned long uid, const std::string& desc)
{
    TRACE_ENTER();
    QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
    item->setText(0, QString("%1").arg(uid));
    item->setText(1, QString(desc.c_str()));
    treeWidget->addTopLevelItem(item);
    TRACE_EXIT();
}

void WatcherStreamListDialog::selectStream()
{
    TRACE_ENTER();
    QTreeWidgetItem *item = treeWidget->currentItem();
    if (item) {
	unsigned long uid = item->text(0).toULong();
	LOG_DEBUG("Switching to stream uid " << uid);
	emit streamChanged(uid);
    }
    hide();
    TRACE_EXIT();
}

void WatcherStreamListDialog::selectStream(QTreeWidgetItem *item, int /* column */)
{
    TRACE_ENTER();
    unsigned long uid = item->text(0).toULong();
    LOG_DEBUG("Switching to stream uid " << uid);
    emit streamChanged(uid);
    hide();
    TRACE_EXIT();
}
