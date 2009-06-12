
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
#include <osgDB/FileUtils>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Watcher3D includes
#include "watcher3D.h"
#include "nodeActorProxy.h"

INIT_LOGGER(Watcher3D, "Watcher3D");

// This is just for convenience/testing, should be removed later
std::string GetPosition(dtCore::RefPtr<dtDAL::ActorProxy> proxy)
{
   std::ostringstream ss;
   dtCore::Transform trans;
   float x1, y1, z1;

   (dynamic_cast<dtCore::Transformable*>(proxy->GetActor()))->GetTransform(trans,dtCore::Transformable::ABS_CS);
   trans.GetTranslation(x1,y1,z1);
   ss << x1 << " " << y1 << " " << z1;
   return ss.str();
}

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

    // Add flat terrain
    dtCore::RefPtr<dtDAL::ActorProxy> proxy = mGM->CreateActor("dtcore.Terrain", "Infinite Terrain");
    mGM->AddActor(*proxy);

    // Setup the camera
    dtCore::Transform camPos;
    camPos.SetTranslation(0.0f, -100.0f, 20.0f);
    GetCamera()->SetTransform(camPos);
    mFlyMotionModel = new dtCore::FlyMotionModel(GetKeyboard(), GetMouse());
    mFlyMotionModel->SetTarget(GetCamera());

    // dtDAL::Project::GetInstance().SetContext(".");
    // dtDAL::Map &myMap = dtDAL::Project::GetInstance().GetMap("Watcher3D");
    // dtDAL::Project::GetInstance().LoadMapIntoScene(myMap, *GetScene());

    // Load actor libraries (e.g., libNodeActor.so and libEdgeActor.so.)
    mGM->LoadActorRegistry("NodeActor");
    mGM->LoadActorRegistry("EdgeActor");

    // Print all known actor types
    std::vector<const dtDAL::ActorType*> actorTypes;
    mGM->GetActorTypes(actorTypes);
    for(unsigned int i = 0; i < actorTypes.size(); i++)
      std::cout << actorTypes[i]->GetCategory() << " " << actorTypes[i]->GetName() << std::endl;

    // Add nodes and edges
    dtCore::RefPtr<const dtDAL::ActorType> nodeActorType = mGM->FindActorType("Watcher3D Actors", "Node");
    dtCore::RefPtr<const dtDAL::ActorType> edgeActorType = mGM->FindActorType("Watcher3D Actors", "Edge");
    dtCore::RefPtr<dtDAL::ActorProxy> nodeActorProxy = dynamic_cast<dtDAL::ActorProxy*>(mGM->CreateActor(*nodeActorType).get());
    dtCore::RefPtr<dtDAL::ActorProxy> edgeActorProxy = dynamic_cast<dtDAL::ActorProxy*>(mGM->CreateActor(*edgeActorType).get());
    if(nodeActorProxy)
        mGM->AddActor(*nodeActorProxy);
    if(edgeActorProxy)
        mGM->AddActor(*edgeActorProxy);
    /*
    dtCore::RefPtr<dtDAL::ActorProxy> edgeActorProxy = mGM->CreateActor("Watcher3D Actors", "Node");
    dtCore::RefPtr<dtDAL::ActorProxy> nodeActorProxy = mGM->CreateActor("Watcher3D Actors", "Edge");
    mGM->AddActor(*nodeActorProxy);
    mGM->AddActor(*edgeActorProxy);
    */

    // Change some properties
    edgeActorProxy->GetProperty("HeadPos")->FromString(GetPosition(nodeActorProxy));
    // edgeActorProxy->GetProperty("EndPoint")->FromString(GetPosition(mTankProxy2));

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
