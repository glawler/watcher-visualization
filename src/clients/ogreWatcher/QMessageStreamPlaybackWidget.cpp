#include <libwatcher/seekWatcherMessage.h>  // for epoch, eof
#include <libwatcher/playbackTimeRange.h>  // for epoch, eof
#include <libwatcher/listStreamsMessage.h>
#include <libwatcher/speedWatcherMessage.h>
#include <libwatcher/streamDescriptionMessage.h>

// #include "watcherStreamListDialog.h"
#include "QMessageStreamPlaybackWidget.h"
#include "logger.h"

namespace watcher {
    INIT_LOGGER(QMessageStreamPlaybackWidget, "QMessageStreamPlaybackWidget");

    QMessageStreamPlaybackWidget::QMessageStreamPlaybackWidget(QWidget *parent) : 
        QWidget(parent), 
        isPlaybackPaused(false), 
        streamRate(1.0),
        minTime(0), maxTime(0),
        timeRangeQueryTimerId(0)
    {
        setupUi(this); 

        descriptionLabel->setText("Not connected"); 
        playbackTimeLabel->setNum(-1); 
        playbackRangeLabel->setText("Unknown Range"); 

        // setup slots/signals
        connect(rewindToStartButton, SIGNAL(clicked()), this, SLOT(rewindToStartOfPlayback())); 
        connect(rewindButton, SIGNAL(clicked()), this, SLOT(reversePlayback())); 
        connect(playButton, SIGNAL(clicked()), this, SLOT(normalPlayback())); 
        connect(pauseButton, SIGNAL(clicked()), this, SLOT(pausePlayback())); 
        connect(forwardButton, SIGNAL(clicked()), this, SLOT(forwardPlayback())); 
        connect(forwardToEndButton, SIGNAL(clicked()), this, SLOT(forwardToEndOfPlayback())); 
        connect(playbackSlider, SIGNAL(sliderReleased()), this, SLOT(setStreamPlaybackTimeFromSlider())); 
        connect(streamRateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(playbackSetSpeed(double))); 
    }
    // virtual 
    QMessageStreamPlaybackWidget::~QMessageStreamPlaybackWidget() {
    }
    void QMessageStreamPlaybackWidget::timerEvent(QTimerEvent *te) {
        // We request a timerange update every so often to set the slider 
        // correctly. 
        if (te->timerId()==timeRangeQueryTimerId) 
            if (mStream)
                mStream->getMessageTimeRange();
    }
    void QMessageStreamPlaybackWidget::playbackTimeUpdated(watcher::Timestamp ts) {
        LOG_DEBUG("Got new message timestamp: epoch: " << ts << ", offset: " << ((ts-minTime)/1000)); 
        if (ts>maxTime) {
            maxTime=ts;
            playbackSlider->setRange(0, (maxTime-minTime)/1000); 
            playbackRangeLabel->setText(QString("0 - %1").arg((maxTime-minTime)/1000)); 
        }
        else if (ts<minTime) {   // may happen at start before we get our first time range message
            minTime=ts;
            playbackSlider->setRange(0, (maxTime-minTime)/1000); 
            playbackRangeLabel->setText(QString("0 - %1").arg((maxTime-minTime)/1000)); 
        }
        playbackTimeLabel->setNum(static_cast<int>((ts-minTime)/1000)); 
        playbackSlider->setValue(static_cast<int>((ts-minTime)/1000));
    }
    void QMessageStreamPlaybackWidget::messageStreamConnected(bool connected) {
        LOG_DEBUG("QMessageStreamPlaybackWidget " << (connected?"":"dis-") << "connected " << 
                (connected?"to":"from") << " message stream."); 
        if (connected) {
            timeRangeQueryTimerId=startTimer(10000);  // every five seconds, request the new time range.
            playbackSlider->setEnabled(true); 
        }
        else {
            killTimer(timeRangeQueryTimerId);          
            timeRangeQueryTimerId=0;
            descriptionLabel->setText("Not connected"); 
            playbackTimeLabel->setNum(-1); 
            playbackRangeLabel->setText("Unknown Range"); 
            minTime=0;
            maxTime=0; 
            playbackSlider->setEnabled(false); 
        }
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
        TRACE_EXIT();
    }
    void QMessageStreamPlaybackWidget::playbackSetSpeed(double x) {
        TRACE_ENTER();
        if (!mStream || !mStream->connected()) {
            TRACE_EXIT();
            return;
        }
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
        LOG_WARN("Auto-rewind not yet implemented in ogreWatcher"); 
    }
    void QMessageStreamPlaybackWidget::handleStreamRelatedMessage(event::MessagePtr m) {
        // LOG_DEBUG("Got message of type: " << m->type); 
        if (m->type==PLAYBACK_TIME_RANGE_MESSAGE_TYPE) {
            PlaybackTimeRangeMessagePtr trm(boost::dynamic_pointer_cast<PlaybackTimeRangeMessage>(m));
            maxTime=trm->max_;
            minTime=trm->min_; 
            playbackSlider->setRange(0, (maxTime-minTime)/1000); 
            playbackRangeLabel->setText(QString("0 - %1").arg((maxTime-minTime)/1000)); 
            LOG_DEBUG("Got new range: epoch: [" << minTime << " - " << maxTime << "], offset from start: [0 - " << (maxTime-minTime)/1000);  
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
        else if (m->type == STOP_MESSAGE_TYPE) {
            isPlaybackPaused = true;
            mStream->stopStream(); 
            emit playbackPaused(); 
        }
        else if (m->type == START_MESSAGE_TYPE) {
            isPlaybackPaused = false;
            mStream->startStream(); 
            emit playbackPaused(); 
        }
        else if (m->type == STREAM_DESCRIPTION_MESSAGE_TYPE) {
            StreamDescriptionMessagePtr mess(boost::dynamic_pointer_cast<StreamDescriptionMessage>(m));
            descriptionLabel->setText(QString(mess->desc.c_str())); 
        }
    }
} // namespace
