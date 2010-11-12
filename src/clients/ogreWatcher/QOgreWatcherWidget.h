#ifndef QOGRE_WATCHER_WIDGET_H
#define QOGRE_WATCHER_WIDGET_H

#include "QOgreWidget.h"
#include "declareLogger.h"
#include <messageStream.h>

namespace watcher 
{
    using namespace QtOgre; 

    class QOgreWatcherWidget : public QOgreWidget {
        Q_OBJECT

        public:
            QOgreWatcherWidget(QWidget *parent = 0);
            virtual ~QOgreWatcherWidget();

        public slots:
            void resetPosition();
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

        // signals:
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

        private:
            watcher::MessageStreamPtr messageStream;
    };
} // namespace
#endif
