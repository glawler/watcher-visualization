#ifndef QMessageStreamPlaybackWidget_h
#define QMessageStreamPlaybackWidget_h

#include <QWidget>
#include <QSlider>
#include "declareLogger.h"
#include "ui_QMessageStreamPlaybackWidget.h"
#include <messageStream.h>

namespace watcher
{
    /** Forward decl */
	class WatcherStreamListDialog;

    class QMessageStreamPlaybackWidget : public QWidget, public Ui_messageStreamPlaybackForm {
        Q_OBJECT

        public:
            QMessageStreamPlaybackWidget(QWidget *parent=0);
            virtual ~QMessageStreamPlaybackWidget(); 

            void setMessageStream(MessageStreamPtr ms); 
            MessageStreamPtr getMessageStream(); 

            /**
             * Handle incoming stream related messages. 
             */
            void handleStreamRelatedMessage(event::MessagePtr m); 
            
        public slots:
            void pausePlayback();
            void normalPlayback();
            void reversePlayback();
            void forwardPlayback();
            void rewindToStartOfPlayback();
            void forwardToEndOfPlayback();
            void playbackSetSpeed(double speed);
            void setStreamPlaybackTime(); 

        signals:
            void playbackPaused(); 
            void playbackNormal();
            void playbackReversed();
            void playbackForward();
            void rewoundToStartOfPlayback();
            void forwardedToEndOfPlayback();
            void streamRateSet(double); 
            void currentTimestampUpdated(watcher::Timestamp); 
            void messageStreamDirectionChange();

        protected:
            DECLARE_LOGGER();
            void timerEvent(QTimerEvent *); 

        private:
            watcher::MessageStreamPtr messageStream;
            bool isPlaybackPaused;
            float streamRate;
            watcher::Timestamp currentMessageTimestamp; 
            int timeRangeQueryTimerId; 
            /** dialog for display list of streams */
            // watcher::WatcherStreamListDialog *streamsDialog;
    };
};

#endif /* QMessageStreamPlaybackWidget */
