#ifndef WATCHER_PROPRETY_DTAT____H
#define WATCHER_PROPRETY_DTAT____H

#include <vector>
#include "idsCommunications.h"
#include "logger.h"

namespace watcher
{
    class WatcherPropertyData 
    {
        public:
            WatcherPropertyData();
            ~WatcherPropertyData();

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

            char guiLabel[64];    // The ID that is shown in the GUI next to the node; the node's ID label.

            // Don't need color as there is already a color field in the manetNode struct so 
            // we can just use that one.

        protected:
            // NOOP

        private:
            DECLARE_LOGGER();

            WatcherPropertyData(const WatcherPropertyData &noCopies);
            WatcherPropertyData &operator=(const WatcherPropertyData &noCopies);

    };

    //
    // Functions which act on WatcherPropertyData instances. 
    //
    typedef std::vector<WatcherPropertyData*> WatcherPropertiesList;
    WatcherPropertyData *findWatcherPropertyData(unsigned int index, WatcherPropertiesList &theList);

    // Read props in from a cfg file, if we have one. nodeId is default node labelm if not given in the cfg file.
    void loadWatcherPropertyData(WatcherPropertyData *, int nodeId); 

}


#endif //  WATCHER_PROPRETY_DTAT____H
