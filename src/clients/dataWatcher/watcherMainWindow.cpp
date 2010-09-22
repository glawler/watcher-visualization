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

#include <libwatcher/messageStream.h>
#include <libwatcher/dataPointMessage.h>
#include <libwatcher/playbackTimeRange.h>
#include <libwatcher/speedWatcherMessage.h>
#include <libwatcher/seekWatcherMessage.h>
#include <libwatcher/messageTypesAndVersions.h>

#include <logger.h>

#include "watcherMainWindow.h"
#include "watcherConfig.h"
#include "seriesGraphDialog.h"

namespace watcher {

namespace config {
    extern int StreamUid;
} // namespace

namespace ui {

INIT_LOGGER(MainWindow, "MainWindow");

Timestamp EpochTS; // the timestamp of the first message in the event stream
Timestamp CurrentTS;
MessageStreamPtr MsgStream;

void MainWindow::dataPointHandler(const QString& dataName, const QString& fromID, const QString& /*layer*/, qlonglong when, double value)
{
    TRACE_ENTER();

    SeriesGraphDialog *series;
    SeriesMap::iterator it = seriesMap.find(dataName);
    if (it == seriesMap.end()) {
	LOG_INFO("found new data series " << dataName.toStdString());
	/* not found, create a new menu item */
	series = new SeriesGraphDialog(dataName);
	seriesMap[dataName] = series;
	menuSeries->addAction(dataName, series, SLOT(show()));
	QObject::connect(this, SIGNAL(clockTick(qlonglong)), series, SLOT(handleClock(qlonglong)));
	QObject::connect(series, SIGNAL(seekStream(qlonglong)), this, SLOT(seekStream(qlonglong)));
    } else {
	series = it->second;
    }
    series->dataPoint(fromID, when, value);

    TRACE_EXIT();
}

void MainWindow::checkIO()
{
    TRACE_ENTER();

    MsgStream = MessageStreamPtr(new MessageStream(watcher::config::Server));
    MsgStream->connect(true);
    MsgStream->getMessageTimeRange();
    if (watcher::config::StreamUid == -1) {
	MsgStream->setDescription("data Watcher");
	MsgStream->startStream();
    } else {
	MsgStream->subscribeToStream(watcher::config::StreamUid);
    }

    MessagePtr msg;
    QString layer;
    while (MsgStream->getNextMessage(msg)) {
	if (msg->type == DATA_POINT_MESSAGE_TYPE) {
	    LOG_DEBUG("got DataPointMessage");
	    watcher::event::DataPointMessagePtr dp = boost::dynamic_pointer_cast<DataPointMessage>(msg);
	    // TODO:
	    // - add layer when DataPointMessage supports its
	    // - support data point messages for edges as well
	    emit dataPointReceived(QString::fromStdString(dp->dataName),
		    QString::fromStdString(dp->fromNodeID.to_string()),
		    layer, dp->timestamp, dp->dataPoints.front());
	} else if (msg->type == PLAYBACK_TIME_RANGE_MESSAGE_TYPE) {
	    LOG_DEBUG("got playback time range");
	    watcher::event::PlaybackTimeRangeMessagePtr m = boost::dynamic_pointer_cast<PlaybackTimeRangeMessage>(msg);
	    EpochTS = m->min_;
	    LOG_INFO("epoch TS = " << EpochTS);
	} else if (msg->type == SPEED_MESSAGE_TYPE) {
	    watcher::event::SpeedMessagePtr sm = boost::dynamic_pointer_cast<SpeedMessage>(msg);
	} else if (msg->type == SEEK_MESSAGE_TYPE) {
	    watcher::event::SeekMessagePtr sm = boost::dynamic_pointer_cast<SeekMessage>(msg);
	} else if (watcher::event::isFeederEvent(msg->type)) {
	    CurrentTS = msg->timestamp;
	    emit clockTick(msg->timestamp);
	}
    }

    TRACE_EXIT();
}

void MainWindow::setup()
{
    TRACE_ENTER();

    setupUi(this);
    QObject::connect(this,
	SIGNAL(dataPointReceived(const QString& , const QString& , const QString& , qlonglong , double )),
       	this,
	SLOT(dataPointHandler(const QString& , const QString& , const QString& , qlonglong , double )));

    LOG_INFO("spawning checkIO thread");
    checkIOThread = new boost::thread(&MainWindow::checkIO, this);

    TRACE_EXIT();
}

/** Slot for receiving seek requests
 * @param t the watcher::Timestamp value to seek to (milliseconds)
 */
void MainWindow::seekStream(qlonglong t)
{
    TRACE_ENTER();
    LOG_INFO("seeking to " << t);
    MsgStream->setStreamTimeStart(t);
    TRACE_EXIT();
}

} // ui
} // watcher

// vim:sw=4
