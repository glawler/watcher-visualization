
// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtCore/globals.h>
#include <dtCore/object.h>
#include <dtCore/flymotionmodel.h>
#include <dtCore/transform.h>
#include <dtDAL/actorproperty.h>
#include <dtDAL/actortype.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtGame/gameactor.h>
#include <osgDB/FileUtils>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Watcher3D includes
#include "watcher3D.h"
#include "nodeActorProxy.h"

INIT_LOGGER(Watcher3D, "Watcher3D");

Watcher3D::Watcher3D(int argc, char **argv) :
    dtGame::GameApplication(argc,argv),
    mFlyMotionModel(0)
{
    TRACE_ENTER();

    TRACE_EXIT();
}

Watcher3D::~Watcher3D()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

/*
void Watcher3D::Config()
{
    TRACE_ENTER();
    // Load up models here
    TRACE_EXIT();
}
*/
