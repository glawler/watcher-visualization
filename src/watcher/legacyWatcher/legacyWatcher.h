#ifndef LEGACYWATCHER_H
#define LEGACYWATCHER_H

#include "idsCommunications.h"  // for COMMUNICATIONS_LABEL_FAMILY_BANDWIDTH and the like
                                // for globalDispStat
namespace legacyWatcher
{
    int legacyWatcherMain(int argc, char **argv);

    void initWatcherGL();

    void drawManet(void);
    void drawHierarchy(void);

    void ReshapeManet(int awidth, int aheight);
    void ReshapeHierarchy(int awidth, int aheight);

    void jumpToX(int x);
    void jumpToY(int y);
    void jumpToXY(int x, int y);

    void shiftCenterRight(double shift); 
    void shiftCenterLeft(double shift); 
    void shiftCenterDown(double shift);
    void shiftCenterUp(double shift);
    void shiftCenterIn(double shift);
    void shiftCenterOut(double shift);

    // Shift the background image, if loaded. 
    void shiftBackgroundCenterLeft(double dx);
    void shiftBackgroundCenterUp(double dy);

    // let watcher use default shift values
    void shiftCenterRight(); 
    void shiftCenterLeft(); 
    void shiftCenterDown();
    void shiftCenterUp();
    void shiftCenterIn();
    void shiftCenterOut();

    void viewpointReset(void);

    // Zooms out from the users point of view
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

    int isRunningInPlaybackMode();
    int checkIOLive(int notUsed);
    int checkIOGoodwin(int notUsed);

    typedef enum {
        ManetView,
        HierarchyView
    } WatcherView;

    void setActiveView(const WatcherView &view);

    int Key(unsigned char key);

    // Handle layer toggling. 
    // These layers match the #defines in idsCommunications.h
    typedef enum {
        Undefined = 1 << COMMUNICATIONS_LABEL_FAMILY_UNDEFINED,
        Neighbors = 1 << COMMUNICATIONS_LABEL_FAMILY_PHYSICAL,
        Hierarchy = 1 << COMMUNICATIONS_LABEL_FAMILY_HIERARCHY,
        Bandwidth = 1 << COMMUNICATIONS_LABEL_FAMILY_BANDWIDTH,
        Routing = 1 << COMMUNICATIONS_LABEL_FAMILY_ROUTING,
        RoutingOnehop = 1 << COMMUNICATIONS_LABEL_FAMILY_ROUTING_ONEHOP,
        AntennaRadius = 1 << COMMUNICATIONS_LABEL_FAMILY_ANTENNARADIUS,
        SanityCheck = 1 << COMMUNICATIONS_LABEL_FAMILY_SANITYCHECK,
        AnomPaths = 1 << COMMUNICATIONS_LABEL_FAMILY_ANOMPATHS,
        Correlation = 1 << COMMUNICATIONS_LABEL_FAMILY_CORRELATION,
        Alert = 1 << COMMUNICATIONS_LABEL_FAMILY_ALERT,
        Correlation3Hop = 1 << COMMUNICATIONS_LABEL_FAMILY_CORRELATION_3HOP,
        WormholeRouting = 1 << COMMUNICATIONS_LABEL_FAMILY_ROUTING2,
        WormholeRoutingOnehop = 1 << COMMUNICATIONS_LABEL_FAMILY_ROUTING2_ONEHOP,
        FloatingGraph = 1 << COMMUNICATIONS_LABEL_FAMILY_FLOATINGGRAPH,
        NormPaths = 1 << COMMUNICATIONS_LABEL_FAMILY_NORMPATHS
    } Layer;

    void layerToggle(const Layer layer, const bool turnOn); 
    NodeDisplayStatus &getDisplayStatus(void);

    void toggleThreeDView(bool isOn);
    void toggleMonochrome(bool isOn); 
    void toggleBackgroundImage(bool isOn); 

    typedef enum GPSDataFormat { GPS_DATA_FORMAT_UTM, GPS_DATA_FORMAT_DEG_WGS84 }; 
    void setGPSDataFormat(const GPSDataFormat &format);
    GPSDataFormat getGPSDataFormat();

    void clearAllLabels();
    void clearAllEdges();

    bool runningGoodwin(); // return true if running from a playback file. Must be called after legacyWatcherMain()
    void setGoodwinPlaybackSpeed(int val);
    void pauseGoodwin();
    void continueGoodwin();
    void startGoodwin();
    void stopGoodwin();

    // This will be called periodically. 
    // legacyWatcher can use this to do animation, etc. 
    // Returns non zero if the window should be refreshed.
    int doIdle();

    // Called on mouse double-click with the screen coords. 
    // The legacy watcher should find the node closest to x, y and get status.
    // Caller shuld allocate space for a string and pass it in
    int getNodeStatus(const int x, const int y, char *nodeStatusBuf, size_t bufSize);

    // Return the unique id for the node that is at the given coords.
    unsigned int getNodeIdAtCoords(const int x, const int y);

    // args must between 1.0 and 0.0.
    void setBackgroundColor(float r, float g, float b, float a); 

    struct GlobalManetAdj
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

    GlobalManetAdj &getManetAdj();

}; // namespace legacyWatcher

#endif
