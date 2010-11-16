/* 
 * Copyright 2009, 2010 SPARTA, Inc., dba Cobham Analytic Solutions
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
 * @file watcherQtGUIConfig.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2010-10-22
 */
#ifndef WATHCER_CUTIE_GOOEY_CONFIG_H
#define WATHCER_CUTIE_GOOEY_CONFIG_H

#include <string>
#include <QtGui/QWidget>
#include <libwatcher/watcherColors.h>
#include <libwatcher/watcherTypes.h>
#include <declareLogger.h>
#include "watcherGUIConfig.h"

namespace watcher
{
    class QWatcherGUIConfig : public QWidget, public WatcherGUIConfig
    {
        Q_OBJECT 

        public:

            QWatcherGUIConfig(QWidget *parent);
            virtual ~QWatcherGUIConfig(); 

            virtual bool loadConfiguration(); 
            virtual bool saveConfiguration(); 

        public slots:
            void toggleMonochrome(bool isOn); 
            void toggleThreeDView(bool isOn); 
            void toggleBackgroundImage(bool isOn); 
            void toggleLoopPlayback(bool inOn); 
            void showPlaybackTime(bool isOn); 
            void showPlaybackRange(bool isOn); 
            void showWallTime(bool isOn); 
            void toggleGlobalView(bool isOn); 
            void toggleBoundingBox(bool inOn);
            void toggleGroundGrid(bool isOn); 
            void spawnBackgroundColorDialog(); 
            void setGPSScale(); 
            void loadBackgroundImage(void); 

        signals:
            // Let anyone who's interested in the config get notified of init settings.
            void threeDViewToggled(bool threeDView);
            void monochromeToggled(bool monochromeMode);
            void backgroundImageToggled(bool backgroundImage);
            void globalViewToggled(bool showGlobalView); 
            void boundingBoxToggled(bool showBoundingBox); 
            void groundGridToggled(bool showGroundGrid);
            void loopPlaybackToggled(bool autorewind);
            void enableStreamFiltering(bool messageStreamFiltering);
            void checkPlaybackTime(bool showPlaybackTimeInStatusString);
            void checkPlaybackRange(bool showPlaybackRangeString);
            void checkWallTime(bool showWallTimeinStatusString);
            void enableBackgroundImage(bool backgroundImage);        // greys the GUI checkbox
            void gpsScaleUpdated(double prevGpsScale); 

        protected: 
            DECLARE_LOGGER(); 
    };
}

#endif /* WATHCER_CUTIE_GOOEY_CONFIG_H */
