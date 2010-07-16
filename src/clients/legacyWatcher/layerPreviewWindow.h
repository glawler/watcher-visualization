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
 * @file layerPreviewWindow.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2010-07-13
 */
#ifndef LAYER_PREVIEW_WINDOW_H
#define LAYER_PREVIEW_WINDOW_H

#include <QGLWidget>
#include "declareLogger.h"

#include "libwatcher/watcherLayerData.h"
#include "libwatcher/nodeDisplayInfo.h"

namespace watcher
{
    class LayerPreviewWindow : public QGLWidget
    {
        Q_OBJECT

        public:

            /**
             * Create a preview window. 
             * @param WatcherLayerData & - the layer data to preview. Note that this is a reference. 
             *      Any changes made to the layer data object will be shown in the preview window.
             *
             *      The layer must have exactly one node label and floating label.
             */
            LayerPreviewWindow(QWidget *parent = 0);
            ~LayerPreviewWindow(); 

            void setLayerData(WatcherLayerData *l); 

            public slots:

            signals:

        protected:

            NodeDisplayInfo node1, node2;
            WatcherLayerData *layer; 

            void initializeGL();
            void resizeGL(int w, int h);
            void paintGL();

            void drawGraph();
            void drawNode(const NodeDisplayInfo &node);
            void drawEdge(const EdgeDisplayInfo &edge, const NodeDisplayInfo &node1, const NodeDisplayInfo &node2); 
            void drawLabel(const GLfloat &x, const GLfloat &y, const GLfloat &z, const LabelDisplayInfo &l);

        private:

                    DECLARE_LOGGER();
    };
}

#endif // LAYER_PREVIEW_WINDOW_H
