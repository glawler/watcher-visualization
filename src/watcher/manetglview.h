#ifndef MANETGLVIEW_H
#define MANETGLVIEW_H

#include <QGLWidget>
#include "legacyWatcher/legacyWatcher.h"
#include "logger.h"

class manetGLView : public QGLWidget
{
    Q_OBJECT

    public:

        manetGLView(QWidget *parent = 0);
        ~manetGLView();

        void runLegacyWatcherMain(int argc, char **argv);
        void setLegacyWatcherView(const legacyWatcher::WatcherView &view);

        QSize minimumSizeHint() const;
        QSize sizeHint() const;


    public slots:
        void resetPosition();
        void fitToWindow();
        void checkIO();
        void manetView();
        void hierarchyView();

        void toggleBandwidth(bool isOn);
        void toggleUndefined(bool isOn);
        void toggleNeighbors(bool isOn);
        void toggleHierarchy(bool isOn);
        void toggleRouting(bool isOn);
        void toggleRoutingOnehop(bool isOn);
        void toggleAntennaRadius(bool isOn);
        void toggleSanityCheck(bool isOn);
        void toggleAnomPaths(bool isOn);
        void toggleCorrelation(bool isOn);
        void toggleAlert(bool isOn);
        void toggleCorrelation3Hop(bool isOn);
        void toggleWormholeRouting(bool isOn);
        void toggleWormholeRoutingOnehop(bool isOn);
        void toggleFloatingGraph(bool isOn);
        void toggleNormPaths(bool isOn);
        void toggleMonochrome(bool isOn);
        void toggleThreeDView(bool isOn);
        void toggleBackgroundImage(bool isOn);

        void clearAllLabels();
        void clearAllEdges();

        void goodwinStart();
        void goodwinStop();
        void goodwinPause();
        void goodwinContinue();
        void goodwinSetSpeed(int speed);

    signals:
        void positionReset();

        void bandwidthToggled(bool isOn);
        void undefinedToggled(bool isOn);
        void neighborsToggled(bool isOn);
        void hierarchyToggled(bool isOn);
        void routingToggled(bool isOn);
        void routingOnehopToggled(bool isOn);
        void antennaRadiusToggled(bool isOn);
        void sanityCheckToggled(bool isOn);
        void anomPathsToggled(bool isOn);
        void correlationToggled(bool isOn);
        void alertToggled(bool isOn);
        void correlation3HopToggled(bool isOn);
        void wormholeRoutingToggled(bool isOn);
        void wormholeRoutingOnehopToggled(bool isOn);
        void floatingGraphToggled(bool isOn);
        void normPathsToggled(bool isOn);
        void monochromeToggled(bool isOn);
        void threeDViewToggled(bool isOn); 
        void backgroundImageToggled(bool isOn); 

        void labelsCleared();
        void edgesCleared(); 

        void startedGoodwin();
        void stoppedGoodwin();
        void pausedGoodwin(); 
        void continuedGoodwin();

        void runningGoodwin(bool usingGoodwin);

    protected:
        DECLARE_LOGGER();

        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);

        void mouseDoubleClickEvent(QMouseEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);

        void keyPressEvent(QKeyEvent * event);

    private:
        legacyWatcher::WatcherView currentView;
        QPoint lastPos;
};


#endif
