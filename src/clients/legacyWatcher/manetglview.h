/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * @file manetglview.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#ifndef MANETGLVIEW_H
#define MANETGLVIEW_H

#include <QGLWidget>
#include <QMenu>
#include <QSlider>
#include <QTimer>
#include <boost/thread/locks.hpp>
#include "declareLogger.h"
#include "libwatcher/watcherGraph.h"
#include "libwatcher/messageStream.h"

#include "stringIndexedMenuItem.h"

class manetGLView : public QGLWidget
{
    Q_OBJECT

    public:

        manetGLView(QWidget *parent = 0);
        ~manetGLView();

        bool loadConfiguration(); 

        QSize minimumSizeHint() const;
        QSize sizeHint() const;

        void setLayerMenu(QMenu *m) { layerMenu=m; }
        void setPlaybackSlider(QSlider *s);

        /** Update the slider based on current values */
        void updatePlaybackSliderRange();

public slots:

        void resetPosition();
        void fitToWindow();
        void checkIO();

        // timeout callback for watcher to do "idle" work.
        void watcherIdle();

        // void layerToggled(QString, bool);
        void layerToggle(const QString &layer, const bool turnOn);

        void clearAllLabels();
        void clearAllEdges();
        void clearAll();

        void toggleMonochrome(bool isOn);
        void toggleThreeDView(bool isOn);
        void toggleBackgroundImage(bool isOn);
        void toggleLoopPlayback(bool inOn);
        void showKeyboardShortcuts(); 

        void setGPSScale();  // spawn dialog to get new scale value
        void setEdgeWidth(); // spawn dialog to get default edge width

        void showPlaybackTime(bool isOn);
        void showPlaybackRange(bool isOn);
        void showWallTime(bool isOn);

        void pausePlayback();
        void normalPlayback();
        void reversePlayback();
        void forwardPlayback();
        void rewindToStartOfPlayback();
        void forwardToEndOfPlayback();
        void playbackSetSpeed(double speed);

        void toggleNodeSelectedForGraph(unsigned int nodeId);
        void showNodeSelectedForGraph(unsigned int nodeId, bool);

        void scrollingGraphActivated(QString graphName);

        void updatePlaybackSliderFromGUI();
        void sliderMovedInGUI(int newVal);
        void sliderPressedInGUI();

        void spawnAboutBox(); 
        void saveConfiguration();
        void streamFilteringEnabled(bool isEnabled); 

        void shutdown(); 

signals:
        void positionReset();

        void layerToggled(const QString &, bool);

        void labelsCleared();
        void edgesCleared(); 

        void monochromeToggled(bool isOn);
        void threeDViewToggled(bool isOn); 
        void backgroundImageToggled(bool isOn); 
        void loopPlaybackToggled(bool inOn);

        void checkPlaybackTime(bool isOn);
        void checkPlaybackRange(bool isOn);
        void checkWallTime(bool isOn);


        void nodeDataInGraphsToggled(unsigned int nodeId); 
        void nodeDataInGraphsShowed(unsigned int, bool); 

        // Emitted when the rate of the stream is changed.
        void streamRateSet(double); 

        // Emitted when view->backgroun image should be enabled/disabled.
        void enableBackgroundImage(bool);

        void enableStreamFiltering(bool); 

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

        unsigned int getNodeIdAtCoords(const int x, const int y);
        void drawStatusString(); 
        void drawDebugInfo();

    private:

        /** Where we keep our dynamic layers in the GUI */
        QMenu *layerMenu;

        /** The GUI slider which shows and controls backback location */
        QSlider *playbackSlider;

        std::vector<watcher::StringIndexedMenuItem*> layerMenuItems;
        void addLayerMenuItem(const watcher::GUILayer &layer, bool active);

        watcher::MessageStreamPtr messageStream;
        watcher::WatcherGraph wGraph;
        std::string serverName; 

        void connectStream(); // connect to watherd and init the message stream. blocking...
        boost::thread *watcherdConnectionThread;
        boost::thread *maintainGraphThread;
        boost::thread *checkIOThread;
        boost::mutex graphMutex;

        float streamRate; 
        bool playbackPaused;
        bool autorewind;
        bool messageStreamFiltering;
        bool sliderPressed;

        watcher::event::GPSMessage gpsDataFormat;
        
        QPoint lastPos;

        watcher::Timestamp currentMessageTimestamp;
        watcher::Timestamp playbackRangeEnd;
        watcher::Timestamp playbackRangeStart;

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
            int scaleText;
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
        ManetAdj manetAdjInit;
        bool showWallTimeinStatusString;
        bool showPlaybackTimeInStatusString;
        bool showPlaybackRangeString;
        bool showVerboseStatusString;
        bool showDebugInfo;

        float scaleText;
        float scaleLine;
        float gpsScale; 
        float layerPadding;
        float antennaRadius; 
        float ghostLayerTransparency;

        int statusFontPointSize; 

        bool monochromeMode;
        bool threeDView;
        bool backgroundImage; 

        std::string statusFontName;
        watcher::Color hierarchyRingColor;

        watcher::Timestamp playbackStartTime;

        void drawNodeLabel(const watcher::WatcherGraphNode &node, bool physical);
        void gps2openGLPixels(const watcher::GPSMessage::DataFormat &format, const double inx, const double iny, const double inz, GLdouble &x, GLdouble &y, GLdouble &z);
        bool isActive(const watcher::GUILayer &layer); 

        // drawing stuff
        void drawNotConnectedState(); 
        bool autoCenterNodesFlag; 
        void drawManet(void);
        struct QuadranglePoint
        {
            double x;
            double y;
        };
        void drawText(GLdouble x, GLdouble y, GLdouble z, GLdouble scale, char *text, GLdouble lineWidth=1.0);
        void drawLayer(const watcher::GUILayer &layer); 
        void drawEdge(const watcher::WatcherGraphEdge &edge, const watcher::WatcherGraphNode &node1, const watcher::WatcherGraphNode &node2); 
        void drawNode(const watcher::WatcherGraphNode &node, bool physical);
        struct Quadrangle
        {
            QuadranglePoint p[4];
        };
        void maxRectangle( Quadrangle const *q, double , double *xMinRet, double *yMinRet, double *xMaxRet, double *yMaxRet);
        void invert4x4(GLdouble dst[16], GLdouble const src[16]);
        struct XYWorldZToWorldXWorldY
        {
            int x;
            int y;
            GLdouble worldZ;
            GLdouble worldX_ret;
            GLdouble worldY_ret;
        };
        int xyAtZForModelProjViewXY( XYWorldZToWorldXWorldY *xyz, size_t xyz_count, GLdouble modelmatrix[16], GLdouble projmatrix[16], GLint viewport[4]);
        int visibleDrawBoxAtZ(GLint *viewport, GLdouble z, GLdouble modelmatrix[16], GLdouble projmatrix[16], double maxRectAspectRatio, double *xMinRet,
                double *yMinRet, double *xMaxRet, double *yMaxRet);
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
        void arrowZoomReset(void);
        void arrowZoomIn(void);
        void arrowZoomOut(void);
        void rotateX(float deg);
        void rotateY(float deg);
        void rotateZ(float deg);

        void drawLabel(GLfloat x, GLfloat y, GLfloat z, const watcher::LabelDisplayInfoPtr &label);
        void handleSpin(int threeD, const watcher::NodeDisplayInfoPtr &ndi); 
        void handleSize(const watcher::NodeDisplayInfoPtr &ndi); 
        void handleProperties(const watcher::NodeDisplayInfoPtr &ndi); 

        void drawWireframeSphere(GLdouble radius); 
        void drawPyramid(GLdouble radius); 
        void drawCube(GLdouble radius); 
        void drawTeapot(GLdouble radius); 
        void drawTorus(GLdouble innerRadius, GLdouble outerRadius); 
        void drawSphere(GLdouble radius); 
        void drawCircle(GLdouble radius); 
        void drawFrownyCircle(GLdouble); 

        // updating the graph is a separate function as it's done in it's own thread
        void maintainGraph();
};

#endif
