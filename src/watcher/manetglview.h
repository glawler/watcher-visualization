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

        // timeout callback for watcher to do "idle" work.
        void watcherIdle();

        void toggleBandwidth(bool isOn);
        void toggleUndefined(bool isOn);
        void toggleNeighbors(bool isOn);
        void toggleHierarchy(bool isOn);
        void toggleAntennaRadius(bool isOn);
        void toggleSanityCheck(bool isOn);

        // Telcordia Zodiac specific layers. 
        void toggleBase(bool isOn);
        void toggleGroupA(bool isOn);
        void toggleGroupB(bool isOn);
        void toggleGroupD(bool isOn);
        void toggleGroupE(bool isOn);
        void toggleMission(bool isOn);
        void toggleGroupAChildren(bool isOn);
        void toggleGroupBChildren(bool isOn);
        void toggleGroupDChildren(bool isOn);
        void toggleGroupEChildren(bool isOn);
        void toggleMissionChildren(bool isOn);

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

        void toggleNodeSelectedForGraph(unsigned int nodeId);
        void showNodeSelectedForGraph(unsigned int nodeId, bool);

        void saveConfiguration();

    signals:
        void positionReset();

        void bandwidthToggled(bool isOn);
        void undefinedToggled(bool isOn);
        void neighborsToggled(bool isOn);
        void hierarchyToggled(bool isOn);
        void antennaRadiusToggled(bool isOn);
        void sanityCheckToggled(bool isOn);

        // Zodiac layer SIGNALS
        void baseToggled(bool isOn);
        void groupAToggled(bool isOn);
        void groupBToggled(bool isOn);
        void groupDToggled(bool isOn);
        void groupEToggled(bool isOn);
        void missionToggled(bool isOn);
        void groupAChildrenToggled(bool isOn);
        void groupBChildrenToggled(bool isOn);
        void groupDChildrenToggled(bool isOn);
        void groupEChildrenToggled(bool isOn);
        void missionChildrenToggled(bool isOn);

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

        void nodeDataInGraphsToggled(unsigned int nodeId); 
        void nodeDataInGraphsShowed(unsigned int, bool); 

        // Emitted when view->backgroun image should be enabled/disabled.
        void enableBackgroundImage(bool);

    protected:
        DECLARE_LOGGER();

        // Overload the close so we can save the configuration.
        // void closeEvent(QCloseEvent *event);

        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);

        void mouseDoubleClickEvent(QMouseEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void wheelEvent(QWheelEvent *event);

        void keyPressEvent(QKeyEvent * event);

    private:
        legacyWatcher::WatcherView currentView;
        QPoint lastPos;
};


#endif
