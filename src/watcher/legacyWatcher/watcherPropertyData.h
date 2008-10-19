#ifndef WATCHER_PROPRETY_DTAT____H
#define  WATCHER_PROPRETY_DTAT____H

#include <vector>
#include "idsCommunications.h"

typedef struct 
{
    unsigned int identifier;  // for nodes this is the ManetAddr. 

    // Item's shape
    WatcherShape shape;

    // effects on is non-zero
    int sparkle;
    int spin;
    int flash;

    // Size multiplier. 
    float size;

    //
    // Spin data. Used when spin is enabled (spin!=0).
    //
    static const int spinTimeout=100;      // update rotatation every spinTimeout milliseconds
    static const float spinIncrement=5.0;  // Amount of spin per timeout period
    long long int nextSpinUpdate;   // epoch milliseconds, same as destime.
    float spinRotation_x;           // cur rotation for x plane
    float spinRotation_y;           // cur rotation for y plane
    float spinRotation_z;           // cur rotation for d plane, no, wait z plane.

    //
    // Flash data. Flash is done by inverting the color of the thing every
    // x milliseconds.
    //
    static const long long int flashInterval=500;  // flash every flashRate milliseconds
    long long int nextFlashUpdate;      // Next time to invert the colors. 
    int isFlashed;                      // 1 if color is currently inverted.

    // Don't need color as there is already a color field in the manetNode struct so 
    // we can just use that one.

} WatcherPropertyData;

typedef std::vector<WatcherPropertyData*> WatcherPropertiesList;

WatcherPropertyData *findWatcherPropertyData(unsigned int index, WatcherPropertiesList &theList);


#endif //  WATCHER_PROPRETY_DTAT____H
