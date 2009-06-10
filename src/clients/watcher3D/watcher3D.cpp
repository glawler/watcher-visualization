
#include "watcher3D.h"
#include <dtCore/globals.h>
#include <dtCore/object.h>
#include <dtCore/flymotionmodel.h>
#include <osgDB/FileUtils>

INIT_LOGGER(Watcher3D, "Watcher3D");

Watcher3D::Watcher3D(const std::string& configFilename) :
    dtABC::Application(configFilename),
    mText(0),
    mFlyMotionModel(0)
{
    TRACE_ENTER();
    // Generating a default config file if the one passed in is not there.
    if(osgDB::findDataFile(configFilename).empty())
    {
        GenerateDefaultConfigFile();
    }
    TRACE_EXIT();
}

Watcher3D::~Watcher3D()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void Watcher3D::Config()
{
    TRACE_ENTER();
    // Load up models here
    TRACE_EXIT();
}
