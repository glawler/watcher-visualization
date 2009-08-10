/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */


// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtDAL/enginepropertytypes.h>
#include <dtCore/flymotionmodel.h>
#include <dtCore/globals.h>
#include <dtCore/transform.h>
#include <dtGame/gameactor.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Watcher3D includes
#include "libWatcher3D.h"
#include "nodeActor.h"

extern "C" DT_PLUGIN_EXPORT dtGame::GameEntryPoint* CreateGameEntryPoint()
{
    return new LibWatcher3D;
}

extern "C" DT_PLUGIN_EXPORT void DestroyGameEntryPoint(dtGame::GameEntryPoint* entryPoint)
{
    delete entryPoint;
}

LibWatcher3D::LibWatcher3D()
 : dtGame::GameEntryPoint(), motionModel(NULL)
{
}

LibWatcher3D::~LibWatcher3D()
{
}

void LibWatcher3D::Initialize(dtGame::GameApplication& app, int argc, char** argv)
{
    dtGame::GameManager* gameManager = app.GetGameManager();

    // Setup path
    std::string dataPath = dtCore::GetDeltaDataPathList();
    dtCore::SetDataFilePathList(dataPath + ";" + dtCore::GetDeltaRootPath() + 
        "data" + ";" + dtCore::GetDeltaRootPath() + "data/models"); 

    // Add terrain
    dtCore::RefPtr<dtDAL::ActorProxy> terrainActorProxy;
    gameManager->CreateActor("dtcore.Terrain", "Infinite Terrain", terrainActorProxy);
    gameManager->AddActor(*terrainActorProxy);

    // Create camera 
    dtCore::Transform camPos;
    camPos.SetTranslation(0.0f, -100.0f, 20.0f);
    app.GetCamera()->SetTransform(camPos);
    motionModel = new dtCore::FlyMotionModel(app.GetKeyboard(), app.GetMouse());
    motionModel->SetTarget(app.GetCamera());

    // Load actor registry
    gameManager->LoadActorRegistry("Actors"); // (libActors.so)

    // Print actor types
    /*
    std::vector<const dtDAL::ActorType*> actorTypes;
    gameManager->GetActorTypes(actorTypes);
    for(unsigned int i = 0; i < actorTypes.size(); i++)
      std::cout << actorTypes[i]->GetCategory() << " " << actorTypes[i]->GetName() << std::endl;
    */
}

void LibWatcher3D::OnStartup(dtGame::GameApplication &app)
{
    dtGame::GameManager* gameManager = app.GetGameManager();

    // Add some nodes and edges
    dtCore::RefPtr<dtGame::GameActorProxy> nodeActorProxy1;
    dtCore::RefPtr<dtGame::GameActorProxy> nodeActorProxy2;
    dtCore::RefPtr<dtGame::GameActorProxy> edgeActorProxy1;
    gameManager->CreateActor("Watcher3D Actors", "Node", nodeActorProxy1);
    gameManager->CreateActor("Watcher3D Actors", "Node", nodeActorProxy2);
    gameManager->CreateActor("Watcher3D Actors", "Edge", edgeActorProxy1);
    gameManager->AddActor(*nodeActorProxy1, false, true);
    gameManager->AddActor(*nodeActorProxy2, false, true);
    gameManager->AddActor(*edgeActorProxy1, false, true);

    // Change some properties
    dtDAL::Vec3ActorProperty* node1Pos = static_cast<dtDAL::Vec3ActorProperty*>(nodeActorProxy1->GetProperty("pos"));
    dtDAL::Vec3ActorProperty* node2Pos = static_cast<dtDAL::Vec3ActorProperty*>(nodeActorProxy2->GetProperty("pos"));
    osg::Vec3 pos1(0.0, -100.0f, 5.0f);
    osg::Vec3 pos2(0.0, 100.0f, 25.0f);
    node1Pos->SetValue(pos1);
    node2Pos->SetValue(pos2);
    dtDAL::Vec3ActorProperty* edge1HeadPos = static_cast<dtDAL::Vec3ActorProperty*>(edgeActorProxy1->GetProperty("headPos"));
    dtDAL::Vec3ActorProperty* edge1TailPos = static_cast<dtDAL::Vec3ActorProperty*>(edgeActorProxy1->GetProperty("tailPos"));
    edge1HeadPos->SetValue(node1Pos->GetValue());
    edge1TailPos->SetValue(node2Pos->GetValue());
}

