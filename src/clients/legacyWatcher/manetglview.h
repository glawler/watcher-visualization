/* Copyright 2009, 2010 SPARTA, Inc., dba Cobham Analytic Solutions
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
#include "libwatcher/gpsMessage.h"

#include "stringIndexedMenuItem.h"

namespace watcher {
	class WatcherStreamListDialog;
    class LayerConfigurationDialog;
    class NodeConfigurationDialog;
    class WatcherGUIConfig;
}

class manetGLView : public QGLWidget
{
    Q_OBJECT

    public:

        manetGLView(QWidget *parent = 0);
        ~manetGLView();

        QSize minimumSizeHint() const;
        QSize sizeHint() const;

        void setLayerMenu(QMenu *m) { layerMenu=m; }
        void setPlaybackSlider(QSlider *s);

        void setWatcherGUIConfig(watcher::WatcherGUIConfig *); 

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
        void newLayerConnect(const QString name); 
        void clearAllLabels();
        void clearAllEdges();
        void clearAll();
        void configureLayers(); 
        void showKeyboardShortcuts(); 
        void setEdgeWidth();
        void pausePlayback();
        void normalPlayback();
        void reversePlayback();
        void forwardPlayback();
        void rewindToStartOfPlayback();
        void forwardToEndOfPlayback();
        void playbackSetSpeed(double speed);
        void listStreams();
        void selectStream(unsigned long uid);
        void spawnStreamDescription();
    	void reconnect();
        void toggleNodeSelectedForGraph(unsigned int nodeId);
        void showNodeSelectedForGraph(unsigned int nodeId, bool);
        void scrollingGraphActivated(QString graphName);
        void updatePlaybackSliderFromGUI();
        void sliderMovedInGUI(int newVal);
        void sliderPressedInGUI();
        void spawnAboutBox(); 
        void spawnNodeConfigurationDialog();
        void streamFilteringEnabled(bool isOn); 
        void shutdown(); 
        void gpsScaleUpdated(double prevGpsScale); 
        void boundingBoxToggled(bool isOn); 

signals:
        void positionReset();
        void layerToggled(const QString &, bool);
        void connectNewLayer(const QString); 
        void spawnLayerConfigureDialog(); 
        void labelsCleared();
        void edgesCleared(); 
        void changeBackgroundColor();
        void nodeDataInGraphsToggled(unsigned int nodeId); 
        void nodeDataInGraphsShowed(unsigned int, bool); 
        void nodeClicked(size_t nodeId);
        // Emitted when the rate of the stream is changed.
        void streamRateSet(double); 
        // Emitted when view->backgroun image should be enabled/disabled.
        void enableBackgroundImage(bool);

    protected:
        DECLARE_LOGGER();

        // Overload the close so we can save the configuration.
        // void closeEvent(QCloseEvent *event);

        virtual void initializeGL();
        virtual void paintGL();
        virtual void resizeGL(int width, int height);

        void mouseDoubleClickEvent(QMouseEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void wheelEvent(QWheelEvent *event);

        void keyPressEvent(QKeyEvent * event);

        size_t getNodeIdAtCoords(const int x, const int y);
        void drawStatusString(QPainter &); 
        void drawDebugInfo(QPainter &);

    private:

        watcher::WatcherGUIConfig *conf;

        /** Where we keep our dynamic layers in the GUI */
        QMenu *layerMenu;

        /** The GUI slider which shows and controls backback location */
        QSlider *playbackSlider;

        /** dialog for display list of streams */
        watcher::WatcherStreamListDialog *streamsDialog;

        std::vector<watcher::StringIndexedMenuItem*> layerMenuItems;
        void addLayerMenuItem(const watcher::GUILayer &layer, bool active);

        watcher::MessageStreamPtr messageStream;
        watcher::WatcherGraph *wGraph;
        std::string serverName; 

        void connectStream(); // connect to watherd and init the message stream. blocking...
    	void setupStream();
        boost::thread *watcherdConnectionThread;
        boost::thread *maintainGraphThread;
        boost::thread *checkIOThread;
        boost::mutex graphMutex;

        float streamRate; 
        bool playbackPaused;
        bool sliderPressed;

        watcher::event::GPSMessage gpsDataFormat;

        QPoint lastPos;

        watcher::Timestamp currentMessageTimestamp;
        watcher::Timestamp playbackRangeEnd;
        watcher::Timestamp playbackRangeStart;

        void drawNodeLabel(const watcher::NodeDisplayInfo &node, bool physical);
        bool gps2openGLPixels(double &x, double &y, double &z, const watcher::event::GPSMessage::DataFormat &format);
        bool isActive(const watcher::GUILayer &layer); 

        // drawing stuff
        void drawNotConnectedState(); 
        bool autoCenterNodesFlag; 
        void drawManet(void);
        void drawGlobalView();
        void drawBoundingBox(); 
        void drawGroundGrid();
        void drawGraph(watcher::WatcherGraph *&graph); 
        struct QuadranglePoint
        {
            double x;
            double y;
        };
        void drawText(GLdouble x, GLdouble y, GLdouble z, GLdouble scale, char *text, GLdouble lineWidth=1.0);
        void drawEdge(const watcher::EdgeDisplayInfo &edge, const watcher::NodeDisplayInfo &node1, const watcher::NodeDisplayInfo &node2); 
        void drawNode(const watcher::NodeDisplayInfo &node, bool physical);
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
        void getNodeRectangle(double &xMin, double &xMax, double &yMin, double &yMax, double &zMin, double &zMax);
        void scaleAndShiftToSeeOnManet(double xMin,double yMin,double xMax,double yMax,double z, ScaleAndShiftUpdate onChangeOrAlways);
        void scaleAndShiftToCenter(ScaleAndShiftUpdate onChangeOrAlways); 
        void getShiftAmount(GLdouble &x_ret, GLdouble &y_ret); 
        void shiftBackgroundCenterLeft(double dx);
        void shiftBackgroundCenterUp(double dy);
		void zoomBackground(const int &delta); 
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

        void drawLabel(GLfloat x, GLfloat y, GLfloat z, const watcher::LabelDisplayInfo &label, int labelCount);
        void handleSpin(int threeD, const watcher::NodeDisplayInfo &ndi); 
        void handleSize(const watcher::NodeDisplayInfo &ndi); 
        void handleProperties(const watcher::NodeDisplayInfo &ndi); 

        void drawWireframeSphere(GLdouble radius); 
        void drawPyramid(GLdouble radius); 
        void drawCube(GLdouble radius); 
        void drawTeapot(GLdouble radius); 
        void drawTorus(GLdouble innerRadius, GLdouble outerRadius); 
        void drawSphere(GLdouble radius); 
        void drawCircle(GLdouble radius); 
        void drawFrownyCircle(GLdouble); 

        void changeSpeed(double);

        // updating the graph is a separate function as it's done in it's own thread
        void maintainGraph();

        unsigned int nodesDrawn, edgesDrawn, labelsDrawn;
        unsigned int framesDrawn, fpsTimeBase;
        double framesPerSec;

        std::string streamDescription;

        watcher::LayerConfigurationDialog *layerConfigurationDialog;
        watcher::NodeConfigurationDialog *nodeConfigurationDialog;

        size_t prevClickedNodeId;

        bool loadConfiguration(); 

        GLdouble maxNodeArea[3], minNodeArea[3]; 
};

#endif

// vim:sw=4
