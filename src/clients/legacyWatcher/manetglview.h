#ifndef MANETGLVIEW_H
#define MANETGLVIEW_H

#include <QGLWidget>
#include "logger.h"
#include "libwatcher/watcherGraph.h"
#include "messageStream.h"

class manetGLView : public QGLWidget
{
    Q_OBJECT

    public:

        manetGLView(QWidget *parent = 0);
        ~manetGLView();

        void loadConfiguration(); 

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
        void toggleNormPaths(bool isOn);
        void toggleMonochrome(bool isOn);
        void toggleThreeDView(bool isOn);
        void toggleBackgroundImage(bool isOn);

        void clearAllLabels();
        void clearAllEdges();

        void playbackStart();
        void playbackStop();
        void playbackPause();
        void playbackContinue();
        void playbackSetSpeed(int speed);

        void toggleNodeSelectedForGraph(unsigned int nodeId);
        void showNodeSelectedForGraph(unsigned int nodeId, bool);

        void saveConfiguration();

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
        void layerToggle(const watcher::event::GUILayer layer, const bool turnOn);

    private:

        watcher::MessageStream messageStream;
        watcher::WatcherGraph theGraph;

        QPoint lastPos;

        /** 
         * Layer list is an ordered list of known layers. If the layer is currently
         * active, active==true;
         */
        struct LayerListItem { watcher::event::GUILayer layer; bool active; }; 
        typedef boost::shared_ptr<LayerListItem> LayerListItemPtr; 
        typedef std::list<LayerListItemPtr> LayerList;
        LayerList knownLayers; 
        struct DisplayStatus
        {
            float scaleText;
            float scaleLine;
            int monochromeMode;
            int threeDView;
            int backgroundImage; 
        }; 

        struct ManetAdj
        {
            float angleX;
            float angleY;
            float angleZ;
            float scaleX;
            float scaleY;
            float scaleZ;
            float shiftX;
            float shiftY;
            float shiftZ;
        }; 

        ManetAdj manetAdj; 

        float scaleText;
        float scaleLine;
        float layerPadding;
        float antennaRadius; 

        bool monochromeMode;
        bool threeDView;
        bool backgroundImage; 

        void drawNodeLabel(const watcher::WatcherGraphNode &node, const float x, const float y, const float z);
        void gps2openGLPixels(const double x, const double y, const double z, GLdouble &x, GLdouble &y, GLdouble &z); 
        bool isActive(const watcher::GUILayer &layer); 

        // drawing stuff
        enum ScaleAndShiftUpdate
        {
            ScaleAndShiftUpdateOnChange,
            ScaleAndShiftUpdateAlways,
        };
        void scaleAndShiftToSeeOnManet(double xMin,double yMin,double xMax,double yMax,double z, ScaleAndShiftUpdate onChangeOrAlways);
        void scaleAndShiftToCenter(ScaleAndShiftUpdate onChangeOrAlways); 
        void getShiftAmount(GLdouble &x_ret, GLdouble &y_ret); 
        void shiftBackgroundCenterLeft(double dx);
        void shiftBackgroundCenterUp(double dy);
        void shiftCenterRight();
        void shiftCenterRight(double shift);
        void shiftCenterLeft();
        void shiftCenterLeft(double shift);
        void shiftCenterDown();
        void shiftCenterDown(double shift);
        void shiftCenterUp();
        void shiftCenterUp(double shift);
        void shiftCenterIn();
        void shiftCenterIn(double shift);
        void shiftCenterOut();
        void shiftCenterOut(double shift);
        void viewpointReset(void);
        void zoomOut();
        void zoomIn();
        void compressDistance();
        void expandDistance();
        void textZoomReset(void);
        void textZoomIn(void);
        void textZoomOut(void);
        void arrowZoomReset(void);
        void arrowZoomIn(void);
        void arrowZoomOut(void);
        void rotateX(float deg);
        void rotateY(float deg);
        void rotateZ(float deg);

        void handleSpin(int threeD, watcher::NodeDisplayInfoPtr &ndi); 
        void handleSize(watcher::NodeDisplayInfoPtr &ndi); 
        void drawWireframeSphere(GLdouble x, GLdouble y, GLdouble z, GLdouble radius, watcher::NodeDisplayInfoPtr &ndi); 
        void drawPyramid( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, watcher::NodeDisplayInfoPtr &ndi); 
        void drawCube(GLdouble x, GLdouble y, GLdouble z, GLdouble radius, watcher::NodeDisplayInfoPtr &ndi); 
        void drawTeapot(GLdouble x, GLdouble y, GLdouble z, GLdouble radius, watcher::NodeDisplayInfoPtr &ndi); 
        void drawDisk( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, watcher::NodeDisplayInfoPtr &ndi); 
        void drawTorus(GLdouble x, GLdouble y, GLdouble z, GLdouble radius, watcher::NodeDisplayInfoPtr &ndi); 
        void drawSphere( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, watcher::NodeDisplayInfoPtr &ndi); 
        void drawCircle( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, watcher::NodeDisplayInfoPtr &ndi); 
        void drawFrownyCircle(GLdouble x, GLdouble y, GLdouble z, GLdouble); 

};

#endif
