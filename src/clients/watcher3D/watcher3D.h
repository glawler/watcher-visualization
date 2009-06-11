
#ifndef WATCHER_3D_H
#define WATCHER_3D_H

// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtABC/application.h>
#include <dtCore/refptr.h>
#include <dtGame/gamemanager.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Forward declarations to keep compile-time down
namespace dtCore
{
   class Object;
   class FlyMotionModel;
}

class Watcher3D : public dtABC::Application
{
    public:
        Watcher3D(const std::string& configFilename);
    protected:
        virtual ~Watcher3D();

    public:
        virtual void Config();

    private:
        dtCore::RefPtr<dtGame::GameManager> mGM;
        dtCore::RefPtr<dtCore::FlyMotionModel> mFlyMotionModel;
        DECLARE_LOGGER();
};

#endif // WATCHER_3D_H
