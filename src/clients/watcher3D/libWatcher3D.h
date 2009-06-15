
#ifndef LIB_WATCHER_3D_H
#define LIB_WATCHER_3D_H

// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtDAL/plugin_export.h>
#include <dtGame/gameapplication.h>
#include <dtGame/gameentrypoint.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

class DT_PLUGIN_EXPORT LibWatcher3D : public dtGame::GameEntryPoint
{
    public:
        LibWatcher3D();
        virtual ~LibWatcher3D();
        virtual void Initialize(dtGame::GameApplication& app, int argc, char** argv);
        virtual void OnStartup(dtGame::GameApplication& app);
    private:
        dtCore::RefPtr<dtCore::MotionModel> mMotionModel;
};

#endif // LIB_WATCHER_3D_H
