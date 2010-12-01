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
#include "QWatcherMainWindow.h"
#include "ui_ogreWatcher.h"      // the UI class/file for this window: Ui_WatcherMainWindow
#include "singletonConfig.h"
#include "logger.h"

using namespace watcher;
using namespace std;

INIT_LOGGER(QWatcherMainWindow, "QWatcherMainWindow"); 

QWatcherMainWindow::QWatcherMainWindow(Ui_WatcherMainWindow &ui_, QWidget *parent, Qt::WindowFlags flags) : 
    QMainWindow(parent, flags), 
    ui(ui_), 
    messageStreamReactor(NULL),
    messageStreamConnectionThread(NULL)
{
    TRACE_ENTER();
    
    ui.setupUi(this); 

    ui.menuLayers->setTearOffEnabled(true); 
    ui.menuView->setTearOffEnabled(true);

    if(!ui.qWatcherConfig->loadConfiguration()) {
        LOG_FATAL("Error in cfg file, unable to continue"); 
        exit(EXIT_FAILURE); 
    }

    // set up intra-widget slots and signals that are not setup via designer
    
    // (for some reason, designer-qt does not recognize the promoted widget inside 
    // a dock widget otherwise I'd do this inside designer-qt.)
    QObject::connect(this, SIGNAL(messageStreamConnected(bool)), 
            ui.messageStreamPlaybackWidget, SLOT(messageStreamConnected(bool))); 
    QObject::connect(this, SIGNAL(messageStreamConnected(bool)), 
            ui.ogreWidget, SLOT(messageStreamConnected(bool))); 
    QObject::connect(ui.ogreWidget, SIGNAL(currentPlaybackTime(watcher::Timestamp)), 
            ui.messageStreamPlaybackWidget, SLOT(playbackTimeUpdated(watcher::Timestamp))); 
    QObject::connect(ui.ogreWidget, SIGNAL(ogreInitialized()), this, SLOT(ogreInitialized())); 

    // QObject::connect(ui.messageStreamPlaybackWidget, SIGNAL(messageStreamDirectionChange(bool)), 
    //         ui.ogreWidget, 

    TRACE_EXIT();
}

QWatcherMainWindow::~QWatcherMainWindow()
{
    TRACE_ENTER();
    if (messageStreamReactor) {
        delete messageStreamReactor; 
        messageStreamReactor=NULL;
    }

    ui.qWatcherConfig->saveConfiguration(); 
    watcher::SingletonConfig::saveConfig(); 
    
    if (messageStreamConnectionThread) {
        messageStreamConnectionThread->interrupt();
        messageStreamConnectionThread->join();
        delete messageStreamConnectionThread;
        messageStreamConnectionThread=NULL;
    }
    TRACE_EXIT();
}

void QWatcherMainWindow::ogreInitialized() 
{
    // now that OGRE is up and running, we can start the message stream.
    messageStreamConnectionThread=new boost::thread(boost::bind(&QWatcherMainWindow::connectStream, this));
}
    
void QWatcherMainWindow::initMessageStream()
{
    messageStream=MessageStreamPtr(new MessageStream(ui.qWatcherConfig->serverName)); 

    ui.messageStreamPlaybackWidget->setMessageStream(messageStream); 

    if (messageStreamReactor) {
        delete messageStreamReactor; 
        messageStreamReactor=NULL;
    }
    messageStreamReactor=new MessageStreamReactor(messageStream); 

    // GUI window wants GPS updates, new layers, new nodes, and all data messages on all layers.
    messageStreamReactor->addNodeLocationUpdateFunction(
            boost::bind(&watcher::QOgreWatcherWidget::nodeLocationUpdate, ui.ogreWidget, _1, _2, _3, _4)); 
    messageStreamReactor->addNewNodeSeenCallback(
            boost::bind(&watcher::QOgreWatcherWidget::newNodeSeen, ui.ogreWidget, _1)); 
    messageStreamReactor->addNewLayerSeenCallback(
            boost::bind(&watcher::QOgreWatcherWidget::newLayerSeen, ui.ogreWidget, _1));
    messageStreamReactor->addFeederMessageCallback(
            boost::bind(&watcher::QOgreWatcherWidget::handleFeederMessage, ui.ogreWidget, _1));
    messageStreamReactor->addMessageTypeCallback(
            boost::bind(&watcher::QOgreWatcherWidget::handleEdgeMessage, 
                ui.ogreWidget, _1), 
            EDGE_MESSAGE_TYPE); 

    // playback widget messages. 
    messageStreamReactor->addMessageTypeCallback(
            boost::bind(&QMessageStreamPlaybackWidget::handleStreamRelatedMessage, 
                ui.messageStreamPlaybackWidget, _1), 
            PLAYBACK_TIME_RANGE_MESSAGE_TYPE); 
    messageStreamReactor->addMessageTypeCallback(
            boost::bind(&QMessageStreamPlaybackWidget::handleStreamRelatedMessage, 
                ui.messageStreamPlaybackWidget, _1), 
            SPEED_MESSAGE_TYPE); 
    messageStreamReactor->addMessageTypeCallback(
            boost::bind(&QMessageStreamPlaybackWidget::handleStreamRelatedMessage, 
                ui.messageStreamPlaybackWidget, _1), 
            START_MESSAGE_TYPE); 
    messageStreamReactor->addMessageTypeCallback(
            boost::bind(&QMessageStreamPlaybackWidget::handleStreamRelatedMessage, 
                ui.messageStreamPlaybackWidget, _1), 
            STREAM_DESCRIPTION_MESSAGE_TYPE); 
}

void QWatcherMainWindow::connectStream()
{
    TRACE_ENTER();
    initMessageStream(); 
    while (true) {

        boost::this_thread::interruption_point();
        if (!messageStream)
            messageStream=MessageStreamPtr(new MessageStream(ui.qWatcherConfig->serverName));

        while (!messageStream->connected()) {
            boost::this_thread::interruption_point();
            if (!messageStream->connect(true)) {
                LOG_WARN("Unable to connect to server at " << ui.qWatcherConfig->serverName 
                        << ". Trying again in 1 second.");
                sleep(1);
            }
        }

        messageStream->setDescription("ogreWatcher GUI instance"); 
        messageStream->startStream(); 
        messageStream->getMessageTimeRange(); 
        messageStream->enableFiltering(ui.qWatcherConfig->messageStreamFiltering); 

        emit messageStreamConnected(true); 

        /* check every second that the connection is still alive */
        do {
            sleep(1);
            boost::this_thread::interruption_point();
            if (!messageStream || !messageStream->connected()) {
                LOG_INFO("connection to server lost, reconnecting");
                emit messageStreamConnected(false);
                break;
            }
        } while (true);
    }
    TRACE_EXIT();
}

