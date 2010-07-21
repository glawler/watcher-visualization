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
 * @file nodeConfigurationDialog.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2010-07-20
 */
#ifndef NODE_WATCHER_CONFIG_INFO_DIALOG_H
#define NODE_WATCHER_CONFIG_INFO_DIALOG_H

#include <QtGui/QDialog>
#include <string>
#include "ui_nodeConfigurationDialog.h"
#include "libwatcher/watcherGraph.h"
#include "declareLogger.h"

namespace watcher
{
    class NodeConfigurationDialog : public QDialog, public Ui::nodeConfigurationDialog
    {
        Q_OBJECT

        public:

            NodeConfigurationDialog(WatcherGraph *&g, QWidget *parent = 0, Qt::WindowFlags f = 0);
            virtual ~NodeConfigurationDialog(); 

            /**
             * Set the graph which containts the nodes to be configured.
             * Do this before configuring nodes for maximum configuration
             * effectiveness. 
             */
            void setGraph(WatcherGraph *&g) { graph=g; }
            /** 
             * Set widgets to values found in the nodes. Call this 
             * before show() to initialize the dialog to current 
             * node configuration.
             */
            void configureDialog();

            /**
             * Configure the dialog from a partiular node index.
             * The node must exist and be valid!
             */
            void configureDialog(size_t nodeId);

        public slots:

            void setNodeLabelColor(void);
            void setNodeLabelFont(void);
            void setNodeColor(void);
            void setNodeLabel(QString str);
            void setNodeShape(QString str);
            void setNodeSize(int size);

            void nodeClicked(size_t nodeId); 

        signals:

        protected:
    
            /** If true, modify single node at nodes[id] */
            bool useNodeId; 

            /** Index into node array, if in single-node mode */
            size_t curNodeId;

        private:

            WatcherGraph *&graph;

            DECLARE_LOGGER();
    };
}

#endif // NODE_WATCHER_CONFIG_INFO_DIALOG_H
