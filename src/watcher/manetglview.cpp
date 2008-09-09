#include <QtGui>
#include <QtOpenGL>
#include <math.h>

#include "manetglview.h"
#include "legacyWatcher/legacyWatcher.h"

manetGLView::manetGLView(QWidget *parent) : QGLWidget(parent), currentView(legacyWatcher::ManetView)
{
    setFocusPolicy(Qt::StrongFocus); // tab and click to focus
}

manetGLView::~manetGLView()
{
}

void manetGLView::runLegacyWatcherMain(int argc, char **argv)
{
    legacyWatcher::legacyWatcherMain(argc, argv);

    // "main()" may have set different watcher layers, so we need to 
    // tell anyone who is interested that the layer are set or not.
    unsigned int bitmap = legacyWatcher::getDisplayLayerBitmap();
    emit bandwidthToggled(bitmap & legacyWatcher::Bandwidth);
    emit undefinedToggled(bitmap & legacyWatcher::Undefined);
    emit neighborsToggled(bitmap & legacyWatcher::Neighbors);
    emit hierarchyToggled(bitmap & legacyWatcher::Hierarchy);
    emit routingToggled(bitmap & legacyWatcher::Routing);
    emit routingOnehopToggled(bitmap & legacyWatcher::RoutingOnehop);
    emit antennaRadiusToggled(bitmap & legacyWatcher::AntennaRadius);
    emit sanityCheckToggled(bitmap & legacyWatcher::SanityCheck);
    emit anomPathsToggled(bitmap & legacyWatcher::AnomPaths);
    emit correlationToggled(bitmap & legacyWatcher::Correlation);
    emit alertToggled(bitmap & legacyWatcher::Alert);
    emit correlation3HopToggled(bitmap & legacyWatcher::Correlation3Hop);
    emit wormholeRoutingToggled(bitmap & legacyWatcher::WormholeRouting);
    emit wormholeRoutingOnehopToggled(bitmap & legacyWatcher::WormholeRoutingOnehop);
    emit floatingGraphToggled(bitmap & legacyWatcher::FloatingGraph);
    emit normPathsToggled(bitmap & legacyWatcher::NormPaths);

    emit bandwidthToggled(bitmap & legacyWatcher::Bandwidth);

    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(checkIO()));
    timer->start(100);
}

QSize manetGLView::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize manetGLView::sizeHint() const
{
    return QSize(400, 400);
}

void manetGLView::initializeGL()
{
    // qglClearColor(trolltechPurple.dark());
    // object = makeObject();
    // glShadeModel(GL_FLAT);
    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);

    legacyWatcher::initWatcherGL(); 
}

void manetGLView::checkIO()
{
    int update=0;
    if (legacyWatcher::isRunningInPlaybackMode())
        update=legacyWatcher::checkIOGoodwin(0);
    else
        update=legacyWatcher::checkIOLive(0);
    
    if (update)
        updateGL();
}

void manetGLView::paintGL()
{
    //  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //  glLoadIdentity();
    //  glTranslated(0.0, 0.0, -10.0);
    //  glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
    //  glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
    //  glRotated(zRot / 16.0, 0.0, 0.0, 1.0);
    //  glCallList(object);

    qglColor(Qt::blue);
    renderText(-0.2, 0.2, 0.2, "Hello World");

    if (currentView==legacyWatcher::ManetView)
        legacyWatcher::drawManet();
    else if(currentView==legacyWatcher::HierarchyView)
        legacyWatcher::drawHierarchy();
    else
        fprintf(stderr, "Error: I have an impossible watcher view!");

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
}

void manetGLView::resizeGL(int width, int height)
{
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
}

void manetGLView::resetPosition()
{
    legacyWatcher::viewpointReset();
    updateGL();
}


void manetGLView::keyPressEvent(QKeyEvent * event)
{
    quint32 nativeKey = event->nativeVirtualKey();
    int qtKey = event->key();

    if (legacyWatcher::Key(nativeKey) != 0)
    {
        if (nativeKey == 'B' || nativeKey == 'b')
        {
            int bitmap = legacyWatcher::getDisplayLayerBitmap();
            emit bandwidthToggled(bitmap & legacyWatcher::Bandwidth);
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
}

void manetGLView::mouseDoubleClickEvent(QMouseEvent *event)
{
    // GTL - Currently does not work!
    legacyWatcher::jumpToX(event->x());
    legacyWatcher::jumpToY(event->y());
    updateGL();
}
void manetGLView::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void manetGLView::fitToWindow()
{
    // Just use the existing keyboard shortcut in the legacy watcher
    if (legacyWatcher::Key('+') != 0)
    {
        updateGL();
    }
}

void manetGLView::mouseMoveEvent(QMouseEvent *event)
{
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
}

void manetGLView::setLegacyWatcherView(const legacyWatcher::WatcherView &view)
{
    legacyWatcher::setActiveView(view);
    currentView=view;
}

void manetGLView::manetView()
{
    setLegacyWatcherView(legacyWatcher::ManetView);
    updateGL();
}

void manetGLView::hierarchyView()
{
    setLegacyWatcherView(legacyWatcher::HierarchyView);
    updateGL();
}

void manetGLView::toggleBandwidth(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::Bandwidth, isOn);
    emit bandwidthToggled(isOn); 
    updateGL();
}

void manetGLView::toggleUndefined(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::Undefined, isOn);
    emit undefinedToggled(isOn); 
    updateGL();
}
void manetGLView::toggleNeighbors(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::Neighbors, isOn);
    emit neighborsToggled(isOn); 
    updateGL();
}
void manetGLView::toggleHierarchy(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::Hierarchy, isOn);
    emit hierarchyToggled(isOn); 
    updateGL();
}
void manetGLView::toggleRouting(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::Routing, isOn);
    emit routingToggled(isOn); 
    updateGL();
}
void manetGLView::toggleRoutingOnehop(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::RoutingOnehop, isOn);
    emit routingOnehopToggled(isOn); 
    updateGL();
}
void manetGLView::toggleAntennaRadius(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::AntennaRadius, isOn);
    emit antennaRadiusToggled(isOn); 
    updateGL();
}
void manetGLView::toggleSanityCheck(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::SanityCheck, isOn);
    emit sanityCheckToggled(isOn); 
    updateGL();
}
void manetGLView::toggleAnomPaths(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::AnomPaths, isOn);
    emit anomPathsToggled(isOn); 
    updateGL();
}
void manetGLView::toggleCorrelation(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::Correlation, isOn);
    emit correlationToggled(isOn); 
    updateGL();
}
void manetGLView::toggleAlert(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::Alert, isOn);
    emit alertToggled(isOn); 
    updateGL();
}
void manetGLView::toggleCorrelation3Hop(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::Correlation3Hop, isOn);
    emit correlation3HopToggled(isOn); 
    updateGL();
}
void manetGLView::toggleWormholeRouting(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::WormholeRouting, isOn);
    emit wormholeRoutingToggled(isOn); 
    updateGL();
}
void manetGLView::toggleWormholeRoutingOnehop(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::WormholeRoutingOnehop, isOn);
    emit wormholeRoutingOnehopToggled(isOn); 
    updateGL();
}
void manetGLView::toggleFloatingGraph(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::FloatingGraph, isOn);
    emit floatingGraphToggled(isOn); 
    updateGL();
}
void manetGLView::toggleNormPaths(bool isOn)
{
    legacyWatcher::layerToggle(legacyWatcher::NormPaths, isOn);
    emit normPathsToggled(isOn); 
    updateGL();
}
