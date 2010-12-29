#ifndef QMessageStreamPlaybackWidget_h
#define QMessageStreamPlaybackWidget_h

#include <QWidget>
#include <QSlider>
#include <libwatcher/messageStreamReactor.h>
#include "declareLogger.h"
#include "watcherStreamListDialog.h"
#include "ui_QMessageStreamPlaybackWidget.h"

namespace watcher
{
    /** 
     * This class handles interaction between a user and a watcher message stream instance. 
     * The slots are generally triggered from a QWidget that the user interacts with.
     * The signals are generally the message stream getting updated somewhere else (like an 
     * external sync'd stream client) notifiing the GUI that the message stream has changed. 
     *
     * This class has a companion Qt UI file which it dirives from, which gives it direct access
     * to the GUI bits. To use this class open your UI in designer and add a generic widget. 
     * Create a 'promoted widget' and pass it this header file and make the class name match the 
     * name of this class. This promote the QWidget to an instance of this class. The widget will
     * appear empty in designer, but will contain this class (and the GUI widgets) at run time. To 
     * use the slots/signals editor in designer, open the QMessageStreamPlayback.ui filem find the 
     * slots container XML, and paste it directly into your .ui in the same location. The slots
     * and signals for the promoted widget, will then show up and you can set connections if 
     * needed.
     *
     * handleStreamRelatedMessage() should be inviked when a stream related message arrives on the 
     * stream. This class does not read the stream directly and relies on an external entity 
     * (possibly a messageStreamReactor instance) to give it messages. 
     *
     */
    class QMessageStreamPlaybackWidget : public QWidget, public Ui_messageStreamPlaybackForm {
        Q_OBJECT

        public:
            QMessageStreamPlaybackWidget(QWidget *parent=0);
            virtual ~QMessageStreamPlaybackWidget(); 

            /**
             * Handle incoming stream related messages. 
             */
            void handleStreamRelatedMessage(event::MessagePtr m); 

            bool isPaused() const; 
            
        public slots:
            /** 
             * These slots are handled internally and generally do not 
             * need to be called from exteral entities. 
             */
            void pausePlayback();
            void normalPlayback();
            void reversePlayback();
            void forwardPlayback();
            void rewindToStartOfPlayback();
            void forwardToEndOfPlayback();
            void playbackSetSpeed(double speed);
            void setStreamPlaybackTimeFromSlider(); 

            /** @param timestamp, current playback time in unix epoch time */
            void playbackTimeUpdated(watcher::Timestamp); 

            /** 
             * These slots should be called from external entities, to modify the behavior
             * and initialize the instance of the QMessageStreamPlaybackWidget.
             */
            /** Will jump to start of playback when a new message is not recieved for 10 seconds. */
            void setAutoRewind(bool); 

            void setMessageStream(MessageStreamPtr ms); 
            void messageStreamConnected(bool);   // true-->connected to server, false-->disconnected.

            void chooseMessageStream(); 

            /** connects to Message Stream List Dialog streamChanged() signal */
            void selectStream(unsigned long uid);

            /** Called when the stream's description should be changed. */
            void setStreamDescription(); 

            /** Called to create a new stream and reconnect */
            void reconnect(); 

        signals:
            void playbackPaused(); 
            void playbackNormal();
            void playbackReversed();
            void playbackForward();
            void rewoundToStartOfPlayback();
            void forwardedToEndOfPlayback();

            void streamRateSet(double); 
            void messageStreamDirectionChange(bool);    // true-->forward, false-->backward
            
        protected slots:
            void playbackSliderChanged(int val); 

        protected:
            DECLARE_LOGGER();
            void timerEvent(QTimerEvent *); 

        private:
            MessageStreamPtr mStream;
            bool isPlaybackPaused;
            float streamRate;
            watcher::Timestamp minTime, maxTime;  /* smallest and greatest timestamps seen - in unix epoch time */

            /** QTimer Ids for our timer callbacks. */
            int timeRangeQueryTimerId; 

            /** dialog for display list of streams */
            watcher::ui::WatcherStreamListDialog *streamsDialog;
    };
};

#endif /* QMessageStreamPlaybackWidget */
