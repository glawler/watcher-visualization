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
 * @file watcherConfigurationInformation.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2010-07-11
 */
#include "WatcherConfigurationDialog.h"
#include "logger.h"

using namespace watcher;
using namespace std;

INIT_LOGGER(WatcherConfigurationDialog, "WatcherConfigurationDialog"); 

WatcherConfigurationDialog::WatcherConfigurationDialog(std::string &sn, size_t &mn, size_t &ml, QWidget *parent, Qt::WindowFlags f) : 
    QDialog(parent, f), serverName(sn), maxNodes(mn), maxLayers(ml)
{
    TRACE_ENTER();
    setupUi(this);

    maxNodesBox->setValue(maxNodes); 
    maxLayersBox->setValue(maxLayers); 
    serverNameEditBox->setText(serverName.c_str());

    connect(maxNodesBox, SIGNAL(valueChanged(int)), this, SLOT(maxNodesChanged(int))); 
    connect(maxLayersBox, SIGNAL(valueChanged(int)), this, SLOT(maxLayersChanged(int))); 
    connect(serverNameEditBox, SIGNAL(textChanged(const QString &)), this, SLOT(serverNameChanged(const QString &))); 

    TRACE_EXIT();
}

void WatcherConfigurationDialog::maxNodesChanged(int val) 
{
    maxNodes=static_cast<size_t>(val); 
}

void WatcherConfigurationDialog::maxLayersChanged(int val) 
{
    maxLayers=static_cast<size_t>(val); 
}

void WatcherConfigurationDialog::serverNameChanged(const QString &val)
{
    serverName=val.toStdString(); 
}

WatcherConfigurationDialog::~WatcherConfigurationDialog()
{
    TRACE_ENTER();
    TRACE_EXIT();
}
