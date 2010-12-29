/* Copyright 2009,2010 SPARTA, Inc., dba Cobham Analytic Solutions
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
 * @date 2010-12-01
 */
#include "QWatcherMainWindow.h"
#include "ui_messageStreamControl.h"
#include "logger.h"

using namespace watcher;
using namespace std;

INIT_LOGGER(QWatcherMainWindow, "QWatcherMainWindow"); 

const watcher::MessageType QWatcherMainWindow::messageTypes[] = {
    SEEK_MESSAGE_TYPE, 
    START_MESSAGE_TYPE, 
    STOP_MESSAGE_TYPE,
    SPEED_MESSAGE_TYPE,
    PLAYBACK_TIME_RANGE_MESSAGE_TYPE,
    SUBSCRIBE_STREAM_MESSAGE_TYPE,
    STREAM_DESCRIPTION_MESSAGE_TYPE,
    LIST_STREAMS_MESSAGE_TYPE
};

QWatcherMainWindow::QWatcherMainWindow(Ui_WatcherMainWindow &ui_, const string &server_, QWidget *parent, Qt::WindowFlags flags) : 
    QMainWindow(parent, flags), 
    ui(ui_), 
    messageStreamReactor(NULL),
    server(server_),
    messageStreamConnectionThread(NULL)
{
    TRACE_ENTER();
    ui.setupUi(this); 
    connect(this, SIGNAL(messageStreamConnected(bool)), ui.messageStreamPlaybackWidget, SLOT(messageStreamConnected(bool))); 
    messageStreamConnectionThread=new boost::thread(boost::bind(&QWatcherMainWindow::connectStream, this));
    TRACE_EXIT();
}

QWatcherMainWindow::~QWatcherMainWindow()
{
    TRACE_ENTER();
    if (messageStreamReactor) {
        delete messageStreamReactor; 
        messageStreamReactor=NULL;
    }

    if (messageStreamConnectionThread) {
        messageStreamConnectionThread->interrupt();
        messageStreamConnectionThread->join();
        delete messageStreamConnectionThread;
        messageStreamConnectionThread=NULL;
    }
    TRACE_EXIT();
}

void QWatcherMainWindow::initMessageStream()
{
    messageStream->stopStream(); 

    // We only want the control messages.
    messageStream->enableFiltering(true); 
    MessageStreamFilterPtr filter=MessageStreamFilterPtr(new MessageStreamFilter); 
    for (unsigned int i=0; i<sizeof(messageTypes)/sizeof(messageTypes[0]); i++) 
        filter->addMessageType(messageTypes[i]); 
    messageStream->addMessageFilter(filter); 

    // tell the playback widget about the stream
    ui.messageStreamPlaybackWidget->setMessageStream(messageStream); 

    // setup and initalize the message stream callbacks.
    if (messageStreamReactor) {
        delete messageStreamReactor; 
        messageStreamReactor=NULL;
    }
    messageStreamReactor=new MessageStreamReactor(messageStream); 

    for (unsigned int i=0; i<sizeof(messageTypes)/sizeof(messageTypes[0]); i++) 
        messageStreamReactor->addMessageTypeCallback(
                boost::bind(&QMessageStreamPlaybackWidget::handleStreamRelatedMessage, ui.messageStreamPlaybackWidget, _1), 
                messageTypes[i]); 

    messageStream->setDescription("Message Stream Controller"); 
}

void QWatcherMainWindow::connectStream()
{
    TRACE_ENTER();
    while (true) {
        boost::this_thread::interruption_point();
        if (!messageStream) {  // shouldn't happen
            messageStream=MessageStreamPtr(new MessageStream(server)); 
        }
        while (!messageStream->connected()) {
            boost::this_thread::interruption_point();
            if (!messageStream->connect(true)) {
                LOG_WARN("Unable to connect to server at " << server << ". Trying again in 1 second.");
                sleep(1);
            }
        }

        initMessageStream(); 
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

