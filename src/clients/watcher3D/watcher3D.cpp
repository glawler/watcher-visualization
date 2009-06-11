
#include "watcher3D.h"
#include "nodeActorProxy.h"
#include <dtCore/globals.h>
#include <dtCore/object.h>
#include <dtCore/flymotionmodel.h>
#include <dtCore/transform.h>
#include <dtDAL/project.h>
#include <osgDB/FileUtils>

INIT_LOGGER(Watcher3D, "Watcher3D");

Watcher3D::Watcher3D(const std::string& configFilename) :
    dtABC::Application(configFilename),
    mFlyMotionModel(0)
{
    TRACE_ENTER();

    Config();
    mGM = new dtGame::GameManager(*GetScene());

    // Generating a default config file if the one passed in is not there.
    if(osgDB::findDataFile(configFilename).empty())
    {
        GenerateDefaultConfigFile();
    }

    dtCore::Transform camPos;
    camPos.SetTranslation(0.0f, -100.0f, 20.0f);
    GetCamera()->SetTransform(camPos);
    mFlyMotionModel = new dtCore::FlyMotionModel(GetKeyboard(), GetMouse());
    mFlyMotionModel->SetTarget(GetCamera());

    dtDAL::Project::GetInstance().SetContext(".");
    dtDAL::Map &myMap = dtDAL::Project::GetInstance().GetMap("Watcher3D");
    dtDAL::Project::GetInstance().LoadMapIntoScene(myMap, *GetScene());

    dtCore::RefPtr<NodeActorProxy> nodeActorProxy;
    mGM->CreateActor("dtcore.Tasks","Node Actor", nodeActorProxy);
  
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
