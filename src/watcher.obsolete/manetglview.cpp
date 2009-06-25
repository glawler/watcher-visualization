#include <QtGui>
#include <QtOpenGL>
#include <math.h>

#include "manetglview.h"
#include "watcherScrollingGraphControl.h"
#include "legacyWatcher/legacyWatcher.h"
#include "singletonConfig.h"
#include "backgroundImage.h"

INIT_LOGGER(manetGLView, "manetGLView");

using namespace watcher;
using namespace legacyWatcher;
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

    // legacy watcher think's it's main() is getting called. 
    legacyWatcher::legacyWatcherMain(argc, argv);

    // "main()" may have set different watcher layers, so we need to 
    // grab the current values to toggle the GUI and set teh config.
    NodeDisplayStatus &ds = legacyWatcher::getDisplayStatus();

    // causes goodinw control buttons to show/not show.
    emit runningGoodwin(legacyWatcher::runningGoodwin()); 

    //
    // Check configuration for GUI settings.
    //
    singletonConfig &sc=singletonConfig::instance();
    sc.lock();
    libconfig::Config &cfg=sc.getConfig();
    libconfig::Setting &root=cfg.getRoot();

    string prop="layers";
    if (!root.exists(prop))
        root.add(prop, libconfig::Setting::TypeGroup);

    bool boolVal=false;
    if (!ds.familyBitmap)  // If the command line set the bitmap (!0), leave it alone.
    {
        libconfig::Setting &layers=cfg.lookup(prop);

        struct 
        {
            const char *prop;
            legacyWatcher::Layer layer;
        } layerVals[] =
        {
            { "bandwidth", legacyWatcher::Bandwidth },
            { "antennaRadius", legacyWatcher::AntennaRadius },
            { "sanityCheck", legacyWatcher::SanityCheck },
            { "neighbors", legacyWatcher::Neighbors },
            { "hierarchy", legacyWatcher::Hierarchy },

            { "Base", legacyWatcher::Base },
            { "GroupA", legacyWatcher::GroupA },
            { "GroupB", legacyWatcher::GroupB },
            { "GroupD", legacyWatcher::GroupD },
            { "GroupE", legacyWatcher::GroupE }, 
            { "Mission", legacyWatcher::Mission },
            { "GroupAChildren", legacyWatcher::GroupAChildren }, 
            { "GroupBChildren", legacyWatcher::GroupBChildren },
            { "GroupDChildren", legacyWatcher::GroupDChildren },
            { "GroupEChildren", legacyWatcher::GroupEChildren },
            { "MissionChildren", legacyWatcher::MissionChildren },

            { "undefined", legacyWatcher::Undefined }
        };
        for (size_t i=0; i<sizeof(layerVals)/sizeof(layerVals[0]); i++)
        {
            LOG_DEBUG("Looking up layer " << layerVals[i].prop); 
            if (layers.lookupValue(layerVals[i].prop, boolVal))
                legacyWatcher::layerToggle(layerVals[i].layer, boolVal);
            else
                layers.add(layerVals[i].prop, libconfig::Setting::TypeBoolean)=boolVal;
            boolVal=false;
        }
    }

    prop="nodes3d";
    boolVal=ds.threeDView;
    if (root.lookupValue(prop, boolVal))
        ds.threeDView=(boolVal?1:0);
    else
        root.add(prop, libconfig::Setting::TypeBoolean)=boolVal;

    prop="monochrome";
    boolVal=false;
    if (root.lookupValue(prop, boolVal))
        ds.monochromeMode=(boolVal?1:0);
    else
        root.add(prop, libconfig::Setting::TypeBoolean)=boolVal;

    prop="displayBackgroundImage";
    boolVal=ds.backgroundImage;
    if (root.lookupValue(prop, boolVal))
        ds.backgroundImage=(boolVal?1:0);
    else
        root.add(prop, libconfig::Setting::TypeBoolean)=boolVal;


    // Give the GUI the current toggle state of the display.
    emit bandwidthToggled(ds.familyBitmap & legacyWatcher::Bandwidth);
    emit undefinedToggled(ds.familyBitmap & legacyWatcher::Undefined);
    emit neighborsToggled(ds.familyBitmap & legacyWatcher::Neighbors);
    emit hierarchyToggled(ds.familyBitmap & legacyWatcher::Hierarchy);
    emit sanityCheckToggled(ds.familyBitmap & legacyWatcher::SanityCheck);
    emit antennaRadiusToggled(ds.familyBitmap & legacyWatcher::AntennaRadius);

    emit baseToggled(ds.familyBitmap & legacyWatcher::Base);
    emit groupAToggled(ds.familyBitmap & legacyWatcher::GroupA);
    emit groupBToggled(ds.familyBitmap & legacyWatcher::GroupB);
    emit groupDToggled(ds.familyBitmap & legacyWatcher::GroupD);
    emit groupEToggled(ds.familyBitmap & legacyWatcher::GroupE);
    emit missionToggled(ds.familyBitmap & legacyWatcher::Mission);
    emit groupAChildrenToggled(ds.familyBitmap & legacyWatcher::GroupAChildren);
    emit groupBChildrenToggled(ds.familyBitmap & legacyWatcher::GroupBChildren);
    emit groupDChildrenToggled(ds.familyBitmap & legacyWatcher::GroupDChildren);
    emit groupEChildrenToggled(ds.familyBitmap & legacyWatcher::GroupEChildren);
    emit missionChildrenToggled(ds.familyBitmap & legacyWatcher::MissionChildren);

    emit threeDViewToggled(ds.threeDView);
    emit monochromeToggled(ds.monochromeMode);
    emit backgroundImageToggled(ds.backgroundImage);

    //
    // Load background image settings
    //
    prop="backgroundImage";
    if (!root.exists(prop))
        root.add(prop, libconfig::Setting::TypeGroup);
    libconfig::Setting &s=cfg.lookup(prop);

    prop="imageFile"; 
    string strVal;
    if (!s.lookupValue(prop, strVal) || strVal=="none")   // If we don't have the setting or the it's set to 'do not use bg image'
    {
        LOG_INFO("watcherBackground:imageFile entry not found (or it equals \"none\") in configuration file, "
                "disabling background image functionality");
        if (strVal.empty())
            s.add(prop, libconfig::Setting::TypeString)="none";
        toggleBackgroundImage(false);
        emit enableBackgroundImage(false);
    }
    else
    {
        BackgroundImage &bgImage=BackgroundImage::getInstance(); 
        char *ext=rindex(strVal.data(), '.');
        if (!ext)
        {
            LOG_ERROR("I have no idea what kind of file the background image " << strVal << " is. I only support BMP and PPM"); 
            exit(1);
        }
        else if (0==strncasecmp(ext+sizeof(char), "bmp", 3))
        {
            if (!bgImage.loadBMPFile(strVal.data()))
            {
                LOG_ERROR("Unable to load background BMP image in watcher from file: " << strVal); 
                exit(1); 
            }
        }
        else if (0==strncmp("ppm", ext+sizeof(char), 3))
        {
            if (!bgImage.loadPPMFile(strVal.data()))
            {
                LOG_ERROR("Unable to load background PPM image in watcher from file: " << strVal); 
                exit(1); 
            }
        }
    }
    // bg image location and size.
    prop="coordinates";
    float floatVals[5]={0.0, 0.0, 0.0, 0.0, 0.0};
    if (!s.exists(prop))
    {
        s.add(prop, libconfig::Setting::TypeArray);
        for (size_t i=0; i<sizeof(floatVals)/sizeof(floatVals[0]); i++)
            s[prop].add(libconfig::Setting::TypeFloat)=floatVals[i];
    }
    else
    {
        for (size_t i=0; i<sizeof(floatVals)/sizeof(floatVals[0]); i++)
            floatVals[i]=s[prop][i];
        BackgroundImage &bg=BackgroundImage::getInstance();
        bg.setDrawingCoords(floatVals[0], floatVals[1], floatVals[2], floatVals[3], floatVals[4]); 
    }

    // Load GPS data format setting
    prop="gpsDataFormat";
    strVal="";
    if (!root.lookupValue(prop, strVal))
    {
        root.add(prop, libconfig::Setting::TypeString);
        root[prop]="lat-long-alt-WGS84"; // Default
    }

    if (strVal == "UTM") 
        legacyWatcher::setGPSDataFormat(legacyWatcher::GPS_DATA_FORMAT_UTM);
    else if(strVal == "lat-long-alt-WGS84")
        legacyWatcher::setGPSDataFormat(legacyWatcher::GPS_DATA_FORMAT_DEG_WGS84);
    else
    {
        if (strVal.empty())
            LOG_INFO("There is no gpsDataFormat argument in the cfg file, setting to default: lat-long-alt-WGS84");
        else
            LOG_WARN("I don't understand the gpsDataFormat argument in the cfg file, \"" << strVal << "\", setting to default: lat-long-alt-WGS84");

        legacyWatcher::setGPSDataFormat(legacyWatcher::GPS_DATA_FORMAT_DEG_WGS84);
    }

    //
    // Load viewpoint
    //
    GlobalManetAdj &ma=legacyWatcher::getManetAdj();
    prop="viewPoint";
    if (!root.exists(prop))
        root.add(prop, libconfig::Setting::TypeGroup);
    libconfig::Setting &vp=cfg.lookup(prop); 

    struct 
    {
        const char *type;
        float *data[3];
    } viewPoints[] =
    {
        { "angle", { &ma.angleX, &ma.angleY, &ma.angleZ }},
        { "scale", { &ma.scaleX, &ma.scaleY, &ma.scaleZ }},
        { "shift", { &ma.shiftX, &ma.shiftY, &ma.shiftZ }}
    };
    for (size_t i=0; i<sizeof(viewPoints)/sizeof(viewPoints[0]);i++)
    {
        if (!vp.exists(viewPoints[i].type))
        {
            vp.add(viewPoints[i].type, libconfig::Setting::TypeArray);
            for (size_t j=0; j<sizeof(viewPoints[i].data)/sizeof(viewPoints[i].data[0]); j++)
                vp[viewPoints[i].type].add(libconfig::Setting::TypeFloat);
        }
        else
        {
            libconfig::Setting &s=vp[viewPoints[i].type];
            for (int j=0; j<s.getLength(); j++)
                *viewPoints[i].data[j]=s[j];
        }
    }

    LOG_INFO("Set viewpoint - angle: " << ma.angleX << ", " << ma.angleY << ", " << ma.angleZ);
    LOG_INFO("Set viewpoint - scale: " << ma.scaleX << ", " << ma.scaleY << ", " << ma.scaleZ);
    LOG_INFO("Set viewpoint - shift: " << ma.shiftX << ", " << ma.shiftY << ", " << ma.shiftZ);

    // background color
    prop="backgroundColor";
    if (!root.exists(prop))
        root.add(prop, libconfig::Setting::TypeGroup);
    libconfig::Setting &bgColSet=cfg.lookup(prop);

    struct 
    {
        const char *name;
        float val;
    } bgColors[] = 
    {
        { "r", 0.0 }, 
        { "g", 0.0 }, 
        { "b", 0.0 }, 
        { "a", 255.0 }
    };
    for (size_t i=0; i<sizeof(bgColors)/sizeof(bgColors[0]);i++)
        if (!bgColSet.lookupValue(bgColors[i].name, bgColors[i].val))
            bgColSet.add(bgColors[i].name, libconfig::Setting::TypeFloat)=bgColors[i].val;

    legacyWatcher::setBackgroundColor(bgColors[0].val, bgColors[1].val,bgColors[2].val,bgColors[3].val);

    sc.unlock();

    // 
    // Set up timer callbacks.
    //
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

    // int side = qMin(width, height);
    // glViewport((width - side) / 2, (height - side) / 2, side, side);
    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    // glOrtho(-5, +5, +5, -5, 1.0, 50.0);
    // glMatrixMode(GL_MODELVIEW);

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
    if (nativeKey=='C')
    {
        LOG_DEBUG("Got cap C in keyPressEvent - spawning color chooser for background color"); 
        QRgb rgb=0xffffffff;
        bool ok=false;
        rgb=QColorDialog::getRgba(rgb, &ok);
        if (ok)
        {
            legacyWatcher::setBackgroundColor(qRed(rgb)/255.0, qGreen(rgb)/255.0, qBlue(rgb)/255.0, qAlpha(rgb)/255.0);
        }
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
    
    // Check for shift click and move bg image if shifted
    Qt::KeyboardModifiers mods=event->modifiers();
    if (mods & Qt::ShiftModifier)
    {
        // This should work - but doesn't.
        //
        // GLdouble modelmatrix[16];
        // GLdouble projmatrix[16];
        // GLint viewport[4];

        // glGetDoublev(GL_MODELVIEW_MATRIX, modelmatrix);
        // glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
        // glGetIntegerv(GL_VIEWPORT, viewport);

        // GLfloat winx=event->x(), winy=event->y(), winz=0.0;
        // winy=viewport[3]-winy;

        // BackgroundImage &bg=BackgroundImage::getInstance();
        // GLfloat x,y,w,h,z;
        // bg.getDrawingCoords(x, w, y, h, z);

        // GLdouble objx, objy, objz;
        // if (gluUnProject(winx, winy, winz, modelmatrix, projmatrix, viewport, &objx, &objy, &objz)==GL_TRUE)
        // {
        //     GlobalManetAdj &ma=legacyWatcher::getManetAdj();
        //     bg.setDrawingCoords(x+objx, w, y+objy, h, z);
        // }

        // hack - let the centering code center the BG image.
        BackgroundImage &bg=BackgroundImage::getInstance();
        bg.centerImage(true); 
    }
    else
    {
        unsigned int nodeId=legacyWatcher::getNodeIdAtCoords(event->x(), event->y());
        if(nodeId)
        {
            emit nodeDataInGraphsToggled(nodeId);
        }
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
    Qt::KeyboardModifiers mods=event->modifiers();

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
        if (mods & Qt::ShiftModifier)
        {
            legacyWatcher::shiftBackgroundCenterLeft(dx);
            legacyWatcher::shiftBackgroundCenterUp(-dy);
        }
        else
        {
            legacyWatcher::shiftCenterLeft(dx);
            legacyWatcher::shiftCenterUp(dy);
        }
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
//
// ZODIAC LAYERS START
//
void manetGLView::toggleBase(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::Base, isOn);
    emit baseToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleGroupA(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::GroupA, isOn);
    emit groupAToggled(isOn); 
    LOG_INFO("Group A Toggled" << (isOn?" on":" off")); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleGroupB(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::GroupB, isOn);
    LOG_INFO("Group B Toggled" << (isOn?" on":" off")); 
    emit groupBToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleGroupD(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::GroupD, isOn);
    emit groupDToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleGroupE(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::GroupE, isOn);
    emit groupEToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleMission(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::Mission, isOn);
    emit missionToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleGroupAChildren(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::GroupAChildren, isOn);
    emit groupAChildrenToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleGroupBChildren(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::GroupBChildren, isOn);
    emit groupBChildrenToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleGroupDChildren(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::GroupDChildren, isOn);
    emit groupDChildrenToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleGroupEChildren(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::GroupEChildren, isOn);
    emit groupEChildrenToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleMissionChildren(bool isOn)
{
    TRACE_ENTER();
    legacyWatcher::layerToggle(legacyWatcher::MissionChildren, isOn);
    emit missionChildrenToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
//
// ZODIAC LAYERS STOP
//
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
    LOG_DEBUG("Turning background image " << (isOn==true?"on":"off")); 
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

void manetGLView::saveConfiguration()
{
    TRACE_ENTER();
    LOG_DEBUG("Got close event, saving modified configuration"); 

    singletonConfig &sc=singletonConfig::instance();
    sc.lock();
    libconfig::Config &cfg=sc.getConfig();
    libconfig::Setting &root=cfg.getRoot();
    NodeDisplayStatus &ds = legacyWatcher::getDisplayStatus();

    struct 
    {
        const char *prop;
        bool boolVal;
    } boolConfigs[] =
    {
        { "nodes3d",        ds.threeDView },
        { "monochrome",     ds.monochromeMode },
        { "displayBackgroundImage", ds.backgroundImage }
    };

    for (size_t i = 0; i < sizeof(boolConfigs)/sizeof(boolConfigs[0]); i++)
        root[boolConfigs[i].prop]=boolConfigs[i].boolVal;

    string prop="layers";
    libconfig::Setting &layers=cfg.lookup(prop);

    struct 
    {
        const char *prop;
        legacyWatcher::Layer layer;
    } layerVals[] =
    {
        { "bandwidth", legacyWatcher::Bandwidth },
        { "antennaRadius", legacyWatcher::AntennaRadius },
        { "sanityCheck", legacyWatcher::SanityCheck },
        { "neighbors", legacyWatcher::Neighbors },
        { "hierarchy", legacyWatcher::Hierarchy },

        { "Base", legacyWatcher::Base },
        { "GroupA", legacyWatcher::GroupA },
        { "GroupB", legacyWatcher::GroupB },
        { "GroupD", legacyWatcher::GroupD },
        { "GroupE", legacyWatcher::GroupE }, 
        { "Mission", legacyWatcher::Mission },
        { "GroupAChildren", legacyWatcher::GroupAChildren }, 
        { "GroupBChildren", legacyWatcher::GroupBChildren },
        { "GroupDChildren", legacyWatcher::GroupDChildren },
        { "GroupEChildren", legacyWatcher::GroupEChildren },
        { "MissionChildren", legacyWatcher::MissionChildren },

        { "undefined", legacyWatcher::Undefined }
    };
    for (size_t i=0; i<sizeof(layerVals)/sizeof(layerVals[0]); i++)
        layers[layerVals[i].prop]=ds.familyBitmap & layerVals[i].layer ? true : false;

    GlobalManetAdj &ma=legacyWatcher::getManetAdj();
    root["viewPoint"]["angle"][0]=ma.angleX;
    root["viewPoint"]["angle"][1]=ma.angleY;
    root["viewPoint"]["angle"][2]=ma.angleZ;
    root["viewPoint"]["scale"][0]=ma.scaleX;
    root["viewPoint"]["scale"][1]=ma.scaleY;
    root["viewPoint"]["scale"][2]=ma.scaleZ;
    root["viewPoint"]["shift"][0]=ma.shiftX;
    root["viewPoint"]["shift"][1]=ma.shiftY;
    root["viewPoint"]["shift"][2]=ma.shiftZ;

    BackgroundImage &bg=BackgroundImage::getInstance();
    float floatVals[5];
    bg.getDrawingCoords(floatVals[0], floatVals[1], floatVals[2], floatVals[3], floatVals[4]); 
    root["backgroundImage"]["coordinates"][0]=floatVals[0];
    root["backgroundImage"]["coordinates"][1]=floatVals[1];
    root["backgroundImage"]["coordinates"][2]=floatVals[2];
    root["backgroundImage"]["coordinates"][3]=floatVals[3];
    root["backgroundImage"]["coordinates"][4]=floatVals[4];

    legacyWatcher::getBackgroundColor(floatVals[0], floatVals[1], floatVals[2], floatVals[3]);
    root["backgroundColor"]["r"]=floatVals[0];
    root["backgroundColor"]["g"]=floatVals[1];
    root["backgroundColor"]["b"]=floatVals[2];
    root["backgroundColor"]["a"]=floatVals[3];

    GPSDataFormat format=legacyWatcher::getGPSDataFormat();
    switch(format)
    {
        case GPS_DATA_FORMAT_UTM: 
            root["gpsDataFormat"]="UTM"; 
            break;
        case GPS_DATA_FORMAT_DEG_WGS84: 
            root["gpsDataFormat"]="lat-long-alt-WGS84";
            break;
        // Don't put default case
    }

    sc.saveConfig();
    sc.unlock();

    TRACE_EXIT();
}

