#include <seekWatcherMessage.h>  // for epoch, eof
#include <playbackTimeRange.h>  // for epoch, eof
#include <listStreamsMessage.h>
#include <speedWatcherMessage.h>
#include <streamDescriptionMessage.h>

// #include "watcherStreamListDialog.h"
#include "QMessageStreamPlaybackWidget.h"
#include "logger.h"

namespace watcher {
    INIT_LOGGER(QMessageStreamPlaybackWidget, "QMessageStreamPlaybackWidget");

    QMessageStreamPlaybackWidget::QMessageStreamPlaybackWidget(QWidget *parent) : 
        QWidget(parent), 
        server("localhost"),
        isPlaybackPaused(false), 
        autoRewind(false), 
        streamRate(1.0),
        reconnectTimeout(1.0), 
        reconnectTimerId(0), 
        timeRangeQueryTimerId(0),
        autoRewindTimerId(0) 
    {
        setupUi(this); 

        // setup slots/signals
        connect(rewindToStartButton, SIGNAL(clicked()), this, SLOT(rewindToStartOfPlayback())); 
        connect(rewindButton, SIGNAL(clicked()), this, SLOT(reversePlayback())); 
        connect(playButton, SIGNAL(clicked()), this, SLOT(forwardPlayback())); 
        connect(pauseButton, SIGNAL(clicked()), this, SLOT(pausePlayback())); 
        connect(forwardButton, SIGNAL(clicked()), this, SLOT(forwardPlayback())); 
        connect(forwardToEndButton, SIGNAL(clicked()), this, SLOT(forwardToEndOfPlayback())); 
        connect(playbackSlider, SIGNAL(sliderReleased()), this, SLOT(setStreamPlaybackTime())); 
        connect(streamRateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(playbackSetSpeed(double))); 

        timeRangeQueryTimerId=startTimer(10000);  // every ten seconds, request the new time range.
        autoRewindTimerId=startTimer(10000);      // every ten seconds, check for auto rewind
    }
    // virtual 
    QMessageStreamPlaybackWidget::~QMessageStreamPlaybackWidget() {
    }
    void QMessageStreamPlaybackWidget::timerEvent(QTimerEvent *te) {
        // We request a timerange update every so often to set the slider 
        // correctly. 
        if (te->timerId()==timeRangeQueryTimerId) 
            messageStream->getMessageTimeRange();
        if (te->timerId()==autoRewindTimerId) { 
            if (autoRewind) { 
                static time_t noNewMessagesForSeconds=0;
                if (currentMessageTimestamp==playbackSlider->maximum) { 
                    time_t now=time(NULL);
                    if (!noNewMessagesForSeconds)
                        noNewMessagesForSeconds=now;
                    if (now-noNewMessagesForSeconds>10) {
                        LOG_WARN("Autorewind engaged - jumping to start of data...");
                        LOG_DEBUG("playbackRangeEnd " << playbackSlider->maximum << " noNewMessagesForSeconds: " 
                                << noNewMessagesForSeconds); 
                        noNewMessagesForSeconds=0;
                        rewindToStartOfPlayback();
                    }
                }
                else 
                    noNewMessagesForSeconds=0;
            }
        }
    }
    void QMessageStreamPlaybackWidget::setServer(QString serverAddrOrName) {
        server=serverAddrOrName; 
    }
    bool QMessageStreamPlaybackWidget::isPaused(void) const { 
        return isPlaybackPaused;
    }
    void QMessageStreamPlaybackWidget::setMessageStream(MessageStreamPtr ms) {
        messageStream=ms;
    }
    MessageStreamPtr QMessageStreamPlaybackWidget::getMessageStream() { 
        return messageStream;
    }
    void QMessageStreamPlaybackWidget::pausePlayback() {
        TRACE_ENTER();
        if (!messageStream || !messageStream->connected()) {
            TRACE_EXIT();
            return;
        }
        if (!isPlaybackPaused)
            messageStream->stopStream(); 
        else 
            messageStream->startStream();
        emit playbackPaused(); 
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::normalPlayback() {
        TRACE_ENTER();
        if (!messageStream || !messageStream->connected()) {
            TRACE_EXIT();
            return;
        }
        playbackSetSpeed(1.0);
        messageStream->startStream(); 
        emit playbackNormal(); 
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::reversePlayback() {
        TRACE_ENTER();
        if (!messageStream || !messageStream->connected()) {
            TRACE_EXIT();
            return;
        }
        pausePlayback(); 
        if (streamRate != 0.0) {
            if (streamRate < 0.0)
                playbackSetSpeed(-abs(streamRate*2));
            else
                playbackSetSpeed(-abs(streamRate));
            messageStream->startStream(); 
        }
        emit playbackReversed(); 
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::forwardPlayback() {
        TRACE_ENTER();
        if (!messageStream || !messageStream->connected()) {
            TRACE_EXIT();
            return;
        }
        pausePlayback(); 
        if (streamRate != 0.0) {
            if (streamRate > 0.0)
                playbackSetSpeed(abs(streamRate*2));
            else
                playbackSetSpeed(abs(streamRate));
            messageStream->startStream(); 
        }
        emit playbackForward();
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::rewindToStartOfPlayback() {
        TRACE_ENTER();
        if (!messageStream || !messageStream->connected()) {
            TRACE_EXIT();
            return;
        }
        pausePlayback(); 
        messageStream->setStreamTimeStart(SeekMessage::epoch); 
        if (streamRate < 0.0)
            normalPlayback();
        else
            messageStream->startStream(); 
        emit rewoundToStartOfPlayback();
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::forwardToEndOfPlayback() {
        TRACE_ENTER();
        if (!messageStream || !messageStream->connected()) {
            TRACE_EXIT();
            return;
        }
        pausePlayback();
        messageStream->setStreamTimeStart(SeekMessage::eof); 
        normalPlayback();
        emit forwardedToEndOfPlayback();
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::playbackSetSpeed(double x) {
        TRACE_ENTER();
        if (!messageStream || !messageStream->connected()) {
            TRACE_EXIT();
            return;
        }
        messageStream->setStreamRate(x);
        emit streamRateSet(x); 
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::setStreamPlaybackTime() {
        int value=playbackSlider->value(); 
        playbackTimeLabel->setText(QString::number(value)); 
        Timestamp newStart(value); 
        LOG_DEBUG("slider update - new start time: " << newStart << " slider position: " << value << " cur mess ts: " << currentMessageTimestamp);
        currentMessageTimestamp=newStart;  // So it displays in status string immediately.
        newStart*=1000;
        messageStream->clearMessageCache();
        messageStream->setStreamTimeStart(newStart);
        messageStream->startStream();
        emit currentTimestampUpdated(currentMessageTimestamp); 
    }
    void QMessageStreamPlaybackWidget::setAutoRewind(bool val) {
        autoRewind=val;
    }
    void QMessageStreamPlaybackWidget::handleStreamRelatedMessage(event::MessagePtr m) {
        if (m->type==PLAYBACK_TIME_RANGE_MESSAGE_TYPE) {
            PlaybackTimeRangeMessagePtr trm(boost::dynamic_pointer_cast<PlaybackTimeRangeMessage>(m));
            playbackSlider->setRange(trm->max_/1000, trm->min_/1000);
            emit currentTimestampRangeUpdated(trm->max_/1000, trm->min_/1000); 
        }
        else if (m->type == SPEED_MESSAGE_TYPE) {
            // notification from the watcher daemon that the shared stream speed has changed
            SpeedMessagePtr sm(boost::dynamic_pointer_cast<SpeedMessage>(m));
            if ((streamRate > 0.0 && sm->speed < 0.0) || (streamRate < 0.0 && sm->speed > 0.0)) 
                emit messageStreamDirectionChange(sm->speed<0.0); 
            streamRate=sm->speed;
            streamRateSpinBox->setValue(streamRate); 
            emit streamRateSet(streamRate); 
            if (streamRate == 0)
                isPlaybackPaused = true;
        } 
        else if (m->type == STOP_MESSAGE_TYPE) 
            isPlaybackPaused = true;
        else if (m->type == START_MESSAGE_TYPE) 
            isPlaybackPaused = false;
        // else if (m->type == STREAM_DESCRIPTION_MESSAGE_TYPE) {
        //     StreamDescriptionMessagePtr m(dynamic_pointer_cast<StreamDescriptionMessage>(m));
        //     streamDescription = m->desc;
        // }
    }
} // namespace
