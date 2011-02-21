// main
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

// OpenEngine stuff
#include <Core/Engine.h>
#include "Display/BoundedCamera.h"
#include <Display/PerspectiveViewingVolume.h>
#include <Logging/Logger.h>
#include <Logging/StreamLogger.h>
#include <Meta/Config.h>
#include <Renderers/DataBlockBinder.h>
#include <Resources/ResourceManager.h>
#include <Scene/PointLightNode.h>
#include <Scene/ISceneNode.h>
#include <Scene/RenderStateNode.h>

// SDL extension
#include <Display/SDLEnvironment.h>

// Assimp
#include <Resources/AssimpResource.h>

// OpenGL
#include <Renderers/OpenGL/Renderer.h>
#include <Renderers/OpenGL/LightRenderer.h>
#include <Display/RenderCanvas.h>
#include <Display/OpenGL/TextureCopy.h>

// Generic Handler
#include <Utils/BetterMoveHandler.h>
#include <Utils/QuitHandler.h>

#include <Resources/FreeImage.h>

// Photon Mapping stuff
#include <Renderers/OpenGL/PhotonRenderingView.h>

#include <Utils/CUDA/Utils.h>

// Scene setup
#include "SceneCreator.h"

// Ant tweak bar
#include "Handlers.h"
#include <Display/AntTweakBar.h>
#include <Utils/IInspector.h>
#include <Utils/InspectionBar.h>


int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

// name spaces that we will be using.
// this combined with the above imports is almost the same as
// fx. import OpenEngine.Logging.*; in Java.
using namespace OpenEngine::Core;
using namespace OpenEngine::Display;
using namespace OpenEngine::Display::OpenGL;
using namespace OpenEngine::Logging;
using namespace OpenEngine::Renderers::OpenGL;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Utils;

ISceneNode* CreateForest() {
    IModelResourcePtr mdl = ResourceManager<IModelResource>::Create("projects/PhotonMapping/data/fairyForest/fairyForest.obj");
    mdl->Load();
    return mdl->GetSceneNode();
}

ISceneNode* CreateSibenik() {
    IModelResourcePtr mdl = ResourceManager<IModelResource>::Create("projects/PhotonMapping/data/sibenik/sibenik.obj");
    mdl->Load();
    return mdl->GetSceneNode();
}

TransformationNode *geomTrans = new TransformationNode();

ISceneNode* SetupScene(std::string name, BoundedCamera* cam){

    RenderStateNode* rsNode = new RenderStateNode();
    rsNode->DisableOption(RenderStateNode::BACKFACE);
    rsNode->EnableOption(RenderStateNode::LIGHTING);
    rsNode->EnableOption(RenderStateNode::COLOR_MATERIAL);

    TransformationNode* lightTrans = new TransformationNode();
    lightTrans->SetPosition(Vector<3, float>(0.0f, 4.0f, 0.0f));
    rsNode->AddNode(lightTrans);

    PointLightNode* light = new PointLightNode();
    Vector<4, float> lightColor = Vector<4, float>(255.0f, 235.0f, 205.0f, 255.0f) / 255.0f;
    light->ambient = lightColor * 0.3;
    light->diffuse = lightColor * 0.7;
    light->specular = lightColor * 0.3;
    lightTrans->AddNode(light);

    geomTrans = new TransformationNode();

    SceneCreator::CreateScene(name, rsNode, cam, geomTrans);

    return rsNode;
}

// Setup ant tweak bar
AntTweakBar* SetupAntTweakBar(PhotonRenderingView* rv, IRenderer* renderer,
                      IKeyboard* keyboard, IMouse* mouse){
    AntTweakBar* atb = new AntTweakBar();
    atb->ToggleEnabled();
    keyboard->KeyEvent().Attach(*atb);
    mouse->MouseMovedEvent().Attach(*atb);
    mouse->MouseButtonEvent().Attach(*atb);
    atb->AttachTo(*renderer);
    new RayInspectionBar(atb, rv, renderer, geomTrans);
    return atb;
}

/**
 * Main method for the first quarter project of CGD.
 * Corresponds to the
 *   public static void main(String args[])
 * method in Java.
 */
int main(int argc, char** argv) {
    // Setup logging facilities.
    Logger::AddLogger(new StreamLogger(&std::cout));

    string sceneName = argc >= 2 ? string(argv[1]) : "";
    
    // Print usage info.
    logger.info << "========= Efficient Ray Tracing of Dynamic Scenes on the GPU =========" << logger.end;

    if (sceneName.compare("") == 0 || sceneName.compare("help") == 0){
        logger.info << "Usage: ./build/PhotonMapping/PhotonMapping SCENE" << logger.end;
        logger.info << "SCENEs:" << logger.end;
        logger.info << "  cornell" << logger.end;
        logger.info << "  sponza" << logger.end;
        logger.info << "  dragon" << logger.end;
        logger.info << "  bunny" << logger.end;
        exit(0);
    }

    // setup the engine
    Engine* engine = new Engine;
    IEnvironment* env = new SDLEnvironment(SCREEN_WIDTH, SCREEN_HEIGHT, 32);
    //IEnvironment* env = new SDLEnvironment(SCREEN_WIDTH, SCREEN_HEIGHT, 32, FRAME_FULLSCREEN);
    engine->InitializeEvent().Attach(*env);
    engine->ProcessEvent().Attach(*env);
    engine->DeinitializeEvent().Attach(*env);
    
    IRenderer* renderer = new Renderer();

    ResourceManager<IModelResource>::AddPlugin(new AssimpPlugin());
    ResourceManager<ITexture2D>::AddPlugin(new FreeImagePlugin());

    BoundedCamera* camera  = new BoundedCamera(*(new PerspectiveViewingVolume(1, 4000)));
    ISceneNode* scene = SetupScene(sceneName, camera);

    //DataBlockBinder* bob = new DataBlockBinder(*renderer);
    //bob->Bind(*scene);

    IFrame& frame = env->CreateFrame();

    PhotonRenderingView* renderingview = new PhotonRenderingView();
    renderer->InitializeEvent().Attach(*renderingview);    
    renderer->PreProcessEvent().Attach(*renderingview);
    renderer->ProcessEvent().Attach(*renderingview);

    LightRenderer *lightRenderer = new LightRenderer();
    renderer->PreProcessEvent().Attach(*lightRenderer);

    RenderCanvas* canvas = new RenderCanvas(new TextureCopy());
    canvas->SetViewingVolume(camera);
    canvas->SetRenderer(renderer);
    canvas->SetScene(scene);
    frame.SetCanvas(canvas);

    QuitHandler* quit_h = new QuitHandler(*engine);
    env->GetKeyboard()->KeyEvent().Attach(*quit_h);

    AntTweakBar* atb = SetupAntTweakBar(renderingview, renderer, 
                                        env->GetKeyboard(), env->GetMouse());

    HostTraceListener* tracer = new HostTraceListener(renderingview, Vector<2, int>(SCREEN_WIDTH, SCREEN_HEIGHT));
    atb->MouseButtonEvent().Attach(*tracer);
    
    AntToggler* antToggle = new AntToggler(atb);
    atb->KeyEvent().Attach(*antToggle);
    
    BetterMoveHandler *move = new BetterMoveHandler(*camera,
                                                    *(env->GetMouse()),
                                                    true);
    move->SetMoveScale(0.00001);

    engine->InitializeEvent().Attach(*move);
    engine->ProcessEvent().Attach(*move);
    atb->KeyEvent().Attach(*move);
    atb->MouseButtonEvent().Attach(*move);
    atb->MouseMovedEvent().Attach(*move);

    // Start the engine.
    engine->Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}
