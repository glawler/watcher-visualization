
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
 * @file layerConfiguyrationDialog.h.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2010-07-13
 */
#ifndef LAYER_WATCHER_CONFIG_INFO_DIALOG_H
#define LAYER_WATCHER_CONFIG_INFO_DIALOG_H

#include <QtGui/QDialog>
#include <string>
#include "ui_layerConfigurationDialog.h"
#include "libwatcher/watcherLayerData.h"
#include "layerPreviewWindow.h"
#include "declareLogger.h"

namespace watcher
{
    class LayerConfigurationDialog : public QDialog, public Ui::layerConfigurationDialog
    {
        Q_OBJECT

        public:

            explicit LayerConfigurationDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
            virtual ~LayerConfigurationDialog(); 

        public slots:

            // layer stuff
            void addLayer(WatcherLayerData *l); 
            void setCurrentLayer(QListWidgetItem*);
            void layerToggle(std::string &name); 

            // edges
            void updateEdgeLabel(QString text);
            void setEdgeColor();
            void setEdgeLabelColor();
            void setEdgeLabelFont();
            void setEdgeWidth(int w);

            // labels (attached)
            void setNodeLabelColor();
            void setNodeLabelFont(); 

            // labels (floating)
            void setFloatingLabelColor();
            void setFloatingLabelFont();

            void layerToggled(bool toggled);

        signals:

            void layerToggled(QString, bool);

        protected:
    
            bool checkForValidCurrentLayer();
            void setColor(Color &c, QToolButton *b);
            void setFont(std::string &fontFamily, float &pointSize);

            /** 
             * Set the dialog widgets to reflect the currently selected layer. 
             * Assumes currentLayer is set.
             */
            void configureDialogWithCurrentLayer(); 
            
            typedef std::list<WatcherLayerData *> LayerList;
            LayerList layers; 

            WatcherLayerData *currentLayer;

            // LayerPreviewWindow previewWindow;

        private:

            DECLARE_LOGGER();
    };
}

#endif // LAYER_WATCHER_CONFIG_INFO_DIALOG_H
