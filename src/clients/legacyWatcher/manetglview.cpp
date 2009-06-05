#include <QtGui>
#include <QtOpenGL>
#include <math.h>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "manetglview.h"
#include "watcherScrollingGraphControl.h"
// #include "legacyWatcher/legacyWatcher.h"
#include "singletonConfig.h"
#include "backgroundImage.h"

INIT_LOGGER(manetGLView, "manetGLView");

using namespace watcher;
using namespace watcher::event;
using namespace legacyWatcher;
using namespace std;
using namespace boost::posix_time;

static int globalAutoCenterNodesFlag = 0; 
//
// Make the (xMin,yMin)-(xMax,yMax) rectangle on the xy-plane at
// z visible on the Manet screen
//
// onChangeOrAlways - ScaleAndShiftUpdateOnChange to only update scale
//                    and shift values if things have changed enough,
//                    ScaleAndShiftUpdateAlways to update even if things
//                    haven't changed at all Usually will not update
//                    scaling or shifting if nothing has changed
//
void manetGLView::scaleAndShiftToSeeOnManet(
        double xMin, 
        double yMin, 
        double xMax, 
        double yMax, 
        double z,
        ScaleAndShiftUpdate onChangeOrAlways)
{
    static int upfcnt = 0;
    static int nccnt = 0;
    static double prevXMin = DBL_MAX;
    static double prevXMax = -DBL_MAX;
    static double prevYMin = DBL_MAX;
    static double prevYMax = -DBL_MAX;
    static double prevZ = DBL_MAX;
    static GLint prevWidth;
    static GLint prevHeight;
    GLint viewport[4];
    // adjust min and max values so the change less often. Do this
    // by reducing the precision to only three digits of precision 
    // by rounding the lower values down and higher values up.
    double dx = xMax - xMin;
    double dy = yMax - yMin;
    double mul = exp10(floor(log10(dx < dy ? dx : dy)) - 2);
    xMin = mul * floor(xMin / mul);
    yMin = mul * floor(yMin / mul);
    xMax = mul * ceil(xMax / mul);
    yMax = mul * ceil(yMax / mul);
    // get the viewport because the scale and centering might change
    // based on a screen resize
    glGetIntegerv(GL_VIEWPORT, viewport);
    if(onChangeOrAlways == ScaleAndShiftUpdateAlways ||
            prevXMin != xMin ||
            prevYMin != yMin ||
            prevXMax != xMax ||
            prevYMax != yMax ||
            prevZ != z ||
            prevWidth != viewport[2] ||
            prevHeight != viewport[3])
    {
        double wXMin;
        double wYMin;
        double wXMax;
        double wYMax;
        double  nodesWidth = xMax == xMin ? 1 : xMax - xMin;
        double nodesHeight = yMax == yMin ? 1 : yMax - yMin;
        GLdouble modelmatrix[16];
        GLdouble projmatrix[16];
        // Matrices and viewport to convert from world to screen
        // coordinates. 
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.0, 0.0, -20.0);
        glScalef(1.0, 1.0, manetAdj->scaleZ); // getting scale x and y so start with unity
        glRotatef(manetAdj->angleX, 1.0, 0.0, 0.0);
        glRotatef(manetAdj->angleY, 0.0, 1.0, 0.0);
        glRotatef(manetAdj->angleZ, 0.0, 0.0, 1.0);
        glTranslatef(0.0, 0.0, manetAdj->shiftZ + 3); // getting shift x and y so start with zero
        glGetDoublev(GL_MODELVIEW_MATRIX, modelmatrix);
        glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
        glPopMatrix();
        if(visibleDrawBoxAtZ(viewport, z, modelmatrix, projmatrix, nodesWidth/nodesHeight, &wXMin, &wYMin, &wXMax, &wYMax)) 
        {
            static time_t tick = 0;
            time_t now = time(0);
            // get shift and scale
            manetAdj->shiftX = ((wXMin + wXMax) / 2) - ((xMin + xMax) / 2);
            manetAdj->shiftY = ((wYMin + wYMax) / 2) - ((yMin + yMax) / 2);
            manetAdj->scaleX = (wXMax - wXMin) / nodesWidth;
            manetAdj->scaleY = (wYMax - wYMin) / nodesHeight;
            if(manetAdj->scaleX > manetAdj->scaleY)
                manetAdj->scaleX = manetAdj->scaleY;
            else
                manetAdj->scaleY = manetAdj->scaleX;

            BackgroundImage &bgImage=BackgroundImage::getInstance(); 
            if (bgImage.centerImage())
            {
                GLfloat x,y,z,w,h;
                bgImage.getDrawingCoords(x,w,y,h,z);
                bgImage.setDrawingCoords(manetAdj->shiftX, w, manetAdj->shiftY, h, z); 
                bgImage.centerImage(false); 
            }
        }
        prevXMin = xMin;
        prevYMin = yMin;
        prevXMax = xMax;
        prevYMax = yMax;
        prevZ = z;
        prevWidth = viewport[2];
        prevHeight = viewport[3];
    }
    else
    {
        ++nccnt;
    }
    {
        static time_t tick = 0;
        time_t now = time(0);
        if(now > tick)
        {
            if(nccnt)
                nccnt = 0;
            if(upfcnt)
                upfcnt = 0;
            tick = now + 5;
        }
    }
} // scaleAndShiftToSee

//
// Scale and shift the Manet to the center of the Manet screen
//
// onChangeOrAlways - ScaleAndShiftUpdateOnChange to only update scale
//                    and shift values if things have changed enough,
//                    ScaleAndShiftUpdateAlways to update even if things
//                    haven't changed at all Usually will not update
//                    scaling or shifting if nothing has changed
//
void manetGLView::scaleAndShiftToCenter(ScaleAndShiftUpdate onChangeOrAlways)
{
    double xMin = DBL_MAX;
    double xMax = -DBL_MAX;
    double yMin = DBL_MAX;
    double yMax = -DBL_MAX;
    double zMin = DBL_MAX;
    bool includeAntenna = isActive(ANTENNARADIUS_LAYER); 
    // bool includeHierarchy = isActive(HIERARCHY_LAYER); 

    // find drawing extents
    graph_traits<Graph>::vertex_iterator vi, vend;
    for(tie(vi, vend)=vertices(theGraph); vi!=vend; ++vi)
    {
        WatcherGraphNode &n=theGraph[*vi]; 
        GLdouble x, y, z; 
        gps2OpenGLPixels(n.gpsData->x, n.gpsData->y, n.gpsData->z, x, y, z); 

        double r = 0;
        if(includeAntenna)
            r = antennaRadius; 

        // if(includeHierarchy)
        // {
        //     double hr = HIERARCHY_RADIUS(m->nlist[i].level);
        //     if(r < hr)
        //     {
        //         r = hr;
        //     }
        // }
        {
            double nodeXMin = x - r;
            double nodeXMax = x + r;
            double nodeYMin = y - r;
            double nodeYMax = y + r;
            if(nodeXMin < xMin) xMin = nodeXMin;
            if(nodeXMax > xMax) xMax = nodeXMax;
            if(nodeYMin < yMin) yMin = nodeYMin;
            if(nodeYMax > yMax) yMax = nodeYMax;
        }
        if(z < zMin)
            zMin = z;
    }
    scaleAndShiftToSeeOnManet(xMin, yMin, xMax, yMax, zMin, onChangeOrAlways);
} // scaleAndShiftToCenter

void manetGLView::getShiftAmount(GLdouble &x_ret, GLdouble &y_ret)
{
    GLdouble z;
    int i;
    GLdouble modelmatrix[16];
    GLdouble projmatrix[16];
    GLint viewport[4];
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -20.0);
    glScalef(manetAdj->scaleX, manetAdj->scaleY, manetAdj->scaleZ);
    glRotatef(manetAdj->angleX, 1.0, 0.0, 0.0);
    glRotatef(manetAdj->angleY, 0.0, 1.0, 0.0);
    glRotatef(manetAdj->angleZ, 0.0, 0.0, 1.0);
    glTranslatef(manetAdj->shiftX, manetAdj->shiftY, manetAdj->shiftZ + 3);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelmatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glPopMatrix();

    if(globalManet)
    {
        for(i = 0; ; ++i)
        {
            if(i == globalManet->numnodes)
            {
                z = -20;
                break;
            }
            if(globalGpsValidFlag[i])
            {
                z = globalManet->nlist[i].z;
                break;
            }
        }
    }
    else
    {
        z = -20;
    }
    {
        int xmid = viewport[2] / 2;
        int ymid = viewport[3] / 2;
        int dx = viewport[2] / 20; // shift by 1/20th the screen
        int dy = viewport[3] / 20; // shift by 1/20th the screen
        XYWorldZToWorldXWorldY xyz[2] =
        {
            { xmid, ymid, z, 0, 0 },
            { xmid + dx, ymid - dy, z, 0, 0 },
        };
        if(xyAtZForModelProjViewXY( xyz, sizeof(xyz) / sizeof(xyz[0]), modelmatrix, projmatrix, viewport) ==  0)
        {
            x_ret = xyz[0].worldX_ret - xyz[1].worldX_ret;
            y_ret = xyz[1].worldY_ret - xyz[2].worldY_ret;
        }
        else
        {
            x_ret = 20.;
            y_ret = 20.;
        }
    }
    return;
}

void manetGLView::shiftBackgroundCenterLeft(double dx)
{
    GLfloat x, y, w, h, z;
    BackgroundImage &bg=BackgroundImage::getInstance();
    bg.getDrawingCoords(x, w, y, h, z);
    bg.setDrawingCoords(x+dx, w, y, h, z);
}

void manetGLView::shiftBackgroundCenterUp(double dy)
{
    GLfloat x, y, w, h, z;
    BackgroundImage &bg=BackgroundImage::getInstance();
    bg.getDrawingCoords(x, w, y, h, z);
    bg.setDrawingCoords(x, w, y+dy, h, z);
}

void manetGLView::shiftCenterRight()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterRight(shift);
}
void manetGLView::shiftCenterRight(double shift)
{
    manetAdj.shiftX -= shift;
    globalAutoCenterNodesFlag=0;
} 

void manetGLView::shiftCenterLeft()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterLeft(shift);
}
void manetGLView::shiftCenterLeft(double shift)
{
    manetAdj.shiftX += shift;
    globalAutoCenterNodesFlag=0;
} 

void manetGLView::shiftCenterDown()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterDown(shift);
}
void manetGLView::shiftCenterDown(double shift)
{
    manetAdj.shiftY += shift;
    globalAutoCenterNodesFlag=0;
} 

void manetGLView::shiftCenterUp()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterUp(shift);
}
void manetGLView::shiftCenterUp(double shift)
{
    manetAdj.shiftY -= shift;
    globalAutoCenterNodesFlag=0;
} 

void manetGLView::shiftCenterIn()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterIn(shift);
}
void manetGLView::shiftCenterIn(double shift)
{
    manetAdj.shiftZ -= shift;
    globalAutoCenterNodesFlag=0;
} 

void manetGLView::shiftCenterOut()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterOut(shift);
}
void manetGLView::shiftCenterOut(double shift)
{
    manetAdj.shiftZ += shift;
    globalAutoCenterNodesFlag=0;
} 

void manetGLView::viewpointReset(void)
{
    manetAdj = manetAdjInit;
}

void manetGLView::zoomOut()
{
    manetAdj.scaleX /= 1.05;
    if (manetAdj.scaleX < 0.001) 
        manetAdj.scaleX = 0.001;
    manetAdj.scaleY = manetAdj.scaleX;
    globalAutoCenterNodesFlag = 0;
}

void manetGLView::zoomIn()
{
    manetAdj.scaleX *= 1.05;
    manetAdj.scaleY = manetAdj.scaleX;
    globalAutoCenterNodesFlag = 0;
}

void manetGLView::compressDistance()
{
    manetAdj.scaleZ -= 0.1;
    if (manetAdj.scaleZ < 0.02)
    {
        manetAdj.scaleZ = 0.02;
    }
    if(globalAutoCenterNodesFlag && globalManet)
    {
        scaleAndShiftToCenter(ScaleAndShiftUpdateAlways);
    }
} 

void manetGLView::expandDistance()
{
    manetAdj.scaleZ += 0.1;
} 

#define TEXT_SCALE 0.08
#define TEXT_SCALE_ZOOM_FACTOR 1.05
#define ARROW_SCALE_ZOOM_FACTOR 1.05

void manetGLView::textZoomReset(void)
{
    scaleText[NODE_DISPLAY_MANET] = 0.08;
}

void manetGLView::textZoomIn(void)
{
    scaleText[NODE_DISPLAY_MANET] *= TEXT_SCALE_ZOOM_FACTOR;
}

void manetGLView::textZoomOut(void)
{
    scaleText[NODE_DISPLAY_MANET] /= TEXT_SCALE_ZOOM_FACTOR;
}

void manetGLView::arrowZoomReset(void)
{
    scaleLine[NODE_DISPLAY_MANET] = 1.0;
}

void manetGLView::arrowZoomIn(void)
{
    scaleLine[NODE_DISPLAY_MANET] *= ARROW_SCALE_ZOOM_FACTOR;
}

void manetGLView::arrowZoomOut(void)
{
    scaleLine[NODE_DISPLAY_MANET] /= ARROW_SCALE_ZOOM_FACTOR;
}

void manetGLView::rotateX(float deg)
{
    manetAdj.angleX += deg;
    while(manetAdj.angleX >= 360.0)
    {
        manetAdj.angleX -= 360.0;
    }
    while(manetAdj.angleX < 0)
    {
        manetAdj.angleX += 360.0;
    }
    if(globalAutoCenterNodesFlag && globalManet)
    {
        scaleAndShiftToCenter(ScaleAndShiftUpdateAlways);
    }
} 

void manetGLView::rotateY(float deg)
{
    manetAdj.angleY += deg;
    while(manetAdj.angleY >= 360.0)
    {
        manetAdj.angleY -= 360.0;
    }
    while(manetAdj.angleY < 0)
    {
        manetAdj.angleY += 360.0;
    }
    if(globalAutoCenterNodesFlag && globalManet)
    {
        scaleAndShiftToCenter(ScaleAndShiftUpdateAlways);
    }
} 

void manetGLView::rotateZ(float deg)
{
    manetAdj.angleZ += deg;
    while(manetAdj.angleZ >= 360.0)
    {
        manetAdj.angleZ -= 360.0;
    }
    while(manetAdj.angleZ < 0)
    {
        manetAdj.angleZ += 360.0;
    }
    if(globalAutoCenterNodesFlag && globalManet)
    {
        scaleAndShiftToCenter(ScaleAndShiftUpdateAlways);
    }
} 

void manetGLView::gps2openGLPixels(const double x, const double y, const double z, GLdouble &x, GLdouble &y, GLdouble &z) 
{
    TRACE_ENTER();

    // GTL - this should be done when the message is recv'd - not every time we need to 
    // compute the points - which is a whole hell of a lot of times.

    if (gps->dataFormat==GPSMessage::UTM)
    {
        //
        // There is no UTM zone information in the UTM GPS packet, so we assume all data is in a single
        // zone. Because of this, no attempt is made to place the nodes in the correct positions on the 
        // planet surface. We just use the "lat" "long" data as pure x and y coords in a plane, offset
        // by the first coord we get. (Nodes are all centered around 0,0 where, 0,0 is defined 
        // by the first coord we receive. 
        //
        if (gps->y < 91 && gps->x > 0) 
            LOG_WARN("Received GPS data that looks like lat/long in degrees, but GPS data format mode is set to UTM in cfg file."); 

        static double utmXOffset=0.0, utmYOffset=0.0;
        static bool utmOffInit=false;
        if (utmOffInit==false)
        {
            utmOffInit=true;
            utmXOffset=gps->x;
            utmYOffset=gps->y; 

            LOG_INFO("Got first UTM coordinate. Using it for x and y offsets for all other coords. Offsets are: x=" << utmXOffset << " y=" << utmYOffset);
        }

        x=gps->x-utmXOffset;
        y=gps->y-utmYOffset;    
        z=gps->z;

        LOG_DEBUG("UTM given locations: lon=" << location->lon << " lat=" << location->lat << " alt=" << location->alt);
        LOG_DEBUG("UTM node coords: x=" << us->x << " y=" << us->y << " z=" << us->z);
    }
    else // default to lat/long/alt WGS84
    {
        if (gps->x > 180)
            LOG_WARN("Received GPS data that may be UTM (long>180), but GPS data format mode is set to lat/long degrees in cfg file."); 

        x=gps->x*GPSScale;
        y=gps->y*GPSScale;
        z=gps->z-20;

        static double xOff=0.0, yOff=0.0;
        static bool xOffInit=false;
        if (xOffInit==false)
        {
            xOffInit=true;
            xOff=us->x;
            yOff=us->y;

            LOG_INFO("Got first Lat/Long coordinate. Using it for x and y offsets for all other coords. Offsets are: x=" 
                    << xOff << " y=" << yOff);
        }

        us->x-=xOff;
        us->y-=yOff;

        LOG_DEBUG("Got GPS: long:" << location->lon << " lat:" << location->lat << " alt:" << location->alt); 
        LOG_DEBUG("translated GPS: x:" << us->x << " y:" << us->y << " z:" << us->z); 
    }
    TRACE_EXIT();
}

manetGLView::manetGLView(QWidget *parent) : QGLWidget(parent)
{
    TRACE_ENTER();
    showPositionFlag=true;
    manetAdjInit = { 0.0, 0.0, 0.0, .035, .035, .03, 0.0, 0.0, 0.0 }; 
    messageStream=MessageStream::createNewMessageStream(serverName, service); 
    setFocusPolicy(Qt::StrongFocus); // tab and click to focus
    TRACE_EXIT();
}

manetGLView::~manetGLView()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void manetGLView::loadConfiguration()
{
    TRACE_ENTER();

    scaleText=0.08;
    scaleLine=1.0;
    monochromeMode = false;
    threeDView = true;
    backgroundImage = true;

    // causes goodinw control buttons to show/not show.
    emit runningGoodwin(false); 

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
    libconfig::Setting &layers=cfg.lookup(prop);

    struct 
    {
        const char *prop;
        GUILayer layer;
    } layerVals[] =
    {
        { "bandwidth", BANDWIDTH_LAYER },
        { "undefined", UNDEFINED_LAYER },
        { "neighbors", PHYSICAL_LAYER },
        { "hierarchy", HIERARCHY_LAYER },
        { "routing", ROUTING_LAYER },
        { "routingOneHop", ONE_HOP_ROUTING_LAYER },
        { "antennaRadius", ANTENNARADIUS_LAYER },
        { "sanityCheck", SANITY_CHECK_LAYER },
        { "anomPaths", ANOMPATHS_LAYER }, 
        { "correlation", CORROLATION_LAYER },
        { "alert", ALERT_LAYER }, 
        { "correlation3Hop", CORROLATION_3HOP_LAYER },
        { "wormholeRouting", ROUTING2_LAYER },
        { "wormholeRoutingOnehop", ROUTING2_ONE_HOP_LAYER },
        { "normPaths", NORMAL_PATHS_LAYER }
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

    // Give the GUI the current toggle state of the display.
    // GTL: to do - make all this stuff dynamic
    emit bandwidthToggled(isActive(BANDWIDTH_LAYER)); 
    emit undefinedToggled(isActive(UNDEFINED_LAYER)); 
    emit neighborsToggled(isActive(PHYSICAL_LAYER)); 
    emit hierarchyToggled(isActive(HIERARCHY_LAYER));
    emit routingToggled(isActive(ROUTING_LAYER)
            emit routingOnehopToggled(isActive(ONE_HOP_ROUTING_LAYER));
            emit antennaRadiusToggled(isActive(ANTENNARADIUS_LAYER));
            emit sanityCheckToggled(isActive(SANITY_CHECK_LAYER));
            emit anomPathsToggled(isActive(ANOMPATHS_LAYER)); 
            emit correlationToggled(isActive(CORROLATION_LAYER)); 
            emit alertToggled(isActive(ALERT_LAYER)); 
            emit correlation3HopToggled(isActive(CORROLATION_3HOP_LAYER)); 
            emit wormholeRoutingToggled(isActive(ROUTING2_LAYER)); 
            emit wormholeRoutingOnehopToggled(isActive(ROUTING2_ONE_HOP_LAYER)); 
            emit normPathsToggled(isActive(NORMAL_PATHS_LAYER)); 

            struct 
            {
            const char *prop; 
            bool default; 
            bool *val; 
            } boolVals[] = 
            {
            { "nodes3d", true, &threeDView },
            { "monochrome", false, &monochromeMode }, 
            { "displayBackgroundImage", true, &backgroundImage }
            }; 
            for (size_t i=0; i<sizeof(boolVals)/sizeof(boolVals[0]); i++)
            {
                prop=boolVals[i].prop;
                bool boolVal=boolVals[i].default; 
                if (root.lookupValue(prop, boolVal))
                    (*val)=boolVal; 
                else
                    root.add(prop, libconfig::Setting::TypeBoolean)=boolVal;
            }
            emit threeDViewToggled(threeDView);
            emit monochromeToggled(monochromeMode);
            emit backgroundImageToggled(backgroundImage);

            struct 
            {
                const char *prop; 
                float default; 
                float *val; 
            } floatVals[] = 
            {
                { "scaleText", 1.0, &scaleText }, 
                { "scaleLine", 1.0, &scaleLine }, 
                { "layerPadding", 1.0, &layerPadding }, 
                { "gpsScale", 80000.0, &gpsScale }, 
                { "antennaRadius", 200.0, &antennaRadius } 
            }; 
            for (size_t i=0; i<sizeof(floatVals)/sizeof(floatVals[0]); i++)
            {
                prop=floatVals[i].prop;
                float floatVal=floatVal[i].default; 
                if (root.lookupValue(prop, floatVal))
                    (*val)=floatVal; 
                else
                    root.add(prop, libconfig::Setting::TypeFloat)=boolVal;
            }

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
                    LOG_INFO("There is no gpsDataFormat argument in the cfg file, setting to default: lat-long-alt-WGS84")
                else
                    LOG_WARN("I don't understand the gpsDataFormat argument in the cfg file, \"" << strVal << "\", setting to default: lat-long-alt-WGS84")

                        legacyWatcher::setGPSDataFormat(legacyWatcher::GPS_DATA_FORMAT_DEG_WGS84);
            }

            //
            // Load viewpoint
            //
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
                { "angle", { &manetAdj.angleX, &manetAdj.angleY, &manetAdj.angleZ }},
                { "scale", { &manetAdj.scaleX, &manetAdj.scaleY, &manetAdj.scaleZ }},
                { "shift", { &manetAdj.shiftX, &manetAdj.shiftY, &manetAdj.shiftZ }}
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

            LOG_INFO("Set viewpoint - angle: " << manetAdj.angleX << ", " << manetAdj.angleY << ", " << manetAdj.angleZ);
            LOG_INFO("Set viewpoint - scale: " << manetAdj.scaleX << ", " << manetAdj.scaleY << ", " << manetAdj.scaleZ);
            LOG_INFO("Set viewpoint - shift: " << manetAdj.shiftX << ", " << manetAdj.shiftY << ", " << manetAdj.shiftZ);

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

            glClearColor(bgColors[0].val, bgColors[1].val,bgColors[2].val,bgColors[3].val);

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
    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_BLEND);
    // // glEnable(GL_LINE_SMOOTH);
    // // glEnable(GL_CULL_FACE); 
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glShadeModel(GL_SMOOTH);

    // glEnable(GL_LIGHTING);

    // // GLfloat ambLight[] = { 0.25, 0.25, 0.25, 1.0 };    // Ambient light is not full strength, but white
    // // glLightfv(GL_LIGHT1, GL_AMBIENT, ambLight);
    // // glLightfv(GL_LIGHT1, GL_SPECULAR, ambLight);

    // // GLfloat diffLight[]= { 1.0f, 1.0f, 1.0f, 1.0f };    // diffuse light is full strength
    // // glLightfv(GL_LIGHT1, GL_DIFFUSE, diffLight);

    // // GLfloat posLight[]= { 200.0f, 200.0f, 200.0f, 1.0f }; // light is over right shoulder
    // // glLightfv(GL_LIGHT1, GL_POSITION, posLight);

    // // glEnable(GL_LIGHT1);

    // enable lighting
    glEnable(GL_LIGHTING); 

    // enable objects behind one another
    glEnable(GL_DEPTH_TEST);

    // Allow colors to bleed through by alpha (orange edges when nieghor and one hop routing draw the same link).
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set the shading model
    glShadeModel(GL_SMOOTH); 

    // Uncomment these if you want to use glColor(c) instead of 
    // glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c);
    // glEnable(GL_COLOR_MATERIAL);
    // glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Spec is LIGHT0
    // GLfloat specular[]={0.5, 0.5, 0.5, 1.0};
    // GLfloat posSpecLight[]= { -500.0f, 500.0f, 100.0f, 1.0f };
    // glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    // glLightfv(GL_LIGHT0, GL_POSITION, posSpecLight);
    // glEnable(GL_LIGHT0); 

    // Amb is LIGHT1
    GLfloat ambLight[] = { 0.25, 0.25, 0.25, 1.0 };
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambLight);
    glEnable(GL_LIGHT1);

    // or use the ambient light model
    // GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    // glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    // Diffuse is LIGHT2
    GLfloat diffLight[]= { 0.5, 0.5, 0.5, 1.0f };    
    glLightfv(GL_LIGHT2, GL_DIFFUSE, diffLight);
    GLfloat posDiffLight[]= { 500.0, 500.0f, 100.0f, 1.0f };
    glLightfv(GL_LIGHT2, GL_POSITION, posDiffLight); 
    glEnable(GL_LIGHT2);

    TRACE_EXIT();
}

void manetGLView::checkIO()
{
    TRACE_ENTER();

    while(messageStream->isStreamReadable())
    {
        MessagePtr message;
        messageStream->getNextMessage(message);
        LOG_DEBUG("Got message: " << *message);
        theGraph.updateGraph(message);
    }

    theGraph.doMaintanence(); // check expiration, etc. 

    updateGL();  // redraw

    TRACE_EXIT();
}

void manetGLView::watcherIdle()
{
    TRACE_ENTER();

    bool update=false;
    Timestamp now=(Timestamp(time(NULL)))*1000; 

    graph_traits<Graph>::vertex_iterator vi, vend;
    for(tie(vi, vend)=vertices(theGraph); vi!=vend; ++vi)
    {
        WatcherGraphNode &node=theGraph[*vi]; 

        if (node.displayInfo->spin && now > node.displayInfo->nextSpinUpdate)
        {
            node.displayInfo->spinRotation_x+=node.displayInfo->spinIncrement;
            node.displayInfo->spinRotation_y+=node.displayInfo->spinIncrement;
            node.displayInfo->spinRotation_z+=node.displayInfo->spinIncrement;
            node.displayInfo->nextSpinUpdate=curtime+node.displayInfo->spinTimeout;
            update=true;
        }

        // Are we flashing and do we need to invert the color?
        if (node.displayInfo->flash && curtime > node.displayInfo->nextFlashUpdate)  
        {
            node.displayInfo->isFlashed=node.displayInfo->isFlashed?true:false;
            node.displayInfo->nextFlashUpdate=now+node.displayInfo->flashInterval;
            update=true;
        }
    }

    graph_traits<Graph>::edge_iterator ei, eend;
    for(tie(ei, eend)=edges(theGraph); ei!=eend; ++ei)
    {
        WatcherGraphEdge &edge=theGraph[*ei]; 
        if (edge.displayInfo->flash && curtime > edge.displayInfo->nextFlashUpdate)
        {
            edge.displayInfo->isFlashed=edge.displayInfo->isFlashed?true:false;
            edge.displayInfo->nextFlashUpdate=now+edge.displayInfo->flashInterval;
            update=true;
        }
    }

    if (update)
        updateGL();

    TRACE_EXIT();
}

void manetGLView::paintGL()
{
    TRACE_ENTER();
    drawManet();
    TRACE_EXIT();
}

void manetGLView::drawManet(void)
{
    // watcher::Skybox *sb=watcher::Skybox::getSkybox();
    // if (sb)
    //     sb->drawSkybox(manetAdj.angleX, manetAdj.angleY, manetAdj.angleZ);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -20.0);

    glPushMatrix();

    GLfloat black[]={0.0,0.0,0.0,1.0};
    GLfloat blue[]={0.0,0.0,1.0,0.6};

    // draw timestamp at bottom of screen
    date now(second_clock::local_time().date()); 
    glScalef(0.02, 0.02, 0.02);
    if (monochromeMode)
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
    else
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
    drawText(-400, -360, 0, scaleText, now.to_simple_string_type().c_str(), 1.0);

    if (showPositionFlag)
    {
        char buff[160];
        snprintf(buff, sizeof(buff), "Location: %3.1f, %3.1f, %3.1f scale %f",
                manetAdj.shiftX, manetAdj.shiftY, manetAdj.shiftZ, manetAdj.scaleX);
        drawText(-200, -360, 0, scaleText, buff, 1.0);
    }

    glPopMatrix();

    glScalef(manetAdj.scaleX, manetAdj.scaleY, manetAdj.scaleZ);
    glRotatef(manetAdj.angleX, 1.0, 0.0, 0.0);
    glRotatef(manetAdj.angleY, 0.0, 1.0, 0.0);
    glRotatef(manetAdj.angleZ, 0.0, 0.0, 1.0);
    glTranslatef(manetAdj.shiftX, manetAdj.shiftY, manetAdj.shiftZ);

    for (LayerList::iterator li=knownLayers.begin(); li!=knownLayers.end(); ++li)
    {
        if (li->active)
        {
            // each layer is 'layerPadding' above the rest. 
            // layer order is currently defined by order of appearance
            // in the cfg file. 
            glPushMatrix();
            glTranslatef(0.0, 0.0, layerPadding);  
            drawLayer(li->layer); 
            glPopMatrix();

        }
    }

    if (backgroundImage)
        BackgroundImage::getInstance().drawImage(); 

    glFlush();
}

void manetGLView::drawEdge(const WatcherGraphEdge &edge)
{
    TRACE_ENTER(); 

    GLdouble x1, y1, z1, x2, y2, z2, width;
    x1=edge.node1.x; 
    y1=edge.node1.y; 
    z1=edge.node1.z; 
    x2=edge.node2.x; 
    y2=edge.node2.y; 
    z2=edge.node2.z; 
    width=edge.displayInfo->width;

    GLfloat edgeColor[]={
        edge.displayInfo->color.r, 
        edge.displayInfo->color.g, 
        edge.displayInfo->color.b, 
        edge.displayInfo->color.a, 
    };

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, edgeColor); 

    if (!threeDView)
    {
        double ax=atan2(x1-x2,y1-y2);
        double cmx = sin(ax)*width;   // cos(a-M_PI_2)*width
        double cpx = -cmx;            // cos(a+M_PI_2)*width
        double smx = -cos(ax)*width;  // sin(a-M_PI_2)*width
        double spx = -smx;            // cos(a+M_PI_2)*width

        glBegin(GL_POLYGON);

        glNormal3f(0.0, 0.0, 1.0);
        glVertex3f(x1+smx,y1+cmx,z1);
        glVertex3f(x1+spx,y1+cpx,z1);
        glVertex3f(x2+spx,y2+cpx,z2);
        glVertex3f(x1+smx,y1+cmx,z1);

        glEnd();
    }
    else
    {
        glPushMatrix();
        glTranslatef(x1,y1,z1);

        // gluCylinder draws "out the z axis", so rotate view 90 on the y axis and angle-between-the-nodes on the x axis
        // before drawing the cylinder

        glRotated(90.0, 0.0, 1.0, 0.0);                             // y rotate
        glRotated(atan2(y1-y2,x2-x1)*(180/M_PI), 1.0, 0.0, 0.0);    // x rotate, y1-y2,x2-x1 was trail and error wrt the quadrants

        // glRotated(atan2(z1-z2,y2-y1)*(180/M_PI), 0.0, 1.0, 0.0);    // y rotate, y1-y2,x2-x1 was trail and error wrt the quadrants
        // glRotated(atan2(x1-x2,z2-z1)*(180/M_PI), 0.0, 0.0, 1.0);    // z rotate, y1-y2,x2-x1 was trail and error wrt the quadrants
        // float distance=sqrt(pow(x1-x2,2)+pow(y1-y2,2)+pow(z1-z2,2));
        float distance=sqrt(pow(x1-x2,2)+pow(y1-y2,2)+pow(z1-z2,2));

        GLUquadric *q=gluNewQuadric();
        gluCylinder(q, width, 0, distance, 10, 10); 
        gluDeleteQuadric(q);

        glPopMatrix(); 
    }

    // draw the edge's label, if there is one.
    if (edge.displayInfo.label)
    {
        if (monochromeMode)
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
        else
        {
            // This color should be cfg-erable.
            const GLfloat blue[]={0.0,0.0,1.0,0.6};
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
        }

        double lx=(x1+x2)/2.0; 
        double ly=(y1+y2)/2.0; 
        double lz=(z1+z2)/2.0; 
        double a=atan2(x1-x2 , y1-y2);
        double th=10.0;
        drawText(lx+sin(a-M_PI_2),ly+cos(a-M_PI_2)*th, (z1+z2)/2, scaleText, edge.displayInfo.label.c_str());
    }

    TRACE_EXIT(); 
}

bool manetGLView::isActive(const watcher::GUILayer &layer)
{
    TRACE_ENTER();
    LayerList::const_iterator li=find(layerList.begin(). layerList.end(), layer); 
    TRACE_EXIT();
    return li==listLayer.end() ? false : (*li)->active; 
}

void manetGLView::drawNode(const WatcherGraphNode &node)
{
    TRACE_ENTER(); 

    const GLfloat black[]={0.0,0.0,0.0,1.0};
    GLfloat nodeColor[]={
        node.diplayInfo->color.r/255.0, 
        node.diplayInfo->color.g/255.0, 
        node.diplayInfo->color.b/255.0, 
        node.diplayInfo->color.a/255.0, 
    };

    GLdouble x, y, z; 
    gps2openGLPixels(node.gpsData.x, node.gpsData.y, node.gpsData.z, x, y, z); 

    // flashing color: if flashed, invert color. 
    for(unsigned int i=0; i<sizeof(nodeColor); i++) 
        if (node.displayInfo->flash && node.displayInfo->isFlashed)
            nodeColor[i]=1-nodeColor[i]; 

    if (dispStat->monochromeMode)
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
    else
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, nodeColor);

    switch(node.displayInfo.shape)
    {
        case NodeDisplayInfo::CIRCLE: drawCircle(x, y, z, 4); break;
        case NodeDisplayInfo::SQUARE: drawCube(x, y, z); break;
        case NodeDisplayInfo::TRIANGLE: drawPyramid(x, y, z); break;
        case NodeDisplayInfo::TORUS: drawTorus(x, y, z); break;
        case NodeDisplayInfo::TEAPOT: drawTeapot(x, y, z); break;
    }

    if (isActive(ANTENNARADIUS_LAYER))
        drawWireframeSphere(x, y, z, antennaRadius); 

    TRACE_EXIT(); 
}

void manetGLView::drawNodeLabel(const WatcherGraphNode &node, const float x, const float y, const float z)
{
    TRACE_ENTER();

    const GLfloat black[]={0.0,0.0,0.0,1.0};
    const GLfloat nodeColor[]={
        node.diplayInfo->color.r/255.0, 
        node.diplayInfo->color.g/255.0, 
        node.diplayInfo->color.b/255.0, 
        node.diplayInfo->color.a/255.0, 
    };
    if (monochromeMode)
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
    else
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, nodeColor);

    // a little awkward since we're mixing enums, reserved strings, and free form strings
    char buf[64]; 
    if (!node.nodeId.is_v4())
        snprintf(buf, sizeof(buf), "%s", node.nodeId.to_string());  // punt
    else
    {
        unsigned long addr=node.nodeId.to_ulong(); // host byte order. 

        if (node.displayInfo.label==NodeDisplayInfo::labelDefault2String(FOUR_OCTETS))
            snprintf(buf, sizeof(buf), "%d.%d.%d.%d", ((addr)>>24)&0xFF,((addr)>>16)&0xFF,((addr)>>8)&0xFF,(addr)&0xFF); 
        else if (node.displayInfo.label==NodeDisplayInfo::labelDefault2String(THREE_OCTETS))
            snprintf(buf, sizeof(buf), "%d.%d.%d", ((addr)>>16)&0xFF,((addr)>>8)&0xFF,(addr)&0xFF); 
        else if (node.displayInfo.label==NodeDisplayInfo::labelDefault2String(TWO_OCTETS))
            snprintf(buf, sizeof(buf), "%d.%d", ((addr)>>8)&0xFF,(addr)&0xFF); 
        else if (node.displayInfo.label==NodeDisplayInfo::labelDefault2String(LAST_OCTET))
            snprintf(buf, sizeof(buf), "%d", (addr)&0xFF); 
        else if (node.displayInfo.label==NodeDisplayInfo::labelDefault2String(HOSTNAME))
        {
            struct in_addr saddr; 
            saddr.s_addr=htonl(addr); 
            struct hostent *he=gethostbyaddr((const void *)saddr.s_addr, sizeof(saddr.s_addr), AF_INT); 

            if (hostEnt) 
            {
                snprintf(buf, sizeof(buf), "%s", he->h_name); 
                // only do the lookup one time successfully per host. 
                node.displayInfo.label=buf; 
            }
        }
        else
            snprintf(buf, sizeof(buf), "%s", node.nodeId.to_string());  // use what is ever there. 
    }        

    drawText(x, y+6, z+5, scaleText, buf); 

    TRACE_EXIT();
}

void manetGLView::drawLabel(GLfloat x, GLfloat y, GLfloat z, const LabelDisplayInfoPtr &label)
{
    // 
    // GTL ----
    // This code is fundementally broken for more than one label. 
    // Look at nodeDrawLabel in graphics.cpp. 
    // You need to know *all* the labels that will be drawn to 
    // get everything to line up properly. 
    //
    TRACE_ENTER(); 

    GLfloat w=0.0; 
    yOffset+=drawTextHeight(label->labelText.c_str(), scaleText); 
    GLfloat t=drawTextWidth(label->labelText.c_str(), scaleText); 
    if (t>w)
        w=t;

    GLfloat fgColor[]={
        label->foregroundColor.r, 
        label->foregroundColor.g, 
        label->foregroundColor.b, 
        label->foregroundColor.a
    }
    GLfloat bgColor[]={
        label->backgrounColor.r, 
        label->backgrounColor.g, 
        label->backgrounColor.b, 
        label->backgrounColor.a
    }
    GLfloat x=nodex+6;
    GLfloat y=nodey-6;
    GLfloat z=nodez+0.3+(0.3*(us->index & 0xFF));
    static const GLfloat black[]={0.0,0.0,0.0,1.0};

#define TEXT_SCALE 0.08
    GLfloat border_width = 2.0*(scaleText > TEXT_SCALE ? sqrt(scaleText/TEXT_SCALE) : scaleTextTEXT_SCALE);
    GLfloat border_width2 = border_width + border_width;
    h+=border_width2;
    w+=border_width2;

    // "pointer" from node to label
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(nodex,nodey,nodez);
    glVertex3f(x+w*0.03,y,z);
    glVertex3f(x,y,z);
    glVertex3f(x,y,z);
    glVertex3f(x,y-h*0.03,z);
    glEnd();

    // border around label
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
    glBegin(GL_LINE_LOOP);
    glVertex3f(x,y,z+0.2);
    glVertex3f(x+w,y,z+0.2);
    glVertex3f(x+w,y-h,z+0.2);
    glVertex3f(x,y-h,z+0.2);
    glEnd();

    GLfloat localh=drawTextHeight(label->labelTtext.c_str()*scaleText); 

    if (monochromeMode)
        for (unsigned int i=0; i<sizeof(bgColor)/sizeof(bgColor[0]); i++)
            bgColor[i]=1.0;
    else
        for (unsigned int i=0; i<sizeof(bgColor)/sizeof(bgColor[0]); i++)
            bgColor[i]=bgcolor[i]/255.0;

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bgcolor);
    glBegin(GL_POLYGON);
    glVertex3f(x  ,y,z+0.1);
    glVertex3f(x+w,y,z+0.1);
    glVertex3f(x+w,y-localh-0.5,z+0.1);
    glVertex3f(x  ,y-localh-0.5,z+0.1);
    glEnd();

    if (dispStat->monochromeMode)
    {
        fgcolor[0]=0.0;
        fgcolor[1]=0.0;
        fgcolor[2]=0.0;
        fgcolor[3]=1.0;
    }
    else
        for (unsigned int i=0; i<sizeof(fgColor)/sizeof(fgColor[0]); i++)
            fgColor[i]=fgcolor[i]/255.0;

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, fgcolor);
    y-=drawTextHeight("X")*scaleText;
    drawText(x+border_width,y-border_width,z+0.2, scaleText, label->labelText.c_str()); 
    y-=localh-drawTextHeight("X")*scaleText;

    /* Fill in gap at the bottom of the label...  */
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bgcolor);
    glBegin(GL_POLYGON);
    glVertex3f(x  ,y,z+0.1);
    glVertex3f(x+w,y,z+0.1);
    glVertex3f(x+w,y-border_width2,z+0.1);
    glVertex3f(x  ,y-border_width2,z+0.1);
    glEnd();

    TRACE_EXIT(); 
}

void manetGLView::drawLayer(const GUILayer &layer)
{
    TRACE_ENTER();

    graph_traits<Graph>::edge_iterator ei, eend;
    for(tie(ei, eend)=edges(theGraph); ei!=eend; ++ei)
    {
        WatcherGraphEdge &edge=theGraph[*ei]; 
        if (edge.displayInfo.layer==layer)
            drawEdge(edge);

        WatcherGraphEdge::LabelList::iterator li=edge.labels.begin(); 
        WatcherGraphEdge::LabelList::iterator lend=edge.labels.end(); 
        for( ; li!=lend; ++li)
            if ((*li)->displayInfo.layer==layer)
            {
                graph_traits<Graph>::vertex_descriptor n1=source(ei, theGraph); 
                graph_traits<Graph>::vertex_descriptor n2=target(ei, theGraph); 

                float lx=(n1.x+n2.x)/2.0; 
                float ly=(n1.y+n2.y)/2.0; 
                float lz=(n1.z+n2.z)/2.0; 

                GLfloat x, y, z; 
                gps2openGLPixels(lx, ly, lz, x, y, z); 
                drawLabel(*i, x, y, z);
            }
    }

    graph_traits<Graph>::vertex_iterator vi, vend;
    for(tie(vi, vend)=vertices(theGraph); vi!=vend; ++vi)
    {
        WatcherGraphNode &node=theGraph[*vi]; 

        if (node.displayInfo.layer==layer)
            drawNode(node); 

        WatcherGraphEdge::LabelList::iterator li=node.labels.begin(); 
        WatcherGraphEdge::LabelList::iterator lend=node.labels.end(); 
        for( ; li!=lend; ++li)
            if ((*li)->displayInfo.layer==layer)
            {
                GLfloat x, y, z; 
                gps2openGLPixels(lx, ly, lz, x, y, z); 
                drawLabel(*li, x, y, z); 
            }
    }

    TRACE_EXIT();
}

void manetGLView::resizeGL(int width, int height)
{
    TRACE_ENTER();
    // GLint viewport[4];
    // glGetIntegerv(GL_VIEWPORT, viewport);
    // fprintf(stderr, "Reshape cur (%d, %d)\n", viewport[2], viewport[3]);
    // fprintf(stderr, "Reshape given (%d, %d)\n", awidth, aheight);
    glViewport(0, 0, awidth, aheight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0, GLfloat(awidth) / GLfloat(aheight), 1.0, 50.0);
    if(autoCenterNodesFlag) 
        scaleAndShiftToCenter(ScaleAndShiftUpdateOnChange);

    TRACE_EXIT();
}

void manetGLView::resetPosition()
{
    TRACE_ENTER();
    manetAdj = manetAdjInit;
    updateGL();
    TRACE_EXIT();
}

void manetGLView::keyPressEvent(QKeyEvent * event)
{
    TRACE_ENTER();

    quint32 nativeKey = event->nativeVirtualKey();
    int qtKey = event->key();

    if (legacyWatcher::Key(nativeKey) != 0)
        if (nativeKey == 'B' || nativeKey == 'b')
            emit bandwidthToggled(isActive(BANDWIDTH_LAYER)); 
    updateGL();
    if (nativeKey=='C')
    {
        LOG_DEBUG("Got cap C in keyPressEvent - spawning color chooser for background color"); 
        QRgb rgb=0xffffffff;
        bool ok=false;
        rgb=QColorDialog::getRgba(rgb, &ok);
        if (ok)
            glClearColor(qRed(rgb)/255.0, qGreen(rgb)/255.0, qBlue(rgb)/255.0, qAlpha(rgb)/255.0);
    }

    switch(qtKey)
    {
        case Qt::Key_Left:  shiftCenterRight(); break;
        case Qt::Key_Right: shiftCenterLeft(); break;
        case Qt::Key_Up:    shiftCenterDown(); break;
        case Qt::Key_Down:  shiftCenterUp(); break;
        case Qt::Key_N:     shiftCenterIn(); break; 
        case Qt::Key_M:     shiftCenterOut(); break;
        case Qt::Key_Q:     zoomOut(); break;
        case Qt::Key_W:     zoomIn(); break;
        case Qt::Key_A:     textZoomOut(); break;
        case Qt::Key_S:     textZoomIn(); break;
        case Qt::Key_Z:     compressDistance(); break;
        case Qt::Key_X:     expandDistance(); break;

        case Qt::Key_E:     rotateX(-5.0); break;
        case Qt::Key_R:     rotateX(5.0); break;
        case Qt::Key_D:     rotateY(-5.0); break;
        case Qt::Key_F:     rotateY(5.0); break;
        case Qt::Key_C:     rotateZ(-5.0); break;
        case Qt::Key_V:     rotateZ(5.0); break;
        case Qt::Key_Equal:
        case Qt::Key_Plus: 
                            globalAutoCenterNodesFlag = 1; 
                            scaleAndShiftToCenter(ScaleAndShiftUpdateAlways);
                            globalAutoCenterNodesFlag = 0; 
                            break;
        case Qt::Key_Space:
                            globalReplay.runFlag = !globalReplay.runFlag;
                            if (globalReplay.runFlag)
                                messageStream.startStream();
                            else
                                messageStream.stopStream();
                            break;

                            // GTL TODO: add shortcuts for ff/rew, etc. 
                            // case 't': globalReplay.step = 1000; break;
                            // case 'a' - 'a' + 1: arrowZoomOut(); break;
                            // case 's' - 'a' + 1: arrowZoomIn(); break;
                            // case 'r' - 'a' + 1: textZoomReset(); arrowZoomReset(); viewpointReset(); break;

        default:
                            event->ignore();
    }

    updateGL();

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

void manetGLView::layerToggle(const Layer layer, const bool turnOn)
{
    TRACE_ENTER();

    LayerList::iterator li=find(layerList.begin(), layerList.end(), layer); 
    if (li!=layerList.end())
        (*li)->active=turnOn); 
    else
        LOG_WARN("Attempt to toggle non-existent layer " << layer); 

    TRACE_EXIT();
}

void manetGLView::toggleBandwidth(bool isOn)
{
    TRACE_ENTER();
    layerToggle(BANDWIDTH_LAYER, isOn); 
    emit bandwidthToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}

void manetGLView::toggleUndefined(bool isOn)
{
    TRACE_ENTER();
    layerToggle(UNDEFINED_LAYER, isOn); 
    emit undefinedToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleNeighbors(bool isOn)
{
    TRACE_ENTER();
    layerToggle(PHYSICAL_LAYER, isOn); 
    emit neighborsToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleHierarchy(bool isOn)
{
    TRACE_ENTER();
    layerToggle(HIERARCHY_LAYER, isOn); 
    emit hierarchyToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleRouting(bool isOn)
{
    TRACE_ENTER();
    layerToggle(ROUTING_LAYER, isOn); 
    emit routingToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleRoutingOnehop(bool isOn)
{
    TRACE_ENTER();
    layerToggle(ONE_HOP_ROUTING_LAYER, isOn); 
    emit routingOnehopToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleAntennaRadius(bool isOn)
{
    TRACE_ENTER();
    layerToggle(ANTENNARADIUS_LAYER, isOn); 
    emit antennaRadiusToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleSanityCheck(bool isOn)
{
    TRACE_ENTER();
    layerToggle(SANITY_CHECK_LAYER, isOn); 
    emit sanityCheckToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleAnomPaths(bool isOn)
{
    TRACE_ENTER();
    layerToggle(ANOMPATHS_LAYER, isOn); 
    emit anomPathsToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleCorrelation(bool isOn)
{
    TRACE_ENTER();
    layerToggle(CORROLATION_LAYER, isOn); 
    emit correlationToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleAlert(bool isOn)
{
    TRACE_ENTER();
    layerToggle(ALERT_LAYER, isOn); 
    emit alertToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleCorrelation3Hop(bool isOn)
{
    TRACE_ENTER();
    layerToggle(CORROLATION_3HOP_LAYER, isOn); 
    emit correlation3HopToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleWormholeRouting(bool isOn)
{
    TRACE_ENTER();
    layerToggle(ROUTING2_LAYER, isOn); 
    emit wormholeRoutingToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleWormholeRoutingOnehop(bool isOn)
{
    TRACE_ENTER();
    layerToggle(ROUTING2_ONE_HOP_LAYER, isOn); 
    emit wormholeRoutingOnehopToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleNormPaths(bool isOn)
{
    TRACE_ENTER();
    layerToggle(NORMAL_PATHS_LAYER, isOn); 
    emit normPathsToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleMonochrome(bool isOn)
{
    TRACE_ENTER();
    monochromeMode=isOn;
    emit monochromeToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleThreeDView(bool isOn)
{
    TRACE_ENTER();
    threeDView=isOn;
    emit threeDViewToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleBackgroundImage(bool isOn)
{
    TRACE_ENTER();
    LOG_DEBUG("Turning background image " << (isOn==true?"on":"off")); 
    showBackground=isOn; 
    emit backgroundImageToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::clearAllEdges()
{
    TRACE_ENTER();
    graph_traits<Graph>::edge_iterator ei, eend;
    for(tie(ei, eend)=edges(theGraph); ei!=eend; ++ei)
        remove_edge(ei, theGraph);  // GTL does this blow away the label memory as well? 
    TRACE_EXIT();
}
void manetGLView::clearAllLabels()
{
    TRACE_ENTER();
    graph_traits<Graph>::edge_iterator ei, eend;
    for(tie(ei, eend)=edges(theGraph); ei!=eend; ++ei)
    {
        if (theGraph[*ei].labels.size())
            theGraph[*ei].labels.clear()
    }

    graph_traits<Graph>::vertex_iterator vi, vend;
    for(tie(vi, vend)=vertices(theGraph); vi!=vend; ++vi)
    {
        if (theGraph[*vi].labels.size())
            theGraph[*vi].labels.clear()
    }
    emit labelsCleared();
    TRACE_EXIT();
}
void manetGLView::playbackStart()
{
    TRACE_ENTER();
    messageStream.startStream(); 
    TRACE_EXIT();
}
void manetGLView::playbackStop()
{
    TRACE_ENTER();
    messageStream.stopStream(); 
    TRACE_EXIT();
}
void manetGLView::playbackPause()
{
    TRACE_ENTER();
    messageStream.stopStream(); 
    TRACE_EXIT();
}
void manetGLView::playbackContinue()
{
    TRACE_ENTER();
    messageStream.startStream(); 
    TRACE_EXIT();
}
void manetGLView::playbackSetSpeed(int x)
{
    TRACE_ENTER();
    messageStream.setStreamRate((float)x); 
    TRACE_EXIT();
}

// Should be called after a glTranslate()
void manetGLView::handleSpin(int threeD, NodeDisplayInfoPtr &ndi)
{
    if (ndi->spin)
    {
        if (threeD)
        {
            glRotatef(ndi->spinRotation_x, 1.0f, 0.0f, 0.0f);
            glRotatef(ndi->spinRotation_y, 0.0f, 1.0f, 0.0f);
        }
        glRotatef(ndi->spinRotation_z, 0.0f, 0.0f, 1.0f);
    }
}

// Should be called after a glTranslate()
void manetGLView::handleSize(NodeDisplayInfoPtr &ndi)
{
    glScalef(ndi->size, ndi->size, ndi->size);
}

void manetGLView::drawWireframeSphere( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, NodeDisplayInfoPtr &/*ndi*/)
{
    glPushMatrix();

    glTranslatef(x, y, z);

    if (threeDView)
    {
        glPushAttrib(GL_NORMALIZE);
        glNormal3f(0.0, 0.0, 1.0);
        glutWireSphere(radius, 10, 10);
        glPopAttrib();
    }
    else
    {
        GLUquadric* q=NULL;
        q=gluNewQuadric();
        gluDisk(q,radius-1,radius,36,1);
        gluDeleteQuadric(q);
    }
    glPopMatrix();
}

void manetGLView::drawPyramid( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, NodeDisplayInfoPtr &ndi)
{
    glPushMatrix();

    glTranslated(x, y, z);

    handleSize(ndi);
    handleSpin(threeDView, ndi);

    // fprintf(stdout, "Drawing triangle with \"radius\" : %f. x/y offset is %f\n", radius, offset); 

    if (threeDView)
    {
        glPushAttrib(GL_NORMALIZE);
        glScalef(9,9,9);        // Eyeballing it.

        glBegin(GL_TRIANGLES);
        {
            // Front
            glNormal3f( 0.0f, 0.0f, 1.0f);
            glVertex3f( 0.0f, 1.0f, 0.0f);
            glVertex3f(-1.0f,-1.0f, 1.0f);
            glVertex3f( 1.0f,-1.0f, 1.0f);
            // Right
            glNormal3f( 1.0f, 0.0f, 0.0f);
            glVertex3f( 0.0f, 1.0f, 0.0f);
            glVertex3f( 1.0f,-1.0f, 1.0f);
            glVertex3f( 1.0f,-1.0f,-1.0f);
            // Back
            glNormal3f( 0.0f, 0.0f,-1.0f);
            glVertex3f( 0.0f, 1.0f, 0.0f);
            glVertex3f( 1.0f,-1.0f,-1.0f);
            glVertex3f(-1.0f,-1.0f,-1.0f);
            // Left
            glNormal3f(-1.0f, 0.0f, 0.0f);
            glVertex3f( 0.0f, 1.0f, 0.0f);
            glVertex3f(-1.0f,-1.0f,-1.0f);
            glVertex3f(-1.0f,-1.0f, 1.0f);
        }
        glEnd();

        glBegin(GL_QUADS);
        {
            // Bottom
            glNormal3f( 0.0f,-1.0f, 0.0f);
            glVertex3f(-1.0f,-1.0f, 1.0f);
            glVertex3f( 1.0f,-1.0f, 1.0f);
            glVertex3f( 1.0f,-1.0f,-1.0f);
            glVertex3f(-1.0f,-1.0f,-1.0f);
        }
        glEnd();

        glPopAttrib();
    }
    else
    {
        GLfloat offset=2.0*radius*sin(M_PI_4);  // by law of sines

        glPushAttrib(GL_LINE_WIDTH);
        glLineWidth(2.0); 
        glNormal3f( 0.0f, 0.0f, 1.0f); 
        glBegin(GL_LINE_LOOP); 
        {
            glVertex2f(-offset, -offset);
            glVertex2f( offset, -offset); 
            glVertex2f(0, radius); 
        }
        glEnd();
        glPopAttrib();
    }
    glPopMatrix();
}

void manetGLView::drawCube(GLdouble x, GLdouble y, GLdouble z, GLdouble radius, NodeDisplayInfoPtr &ndi)
{
    glPushMatrix();
    glTranslated(x, y, z);

    handleSize(ndi);
    handleSpin(threeDView, ndi);

    GLfloat widthScaled=radius; 

    if (threeDView)
    {
        glPushAttrib(GL_NORMALIZE);
        glNormal3f(0.0, 0.0, 1.0);
        // I had this "easy" call to glutDrawSolidCube, but the shadows did not look as good as when I set the 
        // normal myself.
        //  glutSolidCube(widthScaled*2); 
        glScalef(9,9,9);        // Eyeballing it. - this 9 should be in a header somewhere.
        glBegin(GL_QUADS);
        // Front Face
        glNormal3f( 0.0f, 0.0f, 1.0f);                  // Normal Pointing Towards Viewer
        /* glTexCoord2f(0.0f, 0.0f); */ glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 1 (Front)
        /* glTexCoord2f(1.0f, 0.0f); */ glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 2 (Front)
        /* glTexCoord2f(1.0f, 1.0f); */ glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Front)
        /* glTexCoord2f(0.0f, 1.0f); */ glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 4 (Front)
        // Back Face
        glNormal3f( 0.0f, 0.0f,-1.0f);                  // Normal Pointing Away From Viewer
        /* glTexCoord2f(1.0f, 0.0f); */ glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Back)
        /* glTexCoord2f(1.0f, 1.0f); */ glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 2 (Back)
        /* glTexCoord2f(0.0f, 1.0f); */ glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 3 (Back)
        /* glTexCoord2f(0.0f, 0.0f); */ glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 4 (Back)
        // Top Face
        glNormal3f( 0.0f, 1.0f, 0.0f);                  // Normal Pointing Up
        /* glTexCoord2f(0.0f, 1.0f); */ glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 1 (Top)
        /* glTexCoord2f(0.0f, 0.0f); */ glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 2 (Top)
        /* glTexCoord2f(1.0f, 0.0f); */ glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Top)
        /* glTexCoord2f(1.0f, 1.0f); */ glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 4 (Top)
        // Bottom Face
        glNormal3f( 0.0f,-1.0f, 0.0f);                  // Normal Pointing Down
        /* glTexCoord2f(1.0f, 1.0f); */ glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Bottom)
        /* glTexCoord2f(0.0f, 1.0f); */ glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 2 (Bottom)
        /* glTexCoord2f(0.0f, 0.0f); */ glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 3 (Bottom)
        /* glTexCoord2f(1.0f, 0.0f); */ glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 4 (Bottom)
        // Right face
        glNormal3f( 1.0f, 0.0f, 0.0f);                  // Normal Pointing Right
        /* glTexCoord2f(1.0f, 0.0f); */ glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 1 (Right)
        /* glTexCoord2f(1.0f, 1.0f); */ glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 2 (Right)
        /* glTexCoord2f(0.0f, 1.0f); */ glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Right)
        /* glTexCoord2f(0.0f, 0.0f); */ glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 4 (Right)
        // Left Face
        glNormal3f(-1.0f, 0.0f, 0.0f);                  // Normal Pointing Left
        /* glTexCoord2f(0.0f, 0.0f); */ glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Left)
        /* glTexCoord2f(1.0f, 0.0f); */ glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 2 (Left)
        /* glTexCoord2f(1.0f, 1.0f); */ glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 3 (Left)
        /* glTexCoord2f(0.0f, 1.0f); */ glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 4 (Left)
        glEnd();                                // Done Drawing Quads

        glPopAttrib();
    }
    else
    {
        GLfloat offset=widthScaled;
        glLineWidth(2.0); 
        glBegin(GL_LINE_LOOP);
        glVertex2f(-offset, -offset);
        glVertex2f( offset, -offset);
        glVertex2f( offset,  offset);
        glVertex2f(-offset,  offset); 
        glEnd();
        glLineWidth(1.0); 
    }
    glPopMatrix();
}

void manetGLView::drawTeapot(GLdouble x, GLdouble y, GLdouble z, GLdouble radius, NodeDisplayInfoPtr &ndi)
{
    glPushMatrix();
    glTranslated(x, y, z);

    handleSize(ndi);
    handleSpin(threeDView, ndi);

    GLfloat widthScaled=radius; 

    if (threeDView)
    {
        glPushAttrib(GL_NORMALIZE);
        glNormal3f(0.0, 0.0, 1.0);
        glutSolidTeapot(widthScaled*2); 
        glPopAttrib();
    }
    else
    {
        // A flat teapot is just a square
        GLfloat offset=widthScaled;
        glLineWidth(2.0); 
        glBegin(GL_LINE_LOOP);
        glVertex2f(-offset, -offset);
        glVertex2f( offset, -offset);
        glVertex2f( offset,  offset);
        glVertex2f(-offset,  offset); 
        glEnd();
        glLineWidth(1.0); 
    }
    glPopMatrix();
}

void manetGLView::drawDisk( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, NodeDisplayInfoPtr &ndi)
{
    GLUquadric* q=NULL;

    glPushMatrix();

    glTranslatef(x, y, z);

    handleSize(ndi);
    handleSpin(threeDView, ndi);

    q=gluNewQuadric();
    gluDisk(q,radius-1,radius,36,1);
    gluDeleteQuadric(q);

    glPopMatrix();
}


void manetGLView::drawTorus(GLdouble x, GLdouble y, GLdouble z, GLdouble radius, NodeDisplayInfoPtr &ndi)
{
    GLfloat inner=radius-1;
    GLfloat outer=radius;

    if (threeDView)
    {
        glPushMatrix();
        glTranslated(x, y, z);
        handleSize(ndi);
        handleSpin(threeDView, ndi);

        glPushAttrib(GL_NORMALIZE);
        glNormal3f(0.0, 0.0, 1.0);
        glutSolidTorus(inner, outer, 10, 10);  
        glPopAttrib();
        glPopMatrix();
    }
    else
    {
        GLUquadric* q=NULL;
        glPushMatrix();
        glTranslatef(x, y, z);
        handleSize(ndi);
        handleSpin(threeDView, ndi);
        q=gluNewQuadric();
        gluDisk(q,inner, outer,36,1);
        gluDeleteQuadric(q);
        glPopMatrix();
    }
}

void manetGLView::drawSphere( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, NodeDisplayInfoPtr &ndi)
{

    if (threeDView)
    {
        glPushMatrix();
        glTranslated(x, y, z);
        handleSize(ndi);
        handleSpin(threeDView, ndi);
        glPushAttrib(GL_NORMALIZE);
        glNormal3f(0.0, 0.0, 1.0);
        glutSolidSphere(radius, 10, 10);
        glPopAttrib();
        glPopMatrix();
    }
    else
        drawDisk(x,y,z,radius, ndi); 
}

void manetGLView::drawCircle(GLdouble x, GLdouble y, GLdouble z, GLdouble radius)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    handleSize(ndi);
    handleSpin(threeDView, ndi);
    GLUquadric* q=NULL;
    q=gluNewQuadric();
    gluDisk(q,radius-1,radius,36,1);
    gluDeleteQuadric(q);
    glPopMatrix();
}

void manetGLView::drawFrownyCircle(GLdouble x, GLdouble y, GLdouble z, GLdouble)
{ 
    static GLfloat const dead[]={1.0,0.0,0.0,1.0}; 

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, dead); 
    drawCircle(x, y, z, 7); 
    glBegin(GL_LINES); 
    glVertex3f(x-4.0,y+3.0,z); 
    glVertex3f(x-2.0,y+1.0,z); 
    glVertex3f(x-2.0,y+3.0,z); 
    glVertex3f(x-4.0,y+1.0,z); 

    glVertex3f(x+4.0,y+3.0,z); 
    glVertex3f(x+2.0,y+1.0,z); 
    glVertex3f(x+2.0,y+3.0,z); 
    glVertex3f(x+4.0,y+1.0,z); 

    glVertex3f(x-3.0,y-3.0,z); 
    glVertex3f(x-2.0,y-2.0,z); 
    glVertex3f(x-2.0,y-2.0,z); 
    glVertex3f(x+2.0,y-2.0,z); 
    glVertex3f(x+2.0,y-2.0,z); 
    glVertex3f(x+3.0,y-3.0,z); 

#if 0 
    glVertex3f(x+0.0,y-2.0,z); 
    glVertex3f(x+2.0,y-4.0,z); 
    glVertex3f(x+2.0,y-4.0,z); 
    glVertex3f(x+3.0,y-4.0,z); 
    glVertex3f(x+3.0,y-4.0,z); 
    glVertex3f(x+3.0,y-3.0,z); 
    glVertex3f(x+3.0,y-3.0,z); 
    glVertex3f(x+2.0,y-2.0,z); 
#endif 
    glEnd(); 
    return; 
} /* drawFrownyCircle */ 

void manetGLView::saveConfiguration()
{
    TRACE_ENTER();
    LOG_DEBUG("Got close event, saving modified configuration"); 

    Config &cfg=SingletonConfig::instance();
    singletonConfig::lock(); 
    libconfig::Setting &root=cfg.getRoot();

    struct 
    {
        const char *prop;
        bool boolVal;
    } boolConfigs[] =
    {
        { "nodes3d",        threeDView },
        { "monochrome",     monochromeMode },
        { "displayBackgroundImage", backgroundImage }
    };

    for (size_t i = 0; i < sizeof(boolConfigs)/sizeof(boolConfigs[0]); i++)
        root[boolConfigs[i].prop]=boolConfigs[i].boolVal;

    string prop="layers";
    libconfig::Setting &layers=cfg.lookup(prop);

    struct 
    {
        const char *prop;
        GUILayer layer;
    } layerVals[] =
    {
        { "bandwidth", BANDWIDTH_LAYER },
        { "undefined", UNDEFINED_LAYER },
        { "neighbors", PHYSICAL_LAYER },
        { "hierarchy", HIERARCHY_LAYER },
        { "routing", ROUTING_LAYER },
        { "routingOneHop", ONE_HOP_ROUTING_LAYER },
        { "antennaRadius", ANTENNARADIUS_LAYER },
        { "sanityCheck", SANITY_CHECK_LAYER },
        { "anomPaths", ANOMPATHS_LAYER }, 
        { "correlation", CORROLATION_LAYER },
        { "alert", ALERT_LAYER }, 
        { "correlation3Hop", CORROLATION_3HOP_LAYER },
        { "wormholeRouting", ROUTING2_LAYER },
        { "wormholeRoutingOnehop", ROUTING2_ONE_HOP_LAYER },
        { "normPaths", NORMAL_PATHS_LAYER }
    };
    for (size_t i=0; i<sizeof(layerVals)/sizeof(layerVals[0]); i++)
    {
        LayerList::const_iterator li=find(knownLayers.begin(), knownLayers.end(), layerVals[i].layer);
        if (li!=knownLayers.end())
            layers[layerVals[i].prop]=(*li)->active; 
    }

    root["viewPoint"]["angle"][0]=manetAdj.angleX;
    root["viewPoint"]["angle"][1]=manetAdj.angleY;
    root["viewPoint"]["angle"][2]=manetAdj.angleZ;
    root["viewPoint"]["scale"][0]=manetAdj.scaleX;
    root["viewPoint"]["scale"][1]=manetAdj.scaleY;
    root["viewPoint"]["scale"][2]=manetAdj.scaleZ;
    root["viewPoint"]["shift"][0]=manetAdj.shiftX;
    root["viewPoint"]["shift"][1]=manetAdj.shiftY;
    root["viewPoint"]["shift"][2]=manetAdj.shiftZ;

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

    SingletonConfig::saveConfig();
    SingletonConfig::unlock();

    TRACE_EXIT();
}

