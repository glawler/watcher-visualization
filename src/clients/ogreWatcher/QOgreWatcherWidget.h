#ifndef QOGRE_WATCHER_WIDGET_H
#define QOGRE_WATCHER_WIDGET_H

#include <libwatcher/message_fwd.h>
#include <libwatcher/watcherTypes.h>
#include "QOgreWidget.h"
#include "declareLogger.h"

namespace watcher 
{
    class QOgreWatcherWidget : public QtOgre::QOgreWidget {
        Q_OBJECT

        public:
            QOgreWatcherWidget(QWidget *parent = 0);
            virtual ~QOgreWatcherWidget();

            // must match signature of MessageStreamReactor::NodeLocationUpdateFunction
            /** Call this with the GPS updates of each node */
            bool nodeLocationUpdate(double x, double y, double z, const std::string &nodeID); 

            // must match signature of MessageStreamReactor::MessageCallbackFunction
            /** Call this with the first message that contains a node we've never seen */
            void newNodeSeen(watcher::event::MessagePtr m); 

            /** Call this with the first message that contains a layer we've never seen */
            void newLayerSeen(watcher::event::MessagePtr m); 

            /** Call with new data messages. */
            void handleFeederMessage(watcher::event::MessagePtr m);

            /** Call with new edge messages. */
            void handleEdgeMessage(watcher::event::MessagePtr m); 

        public slots:
            void messageStreamConnected(bool); 

            void resetPosition();
            void toggleGround(bool show); 
            void toggleSky(bool show); 
            void toggleFullscreen(bool fullscreen); 
        //     void fitToWindow();
        //     void checkIO();
        //     // timeout callback for watcher to do "idle" work.
        //     void watcherIdle();
        //     // void layerToggled(QString, bool);
        //     void layerToggle(const QString &layer, const bool turnOn);
        //     void newLayerConnect(const QString name); 
        //     void clearAllLabels();
        //     void clearAllEdges();
        //     void clearAll();
        //     void configureLayers(); 
        //     void showKeyboardShortcuts(); 
        //     void setEdgeWidth();
        //     void listStreams();
        //     void selectStream(unsigned long uid);
        //     void spawnStreamDescription();
        //     void reconnect();
        //     void toggleNodeSelectedForGraph(unsigned int nodeId);
        //     void showNodeSelectedForGraph(unsigned int nodeId, bool);
        //     void scrollingGraphActivated(QString graphName);
        //     void spawnAboutBox(); 
        //     void spawnNodeConfigurationDialog();
        //     void streamFilteringEnabled(bool isOn); 
        //     void shutdown(); 
        //     void gpsScaleUpdated(double prevGpsScale); 
        //     void boundingBoxToggled(bool isOn); 

        signals:
            /** 
             * Only emitted when the timestamp has changed. Only 
             * emitted once a second. 
             */
            void currentPlaybackTime(watcher::Timestamp); 
        //     void positionReset();
        //     void layerToggled(const QString &, bool);
        //     void connectNewLayer(const QString); 
        //     void spawnLayerConfigureDialog(); 
        //     void labelsCleared();
        //     void edgesCleared(); 
        //     void changeBackgroundColor();
        //     void nodeDataInGraphsToggled(unsigned int nodeId); 
        //     void nodeDataInGraphsShowed(unsigned int, bool); 
        //     void nodeClicked(size_t nodeId);
        //     // Emitted when view->backgroun image should be enabled/disabled.
        //     void enableBackgroundImage(bool);

        protected:
            DECLARE_LOGGER();

            /** We use timers in this class, so handle the QTimer event callback. */
            void timerEvent(QTimerEvent *te); 

            virtual void createScene(); 

        private:
            /** Last message timestamp we've seen. */
            watcher::Timestamp lastKnownMessageTime; 
            /** The timer id for emitted last know message timestamp */
            int currentPlaybackTimerId; 
        
            Ogre::SceneNode *m_physicalLayerSceneNode; 
    };
} // namespace
#endif
