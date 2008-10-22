#include <QtGui>
#include <QtOpenGL>
#include <math.h>

#include "manetglview.h"
#include "watcherScrollingGraphControl.h"
#include "legacyWatcher/legacyWatcher.h"

INIT_LOGGER(manetGLView, "manetGLView");

using namespace watcher;
using namespace std;

manetGLView::manetGLView(QWidget *parent) : QGLWidget(parent), currentView(legacyWatcher::ManetView)
{
    TRACE_ENTER();

    setFocusPolicy(Qt::StrongFocus); // tab and click to focus

    TRACE_EXIT();
}

manetGLView::~manetGLView()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void manetGLView::runLegacyWatcherMain(int argc, char **argv)
{
    TRACE_ENTER();

    legacyWatcher::legacyWatcherMain(argc, argv);

    // "main()" may have set different watcher layers, so we need to 
    // tell anyone who is interested that the layer are set or not.
    NodeDisplayStatus &ds = legacyWatcher::getDisplayStatus();
    emit bandwidthToggled(ds.familyBitmap & legacyWatcher::Bandwidth);
    emit undefinedToggled(ds.familyBitmap & legacyWatcher::Undefined);
    emit neighborsToggled(ds.familyBitmap & legacyWatcher::Neighbors);
    emit hierarchyToggled(ds.familyBitmap & legacyWatcher::Hierarchy);
    emit routingToggled(ds.familyBitmap & legacyWatcher::Routing);
    emit routingOnehopToggled(ds.familyBitmap & legacyWatcher::RoutingOnehop);
    emit antennaRadiusToggled(ds.familyBitmap & legacyWatcher::AntennaRadius);
    emit sanityCheckToggled(ds.familyBitmap & legacyWatcher::SanityCheck);
    emit anomPathsToggled(ds.familyBitmap & legacyWatcher::AnomPaths);
    emit correlationToggled(ds.familyBitmap & legacyWatcher::Correlation);
    emit alertToggled(ds.familyBitmap & legacyWatcher::Alert);
    emit correlation3HopToggled(ds.familyBitmap & legacyWatcher::Correlation3Hop);
    emit wormholeRoutingToggled(ds.familyBitmap & legacyWatcher::WormholeRouting);
    emit wormholeRoutingOnehopToggled(ds.familyBitmap & legacyWatcher::WormholeRoutingOnehop);
    emit floatingGraphToggled(ds.familyBitmap & legacyWatcher::FloatingGraph);
    emit normPathsToggled(ds.familyBitmap & legacyWatcher::NormPaths);
    emit bandwidthToggled(ds.familyBitmap & legacyWatcher::Bandwidth);

    // causes goodinw control buttons to show/not show.
    emit runningGoodwin(legacyWatcher::runningGoodwin()); 

    emit threeDViewToggled(ds.threeDView); 
    emit monochromeToggled(ds.monochromeMode); 
    emit backgroundImageToggled(ds.backgroundImage); 

    QTimer *checkIOTimer = new QTimer(this);
    QObject::connect(checkIOTimer, SIGNAL(timeout()), this, SLOT(checkIO()));
    checkIOTimer->start(100);

    QTimer *watcherIdleTimer = new QTimer(this);
    QObject::connect(watcherIdleTimer, SIGNAL(timeout()), this, SLOT(watcherIdle()));
    watcherIdleTimer->start(33); // Let's shoot for 30 frames a second.

    TRACE_EXIT();
}

QSize manetGLView::minimumSizeHint() const
{
    TRACE_ENTER();
    QSize retVal=QSize(50, 50);
    TRACE_EXIT_RET(retVal.width() << retVal.height()); 
    return retVal;
}

QSize manetGLView::sizeHint() const
{
    TRACE_ENTER();
    QSize retVal=QSize(400, 400);
    TRACE_EXIT_RET(retVal.width() << retVal.height()); 
    return retVal;
}

void manetGLView::initializeGL()
{
    TRACE_ENTER();
    // qglClearColor(trolltechPurple.dark());
    // object = makeObject();
    // glShadeModel(GL_FLAT);
    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);

    legacyWatcher::initWatcherGL(); 
    TRACE_EXIT();
}

void manetGLView::checkIO()
{
    TRACE_ENTER();
    int update=0;
    if (legacyWatcher::isRunningInPlaybackMode())
        update=legacyWatcher::checkIOGoodwin(0);
    else
        update=legacyWatcher::checkIOLive(0);
    
    // if (update)
        updateGL();

    TRACE_EXIT();
}

void manetGLView::watcherIdle()
{
    TRACE_ENTER();

    // Fake the glut idle() callback for the legacyWatcher.
    // legacyWatcher can use this timeout for animation, etc. 
    if( legacyWatcher::doIdle())
        updateGL();

    TRACE_EXIT();
}


void manetGLView::paintGL()
{
    TRACE_ENTER();

    //  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //  glLoadIdentity();
    //  glTranslated(0.0, 0.0, -10.0);
    //  glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
    //  glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
    //  glRotated(zRot / 16.0, 0.0, 0.0, 1.0);
    //  glCallList(object);

    if (currentView==legacyWatcher::ManetView)
        legacyWatcher::drawManet();
    else if(currentView==legacyWatcher::HierarchyView)
        legacyWatcher::drawHierarchy();
    else
        fprintf(stderr, "Error: I have an impossible watcher view!");

    // qglColor(Qt::blue);
    // renderText(-0.2, 0.2, 0.2, "Hello World");


    // char *text = 
    //     "Keyboard Shortcuts:\n"
    //     "-------------------\n"
    //     "esc: exit\n"
    //     "n: closer\n"
    //     "m: further\n"
    //     "q: zoom out\n"
    //     "w: zoom in\n"
    //     "a: text zoom out\n"
    //     "s: text zoom in\n"
    //     "ctrl a: arrow zoom out\n"
    //     "ctrl s: arrow zoom in\n"
    //     "x: compress z scale\n"
    //     "z: expand z scale\n"
    //     "e: tilt up\n"
    //     "r: tilt down\n"
    //     "d: rotate left\n"
    //     "f: rotate right\n"
    //     "c: spin clockwise\n"
    //     "v: spin counterclockwise\n"
    //     "b: bandwidth toggle\n"
    //     "ctrl r: reset viewpoint\n"
    //     "\' \' (space): start/stop\n"
    //                     "t: step on second\n";

    // int side = width() > height() ? height() : width(); 
    // glViewport((width() - side) / 2, (height() - side) / 2, side, side);

    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    // glOrtho(-0.5, +0.5, +0.5, -0.5, 4.0, 15.0);
    // glMatrixMode(GL_MODELVIEW);

    // QPainter painter;
    // painter.begin(this);
    // painter.save();
    // QFontMetrics metrics = QFontMetrics(font());
    // // int border = qMax(4, metrics.leading());
    // // QRect rect = metrics.boundingRect(0, 0, width() - 2*border, int(height()*0.125), Qt::AlignLeft | Qt::TextWordWrap, text);
    // painter.setRenderHint(QPainter::TextAntialiasing);
    // painter.setPen(Qt::black);
    // // painter.fillRect(QRect(0, 0, width(), rect.height() + 2*border), QColor(0, 0, 0, 64));
    // // painter.drawText((width() - rect.width())/2, border, rect.width(), rect.height(), Qt::AlignLeft | Qt::TextWordWrap, text);
    // painter.drawText(width()/2, height()/2, QString("Hello"));
    // painter.restore(); 
    // painter.end();
    
    TRACE_EXIT();
}

void manetGLView::resizeGL(int width, int height)
{
    TRACE_ENTER();

//     int side = qMin(width, height);
//     glViewport((width - side) / 2, (height - side) / 2, side, side);
// 
//     glMatrixMode(GL_PROJECTION);
//     glLoadIdentity();
//     glOrtho(-0.5, +0.5, +0.5, -0.5, 4.0, 15.0);
//     glMatrixMode(GL_MODELVIEW);

    if (currentView==legacyWatcher::ManetView)
        legacyWatcher::ReshapeManet(width, height);
    else 
        legacyWatcher::ReshapeHierarchy(width, height);

    TRACE_EXIT();
}

void manetGLView::resetPosition()
{
    TRACE_ENTER();

    legacyWatcher::viewpointReset();
    updateGL();

    TRACE_EXIT();
}


void manetGLView::keyPressEvent(QKeyEvent * event)
{
    TRACE_ENTER();

    quint32 nativeKey = event->nativeVirtualKey();
    int qtKey = event->key();

    if (legacyWatcher::Key(nativeKey) != 0)
    {
        if (nativeKey == 'B' || nativeKey == 'b')
        {
            NodeDisplayStatus &ds=legacyWatcher::getDisplayStatus();
            emit bandwidthToggled(ds.familyBitmap & legacyWatcher::Bandwidth);
        }

        updateGL();
    }
    // legacyWatcher::Key() does not handle arrow keys
    else if (qtKey==Qt::Key_Left || 
             qtKey==Qt::Key_Up   || 
             qtKey==Qt::Key_Right|| 
             qtKey==Qt::Key_Down)
    {
        if (qtKey==Qt::Key_Left)
            legacyWatcher::shiftCenterRight();
        else if (qtKey==Qt::Key_Right)
            legacyWatcher::shiftCenterLeft();
        else if (qtKey==Qt::Key_Up)
            legacyWatcher::shiftCenterDown();
        else 
            legacyWatcher::shiftCenterUp();

        updateGL();
    }
    else
        event->ignore();

    TRACE_EXIT();
}

void manetGLView::mouseDoubleClickEvent(QMouseEvent *event)
{
    TRACE_ENTER();
    
    // GTL - Currently does not work!
    // legacyWatcher::jumpToX(event->x());
    // legacyWatcher::jumpToY(event->y());
    //
    // char buf[512];
    // memset(buf, 0, sizeof(buf)); 
    // if(legacyWatcher::getNodeStatus(event->x(),event->y(), buf, sizeof(buf)))
    // {
    //     QMessageBox::information(this, tr("Watcher"), QString(buf));
    // }
    // updateGL();

    unsigned int nodeId=legacyWatcher::getNodeIdAtCoords(event->x(), event->y());
    if(nodeId)
    {
        emit nodeDataInGraphsToggled(nodeId);
    }

    TRACE_EXIT();
}

void manetGLView::showNodeSelectedForGraph(unsigned int, bool)
{
    TRACE_ENTER();
    // GTL - doesn't do anything now. But could mark the node as being displayed in the 
    // graph dialogs somehow. Add a little "G" or a little graph representation
    // above the node? 
    TRACE_EXIT();
}

void manetGLView::toggleNodeSelectedForGraph(unsigned int)
{
    TRACE_ENTER();
    // GTL - doesn't do anything now. But could mark the node as being displayed in the 
    // graph dialogs somehow. Add a little "G" or a little graph representation
    // above the node? 
    TRACE_EXIT();
}

void manetGLView::mousePressEvent(QMouseEvent *event)
{
    TRACE_ENTER();
    lastPos = event->pos();
    TRACE_EXIT();
}

void manetGLView::fitToWindow()
{
    TRACE_ENTER();
    
    // Just use the existing keyboard shortcut in the legacy watcher
    if (legacyWatcher::Key('+') != 0)
    {
        updateGL();
    }

    TRACE_EXIT();
}

void manetGLView::mouseMoveEvent(QMouseEvent *event)
{
    TRACE_ENTER();

    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    Qt::MouseButtons buttons = event->buttons();

    if ((buttons & Qt::LeftButton) && (buttons & Qt::RightButton))
    {
        if (dy>0)
            legacyWatcher::zoomIn();
        if (dy<0)
            legacyWatcher::zoomOut();
        updateGL();
    }
    else if ((buttons & Qt::LeftButton) && !(buttons & Qt::RightButton))
    {
        legacyWatcher::shiftCenterLeft(dx);
        legacyWatcher::shiftCenterUp(dy);
        updateGL();
    } 
    else if (!(buttons & Qt::LeftButton) && (buttons & Qt::RightButton))
    {
        legacyWatcher::rotateX(dy);
        legacyWatcher::rotateY(dx);
        updateGL();
    }
    lastPos = event->pos();

    TRACE_EXIT();
}

void manetGLView::wheelEvent(QWheelEvent *event)
{
    if(event->delta()>0)
        legacyWatcher::zoomIn();
    else
        legacyWatcher::zoomOut();
}

void manetGLView::setLegacyWatcherView(const legacyWatcher::WatcherView &view)
{
    TRACE_ENTER();
    legacyWatcher::setActiveView(view);
    currentView=view;
    TRACE_EXIT();
}

void manetGLView::manetView()
{
    TRACE_ENTER();
    setLegacyWatcherView(legacyWatcher::ManetView);
    updateGL();
    TRACE_EXIT();
}

void manetGLView::hierarchyView()
{
    TRACE_ENTER();
    setLegacyWatcherView(legacyWatcher::HierarchyView);
    updateGL();
    TRACE_EXIT();
}

void manetGLView::toggleBandwidth(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::Bandwidth, isOn);
    emit bandwidthToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}

void manetGLView::toggleUndefined(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::Undefined, isOn);
    emit undefinedToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleNeighbors(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::Neighbors, isOn);
    emit neighborsToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleHierarchy(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::Hierarchy, isOn);
    emit hierarchyToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleRouting(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::Routing, isOn);
    emit routingToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleRoutingOnehop(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::RoutingOnehop, isOn);
    emit routingOnehopToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleAntennaRadius(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::AntennaRadius, isOn);
    emit antennaRadiusToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleSanityCheck(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::SanityCheck, isOn);
    emit sanityCheckToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleAnomPaths(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::AnomPaths, isOn);
    emit anomPathsToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleCorrelation(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::Correlation, isOn);
    emit correlationToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleAlert(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::Alert, isOn);
    emit alertToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleCorrelation3Hop(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::Correlation3Hop, isOn);
    emit correlation3HopToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleWormholeRouting(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::WormholeRouting, isOn);
    emit wormholeRoutingToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleWormholeRoutingOnehop(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::WormholeRoutingOnehop, isOn);
    emit wormholeRoutingOnehopToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleFloatingGraph(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::FloatingGraph, isOn);
    emit floatingGraphToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleNormPaths(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::NormPaths, isOn);
    emit normPathsToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleMonochrome(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::toggleMonochrome(isOn);
    emit monochromeToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleThreeDView(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::toggleThreeDView(isOn);
    emit threeDViewToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleBackgroundImage(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::toggleBackgroundImage(isOn);
    emit backgroundImageToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::clearAllEdges()
{
    TRACE_ENTER();
    legacyWatcher::clearAllEdges();
    TRACE_EXIT();
}
void manetGLView::clearAllLabels()
{
    TRACE_ENTER();
    legacyWatcher::clearAllLabels();
    emit labelsCleared();
    TRACE_EXIT();
}
void manetGLView::goodwinStart()
{
    TRACE_ENTER();
    legacyWatcher::startGoodwin();  // will reload goodwin file and start from time 0
    TRACE_EXIT();
}
void manetGLView::goodwinStop()
{
    TRACE_ENTER();
    legacyWatcher::stopGoodwin();
    TRACE_EXIT();
}
void manetGLView::goodwinPause()
{
    TRACE_ENTER();
    legacyWatcher::pauseGoodwin();
    TRACE_EXIT();
}
void manetGLView::goodwinContinue()
{
    TRACE_ENTER();
    legacyWatcher::continueGoodwin();
    TRACE_EXIT();
}
void manetGLView::goodwinSetSpeed(int x)
{
    TRACE_ENTER();
    legacyWatcher::setGoodwinPlaybackSpeed(x);
    TRACE_EXIT();
}

