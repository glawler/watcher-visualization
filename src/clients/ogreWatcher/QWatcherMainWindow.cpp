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
 * @file watcherMainWindow.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#include "watcherMainWindow.h"
#include "logger.h"

using namespace watcher;
using namespace std;

INIT_LOGGER(QWatcherMainWindow, "QWatcherMainWindow"); 

QWatcherMainWindow::QWatcherMainWindow(QWidget *parent, Qt::WindowFlags flags) : 
    QMainWindow(parent, flags)
{
    TRACE_ENTER();
    
    setupUi(this); 

    menuLayers->setTearOffEnabled(); 
    menuView->setTearOffEnabled(true);

    if(!qWatcherConfig->loadConfiguration()) {
        LOG_FATAL("Error in cfg file, unable to continue"); 
        exit(EXIT_FAILURE); 
    }

    // set up intra-widget slots and signals that are not setup via designer

    TRACE_EXIT();
}

QWatcherMainWindow::~QWatcherMainWindow()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void QWatcherMainWindow::loadUi(Ui_WatcherMainWindow &ui)
{
    ui.setupUi(this); 
}

void QWatcherMainWindow::initMessageStream(QString serverName)
{
    messageStream=MessageStreamPtr(new MessageStream(conf->serverName)); 
    messageStreamReactor=new MessageStreamReactor(messageStream); 

    // GUI window wants GPS updates, new layers, new nodes, and all data messages on all layers.
    messageStreamReactor->addNodeLocationUpdateFunction(boost::bind(&ui.manetGLView::gps2openGLPixels, this, _1, _2, _3, _4)); 
    messageStreamReactor->addNewNodeSeenCallback(boost::bind(&manetGLView::newNodeSeen, , _1)); 
    messageStreamReactor->addNewLayerSeenCallback(boost::bind(&manetGLView::newNodeSeen, , _1)); 
    messageStreamReactor->addNewNodeSeenCallback(boost::bind(&manetGLView::newNodeSeen, , _1)); 

    // playback widget messages. 
    messageStreamReactor->addMessageTypeCallback(
            boost::bind(&QMessageStreamPlaybackWidget::handleStreamRelatedMessage, messageStreamPlaybackWidget, _1), 
            PLAYBACK_TIME_RANGE_MESSAGE_TYPE); 
    messageStreamReactor->addMessageTypeCallback(
            boost::bind(&QMessageStreamPlaybackWidget::handleStreamRelatedMessage, messageStreamPlaybackWidget, _1), 
            SPEED_MESSAGE_TYPE); 
    messageStreamReactor->addMessageTypeCallback(
            boost::bind(&QMessageStreamPlaybackWidget::handleStreamRelatedMessage, messageStreamPlaybackWidget, _1), 
            START_MESSAGE_TYPE); 
    messageStreamReactor->addMessageTypeCallback(
            boost::bind(&QMessageStreamPlaybackWidget::handleStreamRelatedMessage, messageStreamPlaybackWidget, _1), 
            STREAM_DESCRIPTION_MESSAGE_TYPE); 
}
