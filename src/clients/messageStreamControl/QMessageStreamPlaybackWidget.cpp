#include <QtGui/QInputDialog>
#include <libwatcher/seekWatcherMessage.h>  // for epoch, eof
#include <libwatcher/playbackTimeRange.h>  // for epoch, eof
#include <libwatcher/listStreamsMessage.h>
#include <libwatcher/speedWatcherMessage.h>
#include <libwatcher/streamDescriptionMessage.h>
#include <boost/foreach.hpp>

#include "watcherStreamListDialog.h"
#include "QMessageStreamPlaybackWidget.h"
#include "logger.h"

namespace watcher {
    INIT_LOGGER(QMessageStreamPlaybackWidget, "QMessageStreamPlaybackWidget");

    QMessageStreamPlaybackWidget::QMessageStreamPlaybackWidget(QWidget *parent) : 
        QWidget(parent), 
        isPlaybackPaused(false), 
        streamRate(1.0),
        minTime(0), maxTime(0),
        timeRangeQueryTimerId(0),
        streamsDialog(NULL)
    {
        setupUi(this); 

        descriptionLabel->setText(tr("Not connected")); 
        playbackTimeLabel->setNum(-1); 
        maxTimeLabel->setText(tr("???")); 
        statusLabel->setText(tr("Connecting...")); 

        LOG_DEBUG("Creating streams list dialog..."); 
        streamsDialog = new ui::WatcherStreamListDialog;
        connect(streamsDialog, SIGNAL(streamChanged(unsigned long)), this, SLOT(selectStream(unsigned long)));
        connect(streamsDialog, SIGNAL(reconnect()), this, SLOT(reconnect()));
        connect(streamsDialog->refreshButton, SIGNAL(clicked()), this, SLOT(chooseMessageStream()));
        connect(streamsDialog->renameButton, SIGNAL(clicked()), this, SLOT(setStreamDescription())); 

        // setup slots/signals
        connect(rewindToStartButton, SIGNAL(clicked()), this, SLOT(rewindToStartOfPlayback())); 
        connect(rewindButton, SIGNAL(clicked()), this, SLOT(reversePlayback())); 
        connect(playButton, SIGNAL(clicked()), this, SLOT(normalPlayback())); 
        connect(pauseButton, SIGNAL(clicked()), this, SLOT(pausePlayback())); 
        connect(forwardButton, SIGNAL(clicked()), this, SLOT(forwardPlayback())); 
        connect(forwardToEndButton, SIGNAL(clicked()), this, SLOT(forwardToEndOfPlayback())); 
        connect(playbackSlider, SIGNAL(sliderReleased()), this, SLOT(setStreamPlaybackTimeFromSlider())); 
        connect(playbackSlider, SIGNAL(sliderMoved(int)), this, SLOT(playbackSliderChanged(int))); 
        connect(streamRateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(playbackSetSpeed(double))); 
        connect(streamButton, SIGNAL(clicked()), this, SLOT(chooseMessageStream()));  

        messageStreamConnected(false); 

    }
    // virtual 
    QMessageStreamPlaybackWidget::~QMessageStreamPlaybackWidget() {
        if (streamsDialog) {
            delete streamsDialog;
            streamsDialog=NULL;
        }
    }
    void QMessageStreamPlaybackWidget::timerEvent(QTimerEvent *te) {
        // We request a timerange update every so often to set the slider 
        // correctly. 
        if (te->timerId()==timeRangeQueryTimerId) 
            if (mStream)
                mStream->getMessageTimeRange();
    }
    void QMessageStreamPlaybackWidget::playbackTimeUpdated(watcher::Timestamp ts) {
        // LOG_DEBUG("Got new message timestamp: epoch: " << ts << ", offset: " << ((ts-minTime)/1000)); 
        if (ts>maxTime) 
            maxTime=ts;
        else if (ts<minTime)  // may happen at start before we get our first time range message
            minTime=ts;
        playbackTimeLabel->setNum(static_cast<int>((ts-minTime)/1000)); 
        playbackSlider->setValue(static_cast<int>((ts-minTime)/1000));
    }
    void QMessageStreamPlaybackWidget::playbackSliderChanged(int val) {
        statusLabel->setText(QString(tr("Seeking to %1...")).arg(val)); 
    }
    void QMessageStreamPlaybackWidget::messageStreamConnected(bool connected) {
        LOG_DEBUG("QMessageStreamPlaybackWidget " << (connected?"":"dis-") << "connected " << 
                (connected?"to":"from") << " message stream."); 
        if (connected) {
            timeRangeQueryTimerId=startTimer(1000);  // every second, request the new time range.
            if (mStream)  
                mStream->listStreams();
        }
        else {
            killTimer(timeRangeQueryTimerId);          
            timeRangeQueryTimerId=0;
            descriptionLabel->setText(tr("No Stream")); 
            statusLabel->setText(tr("Lost connection...")); 
            playbackTimeLabel->setNum(-1); 
            maxTimeLabel->setText("???"); 
            minTime=0;
            maxTime=0; 
            streamsDialog->hide(); 
        }
        streamButton->setEnabled(connected); 
        playbackSlider->setEnabled(connected); 
        rewindToStartButton->setEnabled(connected); 
        rewindButton->setEnabled(connected); 
        playButton->setEnabled(connected); 
        pauseButton->setEnabled(connected); 
        forwardButton->setEnabled(connected); 
        forwardToEndButton->setEnabled(connected); 
        streamRateSpinBox->setEnabled(connected); 
    }
    bool QMessageStreamPlaybackWidget::isPaused(void) const { 
        return isPlaybackPaused;
    }
    void QMessageStreamPlaybackWidget::setMessageStream(MessageStreamPtr ms) {
        mStream=ms;
    }
    void QMessageStreamPlaybackWidget::pausePlayback() {
        TRACE_ENTER();
        if (!mStream || !mStream->connected()) {
            TRACE_EXIT();
            return;
        }
        // bool pause=!isPlaybackPaused; 
        // if (pause) {
        isPlaybackPaused=true;
        mStream->stopStream(); 
        emit playbackPaused(); 
        statusLabel->setText(tr("Paused...")); 
        // }
        // else {
        //     isPlaybackPaused=false;
        //     mStream->startStream();
        //     emit playbackNormal(); 
        // }

        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::normalPlayback() {
        TRACE_ENTER();
        if (!mStream || !mStream->connected()) {
            TRACE_EXIT();
            return;
        }
        playbackSetSpeed(1.0);
        mStream->startStream(); 
        emit playbackNormal(); 
        statusLabel->setText(tr("Playing...")); 
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::reversePlayback() {
        TRACE_ENTER();
        if (!mStream || !mStream->connected()) {
            TRACE_EXIT();
            return;
        }
        if (streamRate != 0.0) {
            if (streamRate < 0.0)
                playbackSetSpeed(-abs(streamRate*2));
            else
                playbackSetSpeed(-abs(streamRate));
            mStream->startStream(); 
            statusLabel->setText(QString("Playing %1...").arg(streamRate>0.0?"in reverse":"forward")); 
        }
        emit playbackReversed(); 
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::forwardPlayback() {
        TRACE_ENTER();
        if (!mStream || !mStream->connected()) {
            TRACE_EXIT();
            return;
        }
        if (streamRate != 0.0) {
            if (streamRate > 0.0)
                playbackSetSpeed(abs(streamRate*2));
            else
                playbackSetSpeed(abs(streamRate));
            mStream->startStream(); 
            statusLabel->setText(QString("Playing %1...").arg(streamRate<0.0?"in reverse":"forward")); 
        }
        emit playbackForward();
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::rewindToStartOfPlayback() {
        TRACE_ENTER();
        if (!mStream || !mStream->connected()) {
            TRACE_EXIT();
            return;
        }
        mStream->setStreamTimeStart(SeekMessage::epoch); 
        if (streamRate < 0.0)
            normalPlayback();
        else
            mStream->startStream(); 
        emit rewoundToStartOfPlayback();
        statusLabel->setText(tr("Playing forward from start...")); 
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::forwardToEndOfPlayback() {
        TRACE_ENTER();
        if (!mStream || !mStream->connected()) {
            TRACE_EXIT();
            return;
        }
        mStream->setStreamTimeStart(SeekMessage::eof); 
        normalPlayback();
        emit forwardedToEndOfPlayback();
        statusLabel->setText(tr("Playing live stream...")); 
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::playbackSetSpeed(double x) {
        TRACE_ENTER();
        if (!mStream || !mStream->connected()) {
            TRACE_EXIT();
            return;
        }
        statusLabel->setText(QString(tr("Set playback rate to %1")).arg(x));
        mStream->setStreamRate(x);
        emit streamRateSet(x); 
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::setStreamPlaybackTimeFromSlider() {
        if (!mStream)
            return;
        int value=playbackSlider->value(); 
        playbackTimeLabel->setNum(value); 
        Timestamp newStart(minTime+(value*1000)); 
        LOG_DEBUG("message stream slider update - offset time: " << value << ", message stream time: " << newStart << " (cur range: [ " << minTime << " - " << maxTime << "])"); 
        mStream->clearMessageCache();
        mStream->setStreamTimeStart(newStart);
        mStream->startStream();
    }
    void QMessageStreamPlaybackWidget::setAutoRewind(bool) {
        LOG_WARN("Auto-rewind not yet implemented messageStreamControl"); 
    }
    void QMessageStreamPlaybackWidget::handleStreamRelatedMessage(event::MessagePtr m) {
        LOG_DEBUG("Got message of type: " << m->type); 
        if (m->type==SEEK_MESSAGE_TYPE) { 
            SeekMessagePtr seekMessage(boost::dynamic_pointer_cast<SeekMessage>(m)); 
            switch (seekMessage->rel) {
                case event::SeekMessage::start:
                    playbackSlider->setValue((minTime+seekMessage->offset)/1000.0); 
                    break;
                case event::SeekMessage::cur:
                    playbackSlider->setValue(playbackSlider->value()+((seekMessage->offset)/1000.0)); 
                    break;
                case event::SeekMessage::end:
                    playbackSlider->setValue((maxTime-seekMessage->offset)/1000.0); 
                    break;
            }
        }
        else if (m->type == START_MESSAGE_TYPE) {
            if (isPlaybackPaused) { 
                isPlaybackPaused = false;
                mStream->startStream(); 
                statusLabel->setText(tr("Playing...")); 
            }
        }
        else if (m->type == STOP_MESSAGE_TYPE) {
            if (!isPlaybackPaused) { 
                isPlaybackPaused = true;
                mStream->stopStream(); 
                statusLabel->setText(tr("Stopped")); 
            }
        }
        else if (m->type==PLAYBACK_TIME_RANGE_MESSAGE_TYPE) {
            PlaybackTimeRangeMessagePtr trm(boost::dynamic_pointer_cast<PlaybackTimeRangeMessage>(m));
            LOG_DEBUG("Got new range: epoch: [" << minTime << " - " << maxTime << "], offset from start: [0 - " << ((maxTime-minTime)/1000) << "] curtime: " << trm->cur_); 
            maxTime=trm->max_;
            minTime=trm->min_;
            if (trm->cur_!=event::SeekMessage::eof)
                playbackTimeUpdated(trm->cur_); 
            else 
                playbackTimeUpdated(maxTime); 
            int max=static_cast<int>((maxTime-minTime)/1000); 
            maxTimeLabel->setNum(max); 
            playbackSlider->setRange(0,max); 
        }
        else if (m->type == SPEED_MESSAGE_TYPE) {
            // notification from the watcher daemon that the shared stream speed has changed
            SpeedMessagePtr sm(boost::dynamic_pointer_cast<SpeedMessage>(m));
            if ((streamRate > 0.0 && sm->speed < 0.0) || (streamRate < 0.0 && sm->speed > 0.0)) 
                emit messageStreamDirectionChange(sm->speed<0.0); 
            streamRate=sm->speed;
            streamRateSpinBox->setValue(streamRate); 
            emit streamRateSet(streamRate); 
            if (streamRate == 0) {
                isPlaybackPaused = true;
                mStream->stopStream(); 
                emit playbackPaused(); 
            }
        } 
        else if (m->type == SUBSCRIBE_STREAM_MESSAGE_TYPE) { 
        }
        else if (m->type == STREAM_DESCRIPTION_MESSAGE_TYPE) {
            StreamDescriptionMessagePtr mess(boost::dynamic_pointer_cast<StreamDescriptionMessage>(m));
            descriptionLabel->setText(QString(mess->desc.c_str())); 
            statusLabel->setText(tr("Subscribed to new stream")); 
        }
        else if (m->type == LIST_STREAMS_MESSAGE_TYPE) { 
            ListStreamsMessagePtr mess(boost::dynamic_pointer_cast<ListStreamsMessage>(m));
            BOOST_FOREACH(event::EventStreamInfoPtr ev, mess->evstreams) {
                streamsDialog->addStream(ev->uid, ev->description);
            }
        }
        else  
            LOG_WARN("Got message type that I can't handle."); 
    }
    void QMessageStreamPlaybackWidget::chooseMessageStream() {
        TRACE_ENTER();
        if (!mStream || !mStream->connected()) {
            TRACE_EXIT();
            return;
        }
        streamsDialog->treeWidget->clear();
        LOG_DEBUG("Showing streams list dialog..."); 
        streamsDialog->show();

        mStream->listStreams();
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::selectStream(unsigned long uid) {
        TRACE_ENTER();
        if (!mStream || !mStream->connected()) {
            TRACE_EXIT();
            return;
        }
        mStream->subscribeToStream(uid);
        mStream->getMessageTimeRange(); // get info for new stream
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::setStreamDescription() {
        TRACE_ENTER();
        if (!mStream || !mStream->connected() || !streamsDialog) {
            TRACE_EXIT();
            return;
        }
        bool ok;
        QString text = QInputDialog::getText(streamsDialog, tr("Set Stream Description"), tr("Description:"), QLineEdit::Normal, QString(), &ok);
        if (ok && !text.isEmpty()) {
            mStream->setDescription(text.toStdString()); 
            chooseMessageStream(); // refresh listing with new name.
        }
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::reconnect() {
        TRACE_ENTER(); 
        
        mStream->clearMessageCache(); 
        mStream->reconnect(); 

        // We don't know the stream desc. We could punt to whomever 
        // has control of the stream pointer, but, meh - let's just force the 
        // user to choose a new name as this, presumably, won't be an action that 
        // happens often. 
        setStreamDescription(); 

        TRACE_EXIT(); 
    }
} // namespace
