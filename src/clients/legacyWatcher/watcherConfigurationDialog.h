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
 * @file configurationInformation.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2010-07-11
 */
#ifndef WATCHER_CONFIG_INFO_DIALOG_H
#define WATCHER_CONFIG_INFO_DIALOG_H

#include <QtGui/QDialog>
#include <string>
#include "ui_configurationInformation.h"
#include "declareLogger.h"

namespace watcher
{
    class WatcherConfigurationDialog : public QDialog, public Ui::watcherConfigurationDialog
    {
        Q_OBJECT

        public:

            explicit WatcherConfigurationDialog(std::string &serverName, size_t &maxNodes, size_t &maxLayers, QWidget *parent = 0, Qt::WindowFlags f = 0);
            ~WatcherConfigurationDialog(); 

            public slots:

                void maxNodesChanged(int val);
                void maxLayersChanged(int val);
                void serverNameChanged(const QString &val); 

            signals:

        protected:

            std::string &serverName; 
            size_t &maxNodes, &maxLayers;

        private:

                DECLARE_LOGGER();
    };
}

#endif // WATCHER_CONFIG_INFO_DIALOG_H
