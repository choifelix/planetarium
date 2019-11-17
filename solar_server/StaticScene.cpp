//
// Copyright (c) 2008-2015 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>

#include "StaticScene.h"
#include "Rotator.h"

#include <Urho3D/DebugNew.h>

#define PI 3.14159265
#define SUN_R 3.0f
#define UA 5.0f 
#define RES_T -50.0f

const int MSG_GAME = 32;
const unsigned short GAME_SERVER_PORT = 32000;

URHO3D_DEFINE_APPLICATION_MAIN(StaticScene)

StaticScene::StaticScene(Context* context) :
    Sample(context)
{
    context->RegisterFactory<Rotator>();
    autorised = true;
    tkt = 0;
    sky = true;
    secret = false;
    
    const Vector<String>& arguments=GetArguments();

    sscanf(arguments[0].CString(),"%d",&myPort);
    sscanf(arguments[1].CString(),"%d",&myAngle);

    printf("myPort=%d myAngle=%d\n",myPort, myAngle);
}

void StaticScene::Start()
{
    // Execute base class startup
    Sample::Start();

    SetLogoVisible(false);

    cache = GetSubsystem<ResourceCache>();

    input = GetSubsystem<Input>();
    nbJoysticks=input->GetNumJoysticks();
    if (nbJoysticks>0)
    js=input->GetJoystickByIndex(0);
    else
    js=NULL;

    if (nbJoysticks>0)
    printf("Il y a un joystick.\n");

    pitch_ = 60.0f;

    xpos=1;
    ypos=1;

    cursorLocation=0;

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateInstructions();

    // Setup the viewport for displaying the scene
    SetupViewport();

    // Hook up to the frame update events
    SubscribeToEvents();
}

void StaticScene::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);
    scene_->SetUpdateEnabled(false);

    // Create the Octree component to the scene. This is required before adding any drawable components, or else nothing will
    // show up. The default octree volume will be from (-1000, -1000, -1000) to (1000, 1000, 1000) in world coordinates; it
    // is also legal to place objects outside the volume but their visibility can then not be checked in a hierarchically
    // optimizing manner
    scene_->CreateComponent<Octree>();


    //skybox creation
    skyNode = scene_->CreateChild("skybox");
    Skybox* skybox = skyNode->CreateComponent<Skybox>();
    skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
    skybox->SetMaterial(cache->GetResource<Material>("Materials/skybox_stars.xml"));


    Node* planeNode = scene_->CreateChild("Plane");
    planeNode->SetScale(Vector3(5.0f, 1.0f, 5.0f));
    StaticModel* planeObject = planeNode->CreateComponent<StaticModel>();
    planeObject->SetModel(cache->GetResource<Model>("Models/Disk.mdl"));
    planeObject->SetMaterial(cache->GetResource<Material>("Materials/GreenTransparent.xml"));

    sunPosNode = scene_->CreateChild("SunPos");
    sunPosNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    sunPosNode->SetScale(Vector3(1.0f, 1.0f, 1.0f));

    Sun_graphic = sunPosNode->CreateChild("Sun_graphic");
    Sun_graphic->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Sun_graphic->SetScale(Vector3(SUN_R, SUN_R, SUN_R));

    StaticModel* sunObject = Sun_graphic->CreateComponent<StaticModel>();  
    sunObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    sunObject->SetMaterial(cache->GetResource<Material>("Materials/sun.xml"));

    //secret
    Node * pecheux_graphic = sunPosNode->CreateChild("pecheux_graphic");
    pecheux_graphic->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    pecheux_graphic->SetScale(Vector3(1.0f, 1.0f, 1.0f));

    StaticModel* pecheuxObject = pecheux_graphic->CreateComponent<StaticModel>();  
    pecheuxObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    pecheuxObject->SetMaterial(cache->GetResource<Material>("Materials/pecheux.xml"));

    sunPosRotNode = sunPosNode->CreateChild("SunPosRot");
    sunPosRotNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatorSunPosRot = sunPosRotNode->CreateComponent<Rotator>();
    rotatorSunPosRot->SetRotationSpeed(Vector3(0.0f, RES_T, 0.0f));


    // creation terre + axe de rotation + lune
    earthPosNode = sunPosRotNode->CreateChild("EarthPos");
    earthPosNode->SetPosition(Vector3(5.0f, 0.0f, 0.0f));
    //earthPosNode->SetScale(Vector3(1.0f, 1.0f, 1.0f));
    //Rotator* rotatorEarthPos = earthPosNode->CreateComponent<Rotator>();
    //rotatorEarthPos->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));


    //axe non horizontale de la terre 
    Node* earthInclinedNode = earthPosNode->CreateChild("EarthInclined");
    earthInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    earthInclinedNode->SetRotation(Quaternion(0.0f, 0.0f, 23.0f));

    Node* cylinderInclinedNode = earthInclinedNode->CreateChild("cylinderInclined");
    cylinderInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    cylinderInclinedNode->SetScale(Vector3(0.01f, 2.0f, 0.01f));
    StaticModel* cylinderInclinedObject = cylinderInclinedNode->CreateComponent<StaticModel>();
    cylinderInclinedObject->SetModel(cache->GetResource<Model>("Models/Cylinder.mdl"));

    Node* earthNode = earthInclinedNode->CreateChild("Earth");
    //Node* earthNode = earthPosNode->CreateChild("Earth");
    earthNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    earthNode->SetScale(Vector3(0.3f, 0.3f, 0.3f));
    StaticModel* earthObject = earthNode->CreateComponent<StaticModel>();
    earthObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    earthObject->SetMaterial(cache->GetResource<Material>("Materials/earthmap.xml"));
    Rotator* rotator = earthNode->CreateComponent<Rotator>();
    rotator->SetRotationSpeed(Vector3(0.0f, -30.0f, 0.0f));

    // rotation de la terre autour du soleil
    Node* earthPosRotNode = earthPosNode->CreateChild("EarthPosRot");
    earthPosRotNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatorEarthPosRot = earthPosRotNode->CreateComponent<Rotator>();
    rotatorEarthPosRot->SetRotationSpeed(Vector3(0.0f, -100.0f, 0.0f));


    //creation de la lune
    Node* moonNode = earthPosRotNode->CreateChild("Moon");
    moonNode->SetPosition(Vector3(0.3f, 0.0f, 0.0f));
    moonNode->SetScale(Vector3(0.05f, 0.05f, 0.05f));
    StaticModel* moonObject = moonNode->CreateComponent<StaticModel>();
    moonObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    moonObject->SetMaterial(cache->GetResource<Material>("Materials/moonmap.xml"));
    //rotaion de la lune autoure de la terre
    Rotator* rotatorMoon = moonNode->CreateComponent<Rotator>();
    rotatorMoon->SetRotationSpeed(Vector3(0.0f, -30.0f, 0.0f));


    // creation Mars
    Node * Mars_orbit = sunPosNode->CreateChild("MarsOrbit");
    Mars_orbit->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatorMars = Mars_orbit->CreateComponent<Rotator>();
    rotatorMars->SetRotationSpeed(Vector3(0.0f, RES_T *0.55f, 0.0f));


    marsPosNode = Mars_orbit->CreateChild("marsPos");
    marsPosNode->SetPosition(Vector3(1.5 * 5.0f, 0.0f, 0.0f));
    //earthPosNode->SetScale(Vector3(1.0f, 1.0f, 1.0f));
    //Rotator* rotatorEarthPos = earthPosNode->CreateComponent<Rotator>();
    //rotatorEarthPos->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));


    //axe non horizontale de Mars
    Node* marsInclinedNode = marsPosNode->CreateChild("marsInclined");
    marsInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    marsInclinedNode->SetRotation(Quaternion(0.0f, 0.0f, 23.0f));

    Node* Mars_cylinderInclinedNode = marsInclinedNode->CreateChild("MarscylinderInclined");
    Mars_cylinderInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Mars_cylinderInclinedNode->SetScale(Vector3(0.01f, 2.0f, 0.01f));
    //StaticModel* Mars_cylinderInclinedObject = Mars_cylinderInclinedNode->CreateComponent<StaticModel>();
    //Mars_cylinderInclinedObject->SetModel(cache->GetResource<Model>("Models/Cylinder.mdl"));

    Node* marsNode = marsInclinedNode->CreateChild("Mars");
    marsNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    marsNode->SetScale(Vector3(0.25f, 0.25f, 0.25f));
    StaticModel* marsObject = marsNode->CreateComponent<StaticModel>();
    marsObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    marsObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/marsmap.xml"));

    // rotation de Mars autour du soleil
    Node* marsPosRotNode = marsPosNode->CreateChild("MarsPosRot");
    marsPosRotNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatorMarsPosRot = marsPosRotNode->CreateComponent<Rotator>();
    rotatorMarsPosRot->SetRotationSpeed(Vector3(0.0f, 10.0f, 0.0f));





     //###################### creation mercure #############################
    Node * mercure_orbit = sunPosNode->CreateChild("mercureOrbit");
    mercure_orbit->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatormercure = mercure_orbit->CreateComponent<Rotator>();
    rotatormercure->SetRotationSpeed(Vector3(0.0f, RES_T*10.0f, 0.0f));


    Node * mercurePosNode = mercure_orbit->CreateChild("mercurePos");
    mercurePosNode->SetPosition(Vector3(0.4 * 5.0f, 0.0f, 0.0f));
    //earthPosNode->SetScale(Vector3(1.0f, 1.0f, 1.0f));
    //Rotator* rotatorEarthPos = earthPosNode->CreateComponent<Rotator>();
    //rotatorEarthPos->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));

    // Node * mercure_orbitTorus = mercure_orbit->CreateChild("mercurOrbit_Torus");
    // mercure_orbitTorus->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    // mercure_orbitTorus->SetScale(Vector3(0.4 * 5.0f, 0.1f, 0.4 * 5.0f));
    // StaticModel* mercure_orbitObject = mercure_orbitTorus->CreateComponent<StaticModel>();
    // mercure_orbitObject->SetModel(cache->GetResource<Model>("Models/Torus.mdl"));
    //mercure_orbitObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/mercuremap.xml"));


    //axe non horizontale de mercure
    Node* mercureInclinedNode = mercurePosNode->CreateChild("mercureInclined");
    mercureInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    mercureInclinedNode->SetRotation(Quaternion(0.0f, 0.0f, 23.0f));

    Node* mercure_cylinderInclinedNode = mercureInclinedNode->CreateChild("mercurecylinderInclined");
    mercure_cylinderInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    mercure_cylinderInclinedNode->SetScale(Vector3(0.01f, 2.0f, 0.01f));
    //StaticModel* mercure_cylinderInclinedObject = mercure_cylinderInclinedNode->CreateComponent<StaticModel>();
    //mercure_cylinderInclinedObject->SetModel(cache->GetResource<Model>("Models/Cylinder.mdl"));

    Node* mercureNode = mercureInclinedNode->CreateChild("mercure");
    mercureNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    mercureNode->SetScale(Vector3(0.15f, 0.15f, 0.15f));
    StaticModel* mercureObject = mercureNode->CreateComponent<StaticModel>();
    mercureObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    mercureObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/mercuremap.xml"));

    // rotation de mercure autour du soleil
    Node* mercurePosRotNode = mercurePosNode->CreateChild("mercurePosRot");
    mercurePosRotNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatormercurePosRot = mercurePosRotNode->CreateComponent<Rotator>();
    rotatormercurePosRot->SetRotationSpeed(Vector3(0.0f, 10.0f, 0.0f));





    //###################### creation venus #############################
    Node * venus_orbit = sunPosNode->CreateChild("venusOrbit");
    venus_orbit->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatorvenus = venus_orbit->CreateComponent<Rotator>();
    rotatorvenus->SetRotationSpeed(Vector3(0.0f, RES_T * 1.62f, 0.0f));


    Node * venusPosNode = venus_orbit->CreateChild("venusPos");
    venusPosNode->SetPosition(Vector3(0.7 * 5.0f, 0.0f, 0.0f));
    //earthPosNode->SetScale(Vector3(1.0f, 1.0f, 1.0f));
    //Rotator* rotatorEarthPos = earthPosNode->CreateComponent<Rotator>();
    //rotatorEarthPos->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));


    //axe non horizontale de venus
    Node* venusInclinedNode = venusPosNode->CreateChild("venusInclined");
    venusInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    venusInclinedNode->SetRotation(Quaternion(0.0f, 0.0f, 23.0f));

    Node* venus_cylinderInclinedNode = venusInclinedNode->CreateChild("venuscylinderInclined");
    venus_cylinderInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    venus_cylinderInclinedNode->SetScale(Vector3(0.01f, 2.0f, 0.01f));
    //StaticModel* venus_cylinderInclinedObject = venus_cylinderInclinedNode->CreateComponent<StaticModel>();
    //venus_cylinderInclinedObject->SetModel(cache->GetResource<Model>("Models/Cylinder.mdl"));

    Node* venusNode = venusInclinedNode->CreateChild("venus");
    venusNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    venusNode->SetScale(Vector3(0.28f, 0.28f, 0.28f));
    StaticModel* venusObject = venusNode->CreateComponent<StaticModel>();
    venusObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    venusObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/venusmap.xml"));

    // rotation de venus autour du soleil
    Node* venusPosRotNode = venusPosNode->CreateChild("venusPosRot");
    venusPosRotNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatorvenusPosRot = venusPosRotNode->CreateComponent<Rotator>();
    rotatorvenusPosRot->SetRotationSpeed(Vector3(0.0f, 10.0f, 0.0f));





    //############## creation jupiter ###################
    Node * jupiter_orbit = sunPosNode->CreateChild("jupiterOrbit");
    jupiter_orbit->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatorjupiter = jupiter_orbit->CreateComponent<Rotator>();
    rotatorjupiter->SetRotationSpeed(Vector3(0.0f, RES_T /12 , 0.0f));


    jupiterPosNode = jupiter_orbit->CreateChild("jupiterPos");
    jupiterPosNode->SetPosition(Vector3(2.3f * UA, 0.0f, 0.0f));

    Node* lightNode_jupiter = jupiterPosNode->CreateChild("DirectionalLight");
    lightNode_jupiter->SetPosition(Vector3( -1.5, 0.0f, 0.0f));
    Light* light_jupiter = lightNode_jupiter->CreateComponent<Light>();
    light_jupiter->SetBrightness(1.0);
    //earthPosNode->SetScale(Vector3(1.0f, 1.0f, 1.0f));
    //Rotator* rotatorEarthPos = earthPosNode->CreateComponent<Rotator>();
    //rotatorEarthPos->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));


    //axe non horizontale de jupiter
    Node* jupiterInclinedNode = jupiterPosNode->CreateChild("jupiterInclined");
    jupiterInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    jupiterInclinedNode->SetRotation(Quaternion(0.0f, 0.0f, 23.0f));

    Node* jupiter_cylinderInclinedNode = jupiterInclinedNode->CreateChild("jupitercylinderInclined");
    jupiter_cylinderInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    jupiter_cylinderInclinedNode->SetScale(Vector3(0.01f, 2.0f, 0.01f));
    //StaticModel* jupiter_cylinderInclinedObject = jupiter_cylinderInclinedNode->CreateComponent<StaticModel>();
    //jupiter_cylinderInclinedObject->SetModel(cache->GetResource<Model>("Models/Cylinder.mdl"));

    Node* jupiterNode = jupiterInclinedNode->CreateChild("jupiter");
    jupiterNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    jupiterNode->SetScale(Vector3(1.0f, 1.0f, 1.0f));
    StaticModel* jupiterObject = jupiterNode->CreateComponent<StaticModel>();
    jupiterObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    jupiterObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/jupitermap.xml"));

    // rotation de jupiter autour du soleil
    Node* jupiterPosRotNode = jupiterPosNode->CreateChild("jupiterPosRot");
    jupiterPosRotNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatorjupiterPosRot = jupiterPosRotNode->CreateComponent<Rotator>();
    rotatorjupiterPosRot->SetRotationSpeed(Vector3(0.0f, 10.0f, 0.0f));







     //############## creation saturne ###################
    Node * saturne_orbit = sunPosNode->CreateChild("saturneOrbit");
    saturne_orbit->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatorsaturne = saturne_orbit->CreateComponent<Rotator>();
    rotatorsaturne->SetRotationSpeed(Vector3(0.0f, RES_T /29 , 0.0f));


    Node * saturnePosNode = saturne_orbit->CreateChild("saturnePos");
    saturnePosNode->SetPosition(Vector3(3.5f * UA, 0.0f, 0.0f));

    Node* lightNode_saturne = saturnePosNode->CreateChild("DirectionalLight");
    lightNode_saturne->SetPosition(Vector3( -1.5, 0.0f, 0.0f));
    Light* light_saturne = lightNode_saturne->CreateComponent<Light>();
    light_saturne->SetBrightness(1.0);
    //earthPosNode->SetScale(Vector3(1.0f, 1.0f, 1.0f));
    //Rotator* rotatorEarthPos = earthPosNode->CreateComponent<Rotator>();
    //rotatorEarthPos->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));


    //axe non horizontale de saturne
    Node* saturneInclinedNode = saturnePosNode->CreateChild("saturneInclined");
    saturneInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    saturneInclinedNode->SetRotation(Quaternion(0.0f, 0.0f, 23.0f));

    Node* saturne_cylinderInclinedNode = saturneInclinedNode->CreateChild("saturnecylinderInclined");
    saturne_cylinderInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    saturne_cylinderInclinedNode->SetScale(Vector3(0.01f, 2.0f, 0.01f));
    //StaticModel* saturne_cylinderInclinedObject = saturne_cylinderInclinedNode->CreateComponent<StaticModel>();
    //saturne_cylinderInclinedObject->SetModel(cache->GetResource<Model>("Models/Cylinder.mdl"));

    Node* saturneNode = saturneInclinedNode->CreateChild("saturne");
    saturneNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    saturneNode->SetScale(Vector3(0.9f, 0.9f, 0.9f));
    StaticModel* saturneObject = saturneNode->CreateComponent<StaticModel>();
    saturneObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    saturneObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/saturnemap.xml"));


    Node * saturn_ring = saturneInclinedNode->CreateChild("ring");
    saturn_ring->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    saturn_ring->SetScale(Vector3(1.5f, 0.01f, 1.5f));
    StaticModel* saturn_ringObject = saturn_ring->CreateComponent<StaticModel>();
    saturn_ringObject->SetModel(cache->GetResource<Model>("Models/Torus.mdl"));
    //saturn_ringObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/ring_saturne.xml"));

    // rotation de saturne autour du soleil
    Node* saturnePosRotNode = saturnePosNode->CreateChild("saturnePosRot");
    saturnePosRotNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatorsaturnePosRot = saturnePosRotNode->CreateComponent<Rotator>();
    rotatorsaturnePosRot->SetRotationSpeed(Vector3(0.0f, 10.0f, 0.0f));





     //############## creation uranus ###################
    Node * uranus_orbit = sunPosNode->CreateChild("uranusOrbit");
    uranus_orbit->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatoruranus = uranus_orbit->CreateComponent<Rotator>();
    rotatoruranus->SetRotationSpeed(Vector3(0.0f, RES_T /84 , 0.0f));


    uranusPosNode = uranus_orbit->CreateChild("uranusPos");
    uranusPosNode->SetPosition(Vector3(5.0f * UA, 0.0f, 0.0f));

    Node* lightNode_uranus = uranusPosNode->CreateChild("DirectionalLight");
    lightNode_uranus->SetPosition(Vector3( -1.5, 0.0f, 0.0f));
    Light* light_uranus = lightNode_uranus->CreateComponent<Light>();
    light_uranus->SetBrightness(1.0);
    //earthPosNode->SetScale(Vector3(1.0f, 1.0f, 1.0f));
    //Rotator* rotatorEarthPos = earthPosNode->CreateComponent<Rotator>();
    //rotatorEarthPos->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));


    //axe non horizontale de uranus
    Node* uranusInclinedNode = uranusPosNode->CreateChild("uranusInclined");
    uranusInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    uranusInclinedNode->SetRotation(Quaternion(0.0f, 0.0f, 23.0f));

    Node* uranus_cylinderInclinedNode = uranusInclinedNode->CreateChild("uranuscylinderInclined");
    uranus_cylinderInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    uranus_cylinderInclinedNode->SetScale(Vector3(0.01f, 2.0f, 0.01f));
    //StaticModel* uranus_cylinderInclinedObject = uranus_cylinderInclinedNode->CreateComponent<StaticModel>();
    //uranus_cylinderInclinedObject->SetModel(cache->GetResource<Model>("Models/Cylinder.mdl"));

    Node* uranusNode = uranusInclinedNode->CreateChild("uranus");
    uranusNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    uranusNode->SetScale(Vector3(0.57f, 0.57f, 0.57f));
    StaticModel* uranusObject = uranusNode->CreateComponent<StaticModel>();
    uranusObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    uranusObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/uranusmap.xml"));

    // rotation de uranus autour du soleil
    Node* uranusPosRotNode = uranusPosNode->CreateChild("uranusPosRot");
    uranusPosRotNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatoruranusPosRot = uranusPosRotNode->CreateComponent<Rotator>();
    rotatoruranusPosRot->SetRotationSpeed(Vector3(0.0f, 10.0f, 0.0f));





     //############## creation neptune ###################
    Node * neptune_orbit = sunPosNode->CreateChild("neptuneOrbit");
    neptune_orbit->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatorneptune = neptune_orbit->CreateComponent<Rotator>();
    rotatorneptune->SetRotationSpeed(Vector3(0.0f, RES_T /165 , 0.0f));


    Node * neptunePosNode = neptune_orbit->CreateChild("neptunePos");
    neptunePosNode->SetPosition(Vector3(7.5f * UA, 0.0f, 0.0f));

    Node* lightNode_neptune = neptunePosNode->CreateChild("DirectionalLight");
    lightNode_neptune->SetPosition(Vector3( -1.5, 0.0f, 0.0f));
    Light* light_neptune = lightNode_neptune->CreateComponent<Light>();
    light_neptune->SetBrightness(1.0);
    //earthPosNode->SetScale(Vector3(1.0f, 1.0f, 1.0f));
    //Rotator* rotatorEarthPos = earthPosNode->CreateComponent<Rotator>();
    //rotatorEarthPos->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));


    //axe non horizontale de neptune
    Node* neptuneInclinedNode = neptunePosNode->CreateChild("neptuneInclined");
    neptuneInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    neptuneInclinedNode->SetRotation(Quaternion(0.0f, 0.0f, 23.0f));

    Node* neptune_cylinderInclinedNode = neptuneInclinedNode->CreateChild("neptunecylinderInclined");
    neptune_cylinderInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    neptune_cylinderInclinedNode->SetScale(Vector3(0.01f, 2.0f, 0.01f));
    //StaticModel* neptune_cylinderInclinedObject = neptune_cylinderInclinedNode->CreateComponent<StaticModel>();
    //neptune_cylinderInclinedObject->SetModel(cache->GetResource<Model>("Models/Cylinder.mdl"));

    Node* neptuneNode = neptuneInclinedNode->CreateChild("neptune");
    neptuneNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    neptuneNode->SetScale(Vector3(0.53f, 0.53f, 0.53f));
    StaticModel* neptuneObject = neptuneNode->CreateComponent<StaticModel>();
    neptuneObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    neptuneObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/neptunemap.xml"));

    // rotation de neptune autour du soleil
    Node* neptunePosRotNode = neptunePosNode->CreateChild("neptunePosRot");
    neptunePosRotNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotatorneptunePosRot = neptunePosRotNode->CreateComponent<Rotator>();
    rotatorneptunePosRot->SetRotationSpeed(Vector3(0.0f, 10.0f, 0.0f));



    //################# material for rocket ######################
    rocket_traj_center = sunPosRotNode->CreateChild("rocket_traj_center");
    rocket_traj_center->SetPosition(Vector3( -((5.0f + 1.5*5.0f)/2 -5), 0.0f, 0.0f));

    rocketPosNode = earthPosNode->CreateChild("rocketPos");
    //rocketPosNode->SetParent(rocket_orbit);

    Node* rocketInclinedNode = rocketPosNode->CreateChild("rocketInclined");
    rocketInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    rocketInclinedNode->SetRotation(Quaternion(90.0f, 90.0f, 90.0f));
    


    Node* rocketNode = rocketInclinedNode->CreateChild("rocket");
    rocketNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    rocketNode->SetScale(Vector3(0.02f, 0.02f, 0.02f));
    StaticModel* rocketObject = rocketNode->CreateComponent<StaticModel>();
    rocketObject->SetModel(cache->GetResource<Model>("Models/fusee.mdl"));
    rocketObject->SetMaterial(cache->GetResource<Material>("Materials/fusee.xml"));



    

    // Create a directional light to the world so that we can see something. The light scene node's orientation controls the
    // light direction; we will use the SetDirection() function which calculates the orientation from a forward direction vector.
    // The light will use default settings (white light, no shadows)
    Node* lightNode = scene_->CreateChild("DirectionalLight");
    //lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f)); // The direction vector does not need to be normalized
    Light* light = lightNode->CreateComponent<Light>();
    //light->SetLightType(LIGHT_SPOT);
    light->SetBrightness(1.0);

    Node* lightNode1 = scene_->CreateChild("DirectionalLight");
    lightNode1->SetPosition(Vector3(SUN_R, 0.0f, 0.0f));
    Light* light1 = lightNode1->CreateComponent<Light>();
    light->SetBrightness(1.0);

    Node* lightNode2 = scene_->CreateChild("DirectionalLight");
    lightNode2->SetPosition(Vector3(-SUN_R, 0.0f, 0.0f));
    Light* light2 = lightNode2->CreateComponent<Light>();
    light->SetBrightness(1.0);

    Node* lightNode3 = scene_->CreateChild("DirectionalLight");
    lightNode3->SetPosition(Vector3(0.0f, SUN_R, 0.0f));
    Light* light3 = lightNode3->CreateComponent<Light>();
    light->SetBrightness(1.0);

    Node* lightNode4 = scene_->CreateChild("DirectionalLight");
    lightNode4->SetPosition(Vector3(0.0f, -SUN_R, 0.0f));
    Light* light4 = lightNode4->CreateComponent<Light>();
    light->SetBrightness(1.0);

    Node* lightNode5 = scene_->CreateChild("DirectionalLight");
    lightNode5->SetPosition(Vector3(0.0f, 0.0f, SUN_R));
    Light* light5 = lightNode5->CreateComponent<Light>();
    light->SetBrightness(1.0);

    Node* lightNode6 = scene_->CreateChild("DirectionalLight");
    lightNode6->SetPosition(Vector3(0.0f, 0.0f, -SUN_R));
    Light* light6 = lightNode6->CreateComponent<Light>();
    light->SetBrightness(1.0);

    // Create a scene node for the camera, which we will move around
    // The camera will use default settings (1000 far clip distance, 45 degrees FOV, set aspect ratio automatically)
    cameraNode_ = scene_->CreateChild("Camera");
    cameraNode_->CreateComponent<Camera>();

    // Set an initial position for the camera scene node above the plane
    cameraNode_->SetPosition(Vector3(0.0f, 15.0f, 0.0f));
    //cameraNode_->SetRotation(Quaternion(90.0f, 90.0f, 90.0f));
    
}


void StaticScene::CreateInstructions()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();

/*
    // Construct new Text object, set string to display and font to use
    Text* instructionText = ui->GetRoot()->CreateChild<Text>();
    instructionText->SetText("Use WASD keys and mouse/touch to move");
    instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);

    // Position the text relative to the screen center
    instructionText->SetHorizontalAlignment(HA_CENTER);
    instructionText->SetVerticalAlignment(VA_CENTER);
    instructionText->SetPosition(0, ui->GetRoot()->GetHeight() / 4);
*/
}

void StaticScene::SetupViewport()
{
    Renderer* renderer = GetSubsystem<Renderer>();

    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen. We need to define the scene and the camera
    // at minimum. Additionally we could configure the viewport screen size and the rendering path (eg. forward / deferred) to
    // use, but now we just use full screen and default render path configured in the engine command line options
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
}


float StaticScene::Dist( Vector3 a, Vector3 b){
    return sqrt( pow(a.x_ - b.x_,2) + pow(a.y_ - b.y_,2) + pow(a.z_ - b.z_,2) );
}

void StaticScene::rocketLaunch(){
    Vector3 earthPos = earthPosNode->GetWorldPosition();
    Vector3 marsPos  = marsPosNode->GetWorldPosition();
    Vector3 sunPos   = sunPosNode->GetWorldPosition();

    float terre_soleil = Dist(earthPos, sunPos) ;
    float mars_soleil  = Dist(marsPos , sunPos) ;
    float terre_mars   = Dist(earthPos, marsPos);

    //int angle_terre_mars = acos(terre_soleil/mars_soleil)* 180.0 / PI;
    int angle_terre_mars = acos( (pow(terre_soleil,2) + pow(mars_soleil,2) - pow(terre_mars,2))/(2*terre_soleil*mars_soleil)   )* 180.0 / PI;


    // printf("%d\n",angle_terre_mars );
    // ResourceCache* cache = GetSubsystem<ResourceCache>();
    // UI* ui = GetSubsystem<UI>();
    // Text* instructionText = ui->GetRoot()->CreateChild<Text>();
    // char buffer [50];
    // instructionText->SetText(buffer);
    // instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
    
    // sprintf(buffer, "%d", angle_terre_mars);
    // instructionText->SetText(buffer);
    // instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);



    if (angle_terre_mars != 44 and !autorised ){
        autorised = true;
    }

    if (angle_terre_mars == 44 and autorised ){
        autorised = false;
        tkt++;
        if(tkt%2 == 0){

            //create center of rocket orbit
            Node * trajectory_center = scene_->CreateChild("trajectory_center");
            Vector3 ctrPos = rocket_traj_center->GetWorldPosition();
            trajectory_center->SetPosition(ctrPos);

            //create rocket position
            Node * rocket_orbit = trajectory_center->CreateChild("rocket_orbit");
            rocket_orbit->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
            Rotator* rotatorRocket = rocket_orbit->CreateComponent<Rotator>();
            rotatorRocket->SetRotationSpeed(Vector3(0.0f, RES_T * 0.73 , 0.0f));


            // Node * rocketPosNode = rocket_orbit->CreateChild("rocketPos");
            // rocketPosNode->SetPosition(Vector3( (5.0f + 1.5*5.0f)/2, 0.0f, 0.0f));
            rocketPosNode->SetParent(earthPosNode);
            rocketPosNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
            rocketPosNode->SetParent(rocket_orbit);

         //    rocketInclinedNode->SetParent(rocketPosNode);
         //    rocketInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
            // rocketInclinedNode->SetRotation(Quaternion(90.0f, 90.0f, 90.0f));


            // rocketNode->SetParent(rocketInclinedNode);
         //    rocketNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
         //    rocketNode->SetScale(Vector3(0.02f, 0.02f, 0.02f));
            // StaticModel* rocketObject = rocketNode->CreateComponent<StaticModel>();
            // rocketObject->SetModel(cache->GetResource<Model>("Models/fusee.mdl"));
            // rocketObject->SetMaterial(cache->GetResource<Material>("Materials/fusee.xml"));
        }


    }

    Vector3 rocketPos = rocketPosNode->GetWorldPosition();

    if(Dist(rocketPos,marsPos) < 0.01){
        rocketPosNode->SetParent(marsPosNode);
        rocketPosNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));

        //rocketInclinedNode = rocketPosNode->CreateChild("rocketInclined");
     //    rocketInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
     //    rocketInclinedNode->SetRotation(Quaternion(90.0f, 90.0f, 90.0f));


        // //Node* rocketNode = rocketInclinedNode->CreateChild("rocket");
     //    rocketNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
     //    rocketNode->SetScale(Vector3(0.02f, 0.02f, 0.02f));
     //    // StaticModel* rocketObject = rocketNode->CreateComponent<StaticModel>();
        // rocketObject->SetModel(cache->GetResource<Model>("Models/fusee.mdl"));
        // rocketObject->SetMaterial(cache->GetResource<Material>("Materials/fusee.xml"));

    }

    

}


void StaticScene::MoveCamera(float timeStep)
{
    // Do not move if the UI has a focused element (the console)
    if (GetSubsystem<UI>()->GetFocusElement())
        return;

    Input* input = GetSubsystem<Input>();

    // Movement speed as world units per second
    float MOVE_SPEED = 0.5f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
    IntVector2 mouseMove = input->GetMouseMove();
/*
    yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
    pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
    //pitch_ = 60.0f;

    pitch_ = Clamp(pitch_, -90.0f, 90.0f);
*/

    // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
    // Use the Translate() function (default local space) to move relative to the node's orientation.
    if (input->GetKeyDown(KEY_LSHIFT))
        MOVE_SPEED = 10;
    if (input->GetKeyDown('Z'))
        cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('S'))
        cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('Q'))
        cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('D'))
        cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
    
/*
    //Vector3 cpos=cameraNode_->GetPosition();
    Vector3 cpos=earthPosNode->GetPosition();
    if (input->GetKeyDown('J'))
    {
                float dx=MOVE_SPEED * timeStep;
                cpos.x_=cpos.x_+dx;
                //cameraNode_->SetPosition(cpos);
                earthPosNode->SetPosition(cpos);
            //cameraNode_->Translate(Vector3(MOVE_SPEED * timeStep,0.0f,0.0f));
    }
    if (input->GetKeyDown('H'))
    {
                float dx=MOVE_SPEED * timeStep;
                cpos.x_=cpos.x_-dx;
                //cameraNode_->SetPosition(cpos);
                earthPosNode->SetPosition(cpos);
            //cameraNode_->Translate(Vector3(MOVE_SPEED * timeStep,0.0f,0.0f));
    }
        //cameraNode_->Translate(Vector3(-MOVE_SPEED * timeStep,0.0f,0.0f));
    if (input->GetKeyDown('U'))
        {
                float dz=MOVE_SPEED * timeStep;
                cpos.z_=cpos.z_+dz;
                //cameraNode_->SetPosition(cpos);
                earthPosNode->SetPosition(cpos);
        }
        //cameraNode_->Translate(Vector3(0.0f,MOVE_SPEED * timeStep,0.0f));
    if (input->GetKeyDown('N'))
        {
                float dz=-MOVE_SPEED * timeStep;
                cpos.z_=cpos.z_+dz;
                //cameraNode_->SetPosition(cpos);
                earthPosNode->SetPosition(cpos);
        }

    if (nbJoysticks>0)
        ManageJoystick(timeStep);
*/
    if (input->GetKeyPress('I'))
        {
        printf("I\n");
        myAngle-=36;
        printf("myAngle=%d\n",myAngle);
    }

    if (input->GetKeyPress('O'))
        {
        printf("O\n");
        myAngle+=36;
        printf("myAngle=%d\n",myAngle);
    }

    // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
    //cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
    cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

}

void StaticScene::ManageJoystick(float timeStep)
{
    const float MOVE_SPEED = 20.0f;
    Vector3 cpos=cameraNode_->GetPosition();
    //Vector3 posn=posNode->GetPosition();

    double xJ0=js->GetAxisPosition(0);
    double yJ0=js->GetAxisPosition(1);
    double xJ1=js->GetAxisPosition(2);
    double yJ1=js->GetAxisPosition(3);

        if (xJ0>0.1)
            {
                    float dx=MOVE_SPEED* xJ0 * timeStep;
                    cpos.x_=cpos.x_+dx;
                    cameraNode_->SetPosition(cpos);
            }
        if (xJ0<-0.1)
            {
                    float dx=MOVE_SPEED * xJ0 * timeStep;
                    cpos.x_=cpos.x_+dx;
                    cameraNode_->SetPosition(cpos);
            }
        if (yJ0<-0.1)
            {
                    float dz=MOVE_SPEED * yJ0 * timeStep;
                    cpos.z_=cpos.z_-dz;
                    cameraNode_->SetPosition(cpos);
            }
        if (yJ0>0.1)
            {
                    float dz=MOVE_SPEED * yJ0 * timeStep;
                    cpos.z_=cpos.z_-dz;
                    cameraNode_->SetPosition(cpos);
            }

/*
    printf("Buttons=%d\n",js->GetNumButtons());
    printf("01234567890123456789\n");
    printf("00000000001111111111\n");
    for (int i=0;i<js->GetNumButtons();i++)
        if (js->GetButtonDown(i)==true)
            printf("1");
        else
            printf("0");
    printf("\n");
    printf("Hats=%d\n",js->GetNumHats());

    printf("%f %f\n", (float)js->GetAxisPosition(2), (float)js->GetAxisPosition(3)); 
*/
    
        if (xJ1>0.1)
            {
                    float dy=MOVE_SPEED * 2.0 * xJ1 * timeStep;
                    cpos.y_=cpos.y_-dy;
                    cameraNode_->SetPosition(cpos);
            }
        if (xJ1<-0.1)
            {
                    float dy=MOVE_SPEED * 2.0 * xJ1 * timeStep;
                    cpos.y_=cpos.y_-dy;
                    cameraNode_->SetPosition(cpos);
            }

        if (yJ1>0.1)
            {
                    float amount=MOVE_SPEED * 2.0 * yJ1 * timeStep;
            printf("amount=%f\n",amount);
            pitch_ +=amount;
            }
        if (yJ1<-0.1)
            {
                    float amount=MOVE_SPEED * 2.0 * yJ1 * timeStep;
            printf("amount=%f\n",amount);
            pitch_ +=amount;
            }

    if (js->GetButtonPress(13))
            {
            std::cout << "--------------------------------------------------------- bouton 13" << std::endl;
            if (possibleDirections[cursorLocation].wt!=-1)
            {
                moveObjectToPoint((char*)"MoralePawn",possibleDirections[cursorLocation].w);
                cursorLocation=possibleDirections[cursorLocation].wt;
            }
            }
    if (js->GetButtonPress(14))
            {
            std::cout << "--------------------------------------------------------- bouton 14" << std::endl;
            if (possibleDirections[cursorLocation].et!=-1)
            {
                moveObjectToPoint((char*)"MoralePawn",possibleDirections[cursorLocation].e);
                cursorLocation=possibleDirections[cursorLocation].et;
            }
            }
    if (js->GetButtonPress(11))
            {
            std::cout << "--------------------------------------------------------- bouton 11" << std::endl;
            if (possibleDirections[cursorLocation].nt!=-1)
            {
                moveObjectToPoint((char*)"MoralePawn",possibleDirections[cursorLocation].n);
                cursorLocation=possibleDirections[cursorLocation].nt;
            }
            }
    if (js->GetButtonPress(12))
            {
            std::cout << "--------------------------------------------------------- bouton 12" << std::endl;
            if (possibleDirections[cursorLocation].st!=-1)
            {
                moveObjectToPoint((char*)"MoralePawn",possibleDirections[cursorLocation].s);
                cursorLocation=possibleDirections[cursorLocation].st;
            }
            }

        pitch_ = Clamp(pitch_, -90.0f, 90.0f);

    if (js->GetButtonPress(4))
            {
                    cpos.x_=0.0;
                    cpos.y_=15.0;
                    cpos.z_=0.0;
            pitch_=60.0;
                    cameraNode_->SetPosition(cpos);
            }

}

void StaticScene::SubscribeToEvents()
{
    // Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(StaticScene, HandleUpdate));

        // Start server

        Network* network = GetSubsystem<Network>();
        network->StartServer(myPort);

        // Subscribe to network events

        SubscribeToEvent(E_CLIENTCONNECTED, URHO3D_HANDLER(StaticScene, HandleClientConnected));
        SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(StaticScene, HandleClientDisconnected));
        SubscribeToEvent(E_NETWORKMESSAGE, URHO3D_HANDLER(StaticScene, HandleNetworkMessage));

}

void StaticScene::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    // Move the camera, scale movement with time step
    
    MoveCamera(timeStep);
    rocketLaunch();
}

void StaticScene::HandleClientConnected(StringHash eventType, VariantMap& eventData)
{
        printf("Client connected\n");
}

void StaticScene::HandleClientDisconnected(StringHash eventType, VariantMap& eventData)
{
        printf("Client disconnected\n");
}

void StaticScene::HandleNetworkMessage(StringHash eventType, VariantMap& eventData)
{
        float vx,vz;
        // Quaternion qtn = cameraNode_->GetRotation();
        // float pitch_ = qtn.x_;
        // float yaw_ = qtn.y_;

        Network* network = GetSubsystem<Network>();

        using namespace NetworkMessage;

        int msgID = eventData[P_MESSAGEID].GetInt();
        Connection* remoteSender = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());

        std::cout << "HandleNetworkMessage" << std::endl;

        if (msgID == MSG_GAME)
        {
            
            const PODVector<unsigned char>& data = eventData[P_DATA].GetBuffer();
            // Use a MemoryBuffer to read the message data so that there is no unnecessary copying
            MemoryBuffer msg(data);
            String text = msg.ReadString();
            char s[100],com[100];
            int tx,tz,tnum,torient;
            strcpy(s,text.CString());
            printf("Message received:%s\n",s);
            float timeStep = 0.1;
            float MOVE_SPEED = 50.0f;

        
            // Read ZQSD keys and move the camera scene node to the corresponding direction if they are pressed
            // Use the Translate() function (default local space) to move relative to the node's orientation.
            if (s[0]=='z') {
                //cameraNode_->SetParent(scene_);
                printf("command interpreted : %s , %f\n", s,timeStep);
                /*cameraNode_->SetRotation(Quaternion(pitch_, yaw_ - myAngle, 0.0f));
                cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
                cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));*/
                cameraNode_->SetRotation(Quaternion(pitch_, yaw_ - myAngle, 0.0f));
                cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
                //cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
            }
        
            else if (s[0]=='q') {
                //cameraNode_->SetParent(scene_);
                // cameraNode_->SetRotation(Quaternion(pitch_, yaw_ - myAngle, 0.0f));
                // cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
                // cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

                cameraNode_->SetRotation(Quaternion(pitch_, yaw_ - myAngle, 0.0f));
                cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
                //cameraNode_->SetRotation(Quaternion(pitch_, yaw_ , 0.0f));
            }
        
            else if (s[0]=='s') {
                //cameraNode_->SetParent(scene_);
                cameraNode_->SetRotation(Quaternion(pitch_, yaw_ - myAngle, 0.0f));
                cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
                //cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
            }
        
            else if (s[0]=='d') {
                //cameraNode_->SetParent(scene_);
                cameraNode_->SetRotation(Quaternion(pitch_, yaw_ - myAngle, 0.0f));
                cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
                //cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
            }
        
            // else if (s[0]=='o') {
            //     // pitch_ += 30.0f;
            //     // pitch_ = Clamp(pitch_, -90.0f, 90.0f);
            //     // printf("command interpreted pitch: %s , %f \n", s,pitch_);
            //     // cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 90.0f));

            //     pitch_ += 30.0f;
            //     pitch_ = Clamp(pitch_, -90.0f, 90.0f);

            // }

            else if (s[0] == 'o'){
                cameraNode_->SetRotation(Quaternion(pitch_, yaw_ - myAngle, 0.0f));
                cameraNode_->Translate(Vector3::UP * MOVE_SPEED * timeStep);
            }

            else if (s[0] == 'l'){
                cameraNode_->SetRotation(Quaternion(pitch_, yaw_ - myAngle, 0.0f));
                cameraNode_->Translate(Vector3::DOWN * MOVE_SPEED * timeStep);
            }
        
            else if (s[0]=='k') {
                yaw_ -= 30.0f;
                cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
            }
        
            // else if (s[0]=='l') {
            //     pitch_ -= 30.0f;
            //     pitch_ = Clamp(pitch_, -90.0f, 90.0f);
            //     cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
            // }
        
            else if (s[0]=='m') {
                yaw_ += 30.0f;
                cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
            }

            else if (s[0]=='f') {
                pitch_ = 0;
                yaw_   = myAngle;
                camera_fusee = rocketPosNode->CreateChild("camera_fusee");
                camera_fusee->SetPosition(Vector3(-0.2f, 0.1f, -1.0f));
                cameraNode_->SetParent(camera_fusee);
                cameraNode_->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
            }

            else if (s[0]=='t') {
                pitch_ = 0;
                yaw_   = myAngle;
                camera_fusee = earthPosNode->CreateChild("camera_fusee");
                camera_fusee->SetPosition(Vector3(-0.3f, 0.1f, -1.0f));
                cameraNode_->SetParent(camera_fusee);
                cameraNode_->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
            }

            else if (s[0]=='S') {
                pitch_ = 0;
                yaw_   = myAngle;
                camera_fusee = sunPosNode->CreateChild("camera_fusee");
                camera_fusee->SetPosition(Vector3(0.0f, 5.1f, -5.0f));
                cameraNode_->SetParent(camera_fusee);
                cameraNode_->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
            }

            else if (s[0]=='p')
            {
                if(scene_->IsUpdateEnabled())
                    scene_->SetUpdateEnabled( false);
                else
                    scene_->SetUpdateEnabled(true);
            }

            else if (s[0] == 'b')
            {
                if(sky){
                    sky = false;
                    skyNode->RemoveAllComponents();
                }
                else{
                    sky = true;
                    Skybox* skybox = skyNode->CreateComponent<Skybox>();
                    skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
                    skybox->SetMaterial(cache->GetResource<Material>("Materials/skybox_stars.xml"));
                }
            }

            else if (s[0]=='y'){
                if(!secret){
                    Sun_graphic->RemoveAllComponents();
                    StaticModel* sunObject = Sun_graphic->CreateComponent<StaticModel>();
                    sunObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
                    sunObject->SetMaterial(cache->GetResource<Material>("Materials/pecheux.xml"));
                    secret = true;
                }
                else{
                    Sun_graphic->RemoveAllComponents();
                    StaticModel* sunObject = Sun_graphic->CreateComponent<StaticModel>();  
                    sunObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
                    sunObject->SetMaterial(cache->GetResource<Material>("Materials/sun.xml"));
                    secret = false;

                }
            }
            else if (s[0]=='r'){
                pitch_ = 0;
                yaw_   = myAngle;
                camera_fusee = rocket_traj_center->CreateChild("camera_soleil");
                camera_fusee->SetPosition(Vector3(0.0f, 0.0f, -3.0f));
                cameraNode_->SetParent(camera_fusee);
                cameraNode_->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
            }
            else if (s[0]=='j'){
                pitch_ = 0;
                yaw_   = myAngle;
                camera_fusee = jupiterPosNode->CreateChild("camera_soleil");
                camera_fusee->SetPosition(Vector3(0.0f, 0.0f, -3.0f));
                cameraNode_->SetParent(camera_fusee);
                cameraNode_->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
            }

            else if (s[0]=='u'){
                pitch_ = 0;
                yaw_   = myAngle;
                camera_fusee = uranusPosNode->CreateChild("camera_soleil");
                camera_fusee->SetPosition(Vector3(0.0f, 0.0f, -3.0f));
                cameraNode_->SetParent(camera_fusee);
                cameraNode_->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
            }
            else if (s[0] == '*')
            {
                if(!sky_secret){
                    sky_secret = true;
                    skyNode->RemoveAllComponents();
                    Skybox* skybox = skyNode->CreateComponent<Skybox>();
                    skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
                    skybox->SetMaterial(cache->GetResource<Material>("Materials/pecheux_sky.xml"));
                }
                else{
                    sky_secret = false;
                    skyNode->RemoveAllComponents();
                    Skybox* skybox = skyNode->CreateComponent<Skybox>();
                    skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
                    skybox->SetMaterial(cache->GetResource<Material>("Materials/skybox_stars.xml"));
                }
            }

                   
            else if (s[0]=='x')
            {
                    // quit
            }  
        }
}


// ===================================================================

void StaticScene::CreateObject(char *uniqname,
        Vector3& pos, Vector3& scale, Quaternion& quat,
        char *model, char *material1, char *material2, int visible)
{

        Node* oNode = scene_->CreateChild(uniqname);
        oNode->SetPosition(pos);
        oNode->SetScale(scale);
        oNode->SetRotation(quat);
        StaticModel* oObject = oNode->CreateComponent<StaticModel>();
        char oModel[100];
        sprintf(oModel,"Models/%s",model);
        oObject->SetModel(cache->GetResource<Model>(oModel));
        char oMaterial1[100];
        sprintf(oMaterial1,"Materials/%s",material1);
        Material *mm1=cache->GetResource<Material>(oMaterial1);
        oObject->SetMaterial(cache->GetResource<Material>(oMaterial1));
        char oMaterial2[100];
        sprintf(oMaterial2,"Materials/%s",material2);
        Material *mm2=cache->GetResource<Material>(oMaterial2);

        oObject->SetMaterial(mm1);

        if (visible==1)
        {
                oObject->SetMaterial(mm1);
                //oNode->SetEnabled(true);
        }
        else
        {
                oObject->SetMaterial(mm2);
                //oNode->SetEnabled(false);
        }

        nodeMap.insert(std::make_pair(uniqname,oNode));
}

void StaticScene::CreateObjectAtPoint(char *uniqname, char *pointname,
        Vector3& scale, Quaternion& quat,
        char *model, char *material1, char *material2, int visible)
{
        Node* oNode = scene_->CreateChild(uniqname);
    Vector3 *n=pointMap[pointname];
        oNode->SetPosition(*n);
        oNode->SetScale(scale);
        oNode->SetRotation(quat);
        StaticModel* oObject = oNode->CreateComponent<StaticModel>();
        char oModel[100];
        sprintf(oModel,"Models/%s",model);
        oObject->SetModel(cache->GetResource<Model>(oModel));
        char oMaterial1[100];
        sprintf(oMaterial1,"Materials/%s",material1);
        Material *mm1=cache->GetResource<Material>(oMaterial1);
        oObject->SetMaterial(cache->GetResource<Material>(oMaterial1));
        char oMaterial2[100];
        sprintf(oMaterial2,"Materials/%s",material2);
        Material *mm2=cache->GetResource<Material>(oMaterial2);

        oObject->SetMaterial(mm1);

        if (visible==1)
        {
                oObject->SetMaterial(mm1);
        }
        else
        {
                oObject->SetMaterial(mm2);
        }

        nodeMap.insert(std::make_pair(uniqname,oNode));
}

void StaticScene::CreateObjectFromString(char *command)
{
        float posx, posy, posz;
        float scalex, scaley, scalez;
        float quatx, quaty, quatz;
        char uniqname[100];
        char model[100];
        char material1[100];
        char material2[100];
        int vis;

        sscanf(command+3,"%s %f %f %f %f %f %f %f %f %f %s %s %s %d",
                uniqname, &posx, &posy, &posz, &scalex, &scaley, &scalez,
                &quatx, &quaty, &quatz, model, material1, material2, &vis);

        printf("CreateObjectFromString %s %f %f %f %f %f %f %f %f %f %s %s %s %d\n",
                uniqname, posx, posy, posz, scalex, scaley, scalez,
                quatx, quaty, quatz, model, material1, material2, vis);

        Vector3 pos(posx,posy,posz);
        Vector3 scale(scalex,scaley,scalez);
        Quaternion quat(quatx,quaty,quatz);

        CreateObject(uniqname,pos,scale,quat,model,material1,material2,vis);
}

void StaticScene::CreateObjectAtPointFromString(char *command)
{
        float scalex, scaley, scalez;
        float quatx, quaty, quatz;
        char uniqname[100];
        char pointname[100];
        char model[100];
        char material1[100];
        char material2[100];
        int vis;

        sscanf(command+3,"%s %s %f %f %f %f %f %f %s %s %s %d",
                uniqname, pointname, &scalex, &scaley, &scalez,
                &quatx, &quaty, &quatz, model, material1, material2, &vis);

        printf("CreateObjectAtPointFromString %s %s %f %f %f %f %f %f %s %s %s %d\n",
                uniqname, pointname, scalex, scaley, scalez,
                quatx, quaty, quatz, model, material1, material2, vis);

        Vector3 scale(scalex,scaley,scalez);
        Quaternion quat(quatx,quaty,quatz);

        CreateObjectAtPoint(uniqname,pointname,scale,quat,model,material1,material2,vis);
}

Vector3* StaticScene::CreatePoint(char *uniqname, Vector3 *pos)
{
        pointMap.insert(std::make_pair(uniqname,pos));
    return pos;
}

Vector3* StaticScene::CreatePointFromString(char *command)
{
        float posx, posy, posz;
        float scalex, scaley, scalez;
        float quatx, quaty, quatz;
        char uniqname[100];
        char model[100];
        char material1[100];
        char material2[100];
        int vis;

        sscanf(command+3,"%s %f %f %f",
                uniqname, &posx, &posy, &posz);

        printf("CreatePointFromString %s %f %f %f\n",
                uniqname, posx, posy, posz);

        Vector3 *pos=new Vector3(posx,posy,posz);

        return CreatePoint(uniqname,pos);
}

void StaticScene::moveObjectToPointFromString(char *command)
{
        char uniqname[100];
        char pointname[100];

        sscanf(command+3,"%s %s",
                uniqname, pointname);

        printf("moveObjectToPointFromString %s %s\n",
                uniqname, pointname);

    moveObjectToPoint(uniqname, pointname);
}

void StaticScene::moveObjectToPoint(char *uniqname, char *pointname)
{
    std::cout << "moveObjectToPoint " << uniqname << "," << pointname << std::endl;

        Node* oNode = nodeMap[uniqname];
    Vector3 *n=pointMap[pointname];
        oNode->SetPosition(*n);
}
