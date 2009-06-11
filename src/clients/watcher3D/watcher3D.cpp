
// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtCore/globals.h>
#include <dtCore/object.h>
#include <dtCore/flymotionmodel.h>
#include <dtCore/transform.h>
#include <dtDAL/project.h>
#include <osgDB/FileUtils>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Watcher3D includes
#include "watcher3D.h"
#include "nodeActorProxy.h"

INIT_LOGGER(Watcher3D, "Watcher3D");

Watcher3D::Watcher3D(const std::string& configFilename) :
    dtABC::Application(configFilename),
    mFlyMotionModel(0)
{
    TRACE_ENTER();

    std::cout << "c2" << std::endl;
    Config();
    mGM = new dtGame::GameManager(*GetScene());
    std::cout << "c3" << std::endl;

    // Generating a default config file if the one passed in is not there.
    if(osgDB::findDataFile(configFilename).empty())
    {
        GenerateDefaultConfigFile();
    }

    std::cout << "c4" << std::endl;
    dtCore::Transform camPos;
    camPos.SetTranslation(0.0f, -100.0f, 20.0f);
    GetCamera()->SetTransform(camPos);
    mFlyMotionModel = new dtCore::FlyMotionModel(GetKeyboard(), GetMouse());
    mFlyMotionModel->SetTarget(GetCamera());

    // dtDAL::Project::GetInstance().SetContext(".");
    // dtDAL::Map &myMap = dtDAL::Project::GetInstance().GetMap("Watcher3D");
    // dtDAL::Project::GetInstance().LoadMapIntoScene(myMap, *GetScene());

    std::cout << "c5" << std::endl;
    dtCore::RefPtr<NodeActorProxy> nodeActorProxy;
    std::cout << "c6" << std::endl;
    mGM->AddActor(nodeActorProxy,false,true);
    std::cout << "c7" << std::endl;
  
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
